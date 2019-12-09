#pragma once

#include "../types/Achievement.h"

#include "ListBoxRowWithIcon.h"
#include <gtkmm-3.0/gtkmm/togglebutton.h>

/**
 * This class represents an achievement entry on the achievements view
 */
class AchievementBoxRow : public ListBoxRowWithIcon {
public:
    AchievementBoxRow(const Achievement_t& data);
    virtual ~AchievementBoxRow();

    /**
     * Invert (or toggle) the achievement represented by this row
     */
    void invert();

    /**
     * While invert is triggered by mouse clicks, these 
     * will are programmatically called and will trigger the click event
     * if necessary
     */
    void invert_scripted();
    void lock_scripted();
    void unlock_scripted();

    Achievement_t get_achievement() { return m_data; };
private:
    bool m_active;
    Achievement_t m_data;
    Gtk::ToggleButton m_lock_unlock_button;
};