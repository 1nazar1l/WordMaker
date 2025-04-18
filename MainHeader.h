#pragma once

#include <SFML/Graphics.hpp>
#include <iostream>
#include <cstdlib>

using namespace std;
using namespace sf;

float percentageX(float percentage) {
    VideoMode desktop = VideoMode::getDesktopMode();
    return (percentage / 100.0f) * static_cast<float>(desktop.width);
}

float percentageY(float percentage) {
    VideoMode desktop = VideoMode::getDesktopMode();
    return (percentage / 100.0f) * static_cast<float>(desktop.height);
}

void addInfoToWindow(Text& text, Font& font, const string& str, int fontSize, Color color, float xPercentage, float yPercentage, int paddingW = 0, int paddingH = 0) {
    text.setFont(font);
    text.setString(str);
    text.setCharacterSize(fontSize);
    text.setFillColor(color);
    text.setPosition(percentageX(xPercentage) - paddingW, percentageY(yPercentage) - paddingH);
}

void closeEvents(Event& event, RenderWindow& window) {
    if (event.type == Event::Closed) {
        window.close();
    }
    if (event.type == Event::KeyPressed && event.key.code == Keyboard::Escape) {
        window.close();
    }
}