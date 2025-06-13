#pragma once

#include <codecvt>
#include <cwctype>
#include <locale>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <vector>
#include <filesystem>
#include <cassert>
#include <concepts>
#include <ranges>

#include <String/StringType.hpp>

namespace RC
{
    // =======================================================================
    // MODERN C++20 STRING CONCEPTS
    // =======================================================================
    template <typename T>
    concept Character = 
        std::is_same_v<std::remove_cv_t<T>, char> ||
        std::is_same_v<std::remove_cv_t<T>, wchar_t> ||
        std::is_same_v<std::remove_cv_t<T>, char8_t> ||
        std::is_same_v<std::remove_cv_t<T>, char16_t> ||
        std::is_same_v<std::remove_cv_t<T>, char32_t>;

    template <typename T>
    concept StringLikeRange = std::ranges::contiguous_range<T> &&
                              Character<std::ranges::range_value_t<T>>;

    template <typename T>
    concept StringLikePointer = std::is_pointer_v<std::decay_t<T>> &&
                                Character<std::remove_pointer_t<std::decay_t<T>>>;

    template <typename T>
    concept StringLike = StringLikeRange<T> ||
                         StringLikePointer<T> ||
                         std::is_same_v<std::decay_t<T>, std::filesystem::path>;

    template <typename T>
    struct string_char_helper {
        using type = std::remove_cv_t<std::ranges::range_value_t<std::decay_t<T>>>;
    };

    template <StringLikePointer T>
    struct string_char_helper<T> {
        using type = std::remove_cv_t<std::remove_pointer_t<std::decay_t<T>>>;
    };

    template <>
    struct string_char_helper<std::filesystem::path> {
        using type = std::filesystem::path::value_type;
    };

    template <StringLike T>
    using string_char_t = typename string_char_helper<std::decay_t<T>>::type;

    template <typename T>
    concept Utf8StringLike = StringLike<T> && 
                             std::is_same_v<string_char_t<T>, char>;

    /* explode_by_occurrence -> START

    FUNCTION: explode_by_occurrence
    Find or explode a string by a delimiter

    The defaults for these functions are set in stone
    If you change them then you'll have to review every single usage in the entire solution
    */

    enum class ExplodeType
    {
        FromStart,
        FromEnd
    };

    template <typename CharT>
    auto inline explode_by_occurrence(const std::basic_string<CharT>& in_str_wide, const CharT delimiter, ExplodeType start_or_end) -> std::basic_string<CharT>
    {
        size_t occurrence = (start_or_end == ExplodeType::FromStart ? in_str_wide.find_first_of(delimiter) : in_str_wide.find_last_of(delimiter));

        std::basic_string<CharT> return_value;
        if (occurrence != std::basic_string<CharT>::npos)
        {
            return_value = start_or_end == ExplodeType::FromEnd ? in_str_wide.substr(occurrence + 1, std::basic_string<CharT>::npos)
                                                                : in_str_wide.substr(0, occurrence);
        }
        else
        {
            return_value = in_str_wide;
        }

        return return_value;
    }

    template <typename CharT>
    auto inline explode_by_occurrence(const std::basic_string<CharT>& in_str, const char delimiter, const int32_t occurrence) -> std::basic_string<CharT>
    {
        size_t found_occurrence{};
        for (int64_t i = 0; i < std::count(in_str.begin(), in_str.end(), delimiter); i++)
        {
            found_occurrence = in_str.find(delimiter, found_occurrence + 1);
            if (i + 1 == occurrence)
            {
                return in_str.substr(0, found_occurrence);
            }
        }

        // No occurrence was found, returning empty string for now
        return {};
    }

    template <typename CharT>
    auto inline explode_by_occurrence(const std::basic_string<CharT>& in_str, const char delimiter) -> std::vector<std::basic_string<CharT>>
    {
        std::vector<std::basic_string<CharT>> result;

        size_t counter{};
        size_t start_offset{};

        for (const CharT* current_char = in_str.c_str(); *current_char; ++current_char)
        {
            if (*current_char == delimiter || counter == in_str.length() - 1)
            {
                std::basic_string<CharT> sub_str = in_str.substr(start_offset, counter - start_offset + (counter == in_str.length() - 1 ? 1 : 0));
                if (start_offset > 0)
                {
                    sub_str.erase(0, 1);
                }
                result.emplace_back(sub_str);

                start_offset = counter;
            }

            ++counter;
        }

        return result;
    }

    template <typename CharT>
    auto inline explode_by_occurrence(const std::basic_string<CharT>& in_str, const CharT delimiter, const int32_t occurrence) -> std::basic_string<CharT>
    {
        size_t found_occurrence{};
        for (int64_t i = 0; i < std::count(in_str.begin(), in_str.end(), delimiter); i++)
        {
            found_occurrence = in_str.find(delimiter, found_occurrence + 1);
            if (i + 1 == occurrence)
            {
                return in_str.substr(0, found_occurrence);
            }
        }

        // No occurrence was found, returning empty string for now
        return {};
    }

