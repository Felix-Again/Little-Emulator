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
#include <optional>

class CPU{

    public:

        class MemoryBus{

            public:

                uint8_t readByte(uint16_t address){
                    return this->memory[address];
                }

                uint16_t readWord(uint16_t address){
                    uint8_t lsByte = this->readByte(address);

                    uint16_t next = address + 1;
                    uint8_t msByte = this->readByte(next);

                    return (static_cast<uint16_t>(msByte)) << 8 | static_cast<uint16_t>(lsByte);
                }

                void writeByte(uint16_t address, uint8_t value){
                    this->memory[address] = value;
                }

                void writeWord(uint16_t address, uint16_t value){
                    this->writeByte(address, value & 0xFF);
                    this->writeByte(address+1, (value >> 8) & 0xFF);
                }

            private:

                // Contents from 0x100 to 0x3FFF include the cartridge memory
                uint8_t memory[0xFFFF];
        };

        void executeASM(std::string line){

            if (line.find_last_not_of("\n\t\r") == std::string::npos){
                return;
            }

            std::vector<std::string> parameters;
            std::stringstream ss(line);

            std::string word;

            ss >> word;
            std::function func = ASMInstructions[word];

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

            std::cout << "SP register: " << std::bitset<16>(this->regs.SP) << std::endl;
            std::cout << "PC register: " << std::bitset<16>(this->regs.PC) << std::endl;
            std::cout << "F register: " << std::bitset<8>(this->regs.F) << std::endl;
            std::cout << std::endl;

            #endif
        }

    private:

        MemoryBus memory;
        bool isHalted;

        struct Registers {

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

            uint8_t A = 0;
            uint8_t B = 0;
            uint8_t C = 0;
            uint8_t D = 0;
            uint8_t E = 0;
            uint8_t H = 0;
            uint8_t L = 0;

            uint8_t F = 0;

            uint16_t SP = 0;
            uint16_t PC = 0;
        } regs;

        enum class RegisterPairs : uint16_t { AF, BC, DE, HL };

        enum class InstructionType { 
            ADD,
            JP,
            LD,
            PUSH,
            POP,
            CALL,
            RET,
            NOP,
            HALT
        };
        
        enum class Target {A, B, C, D, E, H, L, F, AF, BC, DE, HL, SP, PC};

        enum class JumpTest {NotZero, NotCarry, Zero, Carry, Always};

        enum class LoadTarget { A, B, C, D, E, h, L, HLI};

        enum class LoadSource { A, B, C, D, E, H, D8, HLI};

        enum class StackTarget { BC, DE, HL };

        struct LoadByte {
            LoadTarget loadTarget;
            LoadSource loadSource;
        };

        struct Instruction{
            InstructionType type;
            Target target;
            JumpTest jumpTest;
            LoadByte loadByte;
            StackTarget stackTarget;

