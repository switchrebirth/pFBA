//
// Created by cpasjuste on 30/01/18.
//

#include "gui_menu.h"
#include "gui_emu.h"
#include "gui_romlist.h"

using namespace c2d;

class MenuLine : public c2d::Rectangle {

public:

    MenuLine(Gui *ui, FloatRect &rect)
            : Rectangle(rect) {

        setFillColor(Color::Transparent);

        this->ui = ui;
        Font *font = ui->getSkin()->font;
        int fontSize = ui->getFontSize();

        name = new Text("OPTION NAME", *font, (unsigned int) fontSize);
        name->setOutlineThickness(1);
        name->setOutlineColor(Color::Black);
        name->setOrigin(0, fontSize / 2);
        name->setPosition(16, getSize().y / 2);
        name->setSizeMax(Vector2f((getSize().x * 0.66f) - 32, 0));
        add(name);

        value = new Text("OPTION VALUE", *font, (unsigned int) fontSize);
        value->setOutlineThickness(1);
        value->setOutlineColor(Color::Black);
        value->setOrigin(0, fontSize / 2);
        value->setPosition((getSize().x * 0.66f) + 16, getSize().y / 2);
        value->setSizeMax(Vector2f((getSize().x * 0.33f) - 32, 0));
        add(value);
    }

    void update(Option *opt) {

        this->option = opt;
        name->setString(option->getName());

        if (texture != NULL) {
            delete (texture);
            texture = NULL;
        }

        if (option->flags == Option::Type::INPUT) {
            Skin::Button *button = ui->getSkin()->getButton(option->value);
            if (button) {
                if (ui->getIo()->exist(button->path.c_str())) {
                    texture = new C2DTexture(button->path.c_str());
                    if (texture->available) {
                        value->setVisibility(Hidden);
                        float tex_scaling = std::min(
                                ((getSize().x * 0.33f) - 32) / texture->getSize().x,
                                (getSize().y / 2 + 4) / texture->getSize().y);
                        texture->setScale(tex_scaling, tex_scaling);
                        texture->setPosition((getSize().x * 0.66f) + 16, getSize().y / 2 - 3);
                        texture->setOrigin(0, texture->getSize().y / 2);
                        add(texture);
                    } else {
                        delete (texture);
                        texture = NULL;
                        value->setVisibility(Visible);
                        value->setString(button->name);
                    }
                } else {
                    value->setVisibility(Visible);
                    value->setString(button->name);
                }
            } else {
                char btn[16];
                snprintf(btn, 16, "%i", option->value);
                value->setVisibility(Visible);
                value->setString(btn);
            }
        } else {
            value->setVisibility(Visible);
            value->setString(option->getValue());
        }
    }

    Gui *ui = NULL;
    Text *name = NULL;
    Text *value = NULL;
    Texture *texture = NULL;
    Option *option;
};

GuiMenu::GuiMenu(Gui *ui) : Rectangle(Vector2f(0, 0)) {

    printf("GuiMenu (%p)\n", this);
    this->ui = ui;

    setFillColor(fillColor[0]);
    setOutlineColor(COL_YELLOW);
    setOutlineThickness(2);
    if (ui->getRenderer()->getSize().y < 544) {
        setPosition(UI_MARGIN * ui->getScaling(), UI_MARGIN * ui->getScaling());
        setSize(ui->getRenderer()->getSize().x - (UI_MARGIN * ui->getScaling() * 2),
                ui->getRenderer()->getSize().y - (UI_MARGIN * ui->getScaling() * 2));
    } else {
        setPosition(UI_MARGIN * ui->getScaling() * 4, UI_MARGIN * ui->getScaling() * 4);
        setSize(ui->getRenderer()->getSize().x - (UI_MARGIN * ui->getScaling() * 8),
                ui->getRenderer()->getSize().y - (UI_MARGIN * ui->getScaling() * 8));
    }

    // menu title
    title = new Text("TITLE_______________________", *ui->getSkin()->font);
    title->setSizeMax(Vector2f(getSize().x - 16, 0));
    title->setFillColor(Color::White);
    title->setOutlineThickness(2);
    title->setOutlineColor(COL_RED);
    title->setStyle(c2d::Text::Underlined);
    title->setPosition(20 * ui->getScaling(), 20 * ui->getScaling());
    int start_y = (int) (title->getGlobalBounds().top + title->getGlobalBounds().height + 16 * ui->getScaling());
    add(title);

    // calculate lines per menu
    float font_size = ui->getFontSize();
    float line_height = ui->getFont()->getLineSpacing((unsigned int) font_size) + 8;
    int max_lines = (int) ((getSize().y - start_y) / line_height) * 2;

    // add selection rectangle (highlight)
    highlight = new Rectangle(Vector2f(((getSize().x / 2) * 0.3f) - 4, line_height));
    highlight->setOutlineThickness(1);
    highlight->setOutlineColor(COL_ORANGE);
#ifdef __TINYGL__
    highlight->setFillColor(Color(61, 100, 20, 255));
#else
    highlight->setFillColor(Color(153, 255, 51, 100));
#endif
    add(highlight);

    // add lines of text
    for (int i = 0; i < max_lines; i++) {

        FloatRect rect = {0, start_y + (i * line_height), getSize().x / 2, line_height};
        if (i >= max_lines / 2) {
            rect.left = getSize().x / 2;
            rect.top = start_y + ((i - (max_lines / 2)) * line_height);
        }

        lines.push_back(new MenuLine(ui, rect));
        add(lines[i]);
    }

    // build menus
    optionMenuGui = new OptionMenu(NULL, ui->getConfig()->getOptions());
    optionMenuGui->addChild("EXIT");
    optionMenuRom = new OptionMenu(NULL, ui->getConfig()->getOptions(true), true);
    optionMenuRom->addChild("RETURN");
    optionMenuRom->addChild("STATES");
    optionMenuRom->addChild("EXIT");

    setVisibility(Hidden);
}

