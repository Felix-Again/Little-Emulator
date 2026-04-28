#include <iostream>
#include <fstream>
#include <filesystem>
#include <stdexcept>
#include <cstdint>
#include <bitset>
#include <map>
#include <vector>
#include <functional>
#include <string>
#include <sstream>

// This is a very important part, we need to setup the registers.
// A, B, C, D, E, H and L are 8 bits long. 
// However, they (except A), can be treated as BC, DE and HL, which are each 16 bits long. 
// SP and PC are also 16 bits long.
// F is also 8 bits long.

// F only uses the 4 left-most bits. A full F register looks like this: 11110000.
// The 7th Bit (10000000) means "Zero"
// The 6th Bit (01000000) means "Subtraction"
// The 5th Bit (00100000) means "Half-Carry"
// The 4th Bit (00010000) means "Carry"
// Obviously, we started counting at the 0th index.

uint8_t A = 0, B = 0, C = 10, D = 0, E = 0, H = 0, L = 0, F = 0;
uint16_t SP = 0, PC = 0;

std::map<std::string, uint8_t*> regAlias8bit = {
    {"A", &A}, 
    {"B", &B}, 
    {"C", &C}, 
    {"D", &D}, 
    {"E", &E}, 
    {"H", &H}, 
    {"L", &L}, 
    {"F", &F}
};

std::map<std::string, uint16_t*> regAlias16bit = {
    {"SP", &SP},
    {"PC", &PC}
};

// This following functions will be useful when treating pairs of 8 bits registers.
void mixRegisters(uint8_t& reg1, uint8_t& reg2, uint16_t val){
    // reg1 = 00000000
    // reg2 = 00000000
    // val = 1011011101101001

    // Then, we need:
    // reg1 = 10110111
    // reg2 = 01101001

    // That means, using bitwise operations:
    reg1 = (val & 0xFF00)>>8; // Set the right-most 8 bits to 0 and displaces every other 8 bits to the right.
    reg2 = (val & 0x00FF); // Set the left-most 8 bits to 0. We don't need to displace anything.
}

uint16_t getMixedRegisters(uint8_t reg1, uint8_t reg2){

    // This basically does the inverse of that mixRegisters() function.
    
    return ((reg1 << 8) | reg2);

}

// The following overflowAdd functions will let us add small values and know if they overflow or not.
std::pair<uint16_t, bool> overflowAdd(uint16_t val0, uint16_t val1){

    bool overflow = false;
    if (val0+val1 < val0 || val0+val1 < val1){
        overflow = true;
    }

    return std::make_pair(val0+val1, overflow);
}

std::pair<uint8_t, bool> overflowAdd(uint8_t val0, uint8_t val1){
    
    bool overflow = false;
    if (val0+val1 < val0 || val0+val1 < val1){
        overflow = true;
    }
    
    return std::make_pair(val0+val1, overflow);
    
}

void changeFlag(bool null, bool subtraction, bool carry, bool halfcarry){

    if (null){
        F |= 0b10000000; // Sets the ZERO bit to 1.
    }
    else{
        F &= ~0b10000000;
    }
    if (subtraction){
        F |= 0b01000000;; // Sets the SUBTRACTION bit to 1.
    }
    else{
        F &= ~0b01000000;
    }
    if (halfcarry){
        F |= 0b00100000; // Sets the HALF-CARRY bit to 1.
    } 
    else {
        F &= ~0b00100000;
    }
    if (carry){
        F |= 0b00010000; // Sets the CARRY bit to 1.
    }
    else{
        F &= ~0B000100000;
    }

}

