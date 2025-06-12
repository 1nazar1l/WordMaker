#pragma once

#include <fstream>
#include <iostream>
#include <string>  
#include <vector>

class RuValidWord {
private:
    std::vector<std::wstring> dictionary;

public:
    RuValidWord(const std::string& dictionaryPath) {
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