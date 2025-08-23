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

import re
import sys


def patch_exported_modular_target(script_file: str):
    index = None
    prefix = None

    with open(script_file, 'r') as file:
        lines = file.readlines()

    for i, line in enumerate(lines):
        if match := re.match(r'^(.*IMPORTED_CXX_MODULES_LINK_LIBRARIES).*$', line):
            index = i
            prefix = match.group(1)
            continue

        if re.match(r'^.*FILES.*$', line):
            lines[i] = re.sub(r'"\$\{_IMPORT_PREFIX}/[^\"]*\.(cpp|cxx)"', '', lines[i])
            print('[INFO] Patched: ', lines[i])
            continue

        match = re.match(r'^.*INTERFACE_LINK_LIBRARIES(.*[\r\n]*)', line)

        if match and index is not None:
            lines[index] = prefix + match.group(1)
            print('[INFO] Patched: ', lines[index])
            index = None

    with open(script_file, 'w') as file:
        file.writelines(lines)


if __name__ == "__main__":
    patch_exported_modular_target(sys.argv[1])
