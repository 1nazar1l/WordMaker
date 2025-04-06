#include <SFML/Graphics.hpp>
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

void updateAndDraw(RenderWindow& window, const Text& counterText, const Text& targetText, const Text& inputText, const Text& lettersText, const Text& guessedText, const Text& timerText) {
    window.clear(Color::Black);
    window.draw(counterText);
    window.draw(targetText);
    window.draw(inputText);
    window.draw(lettersText);
    window.draw(guessedText);
    window.draw(timerText);
    window.display();
}

string getRandomWord(string randomWordsFile) {
    RandomWord randomWord(randomWordsFile);
    randomWord.loadWords();
    return randomWord.getRandomWord();
}

void addInfoToWindow(Text& word, Font& font, const string& str, int fontSize, Color color, int xPosition, int yPosition) {
    word.setFont(font);
    word.setString(str);
    word.setCharacterSize(fontSize);
    word.setFillColor(color);
    word.setPosition(xPosition, yPosition); 
}

void updateTimer(Clock& gameClock, float& timeRemaining, Text& timerText) {
    float elapsed = gameClock.restart().asSeconds();
    timeRemaining -= elapsed;
    if (timeRemaining < 0) timeRemaining = 0;
    timerText.setString("Time: " + to_string((int)timeRemaining));
}

int main() {
    ValidWord validator("validWords.txt");
    validator.loadWords();

    string targetWord = getRandomWord("randomWords.txt");
    if (targetWord.empty()) {
        cerr << "No words available!" << endl;
        return EXIT_FAILURE;
    }

    RenderWindow window(VideoMode(800, 600), "Word Game");

    Font font;
    if (!font.loadFromFile("arial.ttf")) {
        cerr << "Failed to load font!" << endl;
        return EXIT_FAILURE;
    }

    Text timerText;
    Text counterText;
    Text targetText;
    Text inputText;
    Text lettersText;
    Text guessedText;

    Clock gameClock;
    float timeRemaining = 30.0f; // 30 секунд
    
    unsigned int counter = 0;

    string playerInput;

    // Получаем буквы которые пользователь сможет вводить(буквы получаются из случайного слова)
    auto availableLetters = createLetterMap(targetWord);
    auto currentLetters = availableLetters;
    string lettersInfo = "Available letters: ";
    for (auto& pair : availableLetters) {
        lettersInfo += string(1, pair.first) + "(" + to_string(pair.second) + ") ";
    }

    string guessedWords[100];
    int guessedCount = 0;


    addInfoToWindow(timerText, font, "", 24, Color::Red, 650, 20);
    addInfoToWindow(counterText, font, "Count: " + to_string(counter), 24, Color::White, 20, 20);
    addInfoToWindow(targetText, font, "Word: " + targetWord, 30, Color::Green, 20, 60);
    addInfoToWindow(inputText, font, "", 40, Color::White, 20, 120);
    addInfoToWindow(lettersText, font, lettersInfo, 20, Color::Yellow, 20, 180);
    addInfoToWindow(guessedText, font, "", 20, Color::Cyan, 400, 60);

    while (window.isOpen()) {
        Event event;
        updateTimer(gameClock, timeRemaining, timerText); // Обновление таймера

        while (window.pollEvent(event)) {
            if (event.type == Event::Closed || (event.type == Event::KeyPressed && event.key.code == Keyboard::Escape)) {
                window.close();
            }
            // Ввод текста
            if (event.type == Event::TextEntered) {
                if (event.text.unicode == '\b') {
                    if (!playerInput.empty()) {
                        char lastChar = tolower(playerInput.back());
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
                inputText.setString("Your input: " + playerInput);
            }

            if (event.type == Event::KeyPressed && event.key.code == Keyboard::Enter) {
                if (validator.isValid(playerInput) && !playerInput.empty()) {
                    // Было ли слово угадано ранее
                    bool alreadyGuessed = false;
                    for (int i = 0; i < guessedCount; ++i) {
                        if (guessedWords[i] == playerInput) {
                            alreadyGuessed = true;
                            break;
                        }
                    }

                    if (!alreadyGuessed && guessedCount < 100) {
                        guessedWords[guessedCount++] = playerInput;
                        counter++;

                        string guessedStr = "Guessed words:\n";
                        for (int i = 0; i < guessedCount; ++i) {
                            guessedStr += guessedWords[i] + "\n";
                        }
                        guessedText.setString(guessedStr);
                    }
                }
                counterText.setString("Count: " + to_string(counter));
                currentLetters = availableLetters;
                playerInput.clear();
                inputText.setString("Your input: ");
            }
        }

        updateAndDraw(window, counterText, targetText, inputText, lettersText, guessedText, timerText);
    }

    return 0;
}