/*
 * Copyright 2022-2023 - All rights reserved.
 * License: https://wiimag.com/LICENSE
 */

#include "session.h"

#include <framework/imgui.h>
#include <framework/common.h>
#include <framework/config.h>
#include <framework/jobs.h>
#include <framework/string.h>
#include <framework/array.h>
#include <framework/scoped_string.h>
#include <framework/profiler.h>

#include <foundation/environment.h>
#include <foundation/path.h>
#include <foundation/assert.h>
#include <foundation/fs.h>

static const char IMGUI_FILE_NAME[] = "imgui.ini";

static config_handle_t _session_config{};
static string_t _session_working_dir{};

struct session_backup_info_t
{
    char session_dir_buffer[BUILD_MAX_PATHLEN];
    char backup_folder_path_buffer[BUILD_MAX_PATHLEN];
};

//
// # PRIVATE
//

FOUNDATION_STATIC void session_cleanup()
{
    config_deallocate(_session_config);
    string_deallocate(_session_working_dir.str);
}

FOUNDATION_STATIC void session_load_config()
{
    if (_session_config)
        return;
    
    if (main_is_interactive_mode())
    {
        string_const_t session_file_path = session_get_file_path();
        _session_config = config_parse_file(STRING_ARGS(session_file_path), CONFIG_OPTION_PRESERVE_INSERTION_ORDER);
    }

    if (!_session_config)
        _session_config = config_allocate();
}

FOUNDATION_STATIC void session_backup(const char* _session_dir, size_t _session_dir_length)
{
    session_backup_info_t backup_info;
    
    string_t session_dir = string_copy(STRING_BUFFER(backup_info.session_dir_buffer), _session_dir, _session_dir_length);

    // Generate folder backup name
    string_const_t root_dir = path_directory_name(STRING_ARGS(session_dir));
    string_const_t root_folder_name = path_base_file_name(STRING_ARGS(session_dir));

    string_const_t today_date_string = string_from_date(time_now());

    char backup_folder_name_buffer[BUILD_MAX_PATHLEN];
    string_t backup_folder_name = string_copy(STRING_BUFFER(backup_folder_name_buffer), STRING_ARGS(root_folder_name));
    backup_folder_name = string_concat(STRING_BUFFER(backup_folder_name_buffer), STRING_ARGS(backup_folder_name), STRING_CONST("_"));
    backup_folder_name = string_concat(STRING_BUFFER(backup_folder_name_buffer), STRING_ARGS(backup_folder_name), STRING_ARGS(today_date_string));
    
    string_t backup_folder_path = string_copy(STRING_BUFFER(backup_info.backup_folder_path_buffer), STRING_ARGS(root_dir));
    backup_folder_path = path_append(STRING_ARGS(backup_folder_path), sizeof(backup_info.backup_folder_path_buffer), STRING_CONST("backups"));
    backup_folder_path = path_concat(STRING_BUFFER(backup_info.backup_folder_path_buffer), STRING_ARGS(backup_folder_path), STRING_ARGS(backup_folder_name));

    if (fs_is_directory(STRING_ARGS(backup_folder_path)))
        return; // Backup already did for today.

    job_execute([](void* payload)
    {
        session_backup_info_t* backup_info = (session_backup_info_t*)payload;
        
        string_const_t session_dir = string_to_const(backup_info->session_dir_buffer);
        string_const_t backup_folder_path = string_to_const(backup_info->backup_folder_path_buffer);

        TIME_TRACKER("Creating backup of session folder %.*s", STRING_FORMAT(session_dir));

        // Copy all files from session dir to backup folder
        bool success = true;
        string_t* filenames = fs_matching_files(STRING_ARGS(session_dir), STRING_CONST("*.*"), true);
        foreach(f, filenames)
        {
            char source_path_buffer[BUILD_MAX_PATHLEN];
            string_t source_file_path = path_concat(STRING_BUFFER(source_path_buffer), STRING_ARGS(session_dir), STRING_ARGS(*f));

            // Skip filename starting with cache/
            if (string_starts_with(STRING_ARGS(*f), STRING_CONST("cache/")))
                continue;

            if (string_starts_with(STRING_ARGS(*f), STRING_CONST("thumbnails/")))
                continue;

            if (string_starts_with(STRING_ARGS(*f), STRING_CONST("shots/")))
                continue;

            char dest_path_buffer[BUILD_MAX_PATHLEN];
            string_t backup_file_path = path_concat(STRING_BUFFER(dest_path_buffer), STRING_ARGS(backup_folder_path), STRING_ARGS(*f));
            string_const_t backup_file_path_dir = path_directory_name(STRING_ARGS(backup_file_path));

            if (!fs_is_directory(STRING_ARGS(backup_file_path_dir)) && !fs_make_directory(STRING_ARGS(backup_file_path_dir)))
            {
                log_errorf(0, ERROR_ACCESS_DENIED, STRING_CONST("Failed to create directory `%.*s`"), STRING_FORMAT(backup_file_path_dir));
                success = false;
                break;
            }

            if (!fs_copy_file(STRING_ARGS(source_file_path), STRING_ARGS(backup_file_path)))
            {
                log_errorf(0, ERROR_ACCESS_DENIED, STRING_CONST("Failed to copy source file `%.*s` > `%.*s`"),
                    STRING_FORMAT(source_file_path), STRING_FORMAT(backup_file_path));
                success = false;
                break;
            }
        }
        string_array_deallocate(filenames);
            
        return success ? 0 : -1;
    }, & backup_info, sizeof(backup_info), JOB_DEALLOCATE_AFTER_EXECUTION);
}

