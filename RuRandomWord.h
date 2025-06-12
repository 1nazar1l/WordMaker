#pragma once

#include <string>
#include <fstream>
#include <iostream>
#include <ctime>
#include <cstdlib>
#include <vector>
#include <locale>

class RuRandomWord {
public:
    RuRandomWord(const std::string& filename) : filename(filename) {
        std::srand(static_cast<unsigned>(std::time(nullptr)));
    }

    void loadWords() {
        std::wifstream inputFile(filename);
        inputFile.imbue(std::locale(""));

        if (!inputFile) {
            std::wcerr << L"Не удалось открыть файл: " << filename.c_str() << std::endl;
            return;
        }

        std::wstring word;
        while (std::getline(inputFile, word)) {
            if (!word.empty()) {
                words.push_back(word);
            }
        }
    }

    std::wstring getRandomWord() {
        if (words.empty()) {
            std::wcerr << L"Слова не загружены или файл пуст!" << std::endl;
            return L"";
        }
        return words[std::rand() % words.size()];
    }

private:
    std::vector<std::wstring> words;
    std::string filename;
};