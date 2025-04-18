#include <SFML/Graphics.hpp>
#include <iostream>
#include <sstream>
#include <string>

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

void updateTimer(Clock& gameClock, int& timeRemaining, Text& timerText, bool isPaused) {
    if (isPaused) return; // Не обновляем таймер на паузе

    Time elapsed = gameClock.getElapsedTime();
    if (elapsed.asSeconds() >= 1.0f && timeRemaining > 0) {
        timeRemaining--;
        gameClock.restart();
        stringstream ss;
        ss << "Time: " << timeRemaining;
        timerText.setString(ss.str());
    }
}

string getRandomWord(string randomWordsFile) {
    RandomWord randomWord(randomWordsFile);
    randomWord.loadWords();
    return randomWord.getRandomWord();
}

int main() {
    bool breakPage = false;
    bool isPaused = false; // Добавляем флаг паузы

    string gameStage = "MENU";
    VideoMode desktop = VideoMode::getDesktopMode();
    RenderWindow window(desktop, "Game", Style::Fullscreen);

    Font font;
    if (!font.loadFromFile("fonts/font1.ttf")) {
        return EXIT_FAILURE;
    }

    // Menu text
    Text startGameText;
    Text settingsText;
    addInfoToWindow(startGameText, font, "Start Game", 40, Color::White, 50, 50, 130);
    addInfoToWindow(settingsText, font, "Settings", 40, Color::White, 50, 60, 100);

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
    Text pauseText;
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


            Event event;
            while (window.pollEvent(event)) {
                closeEvents(event, window);

                Vector2i mousePos = Mouse::getPosition(window);

                if (event.type == Event::MouseButtonPressed) {
                    if (event.mouseButton.button == Mouse::Left) {
                        if (startGameText.getGlobalBounds().contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y))) {
                            gameStage = "GAME"; 
                            cout << "game1\n";
                        }
                        else if (settingsText.getGlobalBounds().contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y))) {
                            gameStage = "SETTINGS";
                        }
                    }
                }
            }
            Color backgroundColor(123, 205, 186);
            window.clear(backgroundColor);
            window.draw(startGameText);
            window.draw(settingsText);
            window.display();
        }
        else if (gameStage == "SETTINGS") {
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
            addInfoToWindow(pauseText, font, "Pause", 40, Color::White, 90, 90);

            gameClock.restart(); // Сброс таймера

            // Основной игровой цикл
            while (gameStage == "GAME" && window.isOpen()) {
                Event event;
                updateTimer(gameClock, timeRemaining, timerText, isPaused);
                if (isPaused) {
                    inputText.setFillColor(Color(150, 150, 150)); // Серый цвет при паузе
                }
                else {
                    inputText.setFillColor(Color::White); // Белый цвет при активной игре
                }
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
                            if (pauseText.getGlobalBounds().contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y))) {
                                isPaused = !isPaused; // Переключаем состояние паузы
                                if (isPaused) {
                                    addInfoToWindow(pauseText, font, "Resume", 40, Color::White, 90, 90, 20);
                                }
                                else {
                                    addInfoToWindow(pauseText, font, "Pause", 40, Color::White, 90, 90);
                                    gameClock.restart(); // Перезапускаем таймер при снятии паузы
                                }
                            }
                        }

                    }

                    // Обработка ввода
                    if (event.type == Event::TextEntered && !isPaused) {  // Добавляем условие !isPaused
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
                    if (event.type == Event::KeyPressed && event.key.code == Keyboard::Enter && !playerInput.empty() && !isPaused) {
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

                if (gameStage != "GAME") {
                    isPaused = false;
                    break; // Выходим, если перешли в меню
                }
                // Отрисовка
                window.clear(Color::Black);
                window.draw(counterText);
                window.draw(targetText);
                window.draw(inputText);
                window.draw(lettersText);
                window.draw(guessedText);
                window.draw(timerText);
                window.draw(pauseText);
                if (!isPaused) {
                    addInfoToWindow(endGameText, font, "End Game", 40, Color::White, 0, 0, -10, -10);
                    window.draw(endGameText);
                }
                else {
                    addInfoToWindow(endGameText, font, "End Game", 40, Color::White, 0, 0, 100, 100);
                    window.draw(endGameText);
                }
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