    /**
     * Breaks an input string into a vector of substrings based on a given delimiter.
     * <br>It treats sections that start with a delimiter character and enclosed in double quotes (`"`) as a single substring, ignoring any delimiters inside
     * the quotes. <br>It supports an escape character (default `\`) to capture double quotes as part of a string.
     *
     * @tparam CharT The character type.
     * @param in_str  The input string to be split.
     * @param delimiter (optional) The character used to split the string. Default: ` `.
     * @param escape_character (optional) The character used for escaping quotes. Default: `\`.
     * @return A vector of substrings, split by the delimiter, with quoted substrings preserved.
     */
    template <typename CharT>
    auto inline explode_by_occurrence_with_quotes(const std::basic_string<CharT>& in_str,
                                                  const CharT delimiter = STR(' '),
                                                  const CharT escape_character = STR('\\')) -> std::vector<std::basic_string<CharT>>
    {
        constexpr auto quotation_symbol = STR('"');
        assert(delimiter != quotation_symbol && "Double quote (\") can't be used as delimiter");
        assert(delimiter != escape_character && "Delimiter can't be the same as the escape character");
        assert(escape_character != quotation_symbol && "Double quote (\") can't be used as escape character");

        std::vector<std::basic_string<CharT>> result;
        std::basic_string<CharT> current;
        auto in_quotes = false;

        auto add_current_to_vector = [&result, &current]() {
            if (!current.empty())
            {
                result.push_back(current);
                current.clear();
            }
        };

        auto is_valid_quote_symbol = [&in_quotes, &in_str, &delimiter, &escape_character](int position) {
            if (position >= 0 && position < in_str.size() && in_str[position] == quotation_symbol && (position == 0 || in_str[position - 1] != escape_character))
            {
                return in_quotes ? position + 1 == in_str.size() || in_str[position + 1] == delimiter : position == 0 || in_str[position - 1] == delimiter;
            }
            return false;
        };

        for (size_t i = 0; i < in_str.size(); i++)
        {
            if (is_valid_quote_symbol(i))
            {
                in_quotes = !in_quotes;
                continue;
            }
            const auto current_char = in_str[i];
            if ((!in_quotes && current_char == delimiter))
            {
                add_current_to_vector();
                continue;
            }
            if (current_char != escape_character || in_str[i + 1] != quotation_symbol)
            {
                current.push_back(current_char);
            }
        }
        add_current_to_vector();

        return result;
    }
    /* explode_by_occurrence -> END */

    auto inline to_wstring(const std::string& input) -> std::wstring
    {
#pragma warning(disable : 4996)
        static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter{};
        return converter.from_bytes(input);
#pragma warning(default : 4996)
    }

    auto inline to_wstring(const char* pInput)
    {
        auto temp_input = std::string{pInput};
        return to_wstring(temp_input);
    }

    auto inline to_wstring(std::string_view input) -> std::wstring
    {
#ifdef PLATFORM_WINDOWS
#pragma warning(disable : 4996)
        static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter{};
        return converter.from_bytes(input.data(), input.data() + input.length());
#pragma warning(default : 4996)
#else
#if __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif
        static std::wstring_convert<std::codecvt_utf8<wchar_t>> converter{};
        return converter.from_bytes(input.data(), input.data() + input.length());
#endif
#if __clang__
#pragma clang diagnostic pop
#endif
    }

    auto inline to_wstring(std::wstring_view input) -> std::wstring
    {
        return std::wstring{input};
    }

    auto inline to_wstring(const std::wstring& input) -> std::wstring
    {
        return std::wstring{input};
    }

    auto inline to_wstring(const std::u16string& input) -> std::wstring
    {
#ifdef PLATFORM_WINDOWS
        return {input.begin(), input.end()};
#else
        throw std::runtime_error{"There is no reason to use this function on non-Windows platforms"};
#endif
    }

    auto inline to_wstring(std::u16string_view input) -> std::wstring
    {
#ifdef PLATFORM_WINDOWS
        return {input.begin(), input.end()};
#else
        throw std::runtime_error{"There is no reason to use this function on non-Windows platforms"};
#endif
    }

    auto inline to_wstring(std::u32string_view input) -> std::wstring
    {
#if __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif
#pragma warning(disable : 4996)
        // First convert char32_t to UTF-8, then UTF-8 to wchar_t
        static std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> u32_converter{};
        std::string utf8_temp = u32_converter.to_bytes(input.data(), input.data() + input.length());
        
        static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> wchar_converter{};
        return wchar_converter.from_bytes(utf8_temp);
#pragma warning(default : 4996)
#if __clang__
#pragma clang diagnostic pop
#endif
    }

    auto inline to_wstring(std::u8string_view input) -> std::wstring
    {
#pragma warning(disable : 4996)
        static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter{};
        return converter.from_bytes(reinterpret_cast<const char*>(input.data()), 
                                   reinterpret_cast<const char*>(input.data() + input.length()));
#pragma warning(default : 4996)
    }

