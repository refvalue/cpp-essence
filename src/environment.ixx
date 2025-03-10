/*
 * Copyright (c) 2024 The RefValue Project
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

module;

#include <essence/compat.hpp>

export module essence.basic:environment;
import :abi.string;
import :abi.vector;
import std;

export namespace essence {
    /**
     * Gets the executable path.
     * @return The executable path.
     */
    ES_API(CPPESSENCE) abi::string get_executable_path();

    /**
     * Gets the module (shared object or dynamic library) path by the address of a symbol that belongs to that module.
     * @param address The address of a symbol.
     * @return The path of the module or an empty string if failed.
     */
    ES_API(CPPESSENCE) abi::string get_module_path(const void* address);

    /**
     * Gets the command-line arguments.
     * @return The command-line arguments.
     */
    ES_API(CPPESSENCE) abi::vector<abi::string> get_command_line_args();

    /**
     * Schedules deleting a directory when the application exits. The operation cannot be cancelled.
     * @param path The path.
     */
    ES_API(CPPESSENCE) void delete_directory_at_exit(std::string_view path);
} // namespace essence
