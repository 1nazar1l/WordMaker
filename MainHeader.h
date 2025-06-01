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

void addInfoToWindow(Text& text, Font& font, const string& str, int fontSize, Color color, float xPercentage, float yPercentage) {
    VideoMode desktop = VideoMode::getDesktopMode();
    int width = static_cast<float>(desktop.width);
    if (width < 1300) {
        fontSize = fontSize - (fontSize * 0.2);
    }
    else if (width >= 1300 && width < 1400) {
        fontSize = fontSize;
    }
    else if (width >= 1400 && width < 1500) {
        fontSize = fontSize + (fontSize * 0.2);
    }
    else if (width >= 1500 && width < 1600) {
        fontSize = fontSize + (fontSize * 0.3);
    }
    else if (width >= 1600 && width < 1700) {
        fontSize = fontSize + (fontSize * 0.4);
    }
    else if (width >= 1700 && width < 1800) {
        fontSize = fontSize + (fontSize * 0.5);
    }
    else if (width >= 1800 && width < 1900) {
        fontSize = fontSize + (fontSize * 0.6);
    }
    else if (width >= 1900 && width < 2000) {
        fontSize = fontSize + (fontSize * 0.7);
    }
    else if (width >= 2000 && width < 2100) {
        fontSize = fontSize + (fontSize * 0.8);
    }
    else if (width >= 2100 && width < 2200) {
        fontSize = fontSize + (fontSize * 0.9);
    }
    text.setFont(font);
    text.setString(str);
    text.setCharacterSize(fontSize);
    text.setFillColor(color);
    text.setPosition(percentageX(xPercentage) - text.getLocalBounds().width / 2, percentageY(yPercentage));
}

void closeEvents(Event& event, RenderWindow& window) {
    if (event.type == Event::Closed) {
        window.close();
    }
    if (event.type == Event::KeyPressed && event.key.code == Keyboard::Escape) {
        window.close();
    }
}

void createButtonHitBox(RectangleShape& rectangle, float width, float height, float xPercentage, float yPercentage) {
    rectangle.setSize(Vector2f(percentageX(width), percentageY(height)));
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

bool mouseIn(RenderWindow& window, CircleShape& btn) {
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

bool click(Event& event, RenderWindow& window, CircleShape& btn) {
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

int getVolumeValue(RenderWindow& window, bool& isDragging, RectangleShape& track, CircleShape& thumb) {

    if (isDragging) {
        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
        float newX = mousePos.x - thumb.getRadius();

        float minX = track.getPosition().x - thumb.getRadius();
        float maxX = track.getPosition().x + track.getSize().x - thumb.getRadius();
        newX = std::max(minX, std::min(newX, maxX));

        thumb.setPosition(newX, thumb.getPosition().y);

        float sliderWidth = track.getSize().x;
        float relativePos = (newX - minX) / sliderWidth;
        int value = static_cast<int>(relativePos * 100);

        static int lastValue = -1;
        if (value != lastValue) {
            lastValue = value;
            return lastValue;
        }
    }
}

void createThumb(RectangleShape& track, CircleShape& thumb, Color& color, float xPosition, float yPosition) {
    thumb.setFillColor(color);
    thumb.setPosition(percentageX(xPosition) - track.getLocalBounds().width / 2, percentageY(yPosition) - track.getLocalBounds().height / 2);
}