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
     * Unlock the achievement represented by this row if it is locked
     */
    void unlock();
    /**
     * Lock the achievement represented by this row if it is unlocked
     */
    void lock();
    /**
     * Invert (or toggle) the achievement represented by this row
     */
    void invert();

    Achievement_t get_achievement() { return m_data; };
private:
    bool m_active;
    Achievement_t m_data;
    Gtk::ToggleButton m_lock_unlock_button;
};