    auto inline to_string(const std::wstring& input) -> std::string
    {
#pragma warning(disable : 4996)
        static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter{};
        return converter.to_bytes(input);
#pragma warning(default : 4996)
    }

    auto inline to_string(const wchar_t* pInput)
    {
        auto temp_input = std::wstring{pInput};
        return to_string(temp_input);
    }

    auto inline to_string(std::wstring_view input) -> std::string
    {
        auto temp_input = std::wstring{input};
        return to_string(temp_input);
    }

    auto inline to_string(std::u16string_view input) -> std::string
    {
#if __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif
#pragma warning(disable : 4996)
        static std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> converter{};
        return converter.to_bytes(input.data(), input.data() + input.length());
#pragma warning(default : 4996)
#if __clang__
#pragma clang diagnostic pop
#endif
    }

    auto inline to_string(std::u32string_view input) -> std::string
    {
#if __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif
#pragma warning(disable : 4996)
        static std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> converter{};
        return converter.to_bytes(input.data(), input.data() + input.length());
#pragma warning(default : 4996)
#if __clang__
#pragma clang diagnostic pop
#endif
    }

    auto inline to_string(std::u8string_view input) -> std::string
    {
        // u8string is already UTF-8, just need to convert types
        return std::string(reinterpret_cast<const char*>(input.data()), input.length());
    }

    auto inline to_string(std::string_view input) -> std::string
    {
        return std::string(input);
    }

    auto inline to_string(const std::string& input) -> std::string
    {
        return input;
    }

    auto inline to_u16string(const std::wstring& input) -> std::u16string
    {
        return {input.begin(), input.end()};
    }

    auto inline to_u16string(std::wstring_view input) -> std::u16string
    {
        auto temp_input = std::wstring{input};
        return to_u16string(temp_input);
    }

    auto inline to_u16string(const std::string& input) -> std::u16string
    {
        return {input.begin(), input.end()};
    }

    auto inline to_u16string(std::string_view input) -> std::u16string
    {
        auto temp_input = std::string{input};
        return to_u16string(temp_input);
    }

    auto inline to_u16string(std::u32string_view input) -> std::u16string
    {
#if __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif
#pragma warning(disable : 4996)
        // Convert char32_t to UTF-8, then UTF-8 to char16_t
        static std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> u32_converter{};
        std::string utf8_temp = u32_converter.to_bytes(input.data(), input.data() + input.length());
        
        static std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> u16_converter{};
        return u16_converter.from_bytes(utf8_temp);
#pragma warning(default : 4996)
#if __clang__
#pragma clang diagnostic pop
#endif
    }

    auto inline to_u16string(std::u8string_view input) -> std::u16string
    {
#if __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif
#pragma warning(disable : 4996)
        static std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> converter{};
        return converter.from_bytes(reinterpret_cast<const char*>(input.data()), 
                                   reinterpret_cast<const char*>(input.data() + input.length()));
#pragma warning(default : 4996)
#if __clang__
#pragma clang diagnostic pop
#endif
    }

    auto inline to_u16string(const std::u16string& input) -> std::u16string
    {
        return input;
    }

    auto inline to_u16string(std::u16string_view input) -> std::u16string
    {
        return std::u16string{input};
    }

    // Auto String Conversion

    // =======================================================================
    // DEPRECATED C++17 TRAITS
    // =======================================================================
#ifdef ENABLE_STRING_TRAIT_DEPRECATION_WARNING
    #define RC_DEPRECATED_STRING_TRAIT [[deprecated("This trait is deprecated. Use RC::StringLike concept instead. See upgrade guide for details.")]]
#else
    #define RC_DEPRECATED_STRING_TRAIT
#endif

    // All possible char types in this project
    template <typename T>
    struct RC_DEPRECATED_STRING_TRAIT _can_be_string_view_t : std::false_type
    {
    };
    template <>
    struct RC_DEPRECATED_STRING_TRAIT _can_be_string_view_t<wchar_t*> : std::true_type
    {
    };
    template <>
    struct RC_DEPRECATED_STRING_TRAIT _can_be_string_view_t<char*> : std::true_type
    {
    };
    template <>
    struct RC_DEPRECATED_STRING_TRAIT _can_be_string_view_t<char16_t*> : std::true_type
    {
    };
    template <>
    struct RC_DEPRECATED_STRING_TRAIT _can_be_string_view_t<const wchar_t*> : std::true_type
    {
    };
    template <>
    struct RC_DEPRECATED_STRING_TRAIT _can_be_string_view_t<const char*> : std::true_type
    {
    };
    template <>
    struct RC_DEPRECATED_STRING_TRAIT _can_be_string_view_t<const char16_t*> : std::true_type
    {
    };

    template <typename T>
    struct RC_DEPRECATED_STRING_TRAIT can_be_string_view_t : _can_be_string_view_t<std::decay_t<T>>
    {
    };