//
// # PUBLIC API
//

void session_setup(const char* root_path)
{
    // Setup can be called multiple times, so cleaning up first.
    session_cleanup();
    
    _session_working_dir = string_clone_string(environment_current_working_directory());

    // Make sure user dir is created
    string_const_t user_dir = session_get_user_dir();

    // Load config// Attempt to backup the active session if not running tests.
    if (!main_is_running_tests() && fs_is_directory(STRING_ARGS(user_dir)))
        session_backup(STRING_ARGS(user_dir));
        
    if (main_is_graphical_mode() && fs_make_directory(STRING_ARGS(user_dir)))
        ImGui::LoadIniSettingsFromDisk(session_get_user_file_path(STRING_CONST(IMGUI_FILE_NAME)).str);
    
    session_load_config();
}

void session_shutdown()
{
    session_save();
    session_cleanup();
}

string_const_t session_get_file_path()
{
   return session_get_user_file_path(STRING_CONST("session.json"));
}

const char* session_working_dir()
{
    return _session_working_dir.str;
}

void session_save()
{
    if (main_is_running_tests())
        return;

    string_const_t user_dir = session_get_user_dir();
    if (!fs_is_directory(STRING_ARGS(user_dir)))
        return;

    if (main_is_graphical_mode())
        ImGui::SaveIniSettingsToDisk(session_get_user_file_path(STRING_CONST(IMGUI_FILE_NAME)).str);
    
    config_write_file(session_get_file_path(), _session_config,
          CONFIG_OPTION_WRITE_SKIP_FIRST_BRACKETS |
          CONFIG_OPTION_PRESERVE_INSERTION_ORDER |
          CONFIG_OPTION_WRITE_NO_SAVE_ON_DATA_EQUAL);
    
}

string_const_t session_get_user_dir()
{
    static thread_local char user_dir_buffer[BUILD_MAX_PATHLEN] = { '\0' };
    if (user_dir_buffer[0] != 0)
        return string_const(user_dir_buffer, string_length(user_dir_buffer));

    string_const_t app_dir = environment_application_directory();

    string_t user_dir = string_copy(STRING_BUFFER(user_dir_buffer), STRING_ARGS(app_dir));

    if (main_is_running_tests())
        user_dir = string_concat(STRING_BUFFER(user_dir_buffer), STRING_ARGS(user_dir), STRING_CONST("_tests"));
    
    string_const_t profile_name{};
    if (environment_argument("session", &profile_name, false) && profile_name.length > 0)
    {
        user_dir = string_concat(STRING_BUFFER(user_dir_buffer), STRING_ARGS(user_dir), STRING_CONST("_"));
        user_dir = string_concat(STRING_BUFFER(user_dir_buffer), STRING_ARGS(user_dir), STRING_ARGS(profile_name));
    }

    user_dir = path_clean(STRING_ARGS(user_dir), BUILD_MAX_PATHLEN);

    return string_to_const(user_dir);
}

string_const_t session_get_user_dir(STRING_PARAM(subdir))
{
    static thread_local char user_subdir_buffer[BUILD_MAX_PATHLEN] = { '\0' };

    string_const_t user_dir = session_get_user_dir();
    string_t user_subdir = string_copy(STRING_BUFFER(user_subdir_buffer), STRING_ARGS(user_dir));
    user_subdir = path_append(STRING_ARGS(user_subdir), BUILD_MAX_PATHLEN, subdir, subdir_length);
    user_subdir = path_clean(STRING_ARGS(user_subdir), BUILD_MAX_PATHLEN);
    return string_to_const(user_subdir);
}

