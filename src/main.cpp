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
            /*
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

            #endif */
        }

    private:

        MemoryBus memory;
        bool isHalted;
        bool IME;
        bool haltBug;

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

            // These are made for the HALT instruction. 
            uint8_t IE;
            uint8_t IF;
        } regs;

        enum class Vectors {V0x00, V0x08, V0x10, V0x18, V0x20, V0x28, V0x30, V0x38};

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
            HALT,
            ADC,
            INC,
            DEC,
            RLCA,
            RRCA,
            STOP,
            RLA,
            JR, 
            RRA,
            DAA,
            CPL,
            SCF,
            CCF,
            SUB,
            AND,
            SBC,
            XOR,
            OR,
            CP,
            RST,
            RETI,
            DI,
            RLC,
            RRC,
            RL,
            RR,
            SLA,
            SRA,
            SWAP, 
            SRL,
            BIT,
            RES,
            SET,
            LDH,
            JPHL,
            LDHLSP,
            EI
        };
        
        enum class Target {A, B, C, D, E, H, L, F, AF, BC, DE, HL, SP, PC};

        enum class ArithmeticTarget {A, B, C, D, E, H, L, F, AF, BC, DE, HL, SP, PC, HLI};
        
        enum class ArithmeticSource {A, B, C, D, E, H, L, F, HLI, D8, AF, BC, DE, HL, SP, PC};

        enum class BitwiseSource {A, B, C, D, E, F, H, L, HLI, D8};

        enum class BitFlagSource {A, B, C, D, E, F, H, L, HLI};

        enum class JumpTest {NotZero, NotCarry, Zero, Carry, Always};

        enum class LoadTarget { A, B, C, D, E, H, L, D8, D16, BCI, DEI, HLI, BC, DE, HL, HLDEC, HLINC, SP, A16};

        enum class LoadSource { A, B, C, D, E, H, L, D8, BCI, DEI, HLI, D16, BC, DE, HL, HLDEC, HLINC, SP};

        enum class StackTarget { BC, DE, HL };

        enum class RotateTarget {A, B, C, D, E, H, L, HLI};

        struct Arithmetic{
            ArithmeticTarget arithmeticTarget;
            ArithmeticSource arithmeticSource;
        };

        struct LoadByte {
            LoadTarget loadTarget;
            LoadSource loadSource;
        };

        struct BitFlag {
            unsigned int u3: 3;
            BitFlagSource bitFlagSource;
        };

        struct Instruction{
            InstructionType type;
            JumpTest jumpTest;
            LoadByte loadByte;
            StackTarget stackTarget;
            Arithmetic arithmetic;
            BitwiseSource bitSource;
            BitFlag bitFlag;
            RegisterPairs pair;
            Vectors vector;
            RotateTarget rotateTarget;

            Instruction(InstructionType type, Arithmetic arithmetic) : type(type), arithmetic(arithmetic) {}
            Instruction(InstructionType type, JumpTest jumpTest) : type(type), jumpTest(jumpTest) {}
            Instruction(InstructionType type, StackTarget stackTarget) : type(type), stackTarget(stackTarget) {}
            Instruction(InstructionType type, LoadByte loadByte) : type(type), loadByte(loadByte) {}
            Instruction(InstructionType type, BitwiseSource bitSource) : type(type), bitSource(bitSource) {}
            Instruction(InstructionType type, BitFlag bitFlag) : type(type), bitFlag(bitFlag) {}
            Instruction(InstructionType type, RegisterPairs pair) : type(type), pair(pair) {}
            Instruction(InstructionType type, Vectors vector) : type(type), vector(vector) {}
            Instruction(InstructionType type, RotateTarget rotateTarget) : type(type), rotateTarget(rotateTarget) {}
            Instruction(InstructionType type): type(type) {}

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
                    case 0x00: return Instruction{InstructionType::RLC, RotateTarget::B};
                    case 0x01: return Instruction{InstructionType::RLC, RotateTarget::C};
                    case 0x02: return Instruction{InstructionType::RLC, RotateTarget::D};
                    case 0x03: return Instruction{InstructionType::RLC, RotateTarget::E};
                    case 0x04: return Instruction{InstructionType::RLC, RotateTarget::H};
                    case 0x05: return Instruction{InstructionType::RLC, RotateTarget::L};
                    case 0x06: return Instruction{InstructionType::RLC, RotateTarget::HLI};
                    case 0x07: return Instruction{InstructionType::RLC, RotateTarget::A};
                    case 0x08: return Instruction{InstructionType::RRC, RotateTarget::B};
                    case 0x09: return Instruction{InstructionType::RRC, RotateTarget::C};
                    case 0x0A: return Instruction{InstructionType::RRC, RotateTarget::D};
                    case 0x0B: return Instruction{InstructionType::RRC, RotateTarget::E};
                    case 0x0C: return Instruction{InstructionType::RRC, RotateTarget::H};
                    case 0x0D: return Instruction{InstructionType::RRC, RotateTarget::L};
                    case 0x0E: return Instruction{InstructionType::RRC, RotateTarget::HLI};
                    case 0x0F: return Instruction{InstructionType::RRC, RotateTarget::A};

                    case 0x10: return Instruction{InstructionType::RL, RotateTarget::B};
                    case 0x11: return Instruction{InstructionType::RL, RotateTarget::C};
                    case 0x12: return Instruction{InstructionType::RL, RotateTarget::D};
                    case 0x13: return Instruction{InstructionType::RL, RotateTarget::E};
                    case 0x14: return Instruction{InstructionType::RL, RotateTarget::H};
                    case 0x15: return Instruction{InstructionType::RL, RotateTarget::L};
                    case 0x16: return Instruction{InstructionType::RL, RotateTarget::HLI};
                    case 0x17: return Instruction{InstructionType::RL, RotateTarget::A};
                    case 0x18: return Instruction{InstructionType::RR, RotateTarget::B};
                    case 0x19: return Instruction{InstructionType::RR, RotateTarget::C};
                    case 0x1A: return Instruction{InstructionType::RR, RotateTarget::D};
                    case 0x1B: return Instruction{InstructionType::RR, RotateTarget::E};
                    case 0x1C: return Instruction{InstructionType::RR, RotateTarget::H};
                    case 0x1D: return Instruction{InstructionType::RR, RotateTarget::L};
                    case 0x1E: return Instruction{InstructionType::RR, RotateTarget::HLI};
                    case 0x1F: return Instruction{InstructionType::RR, RotateTarget::A};

                    case 0x20: return Instruction{InstructionType::SLA, RotateTarget::B};
                    case 0x21: return Instruction{InstructionType::SLA, RotateTarget::C};
                    case 0x22: return Instruction{InstructionType::SLA, RotateTarget::D};
                    case 0x23: return Instruction{InstructionType::SLA, RotateTarget::E};
                    case 0x24: return Instruction{InstructionType::SLA, RotateTarget::H};
                    case 0x25: return Instruction{InstructionType::SLA, RotateTarget::L};
                    case 0x26: return Instruction{InstructionType::SLA, RotateTarget::HLI};
                    case 0x27: return Instruction{InstructionType::SLA, RotateTarget::A};
                    case 0x28: return Instruction{InstructionType::SRA, RotateTarget::B};
                    case 0x29: return Instruction{InstructionType::SRA, RotateTarget::C};
                    case 0x2A: return Instruction{InstructionType::SRA, RotateTarget::D};
                    case 0x2B: return Instruction{InstructionType::SRA, RotateTarget::E};
                    case 0x2C: return Instruction{InstructionType::SRA, RotateTarget::H};
                    case 0x2D: return Instruction{InstructionType::SRA, RotateTarget::L};
                    case 0x2E: return Instruction{InstructionType::SRA, RotateTarget::HLI};
                    case 0x2F: return Instruction{InstructionType::SRA, RotateTarget::A};

                    case 0x30: return Instruction{InstructionType::SWAP, RotateTarget::B};
                    case 0x31: return Instruction{InstructionType::SWAP, RotateTarget::C};
                    case 0x32: return Instruction{InstructionType::SWAP, RotateTarget::D};
                    case 0x33: return Instruction{InstructionType::SWAP, RotateTarget::E};
                    case 0x34: return Instruction{InstructionType::SWAP, RotateTarget::H};
                    case 0x35: return Instruction{InstructionType::SWAP, RotateTarget::L};
                    case 0x36: return Instruction{InstructionType::SWAP, RotateTarget::HLI};
                    case 0x37: return Instruction{InstructionType::SWAP, RotateTarget::A};
                    case 0x38: return Instruction{InstructionType::SRL, RotateTarget::B};
                    case 0x39: return Instruction{InstructionType::SRL, RotateTarget::C};
                    case 0x3A: return Instruction{InstructionType::SRL, RotateTarget::D};
                    case 0x3B: return Instruction{InstructionType::SRL, RotateTarget::E};
                    case 0x3C: return Instruction{InstructionType::SRL, RotateTarget::H};
                    case 0x3D: return Instruction{InstructionType::SRL, RotateTarget::L};
                    case 0x3E: return Instruction{InstructionType::SRL, RotateTarget::HLI};
                    case 0x3F: return Instruction{InstructionType::SRL, RotateTarget::A};

                    case 0x40: return Instruction{InstructionType::BIT, BitFlag{0,BitFlagSource::B}};
                    case 0x41: return Instruction{InstructionType::BIT, BitFlag{0,BitFlagSource::C}};
                    case 0x42: return Instruction{InstructionType::BIT, BitFlag{0,BitFlagSource::D}};
                    case 0x43: return Instruction{InstructionType::BIT, BitFlag{0,BitFlagSource::E}};
                    case 0x44: return Instruction{InstructionType::BIT, BitFlag{0,BitFlagSource::H}};
                    case 0x45: return Instruction{InstructionType::BIT, BitFlag{0,BitFlagSource::L}};
                    case 0x46: return Instruction{InstructionType::BIT, BitFlag{0,BitFlagSource::HLI}};
                    case 0x47: return Instruction{InstructionType::BIT, BitFlag{0,BitFlagSource::A}};
                    case 0x48: return Instruction{InstructionType::BIT, BitFlag{1,BitFlagSource::B}};
                    case 0x49: return Instruction{InstructionType::BIT, BitFlag{1,BitFlagSource::C}};
                    case 0x4A: return Instruction{InstructionType::BIT, BitFlag{1,BitFlagSource::D}};
                    case 0x4B: return Instruction{InstructionType::BIT, BitFlag{1,BitFlagSource::E}};
                    case 0x4C: return Instruction{InstructionType::BIT, BitFlag{1,BitFlagSource::H}};
                    case 0x4D: return Instruction{InstructionType::BIT, BitFlag{1,BitFlagSource::L}};
                    case 0x4E: return Instruction{InstructionType::BIT, BitFlag{1,BitFlagSource::HLI}};
                    case 0x4F: return Instruction{InstructionType::BIT, BitFlag{1,BitFlagSource::A}};

                    case 0x50: return Instruction{InstructionType::BIT, BitFlag{2,BitFlagSource::B}};
                    case 0x51: return Instruction{InstructionType::BIT, BitFlag{2,BitFlagSource::C}};
                    case 0x52: return Instruction{InstructionType::BIT, BitFlag{2,BitFlagSource::D}};
                    case 0x53: return Instruction{InstructionType::BIT, BitFlag{2,BitFlagSource::E}};
                    case 0x54: return Instruction{InstructionType::BIT, BitFlag{2,BitFlagSource::H}};
                    case 0x55: return Instruction{InstructionType::BIT, BitFlag{2,BitFlagSource::L}};
                    case 0x56: return Instruction{InstructionType::BIT, BitFlag{2,BitFlagSource::HLI}};
                    case 0x57: return Instruction{InstructionType::BIT, BitFlag{2,BitFlagSource::A}};
                    case 0x58: return Instruction{InstructionType::BIT, BitFlag{3,BitFlagSource::B}};
                    case 0x59: return Instruction{InstructionType::BIT, BitFlag{3,BitFlagSource::C}};
                    case 0x5A: return Instruction{InstructionType::BIT, BitFlag{3,BitFlagSource::D}};
                    case 0x5B: return Instruction{InstructionType::BIT, BitFlag{3,BitFlagSource::E}};
                    case 0x5C: return Instruction{InstructionType::BIT, BitFlag{3,BitFlagSource::H}};
                    case 0x5D: return Instruction{InstructionType::BIT, BitFlag{3,BitFlagSource::L}};
                    case 0x5E: return Instruction{InstructionType::BIT, BitFlag{3,BitFlagSource::HLI}};
                    case 0x5F: return Instruction{InstructionType::BIT, BitFlag{3,BitFlagSource::A}};

                    case 0x60: return Instruction{InstructionType::BIT, BitFlag{4,BitFlagSource::B}};
                    case 0x61: return Instruction{InstructionType::BIT, BitFlag{4,BitFlagSource::C}};
                    case 0x62: return Instruction{InstructionType::BIT, BitFlag{4,BitFlagSource::D}};
                    case 0x63: return Instruction{InstructionType::BIT, BitFlag{4,BitFlagSource::E}};
                    case 0x64: return Instruction{InstructionType::BIT, BitFlag{4,BitFlagSource::H}};
                    case 0x65: return Instruction{InstructionType::BIT, BitFlag{4,BitFlagSource::L}};
                    case 0x66: return Instruction{InstructionType::BIT, BitFlag{4,BitFlagSource::HLI}};
                    case 0x67: return Instruction{InstructionType::BIT, BitFlag{4,BitFlagSource::A}};
                    case 0x68: return Instruction{InstructionType::BIT, BitFlag{5,BitFlagSource::B}};
                    case 0x69: return Instruction{InstructionType::BIT, BitFlag{5,BitFlagSource::C}};
                    case 0x6A: return Instruction{InstructionType::BIT, BitFlag{5,BitFlagSource::D}};
                    case 0x6B: return Instruction{InstructionType::BIT, BitFlag{5,BitFlagSource::E}};
                    case 0x6C: return Instruction{InstructionType::BIT, BitFlag{5,BitFlagSource::H}};
                    case 0x6D: return Instruction{InstructionType::BIT, BitFlag{5,BitFlagSource::L}};
                    case 0x6E: return Instruction{InstructionType::BIT, BitFlag{5,BitFlagSource::HLI}};
                    case 0x6F: return Instruction{InstructionType::BIT, BitFlag{5,BitFlagSource::A}};

                    case 0x70: return Instruction{InstructionType::BIT, BitFlag{6,BitFlagSource::B}};
                    case 0x71: return Instruction{InstructionType::BIT, BitFlag{6,BitFlagSource::C}};
                    case 0x72: return Instruction{InstructionType::BIT, BitFlag{6,BitFlagSource::D}};
                    case 0x73: return Instruction{InstructionType::BIT, BitFlag{6,BitFlagSource::E}};
                    case 0x74: return Instruction{InstructionType::BIT, BitFlag{6,BitFlagSource::H}};
                    case 0x75: return Instruction{InstructionType::BIT, BitFlag{6,BitFlagSource::L}};
                    case 0x76: return Instruction{InstructionType::BIT, BitFlag{6,BitFlagSource::HLI}};
                    case 0x77: return Instruction{InstructionType::BIT, BitFlag{6,BitFlagSource::A}};
                    case 0x78: return Instruction{InstructionType::BIT, BitFlag{7,BitFlagSource::B}};
                    case 0x79: return Instruction{InstructionType::BIT, BitFlag{7,BitFlagSource::C}};
                    case 0x7A: return Instruction{InstructionType::BIT, BitFlag{7,BitFlagSource::D}};
                    case 0x7B: return Instruction{InstructionType::BIT, BitFlag{7,BitFlagSource::E}};
                    case 0x7C: return Instruction{InstructionType::BIT, BitFlag{7,BitFlagSource::H}};
                    case 0x7D: return Instruction{InstructionType::BIT, BitFlag{7,BitFlagSource::L}};
                    case 0x7E: return Instruction{InstructionType::BIT, BitFlag{7,BitFlagSource::HLI}};
                    case 0x7F: return Instruction{InstructionType::BIT, BitFlag{7,BitFlagSource::A}};

                    case 0x80: return Instruction{InstructionType::RES, BitFlag{0,BitFlagSource::B}};
                    case 0x81: return Instruction{InstructionType::RES, BitFlag{0,BitFlagSource::C}};
                    case 0x82: return Instruction{InstructionType::RES, BitFlag{0,BitFlagSource::D}};
                    case 0x83: return Instruction{InstructionType::RES, BitFlag{0,BitFlagSource::E}};
                    case 0x84: return Instruction{InstructionType::RES, BitFlag{0,BitFlagSource::H}};
                    case 0x85: return Instruction{InstructionType::RES, BitFlag{0,BitFlagSource::L}};
                    case 0x86: return Instruction{InstructionType::RES, BitFlag{0,BitFlagSource::HLI}};
                    case 0x87: return Instruction{InstructionType::RES, BitFlag{0,BitFlagSource::A}};
                    case 0x88: return Instruction{InstructionType::RES, BitFlag{1,BitFlagSource::B}};
                    case 0x89: return Instruction{InstructionType::RES, BitFlag{1,BitFlagSource::C}};
                    case 0x8A: return Instruction{InstructionType::RES, BitFlag{1,BitFlagSource::D}};
                    case 0x8B: return Instruction{InstructionType::RES, BitFlag{1,BitFlagSource::E}};
                    case 0x8C: return Instruction{InstructionType::RES, BitFlag{1,BitFlagSource::H}};
                    case 0x8D: return Instruction{InstructionType::RES, BitFlag{1,BitFlagSource::L}};
                    case 0x8E: return Instruction{InstructionType::RES, BitFlag{1,BitFlagSource::HLI}};
                    case 0x8F: return Instruction{InstructionType::RES, BitFlag{1,BitFlagSource::A}};

                    case 0x90: return Instruction{InstructionType::RES, BitFlag{2,BitFlagSource::B}};
                    case 0x91: return Instruction{InstructionType::RES, BitFlag{2,BitFlagSource::C}};
                    case 0x92: return Instruction{InstructionType::RES, BitFlag{2,BitFlagSource::D}};
                    case 0x93: return Instruction{InstructionType::RES, BitFlag{2,BitFlagSource::E}};
                    case 0x94: return Instruction{InstructionType::RES, BitFlag{2,BitFlagSource::H}};
                    case 0x95: return Instruction{InstructionType::RES, BitFlag{2,BitFlagSource::L}};
                    case 0x96: return Instruction{InstructionType::RES, BitFlag{2,BitFlagSource::HLI}};
                    case 0x97: return Instruction{InstructionType::RES, BitFlag{2,BitFlagSource::A}};
                    case 0x98: return Instruction{InstructionType::RES, BitFlag{3,BitFlagSource::B}};
                    case 0x99: return Instruction{InstructionType::RES, BitFlag{3,BitFlagSource::C}};
                    case 0x9A: return Instruction{InstructionType::RES, BitFlag{3,BitFlagSource::D}};
                    case 0x9B: return Instruction{InstructionType::RES, BitFlag{3,BitFlagSource::E}};
                    case 0x9C: return Instruction{InstructionType::RES, BitFlag{3,BitFlagSource::H}};
                    case 0x9D: return Instruction{InstructionType::RES, BitFlag{3,BitFlagSource::L}};
                    case 0x9E: return Instruction{InstructionType::RES, BitFlag{3,BitFlagSource::HLI}};
                    case 0x9F: return Instruction{InstructionType::RES, BitFlag{3,BitFlagSource::A}};

                    case 0xA0: return Instruction{InstructionType::RES, BitFlag{4,BitFlagSource::B}};
                    case 0xA1: return Instruction{InstructionType::RES, BitFlag{4,BitFlagSource::C}};
                    case 0xA2: return Instruction{InstructionType::RES, BitFlag{4,BitFlagSource::D}};
                    case 0xA3: return Instruction{InstructionType::RES, BitFlag{4,BitFlagSource::E}};
                    case 0xA4: return Instruction{InstructionType::RES, BitFlag{4,BitFlagSource::H}};
                    case 0xA5: return Instruction{InstructionType::RES, BitFlag{4,BitFlagSource::L}};
                    case 0xA6: return Instruction{InstructionType::RES, BitFlag{4,BitFlagSource::HLI}};
                    case 0xA7: return Instruction{InstructionType::RES, BitFlag{4,BitFlagSource::A}};
                    case 0xA8: return Instruction{InstructionType::RES, BitFlag{5,BitFlagSource::B}};
                    case 0xA9: return Instruction{InstructionType::RES, BitFlag{5,BitFlagSource::C}};
                    case 0xAA: return Instruction{InstructionType::RES, BitFlag{5,BitFlagSource::D}};
                    case 0xAB: return Instruction{InstructionType::RES, BitFlag{5,BitFlagSource::E}};
                    case 0xAC: return Instruction{InstructionType::RES, BitFlag{5,BitFlagSource::H}};
                    case 0xAD: return Instruction{InstructionType::RES, BitFlag{5,BitFlagSource::L}};
                    case 0xAE: return Instruction{InstructionType::RES, BitFlag{5,BitFlagSource::HLI}};
                    case 0xAF: return Instruction{InstructionType::RES, BitFlag{5,BitFlagSource::A}};

                    case 0xB0: return Instruction{InstructionType::RES, BitFlag{6,BitFlagSource::B}};
                    case 0xB1: return Instruction{InstructionType::RES, BitFlag{6,BitFlagSource::C}};
                    case 0xB2: return Instruction{InstructionType::RES, BitFlag{6,BitFlagSource::D}};
                    case 0xB3: return Instruction{InstructionType::RES, BitFlag{6,BitFlagSource::E}};
                    case 0xB4: return Instruction{InstructionType::RES, BitFlag{6,BitFlagSource::H}};
                    case 0xB5: return Instruction{InstructionType::RES, BitFlag{6,BitFlagSource::L}};
                    case 0xB6: return Instruction{InstructionType::RES, BitFlag{6,BitFlagSource::HLI}};
                    case 0xB7: return Instruction{InstructionType::RES, BitFlag{6,BitFlagSource::A}};
                    case 0xB8: return Instruction{InstructionType::RES, BitFlag{7,BitFlagSource::B}};
                    case 0xB9: return Instruction{InstructionType::RES, BitFlag{7,BitFlagSource::C}};
                    case 0xBA: return Instruction{InstructionType::RES, BitFlag{7,BitFlagSource::D}};
                    case 0xBB: return Instruction{InstructionType::RES, BitFlag{7,BitFlagSource::E}};
                    case 0xBC: return Instruction{InstructionType::RES, BitFlag{7,BitFlagSource::H}};
                    case 0xBD: return Instruction{InstructionType::RES, BitFlag{7,BitFlagSource::L}};
                    case 0xBE: return Instruction{InstructionType::RES, BitFlag{7,BitFlagSource::HLI}};
                    case 0xBF: return Instruction{InstructionType::RES, BitFlag{7,BitFlagSource::A}};

                    case 0xC0: return Instruction{InstructionType::SET, BitFlag{0,BitFlagSource::B}};
                    case 0xC1: return Instruction{InstructionType::SET, BitFlag{0,BitFlagSource::C}};
                    case 0xC2: return Instruction{InstructionType::SET, BitFlag{0,BitFlagSource::D}};
                    case 0xC3: return Instruction{InstructionType::SET, BitFlag{0,BitFlagSource::E}};
                    case 0xC4: return Instruction{InstructionType::SET, BitFlag{0,BitFlagSource::H}};
                    case 0xC5: return Instruction{InstructionType::SET, BitFlag{0,BitFlagSource::L}};
                    case 0xC6: return Instruction{InstructionType::SET, BitFlag{0,BitFlagSource::HLI}};
                    case 0xC7: return Instruction{InstructionType::SET, BitFlag{0,BitFlagSource::A}};
                    case 0xC8: return Instruction{InstructionType::SET, BitFlag{1,BitFlagSource::B}};
                    case 0xC9: return Instruction{InstructionType::SET, BitFlag{1,BitFlagSource::C}};
                    case 0xCA: return Instruction{InstructionType::SET, BitFlag{1,BitFlagSource::D}};
                    case 0xCB: return Instruction{InstructionType::SET, BitFlag{1,BitFlagSource::E}};
                    case 0xCC: return Instruction{InstructionType::SET, BitFlag{1,BitFlagSource::H}};
                    case 0xCD: return Instruction{InstructionType::SET, BitFlag{1,BitFlagSource::L}};
                    case 0xCE: return Instruction{InstructionType::SET, BitFlag{1,BitFlagSource::HLI}};
                    case 0xCF: return Instruction{InstructionType::SET, BitFlag{1,BitFlagSource::A}};

                    case 0xD0: return Instruction{InstructionType::SET, BitFlag{2,BitFlagSource::B}};
                    case 0xD1: return Instruction{InstructionType::SET, BitFlag{2,BitFlagSource::C}};
                    case 0xD2: return Instruction{InstructionType::SET, BitFlag{2,BitFlagSource::D}};
                    case 0xD3: return Instruction{InstructionType::SET, BitFlag{2,BitFlagSource::E}};
                    case 0xD4: return Instruction{InstructionType::SET, BitFlag{2,BitFlagSource::H}};
                    case 0xD5: return Instruction{InstructionType::SET, BitFlag{2,BitFlagSource::L}};
                    case 0xD6: return Instruction{InstructionType::SET, BitFlag{2,BitFlagSource::HLI}};
                    case 0xD7: return Instruction{InstructionType::SET, BitFlag{2,BitFlagSource::A}};
                    case 0xD8: return Instruction{InstructionType::SET, BitFlag{3,BitFlagSource::B}};
                    case 0xD9: return Instruction{InstructionType::SET, BitFlag{3,BitFlagSource::C}};
                    case 0xDA: return Instruction{InstructionType::SET, BitFlag{3,BitFlagSource::D}};
                    case 0xDB: return Instruction{InstructionType::SET, BitFlag{3,BitFlagSource::E}};
                    case 0xDC: return Instruction{InstructionType::SET, BitFlag{3,BitFlagSource::H}};
                    case 0xDD: return Instruction{InstructionType::SET, BitFlag{3,BitFlagSource::L}};
                    case 0xDE: return Instruction{InstructionType::SET, BitFlag{3,BitFlagSource::HLI}};
                    case 0xDF: return Instruction{InstructionType::SET, BitFlag{3,BitFlagSource::A}};

                    case 0xE0: return Instruction{InstructionType::SET, BitFlag{4,BitFlagSource::B}};
                    case 0xE1: return Instruction{InstructionType::SET, BitFlag{4,BitFlagSource::C}};
                    case 0xE2: return Instruction{InstructionType::SET, BitFlag{4,BitFlagSource::D}};
                    case 0xE3: return Instruction{InstructionType::SET, BitFlag{4,BitFlagSource::E}};
                    case 0xE4: return Instruction{InstructionType::SET, BitFlag{4,BitFlagSource::H}};
                    case 0xE5: return Instruction{InstructionType::SET, BitFlag{4,BitFlagSource::L}};
                    case 0xE6: return Instruction{InstructionType::SET, BitFlag{4,BitFlagSource::HLI}};
                    case 0xE7: return Instruction{InstructionType::SET, BitFlag{4,BitFlagSource::A}};
                    case 0xE8: return Instruction{InstructionType::SET, BitFlag{5,BitFlagSource::B}};
                    case 0xE9: return Instruction{InstructionType::SET, BitFlag{5,BitFlagSource::C}};
                    case 0xEA: return Instruction{InstructionType::SET, BitFlag{5,BitFlagSource::D}};
                    case 0xEB: return Instruction{InstructionType::SET, BitFlag{5,BitFlagSource::E}};
                    case 0xEC: return Instruction{InstructionType::SET, BitFlag{5,BitFlagSource::H}};
                    case 0xED: return Instruction{InstructionType::SET, BitFlag{5,BitFlagSource::L}};
                    case 0xEE: return Instruction{InstructionType::SET, BitFlag{5,BitFlagSource::HLI}};
                    case 0xEF: return Instruction{InstructionType::SET, BitFlag{5,BitFlagSource::A}};

                    case 0xF0: return Instruction{InstructionType::SET, BitFlag{6,BitFlagSource::B}};
                    case 0xF1: return Instruction{InstructionType::SET, BitFlag{6,BitFlagSource::C}};
                    case 0xF2: return Instruction{InstructionType::SET, BitFlag{6,BitFlagSource::D}};
                    case 0xF3: return Instruction{InstructionType::SET, BitFlag{6,BitFlagSource::E}};
                    case 0xF4: return Instruction{InstructionType::SET, BitFlag{6,BitFlagSource::H}};
                    case 0xF5: return Instruction{InstructionType::SET, BitFlag{6,BitFlagSource::L}};
                    case 0xF6: return Instruction{InstructionType::SET, BitFlag{6,BitFlagSource::HLI}};
                    case 0xF7: return Instruction{InstructionType::SET, BitFlag{6,BitFlagSource::A}};
                    case 0xF8: return Instruction{InstructionType::SET, BitFlag{7,BitFlagSource::B}};
                    case 0xF9: return Instruction{InstructionType::SET, BitFlag{7,BitFlagSource::C}};
                    case 0xFA: return Instruction{InstructionType::SET, BitFlag{7,BitFlagSource::D}};
                    case 0xFB: return Instruction{InstructionType::SET, BitFlag{7,BitFlagSource::E}};
                    case 0xFC: return Instruction{InstructionType::SET, BitFlag{7,BitFlagSource::H}};
                    case 0xFD: return Instruction{InstructionType::SET, BitFlag{7,BitFlagSource::L}};
                    case 0xFE: return Instruction{InstructionType::SET, BitFlag{7,BitFlagSource::HLI}};
                    case 0xFF: return Instruction{InstructionType::SET, BitFlag{7,BitFlagSource::A}};

                    default: throw std::runtime_error("Invalid opcode."); break;
                    }
                } else {
                    switch (byte)
                    {
                    case 0x00: return Instruction{InstructionType::NOP};
                    case 0x01: return Instruction{InstructionType::LD, LoadByte{LoadTarget::BC, LoadSource::D16}};
                    case 0x02: return Instruction{InstructionType::LD, LoadByte{LoadTarget::BCI, LoadSource::A}};
                    case 0x03: return Instruction{InstructionType::INC, Arithmetic{ArithmeticTarget::BC}};
                    case 0x04: return Instruction{InstructionType::INC, Arithmetic{ArithmeticTarget::B}};
                    case 0x05: return Instruction{InstructionType::DEC, Arithmetic{ArithmeticTarget::B}};
                    case 0x06: return Instruction{InstructionType::LD, LoadByte{LoadTarget::B, LoadSource::D8}};
                    case 0x07: return Instruction{InstructionType::RLCA};
                    case 0x08: return Instruction{InstructionType::LD, LoadByte{LoadTarget::A16, LoadSource::SP}};
                    case 0x09: return Instruction{InstructionType::ADD, Arithmetic{ArithmeticTarget::HL, ArithmeticSource::BC}};
                    case 0x0A: return Instruction{InstructionType::LD, LoadByte{LoadTarget::A, LoadSource::BCI}};
                    case 0x0B: return Instruction{InstructionType::DEC, Arithmetic{ArithmeticTarget::BC}};
                    case 0x0C: return Instruction{InstructionType::INC, Arithmetic{ArithmeticTarget::C}};
                    case 0x0D: return Instruction{InstructionType::DEC, Arithmetic{ArithmeticTarget::C}};
                    case 0x0E: return Instruction{InstructionType::LD, LoadByte{LoadTarget::C, LoadSource::D8}};
                    case 0x0F: return Instruction{InstructionType::RRCA};

                    case 0x10: return Instruction{InstructionType::STOP};
                    case 0x11: return Instruction{InstructionType::LD, LoadByte{LoadTarget::DE, LoadSource::D16}};
                    case 0x12: return Instruction{InstructionType::LD, LoadByte{LoadTarget::DEI, LoadSource::A}};
                    case 0x13: return Instruction{InstructionType::INC, Arithmetic{ArithmeticTarget::DE}};
                    case 0x14: return Instruction{InstructionType::INC, Arithmetic{ArithmeticTarget::D}};
                    case 0x15: return Instruction{InstructionType::DEC, Arithmetic{ArithmeticTarget::D}};
                    case 0x16: return Instruction{InstructionType::LD, LoadByte{LoadTarget::D, LoadSource::D8}};
                    case 0x17: return Instruction{InstructionType::RLA};
                    case 0x18: return Instruction{InstructionType::JR, JumpTest::Always};
                    case 0x19: return Instruction{InstructionType::ADD, Arithmetic{ArithmeticTarget::HL, ArithmeticSource::DE}};
                    case 0x1A: return Instruction{InstructionType::LD, LoadByte{LoadTarget::A, LoadSource::DEI}};
                    case 0x1B: return Instruction{InstructionType::DEC, Arithmetic{ArithmeticTarget::DE}};
                    case 0x1C: return Instruction{InstructionType::INC, Arithmetic{ArithmeticTarget::E}};
                    case 0x1D: return Instruction{InstructionType::DEC, Arithmetic{ArithmeticTarget::E}};
                    case 0x1E: return Instruction{InstructionType::LD, LoadByte{LoadTarget::E, LoadSource::D8}};
                    case 0x1F: return Instruction{InstructionType::RRA};

                    case 0x20: return Instruction{InstructionType::JR, JumpTest::NotZero};
                    case 0x21: return Instruction{InstructionType::LD, LoadByte{LoadTarget::HL, LoadSource::D16}};
                    case 0x22: return Instruction{InstructionType::LD, LoadByte{LoadTarget::HLINC, LoadSource::A}};
                    case 0x23: return Instruction{InstructionType::INC, Arithmetic{ArithmeticTarget::HL}};
                    case 0x24: return Instruction{InstructionType::INC, Arithmetic{ArithmeticTarget::H}};
                    case 0x25: return Instruction{InstructionType::DEC, Arithmetic{ArithmeticTarget::H}};
                    case 0x26: return Instruction{InstructionType::LD, LoadByte{LoadTarget::H, LoadSource::D8}};
                    case 0x27: return Instruction{InstructionType::DAA};
                    case 0x28: return Instruction{InstructionType::JR, JumpTest::Zero};
                    case 0x29: return Instruction{InstructionType::ADD, Arithmetic{ArithmeticTarget::HL, ArithmeticSource::HL}};
                    case 0x2A: return Instruction{InstructionType::LD, LoadByte{LoadTarget::A, LoadSource::HLINC}};
                    case 0x2B: return Instruction{InstructionType::DEC, Arithmetic{ArithmeticTarget::HL}};
                    case 0x2C: return Instruction{InstructionType::INC, Arithmetic{ArithmeticTarget::L}};
                    case 0x2D: return Instruction{InstructionType::DEC, Arithmetic{ArithmeticTarget::L}};
                    case 0x2E: return Instruction{InstructionType::LD, LoadByte{LoadTarget::L, LoadSource::D8}};
                    case 0x2F: return Instruction{InstructionType::CPL};

                    case 0x30: return Instruction{InstructionType::JR, JumpTest::NotCarry};
                    case 0x31: return Instruction{InstructionType::LD, LoadByte{LoadTarget::SP, LoadSource::D16}};
                    case 0x32: return Instruction{InstructionType::LD, LoadByte{LoadTarget::HLDEC, LoadSource::A}};
                    case 0x33: return Instruction{InstructionType::INC, Arithmetic{ArithmeticTarget::SP}};
                    case 0x34: return Instruction{InstructionType::INC, Arithmetic{ArithmeticTarget::HLI}};
                    case 0x35: return Instruction{InstructionType::DEC, Arithmetic{ArithmeticTarget::HLI}};
                    case 0x36: return Instruction{InstructionType::LD, LoadByte{LoadTarget::HLI, LoadSource::D8}};
                    case 0x37: return Instruction{InstructionType::SCF};
                    case 0x38: return Instruction{InstructionType::JR, JumpTest::Carry};
                    case 0x39: return Instruction{InstructionType::ADD, Arithmetic{ArithmeticTarget::HL, ArithmeticSource::SP}};;
                    case 0x3A: return Instruction{InstructionType::LD, LoadByte{LoadTarget::A, LoadSource::HLDEC}};
                    case 0x3B: return Instruction{InstructionType::DEC, Arithmetic{ArithmeticTarget::SP}};
                    case 0x3C: return Instruction{InstructionType::INC, Arithmetic{ArithmeticTarget::A}};
                    case 0x3D: return Instruction{InstructionType::DEC, Arithmetic{ArithmeticTarget::A}};
                    case 0x3E: return Instruction{InstructionType::LD, LoadByte{LoadTarget::A, LoadSource::D8}};
                    case 0x3F: return Instruction{InstructionType::CCF};

                    case 0x40: return Instruction{InstructionType::LD, LoadByte{LoadTarget::B, LoadSource::B}};
                    case 0x41: return Instruction{InstructionType::LD, LoadByte{LoadTarget::B, LoadSource::C}};
                    case 0x42: return Instruction{InstructionType::LD, LoadByte{LoadTarget::B, LoadSource::D}};
                    case 0x43: return Instruction{InstructionType::LD, LoadByte{LoadTarget::B, LoadSource::E}};
                    case 0x44: return Instruction{InstructionType::LD, LoadByte{LoadTarget::B, LoadSource::H}};
                    case 0x45: return Instruction{InstructionType::LD, LoadByte{LoadTarget::B, LoadSource::L}};
                    case 0x46: return Instruction{InstructionType::LD, LoadByte{LoadTarget::B, LoadSource::HLI}};
                    case 0x47: return Instruction{InstructionType::LD, LoadByte{LoadTarget::B, LoadSource::A}};
                    case 0x48: return Instruction{InstructionType::LD, LoadByte{LoadTarget::C, LoadSource::B}};
                    case 0x49: return Instruction{InstructionType::LD, LoadByte{LoadTarget::C, LoadSource::C}};
                    case 0x4A: return Instruction{InstructionType::LD, LoadByte{LoadTarget::C, LoadSource::D}};
                    case 0x4B: return Instruction{InstructionType::LD, LoadByte{LoadTarget::C, LoadSource::E}};
                    case 0x4C: return Instruction{InstructionType::LD, LoadByte{LoadTarget::C, LoadSource::H}};
                    case 0x4D: return Instruction{InstructionType::LD, LoadByte{LoadTarget::C, LoadSource::L}};
                    case 0x4E: return Instruction{InstructionType::LD, LoadByte{LoadTarget::C, LoadSource::HLI}};
                    case 0x4F: return Instruction{InstructionType::LD, LoadByte{LoadTarget::C, LoadSource::A}};

                    case 0x50: return Instruction{InstructionType::LD, LoadByte{LoadTarget::D, LoadSource::B}};
                    case 0x51: return Instruction{InstructionType::LD, LoadByte{LoadTarget::D, LoadSource::C}};
                    case 0x52: return Instruction{InstructionType::LD, LoadByte{LoadTarget::D, LoadSource::D}};
                    case 0x53: return Instruction{InstructionType::LD, LoadByte{LoadTarget::D, LoadSource::E}};
                    case 0x54: return Instruction{InstructionType::LD, LoadByte{LoadTarget::D, LoadSource::H}};
                    case 0x55: return Instruction{InstructionType::LD, LoadByte{LoadTarget::D, LoadSource::L}};
                    case 0x56: return Instruction{InstructionType::LD, LoadByte{LoadTarget::D, LoadSource::HLI}};
                    case 0x57: return Instruction{InstructionType::LD, LoadByte{LoadTarget::D, LoadSource::A}};
                    case 0x58: return Instruction{InstructionType::LD, LoadByte{LoadTarget::E, LoadSource::B}};
                    case 0x59: return Instruction{InstructionType::LD, LoadByte{LoadTarget::E, LoadSource::C}};
                    case 0x5A: return Instruction{InstructionType::LD, LoadByte{LoadTarget::E, LoadSource::D}};
                    case 0x5B: return Instruction{InstructionType::LD, LoadByte{LoadTarget::E, LoadSource::E}};
                    case 0x5C: return Instruction{InstructionType::LD, LoadByte{LoadTarget::E, LoadSource::H}};
                    case 0x5D: return Instruction{InstructionType::LD, LoadByte{LoadTarget::E, LoadSource::L}};
                    case 0x5E: return Instruction{InstructionType::LD, LoadByte{LoadTarget::E, LoadSource::HLI}};
                    case 0x5F: return Instruction{InstructionType::LD, LoadByte{LoadTarget::E, LoadSource::A}};

                    case 0x60: return Instruction{InstructionType::LD, LoadByte{LoadTarget::H, LoadSource::B}};
                    case 0x61: return Instruction{InstructionType::LD, LoadByte{LoadTarget::H, LoadSource::C}};
                    case 0x62: return Instruction{InstructionType::LD, LoadByte{LoadTarget::H, LoadSource::D}};
                    case 0x63: return Instruction{InstructionType::LD, LoadByte{LoadTarget::H, LoadSource::E}};
                    case 0x64: return Instruction{InstructionType::LD, LoadByte{LoadTarget::H, LoadSource::H}};
                    case 0x65: return Instruction{InstructionType::LD, LoadByte{LoadTarget::H, LoadSource::L}};
                    case 0x66: return Instruction{InstructionType::LD, LoadByte{LoadTarget::H, LoadSource::HLI}};
                    case 0x67: return Instruction{InstructionType::LD, LoadByte{LoadTarget::H, LoadSource::A}};
                    case 0x68: return Instruction{InstructionType::LD, LoadByte{LoadTarget::L, LoadSource::B}};
                    case 0x69: return Instruction{InstructionType::LD, LoadByte{LoadTarget::L, LoadSource::C}};
                    case 0x6A: return Instruction{InstructionType::LD, LoadByte{LoadTarget::L, LoadSource::D}};
                    case 0x6B: return Instruction{InstructionType::LD, LoadByte{LoadTarget::L, LoadSource::E}};
                    case 0x6C: return Instruction{InstructionType::LD, LoadByte{LoadTarget::L, LoadSource::H}};
                    case 0x6D: return Instruction{InstructionType::LD, LoadByte{LoadTarget::L, LoadSource::L}};
                    case 0x6E: return Instruction{InstructionType::LD, LoadByte{LoadTarget::L, LoadSource::HLI}};
                    case 0x6F: return Instruction{InstructionType::LD, LoadByte{LoadTarget::L, LoadSource::A}};

                    case 0x70: return Instruction{InstructionType::LD, LoadByte{LoadTarget::HLI, LoadSource::B}};
                    case 0x71: return Instruction{InstructionType::LD, LoadByte{LoadTarget::HLI, LoadSource::C}};
                    case 0x72: return Instruction{InstructionType::LD, LoadByte{LoadTarget::HLI, LoadSource::D}};
                    case 0x73: return Instruction{InstructionType::LD, LoadByte{LoadTarget::HLI, LoadSource::E}};
                    case 0x74: return Instruction{InstructionType::LD, LoadByte{LoadTarget::HLI, LoadSource::H}};
                    case 0x75: return Instruction{InstructionType::LD, LoadByte{LoadTarget::HLI, LoadSource::L}};
                    case 0x76: return Instruction{InstructionType::HALT};
                    case 0x77: return Instruction{InstructionType::LD, LoadByte{LoadTarget::HLI, LoadSource::A}};
                    case 0x78: return Instruction{InstructionType::LD, LoadByte{LoadTarget::A, LoadSource::B}};
                    case 0x79: return Instruction{InstructionType::LD, LoadByte{LoadTarget::A, LoadSource::C}};
                    case 0x7A: return Instruction{InstructionType::LD, LoadByte{LoadTarget::A, LoadSource::D}};
                    case 0x7B: return Instruction{InstructionType::LD, LoadByte{LoadTarget::A, LoadSource::E}};
                    case 0x7C: return Instruction{InstructionType::LD, LoadByte{LoadTarget::A, LoadSource::H}};
                    case 0x7D: return Instruction{InstructionType::LD, LoadByte{LoadTarget::A, LoadSource::L}};
                    case 0x7E: return Instruction{InstructionType::LD, LoadByte{LoadTarget::A, LoadSource::HLI}};
                    case 0x7F: return Instruction{InstructionType::LD, LoadByte{LoadTarget::A, LoadSource::A}};

                    case 0x80: return Instruction{InstructionType::ADD, Arithmetic{ArithmeticTarget::A, ArithmeticSource::B}};
                    case 0x81: return Instruction{InstructionType::ADD, Arithmetic{ArithmeticTarget::A, ArithmeticSource::C}};
                    case 0x82: return Instruction{InstructionType::ADD, Arithmetic{ArithmeticTarget::A, ArithmeticSource::D}};
                    case 0x83: return Instruction{InstructionType::ADD, Arithmetic{ArithmeticTarget::A, ArithmeticSource::E}};
                    case 0x84: return Instruction{InstructionType::ADD, Arithmetic{ArithmeticTarget::A, ArithmeticSource::H}};
                    case 0x85: return Instruction{InstructionType::ADD, Arithmetic{ArithmeticTarget::A, ArithmeticSource::L}};
                    case 0x86: return Instruction{InstructionType::ADD, Arithmetic{ArithmeticTarget::A, ArithmeticSource::HLI}};
                    case 0x87: return Instruction{InstructionType::ADD, Arithmetic{ArithmeticTarget::A, ArithmeticSource::A}};
                    case 0x88: return Instruction{InstructionType::ADC, Arithmetic{ArithmeticTarget::A, ArithmeticSource::B}};
                    case 0x89: return Instruction{InstructionType::ADC, Arithmetic{ArithmeticTarget::A, ArithmeticSource::C}};
                    case 0x8A: return Instruction{InstructionType::ADC, Arithmetic{ArithmeticTarget::A, ArithmeticSource::D}};
                    case 0x8B: return Instruction{InstructionType::ADC, Arithmetic{ArithmeticTarget::A, ArithmeticSource::E}};
                    case 0x8C: return Instruction{InstructionType::ADC, Arithmetic{ArithmeticTarget::A, ArithmeticSource::H}};
                    case 0x8D: return Instruction{InstructionType::ADC, Arithmetic{ArithmeticTarget::A, ArithmeticSource::L}};
                    case 0x8E: return Instruction{InstructionType::ADC, Arithmetic{ArithmeticTarget::A, ArithmeticSource::HLI}};
                    case 0x8F: return Instruction{InstructionType::ADC, Arithmetic{ArithmeticTarget::A, ArithmeticSource::A}};

                    case 0x90: return Instruction{InstructionType::SUB, Arithmetic{ArithmeticTarget::A, ArithmeticSource::B}};
                    case 0x91: return Instruction{InstructionType::SUB, Arithmetic{ArithmeticTarget::A, ArithmeticSource::C}};
                    case 0x92: return Instruction{InstructionType::SUB, Arithmetic{ArithmeticTarget::A, ArithmeticSource::D}};
                    case 0x93: return Instruction{InstructionType::SUB, Arithmetic{ArithmeticTarget::A, ArithmeticSource::E}};
                    case 0x94: return Instruction{InstructionType::SUB, Arithmetic{ArithmeticTarget::A, ArithmeticSource::H}};
                    case 0x95: return Instruction{InstructionType::SUB, Arithmetic{ArithmeticTarget::A, ArithmeticSource::L}};
                    case 0x96: return Instruction{InstructionType::SUB, Arithmetic{ArithmeticTarget::A, ArithmeticSource::HLI}};
                    case 0x97: return Instruction{InstructionType::SUB, Arithmetic{ArithmeticTarget::A, ArithmeticSource::A}};
                    case 0x98: return Instruction{InstructionType::SBC, Arithmetic{ArithmeticTarget::A, ArithmeticSource::B}};
                    case 0x99: return Instruction{InstructionType::SBC, Arithmetic{ArithmeticTarget::A, ArithmeticSource::C}};
                    case 0x9A: return Instruction{InstructionType::SBC, Arithmetic{ArithmeticTarget::A, ArithmeticSource::D}};
                    case 0x9B: return Instruction{InstructionType::SBC, Arithmetic{ArithmeticTarget::A, ArithmeticSource::E}};
                    case 0x9C: return Instruction{InstructionType::SBC, Arithmetic{ArithmeticTarget::A, ArithmeticSource::H}};
                    case 0x9D: return Instruction{InstructionType::SBC, Arithmetic{ArithmeticTarget::A, ArithmeticSource::L}};
                    case 0x9E: return Instruction{InstructionType::SBC, Arithmetic{ArithmeticTarget::A, ArithmeticSource::HLI}};
                    case 0x9F: return Instruction{InstructionType::SBC, Arithmetic{ArithmeticTarget::A, ArithmeticSource::A}};

                    case 0xA0: return Instruction{InstructionType::AND, BitwiseSource::B};
                    case 0xA1: return Instruction{InstructionType::AND, BitwiseSource::C};
                    case 0xA2: return Instruction{InstructionType::AND, BitwiseSource::D};
                    case 0xA3: return Instruction{InstructionType::AND, BitwiseSource::E};
                    case 0xA4: return Instruction{InstructionType::AND, BitwiseSource::H};
                    case 0xA5: return Instruction{InstructionType::AND, BitwiseSource::L};
                    case 0xA6: return Instruction{InstructionType::AND, BitwiseSource::HLI};
                    case 0xA7: return Instruction{InstructionType::AND, BitwiseSource::A};
                    case 0xA8: return Instruction{InstructionType::XOR, BitwiseSource::B};
                    case 0xA9: return Instruction{InstructionType::XOR, BitwiseSource::C};
                    case 0xAA: return Instruction{InstructionType::XOR, BitwiseSource::D};
                    case 0xAB: return Instruction{InstructionType::XOR, BitwiseSource::E};
                    case 0xAC: return Instruction{InstructionType::XOR, BitwiseSource::H};
                    case 0xAD: return Instruction{InstructionType::XOR, BitwiseSource::L};
                    case 0xAE: return Instruction{InstructionType::XOR, BitwiseSource::HLI};
                    case 0xAF: return Instruction{InstructionType::XOR, BitwiseSource::A};

                    case 0xB0: return Instruction{InstructionType::OR, BitwiseSource::B};
                    case 0xB1: return Instruction{InstructionType::OR, BitwiseSource::C};
                    case 0xB2: return Instruction{InstructionType::OR, BitwiseSource::D};
                    case 0xB3: return Instruction{InstructionType::OR, BitwiseSource::E};
                    case 0xB4: return Instruction{InstructionType::OR, BitwiseSource::H};
                    case 0xB5: return Instruction{InstructionType::OR, BitwiseSource::L};
                    case 0xB6: return Instruction{InstructionType::OR, BitwiseSource::HLI};
                    case 0xB7: return Instruction{InstructionType::OR, BitwiseSource::A};
                    case 0xB8: return Instruction{InstructionType::CP, BitwiseSource::B};
                    case 0xB9: return Instruction{InstructionType::CP, BitwiseSource::C};
                    case 0xBA: return Instruction{InstructionType::CP, BitwiseSource::D};
                    case 0xBB: return Instruction{InstructionType::CP, BitwiseSource::E};
                    case 0xBC: return Instruction{InstructionType::CP, BitwiseSource::H};
                    case 0xBD: return Instruction{InstructionType::CP, BitwiseSource::L};
                    case 0xBE: return Instruction{InstructionType::CP, BitwiseSource::HLI};
                    case 0xBF: return Instruction{InstructionType::CP, BitwiseSource::A};

                    case 0xC0: return Instruction{InstructionType::RET, JumpTest::NotZero};
                    case 0xC1: return Instruction{InstructionType::POP, RegisterPairs::BC};
                    case 0xC2: return Instruction{InstructionType::JP, JumpTest::NotZero};
                    case 0xC3: return Instruction{InstructionType::JP, JumpTest::Always};
                    case 0xC4: return Instruction{InstructionType::CALL, JumpTest::NotZero};
                    case 0xC5: return Instruction{InstructionType::PUSH, RegisterPairs::BC};
                    case 0xC6: return Instruction{InstructionType::ADD, Arithmetic{ArithmeticTarget::A, ArithmeticSource::D8}};
                    case 0xC7: return Instruction{InstructionType::RST, Vectors::V0x00};
                    case 0xC8: return Instruction{InstructionType::RET, JumpTest::Zero};
                    case 0xC9: return Instruction{InstructionType::RET, JumpTest::Always};
                    case 0xCA: return Instruction{InstructionType::JP, JumpTest::Zero};
                    case 0xCB: break;
                    case 0xCC: return Instruction{InstructionType::CALL, JumpTest::Zero};
                    case 0xCD: return Instruction{InstructionType::CALL, JumpTest::Always};
                    case 0xCE: return Instruction{InstructionType::ADC, Arithmetic{ArithmeticTarget::A, ArithmeticSource::D8}};
                    case 0xCF: return Instruction{InstructionType::RST, Vectors::V0x08};

                    case 0xD0: return Instruction{InstructionType::RET, JumpTest::NotCarry};
                    case 0xD1: return Instruction{InstructionType::POP, RegisterPairs::DE};
                    case 0xD2: return Instruction{InstructionType::JP, JumpTest::NotCarry};
                    case 0xD3: break;
                    case 0xD4: return Instruction{InstructionType::CALL, JumpTest::NotCarry};
                    case 0xD5: return Instruction{InstructionType::PUSH, RegisterPairs::DE};
                    case 0xD6: return Instruction{InstructionType::SUB, Arithmetic{ArithmeticTarget::A, ArithmeticSource::D8}};
                    case 0xD7: return Instruction{InstructionType::RST, Vectors::V0x10};
                    case 0xD8: return Instruction{InstructionType::RET, JumpTest::Carry};
                    case 0xD9: return Instruction{InstructionType::RETI};
                    case 0xDA: return Instruction{InstructionType::JP, JumpTest::Carry};
                    case 0xDB: break;
                    case 0xDC: return Instruction{InstructionType::CALL, JumpTest::Carry};
                    case 0xDD: break;
                    case 0xDE: return Instruction{InstructionType::SBC, Arithmetic{ArithmeticTarget::A, ArithmeticSource::D8}};
                    case 0xDF: return Instruction{InstructionType::RST, Vectors::V0x18};

                    case 0xE0: return Instruction{InstructionType::LDH, LoadByte{LoadTarget::D8, LoadSource::A}};
                    case 0xE1: return Instruction{InstructionType::POP, RegisterPairs::HL};
                    case 0xE2: return Instruction{InstructionType::LDH, LoadByte{LoadTarget::C, LoadSource::A}};
                    case 0xE3: break;
                    case 0xE4: break;
                    case 0xE5: return Instruction{InstructionType::PUSH, RegisterPairs::HL};
                    case 0xE6: return Instruction{InstructionType::AND, Arithmetic{ArithmeticTarget::A, ArithmeticSource::D8}};
                    case 0xE7: return Instruction{InstructionType::RST, Vectors::V0x20};
                    case 0xE8: return Instruction{InstructionType::ADD, Arithmetic{ArithmeticTarget::SP, ArithmeticSource::D8}};
                    case 0xE9: return Instruction{InstructionType::JPHL, JumpTest::Always};
                    case 0xEA: return Instruction{InstructionType::LDH, LoadByte{LoadTarget::D16, LoadSource::A}};
                    case 0xEB: break;
                    case 0xEC: break;
                    case 0xED: break;
                    case 0xEE: return Instruction{InstructionType::XOR, BitwiseSource::D8};
                    case 0xEF: return Instruction{InstructionType::RST, Vectors::V0x28};

                    case 0xF0: return Instruction{InstructionType::LDH, LoadByte{LoadTarget::A, LoadSource::D8}};
                    case 0xF1: return Instruction{InstructionType::POP, RegisterPairs::AF};
                    case 0xF2: return Instruction{InstructionType::LDH, LoadByte{LoadTarget::A, LoadSource::C}};
                    case 0xF3: return Instruction{InstructionType::DI};
                    case 0xF4: break;
                    case 0xF5: return Instruction{InstructionType::PUSH, RegisterPairs::AF};
                    case 0xF6: return Instruction{InstructionType::OR, BitwiseSource::D8};
                    case 0xF7: return Instruction{InstructionType::RST, Vectors::V0x30};
                    case 0xF8: return Instruction{InstructionType::LDHLSP};
                    case 0xF9: return Instruction{InstructionType::LD, LoadByte{LoadTarget::SP, LoadSource::HL}};
                    case 0xFA: return Instruction{InstructionType::LDH, LoadByte{LoadTarget::A, LoadSource::D16}};
                    case 0xFB: return Instruction{InstructionType::EI};
                    case 0xFC: break;
                    case 0xFD: break;
                    case 0xFE: return Instruction{InstructionType::CP, BitwiseSource::D8};
                    case 0xFF: return Instruction{InstructionType::RST, Vectors::V0x38};

                    default: break;
                    }
                }

                throw std::runtime_error("Invalid instruction opcode.");
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

        std::pair<uint16_t, bool> overflowSub(uint16_t val1, uint16_t val2){

            return std::make_pair(val1 - val2,((val1 - val2) > val1 || (val1 - val2) > val2));

        }

        std::pair<uint8_t, bool> overflowSub(uint8_t val1, uint8_t val2){
            
            return std::make_pair(val1 - val2,((val1 - val2) > val1 || (val1 - val2) > val2));

        }
        
        uint16_t ADD(const Arithmetic& arithmetic){

            ArithmeticSource source = arithmetic.arithmeticSource;
            ArithmeticTarget target = arithmetic.arithmeticTarget;

            if (source == ArithmeticSource::D8 && target != ArithmeticTarget::SP){
                uint8_t value = this->memory.readByte(this->regs.PC+1);

                auto [result, overflow] = overflowSum(this->regs.A, value);

                changeFlag(result == 0, 0, ((this->regs.A & 0xF) + (value & 0xF)) > 0xF, overflow);
                
                this->regs.A = result;
                return this->regs.PC+2;
            }
            else if (target == ArithmeticTarget::A){
                uint8_t value = 0;
                switch (source)
                {
                case ArithmeticSource::A:
                    value = this->regs.A;
                    break;
                case ArithmeticSource::B:
                    value = this->regs.B;
                    break;
                case ArithmeticSource::C:
                    value = this->regs.C;
                    break;
                case ArithmeticSource::D:
                    value = this->regs.D;
                    break;
                case ArithmeticSource::E:
                    value = this->regs.E;
                    break;
                case ArithmeticSource::H:
                    value = this->regs.H;
                    break;
                case ArithmeticSource::L:
                    value = this->regs.L;
                    break;
                case ArithmeticSource::HLI:
                    value = this->memory.readByte(this->getCombined(RegisterPairs::HL));
                default:
                    throw std::runtime_error("Invalid Arithmetic Source at ADD instruction.");
                }

                auto [result, overflow] = overflowSum(this->regs.A, value);

                changeFlag(result == 0, 0, (this->regs.A & 0xF + value & 0xF) > 0xF, overflow);
                this->regs.A = result;
                
                return this->regs.PC+1;

            }
            else if (target == ArithmeticTarget::HL){
                uint16_t value = 0;
                switch (source)
                {
                case ArithmeticSource::AF:
                    value = this->getCombined(RegisterPairs::AF);
                    break;
                case ArithmeticSource::BC:
                    value = this->getCombined(RegisterPairs::BC);
                    break;
                case ArithmeticSource::DE:
                    value = this->getCombined(RegisterPairs::DE);
                    break;
                case ArithmeticSource::HL:
                    value = this->getCombined(RegisterPairs::HL);
                    break;
                case ArithmeticSource::SP:
                    value = this->regs.SP;
                    break;
                case ArithmeticSource::PC:
                    value = this->regs.PC;
                    break;
                
                default:
                    throw std::runtime_error("Invalid Arithmetic Source at ADD instruction.");
                }

                auto [result, overflow] = overflowSum(this->getCombined(RegisterPairs::HL), value);

                changeFlag(result == 0, 0, ((this->getCombined(RegisterPairs::HL) & 0xF) + (value & 0xF)) > 0xF, overflow);
                
                this->setCombined(RegisterPairs::HL, result);
                return this->regs.PC+1;

            }
            else if (target == ArithmeticTarget::SP){
                int8_t value = this->memory.readByte(this->regs.PC+1);

                if (value >= 0){
                    auto [result, overflow] = overflowSum(this->regs.SP, value);
                    
                    changeFlag(result == 0, 0, ((this->regs.SP & 0xF) + (value & 0xF)) > 0xF, overflow);
                    
                    this->regs.SP = result;
                }
                else {
                    auto [result, overflow] = overflowSub(this->regs.SP, abs(value));

                    changeFlag(result == 0, 0, ((this->regs.SP & 0xF) < (abs(value) & 0xF)) > 0xF, overflow);
                    
                    this->regs.SP = result;
                }
                return this->regs.PC+2;
            }
        }

        uint16_t ADC(const Arithmetic& arithmetic){
            
            ArithmeticSource source = arithmetic.arithmeticSource;
            ArithmeticTarget target = arithmetic.arithmeticTarget;

            if (source == ArithmeticSource::D8){
                uint8_t value = this->memory.readByte(this->regs.PC+1) + (this->regs.F >> 4 & 1);

                auto [result, overflow] = overflowSum(this->regs.A, value);

                changeFlag(result == 0, 0, ((this->regs.A & 0xF) +( value & 0xF)) > 0xF, overflow);
                return this->regs.PC+2;
            }
            else if (target == ArithmeticTarget::A){
                uint8_t value = 0;
                switch (source)
                {
                case ArithmeticSource::A:
                    value = this->regs.A;
                    break;
                case ArithmeticSource::B:
                    value = this->regs.B;
                    break;
                case ArithmeticSource::C:
                    value = this->regs.C;
                    break;
                case ArithmeticSource::D:
                    value = this->regs.D;
                    break;
                case ArithmeticSource::E:
                    value = this->regs.E;
                    break;
                case ArithmeticSource::H:
                    value = this->regs.H;
                    break;
                case ArithmeticSource::L:
                    value = this->regs.L;
                    break;
                case ArithmeticSource::HLI:
                    value = this->memory.readByte(this->getCombined(RegisterPairs::HL));
                default:
                    throw std::runtime_error("Invalid Arithmetic Source at ADD instruction.");
                }
                value += (this->regs.F >> 4 & 1);

                auto [result, overflow] = overflowSum(this->regs.A, value);

                changeFlag(result == 0, 0, ((this->regs.A & 0xF) + (value & 0xF)) > 0xF, overflow);
                return this->regs.PC+1;

            }
            else if (target == ArithmeticTarget::HL){
                uint16_t value = 0;
                switch (source)
                {
                case ArithmeticSource::AF:
                    value = this->getCombined(RegisterPairs::AF);
                    break;
                case ArithmeticSource::BC:
                    value = this->getCombined(RegisterPairs::BC);
                    break;
                case ArithmeticSource::DE:
                    value = this->getCombined(RegisterPairs::DE);
                    break;
                case ArithmeticSource::HL:
                    value = this->getCombined(RegisterPairs::HL);
                    break;
                case ArithmeticSource::SP:
                    value = this->regs.SP;
                    break;
                case ArithmeticSource::PC:
                    value = this->regs.PC;
                    break;
                
                default:
                    throw std::runtime_error("Invalid Arithmetic Source at ADD instruction.");
                }

                value += (this->regs.F >> 4 & 1);

                auto [result, overflow] = overflowSum(this->getCombined(RegisterPairs::HL), value);

                changeFlag(result == 0, 0, ((this->getCombined(RegisterPairs::HL)) & (0xF + value & 0xF)) > 0xF, overflow);
                return this->regs.PC+1;

            }
        }
        
        uint16_t SUB(const Arithmetic& arithmetic){
        
            ArithmeticSource source = arithmetic.arithmeticSource;

            uint8_t value = 0;

            switch (source)
            {
            case ArithmeticSource::A:
                value = this->regs.A;
                break;
            case ArithmeticSource::B:
                value = this->regs.B;
                break;
            case ArithmeticSource::C:
                value = this->regs.C;
                break;
            case ArithmeticSource::D:
                value = this->regs.D;
                break;
            case ArithmeticSource::E:
                value = this->regs.E;
                break;
            case ArithmeticSource::H:
                value = this->regs.H;
                break;
            case ArithmeticSource::L:
                value = this->regs.L;
                break;
            case ArithmeticSource::HLI:
                value = this->memory.readByte(this->getCombined(RegisterPairs::HL));
                break;
            case ArithmeticSource::D8:
                value = this->memory.readByte(this->regs.PC+1);
                break;
            
            default:
                throw std::runtime_error("Invalid source in SUB instruction."); 
                break;
            }

            auto [result, overflow] = overflowSub(this->regs.A, value);

            changeFlag(result == 0, 1, ((this->regs.A & 0xF) < (value & 0xF)), overflow);
            
            if (source==ArithmeticSource::D8) return this->regs.PC+2;
            return this->regs.PC+1;
        }

        uint16_t SBC(const Arithmetic& arithmetic){
            
            ArithmeticSource source = arithmetic.arithmeticSource;

            uint8_t value = 0;

            switch (source)
            {
            case ArithmeticSource::A:
                value = this->regs.A;
                break;
            case ArithmeticSource::B:
                value = this->regs.B;
                break;
            case ArithmeticSource::C:
                value = this->regs.C;
                break;
            case ArithmeticSource::D:
                value = this->regs.D;
                break;
            case ArithmeticSource::E:
                value = this->regs.E;
                break;
            case ArithmeticSource::H:
                value = this->regs.H;
                break;
            case ArithmeticSource::L:
                value = this->regs.L;
                break;
            case ArithmeticSource::HLI:
                value = this->memory.readByte(this->getCombined(RegisterPairs::HL));
                break;
            case ArithmeticSource::D8:
                value = this->memory.readByte(this->regs.PC+1);
                break;
            
            default:
                throw std::runtime_error("Invalid source in SUB instruction."); 
                break;
            }

            value += (this->regs.F >> 4) & 1;

            auto [result, overflow] = overflowSub(this->regs.A, value);

            changeFlag(result == 0, 1, ((this->regs.A & 0xF) < (value & 0xF)), overflow);
            
            if (source==ArithmeticSource::D8) return this->regs.PC+2;
            return this->regs.PC+1;
        }

        uint16_t CP(const Arithmetic& arithmetic){

           ArithmeticSource source = arithmetic.arithmeticSource;
           ArithmeticTarget target = arithmetic.arithmeticTarget;

            uint8_t value = 0;

            switch (source)
            {
            case ArithmeticSource::A:
                value = this->regs.A;
                break;
            case ArithmeticSource::B:
                value = this->regs.B;
                break;
            case ArithmeticSource::C:
                value = this->regs.C;
                break;
            case ArithmeticSource::D:
                value = this->regs.D;
                break;
            case ArithmeticSource::E:
                value = this->regs.E;
                break;
            case ArithmeticSource::H:
                value = this->regs.H;
                break;
            case ArithmeticSource::L:
                value = this->regs.L;
                break;
            case ArithmeticSource::HLI:
                value = this->memory.readByte(this->getCombined(RegisterPairs::HL));
                break;
            case ArithmeticSource::D8:
                value = this->memory.readByte(this->regs.PC+1);
                break;
            
            default:
                throw std::runtime_error("Invalid source in SUB instruction."); 
                break;
            }
            
            auto [result, overflow] = overflowSub(this->regs.A, value);

            changeFlag(result == 0, 1, ((this->regs.A & 0xF) < (value & 0xF)), overflow);
               
            return this->regs.PC + 1;

        }

        uint16_t DEC(const Arithmetic& arithmetic){

            ArithmeticTarget target = arithmetic.arithmeticTarget;

            if (target == ArithmeticTarget::HLI){

                uint8_t address = this->getCombined(RegisterPairs::HL);
                uint8_t value = this->memory.readByte(address);

                auto [result, overflow] = overflowSub(value, 1);

                changeFlag(result == 0, 1, (value & 0xF) < 1, (this->regs.F >> 4) & 1);

                this->memory.writeByte(address, result);

                return this->regs.PC+1;
            }

            uint8_t* reg8 = nullptr;


            switch (target)
            {
            case ArithmeticTarget::A:
                reg8 = &this->regs.A;
                break;
            case ArithmeticTarget::B:
                reg8 = &this->regs.B;
                break;
            case ArithmeticTarget::C:
                reg8 = &this->regs.C;
                break;
            case ArithmeticTarget::D:
                reg8 = &this->regs.D;
                break;
            case ArithmeticTarget::E:
                reg8 = &this->regs.E;
                break;
            case ArithmeticTarget::H:
                reg8 = &this->regs.H;
                break;
            case ArithmeticTarget::L:
                reg8 = &this->regs.L;
                break;
            
            default:
                throw std::runtime_error("Invalid target at DEC instruction");
                break;
            }

            if (reg8 != nullptr){

                auto [result, overflow] = overflowSub(*reg8, 1);

                changeFlag(result == 0, 1, (*reg8 & 0xF) < 1, (this->regs.F >> 4) & 1);

                *reg8 = result;

                return this->regs.PC+1;
            }

            uint16_t* reg16 = nullptr;
            uint16_t value = 0;

            switch (target)
            {
            
            case ArithmeticTarget::AF:
                value = this->getCombined(RegisterPairs::AF);
                break;
            case ArithmeticTarget::BC:
                value = this->getCombined(RegisterPairs::BC);
                break;
            case ArithmeticTarget::DE:
                value = this->getCombined(RegisterPairs::DE);
                break;
            case ArithmeticTarget::HL:
                value = this->getCombined(RegisterPairs::HL);
                break;
            case ArithmeticTarget::SP:
                value = this->regs.SP;
                break;
            case ArithmeticTarget::PC:
                value = this->regs.PC;
                break;
            default:
                throw std::runtime_error("Invalid target at DEC instruction.");
                break;
            }

            auto [result, overflow] = overflowSub(value, 1);

            switch (target)
            {
            
            case ArithmeticTarget::AF:
                this->setCombined(RegisterPairs::AF, result);
                break;
            case ArithmeticTarget::BC:
                this->setCombined(RegisterPairs::BC, result);
                break;
            case ArithmeticTarget::DE:
                this->setCombined(RegisterPairs::DE, result);
                value = this->getCombined(RegisterPairs::DE);
                break;
            case ArithmeticTarget::HL:
                this->setCombined(RegisterPairs::HL, result);
                break;
            case ArithmeticTarget::SP:
                this->regs.SP = result;
                break;
            case ArithmeticTarget::PC:
                this->regs.PC = result;
                break;
            default:
                break;
            }

            return this->regs.PC+1;

        }

        uint16_t INC(const Arithmetic& arithmetic){

            ArithmeticTarget target = arithmetic.arithmeticTarget;

            if (target == ArithmeticTarget::HLI){

                uint8_t address = this->getCombined(RegisterPairs::HL);
                uint8_t value = this->memory.readByte(address);

                auto [result, overflow] = overflowSum(value, 1);

                changeFlag(result == 0, false, ((value & 0xF) + 1) > 0xF, (this->regs.F >> 4) & 1);

                this->memory.writeByte(address, result);

                return this->regs.PC+1;
            }

            uint8_t* reg8 = nullptr;


            switch (target)
            {
            case ArithmeticTarget::A:
                reg8 = &this->regs.A;
                break;
            case ArithmeticTarget::B:
                reg8 = &this->regs.B;
                break;
            case ArithmeticTarget::C:
                reg8 = &this->regs.C;
                break;
            case ArithmeticTarget::D:
                reg8 = &this->regs.D;
                break;
            case ArithmeticTarget::E:
                reg8 = &this->regs.E;
                break;
            case ArithmeticTarget::H:
                reg8 = &this->regs.H;
                break;
            case ArithmeticTarget::L:
                reg8 = &this->regs.L;
                break;
            
            default:
                throw std::runtime_error("Invalid target at DEC instruction");
                break;
            }

            if (reg8 != nullptr){

                auto [result, overflow] = overflowSum(*reg8, 1);

                changeFlag(result == 0, 1, ((*reg8 & 0xF) + 1 > 0xF), (this->regs.F >> 4) & 1);

                *reg8 = result;

                return this->regs.PC+1;
            }

            uint16_t* reg16 = nullptr;
            uint16_t value = 0;

            switch (target)
            {
            
            case ArithmeticTarget::AF:
                value = this->getCombined(RegisterPairs::AF);
                break;
            case ArithmeticTarget::BC:
                value = this->getCombined(RegisterPairs::BC);
                break;
            case ArithmeticTarget::DE:
                value = this->getCombined(RegisterPairs::DE);
                break;
            case ArithmeticTarget::HL:
                value = this->getCombined(RegisterPairs::HL);
                break;
            case ArithmeticTarget::SP:
                value = this->regs.SP;
                break;
            case ArithmeticTarget::PC:
                value = this->regs.PC;
                break;
            default:
                throw std::runtime_error("Invalid target at DEC instruction.");
                break;
            }

            auto [result, overflow] = overflowSum(value, 1);

            switch (target)
            {
            
            case ArithmeticTarget::AF:
                this->setCombined(RegisterPairs::AF, result);
                break;
            case ArithmeticTarget::BC:
                this->setCombined(RegisterPairs::BC, result);
                break;
            case ArithmeticTarget::DE:
                this->setCombined(RegisterPairs::DE, result);
                value = this->getCombined(RegisterPairs::DE);
                break;
            case ArithmeticTarget::HL:
                this->setCombined(RegisterPairs::HL, result);
                break;
            case ArithmeticTarget::SP:
                this->regs.SP = result;
                break;
            case ArithmeticTarget::PC:
                this->regs.PC = result;
                break;
            default:
                break;
            }

            return this->regs.PC+1;

        }

        uint16_t AND(const BitwiseSource& bitwiseSource){

            uint8_t byteA = this->regs.A;
            uint8_t value;

            if (bitwiseSource == BitwiseSource::D8){
                value = this->memory.readByte(this->regs.PC+1);

                uint8_t bitwise = byteA & value;

                changeFlag(bitwise == 0, false, true, false);

                this->regs.A = bitwise;

                return this->regs.PC+2;
            }
            
            switch (bitwiseSource)
            {
            case BitwiseSource::A:
                value = this->regs.A;
                break;
            case BitwiseSource::B:
                value = this->regs.B;
                break;
            case BitwiseSource::C:
                value = this->regs.C;
                break;
            case BitwiseSource::D:
                value = this->regs.D;
                break;
            case BitwiseSource::E:
                value = this->regs.E;
                break;
            case BitwiseSource::F:
                value = this->regs.F;
                break;
            case BitwiseSource::H:
                value = this->regs.H;
                break;
            case BitwiseSource::L:
                value = this->regs.L;
                break;
            case BitwiseSource::HLI:
                value = this->memory.readByte(this->getCombined(RegisterPairs::HL));
                break;

            default:
                throw std::runtime_error("Invalid source at ADD instruction");
                break;
            }

            uint8_t bitwise = byteA & value;

            changeFlag(bitwise == 0, false, true, false);

            this->regs.A = bitwise;

            return this->regs.PC+1;
        }

        uint16_t CPL(){

            this->regs.A = ~this->regs.A;

            changeFlag((this->regs.F>>7) & 1, true, true, (this->regs.F >> 4) & 1);

            return this->regs.PC + 1;
        }

        uint16_t OR(const BitwiseSource& bitwiseSource){

            uint8_t byteA = this->regs.A;
            uint8_t value;
            uint8_t bitwise;

            if (bitwiseSource == BitwiseSource::D8){

                value = this->memory.readByte(this->regs.PC+1);

                bitwise = byteA | value;

                changeFlag(bitwise == 0, false, false, false);

                return this->regs.PC+2;

            }

            switch (bitwiseSource)
            {
            case BitwiseSource::A:
                value = this->regs.A;
                break;
            case BitwiseSource::B:
                value = this->regs.B;
                break;
            case BitwiseSource::C:
                value = this->regs.C;
                break;
            case BitwiseSource::D:
                value = this->regs.D;
                break;
            case BitwiseSource::E:
                value = this->regs.E;
                break;
            case BitwiseSource::F:
                value = this->regs.F;
                break;
            case BitwiseSource::H:
                value = this->regs.H;
                break;
            case BitwiseSource::L:
                value = this->regs.L;
                break;
            case BitwiseSource::HLI:
                value = this->memory.readByte(this->getCombined(RegisterPairs::HL));
                break;
            
            default:
                throw std::runtime_error("Invalid source at OR instruction.");
                break;
            }

            bitwise = byteA | bitwise;

            changeFlag(bitwise == 0, false, false, false);

            this->regs.A = bitwise;
            
            return this->regs.PC+1;

        }

        uint16_t XOR(const BitwiseSource& bitwiseSource){

            uint8_t byteA = this->regs.A;
            uint8_t value;
            uint8_t bitwise;

            if (bitwiseSource == BitwiseSource::D8){

                value = this->memory.readByte(this->regs.PC+1);

                bitwise = byteA ^ value;

                changeFlag(bitwise == 0, false, false, false);

                return this->regs.PC+2;

            }

            switch (bitwiseSource)
            {
            case BitwiseSource::A:
                value = this->regs.A;
                break;
            case BitwiseSource::B:
                value = this->regs.B;
                break;
            case BitwiseSource::C:
                value = this->regs.C;
                break;
            case BitwiseSource::D:
                value = this->regs.D;
                break;
            case BitwiseSource::E:
                value = this->regs.E;
                break;
            case BitwiseSource::F:
                value = this->regs.F;
                break;
            case BitwiseSource::H:
                value = this->regs.H;
                break;
            case BitwiseSource::L:
                value = this->regs.L;
                break;
            case BitwiseSource::HLI:
                value = this->memory.readByte(this->getCombined(RegisterPairs::HL));
                break;
            
            default:
                throw std::runtime_error("Invalid source at OR instruction.");
                break;
            }

            bitwise = byteA ^ bitwise;

            changeFlag(bitwise == 0, false, false, false);

            this->regs.A = bitwise;
            
            return this->regs.PC+1;
        }

        uint16_t BIT(const BitFlag& bitFlag){

            BitFlagSource source = bitFlag.bitFlagSource;
            unsigned int u3 = bitFlag.u3;

            bool bitSet;
            uint8_t byte;

            switch (source)
            {
            case BitFlagSource::A:
                byte = this->regs.A;
                break;
            case BitFlagSource::B:
                byte = this->regs.B;
                break;
            case BitFlagSource::C:
                byte = this->regs.C;
                break;
            case BitFlagSource::D:
                byte = this->regs.D;
                break;
            case BitFlagSource::E:
                byte = this->regs.E;
                break;
            case BitFlagSource::F:
                byte = this->regs.F;
                break;
            case BitFlagSource::H:
                byte = this->regs.H;
                break;
            case BitFlagSource::L:
                byte = this->regs.L;
                break;
            case BitFlagSource::HLI:
                byte = this->memory.readByte(this->getCombined(RegisterPairs::HL));
                break;
            
            default:
                throw std::runtime_error("Invalid source at BIT instruction.");
                break;
            }

            bitSet = (byte >> u3) & 1;

            changeFlag(bitSet, 0, 1, (this->regs.F >> 4) & 1);

            return this->regs.PC + 1;
        }

        uint16_t RES(const BitFlag& bitFlag){

            BitFlagSource source = bitFlag.bitFlagSource;
            unsigned int u3 = bitFlag.u3;

            switch (source)
            {
            case BitFlagSource::A:
                this->regs.A &= ~(0b00000001 << u3);
                break;
            case BitFlagSource::B:
                this->regs.B &= ~(0b00000001 << u3);
                break;
            case BitFlagSource::C:
                this->regs.C &= ~(0b00000001 << u3);
                break;
            case BitFlagSource::D:
                this->regs.D &= ~(0b00000001 << u3);
                break;
            case BitFlagSource::E:
                this->regs.E &= ~(0b00000001 << u3);
                break;
            case BitFlagSource::F:
                this->regs.F &= ~(0b00000001 << u3);
                break;
            case BitFlagSource::H:
                this->regs.H &= ~(0b00000001 << u3);
                break;
            case BitFlagSource::L:
                this->regs.L &= ~(0b00000001 << u3);
                break;
            case BitFlagSource::HLI:
                this->memory.writeByte(this->getCombined(RegisterPairs::HL), (this->memory.readByte(this->getCombined(RegisterPairs::HL) & ~(0b00000001 < u3))));
                break;
            
            default:
                throw std::runtime_error("Invalid source at RES instruction.");
                break;
            }

            return this->regs.PC+1;

        }

        uint16_t SET(const BitFlag& bitFlag){

            BitFlagSource source = bitFlag.bitFlagSource;
            unsigned int u3 = bitFlag.u3;

            switch (source)
            {
            case BitFlagSource::A:
                this->regs.A |= (0b00000001 << u3);
                break;
            case BitFlagSource::B:
                this->regs.B |= (0b00000001 << u3);
                break;
            case BitFlagSource::C:
                this->regs.C |= (0b00000001 << u3);
                break;
            case BitFlagSource::D:
                this->regs.D |= (0b00000001 << u3);
                break;
            case BitFlagSource::E:
                this->regs.E |= (0b00000001 << u3);
                break;
            case BitFlagSource::F:
                this->regs.F |= (0b00000001 << u3);
                break;
            case BitFlagSource::H:
                this->regs.H |= (0b00000001 << u3);
                break;
            case BitFlagSource::L:
                this->regs.L |= (0b00000001 << u3);
                break;
            case BitFlagSource::HLI:
                this->memory.writeByte(this->getCombined(RegisterPairs::HL), (this->memory.readByte(this->getCombined(RegisterPairs::HL) | (0b00000001 < u3))));
                break;
            
            default:
                throw std::runtime_error("Invalid source at RES instruction.");
                break;
            }

            return this->regs.PC+1;
        }

        uint16_t RL(const RotateTarget& target){

            uint8_t carry = (this->regs.F >> 4) & 1;
            uint8_t bit7 = 0;
            uint8_t result = 0;

            switch (target)
            {
            case RotateTarget::A:
                bit7 = (this->regs.A >> 7) & 1;
                result = static_cast<uint8_t>(this->regs.A << 1) | carry;
                this->regs.A = result;
                break;
            case RotateTarget::B:
                bit7 = (this->regs.B >> 7) & 1;
                result = static_cast<uint8_t>(this->regs.B << 1) | carry;
                this->regs.B = result;
                break;
            case RotateTarget::C:
                bit7 = (this->regs.C >> 7) & 1;
                result = static_cast<uint8_t>(this->regs.C << 1) | carry;
                this->regs.C = result;
                break;
            case RotateTarget::D:
                bit7 = (this->regs.D >> 7) & 1;
                result = static_cast<uint8_t>(this->regs.D << 1) | carry;
                this->regs.D = result;
                break;
            case RotateTarget::E:
                bit7 = (this->regs.E >> 7) & 1;
                result = static_cast<uint8_t>(this->regs.E << 1) | carry;
                this->regs.E = result;
                break;
            case RotateTarget::H:
                bit7 = (this->regs.H >> 7) & 1;
                result = static_cast<uint8_t>(this->regs.H << 1) | carry;
                this->regs.H = result;
                break;
            case RotateTarget::L:
                bit7 = (this->regs.L >> 7) & 1;
                result = static_cast<uint8_t>(this->regs.L << 1) | carry;
                this->regs.L = result;
                break;
            case RotateTarget::HLI:
                bit7 = (this->memory.readByte(this->getCombined(RegisterPairs::HL)) >> 7) & 1;
                result = static_cast<uint8_t>(this->memory.readByte(this->getCombined(RegisterPairs::HL)) << 1) | carry;
                this->memory.writeByte(this->getCombined(RegisterPairs::HL), result);
                break;
            
            default:
                throw std::runtime_error("Invalid target at the RL instruction.");
                break;
            }

            changeFlag(result == 0, false, false, bit7 == 1);

            return this->regs.PC+1;

        }

        uint16_t RLC(const RotateTarget& target){
            
            uint8_t bit7 = 0;
            uint8_t result = 0;

            switch (target)
            {
            case RotateTarget::A:
                bit7 = (this->regs.A >> 7) & 1;
                result = static_cast<uint8_t>(this->regs.A << 1) | bit7;
                this->regs.A = result;
                break;
            case RotateTarget::B:
                bit7 = (this->regs.B >> 7) & 1;
                result = static_cast<uint8_t>(this->regs.B << 1) | bit7;
                this->regs.B = result;
                break;
            case RotateTarget::C:
                bit7 = (this->regs.C >> 7) & 1;
                result = static_cast<uint8_t>(this->regs.C << 1) | bit7;
                this->regs.C = result;
                break;
            case RotateTarget::D:
                bit7 = (this->regs.D >> 7) & 1;
                result = static_cast<uint8_t>(this->regs.D << 1) | bit7;
                this->regs.D = result;
                break;
            case RotateTarget::E:
                bit7 = (this->regs.E >> 7) & 1;
                result = static_cast<uint8_t>(this->regs.E << 1) | bit7;
                this->regs.E = result;
                break;
            case RotateTarget::H:
                bit7 = (this->regs.H >> 7) & 1;
                result = static_cast<uint8_t>(this->regs.H << 1) | bit7;
                this->regs.H = result;
                break;
            case RotateTarget::L:
                bit7 = (this->regs.L >> 7) & 1;
                result = static_cast<uint8_t>(this->regs.L << 1) | bit7;
                this->regs.L = result;
                break;
            case RotateTarget::HLI:
                bit7 = (this->memory.readByte(this->getCombined(RegisterPairs::HL)) >> 7) & 1;
                result = static_cast<uint8_t>(this->memory.readByte(this->getCombined(RegisterPairs::HL)) << 1) | bit7;
                this->memory.writeByte(this->getCombined(RegisterPairs::HL), result);
                break;
            
            default:
                throw std::runtime_error("Invalid target at the RL instruction.");
                break;
            }

            changeFlag(result == 0, false, false, bit7 == 1);

            return this->regs.PC+1;
        }

        uint16_t RR(const RotateTarget& target){

            uint8_t carry = (this->regs.F >> 4) & 1;
            uint8_t bit0 = 0;
            uint8_t result = 0;

            switch (target)
            {
            case RotateTarget::A:
                bit0 = (this->regs.A) & 1;
                result = static_cast<uint8_t>(this->regs.A >> 1) | static_cast<uint8_t>(carry << 7);
                this->regs.A = result;
                break;
            case RotateTarget::B:
                bit0 = (this->regs.B) & 1;
                result = static_cast<uint8_t>(this->regs.B >> 1) | static_cast<uint8_t>(carry << 7);
                this->regs.B = result;
                break;
            case RotateTarget::C:
                bit0 = (this->regs.C ) & 1;
                result = static_cast<uint8_t>(this->regs.C >> 1) | static_cast<uint8_t>(carry << 7);
                this->regs.C = result;
                break;
            case RotateTarget::D:
                bit0 = (this->regs.D) & 1;
                result = static_cast<uint8_t>(this->regs.D >> 1) | static_cast<uint8_t>(carry << 7);
                this->regs.D = result;
                break;
            case RotateTarget::E:
                bit0 = (this->regs.E) & 1;
                result = static_cast<uint8_t>(this->regs.E >> 1) | static_cast<uint8_t>(carry << 7);
                this->regs.E = result;
                break;
            case RotateTarget::H:
                bit0 = (this->regs.H ) & 1;
                result = static_cast<uint8_t>(this->regs.H >> 1) | static_cast<uint8_t>(carry << 7);
                this->regs.H = result;
                break;
            case RotateTarget::L:
                bit0 = (this->regs.L) & 1;
                result = static_cast<uint8_t>(this->regs.L >> 1) | static_cast<uint8_t>(carry << 7);
                this->regs.L = result;
                break;
            case RotateTarget::HLI:
                bit0 = (this->memory.readByte(this->getCombined(RegisterPairs::HL))) & 1;
                result = static_cast<uint8_t>(this->memory.readByte(this->getCombined(RegisterPairs::HL)) >> 1) | static_cast<uint8_t>(carry << 7);
                this->memory.writeByte(this->getCombined(RegisterPairs::HL), result);
                break;
            
            default:
                throw std::runtime_error("Invalid target at the RL instruction.");
                break;
            }

            changeFlag(result == 0, false, false, bit0 == 1);

            return this->regs.PC+1;

        }

        uint16_t RRC(const RotateTarget& target){

            uint8_t bit0 = 0;
            uint8_t result = 0;

            switch (target)
            {
            case RotateTarget::A:
                bit0 = (this->regs.A) & 1;
                result = static_cast<uint8_t>(this->regs.A >> 1) | static_cast<uint8_t>(bit0 << 7);
                this->regs.A = result;
                break;
            case RotateTarget::B:
                bit0 = (this->regs.B) & 1;
                result = static_cast<uint8_t>(this->regs.B >> 1) | static_cast<uint8_t>(bit0 << 7);
                this->regs.B = result;
                break;
            case RotateTarget::C:
                bit0 = (this->regs.C ) & 1;
                result = static_cast<uint8_t>(this->regs.C >> 1) | static_cast<uint8_t>(bit0 << 7);
                this->regs.C = result;
                break;
            case RotateTarget::D:
                bit0 = (this->regs.D) & 1;
                result = static_cast<uint8_t>(this->regs.D >> 1) | static_cast<uint8_t>(bit0 << 7);
                this->regs.D = result;
                break;
            case RotateTarget::E:
                bit0 = (this->regs.E) & 1;
                result = static_cast<uint8_t>(this->regs.E >> 1) | static_cast<uint8_t>(bit0 << 7);
                this->regs.E = result;
                break;
            case RotateTarget::H:
                bit0 = (this->regs.H ) & 1;
                result = static_cast<uint8_t>(this->regs.H >> 1) | static_cast<uint8_t>(bit0 << 7);
                this->regs.H = result;
                break;
            case RotateTarget::L:
                bit0 = (this->regs.L) & 1;
                result = static_cast<uint8_t>(this->regs.L >> 1) | static_cast<uint8_t>(bit0 << 7);
                this->regs.L = result;
                break;
            case RotateTarget::HLI:
                bit0 = (this->memory.readByte(this->getCombined(RegisterPairs::HL))) & 1;
                result = static_cast<uint8_t>(this->memory.readByte(this->getCombined(RegisterPairs::HL)) >> 1) | static_cast<uint8_t>(bit0 << 7);
                this->memory.writeByte(this->getCombined(RegisterPairs::HL), result);
                break;
            
            default:
                throw std::runtime_error("Invalid target at the RL instruction.");
                break;
            }

            changeFlag(result == 0, false, false, bit0 == 1);

            return this->regs.PC+1;
        }

        uint16_t SLA(const RotateTarget& target){
            
            uint8_t bit7 = 0;
            uint8_t result = 0;

            switch (target)
            {
            case RotateTarget::A:
                bit7 = (this->regs.A >> 7) & 1;
                result = static_cast<uint8_t>(this->regs.A << 1);
                this->regs.A = result;
                break;
            case RotateTarget::B:
                bit7 = (this->regs.B >> 7) & 1;
                result = static_cast<uint8_t>(this->regs.B << 1);
                this->regs.B = result;
                break;
            case RotateTarget::C:
                bit7 = (this->regs.C >> 7) & 1;
                result = static_cast<uint8_t>(this->regs.C << 1);
                this->regs.C = result;
                break;
            case RotateTarget::D:
                bit7 = (this->regs.D >> 7) & 1;
                result = static_cast<uint8_t>(this->regs.D << 1);
                this->regs.D = result;
                break;
            case RotateTarget::E:
                bit7 = (this->regs.E >> 7) & 1;
                result = static_cast<uint8_t>(this->regs.E << 1);
                this->regs.E = result;
                break;
            case RotateTarget::H:
                bit7 = (this->regs.H >> 7) & 1;
                result = static_cast<uint8_t>(this->regs.H << 1);
                this->regs.H = result;
                break;
            case RotateTarget::L:
                bit7 = (this->regs.L >> 7) & 1;
                result = static_cast<uint8_t>(this->regs.L << 1);
                this->regs.L = result;
                break;
            case RotateTarget::HLI:
                bit7 = (this->memory.readByte(this->getCombined(RegisterPairs::HL)) >> 7) & 1;
                result = static_cast<uint8_t>(this->memory.readByte(this->getCombined(RegisterPairs::HL)) << 1);
                this->memory.writeByte(this->getCombined(RegisterPairs::HL), result);
                break;
            
            default:
                throw std::runtime_error("Invalid target at the RL instruction.");
                break;
            }

            changeFlag(result == 0, false, false, bit7 == 1);

            return this->regs.PC+1;

        }

        uint16_t SRA(const RotateTarget& target){
            
            uint8_t bit0 = 0;
            uint8_t bit7 = 0;
            uint8_t result = 0;

            switch (target)
            {
            case RotateTarget::A:
                bit0 = (this->regs.A) & 1;
                bit7 = (this->regs.A >> 7) & 1;
                result = static_cast<uint8_t>(this->regs.A >> 1) | static_cast<uint8_t>(bit7 << 7);
                this->regs.A = result;
                break;
            case RotateTarget::B:
                bit0 = (this->regs.B) & 1;
                bit7 = (this->regs.B >> 7) & 1;
                result = static_cast<uint8_t>(this->regs.B >> 1) | static_cast<uint8_t>(bit7 << 7);
                this->regs.B = result;
                break;
            case RotateTarget::C:
                bit0 = (this->regs.C ) & 1;
                bit7 = (this->regs.C >> 7) & 1;
                result = static_cast<uint8_t>(this->regs.C >> 1) | static_cast<uint8_t>(bit7 << 7);
                this->regs.C = result;
                break;
            case RotateTarget::D:
                bit0 = (this->regs.D) & 1;
                bit7 = (this->regs.D >> 7) & 1;
                result = static_cast<uint8_t>(this->regs.D >> 1) | static_cast<uint8_t>(bit7 << 7);
                this->regs.D = result;
                break;
            case RotateTarget::E:
                bit0 = (this->regs.E) & 1;
                bit7 = (this->regs.E >> 7) & 1;
                result = static_cast<uint8_t>(this->regs.E >> 1) | static_cast<uint8_t>(bit7 << 7);
                this->regs.E = result;
                break;
            case RotateTarget::H:
                bit0 = (this->regs.H ) & 1;
                bit7 = (this->regs.H >> 7) & 1;
                result = static_cast<uint8_t>(this->regs.H >> 1) | static_cast<uint8_t>(bit7 << 7);
                this->regs.H = result;
                break;
            case RotateTarget::L:
                bit0 = (this->regs.L) & 1;
                bit7 = (this->regs.L >> 7) & 1;
                result = static_cast<uint8_t>(this->regs.L >> 1) | static_cast<uint8_t>(bit7 << 7);
                this->regs.L = result;
                break;
            case RotateTarget::HLI:
                bit0 = (this->memory.readByte(this->getCombined(RegisterPairs::HL))) & 1;
                bit7 = (this->memory.readByte(this->getCombined(RegisterPairs::HL)) >> 7) & 1;
                result = static_cast<uint8_t>(this->memory.readByte(this->getCombined(RegisterPairs::HL)) >> 1) | static_cast<uint8_t>(bit7 << 7);
                this->memory.writeByte(this->getCombined(RegisterPairs::HL), result);
                break;
            
            default:
                throw std::runtime_error("Invalid target at the RL instruction.");
                break;
            }

            changeFlag(result == 0, false, false, bit0 == 1);

            return this->regs.PC+1;
        }

        uint16_t SRL(const RotateTarget& target){
            
            uint8_t bit0 = 0;
            uint8_t result = 0;
            
            switch (target)
            {
            case RotateTarget::A:
                bit0 = (this->regs.A) & 1;
                result = static_cast<uint8_t>(this->regs.A >> 1);
                this->regs.A = result;
                break;
            case RotateTarget::B:
                bit0 = (this->regs.B) & 1;
                result = static_cast<uint8_t>(this->regs.B >> 1) ;
                this->regs.B = result;
                break;
            case RotateTarget::C:
                bit0 = (this->regs.C ) & 1;
                result = static_cast<uint8_t>(this->regs.C >> 1);
                this->regs.C = result;
                break;
            case RotateTarget::D:
                bit0 = (this->regs.D) & 1;
                result = static_cast<uint8_t>(this->regs.D >> 1);
                this->regs.D = result;
                break;
            case RotateTarget::E:
                bit0 = (this->regs.E) & 1;
                result = static_cast<uint8_t>(this->regs.E >> 1);
                this->regs.E = result;
                break;
            case RotateTarget::H:
                bit0 = (this->regs.H ) & 1;
                result = static_cast<uint8_t>(this->regs.H >> 1);
                this->regs.H = result;
                break;
            case RotateTarget::L:
                bit0 = (this->regs.L) & 1;
                result = static_cast<uint8_t>(this->regs.L >> 1);
                this->regs.L = result;
                break;
            case RotateTarget::HLI:
                bit0 = (this->memory.readByte(this->getCombined(RegisterPairs::HL))) & 1;
                result = static_cast<uint8_t>(this->memory.readByte(this->getCombined(RegisterPairs::HL)) >> 1);
                this->memory.writeByte(this->getCombined(RegisterPairs::HL), result);
                break;
            
            default:
                throw std::runtime_error("Invalid target at the RL instruction.");
                break;
            }

            changeFlag(result == 0, false, false, bit0 == 1);

            return this->regs.PC+1;
        }

        uint16_t SWAP(const RotateTarget& target){

            uint8_t upper = 0;
            uint8_t lower = 0;
            uint8_t result = 0;

            switch (target)
            {
            case RotateTarget::A:
                upper = (this->regs.A >> 4) & 0b1111;
                lower = (this->regs.A) & 0b1111;
                result = static_cast<uint8_t>(upper << 4) | lower;
                this->regs.A = result;
                break;
            case RotateTarget::B:
                upper = (this->regs.B >> 4) & 0b1111;
                lower = (this->regs.B) & 0b1111;
                result = static_cast<uint8_t>(upper << 4) | lower;
                this->regs.B = result;
                break;
            case RotateTarget::C:
                upper = (this->regs.C >> 4) & 0b1111;
                lower = (this->regs.C) & 0b1111;
                result = static_cast<uint8_t>(upper << 4) | lower;
                this->regs.C = result;
                break;
            case RotateTarget::D:
                upper = (this->regs.D >> 4) & 0b1111;
                lower = (this->regs.D) & 0b1111;
                result = static_cast<uint8_t>(upper << 4) | lower;
                this->regs.D = result;
                break;
            case RotateTarget::E:
                upper = (this->regs.E >> 4) & 0b1111;
                lower = (this->regs.E) & 0b1111;
                result = static_cast<uint8_t>(upper << 4) | lower;
                this->regs.E = result;
                break;
            case RotateTarget::H:
                upper = (this->regs.H >> 4) & 0b1111;
                lower = (this->regs.H) & 0b1111;
                result = static_cast<uint8_t>(upper << 4) | lower;
                this->regs.H = result;
                break;
            case RotateTarget::L:
                upper = (this->regs.L >> 4) & 0b1111;
                lower = (this->regs.L) & 0b1111;
                result = static_cast<uint8_t>(upper << 4) | lower;
                this->regs.L = result;
                break;
            case RotateTarget::HLI:
                upper = (this->memory.readByte(this->getCombined(RegisterPairs::HL)) >> 4) & 0b1111;
                lower = (this->memory.readByte(this->getCombined(RegisterPairs::HL))) & 0b1111;
                result = static_cast<uint8_t>(upper << 4) | lower;
                this->memory.writeByte(this->getCombined(RegisterPairs::HL), result);
                break;
            
            default:
                throw std::runtime_error("Invalid target at SWAP instruction.");
                break;
            }

            changeFlag(result == 0, false, false, false);

            return this->regs.PC+1;

        }

        // JP instruction that receives a boolean condition to decide if it should jump to the designed address of the following 2 bytes.
        uint16_t JP(bool jump){

            if (jump){
                uint8_t lsByte = this->memory.readByte(this->regs.PC+1);
                uint8_t msByte = this->memory.readByte(this->regs.PC+2);

                return static_cast<uint16_t>(msByte << 8 ) | static_cast<uint16_t>(lsByte);
            }
            else return this->regs.PC+3;
        }

        uint16_t JR(bool jump){

            if (jump){
                int8_t offset = this->memory.readByte(this->regs.PC+1);
                return static_cast<uint8_t>(this->regs.PC + offset);

            }else return this->regs.PC+2;
        }

        uint16_t JPHL(bool jump){
            if (jump){
                uint16_t lsByte = this->getCombined(RegisterPairs::HL) & 0b1111;
                uint16_t msByte = (this->getCombined(RegisterPairs::HL) >> 4) & 0b1111;

                return static_cast<uint16_t>(msByte << 8 ) | static_cast<uint16_t>(lsByte);
            }
            else return this->regs.PC+3;
        }

        // LD instruction that receives a LoadByte and do the logic with it.
        // TODO: Add more sources and targets.
        uint16_t LD(const LoadByte& loadByte){

            uint8_t sourceValue = 0;
            uint16_t sourceValue16 = 0;
            uint16_t toReturn = this->regs.PC+1;

            switch (loadByte.loadSource)
            {
            case LoadSource::A:
                sourceValue = this->regs.A;
                break;
            case LoadSource::B:
                sourceValue = this->regs.B;
                break;
            case LoadSource::C:
                sourceValue = this->regs.C;
                break;
            case LoadSource::D:
                sourceValue = this->regs.D;
                break;
            case LoadSource::E:
                sourceValue = this->regs.E;
                break;
            case LoadSource::H:
                sourceValue = this->regs.H;
                break;
            case LoadSource::L:
                sourceValue = this->regs.L;
                break;
            case LoadSource::BC:
                sourceValue16 = this->getCombined(RegisterPairs::BC);
                break;
            case LoadSource::DE:
                sourceValue16 = this->getCombined(RegisterPairs::DE);
                break;
            case LoadSource::HL:
                sourceValue16 = this->getCombined(RegisterPairs::HL);
                break;
            case LoadSource::D16:
                sourceValue16 = this->memory.readWord(this->regs.PC+1);
                toReturn += 2;
                break;
            case LoadSource::D8:
                sourceValue = this->memory.readByte(this->regs.PC+1);
                toReturn+=1;
                break;
            case LoadSource::HLI:
                sourceValue = this->memory.readByte(getCombined(RegisterPairs::HL));
                break;
            case LoadSource::BCI:
                sourceValue = this->memory.readByte(getCombined(RegisterPairs::BC));
                break;
            case LoadSource::DEI:
                sourceValue = this->memory.readByte(getCombined(RegisterPairs::DE));
                break;
            case LoadSource::HLDEC:
                sourceValue = this->memory.readByte(getCombined(RegisterPairs::HL));
                this->setCombined(RegisterPairs::HL, this->getCombined(RegisterPairs::HL)-1);
                break;
            case LoadSource::HLINC:
                sourceValue = this->memory.readByte(getCombined(RegisterPairs::HL));
                this->setCombined(RegisterPairs::HL, this->getCombined(RegisterPairs::HL)+1);
                break;
            case LoadSource::SP:
                sourceValue16 = this->regs.SP;
                break;
            default:
                throw std::runtime_error("Not implemented source on load.");
            }

            switch (loadByte.loadTarget)
            {
            case LoadTarget::A:
                this->regs.A = sourceValue;
                break;
            case LoadTarget::B:
                this->regs.B = sourceValue;
                break;
            case LoadTarget::C:
                this->regs.C = sourceValue;
                break;
            case LoadTarget::D:
                this->regs.D = sourceValue;
                break;
            case LoadTarget::E:
                this->regs.E = sourceValue;
                break;
            case LoadTarget::H:
                this->regs.H = sourceValue;
                break;
            case LoadTarget::L:
                this->regs.L = sourceValue;
                break;
            case LoadTarget::BC:
                this->setCombined(RegisterPairs::BC, sourceValue);
                break;
            case LoadTarget::DE:
                this->setCombined(RegisterPairs::DE, sourceValue);
                break;
            case LoadTarget::HL:
                this->setCombined(RegisterPairs::HL, sourceValue);
                break;
            case LoadTarget::D8:
                this->memory.writeByte(this->regs.PC+1, sourceValue);
                toReturn += 1;
                break;
            case LoadTarget::D16:
                this->memory.writeWord(this->regs.PC+1, sourceValue16);
                toReturn += 2;
                break;
            case LoadTarget::HLINC:
                this->memory.writeByte(this->getCombined(RegisterPairs::HL), sourceValue);
                this->setCombined(RegisterPairs::HL, this->getCombined(RegisterPairs::HL)+1);
                break;
            case LoadTarget::HLDEC:
                this->memory.writeByte(this->getCombined(RegisterPairs::HL), sourceValue);
                this->setCombined(RegisterPairs::HL, this->getCombined(RegisterPairs::HL)-1);
                break;
            case LoadTarget::BCI:
                this->memory.writeByte(this->getCombined(RegisterPairs::BC), sourceValue);
                break;
            case LoadTarget::DEI:
                this->memory.writeByte(this->getCombined(RegisterPairs::DE), sourceValue);
                break;
            case LoadTarget::HLI:
                this->memory.writeByte(this->getCombined(RegisterPairs::HL), sourceValue);
                break;
            case LoadTarget::SP:
                this->regs.SP = sourceValue16;
                break;
            case LoadTarget::A16:
                this->memory.writeByte(this->regs.PC+1, (this->regs.SP & 0xFF));
                this->memory.writeByte(this->regs.PC+2, static_cast<uint8_t>(this->regs.SP >> 8));
                toReturn +=1;
                break;
            default:
                throw std::runtime_error("Not implemented target on load.");
            }

            return toReturn;
        }

        uint16_t LDHLSP(const LoadByte& loadbyte){
            int8_t value = this->memory.readByte(this->regs.PC+1);

            if (value >= 0){
                auto [result, overflow] = overflowSum(this->regs.SP, value);
                
                changeFlag(result == 0, 0, ((this->regs.SP & 0xF) + (value & 0xF)) > 0xF, overflow);
                
                this->regs.SP = result;
                this->setCombined(RegisterPairs::HL, result);
            }
            else {
                auto [result, overflow] = overflowSub(this->regs.SP, abs(value));

                changeFlag(result == 0, 0, ((this->regs.SP & 0xF) < (abs(value) & 0xF)) > 0xF, overflow);
                
                this->regs.SP = result;
                this->setCombined(RegisterPairs::HL, result);
            }

            return this->regs.PC+2;

        }

        uint16_t LDH(const LoadByte& loadbyte){

            uint8_t sourceValue = 0;
            uint16_t toReturn = this->regs.PC+1;

            switch (loadbyte.loadSource)
            {
            case LoadSource::A:
                sourceValue = this->regs.A;
                break;
            case LoadSource::C:
                sourceValue = this->memory.readByte(0xFF + static_cast<uint16_t>(this->regs.C));
                break;
            case LoadSource::D16:
                sourceValue = this->memory.readByte(0xFF + static_cast<uint16_t>(this->memory.readByte(this->regs.PC+1)));
                toReturn += 1;
                break;
            default:
                throw std::runtime_error("Invalid load source at LDH instruction.");
                break;
            }

            switch (loadbyte.loadTarget)
            {
            case LoadTarget::A:
                this->regs.A = sourceValue;
                break;
            case LoadTarget::C:
                this->memory.writeByte(0xFF+ static_cast<uint16_t>(this->regs.C), sourceValue);
                break;
            case LoadTarget::D16:
                this->memory.writeByte(0xFF + static_cast<uint16_t>(this->memory.readByte(this->regs.PC+1)), sourceValue);
                break;
            default:
                throw std::runtime_error("Invalid load target at LDH instruction.");
                break;
            }

            return toReturn;
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

        uint16_t POPAF(){

            uint8_t popped = this->memory.readByte(this->regs.SP);
            changeFlag((popped >> 7) & 1, (popped >> 6) & 1, (popped >> 5) & 1, (popped >> 4) & 1);
            this->regs.SP ++;
            this->regs.A = this->memory.readByte(this->regs.SP);
            this->regs.SP ++;

            return this->regs.PC+1;
        }

        uint16_t POP16(const RegisterPairs& pair){

            uint8_t popped = this->memory.readByte(this->regs.SP);
            switch (pair)
            {
            case RegisterPairs::BC:
                this->regs.C = popped;
                break;
            case RegisterPairs::DE:
                this->regs.E = popped;
                break;
            case RegisterPairs::HL:
                this->regs.E = popped;
                break;
            
            default:
                break;
            }

            this->regs.SP ++;
            popped = this->memory.readByte(this->regs.SP);
            
            switch (pair)
            {
            case RegisterPairs::BC:
                this->regs.B = popped;
                break;
            case RegisterPairs::DE:
                this->regs.D = popped;
                break;
            case RegisterPairs::HL:
                this->regs.H = popped;
                break;
            
            default:
                break;
            }

            return this->regs.PC+1;
        }


        uint16_t PUSH16(const RegisterPairs& pair){

            this->regs.SP --;
            switch (pair)
            {
            case RegisterPairs::AF:
                this->memory.writeByte(this->regs.SP, this->regs.A);
                break;
            case RegisterPairs::BC:
                this->memory.writeByte(this->regs.SP, this->regs.B);
                break;
            case RegisterPairs::DE:
                this->memory.writeByte(this->regs.SP, this->regs.D);
                break;
            case RegisterPairs::HL:
                this->memory.writeByte(this->regs.SP, this->regs.H);
                break;
            
            default:
                break;
            }
            this->regs.SP --;
            switch (pair)
            {
            case RegisterPairs::AF:
                this->memory.writeByte(this->regs.SP, this->regs.F);
                break;
            case RegisterPairs::BC:
                this->memory.writeByte(this->regs.SP, this->regs.C);
                break;
            case RegisterPairs::DE:
                this->memory.writeByte(this->regs.SP, this->regs.E);
                break;
            case RegisterPairs::HL:
                this->memory.writeByte(this->regs.SP, this->regs.L);
                break;
            
            default:
                break;
            }

            return this->regs.PC+1;
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

        uint16_t RETI(bool jump){
            if (jump) return this->POP();
            else return this->regs.PC+1;
        }

        uint16_t RST(const Vectors& vector){

            switch (vector)
            {
            case Vectors::V0x00:
                PUSH(this->regs.PC);
                return 0x00;
                break;
            case Vectors::V0x08:
                PUSH(this->regs.PC);
                return 0x08;
                break;
            case Vectors::V0x10:
                PUSH(this->regs.PC);
                return 0x10;
                break;
            case Vectors::V0x18:
                PUSH(this->regs.PC);
                return 0x18;
                break;
            case Vectors::V0x20:
                PUSH(this->regs.PC);
                return 0x20;
                break;
            case Vectors::V0x28:
                PUSH(this->regs.PC);
                return 0x28;
                break;
            case Vectors::V0x30:
                PUSH(this->regs.PC);
                return 0x30;
                break;
            case Vectors::V0x38:
                PUSH(this->regs.PC);
                return 0x38;
                break;
            default:
                throw std::runtime_error("Invalid vector at RST instruction.");
                break;
            }

            return this->regs.PC+1;
        }

        uint16_t CCF(){

            changeFlag((this->regs.F >> 7) & 1, false, true, ~((this->regs.F >> 4) & 1));

            return this->regs.PC+1;
        }

        uint16_t SCF(){

            changeFlag((this->regs.F >> 7) & 1, false, false, true);

            return this->regs.PC+1;
        }

        uint16_t DI(){
            this->IME = false;
            return this->regs.PC+1;
        }

        uint16_t EI(){
            this->IME = true;
            return this->regs.PC+1;
        }

        uint16_t HALT(){

            uint8_t pendingInterrupts = this->regs.IE & this->regs.IF & 0x1F;

            if (this->IME){
                this->isHalted = true;
            } else {
                if (pendingInterrupts == 0){

                    this->isHalted = true;
                }
                else {

                    this->isHalted = false;
                    this->haltBug = true;
                }
            }

            return this->regs.PC+1;
        }


        uint16_t DAA(){

            uint8_t adjustment = 0;
            bool carry = ((this->regs.F >> 4) & 1) == 1;

            if (((this->regs.F >> 6) & 1) == 1){
                if (((this->regs.F >> 5) & 1) == 1) adjustment += 0x6;
                if (((this->regs.F >> 4) & 1) == 1) adjustment += 0x60;

                this->regs.A -= adjustment;
            }
            else {
                if (((this->regs.F >> 5) & 1) == 1 || (this->regs.A & 0xF) > 0x9) adjustment += 0x6;
                if (((this->regs.F >> 4) & 1) == 1 || this->regs.A > 0x99) {
                    adjustment += 0x60;
                    carry = true;
                }

                this->regs.A += adjustment;
            }

            this->changeFlag(this->regs.A == 0, (this->regs.F >> 6) & 1, 0, carry);

            return this->regs.PC+1;
        }

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

        uint16_t execute(const Instruction& instruction){

            bool jumpCondition = false;
            uint16_t value = 0;
            uint16_t result = 0;

            switch (instruction.type)
            {

            case InstructionType::ADD:
                return ADD(instruction.arithmetic);

            case InstructionType::ADC:
                return ADC(instruction.arithmetic);

            case InstructionType::SUB:
                return SUB(instruction.arithmetic);

            case InstructionType::SBC:
                return SBC(instruction.arithmetic);

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

            if (this->isHalted){
                uint8_t activeInterrupts = this->regs.IE & this->regs.IF & 0x1F;

                if (activeInterrupts > 0){
                    this->isHalted = false;
                }
                else {
                    return;
                }
            }

            uint16_t instructionByte = this->memory.readByte(this->regs.PC);
            bool prefixed = (instructionByte == 0xCB);

            if (prefixed){
                instructionByte = this->memory.readByte(this->regs.PC+1);
            }

            auto instructionOpt = Instruction::getFromByte(instructionByte, prefixed);

            if (instructionOpt.has_value()){
                if (this->haltBug){
                    this->haltBug = false;

                    execute(instructionOpt.value());
                } else {
                    this->regs.PC = execute(instructionOpt.value());
                }
            }
            else {
                throw std::runtime_error("Unkown instruction: 0x" + std::to_string(instructionByte));
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