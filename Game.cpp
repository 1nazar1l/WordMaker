#include <SFML/Graphics.hpp>
#include <iostream>
#include <sstream>
#include <string>

#include "MainHeader.h"
#include "RandomWord.h"
#include "ValidWord.h"
#include "CursorManager.h"

#include <unordered_map>
#include <algorithm>

#include "json.hpp"

using json = nlohmann::json;
using namespace std;
using namespace sf;

unordered_map<char, int> createLetterMap(const string& word) {
    unordered_map<char, int> letters;
    for (char c : word) {
        letters[tolower(c)]++;
    }
    return letters;
}

void createButtonHitBox(RectangleShape& rectangle, int width, int height, int xPos, int yPos) {
    rectangle.setSize(Vector2f(width, height));
    rectangle.setFillColor(Color(0, 0, 0, 0));
    rectangle.setOutlineColor(Color::White);
    //rectangle.setOutlineThickness(2.f);
    rectangle.setPosition(xPos, yPos);

}

void updateTimer(Clock& gameClock, int& timeRemaining, Text& timerText, bool isPaused) {
    if (isPaused) return; // Не обновляем таймер на паузе

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

void updateBackground(RenderWindow& window, Texture& bgTexture, Sprite& bgSprite, string& filename) {
    if (!bgTexture.loadFromFile(filename)) {
        std::cerr << "Failed to load background image!" << std::endl;
    }

    // Создание спрайта
    bgSprite.setTexture(bgTexture);

    // Масштабирование под размер окна
    sf::Vector2u windowSize = window.getSize();
    float scaleX = static_cast<float>(windowSize.x) / bgTexture.getSize().x;
    float scaleY = static_cast<float>(windowSize.y) / bgTexture.getSize().y;
    bgSprite.setPosition(1, -1);
    bgSprite.setScale(scaleX, scaleY);
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

bool mouseIn(RenderWindow& window, RectangleShape& btn) {
    Vector2i mousePos = Mouse::getPosition(window);
    return btn.getGlobalBounds().contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));
}

void drawCursor(RenderWindow& window, CursorManager& cursor, bool& isHover) {
    Vector2i mousePos = Mouse::getPosition(window);
    cursor.update(mousePos, isHover);
    cursor.draw(window);
}

bool click(Event& event, RenderWindow& window, RectangleShape& btn) {
    return (event.type == Event::MouseButtonPressed && event.mouseButton.button == Mouse::Left) && mouseIn(window, btn);
}

