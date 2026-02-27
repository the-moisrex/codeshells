import re
import subprocess
import os

def fix_cpp_code(code: str) -> str:
    """
    Fixes C++ code by addressing specific compilation issues after replacement.
    - Adds missing 'template <typename CharT>' or merges with existing templates.
    - Removes duplicate template declarations.
    - Replaces 'istl::char_type_of_t<decltype(param)>' with 'CharT' where applicable.
    - Replaces 'istl::view(param)' with 'param'.
    - Handles edge cases for function signatures and bodies.
    """

    # Step 1: Replace all 'istl::view(param)' with 'param' in the entire code.
    # This uses a regex to match 'istl::view(...)' and capture the inner content.
    def replace_view(match):
        return match.group(1).strip()

    code = re.sub(r'istl::view\s*\(([^)]+)\)', replace_view, code)

    # Step 2: Replace 'istl::char_type_of_t<decltype(param)>' with 'CharT'.
    # Assume param is a name like 'inp_str' or similar; match the pattern.
    def replace_char_type(match):
        return 'CharT'

    code = re.sub(r'istl::char_type_of_t<decltype\([^)]+\)>', replace_char_type, code)

    # Step 3: Fix function declarations with missing or duplicate templates.
    # Find function declarations that use 'stl::basic_string_view<CharT>'.
    # Pattern for function declaration:
    # Optional templates, then optional attributes, then return type, function name, params, optional noexcept/const etc.

    def fix_declaration(match):
        full_match = match.group(0)
        templates = match.group(1) or ''
        attributes = match.group(4) or ''
        signature = match.group(5)

        if signature is None:
            return full_match

        if 'stl::basic_string_view<CharT>' not in signature:
            return full_match

        # Normalize templates: find all 'template <...>' lines.
        template_lines = re.findall(r'template\s*<\s*typename\s*[^>]+>', templates)

        # Extract existing template params.
        existing_params = []
        for tl in template_lines:
            # Extract params like 'typename T, typename CharT'.
            params = re.findall(r'typename\s*(\w+)', tl)
            existing_params.extend(params)

        # Make unique
        unique_params = []
        seen = set()
        for p in existing_params:
            if p not in seen:
                unique_params.append(p)
                seen.add(p)

        # Add missing CharT if needed
        if 'CharT' not in seen:
            unique_params.append('CharT')
            seen.add('CharT')

        # Add missing T if 'T&&' appears in signature (indicating template T is used)
        if 'T&&' in signature and 'T' not in seen:
            unique_params.insert(0, 'T')  # Insert at beginning if needed
            seen.add('T')

        # Construct new template
        new_template = 'template <' + ', '.join(f'typename {p}' for p in unique_params) + '>' if unique_params else ''

        # Reconstruct: new_template + attributes + signature
        fixed = (new_template + '\n' if new_template else '') + attributes.strip() + '\n' + signature.strip()
        return fixed + '\n'  # Ensure newline for body if present.

    # Regex to match function declarations (simplified, assumes no nested functions).
    # Matches optional templates, optional attributes, then the signature line.
    decl_pattern = re.compile(
        r'((template\s*<.*?>(\s*template\s*<.*?>)*\s*)?)'  # Group 1: templates
        r'(\[\[.*?\]\]\s*)?'  # Group 4: attributes
        r'(\s*(?:constexpr\s*)?(?:bool|auto|void|[a-zA-Z:_]+)\s*[a-zA-Z:_]+\s*\([^)]*\)\s*(noexcept|const)?\s*\{?)',  # Group 5: signature
        re.DOTALL | re.MULTILINE
    )

    code = decl_pattern.sub(fix_declaration, code)

    # Additional cleanup: remove any leftover duplicate templates.
    code = re.sub(r'(template\s*<.*?>\s*){2,}', lambda m: m.group(1), code)

    return code

# Batch process the codebase
if __name__ == "__main__":
    try:
        # Get list of files tracked by git
        git_files = subprocess.check_output(['git', 'ls-files']).decode('utf-8').splitlines()
    except subprocess.CalledProcessError:
        print("Error: Not in a git repository or git not installed.")
        exit(1)

    # Filter for C++ source and header files
    cpp_files = [f for f in git_files if f.endswith(('.cpp', '.hpp', '.h', '.cc', '.cxx'))]

    for file_path in cpp_files:
        if not os.path.exists(file_path):
            continue  # Skip if file doesn't exist (though git ls-files should list existing)

        with open(file_path, 'r', encoding='utf-8') as f:
            original_code = f.read()

        fixed_code = fix_cpp_code(original_code)

        if fixed_code != original_code:
            print(f"Fixing {file_path}")
            with open(file_path, 'w', encoding='utf-8') as f:
                f.write(fixed_code)
        else:
            print(f"No changes needed for {file_path}")