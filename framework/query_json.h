/*
 * Copyright 2022-2023 - All rights reserved.
 * License: https://wiimag.com/LICENSE
 */

#pragma once

#include <framework/string.h>
#include <framework/common.h>

#include <foundation/json.h>
#include <foundation/array.h>
#include <foundation/string.h>

struct json_object_t;

const json_token_t* json_find_token(const json_object_t& json, const char* key, size_t key_length = 0);

double json_read_number(const char* json, const json_token_t* tokens, const json_token_t* value, double default_value = NAN);

struct json_object_t
{
    bool child{ false }; // Child objects to not own the tokens allocation
    const char* buffer;
    size_t token_count;
    json_token_t* tokens;
    const json_token_t* root;
    long status_code{ 0 };
    long error_code{ 0 };
    string_const_t query{};
    bool resolved_from_cache{ false };

    json_object_t()
        : buffer(nullptr)
        , token_count(0)
        , tokens(nullptr)
        , root(nullptr)
        , child(false)
        , resolved_from_cache(false)
    {

    }

    /// <summary>
    /// Construct a json object from a string constant literal.
    /// </summary>
    /// <param name="json_string"></param>
    json_object_t(string_const_t json_string)
        : buffer(json_string.str)
        , token_count(0)
        , tokens(nullptr)
        , root(nullptr)
        , resolved_from_cache(false)
    {
        token_count = json_parse(STRING_ARGS(json_string), nullptr, 0);
        if (token_count > 0)
        {
            array_reserve(tokens, token_count);
            token_count = json_parse(STRING_ARGS(json_string), tokens, token_count);
            root = &tokens[0];
        }
    }

    FOUNDATION_FORCEINLINE json_object_t(const string_t& buffer)
        : json_object_t(string_const(STRING_ARGS(buffer)))
    {
    }

    json_object_t(const json_object_t& json, const json_token_t* obj = nullptr)
        : child(true)
        , buffer(json.buffer)
        , token_count(json.token_count)
        , tokens(json.tokens)
        , root(obj ? obj : json.root)
        , status_code(json.status_code)
        , error_code(json.error_code)
        , query(json.query)
        , resolved_from_cache(json.resolved_from_cache)
    {
    }

    json_object_t(json_object_t&& src) noexcept
        : child(src.child)
        , buffer(src.buffer)
        , token_count(src.token_count)
        , tokens(src.tokens)
        , root(src.root)
        , status_code(src.status_code)
        , error_code(src.error_code)
        , query(src.query)
        , resolved_from_cache(false)
    {
        src.child = true;
        src.buffer = nullptr;
        src.token_count = 0;
        src.tokens = nullptr;
        src.root = nullptr;
        src.query = {};
    }

    json_object_t& operator=(const json_object_t& src) noexcept
    {
        buffer = src.buffer;
        token_count = src.token_count;
        tokens = src.tokens;
        root = src.root;
        child = true;
        status_code = src.status_code;
        error_code = src.error_code;
        query = src.query;
        resolved_from_cache = src.resolved_from_cache;
        return *this;
    }

    json_object_t& operator=(json_object_t&& src) noexcept
    {
        buffer = src.buffer;
        token_count = src.token_count;
        tokens = src.tokens;
        root = src.root;
        status_code = src.status_code;
        error_code = src.error_code;
        query = src.query;
        resolved_from_cache = src.resolved_from_cache;

        src.buffer = nullptr;
        src.token_count = 0;
        src.tokens = nullptr;
        src.root = nullptr;
        query = {};

        return *this;
    }

    FOUNDATION_FORCEINLINE ~json_object_t()
    {
        if (!child)
            array_deallocate(tokens);
    }

    FOUNDATION_FORCEINLINE string_const_t id() const
    {
        if (buffer == nullptr || root == nullptr)
            return string_null();
        return string_const(buffer + root->id, root->id_length);
    }

    FOUNDATION_FORCEINLINE string_const_t to_string() const
    {
        if (buffer == nullptr || root == nullptr)
            return string_null();
        return string_const(buffer + root->value, root->value_length);
    }

    FOUNDATION_FORCEINLINE bool is_valid() const
    {
        return root && tokens;
    }

    FOUNDATION_FORCEINLINE bool is_null() const
    {
        if (root == nullptr || buffer == nullptr)
            return true;

        if (root->type == JSON_UNDEFINED)
            return true;

        if (root->type == JSON_PRIMITIVE)
        {
            string_const_t value = json_token_value(buffer, root);
            return string_equal(STRING_CONST("null"), STRING_ARGS(value));
        }

        return false;
    }

    FOUNDATION_FORCEINLINE bool resolved() const
    {
        return is_valid() && status_code < 400;
    }

