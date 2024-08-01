#include <iostream>
#include <vector>
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

void assemble(std::map<std::string, std::pair<int, int>> map, std::ifstream *file) {
    // run through file and assemble based on type
    std::string line;
    std::map<std::string, int> regmap = {{"B", 0b000}, {"C", 0b001}, {"D", 0b010}, {"E", 0b011},
        {"H", 0b100}, {"L", 0b101}, {"M", 0b110}, {"A", 0b111}};

    while(std::getline(*file, line)) {
        std::vector<char> bytes;
        std::string m = line.substr(0, line.find(" ")); // get mnemonic (labels are later)
        if(map.find(m) == map.end()) continue; // skip if mnemonic not found

        int type = map[m].first;
        int length = map[m].second;

        switch(type) {
            case 1: // CARRY BIT
                if(m == "STC")      bytes.push_back(0b00110111);
                else if(m == "CMC") bytes.push_back(0b00111111);
            break;

            case 2: // SINGLE REGISTER
                if(m == "CMA") {bytes.push_back(0b00101111); break;}
                // both INR and DCR have one argument
                std::string arg = line.substr(line.find(" ") + 1, line.length() - 2);
                if(regmap.find(arg) == regmap.end()) continue; // skip if incorrect register
                int code = regmap[arg];
                if(m == "INR") bytes.push_back((code << 3) + 0b100); // 00XXX100
                else if(m == "DCR") bytes.push_back((code << 3) + 0b101); //00XX101
            break;
        }

        for(char i : bytes) std::cout << std::hex << (int) i << " ";
        std::cout << "\n";
    }
}

int main() {
    std::map<std::string, std::pair<int, int>> map = map_str();
    std::ifstream file("test.asm");
    
    assemble(map, &file);
}