//
// Created by cpasjuste on 16/02/17.
//

#ifndef PFBA_MENU_H
#define PFBA_MENU_H

#include <vector>
#include "option.h"

class OptionMenu {

public:

    OptionMenu(OptionMenu *parent, std::vector<Option> *options, bool isRomCfg = false);
    ~OptionMenu();

    void addChild(const std::string &title);

    std::string title;
    std::vector<int> option_ids;
    std::vector<OptionMenu*> childs;
    OptionMenu *parent = NULL;

private:

};


#endif //PFBA_MENU_H
