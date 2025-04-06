#include <SFML/Graphics.hpp>
#include "RandomWord.h"
#include "ValidWord.h"
#include <unordered_map> // для создания словарей
#include <algorithm>

using namespace std;
using namespace sf;

// Создает словарь с доступными для ввода буквами(пример: слово: стакан. доступны буквы: с(1), т(1), а(2), к(1), н(1))
unordered_map<char, int> createLetterMap(const string& word) {
    unordered_map<char, int> letters; //char-тип ключа, int-тип значения
    for (char c : word) {
        letters[tolower(c)]++;
    }
    return letters;
}

int main() {
    setlocale(LC_ALL, "");
    ValidWord validator("validWords.txt");
    validator.loadWords();

    RandomWord randomWord("randomWords.txt");
    randomWord.loadWords();

    // Создание окна
    RenderWindow window(VideoMode(800, 600), "Word Game");

    // Загрузка шрифта
    Font font;
    if (!font.loadFromFile("arial.ttf")) {
        cerr << "Failed to load font!" << endl;
        return EXIT_FAILURE;
    }

    // Получение случайного слова (один раз при старте)
    string targetWord = randomWord.getRandomWord();
    if (targetWord.empty()) {
        cerr << "No words available!" << endl;
        return EXIT_FAILURE;
    }

    // Создает словарь с доступными для ввода буквами
    auto availableLetters = createLetterMap(targetWord);
    // Нужно для того чтобы если пользователь ввел букву из словаря она от туда убиралась и ее больше нельзя было ввести, а если удалил букву то она обратно вырнулась
    auto currentLetters = availableLetters;

    // Счетчик
    unsigned int counter = 0;
    Text counterText;
    counterText.setFont(font);
    counterText.setString("Count: " + to_string(counter));
    counterText.setCharacterSize(24);
    counterText.setFillColor(Color::White);
    counterText.setPosition(20, 20);

    // Текст с загаданным словом
    Text targetText("Word: " + targetWord, font, 30);
    targetText.setFillColor(Color::Green);
    targetText.setPosition(20, 60);

    // Поле ввода игрока
    string playerInput;
    Text inputText("", font, 40);
    inputText.setFillColor(Color::White);
    inputText.setPosition(20, 120);

    // Текст с доступными буквами
    string lettersInfo = "Available letters: ";
    for (auto& pair : availableLetters) {
        lettersInfo += string(1, pair.first) + "(" + to_string(pair.second) + ") ";
    }
    Text lettersText(lettersInfo, font, 20);
    lettersText.setFillColor(Color::Yellow);
    lettersText.setPosition(20, 180);

    // Массив для угаданных слов (статика)
    string guessedWords[100];
    int guessedCount = 0;
    Text guessedText("", font, 20);
    guessedText.setFillColor(Color::Cyan);
    guessedText.setPosition(400, 60);

    while (window.isOpen()) {
        Event event;
        while (window.pollEvent(event)) {
            if (event.type == Event::Closed ||
                (event.type == Event::KeyPressed && event.key.code == Keyboard::Escape)) {
                window.close();
            }

            // Обработка ввода текста
            if (event.type == Event::TextEntered) {
                if (event.text.unicode == '\b') { // Backspace
                    // Если пользователь удалил последнюю букву то она возвращается в список доступных для ввода
                    if (!playerInput.empty()) {
                        char lastChar = tolower(playerInput.back());
                        currentLetters[lastChar]++; 
                        playerInput.pop_back(); // удаляет последнюю выведенную букву с экрана
                    }
                }
                else if (isalpha(static_cast<char>(event.text.unicode))) { // приводит text к типу char и проверяет является ли буквой
                    char c = tolower(static_cast<char>(event.text.unicode));
                    if (currentLetters[c] > 0) {
                        playerInput += c;
                        currentLetters[c]--; // убирает букву из доступных для ввода
                    }
                }
                inputText.setString("Your input: " + playerInput);
            }

            // Обработка Enter
            if (event.type == Event::KeyPressed && event.key.code == Keyboard::Enter) {
                if (validator.isValid(playerInput) && !playerInput.empty()) {
                    // Проверяем, не угадано ли уже слово
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

                        // Обновляем текст угаданных слов
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

        window.clear(Color::Black);
        window.draw(counterText);
        window.draw(targetText);
        window.draw(inputText);
        window.draw(lettersText);
        window.draw(guessedText);
        window.display();
    }

    return 0;
}