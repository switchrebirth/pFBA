//
// Created by cpasjuste on 16/02/17.
//

#include "option_menu.h"

OptionMenu::OptionMenu(OptionMenu *parent, std::vector<Option> *options, bool isRomCfg) {

    this->parent = parent;

    if (options == NULL) {
        return;
    }

    OptionMenu *menu = this;

    for (unsigned int i = 0; i < options->size(); i++) {

        Option option = options->at(i);

        if (option.flags & Option::Type::HIDDEN
            || option.index == Option::Index::END) {
            continue;
        }

        if (option.flags & Option::Type::MENU) {
            if (i == 0 && !isRomCfg) {
                menu->title = option.getName();
            } else {
                menu = new OptionMenu(this, NULL);
                menu->title = option.getName();
                childs.push_back(menu);
            }
        } else {
            menu->option_ids.push_back(option.index);
        }
    }
}

void OptionMenu::addChild(const std::string &title) {

    OptionMenu *m = new OptionMenu(NULL, NULL);
    m->title = title;
    childs.push_back(m);
}

OptionMenu::~OptionMenu() {

    for (unsigned int i = 0; i < childs.size(); i++) {
        delete (childs[i]);
    }

    childs.clear();
    option_ids.clear();
}