            // The following is basically a refactoring of the getInstructionFromByte() function.
            // It is aimed to make this code more comprehensible and more consistent.
            // Really, creating a function that reads a string of parameters instead of the parameters
            // Is a really, really, dumb idea. I know. That's why i will refactor the Add() function AGAIN. (I can't handle this anymore...)
            // I aim to do the following:
            // Have the executeASM() function define the targets parameters and the functions only do the "reasoning".
            // So, executeASM() will handle the strings and the instructions will only be happy.
            static std::optional<Instruction> getFromByte(uint8_t byte, bool prefixed){
                
                if (prefixed){
                    switch (byte)
                    {
                    case 0x00:
                        /* code */
                        break;
                    
                    default:
                        break;
                    }
                } else {
                    switch (byte)
                    {
                    case 0x00: break;
                    case 0x01: break;
                    case 0x02: break;
                    case 0x03: break;
                    case 0x04: break;
                    case 0x05: break;
                    case 0x06: break;
                    case 0x07: break;
                    case 0x08: break;
                    case 0x09: return Instruction{InstructionType::ADD, Target::BC};
                    case 0x0A: break;
                    case 0x0B: break;
                    case 0x0C: break;
                    case 0x0D: break;
                    case 0x0E: break;
                    case 0x0F: break;

                    case 0x10: break;
                    case 0x11: break;
                    case 0x12: break;
                    case 0x13: break;
                    case 0x14: break;
                    case 0x15: break;
                    case 0x16: break;
                    case 0x17: break;
                    case 0x18: break;
                    case 0x19: return Instruction{InstructionType::ADD, Target::DE};
                    case 0x1A: break;
                    case 0x1B: break;
                    case 0x1C: break;
                    case 0x1D: break;
                    case 0x1E: break;
                    case 0x1F: break;

                    case 0x20: break;
                    case 0x21: break;
                    case 0x22: break;
                    case 0x23: break;
                    case 0x24: break;
                    case 0x25: break;
                    case 0x26: break;
                    case 0x27: break;
                    case 0x28: break;
                    case 0x29: return Instruction{InstructionType::ADD, Target::HL};
                    case 0x2A: break;
                    case 0x2B: break;
                    case 0x2C: break;
                    case 0x2D: break;
                    case 0x2E: break;
                    case 0x2F: break;

                    case 0x30: break;
                    case 0x31: break;
                    case 0x32: break;
                    case 0x33: break;
                    case 0x34: break;
                    case 0x35: break;
                    case 0x36: break;
                    case 0x37: break;
                    case 0x38: break;
                    case 0x39: return Instruction{InstructionType::ADD, Target::SP};;
                    case 0x3A: break;
                    case 0x3B: break;
                    case 0x3C: break;
                    case 0x3D: break;
                    case 0x3E: break;
                    case 0x3F: break;

                    case 0x40: break;
                    case 0x41: break;
                    case 0x42: break;
                    case 0x43: break;
                    case 0x44: break;
                    case 0x45: break;
                    case 0x46: break;
                    case 0x47: break;
                    case 0x48: break;
                    case 0x49: break;
                    case 0x4A: break;
                    case 0x4B: break;
                    case 0x4C: break;
                    case 0x4D: break;
                    case 0x4E: break;
                    case 0x4F: break;

                    case 0x50: break;
                    case 0x51: break;
                    case 0x52: break;
                    case 0x53: break;
                    case 0x54: break;
                    case 0x55: break;
                    case 0x56: break;
                    case 0x57: break;
                    case 0x58: break;
                    case 0x59: break;
                    case 0x5A: break;
                    case 0x5B: break;
                    case 0x5C: break;
                    case 0x5D: break;
                    case 0x5E: break;
                    case 0x5F: break;

                    case 0x60: break;
                    case 0x61: break;
                    case 0x62: break;
                    case 0x63: break;
                    case 0x64: break;
                    case 0x65: break;
                    case 0x66: break;
                    case 0x67: break;
                    case 0x68: break;
                    case 0x69: break;
                    case 0x6A: break;
                    case 0x6B: break;
                    case 0x6C: break;
                    case 0x6D: break;
                    case 0x6E: break;
                    case 0x6F: break;

                    case 0x70: break;
                    case 0x71: break;
                    case 0x72: break;
                    case 0x73: break;
                    case 0x74: break;
                    case 0x75: break;
                    case 0x76: break;
                    case 0x77: break;
                    case 0x78: break;
                    case 0x79: break;
                    case 0x7A: break;
                    case 0x7B: break;
                    case 0x7C: break;
                    case 0x7D: break;
                    case 0x7E: break;
                    case 0x7F: break;

                    case 0x80: 
                        return Instruction{InstructionType::ADD, Target::B};
                    case 0x81: 
                        return Instruction{InstructionType::ADD, Target::C};
                    case 0x82:
                        return Instruction{InstructionType::ADD, Target::D};
                    case 0x83:
                        return Instruction{InstructionType::ADD, Target::E};
                    case 0x84:
                        return Instruction{InstructionType::ADD, Target::H};
                    case 0x85:
                        return Instruction{InstructionType::ADD, Target::L};
                    case 0x86:
                        return Instruction{InstructionType::ADD, Target::HL};
                    case 0x87:
                        return Instruction{InstructionType::ADD, Target::A};
                        break;
                    case 0x88: break;
                    case 0x89: break;
                    case 0x8A: break;
                    case 0x8B: break;
                    case 0x8C: break;
                    case 0x8D: break;
                    case 0x8E: break;
                    case 0x8F: break;

                    case 0x90: break;
                    case 0x91: break;
                    case 0x92: break;
                    case 0x93: break;
                    case 0x94: break;
                    case 0x95: break;
                    case 0x96: break;
                    case 0x97: break;
                    case 0x98: break;
                    case 0x99: break;
                    case 0x9A: break;
                    case 0x9B: break;
                    case 0x9C: break;
                    case 0x9D: break;
                    case 0x9E: break;
                    case 0x9F: break;

                    case 0xA0: break;
                    case 0xA1: break;
                    case 0xA2: break;
                    case 0xA3: break;
                    case 0xA4: break;
                    case 0xA5: break;
                    case 0xA6: break;
                    case 0xA7: break;
                    case 0xA8: break;
                    case 0xA9: break;
                    case 0xAA: break;
                    case 0xAB: break;
                    case 0xAC: break;
                    case 0xAD: break;
                    case 0xAE: break;
                    case 0xAF: break;

                    case 0xB0: break;
                    case 0xB1: break;
                    case 0xB2: break;
                    case 0xB3: break;
                    case 0xB4: break;
                    case 0xB5: break;
                    case 0xB6: break;
                    case 0xB7: break;
                    case 0xB8: break;
                    case 0xB9: break;
                    case 0xBA: break;
                    case 0xBB: break;
                    case 0xBC: break;
                    case 0xBD: break;
                    case 0xBE: break;
                    case 0xBF: break;

                    case 0xC0: break;
                    case 0xC1: break;
                    case 0xC2: break;
                    case 0xC3: break;
                    case 0xC4: break;
                    case 0xC5: break;
                    case 0xC6: break;
                    case 0xC7: break;
                    case 0xC8: break;
                    case 0xC9: break;
                    case 0xCA: break;
                    case 0xCB: break;
                    case 0xCC: break;
                    case 0xCD: break;
                    case 0xCE: break;
                    case 0xCF: break;

                    case 0xD0: break;
                    case 0xD1: break;
                    case 0xD2: break;
                    case 0xD3: break;
                    case 0xD4: break;
                    case 0xD5: break;
                    case 0xD6: break;
                    case 0xD7: break;
                    case 0xD8: break;
                    case 0xD9: break;
                    case 0xDA: break;
                    case 0xDB: break;
                    case 0xDC: break;
                    case 0xDD: break;
                    case 0xDE: break;
                    case 0xDF: break;

                    case 0xE0: break;
                    case 0xE1: break;
                    case 0xE2: break;
                    case 0xE3: break;
                    case 0xE4: break;
                    case 0xE5: break;
                    case 0xE6: break;
                    case 0xE7: break;
                    case 0xE8: break;
                    case 0xE9: break;
                    case 0xEA: break;
                    case 0xEB: break;
                    case 0xEC: break;
                    case 0xED: break;
                    case 0xEE: break;
                    case 0xEF: break;

                    case 0xF0: break;
                    case 0xF1: break;
                    case 0xF2: break;
                    case 0xF3: break;
                    case 0xF4: break;
                    case 0xF5: break;
                    case 0xF6: break;
                    case 0xF7: break;
                    case 0xF8: break;
                    case 0xF9: break;
                    case 0xFA: break;
                    case 0xFB: break;
                    case 0xFC: break;
                    case 0xFD: break;
                    case 0xFE: break;
                    case 0xFF: break;

                    default: break;
                    }
                }
            }
        };

