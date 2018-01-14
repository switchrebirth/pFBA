//
// Created by cpasjuste on 22/11/16.
//
#include <algorithm>
#include "run.h"
#include "gui_romlist.h"

using namespace c2d;

class GuiRomInfo : public Rectangle {

public:

    GuiRomInfo(const FloatRect &rect, float scale) : Rectangle(rect) {

        setFillColor(Color::Transparent);
        scaling = scale;

        rectangle = new Rectangle(
                FloatRect(0, getSize().y / 2 + UI_MARGIN * scaling,
                          getSize().x, getSize().y / 2 - UI_MARGIN * scaling));
        rectangle->setFillColor(Color::GrayLight);
        rectangle->setOutlineColor(COL_GREEN);
        rectangle->setOutlineThickness(2);
        add(rectangle);
    }

    void update(RomList::Rom *rom) {

        if (texture) {
            delete (texture);
            texture = NULL;
        }

        if (!rom) {

        } else {
            // load preview image
            char path[MAX_PATH];
            sprintf(path, "%s/%s.png", szAppPreviewPath, rom->zip);
            texture = new C2DTexture(path);
            if (!texture->available && rom->parent) {
                // try parent image
                delete (texture);
                memset(path, 0, MAX_PATH);
                sprintf(path, "%s/%s.png", szAppPreviewPath, rom->parent);
                texture = new C2DTexture(path);
            }

            // set preview image
            if (texture->available) {
                float outline = rectangle->getOutlineThickness() * 2;
                texture->setOutlineColor(COL_YELLOW);
                texture->setOutlineThickness(outline);
                texture->setOriginCenter();
                texture->setPosition(Vector2f(getLocalBounds().width / 2, getLocalBounds().height / 4));
                float tex_scaling = std::min(
                        (getLocalBounds().width - UI_MARGIN * scaling + outline * 2) / texture->getSize().x,
                        ((getLocalBounds().height / 2) - UI_MARGIN * scaling + outline * 2) / texture->getSize().y);
                texture->setScale(tex_scaling, tex_scaling);
                add(texture);
            }


        }
    }

    Texture *texture = NULL;
    Rectangle *rectangle;
    float scaling = 1;
};

GuiRomList::GuiRomList(Gui *g, const c2d::Vector2f &size) : Rectangle(size) {

    gui = g;

    // build/init roms list
    rom_list = new RomList(gui->getIo(), &gui->getConfig()->hardwareList,
                           gui->getConfig()->GetRomPaths());

    // filter roms
    filterRoms();

    // set gui main "window"
    setFillColor(Color::Gray);
    setOutlineColor(COL_ORANGE);
    setOutlineThickness(gui->getScaling() < 1 ? 1 : 2);
    setPosition(getOutlineThickness(), getOutlineThickness());
    setSize(Vector2f(getSize().x - getOutlineThickness() * 2, getSize().y - getOutlineThickness() * 2));

    // add title image if available
    Skin *skin = gui->getSkin();
    if (skin->tex_title->available) {
        skin->tex_title->setPosition(UI_MARGIN * gui->getScaling(), UI_MARGIN * gui->getScaling());
        float scale = (getLocalBounds().width / 3) / skin->tex_title->getSize().x;
        skin->tex_title->setScale(scale, scale);
        add(skin->tex_title);
    }

    // add rom list ui
    float top = skin->tex_title->getGlobalBounds().top
                + skin->tex_title->getGlobalBounds().height
                + UI_MARGIN * gui->getScaling();

    FloatRect rect = {
            UI_MARGIN * gui->getScaling(), top,
            (getLocalBounds().width / 2) - UI_MARGIN * gui->getScaling(),
            getLocalBounds().height - top - UI_MARGIN * gui->getScaling()};

    list_box = new ListBox(*skin->font, rect, (std::vector<Io::File *> &) roms);
    list_box->setOutlineThickness(getOutlineThickness());
    list_box->setFillColor(Color::GrayLight);
    list_box->setOutlineColor(COL_ORANGE);
    add(list_box);

    // add rom info ui
    rom_info = new GuiRomInfo(
            FloatRect(
                    (getLocalBounds().width / 2) + UI_MARGIN * gui->getScaling(),
                    UI_MARGIN * gui->getScaling(),
                    (getLocalBounds().width / 2) - UI_MARGIN * gui->getScaling() * 2,
                    getLocalBounds().height - UI_MARGIN * gui->getScaling() * 2), gui->getScaling());
    rom_info->rectangle->setOutlineThickness(getOutlineThickness());
    rom_info->update(roms[0]);
    add(rom_info);

    timer_input = new Timer();
    timer_load = new Timer();
}

