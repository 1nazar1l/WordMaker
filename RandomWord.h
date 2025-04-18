#pragma once
#include <string>
#include <fstream>
#include <iostream>
#include <ctime>
#include <cstdlib>

class RandomWord {
public:
    RandomWord(const std::string& filename) : filename(filename) {
        std::srand(static_cast<unsigned>(std::time(nullptr)));
    }

    void loadWords() {
        std::ifstream inputFile(filename);
        wordCount = 0;

        if (!inputFile) {
            std::cerr << "Failed to open file: " << filename << std::endl;
            return;
        }

        while (wordCount < MAX_WORDS && inputFile >> words[wordCount]) {
            wordCount++;
        }
    }

    std::string getRandomWord() {
        if (wordCount == 0) {
            std::cerr << "No words loaded!" << std::endl;
            return "";
        }
        return words[std::rand() % wordCount];
    }

private:
    static const int MAX_WORDS = 1000;
    std::string words[MAX_WORDS];
    int wordCount = 0;
    std::string filename;
};