    const json_object_t find(const char* path, size_t path_length, bool allow_null = true) const
    {
        const json_object_t& json = *this;
        string_const_t token, r;
        string_split(path, path_length, STRING_CONST("."), &token, &r, false);

        json_object_t ref = json;
        while (token.length > 0)
        {
            string_const_t subToken, orChoiceR;
            string_split(STRING_ARGS(token), STRING_CONST("|"), &subToken, &orChoiceR, false);
            bool foundSubPart = false;
            while (subToken.length > 0)
            {
                auto subRef = ref.get(STRING_ARGS(subToken));
                if (allow_null ? subRef.is_valid() : !subRef.is_null())
                {
                    ref = subRef;
                    if (r.length == 0)
                        return ref;

                    foundSubPart = true;
                    break;
                }

                string_split(STRING_ARGS(orChoiceR), STRING_CONST("|"), &subToken, &orChoiceR, false);
            }

            if (!foundSubPart)
                return json_object_t{};

            string_split(STRING_ARGS(r), STRING_CONST("."), &token, &r, false);
        }

        return json_object_t{};
    }

    const json_object_t get(size_t index) const
    {
        if (root == nullptr || root->child == 0)
            return json_object_t{};

        const json_token_t* c = &tokens[root->child];
        while (index != 0)
        {
            index--;
            if (c->sibling == 0)
                break;
            c = &tokens[c->sibling];
        }
        return json_object_t(*this, c);
    }

    FOUNDATION_FORCEINLINE const json_object_t get(const char* field_name, size_t field_name_size) const
    {
        const json_token_t* field_token = json_find_token(*this, field_name, field_name_size);
        if (field_token == nullptr)
            return json_object_t{};
        return json_object_t(*this, field_token);
    }

    FOUNDATION_FORCEINLINE const json_object_t operator[](size_t index) const
    {
        return get(index);
    }

    FOUNDATION_FORCEINLINE const json_object_t operator[](const char* field_name) const
    {
        return get(field_name, string_length(field_name));
    }

    FOUNDATION_FORCEINLINE const json_object_t operator[](string_const_t field_name) const
    {
        return get(STRING_ARGS(field_name));
    }

    FOUNDATION_FORCEINLINE double as_number(double default_value = DNAN) const
    {
        return json_read_number(buffer, tokens, root, default_value);
    }

    FOUNDATION_FORCEINLINE int as_integer(int default_value = 0) const
    {
        return math_trunc(json_read_number(buffer, tokens, root, (double)default_value));
    }

    FOUNDATION_FORCEINLINE time_t as_time(time_t default_value = 0) const
    {
        if (root->type == JSON_STRING)
        {
            string_const_t value = as_string();
            if (value.length >= 10)
                return string_to_date(value.str, value.length);
        }

        return (time_t)json_read_number(buffer, tokens, root, (double)default_value);
    }

    FOUNDATION_FORCEINLINE double as_boolean(bool default_value = false) const
    {
        if (root == nullptr || buffer == nullptr)
            return default_value;

        if (root->type == JSON_UNDEFINED)
            return default_value;

        if (root->type == JSON_PRIMITIVE)
        {
            string_const_t value = json_token_value(buffer, root);
            if (string_equal(STRING_CONST("true"), STRING_ARGS(value)))
                return true;
            if (string_equal(STRING_CONST("false"), STRING_ARGS(value)))
                return false;
        }

        return default_value;
    }

    string_const_t as_string() const
    {
        if (root == nullptr)
            return string_null();
        string_const_t value = json_token_value(buffer, root);
        if (root->type == JSON_PRIMITIVE && string_equal(STRING_ARGS(value), STRING_CONST("null")))
            return string_null();
        return value;
    }

    FOUNDATION_FORCEINLINE string_t as_string_clone() const
    {
        string_const_t value = as_string();
        return string_clone(STRING_ARGS(value));
    }

    FOUNDATION_FORCEINLINE bool operator!=(const json_object_t& other) const
    {
        return other.root != this->root;
    }

    FOUNDATION_FORCEINLINE bool operator==(const json_object_t& other) const
    {
        return !operator!=(other);
    }

    json_object_t& operator++()
    {
        if (root->sibling == 0)
        {
            this->root = nullptr;
            this->error_code = LONG_MAX;
            this->status_code = LONG_MAX;
        }
        else
        {
            this->root = &tokens[root->sibling];
        }

        return *this;
    }

    FOUNDATION_FORCEINLINE const json_object_t& operator*() const
    {
        return *this;
    }

    FOUNDATION_FORCEINLINE json_object_t begin(size_t index = 0) const
    {
        if (this->root == nullptr)
            return end();
        return get(index);
    }

    FOUNDATION_FORCEINLINE json_object_t end() const
    {
        return json_object_t{};
    }
};

json_object_t json_parse(const string_t& str);

const json_token_t* json_find_token(const char* json, const json_token_t* tokens, const json_token_t& root, const char* key, size_t key_length = 0);

double json_read_number(const char* json, const json_token_t* tokens, const json_token_t& value, double default_value = NAN);
double json_read_number(const json_object_t& json, const char* field_name, size_t field_name_length);
double json_read_number(const json_object_t& json, const json_token_t* value, const char* field_name, size_t field_name_length);

double json_read_number(const char* json, const json_token_t* tokens, const json_token_t& obj, const char* key, double& out_number);

time_t json_read_time(const char* json, const json_token_t* tokens, const json_token_t& obj, const char* key, time_t& out_number);
