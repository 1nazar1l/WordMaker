#include <SFML/Graphics.hpp>
#include <iostream>
#include <sstream>

#include "MainHeader.h"
#include "RandomWord.h"
#include "ValidWord.h"

#include <unordered_map>
#include <algorithm>

using namespace std;
using namespace sf;

unordered_map<char, int> createLetterMap(const string& word) {
    unordered_map<char, int> letters;
    for (char c : word) {
        letters[tolower(c)]++;
    }
    return letters;
}

void updateTimer(Clock& gameClock, int& timeRemaining, Text& timerText) {
    Time elapsed = gameClock.getElapsedTime();
    if (elapsed.asSeconds() >= 1.0f && timeRemaining > 0) {
        timeRemaining--;
        gameClock.restart();

        // Обновляем текст таймера
        stringstream ss;
        ss << "Time: " << timeRemaining;
        timerText.setString(ss.str());
    }
}

void handleMenuEvents(string& gameStage, Text& startGameText, Event& event, Vector2i& mousePos) {
    if (event.type == Event::MouseButtonPressed) {
        if (event.mouseButton.button == Mouse::Left) {
            if (startGameText.getGlobalBounds().contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y))) {
                gameStage = "GAME"; // Переход в режим игры
                cout << "game1\n";
            }
        }
    }
}

void updateMenu(RenderWindow& window, Text& startGameText) {
    window.clear(Color::Black);
    window.draw(startGameText);
    window.display();
}

void startMenu(RenderWindow& window, string& gameStage, Text& startGameText) {
    Event event;
    while (window.pollEvent(event)) {
        closeEvents(event, window);

        Vector2i mousePos = Mouse::getPosition(window);

        handleMenuEvents(gameStage, startGameText, event, mousePos);
    }

    updateMenu(window, startGameText);
}

string getRandomWord(string randomWordsFile) {
    RandomWord randomWord(randomWordsFile);
    randomWord.loadWords();
    return randomWord.getRandomWord();
}

