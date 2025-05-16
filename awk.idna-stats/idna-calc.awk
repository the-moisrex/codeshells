BEGIN {
  FS = ";"
  max_codepoint = 0x3347F   # 210943 decimal
  # max_codepoint = 0x10FFFF

  mapped = 0
  disallowed = 0
  valid = 0
  ignored = 0
  other = 0
  covered = 0
}

{
  # Remove inline comments starting with #
  sub(/#.*$/, "", $0)

  # Trim leading and trailing whitespace from entire line
  gsub(/^[ \t]+|[ \t]+$/, "", $0)

  # Skip empty lines after comment removal
  if ($0 == "") next


  # Split line by semicolon (FS=";")
  # Trim spaces from each field
  for (i = 1; i <= NF; i++) {
    gsub(/^[ \t]+|[ \t]+$/, "", $i)
  }

  # Parse code point or range in $1
  if ($1 ~ /\.\./) {
    split($1, range, /\.\./)
    start = strtonum("0x" range[1])
    end = strtonum("0x" range[2])
  } else {
    start = strtonum("0x" $1)
    end = start
  }


  # Skip ranges that start beyond max_codepoint
  if (start > max_codepoint) {
    next
  }

  # Truncate end if it exceeds max_codepoint
  if (end > max_codepoint) {
    end = max_codepoint
  }

  count = end - start + 1

  status = $2

  if (status == "mapped") {
    mapped += count
  } else if (status == "disallowed") {
    disallowed += count
  } else if (status == "valid") {
    valid += count
  } else if (status == "ignored") {
    ignored += count
  } else {
    other += count
  }

  covered += count
}

END {
  total = max_codepoint + 1
  uncovered = total - covered

  # Assume uncovered code points are valid by default (I think they're disallowed)
  # valid += uncovered

  not_valid = total - valid

  printf "Total code points (up to U+%x): %d\n", max_codepoint , total
  printf "Mapped code points: %d (%.2f%%)\n", mapped, (mapped / total) * 100
  printf "Valid code points: %d (%.2f%%)\n", valid, (valid / total) * 100
  printf "Disallowed code points: %d (%.2f%%)\n", disallowed, (disallowed / total) * 100
  printf "Ignored code points: %d (%.2f%%)\n", ignored, (ignored / total) * 100
  printf "Other code points: %d (%.2f%%)\n", other, (other / total) * 100
  printf "Covered code points in file: %d\n", covered
  printf "Not valid code points: %d (%.2f%%)\n", not_valid, (not_valid / total) * 100
  printf "Uncovered code points: %d\n", uncovered
}

