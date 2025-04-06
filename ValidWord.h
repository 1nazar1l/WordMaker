#pragma once

#include <fstream>
#include <iostream>
#include <string>  // ��������� ��� ���������� ������ string

using namespace std;

class ValidWord {
public:
    ValidWord(const string& filename);
    void loadWords();
    bool isValid(const string& wordToCheck);  // ���������� bool � ��������� ����� ��� ��������

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
        cerr << "�� ������� ������� ����: " << filename << endl;
        return;
    }

    while (wordsCount < 1000 && inputFile >> words[wordsCount]) {
        wordsCount++;
    }
    cout << "Loaded words: " << wordsCount << endl;  // ���������� �����
    inputFile.close();
    for (int i = 0; i < wordsCount; i++) {
        cout << words[i] << endl;
    }
}

bool ValidWord::isValid(const string& wordToCheck) {
    for (int i = 0; i < wordsCount; ++i) {
        if (words[i] == wordToCheck) {
            return true;  // ����� �������
        }
    }
    return false;  // ����� �� �������
}