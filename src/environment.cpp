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

#include "environment.hpp"

#include "char8_t_remediation.hpp"
#include "format_remediation.hpp"

#ifdef _WIN32
#include "encoding.hpp"

#include <algorithm>
#include <memory>
#endif

#include <cstddef>
#include <cstdlib>
#include <filesystem>
#include <mutex>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef NOGDI
#define NOGDI
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <Windows.h>
#include <shellapi.h>
#endif

#include <spdlog/spdlog.h>

namespace essence {
    namespace {
        std::mutex deleting_mutex;
        std::vector<std::filesystem::path> directories_to_delete;

#ifdef _WIN32
        const auto command_line_args{[] {
            abi::vector<abi::string> result;

            if (std::int32_t size{}; std::unique_ptr<wchar_t* [], decltype(&LocalFree)> args {
                    CommandLineToArgvW(GetCommandLineW(), &size), &LocalFree
                }) {
                result.reserve(size);

                for (std::size_t i = 0; i < static_cast<std::size_t>(size); i++) {
                    result.emplace_back(to_utf8_string(args[i]));
                }
            }

            return result;
        }()};

        void create_lazy_rmdir_process() {
            static constexpr std::string_view rmdir_pattern{U8(R"(rmdir /s /q "{}" & )")};
            static constexpr std::string_view prefix{U8(R"(cmd.exe /c timeout /t 3 /nobreak > nul & )")};

            std::string command_line{prefix};
            {
                std::scoped_lock lock{deleting_mutex};

                for (auto&& item : directories_to_delete) {
                    command_line.append(format(rmdir_pattern, from_u8string(item.u8string())));
                }
            }

            command_line.erase(command_line.size() - 3);

            PROCESS_INFORMATION pi{};
            STARTUPINFOW si{
                .cb = sizeof(STARTUPINFOW),
            };

            auto native_command_line = to_native_string(command_line);

            if (CreateProcessW(nullptr, native_command_line.data(), nullptr, nullptr, FALSE, CREATE_NO_WINDOW, nullptr,
                    nullptr, &si, &pi)) {
                CloseHandle(pi.hThread);
                CloseHandle(pi.hProcess);
            }
        }
#elif defined(EMSCRIPTEN)
        abi::vector<abi::string> command_line_args;
#else
        char** raw_argv;
        std::size_t raw_argc;
        abi::vector<abi::string> command_line_args;

        // http://www.dbp-consulting.com/tutorials/debugging/linuxProgramStartup.html
        // What is this __init_array? I thought you'd never ask. You can have code run at this stage as well.
        // Since this is just after returning from running _init which ran our constructors,
        // that means anything in this array will run after constructors are done.
        // You can tell the compiler you want a function to run at this phase. The function will receive the same
        // arguments as main.
        __attribute__((used, section(".init_array")))
        [[maybe_unused]] void (*get_argc_argv)(int, char**, char**) = [](int argc, char** argv, [[maybe_unused]] char** envp) {
            raw_argc = static_cast<std::size_t>(argc);
            raw_argv = argv;
        };

        ES_KEEP_ALIVE [[maybe_unused]] struct init {
            init() {
                command_line_args.reserve(raw_argc);

                for (std::size_t i = 0; i < raw_argc; i++) {
                    command_line_args.emplace_back(raw_argv[i]);
                }
            }
        } force_init;
#endif
    } // namespace

    abi::vector<abi::string> get_command_line_args() {
        return command_line_args;
    }

    void delete_directory_at_exit(std::string_view path) {
        static std::once_flag flag;

        std::call_once(flag, [] {
            static_cast<void>(std::atexit([] {
                spdlog::info(U8("Starting to delete scheduled directories..."));
#ifdef _WIN32
                create_lazy_rmdir_process();
#else
                std::error_code code;
                std::scoped_lock lock{deleting_mutex};

                for (auto&& item : directories_to_delete) {
                    std::filesystem::remove_all(item, code);
                }
#endif
            }));
        });

        std::scoped_lock lock{deleting_mutex};

        directories_to_delete.emplace_back(to_u8string(path));
    }
} // namespace essence
