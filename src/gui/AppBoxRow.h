#pragma once

#include "ListBoxRowWithIcon.h"
#include "../types/Game.h"

/**
 * This class represents a game/app entry on the main game picker menu
 */
class AppBoxRow : public ListBoxRowWithIcon
{
public:
    AppBoxRow(const Game_t& app);
    virtual ~AppBoxRow();

    Game_t get_app() const;

private:
    void launch_new_window();
    Game_t m_app;

protected:

    /**
     * This is used by InputAppidBoxRow for example.
     */
    AppBoxRow();
};