string_t session_get_user_file_path(
    char* buffer, size_t capacity,
    const char* filename, size_t length,
    const char* prefix, size_t prefix_length,
    const char* extension, size_t extension_length,
    bool mkdir)
{
    string_const_t user_dir = session_get_user_dir();
    string_t user_file_path = string_copy(buffer, capacity, STRING_ARGS(user_dir));
    if (prefix != nullptr && prefix_length > 0)
    {
        user_file_path = path_concat(
            buffer, capacity,
            STRING_ARGS(user_file_path),
            prefix, prefix_length);
    }

    user_file_path = path_concat(
        buffer, capacity,
        STRING_ARGS(user_file_path),
        filename, length);

    if (extension != nullptr && extension_length > 0)
    {
        user_file_path = string_concat(buffer, capacity, STRING_ARGS(user_file_path), STRING_CONST("."));
        user_file_path = string_concat(buffer, capacity, STRING_ARGS(user_file_path), extension, extension_length);
    }

    if (mkdir)
    {
        string_const_t dir_path = path_directory_name(STRING_ARGS(user_file_path));
        fs_make_directory(STRING_ARGS(dir_path));
    }

    return user_file_path;
}

string_const_t session_get_user_file_path(const char* filename, size_t length, const char* prefix, size_t prefix_length, const char* extension, size_t extension_length, bool mkdir /*= false*/)
{
    string_t file_path = session_get_user_file_path(SHARED_BUFFER(BUILD_MAX_PATHLEN), filename, length, prefix, prefix_length, extension, extension_length, mkdir);
    return string_to_const(file_path);
}

bool session_key_exists(const char* keyname)
{
    session_load_config();
    return config_exists(_session_config, keyname, string_length(keyname));
}

bool session_get_bool(const char* keyname, bool default_value /*= false*/)
{
    session_load_config();
    return session_get_integer(keyname, default_value ? 1 : 0);
}

int session_get_integer(const char* keyname, int default_value /*= 0*/)
{
    session_load_config();
    return math_trunc(_session_config[keyname].as_number((double)default_value));
}

float session_get_float(const char* keyname, float default_value /*= 0.0f*/)
{
    session_load_config();
    return float(_session_config[keyname].as_number((double)default_value));
}

string_const_t session_get_string(const char* keyname, const char* default_value /*= NULL*/)
{
    session_load_config();
    return _session_config[keyname].as_string(default_value, string_length(default_value));
}

string_t session_get_string(const char* keyname, char* buf, size_t size, const char* default_value /*= nullptr*/)
{
    string_const_t str_value = session_get_string(keyname, default_value);
    string_t str = string_copy(buf, size, STRING_ARGS(str_value));
    return str;
}

bool session_set_string_list(const char* keyname, string_t* strings)
{
    config_handle_t list = config_set_array(_session_config, keyname, string_length(keyname));

    // Clear the array in case
    config_array_clear(list);

    for (unsigned i = 0, n = array_size(strings); i < n; ++i)
        config_array_push(list, strings[i].str, strings[i].length);

    return config_size(list) > 0;
}

string_t* session_get_string_list(const char* keyname)
{
    string_t* strings = nullptr;
    config_handle_t list = _session_config[keyname];

    if (config_value_type(list) == CONFIG_VALUE_ARRAY)
    {
        for (auto e : list)
        {
            string_const_t str = e.as_string();
            if (str.length > 0)
                array_push(strings, string_clone(STRING_ARGS(str)));
        }
    }

    return strings;
}

bool session_set_bool(const char* keyname, bool value)
{
    return session_set_integer(keyname, value ? 1 : 0);
}

bool session_set_integer(const char* keyname, int value)
{
    return config_set(_session_config, keyname, string_length(keyname), (double)value);
}

bool session_set_float(const char* keyname, float value)
{
    return config_set(_session_config, keyname, string_length(keyname), (double)value);
}

bool session_set_string(const char* keyname, const char* value, size_t value_length)
{
    return config_set(_session_config, keyname, string_length(keyname), value, value_length > 0 ? value_length : string_length(value));
}

bool session_clear_value(const char* keyname)
{
    return config_remove(_session_config, keyname, string_length(keyname));
}

bool session_clear_all_values()
{
    config_deallocate(_session_config);
    _session_config = config_allocate();
    return _session_config;
}
