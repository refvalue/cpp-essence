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

#include <exception>
#include <filesystem>
#include <fstream>

#include <essence/abi/json.hpp>
#include <essence/char8_t_remediation.hpp>
#include <essence/cli/arg_parser.hpp>
#include <essence/error_extensions.hpp>
#include <essence/globalization/compiler.hpp>
#include <essence/string.hpp>

#include <spdlog/spdlog.h>

namespace essence::tooling {
    namespace {
        void compile_json_file(const std::filesystem::path& input_path, const std::filesystem::path& output_path) {
            using namespace essence::globalization;

            spdlog::info(U8("Compiling {} to {}..."), from_u8string(input_path.u8string()),
                from_u8string(output_path.u8string()));

            thread_local const auto compiler = make_default_compiler();
            const auto json                  = [&] {
                try {
                    std::ifstream stream;

                    stream.exceptions(std::ios_base::badbit);
                    stream.open(input_path);

                    return abi::json::parse(stream, nullptr, true);
                } catch (const std::exception& ex) {
                    throw source_code_aware_runtime_error{U8("Path"), from_u8string(input_path.u8string()),
                        U8("Message"), U8("Failed to open the JSON file."), U8("Internal"), ex.what()};
                }
            }();

            compiler.to_file(json, from_u8string(output_path.u8string()));
        }

        void compile_all_jsons(
            const std::filesystem::path& input_directory, const std::filesystem::path& output_directory) {
            for (auto&& item : std::filesystem::recursive_directory_iterator{
                     input_directory, std::filesystem::directory_options::skip_permission_denied}) {
                if (const auto extenstion = from_u8string(item.path().extension().u8string());
                    icase_string_comparer{}(extenstion, U8(".json"))) {
                    auto output_path               = output_directory / item.path().lexically_relative(input_directory);
                    const auto output_subdirectory = output_path.parent_path();

                    output_path.replace_extension(U8(".lang"));

                    if (std::error_code code; !std::filesystem::exists(output_subdirectory, code)) {
                        std::filesystem::create_directories(output_subdirectory);
                    }

                    compile_json_file(item.path(), output_path);
                }
            }
        }
    } // namespace
} // namespace essence::tooling

int main(int argc, char* argv[]) try {
    using namespace essence;
    using namespace essence::cli;
    using namespace essence::tooling;

    if (const arg_parser parser; parser.parse(argc, argv), parser) {
        const auto unmatched = parser.unmatched_args();

        if (unmatched.size() == 2) {
            spdlog::error(U8("Missing an argument: the output directory."));
            std::exit(-1);
        }

        if (unmatched.size() == 1) {
            spdlog::error(U8("Missing an argument: the input directory."));
            std::exit(-2);
        }

        compile_all_jsons(to_u8string(unmatched[1]), to_u8string(unmatched[2]));
    }
} catch (const std::exception& ex) {
    spdlog::error(ex.what());

    return -99;
}
