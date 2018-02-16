//
// Created by cpasjuste on 16/02/18.
//

#include "gui_state.h"
#include "gui_emu.h"
#include "gui_romlist.h"

using namespace c2d;

class GUISaveState : public Rectangle {

public:

    GUISaveState(Gui *ui, const FloatRect &rect, int num, const char *zipName) : Rectangle(rect) {

        setOriginCenter();
        setFillColor(Color::GrayLight);
        setOutlineColor(COL_GREEN);
        setOutlineThickness(2);

        memset(path, 0, MAX_PATH);
        memset(shot, 0, MAX_PATH);
        sprintf(path, "%s/%s%i.sav", szAppSavePath, zipName, num);
        sprintf(shot, "%s/%s%i.png", szAppSavePath, zipName, num);

        exist = ui->getIo()->exist(path);
        if (exist) {
            if (ui->getIo()->exist(shot)) {
                texture = (Texture *) new C2DTexture(shot);
                if (texture->available) {
                    add(texture);
                } else {
                    delete (texture);
                    texture = NULL;
                    middle_text = new C2DText("NO PIC", *ui->getFont(), (unsigned int) ui->getFontSize());
                    middle_text->setOutlineThickness(2);
                    middle_text->setOriginCenter();
                    middle_text->setPosition(getLocalBounds().left + getSize().x / 2,
                                             getLocalBounds().top + getSize().y / 2);
                    add(middle_text);
                }
            }
        } else {
            middle_text = new C2DText("NO SAVE", *ui->getFont(), (unsigned int) ui->getFontSize());
            middle_text->setOutlineThickness(2);
            middle_text->setOriginCenter();
            middle_text->setPosition(getLocalBounds().left + getSize().x / 2,
                                     getLocalBounds().top + getSize().y / 2);
            add(middle_text);
        }

        bottom_text = new C2DText("SLOT " + std::to_string(num),
                                  *ui->getFont(), (unsigned int) ui->getFontSize());
        bottom_text->setOutlineThickness(2);
        bottom_text->setOrigin(bottom_text->getLocalBounds().width / 2, 0);
        bottom_text->setPosition(getLocalBounds().left + getSize().x / 2,
                                 getLocalBounds().top + getLocalBounds().width + 8);
        add(bottom_text);
    }

    Texture *texture = NULL;
    Text *middle_text = NULL;
    Text *bottom_text = NULL;
    char path[MAX_PATH];
    char shot[MAX_PATH];
    bool exist = false;
};

GuiState::GuiState(Gui *ui) : Rectangle(Vector2f(0, 0)) {

    this->ui = ui;

    setFillColor(Color::Gray);
    setOutlineColor(COL_ORANGE);
    setOutlineThickness(4);
    setPosition(UI_MARGIN * ui->getScaling(), UI_MARGIN * ui->getScaling());
    setSize(ui->getRenderer()->getSize().x - (UI_MARGIN * ui->getScaling() * 2),
            ui->getRenderer()->getSize().y - (UI_MARGIN * ui->getScaling() * 2));

    //setVisibility(C2D_VISIBILITY_HIDDEN);
    load();
}

void GuiState::load() {

    isEmuRunning = ui->getUiEmu()->getVisibility() == C2D_VISIBILITY_VISIBLE;
    if (isEmuRunning) {
        // if frameskip is enabled, we may get a black buffer,
        // force a frame to be drawn
        if (pBurnDraw == NULL) {
            ui->getUiEmu()->resume();
            ui->getUiEmu()->renderFrame();
            ui->getUiEmu()->pause();
        }
    }

    float width = getSize().x / 5;
    for (int i = 0; i < 4; i++) {
        FloatRect rect = {width + (width * i), width, width, width};
        states[i] = new GUISaveState(ui, rect, i, ui->getUiRomList()->getRom()->name);
        add(states[i]);
    }

    state_index = 0;
    setSelection(0);

    setVisibility(C2D_VISIBILITY_VISIBLE);
}

void GuiState::unload() {

    for (int i = 0; i < 4; i++) {
        delete (states[i]);
    }

    setVisibility(C2D_VISIBILITY_HIDDEN);
}

void GuiState::setSelection(int index) {

    if (index < 0 || index > 3) {
        return;
    }

    for (int i = 0; i < 4; i++) {
        states[i]->setOutlineColor(i == index ? COL_YELLOW : COL_GREEN);
        states[i]->setOutlineThickness(i == index ? 3 : 1);
        states[i]->setLayer(i == index ? 1 : 0);
    }
}

int GuiState::update() {

    int ret = 0;
    int key = ui->getInput()->update()[0].state;

    if (key > 0) {

        if (key & Input::Key::KEY_LEFT) {
            state_index--;
            if (state_index < 0) {
                state_index = 3;
            }
            setSelection(state_index);
        } else if (key & Input::Key::KEY_RIGHT) {
            state_index++;
            if (state_index > 3) {
                state_index = 0;
            }
            setSelection(state_index);
        }

        // FIRE2
        if (key & Input::Key::KEY_FIRE2) {
            unload();
            if (isEmuRunning) {
                ret = UI_KEY_RESUME_ROM;
            } else {
                ret = UI_KEY_SHOW_ROMLIST;
            }
        }

        // QUIT
        if (key & EV_QUIT) {
            return EV_QUIT;
        }

        ui->getRenderer()->delay(INPUT_DELAY);
    }

    return ret;
}
