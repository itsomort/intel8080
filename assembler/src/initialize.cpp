#include <iostream>
#include <fstream>
#include <map>
#include <string>

std::map<std::string, std::pair<int, int>> map_str() {
    std::map<std::string, std::pair<int, int>> fmap;
    std::ifstream file("CODES.txt");

    std::string mnemonic;
    int type, length;
    while(file >> mnemonic >> type >> length) {
        fmap[mnemonic] = std::make_pair(type, length);
    }

    return fmap;
}

int main() {
    std::map<std::string, std::pair<int, int>> map = map_str();
    std::ifstream file("test.txt");
    std::string line;
    while(std::getline(file, line)) {
        std::string m = line.substr(0, line.find(" "));
        std::cout << m << " " << map[m].first << " " << map[m].second << "\n";
    }
}