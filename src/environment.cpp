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

#include <essence/char8_t_remediation.hpp>
#include <essence/compat.hpp>

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
#else
#include <dlfcn.h>
#include <unistd.h>
#endif

module essence.basic;
import std;

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
            static constexpr std::string_view rmdir_pattern{R"(rmdir /s /q "{}" & )"};
            static constexpr std::string_view prefix{R"(cmd.exe /c timeout /t 3 /nobreak > nul & )"};

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

        abi::string get_module_path(std::uint32_t flags, const void* name_or_address) {
            if (HMODULE module; GetModuleHandleExW(flags, static_cast<const wchar_t*>(name_or_address), &module)) {
                std::wstring path(MAX_PATH, L'\0');

                path.resize(GetModuleFileNameW(module, path.data(), static_cast<DWORD>(path.size())));
                path.shrink_to_fit();

                return to_utf8_string(path);
            }

            return {};
        }
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
        [[maybe_unused]] __attribute__((used, section(".init_array"))) void (*get_argc_argv)(
            int, char**, char**) = [](int argc, char** argv, [[maybe_unused]] char** envp) {
            raw_argc = static_cast<std::size_t>(argc);
            raw_argv = argv;
        };

        [[maybe_unused]] ES_KEEP_ALIVE struct init {
            init() {
                command_line_args.reserve(raw_argc);

                for (std::size_t i = 0; i < raw_argc; i++) {
                    command_line_args.emplace_back(raw_argv[i]);
                }
            }
        } force_init;

        std::filesystem::path get_module_full_path(std::string_view filename) {
            std::ifstream stream{format("/proc/{}/maps", getpid()), std::ios_base::in};
            std::string line;

            while (std::getline(stream, line)) {
                const auto components = line | std::views::split(' ') | std::views::transform([](const auto& inner) {
                    return std::filesystem::path{inner.begin(), inner.end()};
                }) | std::views::filter([](const auto& inner) { return inner.is_absolute(); })
                                      | std::ranges::to<std::vector>();

                if (!components.empty() && components.back().filename() == filename) {
                    return components.back();
                }
            }

            return {};
        }
#endif
    } // namespace

    abi::string get_executable_path() {
#ifdef _WIN32
        return get_module_path(GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, nullptr);
#else
        return {};
#endif
    }

    abi::string get_module_path(const void* address) {
#ifdef _WIN32
        return get_module_path(
            GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, address);
#else
        if (Dl_info info{}; dladdr(address, &info)) {
            if (std::filesystem::path path{info.dli_fname}; path.is_relative()) {
                return abi::to_abi_string(get_module_full_path(path.string()).general_string());
            }

            return info.dli_fname;
        }
#endif

        return {};
    }

    abi::vector<abi::string> get_command_line_args() {
        return command_line_args;
    }

    void delete_directory_at_exit(std::string_view path) {
        static std::once_flag flag;

        std::call_once(flag, [] {
            static_cast<void>(std::atexit([] {
                spdlog::info("Starting to delete scheduled directories...");
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
