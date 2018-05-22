//
// Created by cpasjuste on 22/11/16.
//
#include <algorithm>
#include "gui_romlist.h"

using namespace c2d;

#define MIN_SIZE_Y  200

class GuiRomInfo : public Rectangle {

public:

    GuiRomInfo(const Font &font, int fontSize, const FloatRect &rect, float scale) : Rectangle(rect) {

        printf("GuiRomInfo\n");

        setFillColor(Color::Transparent);
        scaling = scale;
        margin = UI_MARGIN * scaling;

        infoBox = new Rectangle(FloatRect(0, getSize().y / 2 + margin,
                                          getSize().x, getSize().y / 2 - margin));
        infoBox->setFillColor(Color::GrayLight);
        infoBox->setOutlineColor(COL_GREEN);
        infoBox->setOutlineThickness(2);

        infoText = new Text("", font, (unsigned int) fontSize);
        infoText->setPosition(margin, margin);
        infoText->setOutlineThickness(2);
        infoText->setSizeMax(Vector2f(infoBox->getSize().x, 0));
        infoText->setLineSpacingModifier((int) (8 * scaling));
        infoBox->add(infoText);

        add(infoBox);
    }

    ~GuiRomInfo() {
        printf("~GuiRomInfo\n");
    }

    void update(RomList::Rom *rom) {

        if (texture) {
            delete (texture);
            texture = NULL;
        }

        if (!rom) {
            infoText->setVisibility(Hidden);
        } else {
            // load preview image
            snprintf(texture_path, 1023, "%s/%s.png", szAppPreviewPath, rom->zip);
            texture = new C2DTexture(texture_path);
            if (!texture->available && rom->parent) {
                // try parent image
                delete (texture);
                memset(texture_path, 0, MAX_PATH);
                snprintf(texture_path, 1023, "%s/%s.png", szAppPreviewPath, rom->parent);
                texture = new C2DTexture(texture_path);
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
            strcpy(rotation, "ROTATION: HORIZONTAL");
            if (rom->flags & BDF_ORIENTATION_VERTICAL) {
                sprintf(rotation, "ROTATION: VERTICAL");
                if (rom->flags & BDF_ORIENTATION_FLIPPED) {
                    strncat(rotation, " / FLIPPED", MAX_PATH);
                }
            }
            snprintf(info, 1024, "ZIP: %s.ZIP\nSTATUS: %s\nSYSTEM: %s\nMANUFACTURER: %s\nYEAR: %s\n%s",
                     rom->zip, rom->state == RomList::RomState::MISSING ? "MISSING" : "AVAILABLE",
                     rom->system, rom->manufacturer, rom->year, rotation);
            infoText->setString(info);
            infoText->setVisibility(Visible);
        }
    }

    Texture *texture = NULL;
    Rectangle *infoBox = NULL;
    Text *infoText = NULL;
    char texture_path[1024];
    char info[1024];
    char rotation[64];
    float margin = 0;
    float scaling = 1;
};

GuiRomList::GuiRomList(Gui *g, const c2d::Vector2f &size) : Rectangle(size) {

    printf("GuiRomList\n");

    ui = g;

    // build/init roms list
    rom_list = new RomList(ui);

    // set gui main "window"
    setFillColor(Color::Gray);
    setOutlineColor(COL_ORANGE);
    setOutlineThickness(ui->getScaling() < 1 ? 1 : 2);
    setPosition(getOutlineThickness(), getOutlineThickness());
    setSize(Vector2f(getSize().x - getOutlineThickness() * 2, getSize().y - getOutlineThickness() * 2));

    // add title image if available
    Skin *skin = ui->getSkin();
    if (skin->tex_title->available) {
        skin->tex_title->setPosition(UI_MARGIN * ui->getScaling(), UI_MARGIN * ui->getScaling());
        float scale = (getLocalBounds().width / 3) / skin->tex_title->getSize().x;
        skin->tex_title->setScale(scale, scale);
        add(skin->tex_title);
    }

    // filter roms
    updateRomList();

    // add rom info ui
    rom_info = new GuiRomInfo(*skin->font, ui->getFontSize(),
                              FloatRect(
                                      (getLocalBounds().width / 2) + UI_MARGIN * ui->getScaling(),
                                      UI_MARGIN * ui->getScaling(),
                                      (getLocalBounds().width / 2) - UI_MARGIN * ui->getScaling() * 2,
                                      getLocalBounds().height - UI_MARGIN * ui->getScaling() * 2), ui->getScaling());
    rom_info->infoBox->setOutlineThickness(getOutlineThickness());
    rom_info->update(!roms.empty() ? roms[0] : NULL);
    add(rom_info);
}

int GuiRomList::update() {

    Input::Player *players = ui->getInput()->update();

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
            if (getSelection() != NULL
                && getSelection()->state != RomList::RomState::MISSING) {
                return UI_KEY_RUN_ROM;
            }
        } else if (key & Input::Key::KEY_START) {
            return UI_KEY_SHOW_MEMU_UI;
        } else if (key & Input::Key::KEY_COIN) {
            if (getSelection() != NULL) {
                return UI_KEY_SHOW_MEMU_ROM;
            }
        } else if (key & EV_QUIT) {
            return EV_QUIT;
        }

