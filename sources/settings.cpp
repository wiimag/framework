/*
 * License: https://wiimag.com/LICENSE
 * Copyright 2023 Wiimag inc. All rights reserved.
 */

#include "settings.h"

#include <framework/imgui.h>
#include <framework/string.h>
#include <framework/common.h>
#include <framework/session.h>
#include <framework/localization.h>

settings_t SETTINGS{};

constexpr const char SESSION_KEY_CURRENT_TAB[] = "current_tab_1";
constexpr const char SESSION_KEY_SEARCH_TERMS[] = "search_terms";
constexpr const char SESSION_KEY_SEARCH_FILTER[] = "search_filter";

#if BUILD_APPLICATION

void settings_draw()
{
    ImGui::Columns(3, "#Settings", false);

    ImGui::SetColumnWidth(0, IM_SCALEF(220.0f));
    ImGui::SetColumnWidth(1, IM_SCALEF(270.0f));

    #if BUILD_ENABLE_LOCALIZATION
    {
        // Select language
        ImGui::AlignTextToFramePadding();
        ImGui::TrTextUnformatted("Language");

        ImGui::NextColumn();
        string_const_t current_language_name = localization_current_language_name();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        if (ImGui::BeginCombo("##Language", current_language_name.str))
        {
            for (unsigned i = 0; i < localization_supported_language_count(); ++i)
            {
                string_const_t language_code = localization_language_code(i);
                string_const_t language_name = localization_language_name(i);
                bool is_selected = string_equal(STRING_ARGS(current_language_name), STRING_ARGS(language_code));
                if (ImGui::Selectable(language_name.str, is_selected))
                {
                    localization_set_current_language(STRING_ARGS(language_code));
                }
                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        ImGui::NextColumn();
    }
    #endif

    // Font scaling
    {
        #if BUILD_ENABLE_LOCALIZATION
        ImGui::NextColumn();
        #endif

        static bool restart_to_apply_effect = false;
        ImGui::AlignTextToFramePadding();
        ImGui::TrTextWrapped("Font scaling");

        ImGui::NextColumn();
        ImGui::ExpandNextItem();
        float font_scaling = session_get_float("font_scaling", 1.0f);
        if (ImGui::InputFloat("##FontScaling", &font_scaling, 0.25, 0, "%.2lf", ImGuiInputTextFlags_AutoSelectAll))
        {
            restart_to_apply_effect = true;
            imgui_set_font_ui_scale(font_scaling);
        }

        ImGui::NextColumn();
        if (restart_to_apply_effect)
        {
            ImGui::AlignTextToFramePadding();
            ImGui::TextColored(ImColor(TEXT_WARN_COLOR), tr("Changing that setting requires restarting the application."));
        }
    }

    // Frame throttling
    {
        ImGui::NextColumn();
        ImGui::AlignTextToFramePadding();
        ImGui::TrTextWrapped("Frame Throttling");

        int frame_throttling = session_get_integer("frame_throttling", 16);

        ImGui::NextColumn();
        ImGui::ExpandNextItem();
        if (ImGui::SliderInt("##frame_throttling", &frame_throttling, 0, 1000, tr("%d milliseconds"), ImGuiSliderFlags_AlwaysClamp))
            session_set_integer("frame_throttling", frame_throttling);

        ImGui::NextColumn();
        ImGui::AlignTextToFramePadding();
        ImGui::TextWrapped(tr("Time to wait before rendering another frame (ms).\nThe higher the number, less resources are used, therefore more battery time!"));
    }
}

#endif 

void settings_initialize()
{
    SETTINGS.current_tab = session_get_integer(SESSION_KEY_CURRENT_TAB, SETTINGS.current_tab);

    // Restore some session settings from the user registry
    string_copy(STRING_BUFFER(SETTINGS.search_terms), STRING_ARGS(session_get_string(SESSION_KEY_SEARCH_TERMS, "")));
    string_copy(STRING_BUFFER(SETTINGS.search_filter), STRING_ARGS(session_get_string(SESSION_KEY_SEARCH_FILTER, "")));
}

void settings_shutdown()
{
    session_set_integer(SESSION_KEY_CURRENT_TAB, SETTINGS.current_tab);
    session_set_string(SESSION_KEY_SEARCH_TERMS, SETTINGS.search_terms);
    session_set_string(SESSION_KEY_SEARCH_FILTER, SETTINGS.search_filter);
}
