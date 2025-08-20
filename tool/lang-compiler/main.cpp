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

import std;
import essence.basic;
import essence.cli;
import essence.i18n;
import essence.serialization;

namespace essence::tooling {
    namespace {
        void compile_json_file(const std::filesystem::path& input_path, const std::filesystem::path& output_path) {
            using namespace essence::i18n;

            spdlog::info("Compiling {} to {}...", input_path.generic_u8string() | std::ranges::to<std::string>(),
                output_path.generic_u8string() | std::ranges::to<std::string>());

            thread_local const auto compiler = make_default_compiler();
            const auto json                  = [&] {
                try {
                    std::ifstream stream;

                    stream.exceptions(std::ios_base::badbit);
                    stream.open(input_path);

                    return abi::json::parse(stream, nullptr, true);
                } catch (const std::exception& ex) {
                    throw formatted_runtime_error{"Path",
                        input_path.generic_u8string() | std::ranges::to<std::string>(), "Message",
                        "Failed to open the JSON file.", "Internal", ex.what()};
                }
            }();

            compiler.to_file(json, output_path.generic_u8string() | std::ranges::to<std::string>());
        }

        void compile_all_jsons(
            const std::filesystem::path& input_directory, const std::filesystem::path& output_directory) {
            for (auto&& item : std::filesystem::recursive_directory_iterator{
                     input_directory, std::filesystem::directory_options::skip_permission_denied}) {
                if (const auto extension = item.path().extension().generic_u8string() | std::ranges::to<std::string>();
                    icase_string_comparer{}(extension, ".json")) {
                    auto output_path               = output_directory / item.path().lexically_relative(input_directory);
                    const auto output_subdirectory = output_path.parent_path();

                    output_path.replace_extension(".lang");

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
            spdlog::error("Missing an argument: the output directory.");
            std::exit(-1);
        }

        if (unmatched.size() == 1) {
            spdlog::error("Missing an argument: the input directory.");
            std::exit(-2);
        }

        compile_all_jsons(
            unmatched[1] | std::ranges::to<std::u8string>(), unmatched[2] | std::ranges::to<std::u8string>());
    }
} catch (const std::exception& ex) {
    spdlog::error(ex.what());

    return -99;
}