        if (timer_input.getElapsedTime().asSeconds() > 12) {
            ui->getRenderer()->delay(INPUT_DELAY / 8);
        } else if (timer_input.getElapsedTime().asSeconds() > 6) {
            ui->getRenderer()->delay(INPUT_DELAY / 5);
        } else if (timer_input.getElapsedTime().asSeconds() > 2) {
            ui->getRenderer()->delay(INPUT_DELAY / 2);
        } else {
            ui->getRenderer()->delay(INPUT_DELAY);
        }

        timer_load.restart();

    } else {

        if (!title_loaded && timer_load.getElapsedTime().asMilliseconds() > (unsigned long) load_delay) {
            rom_info->update(roms.size() > rom_index ? roms[rom_index] : NULL);
            title_loaded = 1;
        }

        timer_input.restart();
    }

    ui->getRenderer()->flip();

    return 0;
}

RomList::Rom *GuiRomList::getSelection() {
    return (RomList::Rom *) list_box->getSelection();
}

void GuiRomList::updateRomList() {

    rom_index = 0;
    roms.clear();

    int showClone = ui->getConfig()->getValue(Option::Index::GUI_SHOW_CLONES);
    int showAll = ui->getConfig()->getValue(Option::Index::GUI_SHOW_ALL);
    int showHardwareCfg = ui->getConfig()->getValue(Option::Index::GUI_SHOW_HARDWARE);
    int showHardware = ui->getConfig()->hardwareList[showHardwareCfg].prefix;

    remove_copy_if(rom_list->list.begin(), rom_list->list.end(), back_inserter(roms),
                   [showAll, showClone, showHardware](RomList::Rom *r) {
                       return (!showAll && r->state != RomList::RomState::WORKING)
                              || (!showClone && r->parent != NULL)
                              || (showHardware != HARDWARE_PREFIX_ALL
                                  && !RomList::IsHardware(r->hardware, showHardware));
                   });

    if (list_box != NULL) {
        delete (list_box);
        list_box = NULL;
    }

    // add rom list ui
    float top = ui->getSkin()->tex_title->getGlobalBounds().top
                + ui->getSkin()->tex_title->getGlobalBounds().height
                + UI_MARGIN * ui->getScaling();

    FloatRect rect = {
            UI_MARGIN * ui->getScaling(), top,
            (getLocalBounds().width / 2) - UI_MARGIN * ui->getScaling(),
            getLocalBounds().height - top - UI_MARGIN * ui->getScaling()};
    list_box = new ListBox(*ui->getSkin()->font, ui->getFontSize(), rect, (std::vector<Io::File *> &) roms);
    list_box->setOutlineThickness(getOutlineThickness());
    list_box->setFillColor(Color::GrayLight);
    list_box->setOutlineColor(COL_ORANGE);
    add(list_box);

    if (rom_info) {
        rom_info->update(NULL);
        title_loaded = 0;
        timer_load.restart();
    }
}

void GuiRomList::setLoadDelay(int delay) {
    load_delay = delay;
}

GuiRomList::~GuiRomList() {

    printf("~GuiRomList\n");
    delete (rom_list);
}