    template <typename T>
    struct RC_DEPRECATED_STRING_TRAIT is_string_like_t : std::false_type
    {
    };

    template <typename CharT>
    struct RC_DEPRECATED_STRING_TRAIT is_string_like_t<std::basic_string<CharT>> : std::true_type
    {
        // T is a string or string view of CharT
    };

    template <typename CharT>
    struct RC_DEPRECATED_STRING_TRAIT is_string_like_t<std::basic_string_view<CharT>> : std::true_type
    {
        // T is a string or string view of CharT
    };

    template <typename T, typename CharT>
    struct RC_DEPRECATED_STRING_TRAIT is_charT_string_type : std::disjunction<std::is_same<T, std::basic_string<CharT>>, std::is_same<T, std::basic_string_view<CharT>>>
    {
        // T is a string or string view of CharT
    };

    template <typename T, typename CharT>
    struct RC_DEPRECATED_STRING_TRAIT not_charT_string_like_t : std::conjunction<is_string_like_t<std::decay_t<T>>, std::negation<is_charT_string_type<std::decay_t<T>, CharT>>>
    {
        // 1. T is a string or string view
        // 2. T is not a string or string view of CharT
    };

    template <typename T>
    auto stringviewify(T&& tp)
    {
        // Convert a char pointer to a string view
        return std::basic_string_view<std::remove_const_t<std::remove_pointer_t<std::decay_t<T>>>>{tp};
    }

    template <typename CharT, typename T>
    auto inline to_charT_string(T&& arg) -> std::basic_string<CharT>
    {
        // Dispatch to the correct conversion function based on the CharT type
        if constexpr (std::is_same_v<CharT, wchar_t>)
        {
            return to_wstring(std::forward<T>(arg));
        }
        else if constexpr (std::is_same_v<CharT, char16_t>)
        {
            return to_u16string(std::forward<T>(arg));
        }
        else if constexpr (std::is_same_v<CharT, char>)
        {
            return to_string(std::forward<T>(arg));
        }
        else if constexpr (std::is_same_v<CharT, char32_t>)
        {
            // For now, convert through UTF-8
            auto utf8 = to_string(std::forward<T>(arg));
#if __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif
#pragma warning(disable : 4996)
            static std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> converter{};
            return converter.from_bytes(utf8);
#pragma warning(default : 4996)
#if __clang__
#pragma clang diagnostic pop
#endif
        }
        else if constexpr (std::is_same_v<CharT, char8_t>)
        {
            // Convert to UTF-8 string and reinterpret
            auto utf8 = to_string(std::forward<T>(arg));
            return std::basic_string<char8_t>(reinterpret_cast<const char8_t*>(utf8.data()), utf8.length());
        }
        else
        {
            static_assert(sizeof(CharT) == 0, "Unsupported target character type");
        }
    }