int main() {
    bool breakPage = false;
    string gameStage = "MENU";
    VideoMode desktop = VideoMode::getDesktopMode();
    RenderWindow window(desktop, "Game", Style::Fullscreen);

    Font font;
    if (!font.loadFromFile("arial.ttf")) {
        return EXIT_FAILURE;
    }

    // Menu text
    Text startGameText;
    addInfoToWindow(startGameText, font, "Start Game", 40, Color::White, 0, 0, -10, -10);

    // Game text
    ValidWord validator("validWords.txt");
    validator.loadWords();

    // Перенесем объявление переменных сюда
    string targetWord;
    Text counterText;
    Text targetText;
    Text inputText;
    Text lettersText;
    Text guessedText;
    Text endGameText;
    Text timerText;
    unsigned int counter = 0;
    string playerInput;
    unordered_map<char, int> availableLetters;
    unordered_map<char, int> currentLetters;
    string lettersInfo;
    string guessedWords[100];
    int guessedCount = 0;

    // Timer variables
    Clock gameClock;
    int timeRemaining = 30;

    while (window.isOpen()) {
        if (gameStage == "MENU") {
            // Сброс таймера при возврате в меню
            timeRemaining = 31;
            timerText.setString("Time: 30");
            inputText.setString("Your input: ");
            startMenu(window, gameStage, startGameText);
        }
        else if (gameStage == "GAME") {
            // Генерация нового слова при переходе в игровой режим
            targetWord = getRandomWord("easyRandomWords.txt");
            if (targetWord.empty()) {
                cerr << "No words available!" << endl;
                return EXIT_FAILURE;
                continue;
            }

            // Сброс игровых переменных
            counter = 0;
            playerInput.clear();
            guessedCount = 0;

            // Обновление доступных букв
            availableLetters = createLetterMap(targetWord);
            currentLetters = availableLetters;

            // Обновление текстовых элементов
            lettersInfo = "Available letters: ";
            for (auto& pair : availableLetters) {
                lettersInfo += string(1, pair.first) + "(" + to_string(pair.second) + ") ";
            }

            addInfoToWindow(counterText, font, "Count: " + to_string(counter), 24, Color::White, 20, 20);
            addInfoToWindow(targetText, font, "Word: " + targetWord, 30, Color::Green, 20, 60);
            addInfoToWindow(inputText, font, "Your input: ", 40, Color::White, 20, 40);
            addInfoToWindow(lettersText, font, lettersInfo, 20, Color::Yellow, 20, 50);
            addInfoToWindow(guessedText, font, "", 20, Color::Cyan, 60, 40);
            addInfoToWindow(endGameText, font, "End Game", 40, Color::White, 0, 0, -10, -10);
            addInfoToWindow(timerText, font, "Time: 30", 40, Color::White, 10, 10);

            gameClock.restart(); // Сброс таймера

            // Основной игровой цикл
            while (gameStage == "GAME" && window.isOpen()) {
                Event event;
                updateTimer(gameClock, timeRemaining, timerText);

                while (window.pollEvent(event)) {
                    closeEvents(event, window);
                    Vector2i mousePos = Mouse::getPosition(window);

                    if (event.type == Event::MouseButtonPressed) {
                        if (event.mouseButton.button == Mouse::Left) {
                            if (endGameText.getGlobalBounds().contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y))) {
                                gameStage = "MENU";
                                cout << "game2\n";
                                break; // Выходим из внутреннего цикла
                            }
                        }
                    }

                    // Обработка ввода
                    if (event.type == Event::TextEntered) {
                        // Обработка backspace
                        if (event.text.unicode == '\b') {
                            if (!playerInput.empty()) {
                                char lastChar = playerInput.back();
                                currentLetters[lastChar]++;  // Возвращаем букву в доступные
                                playerInput.pop_back();
                            }
                        }
                        // Обработка обычных символов
                        else if (isalpha(static_cast<char>(event.text.unicode))) {
                            char c = tolower(static_cast<char>(event.text.unicode));
                            if (currentLetters[c] > 0) {
                                playerInput += c;
                                currentLetters[c]--;
                            }
                        }
                        // Всегда обновляем текст, даже если просто backspace нажали
                        inputText.setString("Your input: " + playerInput);
                    }

                    // Обработка подтверждения слова по Enter
                    if (event.type == Event::KeyPressed && event.key.code == Keyboard::Enter && !playerInput.empty()) {
                        if (validator.isValid(playerInput)) {
                            bool alreadyGuessed = false;
                            for (int i = 0; i < guessedCount; ++i) {
                                if (guessedWords[i] == playerInput) {
                                    alreadyGuessed = true;
                                    break;
                                }
                            }

                            if (!alreadyGuessed && guessedCount < 100) {
                                guessedWords[guessedCount++] = playerInput;
                                counter += playerInput.length();

                                string guessedStr = "Guessed words:\n";
                                for (int i = 0; i < guessedCount; ++i) {
                                    guessedStr += guessedWords[i] + "\n";
                                }
                                guessedText.setString(guessedStr);
                                counterText.setString("Count: " + to_string(counter));
                            }
                        }

                        // Сброс после подтверждения слова
                        currentLetters = availableLetters;
                        playerInput.clear();
                        inputText.setString("Your input: ");
                    }
                }

                if (gameStage != "GAME") break; // Выходим, если перешли в меню

                // Отрисовка
                window.clear(Color::Black);
                window.draw(counterText);
                window.draw(targetText);
                window.draw(inputText);
                window.draw(lettersText);
                window.draw(guessedText);
                window.draw(timerText);
                window.draw(endGameText);
                window.display();

                // Проверка времени
                if (timeRemaining <= 0) {
                    gameStage = "MENU";
                }
            }
        }
    }

    return 0;
}