void GuiMenu::load(bool isRom, OptionMenu *om) {

    isRomMenu = isRom;
    options = isRomMenu ? ui->getConfig()->getOptions(true)
                        : ui->getConfig()->getOptions();

    if (om == NULL) {
        optionMenu = isRomMenu ? optionMenuRom : optionMenuGui;
    } else {
        optionMenu = om;
    }

    isEmuRunning = ui->getUiEmu()->getVisibility() == Visible;
    setFillColor(fillColor[isEmuRunning]);

    optionIndex = 0;
    optionCount = (int) (optionMenu->childs.size() + optionMenu->option_ids.size());

    if (isEmuRunning) {
        // if frameskip is enabled, we may get a black buffer,
        // force a frame to be drawn
        if (pBurnDraw == NULL) {
            ui->getUiEmu()->resume();
            ui->getUiEmu()->renderFrame();
            ui->getUiEmu()->pause();
        }
    }

    if (isRomMenu) {
        char name[128];
        snprintf(name, 128, "%s__________", ui->getUiRomList()->getSelection()->name);
        title->setString(name);
    } else {
        title->setString(optionMenu->title + "__________");
    }

    for (int i = 0; i < lines.size(); i++) {
        lines[i]->setVisibility(Hidden);
    }

    int line_index = 0;
    for (int i = 0; i < optionMenu->option_ids.size(); i++) {

        if (i >= lines.size()) {
            // oups
            break;
        }

        // menu types
        Option *option = ui->getConfig()->getOption(options, optionMenu->option_ids[i]);
        if (option == NULL) {
            optionCount--;
            continue;
        }

        // skip rotation option if not needed
        if ((isOptionHidden(option)) || option->flags & Option::Type::HIDDEN) {
            optionCount--;
            continue;
        }

        lines[line_index]->update(option);
        lines[line_index]->setVisibility(Visible);

        line_index++;
    }

    for (int i = 0; i < optionMenu->childs.size(); i++) {

        if (i >= lines.size()) {
            // oups
            break;
        }

        // don't show custom in-game options when a game is not running
        if (isRomMenu && !isEmuRunning) {
            std::string title = optionMenu->childs[i]->title;
            if (title == "EXIT" || title == "RETURN" || title == "STATES") {
                optionCount--;
                continue;
            }
        }

        lines[line_index]->setVisibility(Visible);
        lines[line_index]->name->setString(optionMenu->childs[i]->title);
        lines[line_index]->value->setString("GO");
        line_index++;
    }

    updateHighlight();

    setVisibility(Visible);
    setLayer(1);
}

void GuiMenu::updateHighlight() {

    /*
    for (int i = 0; i < lines.size(); i++) {
        lines[i]->name->setFillColor(
                optionIndex == i ? COL_ORANGE : Color::White);
        lines[i]->value->setFillColor(
                optionIndex == i ? COL_ORANGE : Color::White);
    }
    */
    highlight->setPosition(lines[optionIndex]->value->getGlobalBounds().left - 2,
                           lines[optionIndex]->getGlobalBounds().top - 4);
}

