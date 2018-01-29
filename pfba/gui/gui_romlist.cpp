//
// Created by cpasjuste on 22/11/16.
//
#include <algorithm>
#include "run.h"
#include "gui_romlist.h"

using namespace c2d;

#define MIN_SIZE_Y  200

class GuiRomInfo : public Rectangle {

public:

    GuiRomInfo(const Font &font, const FloatRect &rect, float scale, int fontSize = 0) : Rectangle(rect) {

        setFillColor(Color::Transparent);
        scaling = scale;
        margin = UI_MARGIN * scaling;

        infoBox = new Rectangle(FloatRect(0, getSize().y / 2 + margin,
                                          getSize().x, getSize().y / 2 - margin));
        infoBox->setFillColor(Color::GrayLight);
        infoBox->setOutlineColor(COL_GREEN);
        infoBox->setOutlineThickness(2);

        infoText = new Text("", font, (unsigned int) fontSize);
        infoText->setOutlineThickness(2);
        infoText->setSizeMax(Vector2f(infoBox->getSize().x, 0));
        infoText->setLineSpacingModifier((int) (8 * scaling));
        infoBox->add(infoText);

        add(infoBox);
    }

    void update(RomList::Rom *rom) {

        if (texture) {
            delete (texture);
            texture = NULL;
        }

        if (!rom) {
            infoText->setVisibility(C2D_VISIBILITY_HIDDEN);
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
                float outline = infoBox->getOutlineThickness() * 2;
                texture->setOutlineColor(COL_YELLOW);
                texture->setOutlineThickness(outline);
                texture->setOriginCenter();
                texture->setPosition(Vector2f(getLocalBounds().width / 2, getLocalBounds().height / 4));
                float tex_scaling = std::min(
                        (getLocalBounds().width - margin + outline * 2) / texture->getSize().x,
                        ((getLocalBounds().height / 2) - margin + outline * 2) / texture->getSize().y);
                texture->setScale(tex_scaling, tex_scaling);
                add(texture);
            } else {
                delete (texture);
                texture = NULL;
            }

            // update info text
            snprintf(info, 512, "ZIP: %s.ZIP\nSTATUS: %s\nSYSTEM: %s\nMANUFACTURER: %s\nYEAR: %s",
                     rom->zip, rom->state == RomList::RomState::MISSING ? "MISSING" : "AVAILABLE",
                     rom->system, rom->manufacturer, rom->year);
            infoText->setString(info);
            infoText->setPosition(margin, (infoText->getGlobalBounds().height / 2) + margin);
            infoText->setVisibility(C2D_VISIBILITY_VISIBLE);
        }
    }

    Texture *texture = NULL;
    Rectangle *infoBox = NULL;
    Text *infoText = NULL;
    char info[512];
    float margin = 0;

    float scaling = 1;
};

GuiRomList::GuiRomList(Gui *g, const c2d::Vector2f &size) : Rectangle(size) {

    gui = g;

    // build/init roms list
    rom_list = new RomList(gui);

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
    rom_info = new GuiRomInfo(*skin->font,
                              FloatRect(
                                      (getLocalBounds().width / 2) + UI_MARGIN * gui->getScaling(),
                                      UI_MARGIN * gui->getScaling(),
                                      (getLocalBounds().width / 2) - UI_MARGIN * gui->getScaling() * 2,
                                      getLocalBounds().height - UI_MARGIN * gui->getScaling() * 2), gui->getScaling(),
                              list_box->getFontSize());
    rom_info->infoBox->setOutlineThickness(getOutlineThickness());
    rom_info->update(roms[0]);
    add(rom_info);

    timer_input = new Timer();
    timer_load = new Timer();
}

int GuiRomList::updateKeys() {


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
            if (getRom() != NULL && getRom()->state != RomList::RomState::MISSING) {
                return Input::Key::KEY_FIRE1;
            }
        } else if (key & Input::Key::KEY_MENU1) {
            return Input::Key::KEY_MENU1;
        } else if (key & Input::Key::KEY_MENU2) {
            if (getRom() != NULL) {
                return Input::Key::KEY_MENU2;
            }
        } else if (key & EV_QUIT) {
            return key;
        }

        if (timer_input->getSeconds() > 12) {
            gui->getRenderer()->delay(INPUT_DELAY / 8);
        } else if (timer_input->getSeconds() > 6) {
            gui->getRenderer()->delay(INPUT_DELAY / 5);
        } else if (timer_input->getSeconds() > 2) {
            gui->getRenderer()->delay(INPUT_DELAY / 2);
        } else {
            gui->getRenderer()->delay(INPUT_DELAY);
        }

        timer_load->reset();

    } else {

        if (!title_loaded && timer_load->getMillis() > load_delay) {
            rom_info->update(roms[rom_index]);
            title_loaded = 1;
        }

        timer_input->reset();
    }

    return 0;
}

RomList::Rom *GuiRomList::getRom() {
    return (RomList::Rom *) list_box->getSelection();
}

void GuiRomList::filterRoms() {

    roms.clear();

    int showClone = gui->getConfig()->getValue(Option::Index::GUI_SHOW_CLONES);
    int showAll = gui->getConfig()->getValue(Option::Index::GUI_SHOW_ALL);
    int showHardwareCfg = gui->getConfig()->getValue(Option::Index::GUI_SHOW_HARDWARE);
    int showHardware = gui->getConfig()->hardwareList[showHardwareCfg].prefix;

    remove_copy_if(rom_list->list.begin(), rom_list->list.end(), back_inserter(roms),
                   [showAll, showClone, showHardware](RomList::Rom *r) {
                       return !showAll && r->state != RomList::RomState::WORKING
                              || !showClone && r->parent != NULL
                              || showHardware != HARDWARE_PREFIX_ALL && !RomList::IsHardware(r->hardware, showHardware);
                   });

    rom_index = 0;
}

void GuiRomList::setLoadDelay(int delay) {
    load_delay = delay;
}

GuiRomList::~GuiRomList() {

    delete (timer_input);
    delete (timer_load);
    delete (rom_list);
}

