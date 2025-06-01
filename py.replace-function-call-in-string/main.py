#!/usr/bin/env python3

def call_toggle(arg1, arg2):
    # Example implementation of call_toggle
    return f"Result {arg1}, {arg2}"

def replace_function(func_name, action, text):
    result = []
    i = 0
    n = len(text)

    while i < n:
        # Look for the $toggle( pattern
        if text.startswith(f"{func_name}(", i):
            start = i + len(f"{func_name}(")
            depth = 1
            j = start
            # Find the matching closing parenthesis
            while j < n and depth > 0:
                if text[j] == '(':
                    depth += 1
                elif text[j] == ')':
                    depth -= 1
                j += 1

            if depth != 0:
                # No matching closing parenthesis found, treat as normal text
                result.append(text[i])
                i += 1
                continue

            # Extract the content inside parentheses
            content = text[start:j-1]

            # Now parse the two arguments separated by a comma at top level
            arg1, arg2 = None, None
            arg_start = 0
            comma_index = -1
            depth_arg = 0
            for k, ch in enumerate(content):
                if ch == '(':
                    depth_arg += 1
                elif ch == ')':
                    depth_arg -= 1
                elif ch == ',' and depth_arg == 0:
                    comma_index = k
                    break

            if comma_index == -1:
                # No comma found, treat as normal text
                result.append(text[i])
                i += 1
                continue

            arg1 = content[:comma_index].strip()
            arg2 = content[comma_index+1:].strip()

            # Call the function and append the result
            replacement = action(arg1, arg2)
            result.append(replacement)

            # Move index past the entire $toggle(...) pattern
            i = j
        else:
            # Normal character, just append
            result.append(text[i])
            i += 1

    return ''.join(result)

# Example usage
input_text = "Example: $toggle(one, two) and nested $toggle(foo(bar), baz(qux))."
output_text = replace_function("$toggle", call_toggle, input_text)
print(output_text)

