#pragma once

#include <fstream>

// TODO: Get more robust parser which better handles spaces.

class IniParser {
public:
    bool load(const std::string& filePath) {
        std::ifstream file(filePath);
        if (!file.is_open()) {
            logfff(0, "Failed to open file: %s\n", filePath.c_str());
            return false;
        }

        std::string line, section;
        while (std::getline(file, line)) {
            line = trim(line);

            if (line.empty() || line[0] == ';' || line[0] == '#') {
                continue;
            }

            if (line.front() == '[' && line.back() == ']') {
                section = line.substr(1, line.size() - 2);
                continue;
            }

            size_t equalsPos = line.find('=');
            if (equalsPos != std::string::npos) {
                std::string key = trim(line.substr(0, equalsPos));
                std::string value = trim(line.substr(equalsPos + 1));
                data[section][key] = value;
            }
        }
        return true;
    }

    std::string get(const std::string& section, const std::string& key, const std::string& defaultValue = "") const {
        auto secIt = data.find(section);
        if (secIt != data.end()) {
            auto keyIt = secIt->second.find(key);
            if (keyIt != secIt->second.end()) {
                return keyIt->second;
            }
        }
        return defaultValue;
    }

private:
    std::map<std::string, std::map<std::string, std::string>> data;

    std::string trim(const std::string& str) const {
        const char* whitespace = " \t\n\r";
        size_t start = str.find_first_not_of(whitespace);
        if (start == std::string::npos) return "";
        size_t end = str.find_last_not_of(whitespace);
        return str.substr(start, end - start + 1);
    }
};