        uint8_t* get8BitReg(const std::string& name){
            
            if (name == "A") return &this->regs.A;
            if (name == "B") return &this->regs.B;
            if (name == "C") return &this->regs.C;
            if (name == "D") return &this->regs.D;
            if (name == "E") return &this->regs.E;
            if (name == "F") return &this->regs.F;
            if (name == "H") return &this->regs.H;
            if (name == "L") return &this->regs.L;

            return nullptr;
        }

        uint8_t* get8BitReg(const Target& target){

            if (target == Target::A) return &this->regs.A;
            if (target == Target::B) return &this->regs.B;
            if (target == Target::C) return &this->regs.C;
            if (target == Target::D) return &this->regs.D;
            if (target == Target::E) return &this->regs.E;
            if (target == Target::F) return &this->regs.F;
            if (target == Target::H) return &this->regs.H;
            if (target == Target::L) return &this->regs.L;

            return nullptr;
        }

        uint16_t* get16BitReg(const std::string& name){

            if (name == "SP") return &this->regs.SP;
            if (name == "PC") return &this->regs.PC;

            return nullptr;

        }

        uint16_t* get16BitReg(const Target& target){

            if (target == Target::SP) return &this->regs.SP;
            if (target == Target::PC) return &this->regs.PC;

            return nullptr;
        }

        uint16_t extract16BitReg(const std::string& name){

            if (name == "SP") return this->regs.SP;
            if (name == "PC") return this->regs.PC;

            if (name == "AF") return (static_cast<uint16_t>(this->regs.A)<<8 | this->regs.F);
            if (name == "BC") return (static_cast<uint16_t>(this->regs.B)<<8 | this->regs.C);
            if (name == "DE") return (static_cast<uint16_t>(this->regs.D)<<8 | this->regs.E);
            if (name == "HL") return (static_cast<uint16_t>(this->regs.H)<<8 | this->regs.L);

            return 0;
        }

        uint16_t extract16BitReg(const Target& target){

            if (target == Target::SP) return this->regs.SP;
            if (target == Target::PC) return this->regs.PC;

            if (target == Target::AF) return getCombined(RegisterPairs::AF);
            if (target == Target::BC) return getCombined(RegisterPairs::BC);
            if (target == Target::DE) return getCombined(RegisterPairs::DE);
            if (target == Target::HL) return getCombined(RegisterPairs::HL);

            return 0;
        }

