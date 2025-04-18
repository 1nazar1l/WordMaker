#pragma once
#include <SFML/Graphics.hpp>

class CursorManager {
public:
    CursorManager();
    bool loadTextures(const std::string& defaultCursorPath, const std::string& hoverCursorPath);
    void update(const sf::Vector2i& mousePos, bool isHovering);
    void draw(sf::RenderWindow& window);

private:
    sf::Texture defaultCursorTex;
    sf::Texture hoverCursorTex;
    sf::Sprite currentCursor;
    bool isHovering;
};

CursorManager::CursorManager() : isHovering(false) {}

bool CursorManager::loadTextures(const std::string& defaultCursorPath, const std::string& hoverCursorPath) {
    if (!defaultCursorTex.loadFromFile(defaultCursorPath) ||
        !hoverCursorTex.loadFromFile(hoverCursorPath)) {
        return false;
    }
    currentCursor.setTexture(defaultCursorTex);
    return true;
}

void CursorManager::update(const sf::Vector2i& mousePos, bool hovering) {
    isHovering = hovering;
    if (isHovering) {
        currentCursor.setTexture(hoverCursorTex);
    }
    else {
        currentCursor.setTexture(defaultCursorTex);
    }
    currentCursor.setPosition(static_cast<float>(mousePos.x),
        static_cast<float>(mousePos.y));
}

void CursorManager::draw(sf::RenderWindow& window) {
    window.draw(currentCursor);
}