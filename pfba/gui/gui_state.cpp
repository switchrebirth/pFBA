//
// Created by cpasjuste on 16/02/18.
//

#include "gui_state.h"
#include "gui_emu.h"
#include "gui_romlist.h"

#define STATES_COUNT 4

using namespace c2d;

class GUISaveState : public Rectangle {

public:

    GUISaveState(Gui *ui, const FloatRect &rect, int num, const char *zipName) : Rectangle(rect) {

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

class GUISaveStateList : public Rectangle {

public:

    GUISaveStateList(Gui *ui, const FloatRect &rect) : Rectangle(rect) {

        //setFillColor(Color::Transparent);

        // add states items
        float width = getSize().x / STATES_COUNT;
        for (int i = 0; i < STATES_COUNT; i++) {
            FloatRect r = {(width * i) + (width / 2), width / 2, width, width};
            states[i] = new GUISaveState(ui, r, i, ui->getUiRomList()->getRom()->name);
            states[i]->setOriginCenter();
            add(states[i]);
        }

        setSelection(0);
    }

    ~GUISaveStateList() {
        for (int i = 0; i < 4; i++) {
            delete (states[i]);
        }
    }

    void setSelection(int idx) {

        if (idx < 0 || idx > STATES_COUNT) {
            return;
        }

        index = idx;
        for (int i = 0; i < STATES_COUNT; i++) {
            states[i]->setOutlineColor(i == index ? COL_YELLOW : COL_GREEN);
            states[i]->setOutlineThickness(i == index ? 6 : 1);
            states[i]->setLayer(i == index ? 1 : 0);
            float scale = i == index ? 1.0f : 0.9f;
            states[i]->setScale(scale, scale);
        }
    }

    void left() {
        index--;
        if (index < 0) {
            index = STATES_COUNT - 1;
        }
        setSelection(index);
    }

    void right() {
        index++;
        if (index > STATES_COUNT - 1) {
            index = 0;
        }
        setSelection(index);
    }

    GUISaveState *states[STATES_COUNT];
    int index = 0;
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

    // TODO: LOAD STATES
    uiStateList = new GUISaveStateList(ui, {
            getLocalBounds().left + getSize().x / 2,
            getLocalBounds().top + getSize().y / 2,
            getSize().x - 64, getSize().x / (STATES_COUNT + 1)
    });
    uiStateList->setOriginCenter();
    add(uiStateList);

    setVisibility(C2D_VISIBILITY_VISIBLE);
}

void GuiState::unload() {

    delete (uiStateList);
    setVisibility(C2D_VISIBILITY_HIDDEN);
}

int GuiState::update() {

    int ret = 0;
    int key = ui->getInput()->update()[0].state;

    if (key > 0) {

        if (key & Input::Key::KEY_LEFT) {
            uiStateList->left();
        } else if (key & Input::Key::KEY_RIGHT) {
            uiStateList->right();
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
