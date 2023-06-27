/*
 * License: https://wiimag.com/LICENSE
 * Copyright 2023 Wiimag inc. All rights reserved.
 */
 
#pragma once

#include <foundation/string.h>

struct settings_t
{
    // UI settings
    int current_tab{ 0 };

    // Preferences
    char search_terms[256]{ '\0' };
    char search_filter[256]{ '\0' };
};

void settings_draw();

void settings_initialize();
void settings_shutdown();

extern settings_t SETTINGS;
