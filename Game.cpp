#include <SFML/Graphics.hpp>
#include <iostream>
#include <sstream>
#include <string>
#include <SFML/Audio.hpp>
#include <functional>
#include <unordered_map>
#include <algorithm>
#include <iomanip>
#include <locale>
#include <cctype>
#include <io.h>
#include <fcntl.h>
#include <fstream>
#include <vector>

#include "MainHeader.h"
#include "RandomWord.h"
#include "RuRandomWord.h"
#include "ValidWord.h"
#include "CursorManager.h"
#include "Music.h"
#include "Sound.h"

#include "json.hpp"


using json = nlohmann::json;
using namespace std;
using namespace sf;

void printLetterMap(const std::unordered_map<char, int>& letterMap) {
    for (const auto& pair : letterMap) {
        std::cout << "'" << pair.first << "': " << pair.second << std::endl;
    }
}

class WordChecker {
private:
    std::vector<std::wstring> dictionary;

public:
    WordChecker(const std::string& dictionaryPath) {
        loadDictionary(dictionaryPath);
    }

    bool checkWord(const std::wstring& word) {
        return std::find(dictionary.begin(), dictionary.end(), word) != dictionary.end();
    }

    const std::vector<std::wstring>& getDictionary() const {
        return dictionary;
    }

private:
    void loadDictionary(const std::string& path) {
        std::wifstream file(path);
        file.imbue(std::locale(""));
        std::wstring word;

        while (std::getline(file, word)) {
            if (!word.empty()) {
                dictionary.push_back(word);
            }
        }
    }
};

bool isRussianChar(wchar_t c) {
    return (c >= L'а' && c <= L'я') || c == L'ё';
}

std::wstring readWordFromFile(const std::string& filename) {
    std::wifstream file(filename);
    file.imbue(std::locale(""));
    std::wstring word;
    std::getline(file, word);
    return word;
}

