/*
 * Copyright 2023 - All rights reserved.
 * License: https://equals-forty-two.com/LICENSE
 * 
 * Framework API types. 
 * All types that is shared with plugins must be defined here.
 *  - Restrict types to constexpr, enum and structs
 */

#pragma once

#include <type_traits>

#include <foundation/platform.h>

#ifndef FOUNDATION_FORCEINLINE
#   define FOUNDATION_FORCEINLINE inline
#endif

typedef uint8_t byte_t;
typedef uint64_t hash_t;
typedef int64_t tick_t;
typedef double deltatime_t;
typedef uint32_t object_t;
typedef uint128_t uuid_t;
typedef void* user_data_t;

constexpr object_t INVALID_HANDLE = UINT32_MAX;

typedef void(*api_callback_t)(void* user_data);
typedef void(*api_callback_payload_t)(void* payload, size_t payload_size, void* user_data);

/*! @def API_ENUM_FLAGS 
 *
 *  @brief Defines bitwise operators for an enum class.
 */
#define API_ENUM_FLAGS(T) \
    FOUNDATION_FORCEINLINE T operator~ (T a) { return static_cast<T>(~(std::underlying_type_t<T>)a); } \
    FOUNDATION_FORCEINLINE bool operator!= (const T a, const std::underlying_type_t<T> b) { return (std::underlying_type_t<T>)a != b; } \
    FOUNDATION_FORCEINLINE bool operator== (const T a, const std::underlying_type_t<T> b) { return (std::underlying_type_t<T>)a == b; } \
    FOUNDATION_FORCEINLINE bool operator&& (const T a, const T b) { return (std::underlying_type_t<T>)a != 0 && (std::underlying_type_t<T>)b != 0; } \
    FOUNDATION_FORCEINLINE bool operator&& (const T a, const bool b) { return (std::underlying_type_t<T>)a != 0 && b; } \
    FOUNDATION_FORCEINLINE constexpr T operator| (const T a, const T b) { return (T)((std::underlying_type_t<T>)a | (std::underlying_type_t<T>)b); } \
    FOUNDATION_FORCEINLINE T operator& (const T a, const T b) { return (T)((std::underlying_type_t<T>)a & (std::underlying_type_t<T>)b); } \
    FOUNDATION_FORCEINLINE T operator^ (const T a, const T b) { return (T)((std::underlying_type_t<T>)a ^ (std::underlying_type_t<T>)b); } \
    FOUNDATION_FORCEINLINE T& operator|= (T& a, const T b) { return (T&)((std::underlying_type_t<T>&)a |= (std::underlying_type_t<T>)b); } \
    FOUNDATION_FORCEINLINE T& operator&= (T& a, const T b) { return (T&)((std::underlying_type_t<T>&)a &= (std::underlying_type_t<T>)b); } \
    FOUNDATION_FORCEINLINE T& operator^= (T& a, const T b) { return (T&)((std::underlying_type_t<T>&)a ^= (std::underlying_type_t<T>)b); } \
    FOUNDATION_FORCEINLINE bool test(const T a, const T b) { return (a & b) == b; } \
    FOUNDATION_FORCEINLINE bool any(const T a, const T b) { return (a & b) != 0; } \
    FOUNDATION_FORCEINLINE bool none(const T a, const T b) { return (a & b) == 0; } \
    FOUNDATION_FORCEINLINE bool one(const T a, const T b) { const auto bits = ((std::underlying_type_t<T>)a & (std::underlying_type_t<T>)b); return bits && !(bits & (bits-1)); }

/*! Set of flags used to customize the registration of a new menu item. */
typedef enum class AppMenuFlags
{
    None = 0,

    /*! Insert the menu items after all other menu items, this helps preserve the system menu order. */
    Append = 1 << 0,

    /*! Menu item defines a shortcut */
    Shortcut = 1 << 1,

} app_menu_flags_t; 
API_ENUM_FLAGS(AppMenuFlags);

