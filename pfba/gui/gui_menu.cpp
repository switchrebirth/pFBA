//
// Created by cpasjuste on 30/01/18.
//

#include "gui_menu.h"
#include "gui_emu.h"
#include "gui_romlist.h"

using namespace c2d;

class MenuLine : public c2d::Rectangle {

public:

    MenuLine(Font &font, int fontSize, FloatRect &rect)
            : Rectangle(rect) {

        setFillColor(Color::Transparent);

        name = new Text("OPTION NAME", font, (unsigned int) fontSize);
        name->setOutlineThickness(1);
        name->setOutlineColor(Color::Black);
        name->setOrigin(0, fontSize / 2);
        name->setPosition(16, getSize().y / 2);
        name->setSizeMax(Vector2f((getSize().x * 0.66f) - 32, 0));
        add(name);

        value = new Text("OPTION VALUE", font, (unsigned int) fontSize);
        value->setOutlineThickness(1);
        value->setOutlineColor(Color::Black);
        value->setOrigin(0, fontSize / 2);
        value->setPosition((getSize().x * 0.66f) + 16, getSize().y / 2);
        value->setSizeMax(Vector2f((getSize().x * 0.33f) - 32, 0));
        add(value);
    }

    c2d::Text *name;
    c2d::Text *value;
    Option *option = NULL;
};

GuiMenu::GuiMenu(Gui *gui) : Rectangle(Vector2f(0, 0)) {

    this->gui = gui;

    Color color = Color::Gray;
    color.a = 220;
    setFillColor(color);
    setOutlineColor(COL_ORANGE);
    setOutlineThickness(4);
    setPosition(UI_MARGIN * gui->getScaling(), UI_MARGIN * gui->getScaling());
    setSize(gui->getRenderer()->getSize().x - (UI_MARGIN * gui->getScaling() * 2),
            gui->getRenderer()->getSize().y - (UI_MARGIN * gui->getScaling() * 2));

    // menu title
    title = new Text("TITLE_______________________", *gui->getSkin()->font);
    title->setSizeMax(Vector2f(getSize().x - 16, 0));
    title->setFillColor(Color::White);
    title->setOutlineThickness(2);
    title->setOutlineColor(COL_RED);
    title->setStyle(c2d::Text::Underlined);
    title->setPosition(20 * gui->getScaling(), 20 * gui->getScaling());
    int start_y = (int) (title->getGlobalBounds().top + title->getGlobalBounds().height + 16 * gui->getScaling());
    add(title);

    // calculate lines per menu
    float font_size = gui->getFontSize();
    float line_height = gui->getFont()->getLineSpacing((unsigned int) font_size) + 8;
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

        lines.push_back(new MenuLine(*gui->getSkin()->font, gui->getFontSize(), rect));
        add(lines[i]);
    }

    // build menus
    optionMenuGui = new OptionMenu(NULL, gui->getConfig()->getOptions());
    optionMenuRom = new OptionMenu(NULL, gui->getConfig()->getOptions(true), true);
}

void GuiMenu::loadMenu(bool isRom, OptionMenu *om) {

    isRomMenu = isRom;
    options = isRomMenu ? gui->getConfig()->getOptions(true)
                        : gui->getConfig()->getOptions();

    if (om == NULL) {
        optionMenu = isRomMenu ? optionMenuRom : optionMenuGui;
    } else {
        optionMenu = om;
    }

    isEmuRunning = gui->getUiEmu()->getVisibility() == C2D_VISIBILITY_VISIBLE;
    optionIndex = 0;
    optionCount = optionMenu->childs.size() + optionMenu->option_ids.size();

    if (isEmuRunning) {
        // if frameskip is enabled, we may get a black buffer,
        // force a frame to be drawn
        if (pBurnDraw == NULL) {
            bPauseOn = false;
            gui->getUiEmu()->drawFrame(true, 0, 0);
            bPauseOn = true;
        }
        if (optionMenu == optionMenuRom) {
            optionMenu->addChild("RETURN");
            optionMenu->addChild("STATES");
            optionMenu->addChild("EXIT");
            optionCount += 3;
        }
    }

    ///////
    // UI
    //////
    if (isRomMenu) {
        char name[128];
        snprintf(name, 128, "%s__________", gui->getUiRomList()->getRom()->name);
        title->setString(name);
    } else {
        title->setString(optionMenu->title + "__________");
    }

    for (int i = 0; i < lines.size(); i++) {
        lines[i]->setVisibility(C2D_VISIBILITY_HIDDEN);
    }

    int line_index = 0;
    for (int i = 0; i < optionMenu->option_ids.size(); i++) {

        if (i >= lines.size()) {
            // oups
            break;
        }

        // menu types
        Option *option = gui->getConfig()->getOption(options, optionMenu->option_ids[i]);
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

        lines[line_index]->name->setString(option->getName());
        lines[line_index]->option = option;
        lines[line_index]->setVisibility(C2D_VISIBILITY_VISIBLE);

        if (option->flags == Option::Type::INPUT) {
            Skin::Button *button = gui->getSkin()->getButton(option->value);
            if (button) {
                if (button->texture) {
                    // TODO
                    lines[line_index]->value->setString(button->name);
                } else {
                    lines[line_index]->value->setString(button->name);
                }
            } else {
                char btn[16];
                snprintf(btn, 16, "%i", option->value);
                lines[line_index]->value->setString(btn);
            }
        } else {
            lines[line_index]->value->setString(option->getValue());
        }

        line_index++;
    }

    for (int i = 0; i < optionMenu->childs.size(); i++) {

        if (i >= lines.size()) {
            // oups
            break;
        }

        lines[line_index]->setVisibility(C2D_VISIBILITY_VISIBLE);
        lines[line_index]->name->setString(optionMenu->childs[i]->title);
        lines[line_index]->value->setString("GO");
        line_index++;
    }

    highlight->setPosition(lines[0]->value->getGlobalBounds().left - 2,
                           lines[0]->getGlobalBounds().top - 5);
}