int main() {
    //std::wstring sourceWord = readWordFromFile("ww.txt");
    //std::wstring sourceWord = L"сллово";
    RuRandomWord wordLoader("ru_words/easyRU.txt");
    wordLoader.loadWords();
    std::wstring sourceWord = wordLoader.getRandomWord();

    if (sourceWord.empty()) {
        std::wcout << L"Ошибка: файл ww.txt пуст или не найден!" << std::endl;
        return EXIT_FAILURE;
    }

    for (auto& c : sourceWord) {
        c = std::tolower(c, std::locale());
    }

    WordChecker checker("ru_words/ruValidWords.txt");
    if (checker.getDictionary().empty()) {
        std::wcout << L"Ошибка: словарь не загружен или пуст!" << std::endl;
        return EXIT_FAILURE;
    }

    std::wstring availableLettersR = sourceWord;
    std::wstring currentInputR;

    const int timesCount = 4;
    const int difCount = 3;
    const int music1Count = 3;
    const int music2Count = 3;
    const int themeCount = 4;    

    int timesToRound[timesCount]{ 30,60,90,120 };
    string difToRound[difCount]{ "easy","normal","hard" };
    int music1ToRound[music1Count]{ 1,2,3 };
    int music2ToRound[music2Count]{ 1,2,3 };
    int themeToRound[themeCount]{ 1,2,3,4 };

    float topMargin = 6.78;
    float marginStep = 9.52;

    float calculatedResult = 0;

    string settingsFilepath = "jsons/settings.json";

    setlocale(LC_ALL, "");
    MusicManager musicManager;
    CursorManager cursorManager;
    SoundManager sfx("sounds/btn.wav", 40.f);

    Color color1, color2;
    ifstream setJson(settingsFilepath);
    json settings = json::parse(setJson);

    int themeNumber = settings["theme_number"];
    string difficulty = settings["difficulty"];
    int roundTime = settings["round_time"];
    int music1Number = settings["music1"];
    int music2Number = settings["music2"];
    int sfxVolume = settings["sfx_volume"];
    int gameVolume = settings["game_volume"];
    int mainVolume = settings["main_volume"];

    ifstream themeJson("jsons/theme" + to_string(themeNumber) + ".json");
    json theme = json::parse(themeJson);
    auto colorFirst = theme["color1"];
    auto colorSecond = theme["color2"];

    color1 = Color(colorFirst[0], colorFirst[1], colorFirst[2]);
    color2 = Color(colorSecond[0], colorSecond[1], colorSecond[2]);

    bool isPaused = false;
    bool anyButtonHovered = false;
    string LANG = "ENG";

    string gameStage = "AUTH_REG";
    VideoMode desktop = VideoMode::getDesktopMode();
    RenderWindow window(desktop, "Game", Style::Fullscreen);

    Font font;

    if (!font.loadFromFile("fonts/font1.ttf")) {
        return EXIT_FAILURE;
    }

    if (!cursorManager.loadTextures("cursors/defaultcursor.png", "cursors/hovercursor.png")) {
        return EXIT_FAILURE;
    }

    Image ruImg;
    Texture ruTexture;
    Sprite ruSprite;
    string ruImgFilename = "imgs/ru.png";
    createIconSprite(ruImg, ruTexture, ruSprite, ruImgFilename, 5, 80);

    float ruW = ruSprite.getGlobalBounds().width;
    float ruH = ruSprite.getGlobalBounds().height;

    Image engImg;
    Texture engTexture;
    Sprite engSprite;
    string engImgFilename = "imgs/eng.png";
    createIconSprite(engImg, engTexture, engSprite, engImgFilename, 5, 90);

    float engW = engSprite.getGlobalBounds().width;
    float engH = engSprite.getGlobalBounds().height;

    RectangleShape ruBtn;
    createIcon(ruBtn, ruW, ruH, 0, 80);

    RectangleShape engBtn;
    createIcon(engBtn, engW, engH, 0, 90);

    //AUTH

    Font font1;

    if (!font1.loadFromFile("reg_auth/OpenSans.ttf")) {
        return EXIT_FAILURE;
    }

    struct AuthTexts {
        Text login;
        Text password;
        Text warning;
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
    addInfoToWindow(authT.warning, font1, "", 40, Color::Black, 50, 70);

    string auth_regFilename = "reg_auth/auth.png";
    updateBackground(window, authBg.texture, authBg.sprite, auth_regFilename);


    createButtonHitBox(authBtn.login, 35.2, 8.4, 50, 29.73);
    createButtonHitBox(authBtn.password, 35.2, 8.4, 50, 45.89);
    createButtonHitBox(authBtn.ready, 35.2, 8.4, 50, 57.70);
    createButtonHitBox(authBtn.toReg, 12.6, 6.5, 50, 85);

    bool loginInputActive = false;
    bool passwordInputActive = false;
    bool isAuthWindow = true;

    string userLogin;
    string userPassword;

    int bestScore;

    //MENU
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


    addInfoToWindow(menuT.startGame, font, "Start", 36, color1, 50, 14.7);
    addInfoToWindow(menuT.settings, font, "Settings", 36, color1, 50, 36.4);
    addInfoToWindow(menuT.leaderboard, font, "LeaderBoard", 30, color1, 50, 58.7);
    addInfoToWindow(menuT.exit, font, "Exit", 36, color1, 50, 79.7);

    createButtonHitBox(menuBtn.start, 25, 15.6, 50, 9.78);
    createButtonHitBox(menuBtn.settings, 25, 15.6, 50, 31.42);
    createButtonHitBox(menuBtn.leaderBoard, 25, 15.6, 50, 53.06);
    createButtonHitBox(menuBtn.exit, 25, 15.6, 50, 74.71);

    //GAME
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

    ValidWord validator("eng_words/engValidWords.txt");
    validator.loadWords();

    Clock gameClock;
    unordered_map<char, int> availableLetters;
    unordered_map<char, int> currentLetters;
    string targetWord;
    string playerInput;
    string guessedWords[100];
    int counter = 0;
    int guessedCount = 0;
    int timeRemaining = roundTime;

    createButtonHitBox(gameBtn.pause, 19.2, 13, 13, 2.61);

    //Endgame
    struct EndgameTexts {
        Text restart;
        Text exit;
        Text score;
        Text isrecord;
        Text difficultyBonus;
        Text timeBonus;
        Text langBonus;
        Text totalScore;
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


    addInfoToWindow(endgameT.restart, font, "Restart", 50, color1, 50, 62.5);
    addInfoToWindow(endgameT.exit, font, "Exit", 50, color1, 50, 82.5);


    createButtonHitBox(endgameBtn.restart, 36.3, 15.6, 50, 58.8);
    createButtonHitBox(endgameBtn.exit, 36.3, 15.6, 50, 78.49);


    //Settings
    struct SettingsTexts {
        Text exitToMenu;
        Text save;
        Text timerParam;
        Text difficultyParam;
        Text music1Param;
        Text music2Param;
        Text themeParam;
        Text sfxVolumeParam;
        Text gameVolumeParam;
        Text mainVolumeParam;
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
        RectangleShape sfx;
        RectangleShape game;
        RectangleShape main;
    };

    SettingsTexts settingsT;
    SettingsBg settingsBg;
    SettingsButtons settingsBtn;

    string settingsFilename = "backgrounds/settings" + to_string(themeNumber) + ".png";
    updateBackground(window, settingsBg.texture, settingsBg.sprite, settingsFilename);

    addInfoToWindow(settingsT.exitToMenu, font, "Exit", 50, color1, 10, 6);
    addInfoToWindow(settingsT.timerParam, font, "Timer", 30, color2, 42, 6.8);
    addInfoToWindow(settingsT.difficultyParam, font, "Difficulty", 28, color2, 42, 16.3);
    addInfoToWindow(settingsT.music1Param, font, "Main song", 30, color2, 42, 25.8);
    addInfoToWindow(settingsT.music2Param, font, "Game song", 30, color2, 42, 35.3);
    addInfoToWindow(settingsT.themeParam, font, "Theme", 30, color2, 42, 44.8);
    addInfoToWindow(settingsT.sfxVolumeParam, font, "Sfx", 30, color2, 42, 54.3);
    addInfoToWindow(settingsT.gameVolumeParam, font, "Game", 30, color2, 42, 63.8);
    addInfoToWindow(settingsT.mainVolumeParam, font, "Main", 30, color2, 42, 73.3);

    addInfoToWindow(settingsT.save, font, "Save", 50, color1, 50, 85);


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

    addInfoToWindow(timerOption, font, to_string(timesToRound[timeIndex]), 25, color1, 59, 7);
    addInfoToWindow(difOption, font, difToRound[difIndex], 25, color1, 59, 16.5);
    addInfoToWindow(music1Option, font, to_string(music1ToRound[music1Index]), 25, color1, 59, 26);
    addInfoToWindow(music2Option, font, to_string(music2ToRound[music2Index]), 25, color1, 59, 35.5);
    addInfoToWindow(themeOption, font, to_string(themeToRound[themeIndex]), 25, color1, 59, 45);

    createButtonHitBox(settingsBtn.exitToMenu, 14.6, 13.6, 10, 3.13);
    createButtonHitBox(settingsBtn.save, 32.4, 13.6, 50, 82.53);

    for (int i = 0; i < 5; i++) {
        createButtonHitBox(settingsBtn.leftStrokes[i], 1.31, 4.3, 52, topMargin);
        createButtonHitBox(settingsBtn.rightStrokes[i], 1.31, 4.3, 65, topMargin);
        topMargin += marginStep;
    }

    createButtonHitBox(settingsBtn.sfx, 13, 2, 58.5, 55.7);
    settingsBtn.sfx.setFillColor(Color(192, 192, 192));
    createButtonHitBox(settingsBtn.game, 13, 2, 58.5, 65.2);
    settingsBtn.game.setFillColor(Color(192, 192, 192));
    createButtonHitBox(settingsBtn.main, 13, 2, 58.5, 74.7);
    settingsBtn.main.setFillColor(Color(192, 192, 192));

    Color thumbsColor;
    thumbsColor = Color(128, 128, 128);
    auto thumbsRadius = percentageY(2.1);


    CircleShape sfxThumb(thumbsRadius);
    createThumb(settingsBtn.sfx, sfxThumb, thumbsColor, 58, 55.7);
    bool sfxThumbIsDragging = false;

    CircleShape gameThumb(thumbsRadius);
    createThumb(settingsBtn.game, gameThumb, thumbsColor, 58, 65.2);
    bool gameThumbIsDragging = false;

    CircleShape mainThumb(thumbsRadius);
    createThumb(settingsBtn.main, mainThumb, thumbsColor, 58, 74.7);
    bool mainThumbIsDragging = false;

    setThumbPosition(window, settingsBtn.sfx, sfxThumb, sfxVolume);
    setThumbPosition(window, settingsBtn.game, gameThumb, gameVolume);
    setThumbPosition(window, settingsBtn.main, mainThumb, mainVolume);

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

    addInfoToWindow(leaderboardT.exit, font, "Exit", 50, color1, 10, 5.5);
    createButtonHitBox(leaderboardBtn.exit, 14.6, 13, 10, 2.5);

    addInfoToWindow(leaderboardT.numberTitle, font, "No", 25, color2, 36, 6.4);
    addInfoToWindow(leaderboardT.userTitle, font, "User", 25, color2, 49, 6.4);
    addInfoToWindow(leaderboardT.scoreTitle, font, "Score", 25, color2, 61, 6.4);

    topMargin = 15;
    marginStep = 8.33;
    for (int i = 0; i < 10; i++) {
        addInfoToWindow(leaderboardT.number[i], font, to_string(i + 1), 23, Color::White, 36, topMargin);
        topMargin += marginStep;
    }

    string plaingLang;
    string langBonusText;
    string difBonus;
    string timeBonus;
    float langBonus;

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
                    addInfoToWindow(authT.warning, font1, "", 40, Color::Black, 50, 70);
                }
                else if (click(event, window, authBtn.toReg) && !isAuthWindow) {
                    isAuthWindow = true;
                    loginInput = "";
                    passwordInput = "";
                    authT.password.setString("");
                    authT.login.setString("");
                    addInfoToWindow(authT.warning, font1, "", 40, Color::Black, 50, 70);
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
                        bool authSuccess = false;
                        for (const auto& user : users["users"]) {
                            if (user["login"] == loginInput && user["password"] == hashPassword(passwordInput)) {
                                authSuccess = true;
                                ifstream inputFile(settingsFilepath);
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
                                    {"theme_number", user["theme_number"]},
                                    {"sfx_volume", user["sfx_volume"]},
                                    {"game_volume", user["game_volume"]},
                                    {"main_volume", user["main_volume"]}
                                };
                                bestScore = user["best_score"];
                                userLogin = user["login"];
                                userPassword = user["password"];
                                ofstream outputFile(settingsFilepath);
                                outputFile << playerSettings.dump(4);
                                outputFile.close();
                                break;
                            }
                        }

                        if (authSuccess) {
                            ifstream inputFile(settingsFilepath);
                            json playerSettings = json::parse(inputFile);
                            inputFile.close();
                            roundTime = playerSettings["round_time"];
                            difficulty = playerSettings["difficulty"];
                            themeNumber = playerSettings["theme_number"];
                            music1Number = playerSettings["music1"];
                            music2Number = playerSettings["music2"];
                            sfxVolume = playerSettings["sfx_volume"];
                            gameVolume = playerSettings["game_volume"];
                            mainVolume = playerSettings["main_volume"];

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
                            auto temporaryColorFirst = theme["color1"];
                            auto temporaryColorSecond = theme["color2"];

                            color1 = Color(temporaryColorFirst[0], temporaryColorFirst[1], temporaryColorFirst[2]);
                            color2 = Color(temporaryColorSecond[0], temporaryColorSecond[1], temporaryColorSecond[2]);


                            settingsT.timerParam.setFillColor(color2);
                            settingsT.difficultyParam.setFillColor(color2);
                            settingsT.music1Param.setFillColor(color2);
                            settingsT.music2Param.setFillColor(color2);
                            settingsT.themeParam.setFillColor(color2);
                            settingsT.sfxVolumeParam.setFillColor(color2);
                            settingsT.gameVolumeParam.setFillColor(color2);
                            settingsT.mainVolumeParam.setFillColor(color2);

                            timeIndex = getCurrentIndex(timesCount, timesToRound, roundTime);
                            difIndex = getCurrentIndexStr(difCount, difToRound, difficulty);
                            themeIndex = getCurrentIndex(themeCount, themeToRound, themeNumber);
                            music1Index = getCurrentIndex(music1Count, music1ToRound, music1Number);
                            music2Index = getCurrentIndex(music2Count, music2ToRound, music2Number);

                            addInfoToWindow(timerOption, font, to_string(timesToRound[timeIndex]), 25, color1, 59, 7);
                            addInfoToWindow(difOption, font, difToRound[difIndex], 25, color1, 59, 16.5);
                            addInfoToWindow(music1Option, font, to_string(music1ToRound[music1Index]), 25, color1, 59, 26);
                            addInfoToWindow(music2Option, font, to_string(music2ToRound[music2Index]), 25, color1, 59, 35.5);
                            addInfoToWindow(themeOption, font, to_string(themeToRound[themeIndex]), 25, color1, 59, 45);

                            addInfoToWindow(leaderboardT.numberTitle, font, "No", 25, color2, 36, 6.4);
                            addInfoToWindow(leaderboardT.userTitle, font, "User", 25, color2, 49, 6.4);
                            addInfoToWindow(leaderboardT.scoreTitle, font, "Score", 25, color2, 61, 6.4);

                            setThumbPosition(window, settingsBtn.sfx, sfxThumb, sfxVolume);
                            setThumbPosition(window, settingsBtn.game, gameThumb, gameVolume);
                            setThumbPosition(window, settingsBtn.main, mainThumb, mainVolume);

                            musicManager.play("musics/main" + to_string(music1ToRound[music1Index]) + ".ogg", mainVolume);
                            gameStage = "MENU";
                        }
                        else {
                            addInfoToWindow(authT.warning, font1, "Login or password is incorrect!!!", 35, Color::Red, 50, 70);
                        }
                    }
                    else {
                        bool userExists = false;
                        for (const auto& user : users["users"]) {
                            if (user["login"] == loginInput) {
                                userExists = true;
                                break;
                            }
                        }

                        if (userExists) {
                            addInfoToWindow(authT.warning, font1, "There is already such a user", 40, Color::Red, 50, 70);
                        }
                        else {
                            if (loginInput != "" && passwordInput != "") {
                                users["users"].push_back({
                                    {"login", loginInput},
                                    {"password", hashPassword(passwordInput)},
                                    {"best_score", 0},
                                    {"difficulty", "easy"},
                                    {"music1", 1},
                                    {"music2", 1},
                                    {"round_time", 30},
                                    {"theme_number", 1},
                                    {"sfx_volume", 50},
                                    {"game_volume", 50},
                                    {"main_volume", 50}
                                });

                                ofstream outputFile("users.json");
                                outputFile << users.dump(4);
                                outputFile.close();

                                addInfoToWindow(authT.warning, font1, "Account created.", 40, Color::Green, 50, 70);
                            }
                            else {
                                addInfoToWindow(authT.warning, font1, "Login or password is incorrect!!!", 35, Color::Red, 50, 70);
                            }
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
            window.draw(authT.warning);


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
                else if (mouseIn(window, ruBtn)) {
                    anyButtonHovered = true;
                }
                else if (mouseIn(window, engBtn)) {
                    anyButtonHovered = true;
                }
                else {
                    leaderboardT.exit.setFillColor(color1);
                    anyButtonHovered = false;
                }

                if (click(event, window, leaderboardBtn.exit)) {
                    sfx.play();
                    gameStage = "MENU";

                    if (LANG == "RU") {
                        txt(menuT.startGame, "Начать игру", 50);
                        txt(menuT.settings, "Настройки", 50);
                        txt(menuT.leaderboard, "Лидеры", 50);
                        txt(menuT.exit, "Выход", 50);
                    }
                    else if (LANG == "ENG") {
                        txt(menuT.startGame, "Start", 50);
                        txt(menuT.settings, "Settings", 50);
                        txt(menuT.leaderboard, "LeaderBoard", 50);
                        txt(menuT.exit, "Exit", 50);
                    }
                }
                else if (click(event, window, ruBtn)) {
                    sfx.play();

                    LANG = "RU";
                    txt(leaderboardT.numberTitle, "Но", 36);
                    txt(leaderboardT.userTitle, "Никнейм", 49);
                    txt(leaderboardT.scoreTitle, "Счёт", 61);
                    txt(leaderboardT.exit, "Выход", 10);
                }
                else if (click(event, window, engBtn)) {
                    sfx.play();

                    LANG = "ENG";
                    txt(leaderboardT.numberTitle, "No", 36);
                    txt(leaderboardT.userTitle, "User", 49);
                    txt(leaderboardT.scoreTitle, "Score", 61);
                    txt(leaderboardT.exit, "Exit", 10);
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

            window.draw(ruBtn);
            window.draw(engBtn);

            window.draw(ruSprite);
            window.draw(engSprite);

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

            musicManager.play("musics/main" + to_string(music1ToRound[music1Index]) + ".ogg", mainVolume);

            Event event;
            Vector2i mousePos = Mouse::getPosition(window);

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
                else if (mouseIn(window, ruBtn)) {
                    anyButtonHovered = true;
                }
                else if (mouseIn(window, engBtn)) {
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
                    sfx.play();
                    gameStage = "GAME";
                }
                else if (click(event, window, menuBtn.settings)) {
                    sfx.play();
                    gameStage = "SETTINGS";

                    if (LANG == "RU") {
                        txt(settingsT.exitToMenu, "Выход", 10);
                        txt(settingsT.timerParam, "Время", 42);
                        txt(settingsT.difficultyParam, "Сложность", 42);
                        txt(settingsT.music1Param, "Фон. музыка", 42);
                        txt(settingsT.music2Param, "Игр. музыка", 42);
                        txt(settingsT.themeParam, "Тема фона", 42);
                        txt(settingsT.sfxVolumeParam, "Звук. эф.", 42);
                        txt(settingsT.gameVolumeParam, "Громк. игр.", 42);
                        txt(settingsT.mainVolumeParam, "Громк. фон.", 42);
                        txt(settingsT.save, "Сохранить", 50);
                    }
                    else if (LANG == "ENG") {
                        txt(settingsT.exitToMenu, "Exit", 10);
                        txt(settingsT.timerParam, "Timer", 42);
                        txt(settingsT.difficultyParam, "Difficulty", 42);
                        txt(settingsT.music1Param, "Main song", 42);
                        txt(settingsT.music2Param, "Game song", 42);
                        txt(settingsT.themeParam, "Theme", 42);
                        txt(settingsT.sfxVolumeParam, "Sfx", 42);
                        txt(settingsT.gameVolumeParam, "Game", 42);
                        txt(settingsT.mainVolumeParam, "Main", 42);
                        txt(settingsT.save, "Save", 50);
                    }
                }
                else if (click(event, window, menuBtn.leaderBoard)) {
                    ifstream inputFile("users.json");
                    json usersData = json::parse(inputFile);
                    inputFile.close();

                    array<pair<string, int>, 10> leaderboard = {};

                    for (const auto& user : usersData["users"]) {
                        if (user["login"] != "" && user["best_score"] > 0) {
                            for (int i = 0; i < 10; ++i) {
                                if (leaderboard[i].second < user["best_score"]) {
                                    for (int j = 9; j > i; --j) {
                                        leaderboard[j] = leaderboard[j - 1];
                                    }
                                    leaderboard[i] = { user["login"], user["best_score"] };
                                    break;
                                }
                            }
                        }
                    }

                    json topUsers = json::array();
                    for (int i = 0; i < 10 && leaderboard[i].second > 0; ++i) {
                        topUsers.push_back({
                            {"login", leaderboard[i].first},
                            {"best_score", leaderboard[i].second}
                            });
                    }

                    topMargin = 15;
                    marginStep = 8.33;
                    int topUsersCount = 0;
                    for (const auto& user : topUsers) {
                        addInfoToWindow(leaderboardT.user[topUsersCount], font, to_string(user["login"]), 19, Color::White, 49, topMargin);
                        addInfoToWindow(leaderboardT.score[topUsersCount], font, to_string(user["best_score"]), 23, Color::White, 61, topMargin);
                        topMargin += marginStep;
                        topUsersCount += 1;
                    }

                    sfx.play();
                    gameStage = "LEADERBOARD";

                    if (LANG == "RU") {
                        txt(leaderboardT.numberTitle, "Но", 36);
                        txt(leaderboardT.userTitle, "Никнейм", 49);
                        txt(leaderboardT.scoreTitle, "Счёт", 61);
                        txt(leaderboardT.exit, "Выход", 10);
                    }
                    else if (LANG == "ENG") {
                        txt(leaderboardT.numberTitle, "No", 36);
                        txt(leaderboardT.userTitle, "User", 49);
                        txt(leaderboardT.scoreTitle, "Score", 61);
                        txt(leaderboardT.exit, "Exit", 10);
                    }
                }
                else if (click(event, window, menuBtn.exit)) {
                    loginInput = "";
                    passwordInput = "";
                    authT.password.setString("");
                    authT.login.setString("");

                    ifstream settingsFile(settingsFilepath);
                    json currentSettings = json::parse(settingsFile);
                    settingsFile.close();

                    ifstream usersFile("users.json");
                    json users = json::parse(usersFile);
                    usersFile.close();

                    for (auto& user : users["users"]) {
                        if (user["login"] == currentSettings["login"]) {

                            user["best_score"] = currentSettings["best_score"];
                            user["difficulty"] = currentSettings["difficulty"];
                            user["music1"] = currentSettings["music1"];
                            user["music2"] = currentSettings["music2"];
                            user["round_time"] = currentSettings["round_time"];
                            user["theme_number"] = currentSettings["theme_number"];
                            user["sfx_volume"] = currentSettings["sfx_volume"];
                            user["game_volume"] = currentSettings["game_volume"];
                            user["main_volume"] = currentSettings["main_volume"];
                            break;
                        }
                    }

                    ofstream outputUsersFile("users.json");
                    outputUsersFile << users.dump(4);
                    outputUsersFile.close();

                    sfx.play();
                    gameStage = "AUTH_REG";
                }
                else if (click(event, window, ruBtn)) {
                    sfx.play();

                    LANG = "RU";
                    txt(menuT.startGame, "Начать игру", 50);
                    txt(menuT.settings, "Настройки", 50);
                    txt(menuT.leaderboard, "Лидеры", 50);
                    txt(menuT.exit, "Выход", 50);
                }
                else if (click(event, window, engBtn)) {
                    sfx.play();

                    LANG = "ENG";
                    txt(menuT.startGame, "Start", 50);
                    txt(menuT.settings, "Settings", 50);
                    txt(menuT.leaderboard, "LeaderBoard", 50);
                    txt(menuT.exit, "Exit", 50);
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

            window.draw(ruBtn);
            window.draw(engBtn);

            window.draw(ruSprite);
            window.draw(engSprite);

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
                else if (mouseIn(window, ruBtn)) {
                    anyButtonHovered = true;
                }
                else if (mouseIn(window, engBtn)) {
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
                            sfx.play();

                            switch (i) {
                            case 0:
                                timeIndex = updateIndex(timeIndex, timesCount, plusOrMinus);
                                addInfoToWindow(timerOption, font, to_string(timesToRound[timeIndex]), 25, color1, 59, 7);
                                break;
                            case 1:
                                difIndex = updateIndex(difIndex, difCount, plusOrMinus);
                                addInfoToWindow(difOption, font, difToRound[difIndex], 25, color1, 59, 16.5);
                                break;
                            case 2:
                                music1Index = updateIndex(music1Index, music1Count, plusOrMinus);
                                addInfoToWindow(music1Option, font, to_string(music1ToRound[music1Index]), 25, color1, 59, 26);
                                break;
                            case 3:
                                music2Index = updateIndex(music2Index, music2Count, plusOrMinus);
                                addInfoToWindow(music2Option, font, to_string(music2ToRound[music2Index]), 25, color1, 59, 35.5);
                                break;
                            case 4:
                                themeIndex = updateIndex(themeIndex, themeCount, plusOrMinus);
                                addInfoToWindow(themeOption, font, to_string(themeToRound[themeIndex]), 25, color1, 59, 45);
                                break;
                            }
                        }
                    }
                }

                if (click(event, window, settingsBtn.exitToMenu)) {
                    sfx.play();
                    gameStage = "MENU";

                    if (LANG == "RU") {
                        txt(menuT.startGame, "Начать игру", 50);
                        txt(menuT.settings, "Настройки", 50);
                        txt(menuT.leaderboard, "Лидеры", 50);
                        txt(menuT.exit, "Выход", 50);
                    }
                    else if (LANG == "ENG") {
                        txt(menuT.startGame, "Start", 50);
                        txt(menuT.settings, "Settings", 50);
                        txt(menuT.leaderboard, "LeaderBoard", 50);
                        txt(menuT.exit, "Exit", 50);
                    }
                }
                else if (click(event, window, settingsBtn.save)) {
                    sfx.play();

                    settings["round_time"] = timesToRound[timeIndex];
                    settings["difficulty"] = difToRound[difIndex];
                    settings["theme_number"] = themeToRound[themeIndex];
                    settings["music1"] = music1ToRound[music1Index];
                    settings["music2"] = music2ToRound[music2Index];
                    settings["sfx_volume"] = sfxVolume;
                    settings["game_volume"] = gameVolume;
                    settings["main_volume"] = mainVolume;
                    settings["login"] = userLogin;
                    settings["password"] = userPassword;

                    roundTime = settings["round_time"];
                    difficulty = settings["difficulty"];
                    themeNumber = settings["theme_number"];
                    music1Number = settings["music1"];
                    music2Number = settings["music2"];
                    sfxVolume = settings["sfx_volume"];
                    gameVolume = settings["game_volume"];
                    mainVolume = settings["main_volume"];

                    std::ofstream out(settingsFilepath);
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
                    settingsT.sfxVolumeParam.setFillColor(color2);
                    settingsT.gameVolumeParam.setFillColor(color2);
                    settingsT.mainVolumeParam.setFillColor(color2);


                    timerOption.setFillColor(color1);
                    difOption.setFillColor(color1);
                    music1Option.setFillColor(color1);
                    music2Option.setFillColor(color1);
                    themeOption.setFillColor(color1);

                    addInfoToWindow(leaderboardT.numberTitle, font, "No", 25, color2, 36, 6.4);
                    addInfoToWindow(leaderboardT.userTitle, font, "User", 25, color2, 49, 6.4);
                    addInfoToWindow(leaderboardT.scoreTitle, font, "Score", 25, color2, 61, 6.4);

                    musicManager.play("musics/main" + to_string(music1ToRound[music1Index]) + ".ogg");
                    musicManager.setVolume(mainVolume);

                    sfx.setVolume(sfxVolume);

                }

                if (click(event, window, sfxThumb)) {
                    sfxThumbIsDragging = true;
                }
                else if (click(event, window, gameThumb)) {
                    gameThumbIsDragging = true;
                }
                else if (click(event, window, mainThumb)) {
                    mainThumbIsDragging = true;
                }
                else if (click(event, window, ruBtn)) {
                    sfx.play();

                    LANG = "RU";

                    txt(settingsT.exitToMenu, "Выход", 10);
                    txt(settingsT.timerParam, "Время", 42);
                    txt(settingsT.difficultyParam, "Сложность", 42);
                    txt(settingsT.music1Param, "Фон. музыка", 42);
                    txt(settingsT.music2Param, "Игр. музыка", 42);
                    txt(settingsT.themeParam, "Тема фона", 42);
                    txt(settingsT.sfxVolumeParam, "Звук. эф.", 42);
                    txt(settingsT.gameVolumeParam, "Громк. игр.", 42);
                    txt(settingsT.mainVolumeParam, "Громк. фон.", 42);
                    txt(settingsT.save, "Сохранить", 50);
                }
                else if (click(event, window, engBtn)) {
                    sfx.play();

                    LANG = "ENG";

                    txt(settingsT.exitToMenu, "Exit", 10);
                    txt(settingsT.timerParam, "Timer", 42);
                    txt(settingsT.difficultyParam, "Difficulty", 42);
                    txt(settingsT.music1Param, "Main song", 42);
                    txt(settingsT.music2Param, "Game song", 42);
                    txt(settingsT.themeParam, "Theme", 42);
                    txt(settingsT.sfxVolumeParam, "Sfx", 42);
                    txt(settingsT.gameVolumeParam, "Game", 42);
                    txt(settingsT.mainVolumeParam, "Main", 42);
                    txt(settingsT.save, "Save", 50);
                }

                if (event.type == Event::MouseButtonReleased && event.mouseButton.button == Mouse::Left) {
                    sfxThumbIsDragging = false;
                    gameThumbIsDragging = false;
                    mainThumbIsDragging = false;
                }
            }
            
            if (sfxThumbIsDragging) {
                sfxVolume = getVolumeValue(window, settingsBtn.sfx, sfxThumb);
            }
            else if (gameThumbIsDragging) {
                gameVolume = getVolumeValue(window, settingsBtn.game, gameThumb);
            }
            else if (mainThumbIsDragging) {
                mainVolume = getVolumeValue(window, settingsBtn.main, mainThumb);
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
            window.draw(settingsT.sfxVolumeParam);
            window.draw(settingsT.gameVolumeParam);
            window.draw(settingsT.mainVolumeParam);
            window.draw(settingsT.save);

            window.draw(timerOption);
            window.draw(difOption);
            window.draw(music1Option);
            window.draw(music2Option);
            window.draw(themeOption);


            for (int i = 0;i < 5;i++) {
                window.draw(settingsBtn.leftStrokes[i]);
                window.draw(settingsBtn.rightStrokes[i]);
            }

            window.draw(settingsBtn.sfx);
            window.draw(settingsBtn.game);
            window.draw(settingsBtn.main);

            window.draw(sfxThumb);
            window.draw(gameThumb);
            window.draw(mainThumb);

            window.draw(ruBtn);
            window.draw(engBtn);

            window.draw(ruSprite);
            window.draw(engSprite);

            drawCursor(window, cursorManager, anyButtonHovered);

            window.display();
        }
        else if (gameStage == "GAME") {
            musicManager.stop();
            musicManager.play("musics/game" + to_string(music2ToRound[music2Index]) + ".ogg", gameVolume);

            anyButtonHovered = false;
            window.setMouseCursorVisible(false);
            string filepath;
            if (LANG == "RU") {
                filepath = "ru_words/" + difficulty + "RU.txt";
                targetWord = getRandomWord(filepath);
                cout << "name" << filepath << " targetWord" << targetWord << endl;
            }
            else if (LANG == "ENG") {
                filepath = "eng_words/" + difficulty + "ENG.txt";
                targetWord = getRandomWord("eng_words/" + difficulty + "ENG.txt");
                cout << "name" << filepath << " targetWord" << targetWord << endl;

            }

            if (targetWord.empty()) {
                cerr << "No words available!" << endl;
                return EXIT_FAILURE;
                continue;
            }

            counter = 0;
            playerInput.clear();
            guessedCount = 0;

            availableLetters = createLetterMap(targetWord);
            printLetterMap(availableLetters);

            currentLetters = availableLetters;
            if (LANG == "RU") {
                addInfoToWindow(gameT.pause, font, "Пауза", 40, Color::White, 12.5, 5.7);
                addInfoToWindow(gameT.counter, font, "Счёт:  " + to_string(counter), 40, Color::White, 58, 5.7);
                addInfoToWindow(gameT.timer, font, "Время:  ", 40, Color::White, 83, 5.7);
                addInfoToWindow(gameT.target, font, "Случайное слово: " + targetWord, 36, Color::White, 50, 26);
                addInfoToWindow(gameT.input, font, "Ты ввёл:        ", 40, Color::White, 33, 55);
                addInfoToWindow(gameT.endGame, font, "Завершить игру", 40, Color::White, 88, 90);
            }
            else if (LANG == "ENG") {
                addInfoToWindow(gameT.pause, font, "Pause", 40, Color::White, 12.5, 5.7);
                addInfoToWindow(gameT.counter, font, "Score:  " + to_string(counter), 40, Color::White, 58, 5.7);
                addInfoToWindow(gameT.timer, font, "Timer:  ", 40, Color::White, 83, 5.7);
                addInfoToWindow(gameT.target, font, "Random word is: " + targetWord, 36, Color::White, 50, 26);
                addInfoToWindow(gameT.input, font, "Your input:   ", 40, Color::White, 33, 55);
                addInfoToWindow(gameT.endGame, font, "End Game", 40, Color::White, 88, 90);
            }

            sf::Text inputText;
            inputText.setFont(font);
            inputText.setCharacterSize(24);
            inputText.setFillColor(sf::Color::White);
            inputText.setPosition(50, 100);

            sf::Text availableText;
            availableText.setFont(font);
            availableText.setCharacterSize(24);
            availableText.setFillColor(sf::Color::Green);
            availableText.setPosition(50, 150);
            availableText.setString(L"Доступные буквы: " + sourceWord);

            gameClock.restart();

            while (gameStage == "GAME" && window.isOpen()) {
                Event event;
                updateTimer(gameClock, timeRemaining, gameT.timer, isPaused, LANG);
                if (isPaused) {
                    gameT.input.setFillColor(Color(150, 150, 150)); 
                }
                else {
                    gameT.input.setFillColor(Color::White);
                }
                while (window.pollEvent(event)) {
                    closeEvents(event, window);
                    Vector2i mousePos = Mouse::getPosition(window);
                    if (click(event, window, gameT.endGame)) {
                        sfx.play();
                        gameStage = "MENU";
                    }
                    else if (click(event, window, gameBtn.pause)) {
                        sfx.play();

                        isPaused = !isPaused;
                        if (isPaused) {
                            if (LANG == "RU") {
                                addInfoToWindow(gameT.pause, font, "Продолжить", 40, Color::White, 13, 5.7);
                            }
                            else if (LANG == "ENG") {
                                addInfoToWindow(gameT.pause, font, "Resume", 40, Color::White, 13, 5.7);
                            }
                        }
                        else {
                            if (LANG == "RU") {
                                addInfoToWindow(gameT.pause, font, "Пауза", 40, Color::White, 12.5, 5.7);
                            }
                            else if (LANG == "ENG") {
                                addInfoToWindow(gameT.pause, font, "Pause", 40, Color::White, 12.5, 5.7);
                            }
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
                    
                    if (LANG == "ENG") {
                        if (event.type == Event::TextEntered && !isPaused) {
                            if (event.text.unicode == '\b') {
                                if (!playerInput.empty()) {
                                    char lastChar = playerInput.back();
                                    currentLetters[lastChar]++;
                                    playerInput.pop_back();
                                }
                            }
                            else if (isalpha(static_cast<char>(event.text.unicode))) {
                                char c = tolower(static_cast<char>(event.text.unicode));
                                if (currentLetters[c] > 0) {
                                    playerInput += c;
                                    currentLetters[c]--;
                                }
                            }
                            if (LANG == "RU") {
                                gameT.input.setString("Ты ввёл:        " + playerInput);
                            }
                            else if (LANG == "ENG") {
                                gameT.input.setString("Your input:   " + playerInput);
                            }
                        }

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
                                    if (LANG == "RU") {
                                        addInfoToWindow(gameT.counter, font, "Счёт:  " + to_string(counter), 40, Color::White, 58, 5.7);
                                    }
                                    else if (LANG == "ENG") {
                                        addInfoToWindow(gameT.counter, font, "Score:  " + to_string(counter), 40, Color::White, 58, 5.7);
                                    }
                                }
                            }

                            currentLetters = availableLetters;
                            playerInput.clear();
                            if (LANG == "RU") {
                                gameT.input.setString("Ты ввёл:        " + playerInput);
                            }
                            else if (LANG == "ENG") {
                                gameT.input.setString("Your input:   " + playerInput);
                            }
                        }
                    }
                    else if (LANG == "RU") {
                        if (event.type == sf::Event::TextEntered) {
                            wchar_t enteredChar = static_cast<wchar_t>(event.text.unicode);

                            if (enteredChar == L'\b') { // Backspace
                                if (!currentInputR.empty()) {
                                    availableLettersR += currentInputR.back();
                                    currentInputR.pop_back();
                                }
                            }
                            else if (enteredChar == L'\r') { // Enter
                                if (checker.checkWord(currentInputR)) {
                                    counter += currentInputR.length();
                                    if (LANG == "RU") {
                                        addInfoToWindow(gameT.counter, font, "Счёт:  " + to_string(counter), 40, Color::White, 58, 5.7);
                                    }
                                    else if (LANG == "ENG") {
                                        addInfoToWindow(gameT.counter, font, "Score:  " + to_string(counter), 40, Color::White, 58, 5.7);
                                    }
                                }
                                // Возвращаем все буквы обратно
                                availableLettersR += currentInputR;
                                currentInputR.clear();
                            }
                            else if (isRussianChar(enteredChar)) {
                                wchar_t lowerChar = std::tolower(enteredChar, std::locale());
                                size_t pos = availableLettersR.find(lowerChar);
                                if (pos != std::wstring::npos) {
                                    currentInputR += lowerChar;
                                    availableLettersR.erase(pos, 1);
                                }
                            }

                            if (LANG == "RU") {
                                gameT.input.setString("Ты ввёл:        " + currentInputR);
                            }
                            else if (LANG == "ENG") {
                                gameT.input.setString("Your input:   " + currentInputR);
                            }

                            inputText.setString(L"Ввод: " + currentInputR);
                            availableText.setString(L"Доступные буквы: " + availableLettersR);
                        }
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

                window.draw(inputText);
                window.draw(availableText);

                if (!isPaused) {
                    if (LANG == "RU") {
                        addInfoToWindow(gameT.endGame, font, "Завершить игру", 40, Color::White, 80, 90);
                    }
                    else if (LANG == "ENG") {
                        addInfoToWindow(gameT.endGame, font, "End Game", 40, Color::White, 88, 90);
                    }
                    window.draw(gameT.endGame);
                }
                else {
                    if (LANG == "RU") {
                        addInfoToWindow(gameT.endGame, font, "Завершить игру", 40, Color::White, 150, 110);
                    }
                    else if (LANG == "ENG") {
                        addInfoToWindow(gameT.endGame, font, "End Game", 40, Color::White, 150, 110);
                    }
                    window.draw(gameT.endGame);
                }

                window.draw(gameBtn.pause);

                drawCursor(window, cursorManager, anyButtonHovered);

                window.display();

                if (timeRemaining <= 0) {
                    gameStage = "ENDGAME";

                    plaingLang = LANG;
                    langBonusText = plaingLang == "RU" ? "(ru)  x0.1: " : "(eng)  x0.5: ";
                    langBonus = plaingLang == "RU" ? 0.1 : 0.5;

                    if (LANG == "RU") {
                        txt(endgameT.restart, "Заново", 50);
                        txt(endgameT.exit, "Выход", 50);
                    }
                    else if (LANG == "ENG") {
                        txt(endgameT.restart, "Restart", 50);
                        txt(endgameT.exit, "Exit", 50);
                    }
                }
            }
        }
        else if (gameStage == "ENDGAME") {
            musicManager.stop();

            Event event;
            window.setMouseCursorVisible(false);
            if (LANG == "RU") {
                addInfoToWindow(endgameT.score, font, "Твой результат: " + to_string(counter), 30, color1, 50, 10);
            }
            else if (LANG == "ENG") {
                addInfoToWindow(endgameT.score, font, "Your score: " + to_string(counter), 30, color1, 50, 10);
            }

            switch (difIndex) {
            case 0:
                calculatedResult = counter + counter * 0.1;
                difBonus = "(easy)  x0.1: " + formatFloat(counter * 0.1);
                break;
            case 1:
                calculatedResult = counter + counter * 0.3;
                difBonus = "(normal)  x0.3: " + formatFloat(counter * 0.3);
                break;
            case 2:
                calculatedResult = counter + counter * 0.6;
                difBonus = "(hard)  x0.6: " + formatFloat(counter * 0.6);
                break;
            }

            switch (timeIndex) {
            case 0:
                calculatedResult = calculatedResult + counter * 0.7;
                timeBonus = "(30s)  x0.7: " + formatFloat(counter * 0.7);
                break;
            case 1:
                calculatedResult = calculatedResult + counter * 0.5;
                timeBonus = "(60s)  x0.5: " + formatFloat(counter * 0.5);
                break;
            case 2:
                calculatedResult = calculatedResult + counter * 0.3;
                timeBonus = "(90s)  x0.3: " + formatFloat(counter * 0.3);
                break;
            case 3:
                calculatedResult = calculatedResult + counter * 0.1;
                timeBonus = "(120s)  x0.1: " + formatFloat(counter * 0.1);
                break;
            }

            if (LANG == "RU") {
                calculatedResult = calculatedResult + counter * langBonus;
                addInfoToWindow(endgameT.langBonus, font, "Яз. бонус" + langBonusText + formatFloat(counter * langBonus), 20, color1, 50, 25);
                addInfoToWindow(endgameT.difficultyBonus, font, "Бонус сложности" + difBonus, 20, color1, 50, 15);
                addInfoToWindow(endgameT.timeBonus, font, "Бонус за время" + timeBonus, 20, color1, 50, 20);
            }
            else if (LANG == "ENG") {
                calculatedResult = calculatedResult + counter * langBonus;
                addInfoToWindow(endgameT.langBonus, font, "Lang bonus" + langBonusText + formatFloat(counter * langBonus), 20, color1, 50, 25);
                addInfoToWindow(endgameT.difficultyBonus, font, "Difficulty bonus" + difBonus, 20, color1, 50, 15);
                addInfoToWindow(endgameT.timeBonus, font, "Time bonus" + timeBonus, 20, color1, 50, 20);
            }

            if (calculatedResult > bestScore) {
                bestScore = floor(calculatedResult);
                settings["best_score"] = bestScore;
                std::ofstream out(settingsFilepath);
                out << settings.dump(4);
                if (LANG == "RU") {
                    addInfoToWindow(endgameT.isrecord, font, "Новый рекорд!!!", 20, color1, 50, 40);
                }
                else if (LANG == "ENG") {
                    addInfoToWindow(endgameT.isrecord, font, "New record!!!", 20, color1, 50, 40);
                }
            }

            if (LANG == "RU") {
                addInfoToWindow(endgameT.totalScore, font, "Конечный результат: " + formatFloat(floor(calculatedResult)), 30, color1, 50, 35);
            }
            else if (LANG == "ENG") {
                addInfoToWindow(endgameT.totalScore, font, "Total score: " + formatFloat(floor(calculatedResult)), 30, color1, 50, 35);
            }

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
                else if (mouseIn(window, ruBtn)) {
                    anyButtonHovered = true;
                }
                else if (mouseIn(window, engBtn)) {
                    anyButtonHovered = true;
                }
                else {
                    endgameT.restart.setFillColor(color1);
                    endgameT.exit.setFillColor(color1);
                    anyButtonHovered = false;
                }

                if (click(event, window, endgameBtn.restart)) {
                    calculatedResult = 0;
                    addInfoToWindow(endgameT.isrecord, font, "", 20, color1, 50, 40);

                    timeRemaining = roundTime;
                    gameT.timer.setString("Timer:  ");
                    gameT.input.setString("Your input: ");
                    window.setMouseCursorVisible(true);
                    endgameT.restart.setFillColor(color1);

                    musicManager.stop();
                    sfx.play();

                    ifstream settingsFile(settingsFilepath);
                    json currentSettings = json::parse(settingsFile);
                    settingsFile.close();

                    ifstream usersFile("users.json");
                    json users = json::parse(usersFile);
                    usersFile.close();

                    for (auto& user : users["users"]) {
                        if (user["login"] == currentSettings["login"]) {
                            user["best_score"] = currentSettings["best_score"];
                            break;
                        }
                    }

                    ofstream outputUsersFile("users.json");
                    outputUsersFile << users.dump(4);
                    outputUsersFile.close();

                    gameStage = "GAME";
                }
                else if (click(event, window, endgameBtn.exit)) {
                    calculatedResult = 0;
                    addInfoToWindow(endgameT.isrecord, font, "", 20, color1, 50, 40);

                    endgameT.exit.setFillColor(color1);
                    sfx.play();

                    ifstream settingsFile(settingsFilepath);
                    json currentSettings = json::parse(settingsFile);
                    settingsFile.close();

                    ifstream usersFile("users.json");
                    json users = json::parse(usersFile);
                    usersFile.close();

                    for (auto& user : users["users"]) {
                        if (user["login"] == currentSettings["login"]) {
                            user["best_score"] = currentSettings["best_score"];
                            break;
                        }
                    }

                    ofstream outputUsersFile("users.json");
                    outputUsersFile << users.dump(4);
                    outputUsersFile.close();

                    if (LANG == "RU") {
                        txt(menuT.startGame, "Начать игру", 50);
                        txt(menuT.settings, "Настройки", 50);
                        txt(menuT.leaderboard, "Лидеры", 50);
                        txt(menuT.exit, "Выход", 50);
                    }
                    else if (LANG == "ENG") {
                        txt(menuT.startGame, "Start", 50);
                        txt(menuT.settings, "Settings", 50);
                        txt(menuT.leaderboard, "LeaderBoard", 50);
                        txt(menuT.exit, "Exit", 50);
                    }

                    gameStage = "MENU";
                }
                else if (click(event, window, ruBtn)) {
                    LANG = "RU";
                    txt(endgameT.restart, "Заново", 50);
                    txt(endgameT.exit, "Выход", 50);
                }
                else if (click(event, window, engBtn)) {
                    LANG = "ENG";
                    txt(endgameT.restart, "Restart", 50);
                    txt(endgameT.exit, "Exit", 50);
                }
            }

            window.clear();
            window.draw(endgameBg.sprite);

            window.draw(endgameT.restart);
            window.draw(endgameT.exit);
            window.draw(endgameT.score);
            window.draw(endgameT.isrecord);
            window.draw(endgameT.difficultyBonus);
            window.draw(endgameT.timeBonus);
            window.draw(endgameT.langBonus);

            window.draw(endgameT.totalScore);

            window.draw(endgameBtn.restart);
            window.draw(endgameBtn.exit);

            window.draw(ruBtn);
            window.draw(engBtn);

            window.draw(ruSprite);
            window.draw(engSprite);

            drawCursor(window, cursorManager, anyButtonHovered);

            window.display();
        }
    }

    return 0;
}