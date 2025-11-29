#!/usr/bin/env python3
# reorder_idna_options.py — Fixed & working version

import re
import sys
from pathlib import Path

# Exact order from your cleaned-up struct
CORRECT_ORDER = [
    "CheckHyphens",
    "UseSTD3ASCIIRules",
    "VerifyDnsLength",
    "CheckACE",
    "CheckBidi",
    "CheckJoiners",
    "IgnoreInvalidPunycode",
    "CheckNFC",
    "CheckDotInclusions",
    "CheckMappingRequired",
    "CheckCombiningMarkAtLabelStart",
    "CheckDecodeAndValidateLabels",
]

def reorder_options_block(block: str) -> str:
    lines = [line.strip() for line in block.splitlines() if line.strip()]
    if not lines:
        return block

    assignments = {}
    for line in lines:
        # Match: .Field = value,   (with optional spaces and trailing comma)
        m = re.match(r"\.(\w+)\s*=\s*(.+?)(?:,|$)", line)
        if m:
            field, value = m.groups()
            assignments[field] = value.strip()

    reordered = []
    for field in CORRECT_ORDER:
        if field in assignments:
            value = assignments.pop(field)
            reordered.append(f"      .{field:<30} = {value},")

    # Append any unknown/extra fields (should not happen, but safe)
    for field in sorted(assignments.keys()):
        value = assignments[field]
        reordered.append(f"      .{field:<30} = {value},")

    return "\n".join(reordered)


def process_content(content: str) -> str:
    # Match: static constexpr idna_options name { ... };
    def replacer(match):
        before = match.group(1)
        body = match.group(2)
        after = match.group(3)
        reordered_body = reorder_options_block(body)
        return f"{before}\n{reordered_body}\n    {after}"

    # Main regex: captures { ... } across multiple lines
    pattern = r"(static\s+constexpr\s+idna_options\s*(?:\w+)?\s*\{)([^}]*?)(\}\s*;)"
    return re.sub(pattern, replacer, content, flags=re.DOTALL)


def main():
    if len(sys.argv) == 1 or (len(sys.argv) == 2 and sys.argv[1] == "-"):
        # Read from stdin, write to stdout (perfect for piping)
        content = sys.stdin.read()
        print(process_content(content), end="")
        return

    for arg in sys.argv[1:]:
        path = Path(arg)
        if not path.is_file():
            print(f"Error: File not found: {path}", file=sys.stderr)
            continue

        content = path.read_text(encoding="utf-8")
        new_content = process_content(content)

        if new_content != content:
            path.write_text(new_content, encoding="utf-8")
            print(f"Reordered: {path}")
        else:
            print(f"No changes: {path}")


if __name__ == "__main__":
    main()
