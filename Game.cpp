#include <SFML/Graphics.hpp>
#include <iostream>
#include <sstream>
#include <string>
#include <SFML/Audio.hpp>

#include "MainHeader.h"
#include "RandomWord.h"
#include "ValidWord.h"
#include "CursorManager.h"
#include "Music.h"

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

int main() {
    setlocale(LC_ALL, "");
    MusicManager musicManager;

    Color color1, color2;
    ifstream setJson("jsons/settings.json");
    json settings = json::parse(setJson);

    int themeNumber = settings["theme_number"];
    string difficulty = settings["difficulty"];
    int roundTime = settings["round_time"];
    int music1Number = settings["music1"];
    int music2Number = settings["music2"];

    ifstream themeJson("jsons/theme" + to_string(themeNumber) + ".json");
    json theme = json::parse(themeJson);
    auto colorFirst = theme["color1"];
    auto colorSecond = theme["color2"];

    color1 = Color(colorFirst[0], colorFirst[1], colorFirst[2]);
    color2 = Color(colorSecond[0], colorSecond[1], colorSecond[2]);

    bool isPaused = false;
    bool anyButtonHovered = false;

    string gameStage = "AUTH_REG";
    VideoMode desktop = VideoMode::getDesktopMode();
    RenderWindow window(desktop, "Game", Style::Fullscreen);

    Font font;
    CursorManager cursorManager;

    if (!font.loadFromFile("fonts/font1.ttf")) {
        return EXIT_FAILURE;
    }

    if (!cursorManager.loadTextures("cursors/defaultcursor.png", "cursors/hovercursor.png")) {
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

    string menuFilename = "backgrounds/menu" + to_string(themeNumber) + ".png";
    updateBackground(window, menuBg.texture, menuBg.sprite, menuFilename);


    addInfoToWindow(menuT.startGame, font, "Start Game", 36, color1, 39.2, 14.7);
    addInfoToWindow(menuT.settings, font, "Settings", 36, color1, 41.8, 36.4);
    addInfoToWindow(menuT.leaderboard, font, "LeaderBoard", 30, color1, 39.2, 58.7);
    addInfoToWindow(menuT.exit, font, "Exit", 36, color1, 46.2, 79.7);

    createButtonHitBox(menuBtn.start, 343, 120, 37.46, 9.78);
    createButtonHitBox(menuBtn.settings, 343, 120, 37.46, 31.42);
    createButtonHitBox(menuBtn.leaderBoard, 343, 120, 37.46, 53.06);
    createButtonHitBox(menuBtn.exit, 343, 120, 37.46, 74.71);

    // Game text
    struct GameTexts {
        Text counter;
        Text target;
        Text input;
        Text letters;
        Text endGame;
        Text timer;
        Text pause;
    };

    struct GameBg {
        Texture texture;
        Sprite sprite;
    };

    struct GameButtons {
        RectangleShape pause;
    };

    GameTexts gameT;
    GameBg gameBg;
    GameButtons gameBtn;

    string gameFilename = "backgrounds/game" + to_string(themeNumber) + ".png";
    updateBackground(window, gameBg.texture, gameBg.sprite, gameFilename);

    ValidWord validator("validWords.txt");
    validator.loadWords();

    Clock gameClock;
    unordered_map<char, int> availableLetters;
    unordered_map<char, int> currentLetters;
    string targetWord;
    string playerInput;
    string guessedWords[100];
    unsigned int counter = 0;
    int guessedCount = 0;
    int timeRemaining = roundTime;

    createButtonHitBox(gameBtn.pause, 262, 100, 2.93, 2.61);

    //Endgame
    struct EndgameTexts {
        Text restart;
        Text exit;
        Text score;
        Text isrecord;

    };

    struct EndgameBg {
        Texture texture;
        Sprite sprite;
    };

    struct EndgameButtons {
        RectangleShape restart;
        RectangleShape exit;
    };

    EndgameTexts endgameT;
    EndgameBg endgameBg;
    EndgameButtons endgameBtn;


    string endgameFilename = "backgrounds/endgame" + to_string(themeNumber) + ".png";
    updateBackground(window, endgameBg.texture, endgameBg.sprite, endgameFilename);


    addInfoToWindow(endgameT.restart, font, "Restart", 50, color1, 39.3, 62.5);
    addInfoToWindow(endgameT.exit, font, "Exit", 50, color1, 45, 82.5);

    addInfoToWindow(endgameT.score, font, "Your score: ", 30, color1, 35, 10);
    addInfoToWindow(endgameT.isrecord, font, "New record!!!", 20, color1, 35, 15);

    createButtonHitBox(endgameBtn.restart, 496, 120, 31.89, 58.8);
    createButtonHitBox(endgameBtn.exit, 496, 120, 31.89, 78.49);


    //Settings
    struct SettingsTexts {
        Text exitToMenu;
        Text save;
        Text timerParam;
        Text difficultyParam;
        Text music1Param;
        Text music2Param;
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

    string settingsFilename = "backgrounds/settings" + to_string(themeNumber) + ".png";
    updateBackground(window, settingsBg.texture, settingsBg.sprite, settingsFilename);

    addInfoToWindow(settingsT.exitToMenu, font, "Exit", 50, color1, 5, 6);
    addInfoToWindow(settingsT.timerParam, font, "Timer", 30, color2, 35, 6.8);
    addInfoToWindow(settingsT.difficultyParam, font, "Difficulty", 28, color2, 35, 16.3);
    addInfoToWindow(settingsT.music1Param, font, "Main song", 30, color2, 35, 25.8);
    addInfoToWindow(settingsT.music2Param, font, "Game song", 30, color2, 35, 35.3);
    addInfoToWindow(settingsT.themeParam, font, "Theme", 30, color2, 35, 44.8);
    addInfoToWindow(settingsT.save, font, "Save", 50, color1, 44, 85);

    const int timesCount = 4;
    const int difCount = 3;
    const int music1Count = 4;
    const int music2Count = 4;
    const int themeCount = 4;

    int timesToRound[timesCount]{30,60,90,120};
    string difToRound[difCount]{"easy","normal","hard"};
    int music1ToRound[music1Count]{ 1,2,3,4 };
    int music2ToRound[music2Count]{ 1,2,3,4 };
    int themeToRound[themeCount]{ 1,2,3,4 };

    int timeIndex = getCurrentIndex(timesCount, timesToRound, roundTime);
    int difIndex = getCurrentIndexStr(difCount, difToRound, difficulty);
    int themeIndex = getCurrentIndex(themeCount, themeToRound, themeNumber);
    int music1Index = getCurrentIndex(music1Count, music1ToRound, music1Number);
    int music2Index = getCurrentIndex(music2Count, music2ToRound, music2Number);

    Text timerOption;
    Text difOption;
    Text music1Option;
    Text music2Option;
    Text themeOption;

    addInfoToWindow(timerOption, font, to_string(timesToRound[timeIndex]), 25, color1, 57, 7);
    addInfoToWindow(difOption, font, difToRound[difIndex], 25, color1, 55.5, 16.5);
    addInfoToWindow(music1Option, font, to_string(music1ToRound[music1Index]), 25, color1, 58, 26);
    addInfoToWindow(music2Option, font, to_string(music2ToRound[music2Index]), 25, color1, 58, 35.5);
    addInfoToWindow(themeOption, font, to_string(themeToRound[themeIndex]), 25, color1, 58, 45);

    createButtonHitBox(settingsBtn.exitToMenu, 200, 100, 2.93, 3.13);
    createButtonHitBox(settingsBtn.save, 443, 100, 33.87, 82.53);

    float leftMargin1 = 51.17;
    float leftMargin2 = 64.15;
    float topMargin = 6.78;
    float marginStep = 9.52;

    for (int i = 0; i < 5; i++) {
        createButtonHitBox(settingsBtn.leftStrokes[i], 18, 33, leftMargin1, topMargin);
        createButtonHitBox(settingsBtn.rightStrokes[i], 18, 33, leftMargin2, topMargin);
        topMargin += marginStep;
    }
    if (gameStage != "AUTH_REG") {
        musicManager.play("songs/main" + to_string(music1ToRound[music1Index]) + ".ogg");
    }

    //LEADERBOARD

    struct LeaderBoardTexts {
        Text exit;
        Text numberTitle;
        Text userTitle;
        Text scoreTitle;
        Text number[10];
        Text user[10];
        Text score[10];
    };

    struct LeaderBoardBg {
        Texture texture;
        Sprite sprite;
    };

    struct LeaderBoardButtons {
        RectangleShape exit;
    };

    LeaderBoardTexts leaderboardT;
    LeaderBoardBg leaderboardBg;
    LeaderBoardButtons leaderboardBtn;

    string leaderboardFilename = "backgrounds/leaderboard" + to_string(themeNumber) + ".png";
    updateBackground(window, leaderboardBg.texture, leaderboardBg.sprite, leaderboardFilename);

    addInfoToWindow(leaderboardT.exit, font, "Exit", 50, color1, 5, 5.5);
    createButtonHitBox(leaderboardBtn.exit, 200, 100, 2.93, 2.5);

    addInfoToWindow(leaderboardT.numberTitle, font, "No", 25, color2, 35, 6.4);
    addInfoToWindow(leaderboardT.userTitle, font, "User", 25, color2, 40, 6.4);
    addInfoToWindow(leaderboardT.scoreTitle, font, "Score", 25, color2, 57, 6.4);

    topMargin = 15;
    marginStep = 8.33;

    for (int i = 1; i < 11; i++) {
        addInfoToWindow(leaderboardT.number[i - 1], font, to_string(i), 23, Color::White, 35, topMargin);
        addInfoToWindow(leaderboardT.user[i - 1], font, "bot" + to_string(1000 * i + i*i - 3 + i), 23, Color::White, 40.2, topMargin);
        addInfoToWindow(leaderboardT.score[i - 1], font, to_string(200 - 4 * i), 23, Color::White, 58.8, topMargin);
        topMargin += marginStep;
    }

    //AUTH

    Font font1;

    if (!font1.loadFromFile("reg_auth/OpenSans.ttf")) {
        return EXIT_FAILURE;
    }

    struct AuthTexts {
        Text login;
        Text password;
    };

    struct AuthBg {
        Texture texture;
        Sprite sprite;
    };

    struct AuthButtons {
        RectangleShape login;
        RectangleShape password;
        RectangleShape ready;
        RectangleShape toReg;

    };

    AuthTexts authT;
    AuthBg authBg;
    AuthButtons authBtn;

    string loginInput;
    string passwordInput;

    addInfoToWindow(authT.login, font1, "", 30, Color::Black, 33.4, 30.8);
    addInfoToWindow(authT.password, font1, "", 30, Color::Black, 33.4, 46.8);

    string auth_regFilename = "reg_auth/auth.png";
    updateBackground(window, authBg.texture, authBg.sprite, auth_regFilename);


    createButtonHitBox(authBtn.login, 481, 64, 32.4f, 29.73);
    createButtonHitBox(authBtn.password, 481, 64, 32.4f, 45.89);
    createButtonHitBox(authBtn.ready, 481, 64, 32.40f, 57.70);
    createButtonHitBox(authBtn.toReg, 173, 50, 43.70f, 85);

    bool loginInputActive = false;
    bool passwordInputActive = false;
    bool isAuthWindow = true;

    string userLogin;
    string userPassword;

    int bestScore;

    while (window.isOpen()) {
        if (gameStage == "AUTH_REG") {
            musicManager.stop();
            window.setMouseCursorVisible(true);
            Event event;

            while (window.pollEvent(event)) {
                closeEvents(event, window);

                if (click(event, window, authBtn.login)) {
                    loginInputActive = true;
                }
                else if (click(event, window, authBtn.password)) {
                    passwordInputActive = true;
                }

                if (click(event, window, authBtn.toReg) && isAuthWindow) {
                    isAuthWindow = false;
                    loginInput = "";
                    passwordInput = "";
                    authT.password.setString("");
                    authT.login.setString("");


                }
                else if (click(event, window, authBtn.toReg) && !isAuthWindow) {
                    isAuthWindow = true;
                    loginInput = "";
                    passwordInput = "";
                    authT.password.setString("");
                    authT.login.setString("");
                }

                if (event.type == Event::TextEntered && loginInputActive) {
                    if (event.text.unicode == '\b') {
                        if (!loginInput.empty()) {
                            loginInput.pop_back();
                        }
                    }
                    else if (loginInput.length() < 20) {
                        if (event.text.unicode >= '0' && event.text.unicode <= '9') {
                            loginInput += static_cast<char>(event.text.unicode);
                        }
                        else if ((event.text.unicode >= 'A' && event.text.unicode <= 'Z') ||
                            (event.text.unicode >= 'a' && event.text.unicode <= 'z')) {
                            loginInput += static_cast<char>(event.text.unicode);
                        }
                    }

                    authT.login.setString(loginInput);
                }
                else if (event.type == Event::TextEntered && passwordInputActive) {
                    if (event.text.unicode == '\b') {
                        if (!passwordInput.empty()) {
                            passwordInput.pop_back();
                        }
                    }
                    else if (passwordInput.length() < 20) {
                        if (event.text.unicode >= '0' && event.text.unicode <= '9') {
                            passwordInput += static_cast<char>(event.text.unicode);
                        }
                        else if ((event.text.unicode >= 'A' && event.text.unicode <= 'Z') ||
                            (event.text.unicode >= 'a' && event.text.unicode <= 'z')) {
                            passwordInput += static_cast<char>(event.text.unicode);
                        }
                    }

                    authT.password.setString(passwordInput);

                }

                if (notclick(event, window, authBtn.login)) {
                    loginInputActive = false;
                }

                if (notclick(event, window, authBtn.password)) {
                    passwordInputActive = false;
                }

                if (click(event, window, authBtn.ready)) {
                    ifstream inputFile("users.json");
                    json users = json::parse(inputFile);
                    inputFile.close();

                    if (!users.contains("users")) {
                        users["users"] = json::array();
                    }

                    if (isAuthWindow) {
                        // Логика авторизации (остаётся без изменений)
                        bool authSuccess = false;
                        for (const auto& user : users["users"]) {
                            if (user["login"] == loginInput && user["password"] == passwordInput) {
                                authSuccess = true;
                                ifstream inputFile("jsons/settings.json");
                                json playerSettings = json::parse(inputFile);
                                inputFile.close();
                                playerSettings = {
                                    {"login", user["login"]},
                                    {"password", user["password"]},
                                    {"best_score", user["best_score"]},
                                    {"difficulty", user["difficulty"]},
                                    {"music1", user["music1"]},
                                    {"music2", user["music2"]},
                                    {"round_time", user["round_time"]},
                                    {"theme_number", user["theme_number"]}

                                };
                                bestScore = user["best_score"];
                                userLogin = user["login"];
                                userPassword = user["password"];
                                ofstream outputFile("jsons/settings.json");
                                outputFile << playerSettings.dump(4);
                                outputFile.close();
                                break;
                            }
                        }

                        if (authSuccess) {
                            ifstream inputFile("jsons/settings.json");
                            json playerSettings = json::parse(inputFile);
                            inputFile.close();
                            roundTime = playerSettings["round_time"];
                            difficulty = playerSettings["difficulty"];
                            themeNumber = playerSettings["theme_number"];
                            music1Number = playerSettings["music1"];
                            music2Number = playerSettings["music2"];

                            menuFilename = "backgrounds/menu" + to_string(themeNumber) + ".png";
                            gameFilename = "backgrounds/game" + to_string(themeNumber) + ".png";
                            endgameFilename = "backgrounds/endgame" + to_string(themeNumber) + ".png";
                            settingsFilename = "backgrounds/settings" + to_string(themeNumber) + ".png";
                            leaderboardFilename = "backgrounds/leaderboard" + to_string(themeNumber) + ".png";

                            updateBackground(window, menuBg.texture, menuBg.sprite, menuFilename);
                            updateBackground(window, gameBg.texture, gameBg.sprite, gameFilename);
                            updateBackground(window, endgameBg.texture, endgameBg.sprite, endgameFilename);
                            updateBackground(window, settingsBg.texture, settingsBg.sprite, settingsFilename);
                            updateBackground(window, leaderboardBg.texture, leaderboardBg.sprite, leaderboardFilename);

                            ifstream themeJson("jsons/theme" + to_string(themeNumber) + ".json");
                            json theme = json::parse(themeJson);
                            colorFirst = theme["color1"];
                            colorSecond = theme["color2"];

                            color1 = Color(colorFirst[0], colorFirst[1], colorFirst[2]);
                            color2 = Color(colorSecond[0], colorSecond[1], colorSecond[2]);


                            settingsT.timerParam.setFillColor(color2);
                            settingsT.difficultyParam.setFillColor(color2);
                            settingsT.music1Param.setFillColor(color2);
                            settingsT.music2Param.setFillColor(color2);
                            settingsT.themeParam.setFillColor(color2);

                            timeIndex = getCurrentIndex(timesCount, timesToRound, roundTime);
                            difIndex = getCurrentIndexStr(difCount, difToRound, difficulty);
                            themeIndex = getCurrentIndex(themeCount, themeToRound, themeNumber);
                            music1Index = getCurrentIndex(music1Count, music1ToRound, music1Number);
                            music2Index = getCurrentIndex(music2Count, music2ToRound, music2Number);

                            addInfoToWindow(timerOption, font, to_string(timesToRound[timeIndex]), 25, color1, 57, 7);
                            addInfoToWindow(difOption, font, difToRound[difIndex], 25, color1, 55.5, 16.5);
                            addInfoToWindow(music1Option, font, to_string(music1ToRound[music1Index]), 25, color1, 58, 26);
                            addInfoToWindow(music2Option, font, to_string(music2ToRound[music2Index]), 25, color1, 58, 35.5);
                            addInfoToWindow(themeOption, font, to_string(themeToRound[themeIndex]), 25, color1, 58, 45);

                            addInfoToWindow(leaderboardT.numberTitle, font, "No", 25, color2, 35, 6.4);
                            addInfoToWindow(leaderboardT.userTitle, font, "User", 25, color2, 40, 6.4);
                            addInfoToWindow(leaderboardT.scoreTitle, font, "Score", 25, color2, 57, 6.4);

                            musicManager.play("songs/main" + to_string(music1ToRound[music1Index]) + ".ogg");
                            gameStage = "MENU";
                        }
                        else {
                            cout << "Логин или пароль не подходит" << endl;
                        }
                    }
                    else {
                        // Логика регистрации с добавлением best_score
                        bool userExists = false;
                        for (const auto& user : users["users"]) {
                            if (user["login"] == loginInput) {
                                userExists = true;
                                break;
                            }
                        }

                        if (userExists) {
                            cout << "Такой пользователь уже есть" << endl;
                        }
                        else {
                            users["users"].push_back({
                                {"login", loginInput},
                                {"password", passwordInput},
                                {"best_score", 0},
                                {"difficulty", "easy"},
                                {"music1", 1},
                                {"music2", 1},
                                {"round_time", 30},
                                {"theme_number", 1}
                            });

                            ofstream outputFile("users.json");
                            outputFile << users.dump(4);
                            outputFile.close();

                            cout << "Аккаунт создан. Начальный рекорд: 0" << endl;
                        }
                    }
                }
            }

            if (isAuthWindow) {
                auth_regFilename = "reg_auth/auth.png";
            }
            else {
                auth_regFilename = "reg_auth/reg.png";
            }
            updateBackground(window, authBg.texture, authBg.sprite, auth_regFilename);

            window.clear();
            window.draw(authBg.sprite);

            window.draw(authBtn.login);
            window.draw(authBtn.password);
            window.draw(authBtn.ready);
            window.draw(authBtn.toReg);

            window.draw(authT.login);
            window.draw(authT.password);


            window.display();
        }
        else if (gameStage == "LEADERBOARD") {
            window.setMouseCursorVisible(false);

            Event event;
            while (window.pollEvent(event)) {
                closeEvents(event, window);

                if (mouseIn(window, leaderboardBtn.exit)) {
                    leaderboardT.exit.setFillColor(color2);
                    anyButtonHovered = true;
                }
                else {
                    leaderboardT.exit.setFillColor(color1);
                    anyButtonHovered = false;
                }

                if (click(event, window, leaderboardBtn.exit)) {
                    gameStage = "MENU";
                }
            }

            window.clear();
            window.draw(leaderboardBg.sprite);

            window.draw(leaderboardBtn.exit);

            window.draw(leaderboardT.exit);

            window.draw(leaderboardT.numberTitle);
            window.draw(leaderboardT.userTitle);
            window.draw(leaderboardT.scoreTitle);

            for (int i = 0;i < 10;i++) {
                window.draw(leaderboardT.number[i]);
                window.draw(leaderboardT.user[i]);
                window.draw(leaderboardT.score[i]);
            }

            drawCursor(window, cursorManager, anyButtonHovered);

            window.display();
        }
        else if (gameStage == "MENU") {
            roundTime = settings["round_time"];
            difficulty = settings["difficulty"];
            themeNumber = settings["theme_number"];
            window.setMouseCursorVisible(false);

            timeRemaining = roundTime + 1;
            gameT.timer.setString("Timer:  ");
            gameT.input.setString("Your input: ");

            musicManager.play("songs/main" + to_string(music1ToRound[music1Index]) + ".ogg");
            //while (gameStage == "MENU" && window.isOpen()) {}
            Event event;
            while (window.pollEvent(event)) {
                closeEvents(event, window);

                if (mouseIn(window, menuBtn.start)) {
                    menuT.startGame.setFillColor(color2);
                    anyButtonHovered = true;
                }
                else if (mouseIn(window, menuBtn.settings)) {
                    menuT.settings.setFillColor(color2);
                    anyButtonHovered = true;
                }
                else if (mouseIn(window, menuBtn.leaderBoard)) {
                    menuT.leaderboard.setFillColor(color2);
                    anyButtonHovered = true;
                }
                else if (mouseIn(window, menuBtn.exit)) {
                    menuT.exit.setFillColor(color2);
                    anyButtonHovered = true;
                }
                else {
                    menuT.startGame.setFillColor(color1);
                    menuT.settings.setFillColor(color1);
                    menuT.leaderboard.setFillColor(color1);
                    menuT.exit.setFillColor(color1);
                    anyButtonHovered = false;
                }

                if (click(event, window, menuBtn.start)) {
                    gameStage = "GAME";
                }
                else if (click(event, window, menuBtn.settings)) {
                    gameStage = "SETTINGS";
                }
                else if (click(event, window, menuBtn.leaderBoard)) {
                    gameStage = "LEADERBOARD";
                }
                else if (click(event, window, menuBtn.exit)) {
                    // Сброс полей ввода
                    loginInput = "";
                    passwordInput = "";
                    authT.password.setString("");
                    authT.login.setString("");

                    // Загрузка текущих настроек
                    ifstream settingsFile("jsons/settings.json");
                    json currentSettings = json::parse(settingsFile);
                    settingsFile.close();

                    // Загрузка пользователей
                    ifstream usersFile("users.json");
                    json users = json::parse(usersFile);
                    usersFile.close();

                    // Поиск и обновление данных текущего пользователя
                    for (auto& user : users["users"]) {
                        if (user["login"] == currentSettings["login"]) {
                            // Обновляем только изменяемые параметры
                            user["best_score"] = currentSettings["best_score"];
                            user["difficulty"] = currentSettings["difficulty"];
                            user["music1"] = currentSettings["music1"];
                            user["music2"] = currentSettings["music2"];
                            user["round_time"] = currentSettings["round_time"];
                            user["theme_number"] = currentSettings["theme_number"];
                            break;
                        }
                    }

                    // Сохранение обновленных данных пользователей
                    ofstream outputUsersFile("users.json");
                    outputUsersFile << users.dump(4);
                    outputUsersFile.close();

                    gameStage = "AUTH_REG";
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

            drawCursor(window, cursorManager, anyButtonHovered);

            window.display();

        }
        else if (gameStage == "SETTINGS") {
            window.setMouseCursorVisible(false);
            Event event;

            while (window.pollEvent(event)) {
                closeEvents(event, window);

                if (mouseIn(window, settingsBtn.exitToMenu)) {
                    settingsT.exitToMenu.setFillColor(color2);
                    anyButtonHovered = true;
                }
                else if (mouseIn(window, settingsBtn.save)) {
                    settingsT.save.setFillColor(color2);
                    anyButtonHovered = true;
                }
                else {
                    settingsT.save.setFillColor(color1);
                    settingsT.exitToMenu.setFillColor(color1);
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
                                addInfoToWindow(timerOption, font, to_string(timesToRound[timeIndex]), 25, color1, 57, 7);
                                break;
                            case 1:
                                difIndex = updateIndex(difIndex, difCount, plusOrMinus);
                                addInfoToWindow(difOption, font, difToRound[difIndex], 25, color1, 55.5, 16.5);
                                break;
                            case 2:
                                music1Index = updateIndex(music1Index, music1Count, plusOrMinus);
                                addInfoToWindow(music1Option, font, to_string(music1ToRound[music1Index]), 25, color1, 58, 26);
                                break;
                            case 3:
                                music2Index = updateIndex(music2Index, music2Count, plusOrMinus);
                                addInfoToWindow(music2Option, font, to_string(music2ToRound[music2Index]), 25, color1, 58, 35.5);
                                break;
                            case 4:
                                themeIndex = updateIndex(themeIndex, themeCount, plusOrMinus);
                                addInfoToWindow(themeOption, font, to_string(themeToRound[themeIndex]), 25, color1, 58, 45);
                                break;
                            }
                        }
                    }
                }

                if (click(event, window, settingsBtn.exitToMenu)) {
                    gameStage = "MENU";
                }
                else if (click(event, window, settingsBtn.save)) {
                    // Сохранение
                    settings["round_time"] = timesToRound[timeIndex];
                    settings["difficulty"] = difToRound[difIndex];
                    settings["theme_number"] = themeToRound[themeIndex];
                    settings["music1"] = music1ToRound[music1Index];
                    settings["music2"] = music2ToRound[music2Index];
                    settings["login"] = userLogin;
                    settings["password"] = userPassword;
                    roundTime = settings["round_time"];
                    difficulty = settings["difficulty"];
                    themeNumber = settings["theme_number"];
                    music1Number = settings["music1"];
                    music2Number = settings["music2"];
                    std::ofstream out("jsons/settings.json");
                    out << settings.dump(4);

                    menuFilename = "backgrounds/menu" + to_string(themeNumber) + ".png";
                    gameFilename = "backgrounds/game" + to_string(themeNumber) + ".png";
                    endgameFilename = "backgrounds/endgame" + to_string(themeNumber) + ".png";
                    settingsFilename = "backgrounds/settings" + to_string(themeNumber) + ".png";
                    leaderboardFilename = "backgrounds/leaderboard" + to_string(themeNumber) + ".png";

                    updateBackground(window, menuBg.texture, menuBg.sprite, menuFilename);
                    updateBackground(window, gameBg.texture, gameBg.sprite, gameFilename);
                    updateBackground(window, endgameBg.texture, endgameBg.sprite, endgameFilename);
                    updateBackground(window, settingsBg.texture, settingsBg.sprite, settingsFilename);
                    updateBackground(window, leaderboardBg.texture, leaderboardBg.sprite, leaderboardFilename);
                    
                    ifstream themeJson("jsons/theme" + to_string(themeNumber) + ".json");
                    json theme = json::parse(themeJson);
                    colorFirst = theme["color1"];
                    colorSecond = theme["color2"];

                    color1 = Color(colorFirst[0], colorFirst[1], colorFirst[2]);
                    color2 = Color(colorSecond[0], colorSecond[1], colorSecond[2]);


                    settingsT.timerParam.setFillColor(color2);
                    settingsT.difficultyParam.setFillColor(color2);
                    settingsT.music1Param.setFillColor(color2);
                    settingsT.music2Param.setFillColor(color2);
                    settingsT.themeParam.setFillColor(color2);

                    addInfoToWindow(leaderboardT.numberTitle, font, "No", 25, color2, 35, 6.4);
                    addInfoToWindow(leaderboardT.userTitle, font, "User", 25, color2, 40, 6.4);
                    addInfoToWindow(leaderboardT.scoreTitle, font, "Score", 25, color2, 57, 6.4);

                    musicManager.play("songs/main" + to_string(music1ToRound[music1Index]) + ".ogg");

                }

            }
            window.clear();
            window.draw(settingsBg.sprite);

            window.draw(settingsBtn.exitToMenu);
            window.draw(settingsBtn.save);

            window.draw(settingsT.exitToMenu);
            window.draw(settingsT.timerParam);
            window.draw(settingsT.difficultyParam);
            window.draw(settingsT.music1Param);
            window.draw(settingsT.music2Param);
            window.draw(settingsT.themeParam);
            window.draw(settingsT.save);

            window.draw(timerOption);
            window.draw(difOption);
            window.draw(music1Option);
            window.draw(music2Option);
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
            targetWord = getRandomWord(difficulty + "RandomWords.txt");
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

            addInfoToWindow(gameT.pause, font, "Pause", 40, Color::White, 6, 5.7);
            addInfoToWindow(gameT.counter, font, "Score:  " + to_string(counter), 40, Color::White, 50, 5.7);
            addInfoToWindow(gameT.timer, font, "Timer:  ", 40, Color::White, 76, 5.7);
            addInfoToWindow(gameT.target, font, "Random word is: " + targetWord, 36, Color::White, 20, 26);
            addInfoToWindow(gameT.input, font, "Your input:   ", 40, Color::White, 20, 55);
            addInfoToWindow(gameT.endGame, font, "End Game", 40, Color::White, 75, 90);

            gameClock.restart(); // Сброс таймера

            musicManager.play("songs/game" + to_string(music2ToRound[music2Index]) + ".ogg");

            // Основной игровой цикл
            while (gameStage == "GAME" && window.isOpen()) {
                Event event;
                updateTimer(gameClock, timeRemaining, gameT.timer, isPaused);
                if (isPaused) {
                    gameT.input.setFillColor(Color(150, 150, 150)); // Серый цвет при паузе
                }
                else {
                    gameT.input.setFillColor(Color::White); // Белый цвет при активной игре
                }
                while (window.pollEvent(event)) {
                    closeEvents(event, window);
                    Vector2i mousePos = Mouse::getPosition(window);
                    if (click(event, window, gameT.endGame)) {
                        gameStage = "MENU";
                    }
                    else if (click(event, window, gameBtn.pause)) {
                        isPaused = !isPaused;
                        if (isPaused) {
                            addInfoToWindow(gameT.pause, font, "Resume", 40, Color::White, 4.5, 5.7);
                        }
                        else {
                            addInfoToWindow(gameT.pause, font, "Pause", 40, Color::White, 6, 5.7);
                            gameClock.restart();
                        }
                    }
                    
                    if (mouseIn(window, gameBtn.pause)) {
                        gameT.pause.setFillColor(color2);
                        anyButtonHovered = true;
                    }
                    else if (mouseIn(window, gameT.endGame)) {
                        gameT.endGame.setFillColor(color2);
                        anyButtonHovered = true;
                    }
                    else {
                        gameT.pause.setFillColor(Color::White);
                        gameT.endGame.setFillColor(Color::White);
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
                        gameT.input.setString("Your input:   " + playerInput);
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
                                gameT.counter.setString("Score:  " + to_string(counter));
                            }
                        }

                        // Сброс после подтверждения слова
                        currentLetters = availableLetters;
                        playerInput.clear();
                        gameT.input.setString("Your input: ");
                    }
                }

                if (gameStage != "GAME") {
                    isPaused = false;
                }

                window.draw(gameBg.sprite);

                window.draw(gameT.counter);
                window.draw(gameT.target);
                window.draw(gameT.input);
                window.draw(gameT.timer);
                window.draw(gameT.pause);

                if (!isPaused) {
                    addInfoToWindow(gameT.endGame, font, "End Game", 40, Color::White, 75, 90);
                    window.draw(gameT.endGame);
                }
                else {
                    addInfoToWindow(gameT.endGame, font, "End Game", 40, Color::White, 110, 110);
                    window.draw(gameT.endGame);
                }

                window.draw(gameBtn.pause);

                drawCursor(window, cursorManager, anyButtonHovered);

                window.display();

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

                if (mouseIn(window, endgameBtn.restart)) {
                    endgameT.restart.setFillColor(color2);
                    anyButtonHovered = true;
                }
                else if (mouseIn(window, endgameBtn.exit)) {
                    endgameT.exit.setFillColor(color2);
                    anyButtonHovered = true;
                }
                else {
                    endgameT.restart.setFillColor(color1);
                    endgameT.exit.setFillColor(color1);
                    anyButtonHovered = false;
                }

                if (click(event, window, endgameBtn.restart)) {
                    timeRemaining = roundTime;
                    gameT.timer.setString("Timer:  ");
                    gameT.input.setString("Your input: ");
                    window.setMouseCursorVisible(true);
                    endgameT.restart.setFillColor(color1);

                    musicManager.stop();
                    gameStage = "GAME";
                }
                else if (click(event, window, endgameBtn.exit)) {
                    endgameT.exit.setFillColor(color1);
                    gameStage = "MENU";
                }
            }
            endgameT.score.setString("Your score: " + to_string(counter));

            if (counter > bestScore) {
                bestScore = counter;
                settings["best_score"] = bestScore;
                std::ofstream out("jsons/settings.json");
                out << settings.dump(4);

                window.draw(endgameT.isrecord);

            }
            
            window.clear();
            window.draw(endgameBg.sprite);

            window.draw(endgameT.restart);
            window.draw(endgameT.exit);
            window.draw(endgameT.score);

            window.draw(endgameBtn.restart);
            window.draw(endgameBtn.exit);

            drawCursor(window, cursorManager, anyButtonHovered);

            window.display();
        }
    }

    return 0;
}