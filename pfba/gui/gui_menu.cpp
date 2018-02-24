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

    void update(Option *option) {

        this->option = option;
        name->setString(option->getName());

        if (option->flags == Option::Type::INPUT) {
            Skin::Button *buttonTex = ui->getSkin()->getButton(option->value);
            if (buttonTex) {
                if (buttonTex->texture) {
                    // TODO: load button texture
                    value->setString(buttonTex->name);
                } else {
                    value->setString(buttonTex->name);
                }
            } else {
                char btn[16];
                snprintf(btn, 16, "%i", option->value);
                value->setString(btn);
            }
        } else {
            value->setString(option->getValue());
        }
    }

    Gui *ui;
    c2d::Text *name;
    c2d::Text *value;
    Option *option = NULL;
};

GuiMenu::GuiMenu(Gui *ui) : Rectangle(Vector2f(0, 0)) {

    this->ui = ui;

    setFillColor(fillColor[0]);
    setOutlineColor(COL_ORANGE);
    setOutlineThickness(4);
    setPosition(UI_MARGIN * ui->getScaling(), UI_MARGIN * ui->getScaling());
    setSize(ui->getRenderer()->getSize().x - (UI_MARGIN * ui->getScaling() * 2),
            ui->getRenderer()->getSize().y - (UI_MARGIN * ui->getScaling() * 2));

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
    highlight->setFillColor(Color(153, 255, 51, 100));
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
    optionMenuRom = new OptionMenu(NULL, ui->getConfig()->getOptions(true), true);

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
        if (optionMenu == optionMenuRom) {
            optionMenu->addChild("RETURN");
            optionMenu->addChild("STATES");
            optionMenu->addChild("EXIT");
            optionCount += 3;
        }
    }

    if (isRomMenu) {
        char name[128];
        snprintf(name, 128, "%s__________", ui->getUiRomList()->getSelection()->name);
        title->setString(name);
    } else {
        title->setString(optionMenu->title + "__________");
        if (optionMenu == optionMenuGui) {
            optionMenu->addChild("EXIT");
            optionCount += 1;
        }
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
        if ((isRomMenu && isOptionHidden(option))
            || option->flags & Option::Type::HIDDEN) {
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

        lines[line_index]->setVisibility(Visible);
        lines[line_index]->name->setString(optionMenu->childs[i]->title);
        lines[line_index]->value->setString("GO");
        line_index++;
    }

    highlight->setPosition(lines[0]->value->getGlobalBounds().left - 2,
                           lines[0]->getGlobalBounds().top - 4);

    setVisibility(Visible);
    setLayer(1);
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
            highlight->setPosition(lines[optionIndex]->value->getGlobalBounds().left - 2,
                                   lines[optionIndex]->getGlobalBounds().top - 4);
        }
        // DOWN
        if (key & Input::Key::KEY_DOWN) {
            optionIndex++;
            if (optionIndex >= optionCount)
                optionIndex = 0;
            highlight->setPosition(lines[optionIndex]->value->getGlobalBounds().left - 2,
                                   lines[optionIndex]->getGlobalBounds().top - 4);
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
                            ui->getUiEmu()->getVideo()->updateScaling();
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
                if (option && option->flags == Option::Type::INPUT) {
                    int new_key = 0;
                    int res = ui->getUiMessageBox()->show("NEW INPUT", "PRESS A BUTTON", "", "", &new_key, 5);
                    if (res != MessageBox::TIMEOUT) {
                        // TODO: update ui
                        option->value = new_key;
                        option_changed = true;
                        lines[optionIndex]->update(option);
                    }
                }
            } else {
                OptionMenu *menu = optionMenu->childs[optionIndex - optionMenu->option_ids.size()];
                if (menu->title == "EXIT") {
                    if (optionMenu == optionMenuGui) {
                        optionMenu->childs.erase(optionMenu->childs.end() - 1, optionMenu->childs.end());
                        setVisibility(Hidden);
                        ret = EV_QUIT;
                    } else {
                        optionMenu->childs.erase(optionMenu->childs.end() - 3, optionMenu->childs.end());
                        setVisibility(Hidden);
                        ret = UI_KEY_STOP_ROM;
                    }
                } else if (menu->title == "STATES") {
                    optionMenu->childs.erase(optionMenu->childs.end() - 3, optionMenu->childs.end());
                    setVisibility(Hidden);
                    ret = UI_KEY_SHOW_MEMU_STATE;
                } else if (menu->title == "RETURN") {
                    optionMenu->childs.erase(optionMenu->childs.end() - 3, optionMenu->childs.end());
                    setVisibility(Hidden);
                    ret = UI_KEY_RESUME_ROM;
                } else {
                    if (isEmuRunning && optionMenu == optionMenuRom) {
                        optionMenu->childs.erase(optionMenu->childs.end() - 3, optionMenu->childs.end());
                    }
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
                    optionMenu->childs.erase(optionMenu->childs.end() - 3, optionMenu->childs.end());
                    setVisibility(Hidden);
                    ret = UI_KEY_RESUME_ROM;
                } else {
                    if (!isRomMenu) {
                        optionMenu->childs.erase(optionMenu->childs.end() - 1, optionMenu->childs.end());
                    }
                    ret = UI_KEY_SHOW_ROMLIST;
                }
            } else {
                load(isRomMenu, optionMenu->parent);
            }
        }

        // QUIT ?
        if (key & EV_QUIT) {
            return EV_QUIT;
        }

        ui->getRenderer()->delay(INPUT_DELAY);
    }

    if (option_changed) {
        if (isRomMenu) {
            ui->getConfig()->save(ui->getUiRomList()->getSelection());
            if (isEmuRunning) {
                ui->updateInputMapping(true);
            }
        } else {
            ui->getConfig()->save();
        }
    }

    ui->getRenderer()->flip();

    return ret;
}

bool GuiMenu::isOptionHidden(Option *option) {

    RomList::Rom *rom = ui->getUiRomList()->getSelection();

    if (option->index == Option::Index::ROM_ROTATION
        && rom != NULL && !(rom->flags & BDF_ORIENTATION_VERTICAL)) {
        return true;
    }

    if (option->index == Option::Index::ROM_NEOBIOS
        && rom != NULL && !(RomList::IsHardware(rom->hardware, HARDWARE_PREFIX_SNK))) {
        return true;
    }

    return false;
}

GuiMenu::~GuiMenu() {

    delete (optionMenuGui);
    delete (optionMenuRom);
}

