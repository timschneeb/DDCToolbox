#pragma once
#ifndef _ENUM_CLASS_BITWISE_H_
#define _ENUM_CLASS_BITWISE_H_

#include <type_traits>

//unary ~operator
template <typename Enum, typename std::enable_if_t<std::is_enum<Enum>::value, int> = 0>
constexpr inline Enum& operator~ (Enum& val)
{
    val = static_cast<Enum>(~static_cast<std::underlying_type_t<Enum>>(val));
    return val;
}

// & operator
template <typename Enum, typename std::enable_if_t<std::is_enum<Enum>::value, int> = 0>
constexpr inline Enum operator& (Enum lhs, Enum rhs)
{
    return static_cast<Enum>(static_cast<std::underlying_type_t<Enum>>(lhs) & static_cast<std::underlying_type_t<Enum>>(rhs));
}

// &= operator
template <typename Enum, typename std::enable_if_t<std::is_enum<Enum>::value, int> = 0>
constexpr inline Enum operator&= (Enum& lhs, Enum rhs)
{
    lhs = static_cast<Enum>(static_cast<std::underlying_type_t<Enum>>(lhs) & static_cast<std::underlying_type_t<Enum>>(rhs));
    return lhs;
}

//| operator

template <typename Enum, typename std::enable_if_t<std::is_enum<Enum>::value, int> = 0>
constexpr inline Enum operator| (Enum lhs, Enum rhs)
{
    return static_cast<Enum>(static_cast<std::underlying_type_t<Enum>>(lhs) | static_cast<std::underlying_type_t<Enum>>(rhs));
}
//|= operator

template <typename Enum, typename std::enable_if_t<std::is_enum<Enum>::value, int> = 0>
constexpr inline Enum& operator|= (Enum& lhs, Enum rhs)
{
    lhs = static_cast<Enum>(static_cast<std::underlying_type_t<Enum>>(lhs) | static_cast<std::underlying_type_t<Enum>>(rhs));
    return lhs;
}

template<typename T>
class BitFlags
{
public:

    constexpr inline BitFlags() = default;
    constexpr inline BitFlags(T value) { mValue = value; }
    constexpr inline BitFlags operator| (T rhs) const { return mValue | rhs; }
    constexpr inline BitFlags operator& (T rhs) const { return mValue & rhs; }
    constexpr inline BitFlags operator~ () const { return ~mValue; }
    constexpr inline operator T() const { return mValue; }
    constexpr inline BitFlags& operator|=(T rhs) { mValue |= rhs; return *this; }
    constexpr inline BitFlags& operator&=(T rhs) { mValue &= rhs; return *this; }
    constexpr inline bool test(T rhs) const { return (mValue & rhs) == rhs; }
    constexpr inline void set(T rhs) { mValue |= rhs; }
    constexpr inline void clear(T rhs) { mValue &= ~rhs; }

private:
    T mValue;
};

#endif // _ENUM_CLASS_BITWISE_H_