    template <typename CharT, typename T_Path>
    auto inline to_charT_string_path(T_Path&& arg) -> std::basic_string<CharT>
    {
        static_assert(std::is_same_v<std::decay_t<T_Path>, std::filesystem::path>, "Input must be std::filesystem::path");

        if constexpr (std::is_same_v<CharT, wchar_t>) // Covers WIDECHAR and Windows TCHAR
        {
            return arg.wstring();
        }
        else if constexpr (std::is_same_v<CharT, char>) // Covers ANSICHAR, for UTF-8 output
        {
            // For UTF-8 std::string output from path
            std::u8string u8_s = arg.u8string(); // path.u8string() IS UTF-8
            return std::basic_string<CharT>(reinterpret_cast<const CharT*>(u8_s.c_str()), u8_s.length());
        }
        else if constexpr (std::is_same_v<CharT, uint16_t> || std::is_same_v<CharT, char16_t>) // Covers CHAR16 and standard char16_t
        {
            // Goal: Convert path to std::basic_string<uint16_t> or std::basic_string<char16_t> (UTF-16)

            // Option 1: If wchar_t is 16-bit (like on Windows) and represents UTF-16
            if constexpr (sizeof(wchar_t) == sizeof(CharT))
            {
                std::wstring temp_ws = arg.wstring(); // Get UTF-16 as std::wstring
                // If CharT is uint16_t and wchar_t is also 16-bit, this reinterpret_cast is common.
                // If CharT is char16_t and wchar_t is also 16-bit, also common.
                return std::basic_string<CharT>(reinterpret_cast<const CharT*>(temp_ws.c_str()), temp_ws.length());
            }
            // Option 2: Convert from path's u8string (UTF-8) to UTF-16 (CharT)
            // This is more portable if wchar_t size varies or isn't guaranteed to be UTF-16.
            else
            {
                std::u8string u8_s = arg.u8string();
                if (u8_s.empty())
                {
                    return std::basic_string<CharT>();
                }

                try
                {
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4996)
#elif defined(__clang__) || defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

                    // Ensure CharT for the converter is char16_t if that's what codecvt expects
                    // If CharT is uint16_t, we might need to cast the result or use a char16_t intermediate.
                    // For simplicity, let's assume we convert to std::u16string (char16_t based)
                    // and then construct std::basic_string<CharT> from it.

                    using IntermediateChar16Type = char16_t; // Standard type for codecvt
                    std::wstring_convert<std::codecvt_utf8_utf16<IntermediateChar16Type, 0x10FFFF, std::little_endian>, IntermediateChar16Type> converter;
                    std::basic_string<IntermediateChar16Type> u16_intermediate_s =
                            converter.from_bytes(reinterpret_cast<const char*>(u8_s.data()), reinterpret_cast<const char*>(u8_s.data() + u8_s.length()));

                    // Now construct the final std::basic_string<CharT>
                    // This assumes CharT (e.g., uint16_t) and IntermediateChar16Type (char16_t)
                    // have compatible representations for UTF-16 code units.
                    return std::basic_string<CharT>(reinterpret_cast<const CharT*>(u16_intermediate_s.data()), u16_intermediate_s.length());

#if defined(_MSC_VER)
#pragma warning(pop)
#elif defined(__clang__) || defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
                }
                catch (const std::exception&)
                {
                    // Catching std::exception for broader compatibility
                    throw std::runtime_error("Failed to convert path from UTF-8 to UTF-16");
                }
            }
        }
        else
        {
            // This static_assert will provide a compile-time error for unsupported CharT types.
            static_assert(std::is_same_v<CharT, wchar_t> || std::is_same_v<CharT, char> || std::is_same_v<CharT, uint16_t> || std::is_same_v<CharT, char16_t>,
                          "to_charT_string_path: Unsupported target CharT for path conversion");
            return std::basic_string<CharT>(); // Should be unreachable due to static_assert
        }
    }

    // Convert any string-like to a string of generic CharT
    template <typename TargetCharT, typename T>
    auto inline to_charT(T&& arg)
    {
        if constexpr (StringLike<T>)
        {
            using SourceCharT = string_char_t<T>;

            if constexpr (std::is_same_v<SourceCharT, TargetCharT>) {
                // Already correct type - pass through
                return std::forward<T>(arg);
            }
            else if constexpr (std::is_same_v<std::decay_t<T>, std::filesystem::path>) {
                // Special handling for filesystem paths
                return to_charT_string_path<TargetCharT>(arg);
            }
            else {
                // Convert via string_view with explicit type handling
                if constexpr (std::is_same_v<SourceCharT, char>) {
                    if constexpr (StringLikePointer<T>) {
                        std::string_view view(arg);
                        return to_charT_string<TargetCharT>(view);
                    } else {
                        std::string_view view(std::ranges::data(arg), std::ranges::size(arg));
                        return to_charT_string<TargetCharT>(view);
                    }
                }
                else if constexpr (std::is_same_v<SourceCharT, wchar_t>) {
                    if constexpr (StringLikePointer<T>) {
                        std::wstring_view view(arg);
                        return to_charT_string<TargetCharT>(view);
                    } else {
                        std::wstring_view view(std::ranges::data(arg), std::ranges::size(arg));
                        return to_charT_string<TargetCharT>(view);
                    }
                }
                else if constexpr (std::is_same_v<SourceCharT, char16_t>) {
                    if constexpr (StringLikePointer<T>) {
                        std::u16string_view view(arg);
                        return to_charT_string<TargetCharT>(view);
                    } else {
                        std::u16string_view view(std::ranges::data(arg), std::ranges::size(arg));
                        return to_charT_string<TargetCharT>(view);
                    }
                }
                else if constexpr (std::is_same_v<SourceCharT, char32_t>) {
                    if constexpr (StringLikePointer<T>) {
                        std::u32string_view view(arg);
                        return to_charT_string<TargetCharT>(view);
                    } else {
                        std::u32string_view view(std::ranges::data(arg), std::ranges::size(arg));
                        return to_charT_string<TargetCharT>(view);
                    }
                }
                else if constexpr (std::is_same_v<SourceCharT, char8_t>) {
                    if constexpr (StringLikePointer<T>) {
                        std::u8string_view view(arg);
                        return to_charT_string<TargetCharT>(view);
                    } else {
                        std::u8string_view view(std::ranges::data(arg), std::ranges::size(arg));
                        return to_charT_string<TargetCharT>(view);
                    }
                }
                else {
                    // This should never happen if Character concept is complete
                    static_assert(!std::is_same_v<SourceCharT, SourceCharT>, "Unsupported source character type in to_charT");
                    return std::basic_string<TargetCharT>{};
                }
            }
        }
        else {
            // Not string-like, pass through unchanged
            return std::forward<T>(arg);
        }
    }

    // Ensure that a string is compatible with UE4SS, converting it if neccessary
    template <typename T>
    auto inline ensure_str(T&& arg) /* -> StringType */
    {
        return ensure_str_as<CharType>(std::forward<T>(arg)); // CharType is the project's native char type
    }

    template <typename TargetCharT, typename T>
    auto inline ensure_str_as(T&& arg) -> std::basic_string<TargetCharT>
    {
        return to_charT<TargetCharT>(std::forward<T>(arg));
    }

    template <StringLike T>
    auto inline to_utf8_string(T&& arg) -> std::string
    {
        // Fast path for already UTF-8
        if constexpr (Utf8StringLike<T>) {
            if constexpr (StringLikePointer<T>) {
                return std::string(arg);
            }
            else if constexpr (std::is_same_v<std::decay_t<T>, std::string>) {
                return std::forward<T>(arg);  // Perfect forwarding
            }
            else {
                return std::string(std::ranges::data(arg), std::ranges::size(arg));
            }
        }
        else {
            return to_charT<char>(std::forward<T>(arg));
        }
    }

    // You can add more to_* function if needed

    // Auto Type Conversion Done

    /**
     * Normalizes a path for use in Lua, ensuring:
     * 1. UTF-8 encoding for proper Unicode handling
     * 2. Forward slashes for consistency across platforms
     * 
     * @param path The path to normalize
     * @return A UTF-8 encoded string with forward slashes
     * @throws std::runtime_error if conversion fails
     */
    auto inline normalize_path_for_lua(const std::filesystem::path& path) -> std::string
    {
        std::string utf8_path = to_utf8_string(path);
        
        // Replace backslashes with forward slashes for Lua
        std::replace(utf8_path.begin(), utf8_path.end(), '\\', '/');
        
        return utf8_path;
    }
    
    /**
     * Creates a Windows-compatible wide string from a UTF-8 path string
     * This is useful when opening files with Windows APIs that expect UTF-16
     * 
     * @param utf8_path UTF-8 encoded path string
     * @return Wide string (UTF-16) for Windows APIs
     * @throws std::runtime_error if conversion fails
     */
    auto inline utf8_to_wpath(const std::string& utf8_path) -> std::wstring
    {
        // No fallbacks - if this fails, it should throw since it's a critical error
        // that indicates invalid UTF-8 input
        return to_wstring(utf8_path);
    }

    auto inline to_generic_string(const auto& input) -> StringType
    {
        if constexpr (std::is_same_v<std::remove_cvref_t<std::remove_pointer_t<std::remove_cvref_t<decltype(input)>>>, StringViewType>)
        {
            return StringType{input};
        }
        else if constexpr (std::is_same_v<std::remove_cvref_t<std::remove_pointer_t<std::remove_cvref_t<decltype(input)>>>, StringType> ||
                           std::is_same_v<std::remove_cvref_t<std::remove_pointer_t<std::remove_cvref_t<decltype(input)>>>, CharType>)
        {
            return input;
        }
        else
        {
#if RC_IS_ANSI == 1
            return to_string(input);
#else
            return ensure_str(input);
#endif
        }
    }
    auto inline ensure_str_const(std::string_view input) -> const StringType&
    {
        static std::unordered_map<std::string_view, StringType> uestringpool;
        static std::shared_mutex uestringpool_lock;

        // Allow multiple readers that are stalled when any thread is writing.
        {
            std::shared_lock<std::shared_mutex> read_guard(uestringpool_lock);
            if (uestringpool.contains(input)) return uestringpool[input];
        }

        auto temp_input = std::string{input};
        auto new_str = ensure_str(temp_input);

        // Stall the readers to insert a new string.
        {
            std::lock_guard<std::shared_mutex> write_guard(uestringpool_lock);
            const auto& [emplaced_iter, unused] = uestringpool.emplace(input, std::move(new_str));
            return emplaced_iter->second;
        }
    }

    namespace String
    {
        template <typename CharT>
        auto inline iequal(std::basic_string_view<CharT> a, std::basic_string_view<CharT> b)
        {
            return a.size() == b.size() && std::equal(a.begin(), a.end(), b.begin(), [](const CharT a_char, const CharT b_char) {
                       return std::towlower((wchar_t)a_char) == std::towlower((wchar_t)b_char);
                   });
        }

        template <typename CharT>
        auto inline iequal(std::basic_string<CharT>& a, const CharT* b)
        {
            return iequal(std::basic_string_view<CharT>{a}, std::basic_string_view<CharT>{b});
        }

        template <typename CharT>
        auto inline iequal(const CharT* a, std::basic_string<CharT>& b)
        {
            return iequal(std::basic_string_view<CharT>{a}, std::basic_string_view<CharT>{b});
        }

        template <typename CharT>
        auto inline str_cmp_insensitive(const CharT* a, std::basic_string<CharT>& b)
        {
            return iequal(std::basic_string_view<CharT>{a}, std::basic_string_view<CharT>{b});
        }
    } // namespace String

    // Numeric array parsing utilities
    namespace String
    {
        // Helper to trim whitespace from string_view
        template <typename CharT>
        auto inline trim(std::basic_string_view<CharT> sv) -> std::basic_string_view<CharT>
        {
            const CharT* whitespace = nullptr;
            if constexpr (std::is_same_v<CharT, char>)
            {
                whitespace = " \t\n\r";
            }
            else
            {
                whitespace = L" \t\n\r";
            }
            
            size_t start = sv.find_first_not_of(whitespace);
            if (start == std::basic_string_view<CharT>::npos) return {};

            size_t end = sv.find_last_not_of(whitespace);

            // Optimization: avoid substr if already trimmed
            if (start == 0 && end == sv.size() - 1)
            {
                return sv;
            }

            return sv.substr(start, end - start + 1);
        }

        // Helper to remove parentheses if present
        template <typename CharT>
        auto inline remove_parentheses(std::basic_string_view<CharT> sv) -> std::basic_string_view<CharT>
        {
            if (sv.size() >= 2 && sv.front() == static_cast<CharT>('(') && sv.back() == static_cast<CharT>(')'))
            {
                return sv.substr(1, sv.size() - 2);
            }
            return sv;
        }

        // Convert string to numeric type with proper bounds checking
        template <typename T, typename CharT>
        auto string_to_numeric(const std::basic_string<CharT>& s) -> T
        {
            if constexpr (std::is_same_v<T, float>)
            {
                if constexpr (std::is_same_v<CharT, wchar_t>)
                {
                    return std::stof(to_string(s));
                }
                else
                {
                    return std::stof(s);
                }
            }
            else if constexpr (std::is_same_v<T, double>)
            {
                if constexpr (std::is_same_v<CharT, wchar_t>)
                {
                    return std::stod(to_string(s));
                }
                else
                {
                    return std::stod(s);
                }
            }
            else if constexpr (std::is_same_v<T, int32_t>)
            {
                if constexpr (std::is_same_v<CharT, wchar_t>)
                {
                    return std::stoi(to_string(s));
                }
                else
                {
                    return std::stoi(s);
                }
            }
            else if constexpr (std::is_same_v<T, int64_t>)
            {
                if constexpr (std::is_same_v<CharT, wchar_t>)
                {
                    return std::stoll(to_string(s));
                }
                else
                {
                    return std::stoll(s);
                }
            }
            else if constexpr (std::is_same_v<T, uint32_t>)
            {
                if constexpr (std::is_same_v<CharT, wchar_t>)
                {
                    auto val = std::stoul(to_string(s));
                    if (val > std::numeric_limits<uint32_t>::max())
                    {
                        throw std::out_of_range("Value too large for uint32_t");
                    }
                    return static_cast<uint32_t>(val);
                }
                else
                {
                    auto val = std::stoul(s);
                    if (val > std::numeric_limits<uint32_t>::max())
                    {
                        throw std::out_of_range("Value too large for uint32_t");
                    }
                    return static_cast<uint32_t>(val);
                }
            }
            else if constexpr (std::is_same_v<T, uint64_t>)
            {
                if constexpr (std::is_same_v<CharT, wchar_t>)
                {
                    return std::stoull(to_string(s));
                }
                else
                {
                    return std::stoull(s);
                }
            }
            else
            {
                static_assert(sizeof(T) == 0, "Unsupported numeric type");
            }
        }

        // Core parsing result structure
        template <typename NumericType, size_t N>
        struct ParseResult
        {
            bool success = false;
            std::string error;
            size_t components_parsed = 0;
        };

        // Core parsing implementation
        template <typename NumericType, size_t N, typename CharT>
        auto parse_numeric_array_impl(const std::basic_string<CharT>& s, std::array<NumericType, N>& out_array) -> ParseResult<NumericType, N>
        {
            ParseResult<NumericType, N> result;

            if (s.empty())
            {
                result.error = "Input string is empty";
                return result;
            }

            // Preprocessing
            std::basic_string_view<CharT> sv = trim(std::basic_string_view<CharT>{s});
            if (sv.empty())
            {
                result.error = "Input contains only whitespace";
                return result;
            }

            sv = remove_parentheses(sv);

            // Parse components
            std::basic_string<CharT> sv_string{sv};
            size_t start = 0;
            size_t pos = 0;

            while (pos != std::basic_string<CharT>::npos && result.components_parsed < N)
            {
                pos = sv_string.find(static_cast<CharT>(','), start);
                std::basic_string_view<CharT> component = (pos == std::basic_string<CharT>::npos) 
                    ? std::basic_string_view<CharT>{sv_string}.substr(start)
                    : std::basic_string_view<CharT>{sv_string}.substr(start, pos - start);

                component = trim(component);
                if (component.empty())
                {
                    result.error = "Empty component at position " + std::to_string(result.components_parsed);
                    return result;
                }

                try
                {
                    out_array[result.components_parsed] = string_to_numeric<NumericType>(std::basic_string<CharT>{component});
                    result.components_parsed++;
                }
                catch (const std::invalid_argument&)
                {
                    std::string component_str;
                    if constexpr (std::is_same_v<CharT, char>)
                    {
                        component_str = std::basic_string<CharT>{component};
                    }
                    else
                    {
                        component_str = to_string(std::basic_string<CharT>{component});
                    }
                    result.error = "Invalid number format at position " + std::to_string(result.components_parsed) + ": '" + component_str + "'";
                    return result;
                }
                catch (const std::out_of_range&)
                {
                    std::string component_str;
                    if constexpr (std::is_same_v<CharT, char>)
                    {
                        component_str = std::basic_string<CharT>{component};
                    }
                    else
                    {
                        component_str = to_string(std::basic_string<CharT>{component});
                    }
                    result.error = "Number out of range at position " + std::to_string(result.components_parsed) + ": '" + component_str + "'";
                    return result;
                }

                if (pos != std::basic_string<CharT>::npos)
                {
                    start = pos + 1;
                }
            }

            if (result.components_parsed != N)
            {
                result.error = "Wrong number of components: expected " + std::to_string(N) + ", found " + std::to_string(result.components_parsed);
                return result;
            }

            result.success = true;
            return result;
        }

        /**
         * Parses a comma-separated string of numeric values into a fixed-size array.
         * Supports optional parentheses around the values.
         *
         * Examples:
         *   "1.0, 2.0, 3.0" -> {1.0f, 2.0f, 3.0f}
         *   "(1, 2, 3)"      -> {1, 2, 3}
         *   " ( 1.5 , 2.5 , 3.5 ) " -> {1.5f, 2.5f, 3.5f}
         *
         * @param s The input string to parse
         * @param out_array The array to fill with parsed values
         * @return true if parsing succeeded, false otherwise
         */
        template <typename NumericType, size_t N, typename CharT>
        auto try_parse_numeric_array(const std::basic_string<CharT>& s, std::array<NumericType, N>& out_array) -> bool
        {
            return parse_numeric_array_impl(s, out_array).success;
        }

        /**
         * Parses a comma-separated string of numeric values with detailed error reporting.
         *
         * @param s The input string to parse
         * @param out_array The array to fill with parsed values
         * @param error_msg Will be filled with error description if parsing fails
         * @return true if parsing succeeded, false otherwise
         */
        template <typename NumericType, size_t N, typename CharT>
        auto try_parse_numeric_array(const std::basic_string<CharT>& s, std::array<NumericType, N>& out_array, std::string& error_msg) -> bool
        {
            auto result = parse_numeric_array_impl(s, out_array);
            error_msg = result.error;
            return result.success;
        }

        /**
         * Parses a comma-separated string of numeric values, returning an optional result.
         *
         * @param s The input string to parse
         * @return An optional containing the parsed array if successful, nullopt otherwise
         */
        template <typename NumericType, size_t N, typename CharT>
        auto parse_numeric_array(const std::basic_string<CharT>& s) -> std::optional<std::array<NumericType, N>>
        {
            std::array<NumericType, N> result;
            if (parse_numeric_array_impl(s, result).success)
            {
                return result;
            }
            return std::nullopt;
        }
        
        /**
         * Platform-safe string copy that handles the differences between 
         * strncpy_s (Windows) and strncpy (other platforms).
         * Ensures null termination on all platforms.
         * 
         * @param dest Destination buffer
         * @param dest_size Size of destination buffer
         * @param src Source string
         * @param count Maximum number of characters to copy (excluding null terminator)
         */
        inline void safe_string_copy(char* dest, size_t dest_size, 
                                   const char* src, size_t count)
        {
#ifdef PLATFORM_WINDOWS
            strncpy_s(dest, dest_size, src, count);
#else
            size_t copy_count = std::min(count, dest_size - 1);
            strncpy(dest, src, copy_count);
            dest[copy_count] = '\0';
#endif
        }

        /**
         * Platform-safe string copy for arrays with automatic size detection.
         * 
         * @param dest Destination array
         * @param src Source string
         */
        template<size_t N>
        inline void safe_string_copy(char (&dest)[N], const char* src)
        {
            safe_string_copy(dest, N, src, N - 1);
        }

        /**
         * Platform-safe wide string copy.
         * 
         * @param dest Destination buffer
         * @param dest_size Size of destination buffer in wide characters
         * @param src Source wide string
         * @param count Maximum number of wide characters to copy (excluding null terminator)
         */
        inline void safe_string_copy(wchar_t* dest, size_t dest_size,
                                   const wchar_t* src, size_t count)
        {
#ifdef PLATFORM_WINDOWS
            wcsncpy_s(dest, dest_size, src, count);
#else
            size_t copy_count = std::min(count, dest_size - 1);
            wcsncpy(dest, src, copy_count);
            dest[copy_count] = L'\0';
#endif
        }

        /**
         * Platform-safe wide string copy for arrays with automatic size detection.
         * 
         * @param dest Destination wide character array
         * @param src Source wide string
         */
        template<size_t N>
        inline void safe_string_copy(wchar_t (&dest)[N], const wchar_t* src)
        {
            safe_string_copy(dest, N, src, N - 1);
        }
    } // namespace String
} // namespace RC