int GuiMenu::updateKeys() {

    bool option_changed = false;
    int ret = 0;
    int key = gui->getInput()->update()[0].state;

    if (key > 0) {

        // UP
        if (key & Input::Key::KEY_UP) {
            optionIndex--;
            if (optionIndex < 0)
                optionIndex = optionCount - 1;
            highlight->setPosition(lines[optionIndex]->value->getGlobalBounds().left - 2,
                                   lines[optionIndex]->getGlobalBounds().top - 5);
        }
        // DOWN
        if (key & Input::Key::KEY_DOWN) {
            optionIndex++;
            if (optionIndex >= optionCount)
                optionIndex = 0;
            highlight->setPosition(lines[optionIndex]->value->getGlobalBounds().left - 2,
                                   lines[optionIndex]->getGlobalBounds().top - 5);
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
                lines[optionIndex]->value->setString(option->getValue());
                switch (option->index) {
                    case Option::Index::GUI_SHOW_CLONES:
                    case Option::Index::GUI_SHOW_ALL:
                    case Option::Index::GUI_SHOW_HARDWARE:
                        ret = UI_KEY_FILTER_ROMS;
                        break;

                    case Option::ROM_ROTATION:
                    case Option::Index::ROM_SCALING:
                        if (isEmuRunning) {
                            gui->getUiEmu()->getVideo()->updateScaling();
                        }
                        break;
                    case Option::Index::ROM_FILTER:
                        if (isEmuRunning) {
                            gui->getUiEmu()->getVideo()->setFiltering(option->value);
                        }
                        break;
                    case Option::Index::ROM_SHADER:
                        if (isEmuRunning) {
                            gui->getUiEmu()->getVideo()->setShader(option->value);
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
                    // TODO
                    /*
                    int btn = GetButton();
                    if (btn >= 0) {
                        option->value = btn;
                        option_changed = true;
                    }
                    */
                }
            } else {
                OptionMenu *menu = optionMenu->childs[optionIndex - optionMenu->option_ids.size()];
                if (menu->title == "EXIT") {
                    optionMenu->childs.erase(optionMenu->childs.end() - 3, optionMenu->childs.end());
                    setVisibility(C2D_VISIBILITY_HIDDEN);
                    ret = UI_KEY_STOP_ROM;
                } else if (menu->title == "STATES") {
                    // TODO
                    //RunStatesMenu();
                } else if (menu->title == "RETURN") {
                    optionMenu->childs.erase(optionMenu->childs.end() - 3, optionMenu->childs.end());
                    setVisibility(C2D_VISIBILITY_HIDDEN);
                    ret = UI_KEY_RESUME_ROM;
                } else {
                    if (isEmuRunning && optionMenu == optionMenuRom) {
                        optionMenu->childs.erase(optionMenu->childs.end() - 3, optionMenu->childs.end());
                    }
                    loadMenu(isRomMenu, menu);
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
                    setVisibility(C2D_VISIBILITY_HIDDEN);
                    ret = UI_KEY_RESUME_ROM;
                } else {
                    ret = UI_KEY_SHOW_ROMLIST;
                }
            } else {
                loadMenu(isRomMenu, optionMenu->parent);
            }
        }

        // QUIT ?
        if (key & EV_QUIT) {
            return EV_QUIT;
        }

        gui->getRenderer()->delay(INPUT_DELAY);
    }

    if (option_changed) {
        if (isRomMenu) {
            gui->getConfig()->save(gui->getUiRomList()->getRom());
            gui->updateInputMapping(isRomMenu);
        } else {
            gui->getConfig()->save();
        }
    }

    return ret;
}

bool GuiMenu::isOptionHidden(Option *option) {

    // TODO
    return option->index == Option::Index::ROM_ROTATION
           && gui->getUiRomList()->getRom() != NULL
           && !(gui->getUiRomList()->getRom()->flags & BDF_ORIENTATION_VERTICAL);
}

GuiMenu::~GuiMenu() {

    delete (optionMenuGui);
    delete (optionMenuRom);
}

