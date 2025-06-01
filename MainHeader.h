#pragma once

#include <SFML/Graphics.hpp>
#include <iostream>
#include <cstdlib>
#include "CursorManager.h"
#include "RandomWord.h"


using namespace std;
using namespace sf;

std::string hashPassword(const std::string& password) {
    size_t hash = std::hash<std::string>{}(password);

    hash = (hash << 5) + hash;
    const char* hexDigits = "0123456789ABCDEF";
    std::string hashedPassword;

    for (size_t i = 0; i < sizeof(size_t); ++i) {
        unsigned char byte = (hash >> (8 * i)) & 0xFF;
        hashedPassword += hexDigits[(byte >> 4) & 0xF];
        hashedPassword += hexDigits[byte & 0xF];
    }

    return hashedPassword;
}

unordered_map<char, int> createLetterMap(const string& word) {
    unordered_map<char, int> letters;
    for (char c : word) {
        letters[tolower(c)]++;
    }
    return letters;
}

void updateTimer(Clock& gameClock, int& timeRemaining, Text& timerText, bool isPaused) {
    if (isPaused) return;

    Time elapsed = gameClock.getElapsedTime();
    if (elapsed.asSeconds() >= 1.0f && timeRemaining > 0) {
        timeRemaining--;
        gameClock.restart();
        stringstream ss;
        ss << "Timer:  " << timeRemaining;
        timerText.setString(ss.str());
    }
}

string getRandomWord(string randomWordsFile) {
    RandomWord randomWord(randomWordsFile);
    randomWord.loadWords();
    return randomWord.getRandomWord();
}

int updateIndex(int index, const int& maxIndex, string minusOrPlus) {
    if (minusOrPlus == "-") {
        index -= 1;
        if (index < 0) {
            index = maxIndex - 1;
        }
    }
    else if (minusOrPlus == "+") {
        index += 1;
        if (index >= maxIndex) {
            index = 0;
        }
    }
    return index;
}

int getCurrentIndex(const int& length, int massiv[], int selectedParametr) {
    for (int i = 0; i < length;i++) {
        if (massiv[i] == selectedParametr) {
            return i;
            break;
        }
    }
}

int getCurrentIndexStr(const int& length, string massiv[], string selectedParametr) {
    for (int i = 0; i < length;i++) {
        if (massiv[i] == selectedParametr) {
            return i;
            break;
        }
    }
}

std::string formatFloat(float num) {
    std::stringstream ss;
    ss << num;
    std::string s = ss.str();

    size_t dotPos = s.find('.');
    if (dotPos != std::string::npos) {
        s.erase(s.find_last_not_of('0') + 1, std::string::npos);
        if (s.back() == '.') {
            s.pop_back();
        }
    }
    return s;
}

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

int getVolumeValue(RenderWindow& window, RectangleShape& track, CircleShape& thumb) {
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

void createThumb(RectangleShape& track, CircleShape& thumb, Color& color, float xPosition, float yPosition) {
    thumb.setFillColor(color);
    thumb.setPosition(percentageX(xPosition) - track.getLocalBounds().width / 2, percentageY(yPosition) - track.getLocalBounds().height / 2);
}

void setThumbPosition(RenderWindow& window, RectangleShape& track, CircleShape& thumb, int& volume) {
    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
    float minX = track.getPosition().x - thumb.getRadius();
    float sliderWidth = track.getSize().x;

    thumb.setPosition(minX + (sliderWidth * volume / 100.f), thumb.getPosition().y);
}