        std::pair<uint16_t, bool> overflowSum(uint16_t val1, uint16_t val2){

            return std::make_pair(val1 + val2,((val1 + val2) < val1 || (val1 + val2) < val2));

        }

        std::pair<uint8_t, bool> overflowSum(uint8_t val1, uint8_t val2){
            
            return std::make_pair(val1 + val2,((val1 + val2) < val1 || (val1 + val2) < val2));

        }

        void ADD(std::vector<std::string> parameters){

            try {
                // Verify if the parameter is a number. If it is, add it to the A register

                uint8_t value = static_cast<uint8_t>(std::stoi(parameters[0]));

                auto [result, overflow] = this->overflowSum(regs.A, value);

                this -> changeFlag(result == 0, 0, (regs.A & 0xF + value & 0xF) > 0xF, overflow);

                regs.A = result;
                
            } catch (const std::exception& e){
                if (parameters.size() == 1){

                    if (parameters[0].size() == 1){
                        try {
                            
                            // Get the register associated with the string of the parameter
                            uint8_t& reg1 = *this->get8BitReg(parameters[0]);

                            auto [result, overflow] = this->overflowSum(regs.A, reg1);

                            // Change the flag accordingly
                            this->changeFlag(result == 0, 0, (regs.A & 0xF + reg1 & 0xF )>0xF, overflow);
                            // Change the register
                            regs.A = result;

                        } catch (const std::exception& e){
                            std::cerr << "Error: "<< e.what() << std::endl;
                        }
                        
                        
                    }
                    else if (parameters[0].size() == 4){

                        // TODO: Add the ADD function that receives a memory adress.
                    
                    }

                }
                else {

                    if (parameters[0].size() == 1 && parameters[1].size() != 4){

                        try {
                            // Verify if the second parameter is a value. If it is, add it to the first parameter
                            uint8_t value = static_cast<uint8_t>(std::stoi(parameters[1]));

                            uint8_t& reg = *this->get8BitReg(parameters[0]);

                            auto [result , overflow] = this->overflowSum(reg, value);

                            this->changeFlag(result == 0, 0, (reg & 0xF + value & 0xF) > 0xF, overflow);

                            reg = result;
                        } catch (const std::exception& e){
                            try {
                                // If they are both registers, get each one and add the second to the first.
                                uint8_t& reg1 = *this->get8BitReg(parameters[0]);
                                uint8_t& reg2 = *this->get8BitReg(parameters[1]);

                                auto [result, overflow] = this->overflowSum(reg1, reg2);

                                this->changeFlag(result == 0, 0, (reg1 & 0xF + reg2 & 0xF) > 0xF, overflow);
                                reg1 = result;
                            } catch (const std::exception& e){
                                std::cerr << "Error: " << e.what() << std::endl;
                            }
                        }
                        

                    }
                    else if (parameters[0].size() == 2 && parameters[1].size() == 2){

                        try {
                            // If they are both 16bit registers, get each one
                            uint16_t* reg1 = get16BitReg(parameters[0]);

                            uint16_t val1 = extract16BitReg(parameters[0]);
                            uint16_t val2 = extract16BitReg(parameters[1]);

                            auto [result, overflow] = this->overflowSum(val1, val2);

                            changeFlag(result == 0, 0, (val1 & 0xFF + val2 & 0xFF) > 0xFF, overflow);

                            if (reg1 != nullptr){
                                *reg1 = result;
                            }
                            else {
                                if (parameters[0] == "AF") setCombined(RegisterPairs::AF, result);
                                if (parameters[0] == "BC") setCombined(RegisterPairs::BC, result);
                                if (parameters[0] == "DE") setCombined(RegisterPairs::DE, result);
                                if (parameters[0] == "HL") setCombined(RegisterPairs::HL, result);
                            }
                            
                        } catch (const std::exception& e){
                            std::cerr << "Error: " << e.what() << std::endl;
                        }

                    }
                    else if (parameters[1].size() == 4){
                        uint8_t& reg = *this->get8BitReg(parameters[0]);

                        std::string reg16 = parameters[1];
                        reg16.pop_back();
                        reg16.erase(0,1);

                        uint16_t address = extract16BitReg(reg16);

                        uint8_t value = this->memory.readByte(address);

                        auto [result, overflow] = overflowSum(reg, value);

                        changeFlag(result == 0, 0, (reg & 0xF + value & 0xF) > 0xF, overflow);

                        reg = result;

                    }
                }
            }
            

            #ifdef DEBUG

            std::cout << std::bitset<8>(this->regs.A) << std::endl;
            std::cout << std::bitset<8>(this->regs.B) << std::endl;
            std::cout << std::bitset<8>(this->regs.C) << std::endl;
            std::cout << std::bitset<8>(this->regs.D) << std::endl;
            std::cout << std::bitset<8>(this->regs.E) << std::endl;
            std::cout << std::bitset<8>(this->regs.F) << std::endl;
            std::cout << std::bitset<8>(this->regs.H) << std::endl;
            std::cout << std::bitset<8>(this->regs.L) << std::endl;
            

            #endif

        }

