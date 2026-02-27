#!/bin/bash

# Script to replace istl::StringViewifiable auto&& with stl::basic_string_view<CharT>
# and remove istl::view usages across the webpp library

set -e

echo "Starting replacement of istl::StringViewifiable and istl::view usages..."

# Find all relevant files
find_files() {
    find /home/moisrex/Projects/webpp -name "*.hpp" -o -name "*.cpp" | grep -v test | grep -v benchmark
}

# Replace istl::StringViewifiable auto&& with template + stl::basic_string_view<CharT>
replace_stringviewifiable() {
    echo "Replacing istl::StringViewifiable auto&& usages..."
    
    # Pattern 1: Function parameters with istl::StringViewifiable auto&&
    # Replace: template <typename T> func(istl::StringViewifiable auto&& param)
    # With:    template <typename CharT> func(stl::basic_string_view<CharT> const param)
    
    find_files | while read -r file; do
        if grep -q "istl::StringViewifiable.*auto&&" "$file"; then
            echo "Processing: $file"
            
            # Create a temporary file
            temp_file=$(mktemp)
            
            # Process the file with sed
            sed -E '
                # Replace function parameter declarations
                s/template[[:space:]]*<[[:space:]]*typename[[:space:]]+([A-Za-z_][A-Za-z0-9_]*)[[:space:]]*>[[:space:]]*\n([[:space:]]*)\([[:space:]]*istl::StringViewifiable[[:space:]]+auto&&[[:space:]]+([A-Za-z_][A-Za-z0-9_]*)/template <typename CharT>\n\2(stl::basic_string_view<CharT> const \3/g
                
                # Replace single-line function parameters
                s/template[[:space:]]*<[[:space:]]*typename[[:space:]]+([A-Za-z_][A-Za-z0-9_]*)[[:space:]]*>[[:space:]]*\([[:space:]]*istl::StringViewifiable[[:space:]]+auto&&[[:space:]]+([A-Za-z_][A-Za-z0-9_]*)/template <typename CharT> (stl::basic_string_view<CharT> const \2/g
                
                # Replace istl::StringViewifiable in requires clauses or other contexts
                s/istl::StringViewifiable[[:space:]]+auto&&/stl::basic_string_view<CharT> const/g
                
                # Replace istl::StringViewifiable<SomeType> with stl::basic_string_view<CharT>
                s/istl::StringViewifiable[[:space:]]*<[[:space:]]*[^>]+[[:space:]]*>/stl::basic_string_view<CharT>/g
            ' "$file" > "$temp_file"
            
            # Replace the original file
            mv "$temp_file" "$file"
        fi
    done
}

# Remove istl::view usages
remove_view_usages() {
    echo "Removing istl::view usages..."
    
    find_files | while read -r file; do
        if grep -q "istl::view" "$file"; then
            echo "Processing istl::view in: $file"
            
            # Create a temporary file
            temp_file=$(mktemp)
            
            # Process the file with sed
            sed -E '
                # Replace istl::view(param) with param directly
                s/istl::view[[:space:]]*\(([^[)]+)\)/\1/g
                
                # Replace istl::view_of<Type>(param) with param directly
                s/istl::view_of[[:space:]]*<[[:space:]]*[^>]+[[:space:]]*>[[:space:]]*\(([^[)]+)\)/\1/g
            ' "$file" > "$temp_file"
            
            # Replace the original file
            mv "$temp_file" "$file"
        fi
    done
}

# Fix template parameters that might need CharT
fix_template_parameters() {
    echo "Fixing template parameters..."
    
    find_files | while read -r file; do
        if grep -q "stl::basic_string_view<CharT>" "$file"; then
            echo "Checking template parameters in: $file"
            
            # Create a temporary file
            temp_file=$(mktemp)
            
            # Add CharT template parameter where needed
            python3 -c "
import re
import sys

def fix_templates(content):
    lines = content.split('\n')
    result = []
    i = 0
    
    while i < len(lines):
        line = lines[i]
        
        # Check if this line contains stl::basic_string_view<CharT> but no CharT template
        if 'stl::basic_string_view<CharT>' in line and 'template' not in line:
            # Look backwards for a template declaration
            template_found = False
            j = i - 1
            while j >= 0 and j >= i - 10:  # Look back up to 10 lines
                if 'template' in lines[j] and '<' in lines[j]:
                    template_found = True
                    break
                if lines[j].strip() == '' or lines[j].strip().startswith('{') or lines[j].strip().startswith('}'):
                    break
                j -= 1
            
            # If no template found, this might be a function that needs one
            if not template_found and ('(' in line and ')' in line):
                # This looks like a function declaration, check if we need to add template
                if 'constexpr' in line or '[[nodiscard]]' in line:
                    # Insert template before this line
                    indent = re.match(r'^\s*', line).group(0)
                    result.append(indent + 'template <typename CharT>')
        
        result.append(line)
        i += 1
    
    return '\n'.join(result)

with open('$file', 'r') as f:
    content = f.read()

fixed_content = fix_templates(content)

with open('$temp_file', 'w') as f:
    f.write(fixed_content)
"
            
            # Replace the original file if the temp file was created
            if [ -f "$temp_file" ]; then
                mv "$temp_file" "$file"
            fi
        fi
    done
}

# Main execution
echo "Step 1: Replacing istl::StringViewifiable usages..."
replace_stringviewifiable

echo "Step 2: Removing istl::view usages..."
remove_view_usages

echo "Step 3: Fixing template parameters..."
fix_template_parameters

echo "Replacement complete!"
echo ""
echo "Summary of changes:"
echo "- Replaced istl::StringViewifiable auto&& with stl::basic_string_view<CharT> const"
echo "- Removed istl::view() and istl::view_of<>() calls"
echo "- Added template <typename CharT> where needed"
echo ""
echo "Note: You may need to manually fix some cases where the automatic replacement"
echo "was not perfect. Please review the changes and compile to check for errors."