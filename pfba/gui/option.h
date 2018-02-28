//
// Created by cpasjuste on 05/12/16.
//

#ifndef _OPTION_H_
#define _OPTION_H_

#include <string>
#include <vector>

#define BIT(n) (1U<<(n))

class Option {

public:

    enum Type {
        MENU =      BIT(0),
        INTEGER =   BIT(1),
        INPUT =     BIT(2),
        HIDDEN =    BIT(3)
    };

    enum Index {
        MENU_MAIN = 0,
        GUI_SHOW_ALL,
        GUI_SHOW_CLONES,
        GUI_SHOW_HARDWARE,
        GUI_FULLSCREEN,
        MENU_SKIN,
        SKIN_FONT_SIZE,
        MENU_ROM_OPTIONS,
        ROM_SCALING,
        ROM_FILTER,
        ROM_SHADER,
        ROM_ROTATION,
        ROM_SHOW_FPS,
        //ROM_M68K,
        ROM_FRAMESKIP,
        ROM_NEOBIOS,
        ROM_AUDIO,
        MENU_JOYPAD,
        JOY_UP,
        JOY_DOWN,
        JOY_LEFT,
        JOY_RIGHT,
        JOY_FIRE1,
        JOY_FIRE2,
        JOY_FIRE3,
        JOY_FIRE4,
        JOY_FIRE5,
        JOY_FIRE6,
        JOY_COIN1,
        JOY_START1,
        JOY_MENU1,
        JOY_MENU2,
        JOY_AXIS_LX,
        JOY_AXIS_LY,
        JOY_AXIS_RX,
        JOY_AXIS_RY,
        JOY_DEADZONE,
#ifndef NO_KEYBOARD
        MENU_KEYBOARD,
        KEY_UP,
        KEY_DOWN,
        KEY_LEFT,
        KEY_RIGHT,
        KEY_FIRE1,
        KEY_FIRE2,
        KEY_FIRE3,
        KEY_FIRE4,
        KEY_FIRE5,
        KEY_FIRE6,
        KEY_COIN1,
        KEY_START1,
        KEY_MENU1,
        KEY_MENU2,
#endif
        END
    };

    Option(const std::string &text, const std::vector<std::string> &options,
           int defaultValue, Index idx, int flags = INTEGER);

    int flags = INTEGER;
    int index = 0;
    int value = 0;

    const char *getName();

    const char *getValue();

    void next();

    void prev();

private:
    std::string text;
    std::vector<std::string> options;
};

#endif //_OPTION_H_
