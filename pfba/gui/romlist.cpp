//
// Created by cpasjuste on 19/10/16.
//

#include <vector>
#include <string>
#include <algorithm>
#include <skeleton/io.h>

#include "burner.h"
#include "romlist.h"
#include "gui.h"

using namespace c2d;

static inline bool endsWith(std::string const &value, std::string const &ending) {
    if (ending.size() > value.size()) return false;
    return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

RomList::RomList(Gui *gui) {

    printf("RomList\n");

    hardwareList = &gui->getConfig()->hardwareList;
    std::vector<std::string> paths = gui->getConfig()->getRomPaths();

    // UI
    Rectangle *rect = new C2DRectangle(
            Vector2f(gui->getRenderer()->getSize().x - 8, gui->getRenderer()->getSize().y - 8));
    rect->setPosition(4, 4);
    rect->setFillColor(Color::Gray);
    rect->setOutlineColor(Color::Orange);
    rect->setOutlineThickness(4);

    Texture *texture = gui->getSkin()->tex_title;
    texture->setOriginCenter();
    texture->setPosition(Vector2f(rect->getSize().x / 2, rect->getSize().y / 2));
    float scaling = std::min(
            (rect->getSize().x - 64) / texture->getSize().x,
            (rect->getSize().y - 64) / texture->getSize().y);
    texture->setScale(scaling, scaling);
    rect->add(texture);

    char text_str[512] = "Roms found: 0/0";
    Text *text = new Text(text_str, *gui->getSkin()->font);
    text->setOriginBottomLeft();
    text->setOutlineColor(Color::Black);
    text->setOutlineThickness(2);
    text->setPosition(16, rect->getSize().y - 8);
    rect->add(text);

    std::string v = "fba: ";
    v += szAppBurnVer;
    Text *version = new Text(v, *gui->getSkin()->font);
    version->setOriginBottomRight();
    version->setOutlineColor(Color::Black);
    version->setOutlineThickness(2);
    version->setPosition(rect->getSize().x - 8, rect->getSize().y - 8);
    rect->add(version);

    gui->getRenderer()->add(rect);
    // UI

    printf("RomList: building list...\n");
    float time_start = gui->getRenderer()->getElapsedTime().asSeconds();

    std::vector<std::string> files[DIRS_MAX];
    for (unsigned int i = 0; i < paths.size(); i++) {
        if (!paths[i].empty()) {
            files[i] = gui->getIo()->getDirList(paths[i].c_str());
            //printf("RomList: found %i files in `%s`\n", (int) files[i].size(), paths[i].c_str());
        }
    }

    char path[MAX_PATH];
    char pathUppercase[MAX_PATH]; // sometimes on FAT32 short files appear as all uppercase

    for (UINT32 i = 0; i < nBurnDrvCount; i++) {

        nBurnDrvActive = i;

        Rom *rom = new Rom();
        char *zn;
        BurnDrvGetZipName(&zn, 0);
        strncpy(rom->zip, zn, 63);
        rom->drv = i;
        rom->drv_name = BurnDrvGetTextA(DRV_NAME);
        rom->parent = BurnDrvGetTextA(DRV_PARENT);
        rom->name = BurnDrvGetTextA(DRV_FULLNAME);
        rom->year = BurnDrvGetTextA(DRV_DATE);
        rom->manufacturer = BurnDrvGetTextA(DRV_MANUFACTURER);
        rom->system = BurnDrvGetTextA(DRV_SYSTEM);
        rom->genre = BurnDrvGetGenreFlags();
        rom->flags = BurnDrvGetFlags();
        rom->state = RomState::MISSING;
        rom->hardware = BurnDrvGetHardwareCode();

        // add rom to "ALL" game list
        hardwareList->at(0).supported_count++;
        if (rom->parent) {
            hardwareList->at(0).clone_count++;
        }

        // add rom to specific hardware
        Hardware *hardware = GetHardware(rom->hardware);
        if (hardware) {
            hardware->supported_count++;
            if (rom->parent) {
                hardware->clone_count++;
            }
        }

        snprintf(path, MAX_PATH, "%s.zip", rom->zip);
        for (int k = 0; k < MAX_PATH; k++) {
            pathUppercase[k] = (char) toupper(path[k]);
        }

        for (int j = 0; j < DIRS_MAX; j++) {

            if (files[j].empty()) {
                continue;
            }

            auto file = std::find(files[j].begin(), files[j].end(), path);
            if (file == files[j].end()) {
                file = std::find(files[j].begin(), files[j].end(), pathUppercase);
            }

            if (file != files[j].end()) {

                int prefix = (((rom->hardware | HARDWARE_PREFIX_CARTRIDGE) ^ HARDWARE_PREFIX_CARTRIDGE) & 0xff000000);

                switch (prefix) {
                    case HARDWARE_PREFIX_COLECO:
                        if (!endsWith(paths[j], "coleco/")) {
                            continue;
                        }
                        break;
                    case HARDWARE_PREFIX_SEGA_GAME_GEAR:
                        if (!endsWith(paths[j], "gamegear/")) {
                            continue;
                        }
                        break;
                    case HARDWARE_PREFIX_SEGA_MEGADRIVE:
                        if (!endsWith(paths[j], "megadriv/")) {
                            continue;
                        }
                        break;
                    case HARDWARE_PREFIX_MSX:
                        if (!endsWith(paths[j], "msx/")) {
                            continue;
                        }
                        break;
                    case HARDWARE_PREFIX_SEGA_SG1000:
                        if (!endsWith(paths[j], "sg1000/")) {
                            continue;
                        }
                        break;
                    case HARDWARE_PREFIX_SEGA_MASTER_SYSTEM:
                        if (!endsWith(paths[j], "sms/")) {
                            continue;
                        }
                        break;
                    case HARDWARE_PREFIX_PCENGINE:
                        switch (rom->hardware) {
                            case HARDWARE_PCENGINE_PCENGINE:
                                if (!endsWith(paths[j], "pce/")) {
                                    continue;
                                }
                                break;
                            case HARDWARE_PCENGINE_TG16:
                                if (!endsWith(paths[j], "tg16/")) {
                                    continue;
                                }
                                break;
                            case HARDWARE_PCENGINE_SGX:
                                if (!endsWith(paths[j], "sgx/")) {
                                    continue;
                                }
                                break;
                            default:
                                continue;
                        }
                        break;
                    default:
                        if (endsWith(paths[j], "coleco/")
                            || endsWith(paths[j], "gamegear/")
                            || endsWith(paths[j], "megadriv/")
                            || endsWith(paths[j], "msx/")
                            || endsWith(paths[j], "sg1000/")
                            || endsWith(paths[j], "sms/")
                            || endsWith(paths[j], "pce/")
                            || endsWith(paths[j], "sgx/")
                            || endsWith(paths[j], "tg16/")) {
                            continue;
                        }
                        break;
                }

                //snprintf(rom->zip_path, 256, "%s%s", paths[j].c_str(), file->c_str());
                rom->state = BurnDrvIsWorking() ? RomState::WORKING : RomState::NOT_WORKING;
                hardwareList->at(0).available_count++;

                if (rom->parent) {
                    hardwareList->at(0).available_clone_count++;
                }

                if (hardware) {
                    hardware->available_count++;
                    if (rom->parent) {
                        hardware->available_clone_count++;
                    }
                }
                break;
            }
        }

        // set "Io::File"" color for ui
        rom->color = COL_RED;
        if (rom->state == RomList::RomState::NOT_WORKING) {
            rom->color = COL_ORANGE;
        } else if (rom->state == RomList::RomState::WORKING) {
            rom->color = rom->parent == NULL ? COL_GREEN : COL_YELLOW;
        }

        if (rom->state == RomState::MISSING) {
            hardwareList->at(0).missing_count++;
            if (hardware) {
                hardware->missing_count++;
            }
            if (rom->parent) {
                hardwareList->at(0).missing_clone_count++;
                if (hardware) {
                    hardware->missing_clone_count++;
                }
            }
        }

        list.push_back(rom);

        // UI
        if (i % 250 == 0) {
            sprintf(text_str, "Scanning... %i%% - ROMS : %i / %i",
                    (i * 100) / nBurnDrvCount, hardwareList->at(0).supported_count, nBurnDrvCount);
            text->setString(text_str);
            gui->getRenderer()->flip();
        }
        // UI
    }

    /*
    for(int i=0; i<hardwareList->size(); i++) {
        printf("[%s] roms: %i/%i | clones: %i/%i)\n",
               hardwareList->at(i).name.c_str(),
               hardwareList->at(i).available_count, hardwareList->at(i).supported_count,
               hardwareList->at(i).available_clone_count, hardwareList->at(i).clone_count);
    }
    */

    for (int i = 0; i < DIRS_MAX; i++) {
        files[i].clear();
    }

    float time_spent = gui->getRenderer()->getElapsedTime().asSeconds() - time_start;
    printf("RomList: list built in %f\n", time_spent);

    // UI
    // reset title texture for later use
    texture->setOriginTopLeft();
    texture->setPosition(0, 0);
    texture->setScale(1, 1);
    rect->remove(texture);
    // remove ui widgets
    delete (rect);
}

RomList::~RomList() {

    for (auto &file : list) {
        delete (file);
    }

    list.clear();
}