int GuiRomList::updateState() {


    Input::Player *players = gui->getInput()->Update();

    int key = players[0].state;
    if (key > 0) {

        if (key & Input::Key::KEY_UP) {
            rom_index--;
            if (rom_index < 0)
                rom_index = (int) (roms.size() - 1);
            list_box->setSelection(rom_index);
            rom_info->update(NULL);
            title_loaded = 0;
        } else if (key & Input::Key::KEY_DOWN) {
            rom_index++;
            if (rom_index >= roms.size())
                rom_index = 0;
            list_box->setSelection(rom_index);
            rom_info->update(NULL);
            title_loaded = 0;
        } else if (key & Input::Key::KEY_RIGHT) {
            rom_index += list_box->getMaxLines();
            if (rom_index >= roms.size())
                rom_index = (int) (roms.size() - 1);
            list_box->setSelection(rom_index);
            rom_info->update(NULL);
            title_loaded = 0;
        } else if (key & Input::Key::KEY_LEFT) {
            rom_index -= list_box->getMaxLines();
            if (rom_index < 0)
                rom_index = 0;
            list_box->setSelection(rom_index);
            rom_info->update(NULL);
            title_loaded = 0;
        } else if (key & Input::Key::KEY_FIRE1) {
            if (rom != NULL
                && rom->state != RomList::RomState::MISSING) {
                // TODO
                //RunRom(romSelected);
            }
        } else if (key & Input::Key::KEY_MENU1) {
            // TODO
            //RunOptionMenu();
            //if (title != NULL) {
            // refresh preview/title image
            //}
        } else if (key & Input::Key::KEY_MENU2) {
            if (rom != NULL) {
                gui->getConfig()->Load(rom);
                // TODO
                //RunOptionMenu(true);
                //if (title != NULL) {
                // refresh preview/title image
                //}
            }
        } else if (key & EV_QUIT) {
            return key;
        }

        if (timer_input->GetSeconds() > 6) {
            gui->getRenderer()->delay(INPUT_DELAY / 5);
        } else if (timer_input->GetSeconds() > 2) {
            gui->getRenderer()->delay(INPUT_DELAY / 2);
        } else {
            gui->getRenderer()->delay(INPUT_DELAY);
        }
        timer_load->Reset();

    } else {

        if (/*romSelected != NULL &&*/ !title_loaded
                                       && timer_load->GetMillis() >= title_delay) {
            rom_info->update(roms[rom_index]);
            /*
            if (TitleLoad(romSelected)) {
                // refresh preview/title image
            }
            */
            title_loaded = 1;
            timer_load->Reset();
        }
        timer_input->Reset();
    }

    return 0;
}

void GuiRomList::filterRoms() {

    roms.clear();

    int showClone = gui->getConfig()->GetGuiValue(Option::Index::GUI_SHOW_CLONES);
    int showAll = gui->getConfig()->GetGuiValue(Option::Index::GUI_SHOW_ALL);
    int showHardwareCfg = gui->getConfig()->GetGuiValue(Option::Index::GUI_SHOW_HARDWARE);
    int showHardware = gui->getConfig()->hardwareList[showHardwareCfg].prefix;

    remove_copy_if(rom_list->list.begin(), rom_list->list.end(), back_inserter(roms),
                   [showAll, showClone, showHardware](RomList::Rom *r) {
                       return !showAll && r->state != RomList::RomState::WORKING
                              || !showClone && r->parent != NULL
                              || showHardware != HARDWARE_PREFIX_ALL && !RomList::IsHardware(r->hardware, showHardware);
                   });

    rom_index = 0;
}

GuiRomList::~GuiRomList() {

    delete (timer_input);
    delete (timer_load);
    delete (rom_list);
}
