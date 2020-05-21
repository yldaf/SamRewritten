#pragma once

#include "MainPickerWindow.h"

class MainPickerWindowFactory
{
public:
    static MainPickerWindow* create(AppId_t app_id);
};