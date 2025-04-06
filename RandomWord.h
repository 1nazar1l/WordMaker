#pragma once

#include <string>
#include <fstream>
#include <iostream>
#include <cstdlib>
#include <ctime>   
#include <chrono>
#include <thread>

using namespace std;

int getRandomNumber() {
    srand(static_cast<unsigned>(time(0)));
    int m1 = rand();
    this_thread::sleep_for(chrono::seconds(1));
    int m2 = rand();
    m1 = m2;
    return m1;
}

int getRandomIndex(int wordCount) {
    if (wordCount == 0) {
        return 0;
    }
    return getRandomNumber() % wordCount;
}

class RandomWord {
public:
    RandomWord(const string& filename);
    void loadWords();
    string getRandomWord();

private:
    string words[100];
    int wordCount;
    string filename; 
};

RandomWord::RandomWord(const string& filename) : filename(filename), wordCount(0) {}

void RandomWord::loadWords() {
    ifstream inputFile(filename);
    wordCount = 0;

    if (!inputFile) {
        cerr << "Не удалось открыть файл: " << filename << endl;
        return;
    }

    while (wordCount < 100 && inputFile >> words[wordCount]) {
        wordCount++;
    }

    inputFile.close();
}

string RandomWord::getRandomWord() {
    if (wordCount == 0) {
        return "";
    }

    int index = getRandomIndex(wordCount);
    return words[index];
}