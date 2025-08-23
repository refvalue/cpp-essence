#  Copyright (c) 2024 The RefValue Project
#
#  Permission is hereby granted, free of charge, to any person obtaining a copy
#  of this software and associated documentation files (the "Software"), to deal
#  in the Software without restriction, including without limitation the rights
#  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
#  copies of the Software, and to permit persons to whom the Software is
#  furnished to do so, subject to the following conditions:
#
#  The above copyright notice and this permission notice shall be included in
#  all copies or substantial portions of the Software.
#
#  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
#  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
#  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
#  THE SOFTWARE.

import sys
import re
from pathlib import Path
from typing import Sequence, Tuple


def replace_in_file(filepath: Path, rules: Sequence[Tuple[str, str]]):
    text = filepath.read_text(encoding="utf-8")
    total_count = 0

    for pattern, replacement in rules:
        patched, count = re.subn(pattern, replacement, text, flags=re.MULTILINE)
        if count > 0:
            print(f"[INFO] Replaced {count} occurrence(s) of `{pattern}` -> `{replacement}`.")
        text = patched
        total_count += count

    if total_count > 0:
        filepath.write_text(text, encoding="utf-8")
        print(f"[INFO] Patched {total_count} occurrence(s) in `{filepath}`.")
    else:
        print(f"[INFO] No matches found in `{filepath}`.")


if __name__ == "__main__":
    if len(sys.argv) < 4 or (len(sys.argv) - 2) % 2 != 0:
        print("Usage: regex_patch.py <file> <pattern1> <replacement1> [<pattern2> <replacement2> ...]")
        sys.exit(1)

    filepath = Path(sys.argv[1])
    rules = list(zip(sys.argv[2::2], sys.argv[3::2]))
    replace_in_file(filepath, rules)
