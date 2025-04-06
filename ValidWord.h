#pragma once

#include <fstream>
#include <iostream>
#include <string>  // Добавляем для корректной работы string

using namespace std;

class ValidWord {
public:
    ValidWord(const string& filename);
    void loadWords();
    bool isValid(const string& wordToCheck);  // Возвращает bool и принимает слово для проверки

private:
    string words[1000];
    int wordsCount;
    string filename;
};

ValidWord::ValidWord(const string& filename) : filename(filename), wordsCount(0) {}

void ValidWord::loadWords() {
    ifstream inputFile(filename);
    wordsCount = 0;

    if (!inputFile) {
        cerr << "Не удалось открыть файл: " << filename << endl;
        return;
    }

    while (wordsCount < 1000 && inputFile >> words[wordsCount]) {
        wordsCount++;
    }
    cout << "Loaded words: " << wordsCount << endl;  // Отладочный вывод
    inputFile.close();
    for (int i = 0; i < wordsCount; i++) {
        cout << words[i] << endl;
    }
}

bool ValidWord::isValid(const string& wordToCheck) {
    for (int i = 0; i < wordsCount; ++i) {
        if (words[i] == wordToCheck) {
            return true;  // Слово найдено
        }
    }
    return false;  // Слово не найдено
}