void ADD(std::vector<std::string> parameters){

    // The following code may be a bit confusing, but it works.

    // First, we verify the size of the parameters, to verify if we need to use uint16_t or uint8_t values.
    
    if (parameters.size() == 1){ 
        
        // If there is only 1 parameter, we infer that the sum is to the A register.
        // In this case, we can only use 8 bit registers, since A is, itself, a 8 bit register and can't handle 16 bits.
        
        try {
            // We change the value located on the adress of the first parameter to sum to value of the second parameter. Easy.
            auto [result, overflow] = overflowAdd(A, *regAlias8bit[parameters[0]]);
            changeFlag(result == 0, false, overflow, (A & 0xF + result & 0xF) > 0xF);
            A = result;

        }
        catch (const std::exception& e){ // Error handling
            std::cerr << "Error: " << e.what() << std::endl;
        }

    } 
    else {
        if (parameters[0].size() == 2 && parameters[1].size() == 2 ){

                // Safely cast the parameters characters to 1-char-long strings.
                // I found out that using std::to_string(parameter[0][0]) would turn the int value of the char to a string, not the char itself.

                std::string reg1(1, parameters[0][0]);
                std::string reg2(1, parameters[0][1]);
                std::string reg3(1, parameters[1][0]);
                std::string reg4(1, parameters[1][1]);

                // We verify if every register on the parameters is a valid register listed on the regAlias8bit map.
                if (regAlias8bit.count(reg1) && regAlias8bit.count(reg2) && regAlias8bit.count(reg3) && regAlias8bit.count(reg4)){
                    try {
                        // We get the respective uint16_t values for every pair of registers.
                        uint16_t value0 = getMixedRegisters(*regAlias8bit[reg1], *regAlias8bit[reg2]);
                        uint16_t value1 = getMixedRegisters(*regAlias8bit[reg3], *regAlias8bit[reg4]);

                        // Then, we set the first pair of 8bit registers to be the sum of their original value and the value of the mixed 16bit register of the second 8bit pair.
                        auto [result, overflow] = overflowAdd(value0, value1);
                        
                        // We then need to treat the FLAG register accordingly.
                        changeFlag(result == 0, false, overflow, (value0 & 0xF + value1 & 0xF) > 0xF);
                        
                        mixRegisters(*regAlias8bit[reg1], *regAlias8bit[reg2], result);
                    }
                    catch (const std::exception& e){ // Error handling
                        std::cerr << "Error: " << e.what() << std::endl;
                    }
                } 
                else { // If it is not a valid parameter, something is really wrong.
                    std::cerr << "Error: one of the registers in the parameters don't exist." << std::endl;
                }
                
            }
            else { // If we are dealing with only 8bit registers, things get easier.
                try {
                    // We change the value located on the adress of the first parameter to sum to value of the second parameter. Easy.
                    uint8_t value0 = *regAlias8bit[parameters[0]];
                    uint8_t value1 = *regAlias8bit[parameters[1]];
                    
                    auto [result, overflow] = overflowAdd(value0, value1);
                    changeFlag(result == 0, false, overflow, (value0 & 0xF + value1 & 0xF) > 0xF);
                    *regAlias8bit[parameters[0]] = result;
                }
                catch (const std::exception& e){ // Error handling
                    std::cerr << "Error: " << e.what() << std::endl;
                }
            }
    }
    
    #ifdef DEBUG

    if (parameters.size() == 1){ 
        std::cout << "Added the " << parameters[0] << " register to A register >> A = " << std::bitset<8>(A) << " && " << parameters[0] << " = " << std::bitset<8>(*regAlias8bit[parameters[0]]) << std::endl;
    }
    else {
        if (parameters[0].size() == 2 && parameters[1].size() == 2){
            std::string reg1(1, parameters[0][0]);
            std::string reg2(1, parameters[0][1]);
            std::string reg3(1, parameters[1][0]);
            std::string reg4(1, parameters[1][1]);

            std::cout << "Added the " << parameters[1][0] << parameters[1][1] << " register to " << parameters[0][0] << parameters[0][1] << " register. >> " << parameters[0][0] << parameters[0][1] << " = " << std::bitset<16>(getMixedRegisters(*regAlias8bit[reg1], *regAlias8bit[reg2])) << " && " << parameters[1][0] << parameters[1][1] << " = " << std::bitset<16>(getMixedRegisters(*regAlias8bit[reg3], *regAlias8bit[reg4]))<< std::endl;
        }
        else{
            std::cout << "Added the " << parameters[1] << " register to " << parameters[0] << " register. >> " << parameters[0] << " = " << std::bitset<8>(*regAlias8bit[parameters[0]]) << " && " << parameters[1] << " = " << std::bitset<8>(*regAlias8bit[parameters[1]]) << std::endl;
        }
    }

    #endif

}