        // ADD instruction that receives two targets.
        void ADD(const Target& target1, const Target& target2){

            if (get8BitReg(target1) != nullptr){
                uint8_t* reg1 = get8BitReg(target1);
                uint8_t* reg2 = get8BitReg(target2);

                if (reg1 == nullptr || reg2 == nullptr) throw std::runtime_error("Nullptr returned while performing ADD instruction.");
            
                auto [result, overflow] = overflowSum(*reg1, *reg2);

                changeFlag(result == 0, 0, (*reg1 & 0xF + *reg2 & 0xF) > 0xF, overflow);

                *reg1 = result;
            }
            else if (get16BitReg(target1) != nullptr){
                uint16_t* reg1 = get16BitReg(target1);
                uint16_t value2 = extract16BitReg(target2);

                if (reg1 == nullptr) throw std::runtime_error("Nullptr returned while performing ADD instruction.");

                auto [result, overflow] = overflowSum(*reg1, value2);

                changeFlag(this->regs.F >> 7, 0, (*reg1 & 0xF + value2 & 0xF) > 0xF, overflow);

                *reg1 = result;
            }

        }

        // ADD instruction that only receives one target.
        void ADD(const Target& target){

            if (get8BitReg(target) != nullptr){
                uint8_t* reg = get8BitReg(target);

                if (reg == nullptr) throw std::runtime_error("Nullptr returned while performing ADD instruction.");
            
                auto [result, overflow] = overflowSum(this->regs.A, *reg);

                changeFlag(result == 0, 0, (this->regs.A & 0xF + *reg & 0xF) > 0xF, overflow);

                this->regs.A = result;
            }
            else if (get16BitReg(target) != nullptr){
                uint16_t val = extract16BitReg(target);

                auto [result, overflow] = overflowSum(this->getCombined(RegisterPairs::HL), val);

                changeFlag(this->regs.F >> 7, 0, (this->getCombined(RegisterPairs::HL) & 0xF + val & 0xF) > 0xF, overflow);

                this->setCombined(RegisterPairs::HL, result);
            }

        }
        
        // JP instruction that receives a boolean condition to decide if it should jump to the designed address of the following 2 bytes.
        uint16_t JP(bool jump){

            if (jump){
                uint16_t lsByte = this->memory.readByte(this->regs.PC+1);
                uint16_t msByte = this->memory.readByte(this->regs.PC+2);

                return (msByte << 8 ) | lsByte;
            }
            else return this->regs.PC+3;
        }

        // LD instruction that receives a LoadByte and do the logic with it.
        // TODO: Add more sources and targets.
        uint16_t LD(const LoadByte& loadByte){

            uint8_t sourceValue = 0;
            switch (loadByte.loadSource)
            {
            case LoadSource::A:
                sourceValue = this->regs.A;
                break;
            case LoadSource::D8:
                sourceValue = this->memory.readByte(this->regs.PC+1);
                break;
            case LoadSource::HLI:
                sourceValue = this->memory.readByte(getCombined(RegisterPairs::HL));
                break;
            default:
                throw std::runtime_error("Not implemented source on load.");
            }

            switch (loadByte.loadTarget)
            {
            case LoadTarget::A:
                this->regs.A = sourceValue;
                break;
            case LoadTarget::HLI:
                this->memory.writeByte(this->getCombined(RegisterPairs::HL), sourceValue);
                break;
            default:
                throw std::runtime_error("Not implemented target on load.");
            }

            if (loadByte.loadSource == LoadSource::D8) return this->regs.PC+2;
            else return this->regs.PC+1;
        }
        
        void PUSH(uint16_t value){

            this->regs.SP -= 1;
            this->memory.writeByte(this->regs.SP, (value & 0xFF00)>>8);

            this->regs.SP -= 1;
            this->memory.writeByte(this->regs.SP, (value & 0xFF));

        }

        uint16_t POP(){
        
            uint16_t lsByte = this->memory.readByte(this->regs.SP);
            this->regs.SP += 1;
            
            uint16_t msByte = this->memory.readByte(this->regs.SP);
            this->regs.SP += 1;

            return (msByte << 8) | lsByte;
            
        }

        uint16_t CALL(bool jump){
            uint16_t nextPC = this->regs.PC + 3;
            if (jump){
                this->PUSH(nextPC);
                return this->memory.readWord(this->regs.PC+1);
            } else return nextPC;
        }

