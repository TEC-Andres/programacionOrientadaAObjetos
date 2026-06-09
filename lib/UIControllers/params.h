#pragma once

#include <string>
#include <type_traits>

namespace ui {
namespace component {

struct named_param_base {};

template<typename T, typename Tag>
struct Param : named_param_base {
    using tag_type = Tag;
    using value_type = T;
    T value{};

    Param() = default;
    Param(const T& v) : value(v) {}
};



#define UI_DEFINE_PARAM(Name, Type) \
    struct Name##_tag { \
        Param<Type, Name##_tag> operator=(const Type& v) const { \
            return {v}; \
        } \
    }; \
    inline constexpr Name##_tag Name{};

UI_DEFINE_PARAM(text, std::string)
UI_DEFINE_PARAM(width, int)
UI_DEFINE_PARAM(bgColor, std::string)
UI_DEFINE_PARAM(fgColor, std::string)
UI_DEFINE_PARAM(borderColor, std::string)
UI_DEFINE_PARAM(selected, bool)
UI_DEFINE_PARAM(resizable, bool)

UI_DEFINE_PARAM(title, std::string)
UI_DEFINE_PARAM(message, std::string)
UI_DEFINE_PARAM(height, int)
UI_DEFINE_PARAM(titleBgColor, std::string)
UI_DEFINE_PARAM(titleFgColor, std::string)
UI_DEFINE_PARAM(bodyBgColor, std::string)
UI_DEFINE_PARAM(bodyFgColor, std::string)

UI_DEFINE_PARAM(borderFgColor, std::string)
UI_DEFINE_PARAM(borderBgColor, std::string)

UI_DEFINE_PARAM(displacementX, float)
UI_DEFINE_PARAM(displacementY, float)

UI_DEFINE_PARAM(maxLength, int)
UI_DEFINE_PARAM(passwordMode, bool)

UI_DEFINE_PARAM(segment1Visible, int)
UI_DEFINE_PARAM(segment1Text, std::string)
UI_DEFINE_PARAM(segment2Visible, int)
UI_DEFINE_PARAM(segment2Text, std::string)
UI_DEFINE_PARAM(segment3Visible, int)
UI_DEFINE_PARAM(segment3Text, std::string)

#undef UI_DEFINE_PARAM

template<typename T>
struct is_param : std::is_base_of<named_param_base, T> {};

template<typename T>
inline constexpr bool is_param_v = is_param<T>::value;

} // namespace component
} // namespace ui