void ADC(std::vector<std::string> parameters){

}

void SUB(std::vector<std::string> parameters){

}

void SBC(std::vector<std::string> parameters){

}

void AND(std::vector<std::string> parameters){

}

void OR(std::vector<std::string> parameters){

}
void XOR(std::vector<std::string> parameters){

}
void CP(std::vector<std::string> parameters){

}

void INC(std::vector<std::string> parameters){

}

void DEC(std::vector<std::string> parameters){

}

void CCF(std::vector<std::string> parameters){

}

void SFC(std::vector<std::string> parameters){

}

void RRA(std::vector<std::string> parameters){

}

void RLA(std::vector<std::string> parameters){

}

void RRCA(std::vector<std::string> parameters){

}

void CPL(std::vector<std::string> parameters){

}

void BIT(std::vector<std::string> parameters){

}
void RESET(std::vector<std::string> parameters){

}

void SET(std::vector<std::string> parameters){

}

void SRL(std::vector<std::string> parameters){

}

void RR(std::vector<std::string> parameters){

}

void RL(std::vector<std::string> parameters){

}

void RRC(std::vector<std::string> parameters){

}

void RLC(std::vector<std::string> parameters){

}

void SRA(std::vector<std::string> parameters){

}

void SLA(std::vector<std::string> parameters){

}

void SWAP(std::vector<std::string> parameters){

}

// The following map associates every ASM instruction with a function that c++ can run.

std::map<std::string, std::function<void(std::vector<std::string>)>> asmInstructions = {
    {"ADD", ADD},
    {},

};

void executeASMLine(std::string line){

    if (line.empty()){
        return;
    }

    std::vector<std::string> parameters;
    std::stringstream ss(line);

    std::string word;

    ss >> word;
    std::function func = asmInstructions[word];

    ss >> word;
    if (line.find(',') != std::string::npos){
        word.pop_back();
        parameters.push_back(word);
        ss >> word;
        parameters.push_back(word);
    }
    else{
        parameters.push_back(word);
    }

    #ifdef DEBUG

    std::cout << "Passed parameters by the ASM line: " << line << std::endl;

    #endif
    
    func(parameters);

    #ifdef DEBUG

    std::cout << "SP register: " << std::bitset<8>(SP) << std::endl;
    std::cout << "PC register: " << std::bitset<8>(PC) << std::endl;
    std::cout << "F register: " << std::bitset<8>(F) << std::endl;
    std::cout << std::endl;

    #endif

}

int main(){
    
    // First, we need to verify if there is a valid assembly file.
    // For that, we will use the following code:

    std::filesystem::path assemblyPath = "";

    // This for loop will iterate through every file on the program directory.
    // If it finds a .asm file, it will save it as the assemblyPath and break the loop.
    for (const auto& entry : std::filesystem::directory_iterator("./")){
        if (entry.path().extension() == ".asm"){
            assemblyPath = entry.path().string();
            break;
        }
    }

    // If the assemblyPath is still undefined, we throw an error to warn the user.
    if (assemblyPath == ""){
        std::cerr << "Error: could not find a valid assembly file on the program directory." << std::endl;
        return 1;
    }

    // -------------------------------------------- 


    // We try to initialize a ifstream on the assembly file.
    
    std::ifstream assemblyFile(assemblyPath);
    
    // --------------------------------------------

    // With everything ready, we now need to "interpret" the ASM file, the same way that python is interpreted.
    // But how so? ASM is low-level code...
    // Well, I already tried creating my own programing language, which was interpreted by C++,
    // So, this can't be that difficult... right?
    
    // We need to create an interpreter that:
    // Understands every ASM entry and function;
    // Correctly interacts with the registers.
    // This will be painfull...

    std::string line;

    while (std::getline(assemblyFile, line)){
        executeASMLine(line);
    }
    
}