        uint16_t RET(bool jump){
            if (jump) return this->POP();
            else return this->regs.PC+1;
        }

        std::map<std::string, std::function<void(std::vector<std::string>)>> ASMInstructions = {
            {"ADD", [this](std::vector<std::string> parameters) { this->ADD(parameters); }}
        };

        uint16_t getCombined(RegisterPairs pair){
            switch (pair)
            {
            case RegisterPairs::AF:
                return (static_cast<uint16_t>(regs.A) << 8) | regs.F; 
                break;
            case RegisterPairs::BC:
                return (static_cast<uint16_t>(regs.B) << 8) | regs.C; 
                break;
            case RegisterPairs::DE:
                return (static_cast<uint16_t>(regs.D) << 8) | regs.E; 
                break;
            case RegisterPairs::HL:
                return (static_cast<uint16_t>(regs.H) << 8) | regs.L; 
                break;
            default:
                return 0;
                break;
            }
        }

        void setCombined(RegisterPairs pair, uint16_t value){
            uint8_t high = (value >> 8) & 0xFF; // Extracts the upper 8 bits
            uint8_t low = value & 0xFF; // Extracts the lower 8 bits

            switch (pair)
            {
            case RegisterPairs::AF:
                regs.A = high;
                regs.F = low & ~0b00001111;
                break;
            case RegisterPairs::BC:
                regs.B = high;
                regs.C = low;
                break;
            case RegisterPairs::DE:
                regs.D = high;
                regs.E = low;
                break;
            case RegisterPairs::HL:
                regs.H = high;
                regs.L = low;
                break;
            default:
                break;
            }
        }

        void changeFlag(bool zero, bool subtract, bool halfCarry, bool carry){

            zero? regs.F |= 0b10000000 : regs.F &= ~0b10000000;
            subtract? regs.F |= 0b01000000 : regs.F &= ~0b01000000;
            halfCarry? regs.F |= 0b00100000 : regs.F &= ~0b00100000;
            carry? regs.F |= 0b00010000 : regs.F &= ~0b00010000;

        }

