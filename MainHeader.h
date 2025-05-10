#pragma once

#include <SFML/Graphics.hpp>
#include <iostream>
#include <cstdlib>
#include "CursorManager.h"

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
    text.setPosition(percentageX(xPercentage) - text.getLocalBounds().width / 2, percentageY(yPercentage) - paddingH);
}

void closeEvents(Event& event, RenderWindow& window) {
    if (event.type == Event::Closed) {
        window.close();
    }
    if (event.type == Event::KeyPressed && event.key.code == Keyboard::Escape) {
        window.close();
    }
}

void createButtonHitBox(RectangleShape& rectangle, int width, int height, float xPercentage, float yPercentage) {
    rectangle.setSize(Vector2f(width, height));
    rectangle.setFillColor(Color(0, 0, 0, 0));
    rectangle.setOutlineColor(Color::Black);
    //rectangle.setOutlineThickness(2.f);
    rectangle.setPosition(percentageX(xPercentage) - rectangle.getLocalBounds().width / 2, percentageY(yPercentage));
}

void updateBackground(RenderWindow& window, Texture& bgTexture, Sprite& bgSprite, string& filename) {
    if (!bgTexture.loadFromFile(filename)) {
        std::cerr << "Failed to load background image!" << std::endl;
    }
    bgSprite.setTexture(bgTexture);

    sf::Vector2u windowSize = window.getSize();
    float scaleX = static_cast<float>(windowSize.x) / bgTexture.getSize().x;
    float scaleY = static_cast<float>(windowSize.y) / bgTexture.getSize().y;
    bgSprite.setPosition(1, -1);
    bgSprite.setScale(scaleX, scaleY);
}

void drawCursor(RenderWindow& window, CursorManager& cursor, bool& isHover) {
    Vector2i mousePos = Mouse::getPosition(window);
    cursor.update(mousePos, isHover);
    cursor.draw(window);
}

bool mouseIn(RenderWindow& window, RectangleShape& btn) {
    Vector2i mousePos = Mouse::getPosition(window);
    return btn.getGlobalBounds().contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));
}

bool mouseIn(RenderWindow& window, Text& btn) {
    Vector2i mousePos = Mouse::getPosition(window);
    return btn.getGlobalBounds().contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));
}

bool click(Event& event, RenderWindow& window, RectangleShape& btn) {
    return (event.type == Event::MouseButtonPressed && event.mouseButton.button == Mouse::Left) && mouseIn(window, btn);
}

bool click(Event& event, RenderWindow& window, Text& btn) {
    return (event.type == Event::MouseButtonPressed && event.mouseButton.button == Mouse::Left) && mouseIn(window, btn);
}

bool notclick(Event& event, RenderWindow& window, Text& btn) {
    return (event.type == Event::MouseButtonPressed && event.mouseButton.button == Mouse::Left) && !mouseIn(window, btn);
}

bool notclick(Event& event, RenderWindow& window, RectangleShape& btn) {
    return (event.type == Event::MouseButtonPressed && event.mouseButton.button == Mouse::Left) && !mouseIn(window, btn);
}