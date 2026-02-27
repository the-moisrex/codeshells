#!/usr/bin/env python3
"""
Script to replace istl::StringViewifiable auto&& with stl::basic_string_view<CharT>
and remove istl::view usages across the webpp library.
"""

import re
import os
import sys
from pathlib import Path


def find_files(base_path="/home/moisrex/Projects/webpp"):
    """Find all header and source files to process."""
    base = Path(base_path)
    files = []

    for pattern in ["**/*.hpp", "**/*.cpp"]:
        files.extend(base.glob(pattern))

    # Filter out test and benchmark files
    filtered_files = []
    for file in files:
        if "test" not in str(file) and "benchmark" not in str(file):
            filtered_files.append(file)

    return filtered_files


def replace_stringviewifiable(content):
    """Replace istl::StringViewifiable auto&& with template + stl::basic_string_view<CharT>."""
    result = content

    # Pattern to match template declarations with StringViewifiable
    # Example: template <typename T> func(istl::StringViewifiable auto&& param)
    pattern1 = r"(template\s*<\s*typename\s+([A-Za-z_][A-Za-z0-9_]*)\s*>\s*\n(\s*)\([^\)]*istl::StringViewifiable\s+auto&&\s+([A-Za-z_][A-Za-z0-9_]*)"

    def replace_template_func(match):
        indent = match.group(3)
        return f"template <typename CharT>\n{indent}(stl::basic_string_view<CharT> const {match.group(4)}"

    result = re.sub(pattern1, replace_template_func, result, flags=re.MULTILINE)

    # Pattern for single-line template functions
    pattern2 = r"template\s*<\s*typename\s+([A-Za-z_][A-Za-z0-9_]*)\s*>\s*\([^\)]*istl::StringViewifiable\s+auto&&\s+([A-Za-z_][A-Za-z0-9_]*)"
    result = re.sub(
        pattern2,
        r"template <typename CharT> (stl::basic_string_view<CharT> const \2",
        result,
    )

    # Replace istl::StringViewifiable auto&& in function parameters
    result = re.sub(
        r"istl::StringViewifiable\s+auto&&\s+([A-Za-z_][A-Za-z0-9_]*)",
        r"stl::basic_string_view<CharT> const \1",
        result,
    )

    # Replace istl::StringViewifiable<Type> with stl::basic_string_view<CharT>
    result = re.sub(
        r"istl::StringViewifiable\s*<\s*[^>]+\s*>",
        r"stl::basic_string_view<CharT>",
        result,
    )

    return result


def remove_view_usages(content):
    """Remove istl::view and istl::view_of usages."""
    result = content

    # Replace istl::view(param) with param
    result = re.sub(r"istl::view\s*\(\s*([^)]+)\s*\)", r"\1", result)

    # Replace istl::view_of<Type>(param) with param
    result = re.sub(r"istl::view_of\s*<\s*[^>]+\s*>\s*\(\s*([^)]+)\s*\)", r"\1", result)

    return result


def add_missing_template_params(content):
    """Add missing template <typename CharT> where needed."""
    lines = content.split("\n")
    result = []
    i = 0

    while i < len(lines):
        line = lines[i]

        # Check if this line contains stl::basic_string_view<CharT> but we need a template
        if (
            "stl::basic_string_view<CharT>" in line
            and "template" not in line
            and ("constexpr" in line or "[[nodiscard]]" in line or "auto" in line)
            and "(" in line
            and ")" in line
        ):
            # Look backwards for a template declaration in the same function
            template_found = False
            j = i - 1
            while j >= max(0, i - 5):  # Look back up to 5 lines
                if "template" in lines[j] and "<" in lines[j] and "CharT" in lines[j]:
                    template_found = True
                    break
                if (
                    lines[j].strip() == ""
                    or lines[j].strip().startswith("{")
                    or lines[j].strip().startswith("}")
                    or lines[j].strip().startswith(";")
                ):
                    break
                j -= 1

            # If no template found and this looks like a function, add one
            if not template_found:
                # Check if this is likely a function declaration
                if any(
                    keyword in line
                    for keyword in ["constexpr", "[[nodiscard]]", "operator", "(", ")"]
                ):
                    # Find the indentation
                    indent_match = re.match(r"^(\s*)", line)
                    indent = indent_match.group(1) if indent_match else ""
                    result.append(f"{indent}template <typename CharT>")

        result.append(line)
        i += 1

    return "\n".join(result)


def process_file(file_path):
    """Process a single file."""
    try:
        with open(file_path, "r", encoding="utf-8") as f:
            content = f.read()

        original_content = content

        # Apply all transformations
        content = replace_stringviewifiable(content)
        content = remove_view_usages(content)
        content = add_missing_template_params(content)

        # Only write back if changes were made
        if content != original_content:
            with open(file_path, "w", encoding="utf-8") as f:
                f.write(content)
            return True

        return False
    except Exception as e:
        print(f"Error processing {file_path}: {e}")
        return False


def main():
    """Main function."""
    print("Starting replacement of istl::StringViewifiable and istl::view usages...")

    files = find_files()
    processed_files = []

    for file_path in files:
        if process_file(file_path):
            processed_files.append(file_path)
            print(f"Processed: {file_path}")

    print(f"\nReplacement complete!")
    print(f"Processed {len(processed_files)} files.")

    if processed_files:
        print("\nChanges made:")
        print(
            "- Replaced istl::StringViewifiable auto&& with stl::basic_string_view<CharT> const"
        )
        print("- Removed istl::view() and istl::view_of<>() calls")
        print("- Added template <typename CharT> where needed")
        print(
            "\nNote: You may need to manually fix some cases. Please compile to check for errors."
        )


if __name__ == "__main__":
    main()
