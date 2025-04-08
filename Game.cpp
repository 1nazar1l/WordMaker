﻿#include <SFML/Graphics.hpp>

#include "RandomWord.h"
#include "ValidWord.h"
#include "StartRound.h"

#include <unordered_map>
#include <algorithm>

using namespace std;
using namespace sf;

enum GameState {
    MENU,
    GAME
};

int main() {
    VideoMode desktop = VideoMode::getDesktopMode();
    RenderWindow window(desktop, "Game", Style::Fullscreen);

    Font font;
    if (!font.loadFromFile("arial.ttf")) {
        cerr << "Failed to load font!" << endl;
        return EXIT_FAILURE;
    }

    float windowWidth = static_cast<float>(desktop.width);
    float windowHeight = static_cast<float>(desktop.height);

    Texture startGameTexture;
    Texture settingsTexture;
    Texture exitTexture;

    if (!startGameTexture.loadFromFile("start_button.png") ||
        !settingsTexture.loadFromFile("start_button.png") ||
        !exitTexture.loadFromFile("start_button.png")) {
        cerr << "Failed to load button textures!" << endl;
        system("pause");
    }

    // Создаем спрайты для кнопок
    Sprite startGameButton(startGameTexture);
    Sprite settingsButton(settingsTexture);
    Sprite exitButton(exitTexture);

    startGameButton.setPosition(percentageX(50) - 100, percentageY(40));
    settingsButton.setPosition(percentageX(50) - 100, percentageY(60));
    exitButton.setPosition(percentageX(50) - 100, percentageY(80));

    Text startGameText, settingsText, exitText;

    // Настройка текста для кнопок с использованием процентных значений
    addInfoToWindow(startGameText, font, "Start Game", 60, Color::White, 25, 40);
    addInfoToWindow(settingsText, font, "Settings", 60, Color::White, 25, 60);
    addInfoToWindow(exitText, font, "Exit", 60, Color::White, 25, 80);

    // Обработка нажатий мыши
    Event event;
    GameState currentState = MENU; // Храним текущее состояние

    while (window.isOpen()) {
        Event event;
        while (window.pollEvent(event)) {
            if (event.type == Event::Closed) {
                window.close();
            }
            if (currentState == MENU) {
                if (event.type == Event::MouseButtonPressed) {
                    if (event.mouseButton.button == Mouse::Left) {
                        Vector2i mousePos = Mouse::getPosition(window);
                        if (currentState == MENU) { // Если у нас главное меню
                            if (startGameButton.getGlobalBounds().contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y))) {
                                cout << "Starting game..." << endl;
                                currentState = GAME; // Меняем состояние на игру
                            }
                            else if (settingsButton.getGlobalBounds().contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y))) {
                                cout << "Opening settings..." << endl;
                            }
                            else if (exitButton.getGlobalBounds().contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y))) {
                                window.close();
                            }
                        }
                    }
                }

                window.clear();

                window.draw(startGameButton);
                window.draw(settingsButton);
                window.draw(exitButton);
                window.draw(startGameText);
                window.draw(settingsText);
                window.draw(exitText);
            }
            else if (currentState == GAME) {
                StartRound(desktop, window, font, windowWidth, windowHeight);
            }
            
        }

        window.display();
    }
    
    return 0;
}