        uint16_t getInstructionFromByte(uint16_t instructionByte){
            std::vector<std::string> parameters;
            switch (instructionByte)
            {
                case 0x00: break;
                case 0x01: break;
                case 0x02: break;
                case 0x03: break;
                case 0x04: break;
                case 0x05: break;
                case 0x06: break;
                case 0x07: break;
                case 0x08: break;
                case 0x09: break;
                case 0x0A: break;
                case 0x0B: break;
                case 0x0C: break;
                case 0x0D: break;
                case 0x0E: break;
                case 0x0F: break;

                case 0x10: break;
                case 0x11: break;
                case 0x12: break;
                case 0x13: break;
                case 0x14: break;
                case 0x15: break;
                case 0x16: break;
                case 0x17: break;
                case 0x18: break;
                case 0x19: break;
                case 0x1A: break;
                case 0x1B: break;
                case 0x1C: break;
                case 0x1D: break;
                case 0x1E: break;
                case 0x1F: break;

                case 0x20: break;
                case 0x21: break;
                case 0x22: break;
                case 0x23: break;
                case 0x24: break;
                case 0x25: break;
                case 0x26: break;
                case 0x27: break;
                case 0x28: break;
                case 0x29: break;
                case 0x2A: break;
                case 0x2B: break;
                case 0x2C: break;
                case 0x2D: break;
                case 0x2E: break;
                case 0x2F: break;

                case 0x30: break;
                case 0x31: break;
                case 0x32: break;
                case 0x33: break;
                case 0x34: break;
                case 0x35: break;
                case 0x36: break;
                case 0x37: break;
                case 0x38: break;
                case 0x39: break;
                case 0x3A: break;
                case 0x3B: break;
                case 0x3C: break;
                case 0x3D: break;
                case 0x3E: break;
                case 0x3F: break;

                case 0x40: break;
                case 0x41: break;
                case 0x42: break;
                case 0x43: break;
                case 0x44: break;
                case 0x45: break;
                case 0x46: break;
                case 0x47: break;
                case 0x48: break;
                case 0x49: break;
                case 0x4A: break;
                case 0x4B: break;
                case 0x4C: break;
                case 0x4D: break;
                case 0x4E: break;
                case 0x4F: break;

                case 0x50: break;
                case 0x51: break;
                case 0x52: break;
                case 0x53: break;
                case 0x54: break;
                case 0x55: break;
                case 0x56: break;
                case 0x57: break;
                case 0x58: break;
                case 0x59: break;
                case 0x5A: break;
                case 0x5B: break;
                case 0x5C: break;
                case 0x5D: break;
                case 0x5E: break;
                case 0x5F: break;

                case 0x60: break;
                case 0x61: break;
                case 0x62: break;
                case 0x63: break;
                case 0x64: break;
                case 0x65: break;
                case 0x66: break;
                case 0x67: break;
                case 0x68: break;
                case 0x69: break;
                case 0x6A: break;
                case 0x6B: break;
                case 0x6C: break;
                case 0x6D: break;
                case 0x6E: break;
                case 0x6F: break;

                case 0x70: break;
                case 0x71: break;
                case 0x72: break;
                case 0x73: break;
                case 0x74: break;
                case 0x75: break;
                case 0x76: break;
                case 0x77: break;
                case 0x78: break;
                case 0x79: break;
                case 0x7A: break;
                case 0x7B: break;
                case 0x7C: break;
                case 0x7D: break;
                case 0x7E: break;
                case 0x7F: break;

                case 0x80: 
                    parameters = {"A","B"};
                    ADD(parameters);
                    break;
                case 0x81: 
                    parameters = {"A","C"};
                    ADD(parameters);
                    break;
                case 0x82:
                    parameters = {"A","D"};
                    ADD(parameters);
                    break;
                case 0x83:
                    parameters = {"A","E"};
                    ADD(parameters);
                    break;
                case 0x84:
                    parameters = {"A","H"};
                    ADD(parameters);
                    break;
                case 0x85:
                    parameters = {"A","L"};
                    ADD(parameters);
                    break;
                case 0x86:
                    parameters = {"A", "[HL]"};
                    break;
                case 0x87:
                    parameters = {"A","A"};
                    ADD(parameters);
                    break;
                case 0x88: break;
                case 0x89: break;
                case 0x8A: break;
                case 0x8B: break;
                case 0x8C: break;
                case 0x8D: break;
                case 0x8E: break;
                case 0x8F: break;

                case 0x90: break;
                case 0x91: break;
                case 0x92: break;
                case 0x93: break;
                case 0x94: break;
                case 0x95: break;
                case 0x96: break;
                case 0x97: break;
                case 0x98: break;
                case 0x99: break;
                case 0x9A: break;
                case 0x9B: break;
                case 0x9C: break;
                case 0x9D: break;
                case 0x9E: break;
                case 0x9F: break;

                case 0xA0: break;
                case 0xA1: break;
                case 0xA2: break;
                case 0xA3: break;
                case 0xA4: break;
                case 0xA5: break;
                case 0xA6: break;
                case 0xA7: break;
                case 0xA8: break;
                case 0xA9: break;
                case 0xAA: break;
                case 0xAB: break;
                case 0xAC: break;
                case 0xAD: break;
                case 0xAE: break;
                case 0xAF: break;

                case 0xB0: break;
                case 0xB1: break;
                case 0xB2: break;
                case 0xB3: break;
                case 0xB4: break;
                case 0xB5: break;
                case 0xB6: break;
                case 0xB7: break;
                case 0xB8: break;
                case 0xB9: break;
                case 0xBA: break;
                case 0xBB: break;
                case 0xBC: break;
                case 0xBD: break;
                case 0xBE: break;
                case 0xBF: break;

                case 0xC0: break;
                case 0xC1: break;
                case 0xC2: break;
                case 0xC3: break;
                case 0xC4: break;
                case 0xC5: break;
                case 0xC6: break;
                case 0xC7: break;
                case 0xC8: break;
                case 0xC9: break;
                case 0xCA: break;
                case 0xCB: break;
                case 0xCC: break;
                case 0xCD: break;
                case 0xCE: break;
                case 0xCF: break;

                case 0xD0: break;
                case 0xD1: break;
                case 0xD2: break;
                case 0xD3: break;
                case 0xD4: break;
                case 0xD5: break;
                case 0xD6: break;
                case 0xD7: break;
                case 0xD8: break;
                case 0xD9: break;
                case 0xDA: break;
                case 0xDB: break;
                case 0xDC: break;
                case 0xDD: break;
                case 0xDE: break;
                case 0xDF: break;

                case 0xE0: break;
                case 0xE1: break;
                case 0xE2: break;
                case 0xE3: break;
                case 0xE4: break;
                case 0xE5: break;
                case 0xE6: break;
                case 0xE7: break;
                case 0xE8: break;
                case 0xE9: break;
                case 0xEA: break;
                case 0xEB: break;
                case 0xEC: break;
                case 0xED: break;
                case 0xEE: break;
                case 0xEF: break;

                case 0xF0: break;
                case 0xF1: break;
                case 0xF2: break;
                case 0xF3: break;
                case 0xF4: break;
                case 0xF5: break;
                case 0xF6: break;
                case 0xF7: break;
                case 0xF8: break;
                case 0xF9: break;
                case 0xFA: break;
                case 0xFB: break;
                case 0xFC: break;
                case 0xFD: break;
                case 0xFE: break;
                case 0xFF: break;

                default: break;
            
            }

            return this->regs.PC+1;
        }

