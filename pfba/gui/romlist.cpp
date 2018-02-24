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

RomList::RomList(Gui *gui) {

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
            printf("RomList: found %i files in `%s`\n", (int) files[i].size(), paths[i].c_str());
        }
    }

    char path[MAX_PATH];
    char pathUppercase[MAX_PATH]; // sometimes on FAT32 short files appear as all uppercase
    for (UINT32 i = 0; i < nBurnDrvCount; i++) {

        nBurnDrvActive = i;

        Rom *rom = new Rom();
        rom->zip = BurnDrvGetTextA(DRV_NAME);
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

        for (int j = 0; j < DIRS_MAX; j++) {
            if (files[j].empty()) {
                continue;
            }
            sprintf(path, "%s.zip", rom->zip);
            for (int k = 0; k < MAX_PATH; k++) {
                pathUppercase[k] = toupper(path[k]);
            }
            if (std::find(files[j].begin(), files[j].end(), path) != files[j].end() ||
                std::find(files[j].begin(), files[j].end(), pathUppercase) != files[j].end()) {
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