int main() {
    ifstream in("settings/settings.json");
    json settings = json::parse(in);
    int theme = settings["theme"];
    string difficulty = settings["difficulty"];
    int roundTime = settings["round_time"];

    bool isPaused = false; // Добавляем флаг паузы
    bool anyButtonHovered = false;

    string gameStage = "MENU";
    VideoMode desktop = VideoMode::getDesktopMode();
    RenderWindow window(desktop, "Game", Style::Fullscreen);

    Font font;
    if (!font.loadFromFile("fonts/font1.ttf")) {
        return EXIT_FAILURE;
    }

    // Menu text
    struct MenuTexts {
        Text startGame;
        Text settings;
        Text leaderboard;
        Text exit;
    };

    struct MenuBg {
        Texture texture;
        Sprite sprite;
    };

    struct MenuButtons {
        RectangleShape start;
        RectangleShape settings;
        RectangleShape leaderBoard;
        RectangleShape exit;
    };

    MenuTexts menuT;
    MenuBg menuBg;
    MenuButtons menuBtn;

    string menuFilename = "backgrounds/menu" + to_string(theme) + ".png";
    updateBackground(window, menuBg.texture, menuBg.sprite, menuFilename);


    addInfoToWindow(menuT.startGame, font, "Start Game", 36, Color(226, 207, 234), 0, 0);
    addInfoToWindow(menuT.settings, font, "Settings", 36, Color(226, 207, 234), 0, 0);
    addInfoToWindow(menuT.leaderboard, font, "LeaderBoard", 30, Color(226, 207, 234), 0, 0);
    addInfoToWindow(menuT.exit, font, "Exit", 36, Color(226, 207, 234), 0, 0);

    menuT.startGame.setPosition(535, 113);
    menuT.settings.setPosition(570, 279);
    menuT.leaderboard.setPosition(535, 450);
    menuT.exit.setPosition(630, 611);

    createButtonHitBox(menuBtn.start, 343, 120, 511, 75);
    createButtonHitBox(menuBtn.settings, 343, 120, 511, 241);
    createButtonHitBox(menuBtn.leaderBoard, 343, 120, 511, 407);
    createButtonHitBox(menuBtn.exit, 343, 120, 511, 573);

    CursorManager cursorManager;
    if (!cursorManager.loadTextures("cursors/defaultcursor.png", "cursors/hovercursor.png")) {
        cerr << "Failed to load cursor textures!" << endl;
        return EXIT_FAILURE;
    }



    // Game text
    Texture gameTexture;
    Sprite gameSprite;
    string gameFilename = "backgrounds/game" + to_string(theme) + ".png";

    ValidWord validator("validWords.txt");
    validator.loadWords();

    // Перенесем объявление переменных сюда
    string targetWord;
    Text counterText;
    Text targetText;
    Text inputText;
    Text lettersText;
    Text endGameText;
    Text timerText;
    Text pauseText;
    unsigned int counter = 0;
    string playerInput;
    unordered_map<char, int> availableLetters;
    unordered_map<char, int> currentLetters;
    string guessedWords[100];
    int guessedCount = 0;
    
    updateBackground(window, gameTexture, gameSprite, gameFilename);

    RectangleShape pauseButtonHitBox;
    createButtonHitBox(pauseButtonHitBox, 262, 100, 40, 20);


    // Timer variables
    Clock gameClock;
    int timeRemaining = roundTime;

    //Endgame
    Texture endgameTexture;
    Sprite endgameSprite;
    string endgameFilename = "backgrounds/endgame" + to_string(theme) + ".png";

    Text restartText;
    Text exitTextEnd;
    Text scoreText;
    Text isrecordText;

    addInfoToWindow(restartText, font, "Restart", 50, Color(226, 207, 234), 39.3, 62.5);
    addInfoToWindow(exitTextEnd, font, "Exit", 50, Color(226, 207, 234), 45, 82.5);

    addInfoToWindow(scoreText, font, "Your score: ", 30, Color(226, 207, 234), 35, 10);
    addInfoToWindow(isrecordText, font, "New record!!!", 20, Color(226, 207, 234), 35, 15);

    RectangleShape restartButtonHitBox;
    RectangleShape exitToMenuButtonHitBox;

    createButtonHitBox(restartButtonHitBox, 496, 120, 435, 451);
    createButtonHitBox(exitToMenuButtonHitBox, 496, 120, 435, 602);

    updateBackground(window, endgameTexture, endgameSprite, endgameFilename);

    //Settings
    struct SettingsTexts {
        Text exitToMenu;
        Text save;
        Text timerParam;
        Text difficultyParam;
        Text musicParam;
        Text themeParam;
    };

    struct SettingsBg {
        Texture texture;
        Sprite sprite;
    };

    struct SettingsButtons {
        RectangleShape exitToMenu;
        RectangleShape save;
        RectangleShape leftStrokes[5];
        RectangleShape rightStrokes[5];
    };

    SettingsTexts settingsT;
    SettingsBg settingsBg;
    SettingsButtons settingsBtn;

    string settingsFilename = "backgrounds/settings" + to_string(theme) + ".png";
    updateBackground(window, settingsBg.texture, settingsBg.sprite, settingsFilename);

    addInfoToWindow(settingsT.exitToMenu, font, "Exit", 50, Color(226, 207, 234), 5, 6);
    addInfoToWindow(settingsT.timerParam, font, "Timer", 30, Color(160, 108, 213), 35, 6.8);
    addInfoToWindow(settingsT.difficultyParam, font, "Difficulty", 28, Color(160, 108, 213), 35, 16.3);
    addInfoToWindow(settingsT.musicParam, font, "Music", 30, Color(160, 108, 213), 35, 25.8);
    addInfoToWindow(settingsT.themeParam, font, "Theme", 30, Color(160, 108, 213), 35, 35.3);
    addInfoToWindow(settingsT.save, font, "Save", 50, Color(226, 207, 234), 44, 85);

    const int timesCount = 4;
    const int difCount = 3;
    const int musicCount = 4;
    const int themeCount = 4;

    int timesToRound[timesCount]{ 30, 60, 90, 120 };
    string difToRound[3]{ "easy", "normal", "hard" };
    int musicToRound[4]{ 1, 2, 3, 4 };
    int themeToRound[4]{ 1, 2, 3, 4 };

    int timeIndex = 0;
    int difIndex = 0;
    int musicIndex = 0;
    int themeIndex = 0;

    Text timerOption;
    Text difOption;
    Text musicOption;
    Text themeOption;

    addInfoToWindow(timerOption, font, to_string(timesToRound[timeIndex]), 25, Color(226, 207, 234), 57, 7);
    addInfoToWindow(difOption, font, difToRound[difIndex], 25, Color(226, 207, 234), 55.5, 16.5);
    addInfoToWindow(musicOption, font, to_string(musicToRound[musicIndex]), 25, Color(226, 207, 234), 58, 26);
    addInfoToWindow(themeOption, font, to_string(themeToRound[themeIndex]), 25, Color(226, 207, 234), 58, 35.5);

    createButtonHitBox(settingsBtn.exitToMenu, 200, 100, 40, 24);
    createButtonHitBox(settingsBtn.save, 443, 100, 462, 633);

    float leftMargin1 = 698;
    float leftMargin2 = 875;
    float topMargin = 52;

    for (int i = 0;i < 5;i++) {
        createButtonHitBox(settingsBtn.leftStrokes[i], 18, 33, leftMargin1, topMargin);
        createButtonHitBox(settingsBtn.rightStrokes[i], 18, 33, leftMargin2, topMargin);
        topMargin += 73;
    }

    while (window.isOpen()) {
        if (gameStage == "MENU") {
            window.setMouseCursorVisible(false);
            // Сброс таймера при возврате в меню
            timeRemaining = roundTime + 1;
            timerText.setString("Timer:  ");
            inputText.setString("Your input: ");


            Event event;
            while (window.pollEvent(event)) {
                closeEvents(event, window);

                Vector2i mousePos = Mouse::getPosition(window);

                if (event.type == Event::MouseButtonPressed) {
                    if (event.mouseButton.button == Mouse::Left) {
                        if (menuBtn.start.getGlobalBounds().contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y))) {
                            gameStage = "GAME"; 
                            cout << "game1\n";
                        }
                        else if (menuBtn.settings.getGlobalBounds().contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y))) {
                            gameStage = "SETTINGS";
                        }
                        else if (menuBtn.exit.getGlobalBounds().contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y))) {
                            window.close();
                        }
                    }
                }
                if (menuBtn.start.getGlobalBounds().contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y))) {
                    menuT.startGame.setFillColor(Color(160, 108, 213));
                    anyButtonHovered = true;
                }
                else if (menuBtn.settings.getGlobalBounds().contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y))) {
                    menuT.settings.setFillColor(Color(160, 108, 213));
                    anyButtonHovered = true;

                }
                else if (menuBtn.leaderBoard.getGlobalBounds().contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y))) {
                    menuT.leaderboard.setFillColor(Color(160, 108, 213));
                    anyButtonHovered = true;

                }
                else if (menuBtn.exit.getGlobalBounds().contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y))) {
                    menuT.exit.setFillColor(Color(160, 108, 213));
                    anyButtonHovered = true;
                }

                else {
                    menuT.startGame.setFillColor(Color(226, 207, 234));
                    menuT.settings.setFillColor(Color(226, 207, 234));
                    menuT.leaderboard.setFillColor(Color(226, 207, 234));
                    menuT.exit.setFillColor(Color(226, 207, 234));
                    anyButtonHovered = false;
                }
            }
            window.clear();
            window.draw(menuBg.sprite);

            window.draw(menuBtn.start);
            window.draw(menuBtn.settings);
            window.draw(menuBtn.leaderBoard);
            window.draw(menuBtn.exit);

            window.draw(menuT.startGame);
            window.draw(menuT.settings);
            window.draw(menuT.leaderboard);
            window.draw(menuT.exit);

            Vector2i mousePos = Mouse::getPosition(window);
            cursorManager.update(mousePos, anyButtonHovered);
            cursorManager.draw(window);

            window.display();
        }
        else if (gameStage == "SETTINGS") {
            window.setMouseCursorVisible(false);
            Event event;

            while (window.pollEvent(event)) {
                closeEvents(event, window);

                if (mouseIn(window, settingsBtn.exitToMenu)) {
                    settingsT.exitToMenu.setFillColor(Color(160, 108, 213));
                    anyButtonHovered = true;
                }
                else if (mouseIn(window, settingsBtn.save)) {
                    settingsT.save.setFillColor(Color(160, 108, 213));
                    anyButtonHovered = true;
                }
                else {
                    settingsT.save.setFillColor(Color(226, 207, 234));
                    settingsT.exitToMenu.setFillColor(Color(226, 207, 234));
                    anyButtonHovered = false;

                    for (int i = 0;i < 5;i++) {
                        if (mouseIn(window, settingsBtn.leftStrokes[i]) || mouseIn(window, settingsBtn.rightStrokes[i])) {
                            anyButtonHovered = true;
                            break;
                        }
                    }
                }

                for (int i = 0;i < 5;i++) {
                    if (event.type == Event::MouseButtonPressed && event.mouseButton.button == Mouse::Left) {
                        bool isLeft = mouseIn(window, settingsBtn.leftStrokes[i]);
                        bool isRight = mouseIn(window, settingsBtn.rightStrokes[i]);
                        string plusOrMinus = isLeft ? "-": "+";
                        if (isLeft || isRight) {
                            switch (i) {
                            case 0:
                                timeIndex = updateIndex(timeIndex, timesCount, plusOrMinus);
                                addInfoToWindow(timerOption, font, to_string(timesToRound[timeIndex]), 25, Color(226, 207, 234), 57, 7);
                                break;
                            case 1:
                                difIndex = updateIndex(difIndex, difCount, plusOrMinus);
                                addInfoToWindow(difOption, font, difToRound[difIndex], 25, Color(226, 207, 234), 55.5, 16.5);
                                break;
                            case 2:
                                musicIndex = updateIndex(musicIndex, musicCount, plusOrMinus);
                                addInfoToWindow(musicOption, font, to_string(musicToRound[musicIndex]), 25, Color(226, 207, 234), 58, 26);
                                break;
                            case 3:
                                themeIndex = updateIndex(themeIndex, themeCount, plusOrMinus);
                                addInfoToWindow(themeOption, font, to_string(themeToRound[themeIndex]), 25, Color(226, 207, 234), 58, 35.5);
                                break;
                            }
                        }
                    }
                }

                if (click(event, window, settingsBtn.exitToMenu)) {
                    gameStage = "MENU";
                }
                else if (click(event, window, settingsBtn.save)) {
                    cout << "save";
                }
            }
            window.clear();
            window.draw(settingsBg.sprite);

            window.draw(settingsBtn.exitToMenu);
            window.draw(settingsBtn.save);

            window.draw(settingsT.exitToMenu);
            window.draw(settingsT.timerParam);
            window.draw(settingsT.difficultyParam);
            window.draw(settingsT.musicParam);
            window.draw(settingsT.themeParam);
            window.draw(settingsT.save);

            window.draw(timerOption);
            window.draw(difOption);
            window.draw(musicOption);
            window.draw(themeOption);

            drawCursor(window, cursorManager, anyButtonHovered);

            for (int i = 0;i < 5;i++) {
                window.draw(settingsBtn.leftStrokes[i]);
                window.draw(settingsBtn.rightStrokes[i]);
            }
            window.display();
        }
        else if (gameStage == "GAME") {
            anyButtonHovered = false;
            window.setMouseCursorVisible(false);
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

            addInfoToWindow(pauseText, font, "Pause", 40, Color::White, 6, 5.7);
            addInfoToWindow(counterText, font, "Score:  " + to_string(counter), 40, Color::White, 50, 5.7);
            addInfoToWindow(timerText, font, "Timer:  ", 40, Color::White, 76, 5.7);
            addInfoToWindow(targetText, font, "Random word is: " + targetWord, 36, Color::White, 20, 26);
            addInfoToWindow(inputText, font, "Your input:   ", 40, Color::White, 20, 55);
            addInfoToWindow(endGameText, font, "End Game", 40, Color::White, 75, 90);

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
                            if (pauseButtonHitBox.getGlobalBounds().contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y))) {
                                isPaused = !isPaused; // Переключаем состояние паузы
                                if (isPaused) {
                                    addInfoToWindow(pauseText, font, "Resume", 40, Color::White, 4.5, 5.7);
                                }
                                else {
                                    addInfoToWindow(pauseText, font, "Pause", 40, Color::White, 6, 5.7);
                                    gameClock.restart(); // Перезапускаем таймер при снятии паузы
                                }
                            }
                        }
                    }

                    if (pauseButtonHitBox.getGlobalBounds().contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y))) {
                        pauseText.setFillColor(Color(160, 108, 213));
                        anyButtonHovered = true;
                    }
                    else {
                        pauseText.setFillColor(Color::White);
                        anyButtonHovered = false;
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
                        inputText.setString("Your input:   " + playerInput);
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
                                counterText.setString("Score:  " + to_string(counter));
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
                window.draw(gameSprite);

                window.draw(counterText);
                window.draw(targetText);
                window.draw(inputText);
                window.draw(timerText);
                window.draw(pauseText);
                if (!isPaused) {
                    addInfoToWindow(endGameText, font, "End Game", 40, Color::White, 75, 90);
                    window.draw(endGameText);
                }
                else {
                    addInfoToWindow(endGameText, font, "End Game", 40, Color::White, 110, 110);
                    window.draw(endGameText);
                }
                Vector2i mousePos = Mouse::getPosition(window);
                cursorManager.update(mousePos, anyButtonHovered);
                cursorManager.draw(window);

                window.draw(pauseButtonHitBox);

                window.display();

                // Проверка времени
                if (timeRemaining <= 0) {
                    gameStage = "ENDGAME";
                }
            }
        }
        else if (gameStage == "ENDGAME") {
            Event event;
            window.setMouseCursorVisible(false);

            while (window.pollEvent(event)) {
                closeEvents(event, window);

                Vector2i mousePos = Mouse::getPosition(window);
                if (restartButtonHitBox.getGlobalBounds().contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y))) {
                    restartText.setFillColor(Color(160, 108, 213));
                    anyButtonHovered = true;
                }
                else if (exitToMenuButtonHitBox.getGlobalBounds().contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y))) {
                    exitTextEnd.setFillColor(Color(160, 108, 213));
                    anyButtonHovered = true;
                }
                else {
                    restartText.setFillColor(Color(226, 207, 234));
                    exitTextEnd.setFillColor(Color(226, 207, 234));
                    anyButtonHovered = false;
                }

                if (event.type == Event::MouseButtonPressed) {
                    if (event.mouseButton.button == Mouse::Left) {
                        if (restartButtonHitBox.getGlobalBounds().contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y))) {
                            timeRemaining = roundTime;
                            timerText.setString("Timer:  ");
                            inputText.setString("Your input: ");
                            window.setMouseCursorVisible(true);
                            restartText.setFillColor(Color(226, 207, 234));
                            gameStage = "GAME";
                        }
                        else if (exitToMenuButtonHitBox.getGlobalBounds().contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y))) {
                            exitTextEnd.setFillColor(Color(226, 207, 234));
                            gameStage = "MENU";
                        }
                    }
                }
            }
            scoreText.setString("Your score: " + to_string(counter));
            
            window.clear();
            window.draw(endgameSprite);

            window.draw(restartText);
            window.draw(exitTextEnd);
            window.draw(scoreText);
            window.draw(isrecordText);

            window.draw(restartButtonHitBox);
            window.draw(exitToMenuButtonHitBox);

            Vector2i mousePos = Mouse::getPosition(window);
            cursorManager.update(mousePos, anyButtonHovered);
            cursorManager.draw(window);

            window.display();
        }
    }

    return 0;
}