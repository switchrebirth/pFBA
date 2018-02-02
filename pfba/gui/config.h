//
// Created by cpasjuste on 05/12/16.
//

#ifndef _CONFIG_H_
#define _CONFIG_H_

#include "libconfig.h"

#include "gui.h"

class Gui;

#include "option.h"

class Config {

public:

    Config(const std::string &cfgPath, c2d::Renderer *renderer);

    ~Config() {};

    void load(RomList::Rom *rom = NULL);

    void save(RomList::Rom *rom = NULL);

    int getValue(int id, bool rom = false);

    const char *getRomPath(int n);

    std::vector<std::string> getRomPaths();

    std::vector<Option> *getOptions(bool rom = false);

    Option *getOption(std::vector<Option> *options, int index);

    int getOptionPos(std::vector<Option> *options, int index);

    int *getGuiPlayerInputKeys(int player);

    int *getGuiPlayerInputButtons(int player);

    int *getRomPlayerInputKeys(int player);

    int *getRomPlayerInputButtons(int player);

    std::vector<RomList::Hardware> hardwareList;

private:
    std::vector<std::string> roms_paths;
    std::vector<Option> options_gui;
    std::vector<Option> options_rom;
    std::string configPath;
    bool done = false;

    int keyboard_keys[KEY_COUNT];
    int joystick_keys[KEY_COUNT];
};

#endif //_CONFIG_H_