        uint16_t execute(const Instruction& instruction){

            if (this->isHalted) return this->regs.PC;

            bool jumpCondition = false;
            uint16_t value = 0;
            uint16_t result = 0;

            switch (instruction.type)
            {

            case InstructionType::ADD:
                ADD(instruction.target);
                return this->regs.PC+1;

            case InstructionType::JP:
                switch (instruction.jumpTest)
                {
                case JumpTest::NotZero:
                    jumpCondition = !(this->regs.F>>7 & 1);
                    break;
                case JumpTest::Zero:
                    jumpCondition = this->regs.F>>7 & 1;
                    break;
                case JumpTest::NotCarry:
                    jumpCondition = !(this->regs.F>>4 & 1);
                    break;
                case JumpTest::Carry:
                    jumpCondition = this->regs.F>>4 & 1;
                    break;
                case JumpTest::Always:
                    jumpCondition = true;
                    break;
                }
                return JP(jumpCondition);

            case InstructionType::LD:
                return LD(instruction.loadByte);

            case InstructionType::PUSH:

                switch (instruction.stackTarget)
                {
                case StackTarget::BC:
                    value = getCombined(RegisterPairs::BC);
                    break;
                case StackTarget::DE:
                    value = getCombined(RegisterPairs::DE);
                    break;
                case StackTarget::HL:
                    value = getCombined(RegisterPairs::HL);
                    break;
                default:
                    throw std::runtime_error("Unsupported target at PUSH instruction.");
                }

                PUSH(value);  

                return this->regs.PC+1;
            
            case InstructionType::POP:
                
                result = this->POP();
                switch (instruction.stackTarget)
                {
                case StackTarget::BC:
                    this->setCombined(RegisterPairs::BC, result);
                    break;
                
                case StackTarget::DE:
                    this->setCombined(RegisterPairs::DE, result);
                    break;
                    
                case StackTarget::HL:
                    this->setCombined(RegisterPairs::HL, result);
                    break;
                default:
                    throw std::runtime_error("Unsupported stack target at POP.");
                }

                return this->regs.PC+1;

            case InstructionType::CALL:
                switch (instruction.jumpTest)
                {
                case JumpTest::NotZero:
                    jumpCondition = !(this->regs.F>>7 & 1);
                    break;
                case JumpTest::Zero:
                    jumpCondition = this->regs.F>>7 & 1;
                    break;
                case JumpTest::NotCarry:
                    jumpCondition = !(this->regs.F>>4 & 1);
                    break;
                case JumpTest::Carry:
                    jumpCondition = this->regs.F>>4 & 1;
                    break;
                case JumpTest::Always:
                    jumpCondition = true;
                    break;
                }
                return this->CALL(jumpCondition);

            case InstructionType::RET:
                switch (instruction.jumpTest)
                {
                case JumpTest::NotZero:
                    jumpCondition = !(this->regs.F>>7 & 1);
                    break;
                case JumpTest::Zero:
                    jumpCondition = this->regs.F>>7 & 1;
                    break;
                case JumpTest::NotCarry:
                    jumpCondition = !(this->regs.F>>4 & 1);
                    break;
                case JumpTest::Carry:
                    jumpCondition = this->regs.F>>4 & 1;
                    break;
                case JumpTest::Always:
                    jumpCondition = true;
                    break;
                }
                return this->RET(jumpCondition);

            case InstructionType::NOP:
                return this->regs.PC+1;

            default:
                throw std::runtime_error("Not implemented instruction.");
                return this->regs.PC+1;
            }
        }

        void step(){

            uint16_t instructionByte = this->memory.readByte(this->regs.PC);
            bool prefixed = (instructionByte == 0xCB);

            if (prefixed){
                instructionByte = this->memory.readByte(this->regs.PC+1);
            }

            auto instructionOpt = Instruction::getFromByte(instructionByte, prefixed);

            if (instructionOpt.has_value()){
                this->regs.PC = execute(instructionOpt.value());
            }
            else {
                throw std::runtime_error("Unkown instruction: 0x" + std::to_string(instructionByte));
            }

            try {
                uint16_t nextPC = this->getInstructionFromByte(instructionByte);
                
            } catch (const std::exception& e){
                std::cerr << "Error: " << e.what() << std::endl;
            }

        }
};

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
    CPU test;

    while (std::getline(assemblyFile, line)){
        test.executeASM(line);
    }
    
}