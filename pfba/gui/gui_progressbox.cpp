//
// Created by cpasjuste on 04/02/18.
//

#include "gui_progressbox.h"

using namespace c2d;

GuiProgressBox::GuiProgressBox(Gui *gui)
        : Rectangle(Vector2f(gui->getRenderer()->getSize().x / 2,
                             gui->getRenderer()->getSize().y / 2)) {

    float w = getSize().x;
    float h = getSize().y;

    setSize(w, h);
    setPosition(w, h);
    setOriginCenter();
    setFillColor(Color::GrayLight);
    setOutlineColor(COL_ORANGE);
    setOutlineThickness(2);

    float margin = UI_MARGIN * gui->getScaling();

    title = new Text("TITLE", *gui->getFont());
    title->setPosition(margin, margin + 16);
    title->setSizeMax(Vector2f(w - (margin * 2), 0));
    title->setOutlineThickness(2);
    title->setOutlineColor(Color::Black);
    add(title);

    progress_bg = new Rectangle(
            FloatRect(margin, h - margin - (h / 6),
                      w - (margin * 2), h / 6));
    progress_bg->setFillColor(Color::Gray);
    progress_bg->setOutlineColor(COL_GREEN);
    progress_bg->setOutlineThickness(2);
    add(progress_bg);

    progress_fg = new Rectangle(
            FloatRect(progress_bg->getPosition().x + 1,
                      progress_bg->getPosition().y + 1,
                      2,
                      progress_bg->getSize().y - 2));
    progress_fg->setFillColor(COL_YELLOW);
    add(progress_fg);

    message = new Text("MESSAGE", *gui->getFont(), (unsigned int) gui->getFontSize());
    message->setPosition(margin, progress_bg->getPosition().y - C2D_DEFAULT_CHAR_SIZE - 16);
    message->setSizeMax(Vector2f(w - (margin * 2), 0));
    message->setOutlineThickness(2);
    message->setOutlineColor(Color::Black);
    add(message);
}

void GuiProgressBox::setTitle(std::string title) {

    this->title->setString(title);
}

void GuiProgressBox::setProgress(float progress) {

    progress_fg->setSize(
            progress * (progress_bg->getSize().x - 2),
            progress_fg->getSize().y);
}

void GuiProgressBox::setMessage(std::string message) {

    this->message->setString(message);
}