int GuiMenu::update() {

    int ret = 0;
    bool option_changed = false;
    int key = ui->getInput()->update()[0].state;

    if (key > 0) {

        // UP
        if (key & Input::Key::KEY_UP) {
            optionIndex--;
            if (optionIndex < 0)
                optionIndex = optionCount - 1;
            updateHighlight();
        }
        // DOWN
        if (key & Input::Key::KEY_DOWN) {
            optionIndex++;
            if (optionIndex >= optionCount)
                optionIndex = 0;
            updateHighlight();
        }
        // LEFT /RIGHT
        if ((key & Input::Key::KEY_LEFT || key & Input::Key::KEY_RIGHT)
            && optionIndex < optionMenu->option_ids.size()) {
            Option *option = lines[optionIndex]->option;
            if (!option) {
                return ret;
            }
            option_changed = true;
            if (option->flags == Option::Type::INTEGER) {
                if (key & Input::Key::KEY_LEFT) {
                    option->prev();
                } else {
                    option->next();
                }
                lines[optionIndex]->update(option);

                switch (option->index) {
                    case Option::Index::GUI_SHOW_CLONES:
                    case Option::Index::GUI_SHOW_ALL:
                    case Option::Index::GUI_SHOW_HARDWARE:
                        ret = UI_KEY_FILTER_ROMS;
                        break;

                    case Option::ROM_ROTATION:
                    case Option::Index::ROM_SCALING:
                        if (isEmuRunning) {
#ifndef __NX__
                            ui->getUiEmu()->getVideo()->updateScaling();
#endif
                        }
                        break;
                    case Option::Index::ROM_FILTER:
                        if (isEmuRunning) {
                            ui->getUiEmu()->getVideo()->setFiltering(option->value);
                        }
                        break;
                    case Option::Index::ROM_SHADER:
                        if (isEmuRunning) {
                            ui->getUiEmu()->getVideo()->setShader(option->value);
                        }
                        break;
                    default:
                        break;
                }
            }
        }

        // FIRE1
        if (key & Input::Key::KEY_FIRE1) {
            if (optionIndex < optionMenu->option_ids.size()) {
                Option *option = lines[optionIndex]->option;
                if (option->flags == Option::Type::INPUT) {
                    int new_key = 0;
                    int res = ui->getUiMessageBox()->show("NEW INPUT", "PRESS A BUTTON", "", "", &new_key, 5);
                    if (res != MessageBox::TIMEOUT) {
                        option->value = new_key;
                        option_changed = true;
                        lines[optionIndex]->update(option);
                    }
                }
            } else {
                // extra options in menu (manually added)
                OptionMenu *menu = optionMenu->childs[optionIndex - optionMenu->option_ids.size()];
                if (menu->title == "EXIT") {
                    setVisibility(Hidden);
                    ret = isRomMenu ? UI_KEY_STOP_ROM : EV_QUIT;
                } else if (menu->title == "STATES") {
                    setVisibility(Hidden);
                    ret = UI_KEY_SHOW_MEMU_STATE;
                } else if (menu->title == "RETURN") {
                    setVisibility(Hidden);
                    ret = UI_KEY_RESUME_ROM;
                } else {
                    load(isRomMenu, menu);
                }
            }
        }

        // FIRE2
        if (key & Input::Key::KEY_FIRE2
            || (key & Input::Key::KEY_MENU1 && !isRomMenu)
            || (key & Input::Key::KEY_MENU2 && isRomMenu)) {
            if (optionMenu->parent == NULL) {
                if (isEmuRunning) {
                    setVisibility(Hidden);
                    ret = UI_KEY_RESUME_ROM;
                } else {
                    ret = UI_KEY_SHOW_ROMLIST;
                }
            } else {
                load(isRomMenu, optionMenu->parent);
            }
        }

        if (key & EV_QUIT) {
            return EV_QUIT;
        }

        ui->getRenderer()->delay(INPUT_DELAY);
    }

    if (option_changed) {
        if (isRomMenu) {
            ui->getConfig()->save(ui->getUiRomList()->getSelection());
        } else {
            ui->getConfig()->save();
        }
    }

    ui->getRenderer()->flip();

    return ret;
}

bool GuiMenu::isOptionHidden(Option *option) {

    RomList::Rom *rom = ui->getUiRomList()->getSelection();

    if (isRomMenu && option->index == Option::Index::ROM_ROTATION
        && rom != NULL && !(rom->flags & BDF_ORIENTATION_VERTICAL)) {
        return true;
    }

    if (isRomMenu && option->index == Option::Index::ROM_NEOBIOS
        && rom != NULL && !(RomList::IsHardware(rom->hardware, HARDWARE_PREFIX_SNK))) {
        return true;
    }

#ifdef __NX__
    // TODO: disabled until gpu ?
    if (option->index == Option::Index::ROM_FILTER
        || option->index == Option::Index::ROM_SHOW_FPS
        || option->index == Option::Index::ROM_FRAMESKIP) {
        return true;
    }
#endif

    return false;
}

GuiMenu::~GuiMenu() {
    printf("~GuiMenu\n");
    delete (optionMenuGui);
    delete (optionMenuRom);
}
