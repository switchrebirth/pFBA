/*
 * FinalBurn Alpha
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include "c2dui.h"
#include "gui.h"
#include "gui_romlist.h

using namespace c2d;

#ifdef __PSP2__
#include <psp2/power.h>
#include <psp2/io/dirent.h>

int _newlib_heap_size_user = 192 * 1024 * 1024;
#define SCR_W   960
#define SCR_H   544
#elif __PS3__
#define SCR_W   1280
#define SCR_H   720
#elif __3DS__
#define SCR_W   400
#define SCR_H   240
#elif __NX__
#define SCR_W   1280
#define SCR_H   720
#else
#define SCR_W   1280
#define SCR_H   720
#endif

Renderer *renderer;
Input *inp;
Io *io;
Config *config;
Gui *ui;
Skin *skin;

/////////
// FBA
/////////
char szAppBurnVer[16] = VERSION;
// replaces ips_manager.cpp
bool bDoIpsPatch = 0;

void IpsApplyPatches(UINT8 *base, char *rom_name) {}

// needed by cps3run.cpp and dataeast/d_backfire.cpp
void Reinitialise() {}

// needed by neo_run.cpp
void wav_exit() {}

int bRunPause;
/////////
// FBA
/////////

int main(int argc, char **argv) {

    BurnPathsInit();
    BurnLibInit();

    // buttons used for ui config menu
    std::vector<Skin::Button> buttons;

#ifdef __PSP2__
    // set max cpu speed
    scePowerSetArmClockFrequency(444);
    scePowerSetBusClockFrequency(222);
    scePowerSetGpuClockFrequency(222);
    scePowerSetGpuXbarClockFrequency(166);
    // see c2d.h for key id
    buttons.emplace_back(KEY_JOY_UP_DEFAULT, "UP");
    buttons.emplace_back(KEY_JOY_DOWN_DEFAULT, "DOWN");
    buttons.emplace_back(KEY_JOY_LEFT_DEFAULT, "LEFT");
    buttons.emplace_back(KEY_JOY_RIGHT_DEFAULT, "RIGHT");
    buttons.emplace_back(KEY_JOY_FIRE1_DEFAULT, "TRIANGLE");
    buttons.emplace_back(KEY_JOY_FIRE2_DEFAULT, "CIRCLE");
    buttons.emplace_back(KEY_JOY_FIRE3_DEFAULT, "CROSS");
    buttons.emplace_back(KEY_JOY_FIRE4_DEFAULT, "SQUARE");
    buttons.emplace_back(KEY_JOY_FIRE5_DEFAULT, "L");
    buttons.emplace_back(KEY_JOY_FIRE6_DEFAULT, "R");
    buttons.emplace_back(KEY_JOY_COIN1_DEFAULT, "SELECT");
    buttons.emplace_back(KEY_JOY_START1_DEFAULT, "START");
#elif __NX__
    // see c2d.h for key id
    buttons.emplace_back(KEY_JOY_UP_DEFAULT, "UP");
    buttons.emplace_back(KEY_JOY_DOWN_DEFAULT, "DOWN");
    buttons.emplace_back(KEY_JOY_LEFT_DEFAULT, "LEFT");
    buttons.emplace_back(KEY_JOY_RIGHT_DEFAULT, "RIGHT");
    buttons.emplace_back(KEY_JOY_FIRE1_DEFAULT, "A");
    buttons.emplace_back(KEY_JOY_FIRE2_DEFAULT, "B");
    buttons.emplace_back(KEY_JOY_FIRE3_DEFAULT, "X");
    buttons.emplace_back(KEY_JOY_FIRE4_DEFAULT, "Y");
    buttons.emplace_back(KEY_JOY_FIRE5_DEFAULT, "L");
    buttons.emplace_back(KEY_JOY_FIRE6_DEFAULT, "R");
    buttons.emplace_back(KEY_JOY_COIN1_DEFAULT, "+");
    buttons.emplace_back(KEY_JOY_START1_DEFAULT, "-");
    // switch special keys
    buttons.emplace_back(KEY_JOY_ZL_DEFAULT, "ZL");
    buttons.emplace_back(KEY_JOY_ZR_DEFAULT, "ZR");
#endif

    renderer = (Renderer *) new C2DRenderer(Vector2f(SCR_W, SCR_H));
    inp = new C2DInput(renderer);
    io = new C2DIo();

    // build default roms path
    std::string homePath = C2DUI_HOME_PATH;
    std::vector<std::string> roms_paths;
    roms_paths.clear();
    roms_paths.emplace_back(homePath + "roms/");
    roms_paths.emplace_back(homePath + "coleco/");
    roms_paths.emplace_back(homePath + "gamegear/");
    roms_paths.emplace_back(homePath + "megadriv/");
    roms_paths.emplace_back(homePath + "msx/");
    roms_paths.emplace_back(homePath + "pce/");
    roms_paths.emplace_back(homePath + "sg1000/");
    roms_paths.emplace_back(homePath + "sgx/");
    roms_paths.emplace_back(homePath + "sms/");
    roms_paths.emplace_back(homePath + "tg16/");
    for (size_t i = roms_paths.size(); i < DIRS_MAX; i++) {
        roms_paths.emplace_back("");
    }

    // build hardwares list
    std::vector<RomList::Hardware> hardwareList;
    hardwareList.emplace_back(HARDWARE_PREFIX_ALL, "All");
    hardwareList.emplace_back(HARDWARE_PREFIX_SNK, "Neo Geo");
    hardwareList.emplace_back(HARDWARE_PREFIX_CAPCOM, "CPS-1");
    hardwareList.emplace_back(HARDWARE_PREFIX_CPS2, "CPS-2");
    hardwareList.emplace_back(HARDWARE_PREFIX_CPS3, "CPS-3");
    hardwareList.emplace_back(HARDWARE_PREFIX_IGS_PGM, "PGM");
    hardwareList.emplace_back(HARDWARE_PREFIX_PSIKYO, "Psikyo");
    hardwareList.emplace_back(HARDWARE_PREFIX_CAVE, "Cave");
    hardwareList.emplace_back(HARDWARE_PREFIX_SEGA, "Sega");
    hardwareList.emplace_back(HARDWARE_PREFIX_CAPCOM_MISC, "Capcom (Other)");
    hardwareList.emplace_back(HARDWARE_PREFIX_DATAEAST, "Data East");
    hardwareList.emplace_back(HARDWARE_PREFIX_GALAXIAN, "Galaxian");
    hardwareList.emplace_back(HARDWARE_PREFIX_IREM, "Irem");
    hardwareList.emplace_back(HARDWARE_PREFIX_KANEKO, "Kaneko");
    hardwareList.emplace_back(HARDWARE_PREFIX_KONAMI, "Konami");
    hardwareList.emplace_back(HARDWARE_PREFIX_MISC_PRE90S, "Misc (pre 90s)");
    hardwareList.emplace_back(HARDWARE_PREFIX_MISC_POST90S, "Misc (post 90s)");
    hardwareList.emplace_back(HARDWARE_PREFIX_PACMAN, "Pacman");
    hardwareList.emplace_back(HARDWARE_PREFIX_SETA, "Seta");
    hardwareList.emplace_back(HARDWARE_PREFIX_TAITO, "Taito");
    hardwareList.emplace_back(HARDWARE_PREFIX_TECHNOS, "Technos");
    hardwareList.emplace_back(HARDWARE_PREFIX_TOAPLAN, "Toaplan");
    hardwareList.emplace_back(HARDWARE_PREFIX_SEGA_MEGADRIVE, "Sega MegaDrive");
    hardwareList.emplace_back(HARDWARE_PREFIX_SEGA_MASTER_SYSTEM, "Sega MasterSystem");
    hardwareList.emplace_back(HARDWARE_PREFIX_SEGA_SG1000, "Sega SG1000");
    hardwareList.emplace_back(HARDWARE_PREFIX_SEGA_GAME_GEAR, "Sega GameGear");
    hardwareList.emplace_back(HARDWARE_PREFIX_PCENGINE, "PC-Engine");
    hardwareList.emplace_back(HARDWARE_PREFIX_COLECO, "Coleco");
    hardwareList.emplace_back(HARDWARE_PREFIX_MIDWAY, "Midway");
    hardwareList.emplace_back(HARDWARE_PREFIX_MSX, "MSX");
    std::vector<std::string> hardware_names;
    for (unsigned int i = 0; i < hardwareList.size(); i++) {
        hardware_names.emplace_back(hardwareList[i].name);
    }

    ////////////////////////////////////////////////////////////
    // options needs to be in sync/order with "Option::Index"
    ////////////////////////////////////////////////////////////
    // main/gui config
    std::vector<Option> options;
    options.emplace_back(Option("MAIN", {"MAIN"}, 0, Option::Index::MENU_MAIN, Option::Type::MENU));
    options.emplace_back(Option("SHOW_ALL", {"WORKING", "ALL"}, 1, Option::Index::GUI_SHOW_ALL));
    options.emplace_back(Option("SHOW_CLONES", {"OFF", "ON"}, 0, Option::Index::GUI_SHOW_CLONES));
    options.emplace_back(Option("SHOW_HARDWARE", hardware_names, 0, Option::Index::GUI_SHOW_HARDWARE));
    options.emplace_back(
            Option("FULLSCREEN", {"OFF", "ON"}, 1, Option::Index::GUI_FULLSCREEN, Option::Type::HIDDEN));

    // skin config, hidden in gui for now
    options.emplace_back(
            Option("SKIN", {"SKIN"}, 0, Option::Index::MENU_SKIN, Option::Type::MENU | Option::Type::HIDDEN));
    options.emplace_back(
            Option("SKIN_FONT_SIZE", {"20"}, 20, Option::Index::SKIN_FONT_SIZE, Option::Type::HIDDEN));
    // default rom config
    options.emplace_back(
            Option("EMULATION", {"EMULATION"}, 0, Option::Index::MENU_ROM_OPTIONS, Option::Type::MENU));
    options.emplace_back(
            Option("SCALING", {"NONE", "2X", "3X", "FIT", "FIT 4:3", "FULL"},
                   3, Option::Index::ROM_SCALING));
#ifdef __NX__
    options.emplace_back(
            Option("FILTER", {"POINT", "LINEAR"}, 1, Option::Index::ROM_FILTER));
#else
    options.emplace_back(
            Option("FILTER", {"POINT", "LINEAR"}, 0, Option::Index::ROM_FILTER));
#endif
    if (renderer->getShaderList() != NULL) {
        options.emplace_back(
                Option("EFFECT", renderer->getShaderList()->getNames(), 0, Option::Index::ROM_SHADER));
    }
#ifdef __PSP2__
    options.emplace_back(
            Option("ROTATION", {"OFF", "ON", "FLIP", "CAB MODE"}, 0, Option::Index::ROM_ROTATION));
#else
    options.emplace_back(
            Option("ROTATION", {"OFF", "ON", "FLIP"}, 0, Option::Index::ROM_ROTATION));
#endif
    options.emplace_back(Option("SHOW_FPS", {"OFF", "ON"}, 0, Option::Index::ROM_SHOW_FPS));
    options.emplace_back(Option("FRAMESKIP", {"OFF", "1", "2", "3", "4", "5", "6", "7", "8", "9"},
                                0, Option::Index::ROM_FRAMESKIP, Option::Type::INTEGER));
    //options.emplace_back(Option("M68K", {"ASM", "C"}, 0, Option::Index::ROM_M68K));
    options.emplace_back(Option("NEOBIOS", {"UNIBIOS_3_2", "AES_ASIA", "AES_JPN", "DEVKIT", "MVS_ASIA_EUR_V6S1",
                                            "MVS_ASIA_EUR_V5S1", "MVS_ASIA_EUR_V3S4", "MVS_USA_V5S2",
                                            "MVS_USA_V5S4", "MVS_USA_V5S6", "MVS_JPN_V6", "MVS_JPN_V5",
                                            "MVS_JPN_V3S4", "NEO_MVH_MV1C", "MVS_JPN_J3", "DECK_V6"},
                                0, Option::Index::ROM_NEOBIOS));
    options.emplace_back(Option("AUDIO", {"OFF", "ON"}, 1, Option::Index::ROM_AUDIO));

    // joystick
    options.emplace_back(Option("JOYPAD", {"JOYPAD"}, 0, Option::Index::MENU_JOYPAD, Option::Type::MENU));
    options.emplace_back(
            Option("JOY_UP", {"-1"}, KEY_JOY_UP_DEFAULT, Option::Index::JOY_UP, Option::Type::INPUT));
    options.emplace_back(
            Option("JOY_DOWN", {"-1"}, KEY_JOY_DOWN_DEFAULT, Option::Index::JOY_DOWN, Option::Type::INPUT));
    options.emplace_back(
            Option("JOY_LEFT", {"-1"}, KEY_JOY_LEFT_DEFAULT, Option::Index::JOY_LEFT, Option::Type::INPUT));
    options.emplace_back(
            Option("JOY_RIGHT", {"-1"}, KEY_JOY_RIGHT_DEFAULT, Option::Index::JOY_RIGHT, Option::Type::INPUT));
    options.emplace_back(
            Option("JOY_FIRE1", {"0"}, KEY_JOY_FIRE1_DEFAULT, Option::Index::JOY_FIRE1, Option::Type::INPUT));
    options.emplace_back(
            Option("JOY_FIRE2", {"1"}, KEY_JOY_FIRE2_DEFAULT, Option::Index::JOY_FIRE2, Option::Type::INPUT));
    options.emplace_back(
            Option("JOY_FIRE3", {"2"}, KEY_JOY_FIRE3_DEFAULT, Option::Index::JOY_FIRE3, Option::Type::INPUT));
    options.emplace_back(
            Option("JOY_FIRE4", {"3"}, KEY_JOY_FIRE4_DEFAULT, Option::Index::JOY_FIRE4, Option::Type::INPUT));
    options.emplace_back(
            Option("JOY_FIRE5", {"4"}, KEY_JOY_FIRE5_DEFAULT, Option::Index::JOY_FIRE5, Option::Type::INPUT));
    options.emplace_back(
            Option("JOY_FIRE6", {"5"}, KEY_JOY_FIRE6_DEFAULT, Option::Index::JOY_FIRE6, Option::Type::INPUT));
    options.emplace_back(
            Option("JOY_COIN1", {"6"}, KEY_JOY_COIN1_DEFAULT, Option::Index::JOY_COIN1, Option::Type::INPUT));
    options.emplace_back(
            Option("JOY_START1", {"7"}, KEY_JOY_START1_DEFAULT, Option::Index::JOY_START1, Option::Type::INPUT));
    //options.emplace_back(
    //        Option("JOY_MENU1", {"6"}, KEY_JOY_MENU1_DEFAULT, Option::Index::JOY_MENU1, Option::Type::INPUT));
    //options.emplace_back(
    //        Option("JOY_MENU2", {"7"}, KEY_JOY_MENU2_DEFAULT, Option::Index::JOY_MENU2, Option::Type::INPUT));
    // TODO: add gui option for axis in option menu
    options.emplace_back(
            Option("JOY_AXIS_LX", {"0"}, KEY_JOY_AXIS_LX, Option::Index::JOY_AXIS_LX, Option::Type::HIDDEN));
    options.emplace_back(
            Option("JOY_AXIS_LY", {"1"}, KEY_JOY_AXIS_LY, Option::Index::JOY_AXIS_LY, Option::Type::HIDDEN));
    options.emplace_back(
            Option("JOY_AXIS_RX", {"2"}, KEY_JOY_AXIS_RX, Option::Index::JOY_AXIS_RX, Option::Type::HIDDEN));
    options.emplace_back(
            Option("JOY_AXIS_RY", {"3"}, KEY_JOY_AXIS_RY, Option::Index::JOY_AXIS_RY, Option::Type::HIDDEN));
    options.emplace_back(Option("JOY_DEADZONE",
                                {"2000", "4000", "6000", "8000", "10000", "12000", "14000", "16000",
                                 "18000", "20000", "22000", "24000", "26000", "28000", "30000"}, 3,
                                Option::Index::JOY_DEADZONE, Option::Type::INTEGER));

#ifndef NO_KEYBOARD
    // keyboard
    options.emplace_back(Option("KEYBOARD", {"KEYBOARD"}, 0, Option::Index::MENU_KEYBOARD, Option::Type::MENU));
    options.emplace_back(Option("KEY_UP", {std::to_string(KEY_KB_UP_DEFAULT)}, KEY_KB_UP_DEFAULT,
                                Option::Index::KEY_UP, Option::Type::INPUT));        // KP_UP
    options.emplace_back(Option("KEY_DOWN", {std::to_string(KEY_KB_DOWN_DEFAULT)}, KEY_KB_DOWN_DEFAULT,
                                Option::Index::KEY_DOWN, Option::Type::INPUT));    // KP_DOWN
    options.emplace_back(Option("KEY_LEFT", {std::to_string(KEY_KB_LEFT_DEFAULT)}, KEY_KB_LEFT_DEFAULT,
                                Option::Index::KEY_LEFT, Option::Type::INPUT));    // KP_LEFT
    options.emplace_back(Option("KEY_RIGHT", {std::to_string(KEY_KB_RIGHT_DEFAULT)}, KEY_KB_RIGHT_DEFAULT,
                                Option::Index::KEY_RIGHT, Option::Type::INPUT));  // KP_RIGHT
    options.emplace_back(Option("KEY_FIRE1", {std::to_string(KEY_KB_FIRE1_DEFAULT)}, KEY_KB_FIRE1_DEFAULT,
                                Option::Index::KEY_FIRE1, Option::Type::INPUT));  // KP_1
    options.emplace_back(Option("KEY_FIRE2", {std::to_string(KEY_KB_FIRE2_DEFAULT)}, KEY_KB_FIRE2_DEFAULT,
                                Option::Index::KEY_FIRE2, Option::Type::INPUT));  // KP_2
    options.emplace_back(Option("KEY_FIRE3", {std::to_string(KEY_KB_FIRE3_DEFAULT)}, KEY_KB_FIRE3_DEFAULT,
                                Option::Index::KEY_FIRE3, Option::Type::INPUT));  // KP_3
    options.emplace_back(Option("KEY_FIRE4", {std::to_string(KEY_KB_FIRE4_DEFAULT)}, KEY_KB_FIRE4_DEFAULT,
                                Option::Index::KEY_FIRE4, Option::Type::INPUT));  // KP_4
    options.emplace_back(Option("KEY_FIRE5", {std::to_string(KEY_KB_FIRE5_DEFAULT)}, KEY_KB_FIRE5_DEFAULT,
                                Option::Index::KEY_FIRE5, Option::Type::INPUT));  // KP_5
    options.emplace_back(Option("KEY_FIRE6", {std::to_string(KEY_KB_FIRE6_DEFAULT)}, KEY_KB_FIRE6_DEFAULT,
                                Option::Index::KEY_FIRE6, Option::Type::INPUT));  // KP_6
    options.emplace_back(Option("KEY_COIN1", {std::to_string(KEY_KB_COIN1_DEFAULT)}, KEY_KB_COIN1_DEFAULT,
                                Option::Index::KEY_COIN1, Option::Type::INPUT));  // ESCAPE
    options.emplace_back(Option("KEY_START1", {std::to_string(KEY_KB_START1_DEFAULT)}, KEY_KB_START1_DEFAULT,
                                Option::Index::KEY_START1, Option::Type::INPUT));// ENTER
    //options.emplace_back(Option("KEY_MENU1", {std::to_string(KEY_KB_MENU1_DEFAULT)}, KEY_KB_MENU1_DEFAULT,
    //                                Option::Index::KEY_MENU1, Option::Type::INPUT));
    //options.emplace_back(Option("KEY_MENU2", {std::to_string(KEY_KB_MENU2_DEFAULT)}, KEY_KB_MENU2_DEFAULT,
    //                                Option::Index::KEY_MENU2, Option::Type::INPUT));
#endif

    //
    options.emplace_back(Option("END", {"END"}, 0, Option::Index::END, Option::Type::MENU));


    // load configuration
    config = new Config(C2DUI_HOME_PATH, roms_paths, options, hardwareList);

    // skin
    Skin *skin = new Skin(C2DUI_HOME_PATH, buttons);

    // audio
    Audio *audio = new C2DAudio(48000);
    audio->pause(1);

    // run gui
    ui = new Gui(io, renderer, skin, config, inp, audio);
    ui->run();

    // quit
    BurnLibExit();

    delete (ui);
    delete (audio);
    delete (skin);
    delete (config);
    delete (io);
    delete (inp);
    delete (renderer);

#ifdef __PSP2__
    scePowerSetArmClockFrequency(266);
    scePowerSetBusClockFrequency(166);
    scePowerSetGpuClockFrequency(166);
    scePowerSetGpuXbarClockFrequency(111);
#endif

    return 0;
}
