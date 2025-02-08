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

export module essence.basic:char8_t_remediation;
import std;

#if __cpp_char8_t >= 202207L
export namespace essence {
    template <std::size_t N, bool CharLiteral>
    struct char_string_literal {
        static constexpr auto size         = N;
        static constexpr auto char_literal = CharLiteral;

        char data[N];
    };

    char_string_literal(char8_t) -> char_string_literal<1, true>;

    template <std::size_t N>
    char_string_literal(const char8_t (&)[N]) -> char_string_literal<N, false>;

    template <char_string_literal Literal>
    constexpr decltype(auto) as_char_v =
        []() -> std::conditional_t<Literal.char_literal, char, const char (&)[Literal.size]> {
        if constexpr (Literal.char_literal) {
            return Literal.data[0];
        } else {
            return Literal.data;
        }
    }
    ();

    inline std::string from_u8string(std::u8string_view str) {
        return {str.begin(), str.end()};
    }

    inline std::u8string to_u8string(std::string_view str) {
        return {str.begin(), str.end()};
    }
} // namespace essence
#else
#error "C++23 standard P2513R4: `DR20 char8_t Compatibility and Portability Fix` must be supported."
#endif
