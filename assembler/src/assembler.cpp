#include <iostream>
#include <vector>
#include <fstream>
#include <map>
#include <string>

void printError(int line, std::string text) {
    std::cout << "Line " << line << ": " << text << "\n";
}

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
    std::ofstream fs("output.bin", std::ios::out | std::ios::binary);
    int linenum = 0;


    while(std::getline(*file, line)) {
        linenum++;
        std::vector<char> bytes;
        std::string m = line.substr(0, line.find(" ")); // get mnemonic (labels are later)
        if(map.find(m) == map.end()) {
            printError(linenum, "Mnemonic not found.");
            continue;
        } // skip if mnemonic not found

        int type = map[m].first;
        int length = map[m].second;

        switch(type) {
            case 1: { // CARRY BIT
                if(m == "STC")      bytes.push_back(0b00110111);
                else if(m == "CMC") bytes.push_back(0b00111111);
                break; 
            }

            case 2: { // SINGLE REGISTER
                if(m == "CMA") {
                    bytes.push_back(0b00101111); 
                    break;
                }
                // both INR and DCR have one argument
                std::string arg = line.substr(line.find(" ") + 1, 1);
                if(regmap.find(arg) == regmap.end()) {
                    printError(linenum, "Register not found.");
                    continue;
                } // skip if incorrect register
                int code = regmap[arg];
                if(m == "INR") bytes.push_back((code << 3) + 0b100); // 00XXX100
                else if(m == "DCR") bytes.push_back((code << 3) + 0b101); //00XX101
                break; 
            }

            case 3: { // NOP
                bytes.push_back(0);
                break; 
            }

            case 4: { // MOV DST, SRC
                std::string dest = line.substr(line.find(" ") + 1, 1);
                std::string src = line.substr(line.find(",") + 2, 1);
                int destcode = regmap[dest];
                int srccode = regmap[src];
                std::cout << destcode << " " << srccode << "\n";
                if(destcode == 0b110 && destcode == srccode) {
                    printError(linenum, "Destination and Source register cannot both be memory.");
                    continue;
                }
                bytes.push_back((0b01 << 6) + (destcode << 3) + (srccode));
                break;
            }

            case 5: { // LDAX/STAX
                std::string reg = line.substr(line.find(" ") + 1, 1);
                if(reg != "B" && reg != "D") {
                    printError(linenum, "Register for LDAX/STAX should be 'B' or 'D'.");
                    continue;
                }
                int inscode = 0; // default LDAX
                if(m == "STAX") inscode = 1;

                int regcode = 0; // default B
                if(reg == "D") regcode = 1;

                bytes.push_back((inscode << 4) + (regcode << 3) + 0b010);
                break;
            }
        }

        for(char i : bytes) {
            fs.write(reinterpret_cast<const char*>(&i), sizeof(i));
        }
    }
}

int main() {
    std::map<std::string, std::pair<int, int>> map = map_str();
    std::ifstream file("test.asm");
    
    assemble(map, &file);
}