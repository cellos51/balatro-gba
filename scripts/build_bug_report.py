#!/usr/bin/env python3

import sys
import re

def grep_binary_offsets(path, pattern):
    with open(path, "rb") as f:
        data = f.read()

    results = []
    for match in re.finditer(rb"[ -~]{4,}", data):
        s = match.group()
        if re.search(pattern.encode(), s):
            results.append((match.start(), s.decode("utf-8", errors="ignore")))
    return results

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python script.py <file>")
        sys.exit(1)

    file_path = sys.argv[1]
    pattern   = "GBALATRO_VERSION"

    for offset, text in grep_binary_offsets(file_path, pattern):
        version = text.split(":")[1]
        print(f"git hash: {version}")
