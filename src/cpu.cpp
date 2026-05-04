#include "cpu.hpp"

#ifdef DEBUG

std::ofstream logFile("./emulator.log");

#endif
#ifdef BLARGGTESTROMLOG

std::ofstream logFile("./emulator.log");

#endif

CPU::CPU(SystemBus* systemBus) : systemBus(systemBus){
    this->isStopped = false;
    this->isHalted = false;
    this->haltBug = false;
    this->IME = false;
    this->booted = false;
    this->booting = false;
    this->loadedROM = false;
    this->loadingROM = false;
}

void CPU::linkPointers(GPU* gpu, MemoryBus* memoryBus, CycleCounter* cycleCounter){

    this->gpu = gpu;
    this->memory = memoryBus;
    this->clock = cycleCounter;

}

void CPU::executeASM(){
    
    this->step();

    #ifdef DEBUG

    std::cout << "A register: " << std::bitset<8>(this->regs.A) << std::endl;
    std::cout << "B register: " << std::bitset<8>(this->regs.B) << std::endl;
    std::cout << "C register: " << std::bitset<8>(this->regs.C) << std::endl;
    std::cout << "D register: " << std::bitset<8>(this->regs.D) << std::endl;
    std::cout << "E register: " << std::bitset<8>(this->regs.E) << std::endl;
    std::cout << "H register: " << std::bitset<8>(this->regs.H) << std::endl;
    std::cout << "L register: " << std::bitset<8>(this->regs.L) << std::endl;
    std::cout << "SP register: " << std::bitset<16>(this->regs.SP) << std::endl;
    std::cout << "PC register: " << std::bitset<16>(this->regs.PC) << std::endl;
    std::cout << "F register: " << std::bitset<8>(this->regs.F) << std::endl;
    std::cout << std::endl;

    #endif
}

enum class CPU::HardwareRegisters {
    P1,
    JOYP,
    SB,
    SC,
    DIV,
    TIMA,
    TMA,
    TAC,
    IF,
    NR10,
    NR11,
    NR12,
    NR13,
    NR14,
    NR21,
    NR22,
    NR23,
    NR24,
    NR30,
    NR31,
    NR32,
    NR33,
    NR34,
    NR41,
    NR42,
    NR43,
    NR44,
    NR50,
    NR51,
    NR52,
    WRAM,
    LCDC,
    STAT,
    SCY,
    SCX,
    LY,
    LYC,
    DMA,
    BGP,
    OBP0,
    OBP1,
    WY,
    WX,
    BANK,
    IE
};

uint8_t CPU::getHardwareRegister(const HardwareRegisters& hardwareRegister){

    switch(hardwareRegister){

        case HardwareRegisters::P1:
        case HardwareRegisters::JOYP:
            return this->memory->readByte(0xFF00);
        break;
        case HardwareRegisters::SB:
            return this->memory->readByte(0xFF01);
        case HardwareRegisters::SC:
            return this->memory->readByte(0xFF02);
        case HardwareRegisters::DIV:
            return this->memory->readByte(0xFF04);
        case HardwareRegisters::TIMA:
            return this->memory->readByte(0xFF05);
        case HardwareRegisters::TMA:
            return this->memory->readByte(0xFF06);
        case HardwareRegisters::TAC:
            return this->memory->readByte(0xFF07);
        case HardwareRegisters::IF:
            return this->memory->readByte(0xFF0F);
        case HardwareRegisters::NR10:
            return this->memory->readByte(0xFF10);
        case HardwareRegisters::NR11:
            return this->memory->readByte(0xFF11);
        case HardwareRegisters::NR12:
            return this->memory->readByte(0xFF12);
        case HardwareRegisters::NR13:
            return this->memory->readByte(0xFF13);
        case HardwareRegisters::NR14:
            return this->memory->readByte(0xFF14);
        case HardwareRegisters::NR21:
            return this->memory->readByte(0xFF16);
        case HardwareRegisters::NR22:
            return this->memory->readByte(0xFF17);
        case HardwareRegisters::NR23:
            return this->memory->readByte(0xFF18);
        case HardwareRegisters::NR24:
            return this->memory->readByte(0xFF19);
        case HardwareRegisters::NR30:
            return this->memory->readByte(0xFF1A);
        case HardwareRegisters::NR31:
            return this->memory->readByte(0xFF1B);
        case HardwareRegisters::NR32:
            return this->memory->readByte(0xFF1C);
        case HardwareRegisters::NR33:
            return this->memory->readByte(0xFF1D);
        case HardwareRegisters::NR34:
            return this->memory->readByte(0xFF1E);
        case HardwareRegisters::NR41:
            return this->memory->readByte(0xFF20);
        case HardwareRegisters::NR42:
            return this->memory->readByte(0xFF21);
        case HardwareRegisters::NR43:
            return this->memory->readByte(0xFF22);
        case HardwareRegisters::NR44:
            return this->memory->readByte(0xFF23);
        case HardwareRegisters::NR50:
            return this->memory->readByte(0xFF24);
        case HardwareRegisters::NR51:
            return this->memory->readByte(0xFF25);
        case HardwareRegisters::NR52:
            return this->memory->readByte(0xFF26);
        case HardwareRegisters::LCDC:
            return this->memory->readByte(0xFF40);
        case HardwareRegisters::STAT:
            return this->memory->readByte(0xFF41);
        case HardwareRegisters::SCY:
            return this->memory->readByte(0xFF42);
        case HardwareRegisters::SCX:
            return this->memory->readByte(0xFF43);
        case HardwareRegisters::LY:
            return this->memory->readByte(0xFF44);
        case HardwareRegisters::LYC:
            return this->memory->readByte(0xFF45);
        case HardwareRegisters::DMA:
            return this->memory->readByte(0xFF46);
        case HardwareRegisters::WY:
            return this->memory->readByte(0xFF4A);
        case HardwareRegisters::WX:
            return this->memory->readByte(0xFF4B);
        case HardwareRegisters::BANK:
            return this->memory->readByte(0xFF50);
        case HardwareRegisters::IE:
            return this->memory->readByte(0xFFFF);
        default:
            throw std::runtime_error("Invalid hardware register.");
    }

    return 0;
}

void CPU::setPostBoot(){

    if (!this->booting){
        this->booting = true;
        
        //Set every memory address to 0x0000;
        for (int i = 0; i < 0x10000; i++){
            memory->writeByte(i, 0x0000);
        }

        this->regs.A = 0x01;
        this->regs.B = 0x00;
        this->regs.C = 0x13;
        this->regs.D = 0x00;
        this->regs.E = 0xD8;
        this->regs.H = 0x01;
        this->regs.L = 0x4D;
        this->regs.SP = 0xFFFE;
        this->regs.PC = 0x100;
        this->regs.F = 0xB0;


        this->booted = true;
    }

}

void CPU::loadROM(std::string romPath){

    if (!this->loadingROM){
        this->loadingROM = true;
        std::ifstream rom(romPath, std::ios::binary);

        uint8_t byte;
        uint16_t i = 0;

        
        while (rom.read(reinterpret_cast<char*>(&byte), sizeof(byte))){
            //std::cout << byte << std::endl;
            this->memory->writeByte(i, byte);
            //std::cout << "Hex value passed: 0x" << std::hex << (int)memory->readByte(i) << std::endl;
            i++;
            
        }

        rom.close();

        this->loadedROM = true;
    }
}

enum class CPU::Vectors {V0x00, V0x08, V0x10, V0x18, V0x20, V0x28, V0x30, V0x38};

enum class CPU::RegisterPairs : uint16_t { AF, BC, DE, HL };

enum class CPU::InstructionType { 
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

enum class CPU::Target {A, B, C, D, E, H, L, F, AF, BC, DE, HL, SP, PC};

enum class CPU::ArithmeticTarget {A, B, C, D, E, H, L, F, AF, BC, DE, HL, SP, PC, HLI};

enum class CPU::ArithmeticSource {A, B, C, D, E, H, L, F, HLI, D8, AF, BC, DE, HL, SP, PC};

enum class CPU::BitwiseSource {A, B, C, D, E, F, H, L, HLI, D8};

enum class CPU::BitFlagSource {A, B, C, D, E, F, H, L, HLI};

enum class CPU::JumpTest {NotZero, NotCarry, Zero, Carry, Always};

enum class CPU::LoadTarget { A, B, C, D, E, H, L, D8, D16, BCI, DEI, HLI, BC, DE, HL, HLDEC, HLINC, SP, A16, A16I};

enum class CPU::LoadSource { A, B, C, D, E, H, L, D8, BCI, DEI, HLI, D16, BC, DE, HL, HLDEC, HLINC, SP, A16I};

enum class CPU::StackTarget { BC, DE, HL };

enum class CPU::RotateTarget {A, B, C, D, E, H, L, HLI};

struct CPU::Arithmetic{
    ArithmeticTarget arithmeticTarget;
    ArithmeticSource arithmeticSource;
};

struct CPU::LoadByte {
    LoadTarget loadTarget;
    LoadSource loadSource;
};

struct CPU::BitFlag {
    unsigned int u3: 3;
    BitFlagSource bitFlagSource;
};

struct CPU::Instruction{
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
    uint8_t mcycles;

    Instruction(InstructionType type, Arithmetic arithmetic, uint8_t mcycles) : type(type), arithmetic(arithmetic), mcycles(mcycles) {}
    Instruction(InstructionType type, JumpTest jumpTest, uint8_t mcycles) : type(type), jumpTest(jumpTest), mcycles(mcycles) {}
    Instruction(InstructionType type, StackTarget stackTarget, uint8_t mcycles) : type(type), stackTarget(stackTarget), mcycles(mcycles) {}
    Instruction(InstructionType type, LoadByte loadByte, uint8_t mcycles) : type(type), loadByte(loadByte), mcycles(mcycles) {}
    Instruction(InstructionType type, BitwiseSource bitSource, uint8_t mcycles) : type(type), bitSource(bitSource), mcycles(mcycles) {}
    Instruction(InstructionType type, BitFlag bitFlag, uint8_t mcycles) : type(type), bitFlag(bitFlag), mcycles(mcycles) {}
    Instruction(InstructionType type, RegisterPairs pair, uint8_t mcycles) : type(type), pair(pair), mcycles(mcycles) {}
    Instruction(InstructionType type, Vectors vector, uint8_t mcycles) : type(type), vector(vector), mcycles(mcycles) {}
    Instruction(InstructionType type, RotateTarget rotateTarget, uint8_t mcycles) : type(type), rotateTarget(rotateTarget), mcycles(mcycles) {}
    Instruction(InstructionType type, uint8_t mcycles): type(type), mcycles(mcycles) {}

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
            case 0x00: return Instruction{InstructionType::RLC, RotateTarget::B, 2};
            case 0x01: return Instruction{InstructionType::RLC, RotateTarget::C, 2};
            case 0x02: return Instruction{InstructionType::RLC, RotateTarget::D, 2};
            case 0x03: return Instruction{InstructionType::RLC, RotateTarget::E, 2};
            case 0x04: return Instruction{InstructionType::RLC, RotateTarget::H, 2};
            case 0x05: return Instruction{InstructionType::RLC, RotateTarget::L, 2};
            case 0x06: return Instruction{InstructionType::RLC, RotateTarget::HLI, 4};
            case 0x07: return Instruction{InstructionType::RLC, RotateTarget::A, 2};
            case 0x08: return Instruction{InstructionType::RRC, RotateTarget::B, 2};
            case 0x09: return Instruction{InstructionType::RRC, RotateTarget::C, 2};
            case 0x0A: return Instruction{InstructionType::RRC, RotateTarget::D, 2};
            case 0x0B: return Instruction{InstructionType::RRC, RotateTarget::E, 2};
            case 0x0C: return Instruction{InstructionType::RRC, RotateTarget::H, 2};
            case 0x0D: return Instruction{InstructionType::RRC, RotateTarget::L, 2};
            case 0x0E: return Instruction{InstructionType::RRC, RotateTarget::HLI, 4};
            case 0x0F: return Instruction{InstructionType::RRC, RotateTarget::A, 2};

            case 0x10: return Instruction{InstructionType::RL, RotateTarget::B, 2};
            case 0x11: return Instruction{InstructionType::RL, RotateTarget::C, 2};
            case 0x12: return Instruction{InstructionType::RL, RotateTarget::D, 2};
            case 0x13: return Instruction{InstructionType::RL, RotateTarget::E, 2};
            case 0x14: return Instruction{InstructionType::RL, RotateTarget::H, 2};
            case 0x15: return Instruction{InstructionType::RL, RotateTarget::L, 2};
            case 0x16: return Instruction{InstructionType::RL, RotateTarget::HLI, 4};
            case 0x17: return Instruction{InstructionType::RL, RotateTarget::A, 2};
            case 0x18: return Instruction{InstructionType::RR, RotateTarget::B, 2};
            case 0x19: return Instruction{InstructionType::RR, RotateTarget::C, 2};
            case 0x1A: return Instruction{InstructionType::RR, RotateTarget::D, 2};
            case 0x1B: return Instruction{InstructionType::RR, RotateTarget::E, 2};
            case 0x1C: return Instruction{InstructionType::RR, RotateTarget::H, 2};
            case 0x1D: return Instruction{InstructionType::RR, RotateTarget::L, 2};
            case 0x1E: return Instruction{InstructionType::RR, RotateTarget::HLI, 4};
            case 0x1F: return Instruction{InstructionType::RR, RotateTarget::A, 2};

            case 0x20: return Instruction{InstructionType::SLA, RotateTarget::B, 2};
            case 0x21: return Instruction{InstructionType::SLA, RotateTarget::C, 2};
            case 0x22: return Instruction{InstructionType::SLA, RotateTarget::D, 2};
            case 0x23: return Instruction{InstructionType::SLA, RotateTarget::E, 2};
            case 0x24: return Instruction{InstructionType::SLA, RotateTarget::H, 2};
            case 0x25: return Instruction{InstructionType::SLA, RotateTarget::L, 2};
            case 0x26: return Instruction{InstructionType::SLA, RotateTarget::HLI, 4};
            case 0x27: return Instruction{InstructionType::SLA, RotateTarget::A, 2};
            case 0x28: return Instruction{InstructionType::SRA, RotateTarget::B, 2};
            case 0x29: return Instruction{InstructionType::SRA, RotateTarget::C, 2};
            case 0x2A: return Instruction{InstructionType::SRA, RotateTarget::D, 2};
            case 0x2B: return Instruction{InstructionType::SRA, RotateTarget::E, 2};
            case 0x2C: return Instruction{InstructionType::SRA, RotateTarget::H, 2};
            case 0x2D: return Instruction{InstructionType::SRA, RotateTarget::L, 2};
            case 0x2E: return Instruction{InstructionType::SRA, RotateTarget::HLI, 4};
            case 0x2F: return Instruction{InstructionType::SRA, RotateTarget::A, 2};

            case 0x30: return Instruction{InstructionType::SWAP, RotateTarget::B, 2};
            case 0x31: return Instruction{InstructionType::SWAP, RotateTarget::C, 2};
            case 0x32: return Instruction{InstructionType::SWAP, RotateTarget::D, 2};
            case 0x33: return Instruction{InstructionType::SWAP, RotateTarget::E, 2};
            case 0x34: return Instruction{InstructionType::SWAP, RotateTarget::H, 2};
            case 0x35: return Instruction{InstructionType::SWAP, RotateTarget::L, 2};
            case 0x36: return Instruction{InstructionType::SWAP, RotateTarget::HLI, 4};
            case 0x37: return Instruction{InstructionType::SWAP, RotateTarget::A, 2};
            case 0x38: return Instruction{InstructionType::SRL, RotateTarget::B, 2};
            case 0x39: return Instruction{InstructionType::SRL, RotateTarget::C, 2};
            case 0x3A: return Instruction{InstructionType::SRL, RotateTarget::D, 2};
            case 0x3B: return Instruction{InstructionType::SRL, RotateTarget::E, 2};
            case 0x3C: return Instruction{InstructionType::SRL, RotateTarget::H, 2};
            case 0x3D: return Instruction{InstructionType::SRL, RotateTarget::L, 2};
            case 0x3E: return Instruction{InstructionType::SRL, RotateTarget::HLI, 4};
            case 0x3F: return Instruction{InstructionType::SRL, RotateTarget::A, 2};

            case 0x40: return Instruction{InstructionType::BIT, BitFlag{0,BitFlagSource::B}, 2};
            case 0x41: return Instruction{InstructionType::BIT, BitFlag{0,BitFlagSource::C}, 2};
            case 0x42: return Instruction{InstructionType::BIT, BitFlag{0,BitFlagSource::D}, 2};
            case 0x43: return Instruction{InstructionType::BIT, BitFlag{0,BitFlagSource::E}, 2};
            case 0x44: return Instruction{InstructionType::BIT, BitFlag{0,BitFlagSource::H}, 2};
            case 0x45: return Instruction{InstructionType::BIT, BitFlag{0,BitFlagSource::L}, 2};
            case 0x46: return Instruction{InstructionType::BIT, BitFlag{0,BitFlagSource::HLI}, 3};
            case 0x47: return Instruction{InstructionType::BIT, BitFlag{0,BitFlagSource::A}, 2};
            case 0x48: return Instruction{InstructionType::BIT, BitFlag{1,BitFlagSource::B}, 2};
            case 0x49: return Instruction{InstructionType::BIT, BitFlag{1,BitFlagSource::C}, 2};
            case 0x4A: return Instruction{InstructionType::BIT, BitFlag{1,BitFlagSource::D}, 2};
            case 0x4B: return Instruction{InstructionType::BIT, BitFlag{1,BitFlagSource::E}, 2};
            case 0x4C: return Instruction{InstructionType::BIT, BitFlag{1,BitFlagSource::H}, 2};
            case 0x4D: return Instruction{InstructionType::BIT, BitFlag{1,BitFlagSource::L}, 2};
            case 0x4E: return Instruction{InstructionType::BIT, BitFlag{1,BitFlagSource::HLI}, 3};
            case 0x4F: return Instruction{InstructionType::BIT, BitFlag{1,BitFlagSource::A}, 2};

            case 0x50: return Instruction{InstructionType::BIT, BitFlag{2,BitFlagSource::B}, 2};
            case 0x51: return Instruction{InstructionType::BIT, BitFlag{2,BitFlagSource::C}, 2};
            case 0x52: return Instruction{InstructionType::BIT, BitFlag{2,BitFlagSource::D}, 2};
            case 0x53: return Instruction{InstructionType::BIT, BitFlag{2,BitFlagSource::E}, 2};
            case 0x54: return Instruction{InstructionType::BIT, BitFlag{2,BitFlagSource::H}, 2};
            case 0x55: return Instruction{InstructionType::BIT, BitFlag{2,BitFlagSource::L}, 2};
            case 0x56: return Instruction{InstructionType::BIT, BitFlag{2,BitFlagSource::HLI}, 3};
            case 0x57: return Instruction{InstructionType::BIT, BitFlag{2,BitFlagSource::A}, 2};
            case 0x58: return Instruction{InstructionType::BIT, BitFlag{3,BitFlagSource::B}, 2};
            case 0x59: return Instruction{InstructionType::BIT, BitFlag{3,BitFlagSource::C}, 2};
            case 0x5A: return Instruction{InstructionType::BIT, BitFlag{3,BitFlagSource::D}, 2};
            case 0x5B: return Instruction{InstructionType::BIT, BitFlag{3,BitFlagSource::E}, 2};
            case 0x5C: return Instruction{InstructionType::BIT, BitFlag{3,BitFlagSource::H}, 2};
            case 0x5D: return Instruction{InstructionType::BIT, BitFlag{3,BitFlagSource::L}, 2};
            case 0x5E: return Instruction{InstructionType::BIT, BitFlag{3,BitFlagSource::HLI}, 3};
            case 0x5F: return Instruction{InstructionType::BIT, BitFlag{3,BitFlagSource::A}, 2};

            case 0x60: return Instruction{InstructionType::BIT, BitFlag{4,BitFlagSource::B}, 2};
            case 0x61: return Instruction{InstructionType::BIT, BitFlag{4,BitFlagSource::C}, 2};
            case 0x62: return Instruction{InstructionType::BIT, BitFlag{4,BitFlagSource::D}, 2};
            case 0x63: return Instruction{InstructionType::BIT, BitFlag{4,BitFlagSource::E}, 2};
            case 0x64: return Instruction{InstructionType::BIT, BitFlag{4,BitFlagSource::H}, 2};
            case 0x65: return Instruction{InstructionType::BIT, BitFlag{4,BitFlagSource::L}, 2};
            case 0x66: return Instruction{InstructionType::BIT, BitFlag{4,BitFlagSource::HLI}, 3};
            case 0x67: return Instruction{InstructionType::BIT, BitFlag{4,BitFlagSource::A}, 2};
            case 0x68: return Instruction{InstructionType::BIT, BitFlag{5,BitFlagSource::B}, 2};
            case 0x69: return Instruction{InstructionType::BIT, BitFlag{5,BitFlagSource::C}, 2};
            case 0x6A: return Instruction{InstructionType::BIT, BitFlag{5,BitFlagSource::D}, 2};
            case 0x6B: return Instruction{InstructionType::BIT, BitFlag{5,BitFlagSource::E}, 2};
            case 0x6C: return Instruction{InstructionType::BIT, BitFlag{5,BitFlagSource::H}, 2};
            case 0x6D: return Instruction{InstructionType::BIT, BitFlag{5,BitFlagSource::L}, 2};
            case 0x6E: return Instruction{InstructionType::BIT, BitFlag{5,BitFlagSource::HLI}, 3};
            case 0x6F: return Instruction{InstructionType::BIT, BitFlag{5,BitFlagSource::A}, 2};

            case 0x70: return Instruction{InstructionType::BIT, BitFlag{6,BitFlagSource::B}, 2};
            case 0x71: return Instruction{InstructionType::BIT, BitFlag{6,BitFlagSource::C}, 2};
            case 0x72: return Instruction{InstructionType::BIT, BitFlag{6,BitFlagSource::D}, 2};
            case 0x73: return Instruction{InstructionType::BIT, BitFlag{6,BitFlagSource::E}, 2};
            case 0x74: return Instruction{InstructionType::BIT, BitFlag{6,BitFlagSource::H}, 2};
            case 0x75: return Instruction{InstructionType::BIT, BitFlag{6,BitFlagSource::L}, 2};
            case 0x76: return Instruction{InstructionType::BIT, BitFlag{6,BitFlagSource::HLI}, 3};
            case 0x77: return Instruction{InstructionType::BIT, BitFlag{6,BitFlagSource::A}, 2};
            case 0x78: return Instruction{InstructionType::BIT, BitFlag{7,BitFlagSource::B}, 2};
            case 0x79: return Instruction{InstructionType::BIT, BitFlag{7,BitFlagSource::C}, 2};
            case 0x7A: return Instruction{InstructionType::BIT, BitFlag{7,BitFlagSource::D}, 2};
            case 0x7B: return Instruction{InstructionType::BIT, BitFlag{7,BitFlagSource::E}, 2};
            case 0x7C: return Instruction{InstructionType::BIT, BitFlag{7,BitFlagSource::H}, 2};
            case 0x7D: return Instruction{InstructionType::BIT, BitFlag{7,BitFlagSource::L}, 2};
            case 0x7E: return Instruction{InstructionType::BIT, BitFlag{7,BitFlagSource::HLI}, 3};
            case 0x7F: return Instruction{InstructionType::BIT, BitFlag{7,BitFlagSource::A}, 2};

            case 0x80: return Instruction{InstructionType::RES, BitFlag{0,BitFlagSource::B}, 2};
            case 0x81: return Instruction{InstructionType::RES, BitFlag{0,BitFlagSource::C}, 2};
            case 0x82: return Instruction{InstructionType::RES, BitFlag{0,BitFlagSource::D}, 2};
            case 0x83: return Instruction{InstructionType::RES, BitFlag{0,BitFlagSource::E}, 2};
            case 0x84: return Instruction{InstructionType::RES, BitFlag{0,BitFlagSource::H}, 2};
            case 0x85: return Instruction{InstructionType::RES, BitFlag{0,BitFlagSource::L}, 2};
            case 0x86: return Instruction{InstructionType::RES, BitFlag{0,BitFlagSource::HLI}, 4};
            case 0x87: return Instruction{InstructionType::RES, BitFlag{0,BitFlagSource::A}, 2};
            case 0x88: return Instruction{InstructionType::RES, BitFlag{1,BitFlagSource::B}, 2};
            case 0x89: return Instruction{InstructionType::RES, BitFlag{1,BitFlagSource::C}, 2};
            case 0x8A: return Instruction{InstructionType::RES, BitFlag{1,BitFlagSource::D}, 2};
            case 0x8B: return Instruction{InstructionType::RES, BitFlag{1,BitFlagSource::E}, 2};
            case 0x8C: return Instruction{InstructionType::RES, BitFlag{1,BitFlagSource::H}, 2};
            case 0x8D: return Instruction{InstructionType::RES, BitFlag{1,BitFlagSource::L}, 2};
            case 0x8E: return Instruction{InstructionType::RES, BitFlag{1,BitFlagSource::HLI}, 4};
            case 0x8F: return Instruction{InstructionType::RES, BitFlag{1,BitFlagSource::A}, 2};

            case 0x90: return Instruction{InstructionType::RES, BitFlag{2,BitFlagSource::B}, 2};
            case 0x91: return Instruction{InstructionType::RES, BitFlag{2,BitFlagSource::C}, 2};
            case 0x92: return Instruction{InstructionType::RES, BitFlag{2,BitFlagSource::D}, 2};
            case 0x93: return Instruction{InstructionType::RES, BitFlag{2,BitFlagSource::E}, 2};
            case 0x94: return Instruction{InstructionType::RES, BitFlag{2,BitFlagSource::H}, 2};
            case 0x95: return Instruction{InstructionType::RES, BitFlag{2,BitFlagSource::L}, 2};
            case 0x96: return Instruction{InstructionType::RES, BitFlag{2,BitFlagSource::HLI}, 4};
            case 0x97: return Instruction{InstructionType::RES, BitFlag{2,BitFlagSource::A}, 2};
            case 0x98: return Instruction{InstructionType::RES, BitFlag{3,BitFlagSource::B}, 2};
            case 0x99: return Instruction{InstructionType::RES, BitFlag{3,BitFlagSource::C}, 2};
            case 0x9A: return Instruction{InstructionType::RES, BitFlag{3,BitFlagSource::D}, 2};
            case 0x9B: return Instruction{InstructionType::RES, BitFlag{3,BitFlagSource::E}, 2};
            case 0x9C: return Instruction{InstructionType::RES, BitFlag{3,BitFlagSource::H}, 2};
            case 0x9D: return Instruction{InstructionType::RES, BitFlag{3,BitFlagSource::L}, 2};
            case 0x9E: return Instruction{InstructionType::RES, BitFlag{3,BitFlagSource::HLI},4};
            case 0x9F: return Instruction{InstructionType::RES, BitFlag{3,BitFlagSource::A}, 2};

            case 0xA0: return Instruction{InstructionType::RES, BitFlag{4,BitFlagSource::B}, 2};
            case 0xA1: return Instruction{InstructionType::RES, BitFlag{4,BitFlagSource::C}, 2};
            case 0xA2: return Instruction{InstructionType::RES, BitFlag{4,BitFlagSource::D}, 2};
            case 0xA3: return Instruction{InstructionType::RES, BitFlag{4,BitFlagSource::E}, 2};
            case 0xA4: return Instruction{InstructionType::RES, BitFlag{4,BitFlagSource::H}, 2};
            case 0xA5: return Instruction{InstructionType::RES, BitFlag{4,BitFlagSource::L}, 2};
            case 0xA6: return Instruction{InstructionType::RES, BitFlag{4,BitFlagSource::HLI},4};
            case 0xA7: return Instruction{InstructionType::RES, BitFlag{4,BitFlagSource::A}, 2};
            case 0xA8: return Instruction{InstructionType::RES, BitFlag{5,BitFlagSource::B}, 2};
            case 0xA9: return Instruction{InstructionType::RES, BitFlag{5,BitFlagSource::C}, 2};
            case 0xAA: return Instruction{InstructionType::RES, BitFlag{5,BitFlagSource::D}, 2};
            case 0xAB: return Instruction{InstructionType::RES, BitFlag{5,BitFlagSource::E}, 2};
            case 0xAC: return Instruction{InstructionType::RES, BitFlag{5,BitFlagSource::H}, 2};
            case 0xAD: return Instruction{InstructionType::RES, BitFlag{5,BitFlagSource::L}, 2};
            case 0xAE: return Instruction{InstructionType::RES, BitFlag{5,BitFlagSource::HLI},4};
            case 0xAF: return Instruction{InstructionType::RES, BitFlag{5,BitFlagSource::A}, 2};

            case 0xB0: return Instruction{InstructionType::RES, BitFlag{6,BitFlagSource::B}, 2};
            case 0xB1: return Instruction{InstructionType::RES, BitFlag{6,BitFlagSource::C}, 2};
            case 0xB2: return Instruction{InstructionType::RES, BitFlag{6,BitFlagSource::D}, 2};
            case 0xB3: return Instruction{InstructionType::RES, BitFlag{6,BitFlagSource::E}, 2};
            case 0xB4: return Instruction{InstructionType::RES, BitFlag{6,BitFlagSource::H}, 2};
            case 0xB5: return Instruction{InstructionType::RES, BitFlag{6,BitFlagSource::L}, 2};
            case 0xB6: return Instruction{InstructionType::RES, BitFlag{6,BitFlagSource::HLI},4};
            case 0xB7: return Instruction{InstructionType::RES, BitFlag{6,BitFlagSource::A}, 2};
            case 0xB8: return Instruction{InstructionType::RES, BitFlag{7,BitFlagSource::B}, 2};
            case 0xB9: return Instruction{InstructionType::RES, BitFlag{7,BitFlagSource::C}, 2};
            case 0xBA: return Instruction{InstructionType::RES, BitFlag{7,BitFlagSource::D}, 2};
            case 0xBB: return Instruction{InstructionType::RES, BitFlag{7,BitFlagSource::E}, 2};
            case 0xBC: return Instruction{InstructionType::RES, BitFlag{7,BitFlagSource::H}, 2};
            case 0xBD: return Instruction{InstructionType::RES, BitFlag{7,BitFlagSource::L}, 2};
            case 0xBE: return Instruction{InstructionType::RES, BitFlag{7,BitFlagSource::HLI},4};
            case 0xBF: return Instruction{InstructionType::RES, BitFlag{7,BitFlagSource::A}, 2};

            case 0xC0: return Instruction{InstructionType::SET, BitFlag{0,BitFlagSource::B}, 2};
            case 0xC1: return Instruction{InstructionType::SET, BitFlag{0,BitFlagSource::C}, 2};
            case 0xC2: return Instruction{InstructionType::SET, BitFlag{0,BitFlagSource::D}, 2};
            case 0xC3: return Instruction{InstructionType::SET, BitFlag{0,BitFlagSource::E}, 2};
            case 0xC4: return Instruction{InstructionType::SET, BitFlag{0,BitFlagSource::H}, 2};
            case 0xC5: return Instruction{InstructionType::SET, BitFlag{0,BitFlagSource::L}, 2};
            case 0xC6: return Instruction{InstructionType::SET, BitFlag{0,BitFlagSource::HLI},4};
            case 0xC7: return Instruction{InstructionType::SET, BitFlag{0,BitFlagSource::A}, 2};
            case 0xC8: return Instruction{InstructionType::SET, BitFlag{1,BitFlagSource::B}, 2};
            case 0xC9: return Instruction{InstructionType::SET, BitFlag{1,BitFlagSource::C}, 2};
            case 0xCA: return Instruction{InstructionType::SET, BitFlag{1,BitFlagSource::D}, 2};
            case 0xCB: return Instruction{InstructionType::SET, BitFlag{1,BitFlagSource::E}, 2};
            case 0xCC: return Instruction{InstructionType::SET, BitFlag{1,BitFlagSource::H}, 2};
            case 0xCD: return Instruction{InstructionType::SET, BitFlag{1,BitFlagSource::L}, 2};
            case 0xCE: return Instruction{InstructionType::SET, BitFlag{1,BitFlagSource::HLI},4};
            case 0xCF: return Instruction{InstructionType::SET, BitFlag{1,BitFlagSource::A}, 2};

            case 0xD0: return Instruction{InstructionType::SET, BitFlag{2,BitFlagSource::B}, 2};
            case 0xD1: return Instruction{InstructionType::SET, BitFlag{2,BitFlagSource::C}, 2};
            case 0xD2: return Instruction{InstructionType::SET, BitFlag{2,BitFlagSource::D}, 2};
            case 0xD3: return Instruction{InstructionType::SET, BitFlag{2,BitFlagSource::E}, 2};
            case 0xD4: return Instruction{InstructionType::SET, BitFlag{2,BitFlagSource::H}, 2};
            case 0xD5: return Instruction{InstructionType::SET, BitFlag{2,BitFlagSource::L}, 2};
            case 0xD6: return Instruction{InstructionType::SET, BitFlag{2,BitFlagSource::HLI}, 4};
            case 0xD7: return Instruction{InstructionType::SET, BitFlag{2,BitFlagSource::A}, 2};
            case 0xD8: return Instruction{InstructionType::SET, BitFlag{3,BitFlagSource::B}, 2};
            case 0xD9: return Instruction{InstructionType::SET, BitFlag{3,BitFlagSource::C}, 2};
            case 0xDA: return Instruction{InstructionType::SET, BitFlag{3,BitFlagSource::D}, 2};
            case 0xDB: return Instruction{InstructionType::SET, BitFlag{3,BitFlagSource::E}, 2};
            case 0xDC: return Instruction{InstructionType::SET, BitFlag{3,BitFlagSource::H}, 2};
            case 0xDD: return Instruction{InstructionType::SET, BitFlag{3,BitFlagSource::L}, 2};
            case 0xDE: return Instruction{InstructionType::SET, BitFlag{3,BitFlagSource::HLI}, 4};
            case 0xDF: return Instruction{InstructionType::SET, BitFlag{3,BitFlagSource::A}, 2};

            case 0xE0: return Instruction{InstructionType::SET, BitFlag{4,BitFlagSource::B}, 2};
            case 0xE1: return Instruction{InstructionType::SET, BitFlag{4,BitFlagSource::C}, 2};
            case 0xE2: return Instruction{InstructionType::SET, BitFlag{4,BitFlagSource::D}, 2};
            case 0xE3: return Instruction{InstructionType::SET, BitFlag{4,BitFlagSource::E}, 2};
            case 0xE4: return Instruction{InstructionType::SET, BitFlag{4,BitFlagSource::H}, 2};
            case 0xE5: return Instruction{InstructionType::SET, BitFlag{4,BitFlagSource::L}, 2};
            case 0xE6: return Instruction{InstructionType::SET, BitFlag{4,BitFlagSource::HLI}, 4};
            case 0xE7: return Instruction{InstructionType::SET, BitFlag{4,BitFlagSource::A}, 2};
            case 0xE8: return Instruction{InstructionType::SET, BitFlag{5,BitFlagSource::B}, 2};
            case 0xE9: return Instruction{InstructionType::SET, BitFlag{5,BitFlagSource::C}, 2};
            case 0xEA: return Instruction{InstructionType::SET, BitFlag{5,BitFlagSource::D}, 2};
            case 0xEB: return Instruction{InstructionType::SET, BitFlag{5,BitFlagSource::E}, 2};
            case 0xEC: return Instruction{InstructionType::SET, BitFlag{5,BitFlagSource::H}, 2};
            case 0xED: return Instruction{InstructionType::SET, BitFlag{5,BitFlagSource::L}, 2};
            case 0xEE: return Instruction{InstructionType::SET, BitFlag{5,BitFlagSource::HLI}, 4};
            case 0xEF: return Instruction{InstructionType::SET, BitFlag{5,BitFlagSource::A}, 2};

            case 0xF0: return Instruction{InstructionType::SET, BitFlag{6,BitFlagSource::B}, 2};
            case 0xF1: return Instruction{InstructionType::SET, BitFlag{6,BitFlagSource::C}, 2};
            case 0xF2: return Instruction{InstructionType::SET, BitFlag{6,BitFlagSource::D}, 2};
            case 0xF3: return Instruction{InstructionType::SET, BitFlag{6,BitFlagSource::E}, 2};
            case 0xF4: return Instruction{InstructionType::SET, BitFlag{6,BitFlagSource::H}, 2};
            case 0xF5: return Instruction{InstructionType::SET, BitFlag{6,BitFlagSource::L}, 2};
            case 0xF6: return Instruction{InstructionType::SET, BitFlag{6,BitFlagSource::HLI}, 4};
            case 0xF7: return Instruction{InstructionType::SET, BitFlag{6,BitFlagSource::A}, 2};
            case 0xF8: return Instruction{InstructionType::SET, BitFlag{7,BitFlagSource::B}, 2};
            case 0xF9: return Instruction{InstructionType::SET, BitFlag{7,BitFlagSource::C}, 2};
            case 0xFA: return Instruction{InstructionType::SET, BitFlag{7,BitFlagSource::D}, 2};
            case 0xFB: return Instruction{InstructionType::SET, BitFlag{7,BitFlagSource::E}, 2};
            case 0xFC: return Instruction{InstructionType::SET, BitFlag{7,BitFlagSource::H}, 2};
            case 0xFD: return Instruction{InstructionType::SET, BitFlag{7,BitFlagSource::L}, 2};
            case 0xFE: return Instruction{InstructionType::SET, BitFlag{7,BitFlagSource::HLI}, 4};
            case 0xFF: return Instruction{InstructionType::SET, BitFlag{7,BitFlagSource::A}, 2};

            default: return Instruction{InstructionType::NOP, 0}; break;
            }
        } else {
            switch (byte)
            {
            case 0x00: return Instruction{InstructionType::NOP, 1};
            case 0x01: return Instruction{InstructionType::LD, LoadByte{LoadTarget::BC, LoadSource::D16}, 3};
            case 0x02: return Instruction{InstructionType::LD, LoadByte{LoadTarget::BCI, LoadSource::A}, 2};
            case 0x03: return Instruction{InstructionType::INC, Arithmetic{ArithmeticTarget::BC}, 2};
            case 0x04: return Instruction{InstructionType::INC, Arithmetic{ArithmeticTarget::B}, 1};
            case 0x05: return Instruction{InstructionType::DEC, Arithmetic{ArithmeticTarget::B}, 1};
            case 0x06: return Instruction{InstructionType::LD, LoadByte{LoadTarget::B, LoadSource::D8}, 2};
            case 0x07: return Instruction{InstructionType::RLCA, 1};
            case 0x08: return Instruction{InstructionType::LD, LoadByte{LoadTarget::A16, LoadSource::SP}, 5};
            case 0x09: return Instruction{InstructionType::ADD, Arithmetic{ArithmeticTarget::HL, ArithmeticSource::BC},2};
            case 0x0A: return Instruction{InstructionType::LD, LoadByte{LoadTarget::A, LoadSource::BCI}, 2};
            case 0x0B: return Instruction{InstructionType::DEC, Arithmetic{ArithmeticTarget::BC}, 2};
            case 0x0C: return Instruction{InstructionType::INC, Arithmetic{ArithmeticTarget::C}, 1};
            case 0x0D: return Instruction{InstructionType::DEC, Arithmetic{ArithmeticTarget::C}, 1};
            case 0x0E: return Instruction{InstructionType::LD, LoadByte{LoadTarget::C, LoadSource::D8}, 2};
            case 0x0F: return Instruction{InstructionType::RRCA, 1};

            case 0x10: return Instruction{InstructionType::STOP, 1};
            case 0x11: return Instruction{InstructionType::LD, LoadByte{LoadTarget::DE, LoadSource::D16}, 3};
            case 0x12: return Instruction{InstructionType::LD, LoadByte{LoadTarget::DEI, LoadSource::A}, 2};
            case 0x13: return Instruction{InstructionType::INC, Arithmetic{ArithmeticTarget::DE}, 2};
            case 0x14: return Instruction{InstructionType::INC, Arithmetic{ArithmeticTarget::D}, 1};
            case 0x15: return Instruction{InstructionType::DEC, Arithmetic{ArithmeticTarget::D}, 1};
            case 0x16: return Instruction{InstructionType::LD, LoadByte{LoadTarget::D, LoadSource::D8}, 2};
            case 0x17: return Instruction{InstructionType::RLA, 1};
            case 0x18: return Instruction{InstructionType::JR, JumpTest::Always, 0};
            case 0x19: return Instruction{InstructionType::ADD, Arithmetic{ArithmeticTarget::HL, ArithmeticSource::DE}, 2};
            case 0x1A: return Instruction{InstructionType::LD, LoadByte{LoadTarget::A, LoadSource::DEI}, 2};
            case 0x1B: return Instruction{InstructionType::DEC, Arithmetic{ArithmeticTarget::DE}, 2};
            case 0x1C: return Instruction{InstructionType::INC, Arithmetic{ArithmeticTarget::E}, 1};
            case 0x1D: return Instruction{InstructionType::DEC, Arithmetic{ArithmeticTarget::E}, 1};
            case 0x1E: return Instruction{InstructionType::LD, LoadByte{LoadTarget::E, LoadSource::D8}, 2};
            case 0x1F: return Instruction{InstructionType::RRA, 1};

            case 0x20: return Instruction{InstructionType::JR, JumpTest::NotZero, 0};
            case 0x21: return Instruction{InstructionType::LD, LoadByte{LoadTarget::HL, LoadSource::D16}, 3};
            case 0x22: return Instruction{InstructionType::LD, LoadByte{LoadTarget::HLINC, LoadSource::A}, 2};
            case 0x23: return Instruction{InstructionType::INC, Arithmetic{ArithmeticTarget::HL}, 2};
            case 0x24: return Instruction{InstructionType::INC, Arithmetic{ArithmeticTarget::H}, 1};
            case 0x25: return Instruction{InstructionType::DEC, Arithmetic{ArithmeticTarget::H}, 1};
            case 0x26: return Instruction{InstructionType::LD, LoadByte{LoadTarget::H, LoadSource::D8}, 2};
            case 0x27: return Instruction{InstructionType::DAA, 1};
            case 0x28: return Instruction{InstructionType::JR, JumpTest::Zero, 0};
            case 0x29: return Instruction{InstructionType::ADD, Arithmetic{ArithmeticTarget::HL, ArithmeticSource::HL}, 2};
            case 0x2A: return Instruction{InstructionType::LD, LoadByte{LoadTarget::A, LoadSource::HLINC}, 2};
            case 0x2B: return Instruction{InstructionType::DEC, Arithmetic{ArithmeticTarget::HL}, 2};
            case 0x2C: return Instruction{InstructionType::INC, Arithmetic{ArithmeticTarget::L}, 1};
            case 0x2D: return Instruction{InstructionType::DEC, Arithmetic{ArithmeticTarget::L}, 1};
            case 0x2E: return Instruction{InstructionType::LD, LoadByte{LoadTarget::L, LoadSource::D8},2};
            case 0x2F: return Instruction{InstructionType::CPL, 1};

            case 0x30: return Instruction{InstructionType::JR, JumpTest::NotCarry, 0};
            case 0x31: return Instruction{InstructionType::LD, LoadByte{LoadTarget::SP, LoadSource::D16}, 3};
            case 0x32: return Instruction{InstructionType::LD, LoadByte{LoadTarget::HLDEC, LoadSource::A}, 2};
            case 0x33: return Instruction{InstructionType::INC, Arithmetic{ArithmeticTarget::SP}, 2};
            case 0x34: return Instruction{InstructionType::INC, Arithmetic{ArithmeticTarget::HLI}, 3};
            case 0x35: return Instruction{InstructionType::DEC, Arithmetic{ArithmeticTarget::HLI}, 3};
            case 0x36: return Instruction{InstructionType::LD, LoadByte{LoadTarget::HLI, LoadSource::D8}, 3};
            case 0x37: return Instruction{InstructionType::SCF, 1};
            case 0x38: return Instruction{InstructionType::JR, JumpTest::Carry, 0};
            case 0x39: return Instruction{InstructionType::ADD, Arithmetic{ArithmeticTarget::HL, ArithmeticSource::SP}, 2};;
            case 0x3A: return Instruction{InstructionType::LD, LoadByte{LoadTarget::A, LoadSource::HLDEC}, 2};
            case 0x3B: return Instruction{InstructionType::DEC, Arithmetic{ArithmeticTarget::SP}, 2};
            case 0x3C: return Instruction{InstructionType::INC, Arithmetic{ArithmeticTarget::A}, 1};
            case 0x3D: return Instruction{InstructionType::DEC, Arithmetic{ArithmeticTarget::A}, 1};
            case 0x3E: return Instruction{InstructionType::LD, LoadByte{LoadTarget::A, LoadSource::D8}, 2};
            case 0x3F: return Instruction{InstructionType::CCF, 1};

            case 0x40: return Instruction{InstructionType::LD, LoadByte{LoadTarget::B, LoadSource::B}, 1};
            case 0x41: return Instruction{InstructionType::LD, LoadByte{LoadTarget::B, LoadSource::C}, 1};
            case 0x42: return Instruction{InstructionType::LD, LoadByte{LoadTarget::B, LoadSource::D}, 1};
            case 0x43: return Instruction{InstructionType::LD, LoadByte{LoadTarget::B, LoadSource::E}, 1};
            case 0x44: return Instruction{InstructionType::LD, LoadByte{LoadTarget::B, LoadSource::H}, 1};
            case 0x45: return Instruction{InstructionType::LD, LoadByte{LoadTarget::B, LoadSource::L}, 1};
            case 0x46: return Instruction{InstructionType::LD, LoadByte{LoadTarget::B, LoadSource::HLI}, 2};
            case 0x47: return Instruction{InstructionType::LD, LoadByte{LoadTarget::B, LoadSource::A}, 1};
            case 0x48: return Instruction{InstructionType::LD, LoadByte{LoadTarget::C, LoadSource::B}, 1};
            case 0x49: return Instruction{InstructionType::LD, LoadByte{LoadTarget::C, LoadSource::C}, 1};
            case 0x4A: return Instruction{InstructionType::LD, LoadByte{LoadTarget::C, LoadSource::D}, 1};
            case 0x4B: return Instruction{InstructionType::LD, LoadByte{LoadTarget::C, LoadSource::E}, 1};
            case 0x4C: return Instruction{InstructionType::LD, LoadByte{LoadTarget::C, LoadSource::H}, 1};
            case 0x4D: return Instruction{InstructionType::LD, LoadByte{LoadTarget::C, LoadSource::L}, 1};
            case 0x4E: return Instruction{InstructionType::LD, LoadByte{LoadTarget::C, LoadSource::HLI}, 2};
            case 0x4F: return Instruction{InstructionType::LD, LoadByte{LoadTarget::C, LoadSource::A}, 1};

            case 0x50: return Instruction{InstructionType::LD, LoadByte{LoadTarget::D, LoadSource::B}, 1};
            case 0x51: return Instruction{InstructionType::LD, LoadByte{LoadTarget::D, LoadSource::C}, 1};
            case 0x52: return Instruction{InstructionType::LD, LoadByte{LoadTarget::D, LoadSource::D}, 1};
            case 0x53: return Instruction{InstructionType::LD, LoadByte{LoadTarget::D, LoadSource::E}, 1};
            case 0x54: return Instruction{InstructionType::LD, LoadByte{LoadTarget::D, LoadSource::H}, 1};
            case 0x55: return Instruction{InstructionType::LD, LoadByte{LoadTarget::D, LoadSource::L}, 1};
            case 0x56: return Instruction{InstructionType::LD, LoadByte{LoadTarget::D, LoadSource::HLI}, 2};
            case 0x57: return Instruction{InstructionType::LD, LoadByte{LoadTarget::D, LoadSource::A}, 1};
            case 0x58: return Instruction{InstructionType::LD, LoadByte{LoadTarget::E, LoadSource::B}, 1};
            case 0x59: return Instruction{InstructionType::LD, LoadByte{LoadTarget::E, LoadSource::C}, 1};
            case 0x5A: return Instruction{InstructionType::LD, LoadByte{LoadTarget::E, LoadSource::D}, 1};
            case 0x5B: return Instruction{InstructionType::LD, LoadByte{LoadTarget::E, LoadSource::E}, 1};
            case 0x5C: return Instruction{InstructionType::LD, LoadByte{LoadTarget::E, LoadSource::H}, 1};
            case 0x5D: return Instruction{InstructionType::LD, LoadByte{LoadTarget::E, LoadSource::L}, 1};
            case 0x5E: return Instruction{InstructionType::LD, LoadByte{LoadTarget::E, LoadSource::HLI}, 2};
            case 0x5F: return Instruction{InstructionType::LD, LoadByte{LoadTarget::E, LoadSource::A}, 1};

            case 0x60: return Instruction{InstructionType::LD, LoadByte{LoadTarget::H, LoadSource::B}, 1};
            case 0x61: return Instruction{InstructionType::LD, LoadByte{LoadTarget::H, LoadSource::C}, 1};
            case 0x62: return Instruction{InstructionType::LD, LoadByte{LoadTarget::H, LoadSource::D}, 1};
            case 0x63: return Instruction{InstructionType::LD, LoadByte{LoadTarget::H, LoadSource::E}, 1};
            case 0x64: return Instruction{InstructionType::LD, LoadByte{LoadTarget::H, LoadSource::H}, 1};
            case 0x65: return Instruction{InstructionType::LD, LoadByte{LoadTarget::H, LoadSource::L}, 1};
            case 0x66: return Instruction{InstructionType::LD, LoadByte{LoadTarget::H, LoadSource::HLI}, 2};
            case 0x67: return Instruction{InstructionType::LD, LoadByte{LoadTarget::H, LoadSource::A}, 1};
            case 0x68: return Instruction{InstructionType::LD, LoadByte{LoadTarget::L, LoadSource::B}, 1};
            case 0x69: return Instruction{InstructionType::LD, LoadByte{LoadTarget::L, LoadSource::C}, 1};
            case 0x6A: return Instruction{InstructionType::LD, LoadByte{LoadTarget::L, LoadSource::D}, 1};
            case 0x6B: return Instruction{InstructionType::LD, LoadByte{LoadTarget::L, LoadSource::E}, 1};
            case 0x6C: return Instruction{InstructionType::LD, LoadByte{LoadTarget::L, LoadSource::H}, 1};
            case 0x6D: return Instruction{InstructionType::LD, LoadByte{LoadTarget::L, LoadSource::L}, 1};
            case 0x6E: return Instruction{InstructionType::LD, LoadByte{LoadTarget::L, LoadSource::HLI}, 2};
            case 0x6F: return Instruction{InstructionType::LD, LoadByte{LoadTarget::L, LoadSource::A}, 1};

            case 0x70: return Instruction{InstructionType::LD, LoadByte{LoadTarget::HLI, LoadSource::B}, 2};
            case 0x71: return Instruction{InstructionType::LD, LoadByte{LoadTarget::HLI, LoadSource::C}, 2};
            case 0x72: return Instruction{InstructionType::LD, LoadByte{LoadTarget::HLI, LoadSource::D}, 2};
            case 0x73: return Instruction{InstructionType::LD, LoadByte{LoadTarget::HLI, LoadSource::E}, 2};
            case 0x74: return Instruction{InstructionType::LD, LoadByte{LoadTarget::HLI, LoadSource::H}, 2};
            case 0x75: return Instruction{InstructionType::LD, LoadByte{LoadTarget::HLI, LoadSource::L}, 2};
            case 0x76: return Instruction{InstructionType::HALT, 1};
            case 0x77: return Instruction{InstructionType::LD, LoadByte{LoadTarget::HLI, LoadSource::A}, 2};
            case 0x78: return Instruction{InstructionType::LD, LoadByte{LoadTarget::A, LoadSource::B},1};
            case 0x79: return Instruction{InstructionType::LD, LoadByte{LoadTarget::A, LoadSource::C},1};
            case 0x7A: return Instruction{InstructionType::LD, LoadByte{LoadTarget::A, LoadSource::D},1};
            case 0x7B: return Instruction{InstructionType::LD, LoadByte{LoadTarget::A, LoadSource::E},1};
            case 0x7C: return Instruction{InstructionType::LD, LoadByte{LoadTarget::A, LoadSource::H},1};
            case 0x7D: return Instruction{InstructionType::LD, LoadByte{LoadTarget::A, LoadSource::L},1};
            case 0x7E: return Instruction{InstructionType::LD, LoadByte{LoadTarget::A, LoadSource::HLI}, 2};
            case 0x7F: return Instruction{InstructionType::LD, LoadByte{LoadTarget::A, LoadSource::A},1};

            case 0x80: return Instruction{InstructionType::ADD, Arithmetic{ArithmeticTarget::A, ArithmeticSource::B}, 1};
            case 0x81: return Instruction{InstructionType::ADD, Arithmetic{ArithmeticTarget::A, ArithmeticSource::C}, 1};
            case 0x82: return Instruction{InstructionType::ADD, Arithmetic{ArithmeticTarget::A, ArithmeticSource::D}, 1};
            case 0x83: return Instruction{InstructionType::ADD, Arithmetic{ArithmeticTarget::A, ArithmeticSource::E}, 1};
            case 0x84: return Instruction{InstructionType::ADD, Arithmetic{ArithmeticTarget::A, ArithmeticSource::H}, 1};
            case 0x85: return Instruction{InstructionType::ADD, Arithmetic{ArithmeticTarget::A, ArithmeticSource::L}, 1};
            case 0x86: return Instruction{InstructionType::ADD, Arithmetic{ArithmeticTarget::A, ArithmeticSource::HLI}, 2};
            case 0x87: return Instruction{InstructionType::ADD, Arithmetic{ArithmeticTarget::A, ArithmeticSource::A}, 1};
            case 0x88: return Instruction{InstructionType::ADC, Arithmetic{ArithmeticTarget::A, ArithmeticSource::B}, 1};
            case 0x89: return Instruction{InstructionType::ADC, Arithmetic{ArithmeticTarget::A, ArithmeticSource::C}, 1};
            case 0x8A: return Instruction{InstructionType::ADC, Arithmetic{ArithmeticTarget::A, ArithmeticSource::D}, 1};
            case 0x8B: return Instruction{InstructionType::ADC, Arithmetic{ArithmeticTarget::A, ArithmeticSource::E}, 1};
            case 0x8C: return Instruction{InstructionType::ADC, Arithmetic{ArithmeticTarget::A, ArithmeticSource::H}, 1};
            case 0x8D: return Instruction{InstructionType::ADC, Arithmetic{ArithmeticTarget::A, ArithmeticSource::L}, 1};
            case 0x8E: return Instruction{InstructionType::ADC, Arithmetic{ArithmeticTarget::A, ArithmeticSource::HLI}, 2};
            case 0x8F: return Instruction{InstructionType::ADC, Arithmetic{ArithmeticTarget::A, ArithmeticSource::A}, 1};

            case 0x90: return Instruction{InstructionType::SUB, Arithmetic{ArithmeticTarget::A, ArithmeticSource::B}, 1};
            case 0x91: return Instruction{InstructionType::SUB, Arithmetic{ArithmeticTarget::A, ArithmeticSource::C}, 1};
            case 0x92: return Instruction{InstructionType::SUB, Arithmetic{ArithmeticTarget::A, ArithmeticSource::D}, 1};
            case 0x93: return Instruction{InstructionType::SUB, Arithmetic{ArithmeticTarget::A, ArithmeticSource::E}, 1};
            case 0x94: return Instruction{InstructionType::SUB, Arithmetic{ArithmeticTarget::A, ArithmeticSource::H}, 1};
            case 0x95: return Instruction{InstructionType::SUB, Arithmetic{ArithmeticTarget::A, ArithmeticSource::L}, 1};
            case 0x96: return Instruction{InstructionType::SUB, Arithmetic{ArithmeticTarget::A, ArithmeticSource::HLI}, 2};
            case 0x97: return Instruction{InstructionType::SUB, Arithmetic{ArithmeticTarget::A, ArithmeticSource::A}, 1};
            case 0x98: return Instruction{InstructionType::SBC, Arithmetic{ArithmeticTarget::A, ArithmeticSource::B}, 1};
            case 0x99: return Instruction{InstructionType::SBC, Arithmetic{ArithmeticTarget::A, ArithmeticSource::C}, 1};
            case 0x9A: return Instruction{InstructionType::SBC, Arithmetic{ArithmeticTarget::A, ArithmeticSource::D}, 1};
            case 0x9B: return Instruction{InstructionType::SBC, Arithmetic{ArithmeticTarget::A, ArithmeticSource::E}, 1};
            case 0x9C: return Instruction{InstructionType::SBC, Arithmetic{ArithmeticTarget::A, ArithmeticSource::H}, 1};
            case 0x9D: return Instruction{InstructionType::SBC, Arithmetic{ArithmeticTarget::A, ArithmeticSource::L}, 1};
            case 0x9E: return Instruction{InstructionType::SBC, Arithmetic{ArithmeticTarget::A, ArithmeticSource::HLI}, 2};
            case 0x9F: return Instruction{InstructionType::SBC, Arithmetic{ArithmeticTarget::A, ArithmeticSource::A}, 1};

            case 0xA0: return Instruction{InstructionType::AND, BitwiseSource::B, 1};
            case 0xA1: return Instruction{InstructionType::AND, BitwiseSource::C, 1};
            case 0xA2: return Instruction{InstructionType::AND, BitwiseSource::D, 1};
            case 0xA3: return Instruction{InstructionType::AND, BitwiseSource::E, 1};
            case 0xA4: return Instruction{InstructionType::AND, BitwiseSource::H, 1};
            case 0xA5: return Instruction{InstructionType::AND, BitwiseSource::L, 1};
            case 0xA6: return Instruction{InstructionType::AND, BitwiseSource::HLI, 2};
            case 0xA7: return Instruction{InstructionType::AND, BitwiseSource::A, 1};
            case 0xA8: return Instruction{InstructionType::XOR, BitwiseSource::B, 1};
            case 0xA9: return Instruction{InstructionType::XOR, BitwiseSource::C, 1};
            case 0xAA: return Instruction{InstructionType::XOR, BitwiseSource::D, 1};
            case 0xAB: return Instruction{InstructionType::XOR, BitwiseSource::E, 1};
            case 0xAC: return Instruction{InstructionType::XOR, BitwiseSource::H, 1};
            case 0xAD: return Instruction{InstructionType::XOR, BitwiseSource::L, 1};
            case 0xAE: return Instruction{InstructionType::XOR, BitwiseSource::HLI, 2};
            case 0xAF: return Instruction{InstructionType::XOR, BitwiseSource::A, 1};

            case 0xB0: return Instruction{InstructionType::OR, BitwiseSource::B, 1};
            case 0xB1: return Instruction{InstructionType::OR, BitwiseSource::C, 1};
            case 0xB2: return Instruction{InstructionType::OR, BitwiseSource::D, 1};
            case 0xB3: return Instruction{InstructionType::OR, BitwiseSource::E, 1};
            case 0xB4: return Instruction{InstructionType::OR, BitwiseSource::H, 1};
            case 0xB5: return Instruction{InstructionType::OR, BitwiseSource::L, 1};
            case 0xB6: return Instruction{InstructionType::OR, BitwiseSource::HLI, 2};
            case 0xB7: return Instruction{InstructionType::OR, BitwiseSource::A, 1};
            case 0xB8: return Instruction{InstructionType::CP, BitwiseSource::B, 1};
            case 0xB9: return Instruction{InstructionType::CP, BitwiseSource::C, 1};
            case 0xBA: return Instruction{InstructionType::CP, BitwiseSource::D, 1};
            case 0xBB: return Instruction{InstructionType::CP, BitwiseSource::E, 1};
            case 0xBC: return Instruction{InstructionType::CP, BitwiseSource::H, 1};
            case 0xBD: return Instruction{InstructionType::CP, BitwiseSource::L, 1};
            case 0xBE: return Instruction{InstructionType::CP, BitwiseSource::HLI, 2};
            case 0xBF: return Instruction{InstructionType::CP, BitwiseSource::A, 1};

            case 0xC0: return Instruction{InstructionType::RET, JumpTest::NotZero, 0};
            case 0xC1: return Instruction{InstructionType::POP, RegisterPairs::BC, 3};
            case 0xC2: return Instruction{InstructionType::JP, JumpTest::NotZero, 0};
            case 0xC3: return Instruction{InstructionType::JP, JumpTest::Always, 0};
            case 0xC4: return Instruction{InstructionType::CALL, JumpTest::NotZero, 0};
            case 0xC5: return Instruction{InstructionType::PUSH, RegisterPairs::BC, 4};
            case 0xC6: return Instruction{InstructionType::ADD, Arithmetic{ArithmeticTarget::A, ArithmeticSource::D8}, 2};
            case 0xC7: return Instruction{InstructionType::RST, Vectors::V0x00, 4};
            case 0xC8: return Instruction{InstructionType::RET, JumpTest::Zero, 0};
            case 0xC9: return Instruction{InstructionType::RET, JumpTest::Always, 0};
            case 0xCA: return Instruction{InstructionType::JP, JumpTest::Zero, 0};
            case 0xCB: break;
            case 0xCC: return Instruction{InstructionType::CALL, JumpTest::Zero, 0};
            case 0xCD: return Instruction{InstructionType::CALL, JumpTest::Always, 0};
            case 0xCE: return Instruction{InstructionType::ADC, Arithmetic{ArithmeticTarget::A, ArithmeticSource::D8}, 2};
            case 0xCF: return Instruction{InstructionType::RST, Vectors::V0x08, 4};

            case 0xD0: return Instruction{InstructionType::RET, JumpTest::NotCarry, 0};
            case 0xD1: return Instruction{InstructionType::POP, RegisterPairs::DE, 3};
            case 0xD2: return Instruction{InstructionType::JP, JumpTest::NotCarry, 0};
            case 0xD3: break;
            case 0xD4: return Instruction{InstructionType::CALL, JumpTest::NotCarry, 0};
            case 0xD5: return Instruction{InstructionType::PUSH, RegisterPairs::DE, 4};
            case 0xD6: return Instruction{InstructionType::SUB, Arithmetic{ArithmeticTarget::A, ArithmeticSource::D8}, 2};
            case 0xD7: return Instruction{InstructionType::RST, Vectors::V0x10, 4};
            case 0xD8: return Instruction{InstructionType::RET, JumpTest::Carry, 0};
            case 0xD9: return Instruction{InstructionType::RETI, 4};
            case 0xDA: return Instruction{InstructionType::JP, JumpTest::Carry, 0};
            case 0xDB: break;
            case 0xDC: return Instruction{InstructionType::CALL, JumpTest::Carry, 0};
            case 0xDD: break;
            case 0xDE: return Instruction{InstructionType::SBC, Arithmetic{ArithmeticTarget::A, ArithmeticSource::D8}, 2};
            case 0xDF: return Instruction{InstructionType::RST, Vectors::V0x18, 3};

            case 0xE0: return Instruction{InstructionType::LDH, LoadByte{LoadTarget::D8, LoadSource::A}, 3};
            case 0xE1: return Instruction{InstructionType::POP, RegisterPairs::HL, 3};
            case 0xE2: return Instruction{InstructionType::LDH, LoadByte{LoadTarget::C, LoadSource::A}, 2};
            case 0xE3: break;
            case 0xE4: break;
            case 0xE5: return Instruction{InstructionType::PUSH, RegisterPairs::HL, 4};
            case 0xE6: return Instruction{InstructionType::AND, BitwiseSource::D8, 2};
            case 0xE7: return Instruction{InstructionType::RST, Vectors::V0x20, 4};
            case 0xE8: return Instruction{InstructionType::ADD, Arithmetic{ArithmeticTarget::SP, ArithmeticSource::D8}, 4};
            case 0xE9: return Instruction{InstructionType::JPHL, JumpTest::Always, 1};
            case 0xEA: return Instruction{InstructionType::LD, LoadByte{LoadTarget::A16I, LoadSource::A}, 4};
            case 0xEB: break;
            case 0xEC: break;
            case 0xED: break;
            case 0xEE: return Instruction{InstructionType::XOR, BitwiseSource::D8, 2};
            case 0xEF: return Instruction{InstructionType::RST, Vectors::V0x28, 4};

            case 0xF0: return Instruction{InstructionType::LDH, LoadByte{LoadTarget::A, LoadSource::D8}, 3};
            case 0xF1: return Instruction{InstructionType::POP, RegisterPairs::AF, 3};
            case 0xF2: return Instruction{InstructionType::LDH, LoadByte{LoadTarget::A, LoadSource::C}, 2};
            case 0xF3: return Instruction{InstructionType::DI, 1};
            case 0xF4: break;
            case 0xF5: return Instruction{InstructionType::PUSH, RegisterPairs::AF, 4};
            case 0xF6: return Instruction{InstructionType::OR, BitwiseSource::D8, 2};
            case 0xF7: return Instruction{InstructionType::RST, Vectors::V0x30, 4};
            case 0xF8: return Instruction{InstructionType::LDHLSP, 3};
            case 0xF9: return Instruction{InstructionType::LD, LoadByte{LoadTarget::SP, LoadSource::HL}, 2};
            case 0xFA: return Instruction{InstructionType::LD, LoadByte{LoadTarget::A, LoadSource::A16I}, 4};
            case 0xFB: return Instruction{InstructionType::EI, 1};
            case 0xFC: break;
            case 0xFD: break;
            case 0xFE: return Instruction{InstructionType::CP, BitwiseSource::D8, 2};
            case 0xFF: return Instruction{InstructionType::RST, Vectors::V0x38, 4};

            default: return Instruction{InstructionType::NOP, 0}; break;
            }
        }

        return Instruction{InstructionType::NOP, 0};
    }
    
};

uint8_t* CPU::get8BitReg(const std::string& name){
    
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

uint8_t* CPU::get8BitReg(const Target& target){

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

uint16_t* CPU::get16BitReg(const std::string& name){

    if (name == "SP") return &this->regs.SP;
    if (name == "PC") return &this->regs.PC;

    return nullptr;

}

uint16_t* CPU::get16BitReg(const Target& target){

    if (target == Target::SP) return &this->regs.SP;
    if (target == Target::PC) return &this->regs.PC;

    return nullptr;
}

uint16_t CPU::extract16BitReg(const std::string& name){

    if (name == "SP") return this->regs.SP;
    if (name == "PC") return this->regs.PC;

    if (name == "AF") return (static_cast<uint16_t>(this->regs.A)<<8 | this->regs.F);
    if (name == "BC") return (static_cast<uint16_t>(this->regs.B)<<8 | this->regs.C);
    if (name == "DE") return (static_cast<uint16_t>(this->regs.D)<<8 | this->regs.E);
    if (name == "HL") return (static_cast<uint16_t>(this->regs.H)<<8 | this->regs.L);

    return 0;
}

uint16_t CPU::extract16BitReg(const Target& target){

    if (target == Target::SP) return this->regs.SP;
    if (target == Target::PC) return this->regs.PC;

    if (target == Target::AF) return getCombined(RegisterPairs::AF);
    if (target == Target::BC) return getCombined(RegisterPairs::BC);
    if (target == Target::DE) return getCombined(RegisterPairs::DE);
    if (target == Target::HL) return getCombined(RegisterPairs::HL);

    return 0;
}

std::pair<uint16_t, bool> CPU::overflowSum(uint16_t val1, uint16_t val2){

    uint16_t result = val1 + val2;
    bool overflow = (result < val1);
    
    return {result, overflow};

}

std::pair<uint8_t, bool> CPU::overflowSum(uint8_t val1, uint8_t val2){

    uint8_t result = val1 + val2;
    bool overflow = (result < val1);
    
    return {result, overflow};

}

std::pair<uint16_t, bool> CPU::underflowSub(uint16_t val1, uint16_t val2){

    uint16_t result = val1 - val2;
    bool underflow = val1 < val2;

    return {result, underflow};
}

std::pair<uint8_t, bool> CPU::underflowSub(uint8_t val1, uint8_t val2){
    
    uint8_t result = val1 - val2;
    bool underflow = val1 < val2;

    return {result, underflow};

}

uint16_t CPU::ADD(const Arithmetic& arithmetic){

    ArithmeticSource source = arithmetic.arithmeticSource;
    ArithmeticTarget target = arithmetic.arithmeticTarget;

    if (source == ArithmeticSource::D8 && target != ArithmeticTarget::SP){
        
        uint8_t value = this->memory->readByte(this->regs.PC+1);

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
            value = this->memory->readByte(this->getCombined(RegisterPairs::HL));
            break;
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

        changeFlag((this->regs.F >> 7) & 1, 0, ((this->getCombined(RegisterPairs::HL) & 0x0FFF) + (value & 0x0FFF)) > 0x0FFF, overflow);

        this->setCombined(RegisterPairs::HL, result);
        return this->regs.PC+1;

    }
    else if (target == ArithmeticTarget::SP){
        int8_t value = this->memory->readByte(this->regs.PC+1);

        if (value >= 0){
            auto [result, overflow] = overflowSum(this->regs.SP, value);
            
            changeFlag((this->regs.F >> 7) & 1, 0, ((this->regs.SP & 0x0FFF) + (value & 0x0FFF)) > 0x0FFF, overflow);
            
            this->regs.SP = result;
        }
        else {
            auto [result, overflow] = underflowSub(this->regs.SP, abs(value));

            changeFlag((this->regs.F >> 7) & 1, 0, ((this->regs.SP & 0x0FFF) < (abs(value) & 0x0FFF)), overflow);
            
            this->regs.SP = result;
        }
        return this->regs.PC+2;
    }
    return this->regs.PC+1;
}

uint16_t CPU::ADC(const Arithmetic& arithmetic){
    
    ArithmeticSource source = arithmetic.arithmeticSource;
    ArithmeticTarget target = arithmetic.arithmeticTarget;
    uint8_t carry =  (this->regs.F >> 4 & 1);

    if (source == ArithmeticSource::D8){
        uint8_t value = this->memory->readByte(this->regs.PC+1);
        

        auto [result, overflow] = overflowSum(static_cast<uint16_t>(this->regs.A), static_cast<uint16_t>(value)+static_cast<uint16_t>(carry));

        changeFlag(static_cast<uint8_t>(result) == 0, 0, ((this->regs.A & 0xF) +( value & 0xF) + carry) > 0xF, result > 0xFF);

        this->regs.A = static_cast<uint8_t>(result);

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
            value = this->memory->readByte(this->getCombined(RegisterPairs::HL));
            break;
        default:
            throw std::runtime_error("Invalid Arithmetic Source at ADC instruction.");
        }
        value += (this->regs.F >> 4 & 1);

        auto [result, overflow] = overflowSum(static_cast<uint16_t>(this->regs.A), static_cast<uint16_t>(value)+static_cast<uint16_t>(carry));

        changeFlag(static_cast<uint8_t>(result) == 0, 0, ((this->regs.A & 0xF) +( value & 0xF) + carry) > 0xF, result > 0xFF);

        this->regs.A = static_cast<uint8_t>(result);
        
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
            throw std::runtime_error("Invalid Arithmetic Source at ADC instruction.");
        }

        value += (this->regs.F >> 4 & 1);

        auto [result, overflow] = overflowSum(this->getCombined(RegisterPairs::HL), value);

        changeFlag(result == 0, 0, ((this->getCombined(RegisterPairs::HL)) & (0xF + value & 0xF)) > 0xF, overflow);
        this->setCombined(RegisterPairs::HL, result);
        return this->regs.PC+1;

    }
    return this->regs.PC+1;
}

uint16_t CPU::SUB(const Arithmetic& arithmetic){

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
        value = this->memory->readByte(this->getCombined(RegisterPairs::HL));
        break;
    case ArithmeticSource::D8:
        value = this->memory->readByte(this->regs.PC+1);
        break;
    
    default:
        throw std::runtime_error("Invalid source in SUB instruction."); 
        break;
    }

    auto [result, overflow] = underflowSub(this->regs.A, value);

    changeFlag(result == 0, 1, ((this->regs.A & 0xF) < (value & 0xF)), overflow);

    this->regs.A = result;
    
    if (source==ArithmeticSource::D8) return this->regs.PC+2;
    return this->regs.PC+1;
}

uint16_t CPU::SBC(const Arithmetic& arithmetic){
    
    ArithmeticSource source = arithmetic.arithmeticSource;

    uint8_t value = 0;
    uint8_t carry =  (this->regs.F >> 4 & 1);

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
        value = this->memory->readByte(this->getCombined(RegisterPairs::HL));
        break;
    case ArithmeticSource::D8:
        value = this->memory->readByte(this->regs.PC+1);
        break;
    
    default:
        throw std::runtime_error("Invalid source in SUB instruction."); 
        break;
    }

    auto [result, overflow] = underflowSub(static_cast<uint16_t>(this->regs.A), static_cast<uint16_t>(value) + static_cast<uint16_t>(carry));

    changeFlag(result == 0, 1, ((this->regs.A & 0xF) < (value & 0xF)), overflow);
    
    this->regs.A = result;

    if (source==ArithmeticSource::D8) return this->regs.PC+2;
    return this->regs.PC+1;
}

uint16_t CPU::CP(const BitwiseSource& bitsource){


    uint8_t value = 0;
    uint16_t toReturn = this->regs.PC+1;

    switch (bitsource)
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
    case BitwiseSource::H:
        value = this->regs.H;
        break;
    case BitwiseSource::L:
        value = this->regs.L;
        break;
    case BitwiseSource::HLI:
        value = this->memory->readByte(this->getCombined(RegisterPairs::HL));
        break;
    case BitwiseSource::D8:
        value = this->memory->readByte(this->regs.PC+1);
        toReturn ++;
        break;
    
    default:
        throw std::runtime_error("Invalid source in SUB instruction."); 
        break;
    }
    
    auto [result, overflow] = underflowSub(this->regs.A, value);

    changeFlag(result == 0, 1, ((this->regs.A & 0xF) < (value & 0xF)), overflow);
        
    return toReturn;

}

uint16_t CPU::DEC(const Arithmetic& arithmetic){

    ArithmeticTarget target = arithmetic.arithmeticTarget;

    if (target == ArithmeticTarget::HLI){

        uint16_t address = this->getCombined(RegisterPairs::HL);
        uint8_t value = this->memory->readByte(address);

        auto [result, overflow] = underflowSub(value, 1);

        changeFlag(result == 0, 1, (value & 0xF) < 1, (this->regs.F >> 4) & 1);

        this->memory->writeByte(address, result);

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
        break;
    }

    if (reg8 != nullptr){

        auto [result, overflow] = underflowSub(*reg8, 1);

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

    auto [result, overflow] = underflowSub(value, 1);

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

uint16_t CPU::INC(const Arithmetic& arithmetic){

    ArithmeticTarget target = arithmetic.arithmeticTarget;

    if (target == ArithmeticTarget::HLI){

        uint8_t address = this->getCombined(RegisterPairs::HL);
        uint8_t value = this->memory->readByte(address);

        auto [result, overflow] = overflowSum(value, 1);

        changeFlag(result == 0, false, ((value & 0xF) + 1) > 0xF, (this->regs.F >> 4) & 1);

        this->memory->writeByte(address, result);

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
        break;
    }

    if (reg8 != nullptr){

        auto [result, overflow] = overflowSum(*reg8, 1);

        changeFlag(result == 0, 0, ((*reg8 & 0xF) + 1 > 0xF), (this->regs.F >> 4) & 1);

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
        throw std::runtime_error("Invalid target at INC instruction.");
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

uint16_t CPU::AND(const BitwiseSource& bitwiseSource){

    uint8_t byteA = this->regs.A;
    uint8_t value;

    if (bitwiseSource == BitwiseSource::D8){
        value = this->memory->readByte(this->regs.PC+1);

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
        value = this->memory->readByte(this->getCombined(RegisterPairs::HL));
        break;

    default:
        throw std::runtime_error("Invalid source at AND instruction");
        break;
    }

    uint8_t bitwise = byteA & value;

    changeFlag(bitwise == 0, false, true, false);

    this->regs.A = bitwise;

    return this->regs.PC+1;
}

uint16_t CPU::CPL(){

    this->regs.A = ~this->regs.A;

    changeFlag((this->regs.F>>7) & 1, true, true, (this->regs.F >> 4) & 1);

    return this->regs.PC + 1;
}

uint16_t CPU::OR(const BitwiseSource& bitwiseSource){

    uint8_t byteA = this->regs.A;
    uint8_t value;
    uint8_t bitwise;

    if (bitwiseSource == BitwiseSource::D8){

        value = this->memory->readByte(this->regs.PC+1);

        bitwise = byteA | value;

        changeFlag(bitwise == 0, false, false, false);
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
        value = this->memory->readByte(this->getCombined(RegisterPairs::HL));
        break;
    
    default:
        throw std::runtime_error("Invalid source at OR instruction.");
        break;
    }

    bitwise = byteA | value;

    changeFlag(bitwise == 0, false, false, false);

    this->regs.A = bitwise;
    
    return this->regs.PC+1;

}

uint16_t CPU::XOR(const BitwiseSource& bitwiseSource){

    uint8_t byteA = this->regs.A;
    uint8_t value;
    uint8_t bitwise;

    if (bitwiseSource == BitwiseSource::D8){

        value = this->memory->readByte(this->regs.PC+1);

        bitwise = byteA ^ value;

        changeFlag(bitwise == 0, false, false, false);
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
        value = this->memory->readByte(this->getCombined(RegisterPairs::HL));
        break;
    
    default:
        throw std::runtime_error("Invalid source at OR instruction.");
        break;
    }

    bitwise = byteA ^ value;

    changeFlag(bitwise == 0, false, false, false);

    this->regs.A = bitwise;
    
    return this->regs.PC+1;
}

uint16_t CPU::BIT(const BitFlag& bitFlag){

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
        byte = this->memory->readByte(this->getCombined(RegisterPairs::HL));
        break;
    
    default:
        throw std::runtime_error("Invalid source at BIT instruction.");
        break;
    }

    bitSet = (byte >> u3) & 1;

    changeFlag(bitSet, 0, 1, (this->regs.F >> 4) & 1);

    return this->regs.PC + 2;
}

uint16_t CPU::RES(const BitFlag& bitFlag){

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
        this->memory->writeByte(this->getCombined(RegisterPairs::HL), (this->memory->readByte(this->getCombined(RegisterPairs::HL) & ~(0b00000001 << u3))));
        break;
    
    default:
        throw std::runtime_error("Invalid source at RES instruction.");
        break;
    }

    return this->regs.PC+2;

}

uint16_t CPU::SET(const BitFlag& bitFlag){

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
        this->memory->writeByte(this->getCombined(RegisterPairs::HL), (this->memory->readByte(this->getCombined(RegisterPairs::HL) | (0b00000001 < u3))));
        break;
    
    default:
        throw std::runtime_error("Invalid source at RES instruction.");
        break;
    }

    return this->regs.PC+2;
}

uint16_t CPU::RL(const RotateTarget& target){

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
        bit7 = (this->memory->readByte(this->getCombined(RegisterPairs::HL)) >> 7) & 1;
        result = static_cast<uint8_t>(this->memory->readByte(this->getCombined(RegisterPairs::HL)) << 1) | carry;
        this->memory->writeByte(this->getCombined(RegisterPairs::HL), result);
        break;
    
    default:
        throw std::runtime_error("Invalid target at the RL instruction.");
        break;
    }

    changeFlag(result == 0, false, false, bit7);

    return this->regs.PC+2;

}

uint16_t CPU::RLC(const RotateTarget& target){
    
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
        bit7 = (this->memory->readByte(this->getCombined(RegisterPairs::HL)) >> 7) & 1;
        result = static_cast<uint8_t>(this->memory->readByte(this->getCombined(RegisterPairs::HL)) << 1) | bit7;
        this->memory->writeByte(this->getCombined(RegisterPairs::HL), result);
        break;
    
    default:
        throw std::runtime_error("Invalid target at the RL instruction.");
        break;
    }

    changeFlag(result == 0, false, false, bit7);

    return this->regs.PC+2;
}

uint16_t CPU::RR(const RotateTarget& target){

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
        bit0 = (this->memory->readByte(this->getCombined(RegisterPairs::HL))) & 1;
        result = static_cast<uint8_t>(this->memory->readByte(this->getCombined(RegisterPairs::HL)) >> 1) | static_cast<uint8_t>(carry << 7);
        this->memory->writeByte(this->getCombined(RegisterPairs::HL), result);
        break;
    
    default:
        throw std::runtime_error("Invalid target at the RL instruction.");
        break;
    }

    changeFlag(result == 0, false, false, bit0);

    return this->regs.PC+2;

}

uint16_t CPU::RRC(const RotateTarget& target){

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
        bit0 = (this->memory->readByte(this->getCombined(RegisterPairs::HL))) & 1;
        result = static_cast<uint8_t>(this->memory->readByte(this->getCombined(RegisterPairs::HL)) >> 1) | static_cast<uint8_t>(bit0 << 7);
        this->memory->writeByte(this->getCombined(RegisterPairs::HL), result);
        break;
    
    default:
        throw std::runtime_error("Invalid target at the RL instruction.");
        break;
    }

    changeFlag(result == 0, false, false, bit0);

    return this->regs.PC+2;
}

uint16_t CPU::RRCA(){

    uint8_t bit0 = 0;
    uint8_t result = 0;

    bit0 = (this->regs.A) & 1;
    result = static_cast<uint8_t>(this->regs.A >> 1) | static_cast<uint8_t>(bit0 << 7);
    this->regs.A = result;

    changeFlag(0, 0, 0, bit0);

    return this->regs.PC+1;
}

uint16_t CPU::RRA(){

    uint8_t carry = (this->regs.F >> 4) & 1;
    uint8_t bit0 = 0;
    uint8_t result = 0;

    bit0 = (this->regs.A) & 1;
    result = static_cast<uint8_t>(this->regs.A >> 1) | static_cast<uint8_t>(carry << 7);
    this->regs.A = result;

    changeFlag(0, 0, 0, bit0);

    return this->regs.PC+1;
}

uint16_t CPU::RLCA(){
    uint8_t bit7 = 0;
    uint8_t result = 0;
    
    bit7 = (this->regs.A >> 7) & 1;
    result = static_cast<uint8_t>(this->regs.A << 1) | bit7;
    this->regs.A = result;

    changeFlag(0, 0, 0, bit7);

    return this->regs.PC+1;    
}

uint16_t CPU::RLA(){
    uint8_t carry = (this->regs.F >> 4) & 1;
    uint8_t bit7 = 0;
    uint8_t result = 0;

    bit7 = (this->regs.A >> 7) & 1;
    result = static_cast<uint8_t>(this->regs.A << 1) | carry;
    this->regs.A = result;

    changeFlag(0, 0, 0, bit7);

    return this->regs.PC+1;
}

uint16_t CPU::SLA(const RotateTarget& target){
    
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
        bit7 = (this->memory->readByte(this->getCombined(RegisterPairs::HL)) >> 7) & 1;
        result = static_cast<uint8_t>(this->memory->readByte(this->getCombined(RegisterPairs::HL)) << 1);
        this->memory->writeByte(this->getCombined(RegisterPairs::HL), result);
        break;
    
    default:
        throw std::runtime_error("Invalid target at the RL instruction.");
        break;
    }

    changeFlag(result == 0, false, false, bit7 == 1);

    return this->regs.PC+2;

}

uint16_t CPU::SRA(const RotateTarget& target){
    
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
        bit0 = (this->memory->readByte(this->getCombined(RegisterPairs::HL))) & 1;
        bit7 = (this->memory->readByte(this->getCombined(RegisterPairs::HL)) >> 7) & 1;
        result = static_cast<uint8_t>(this->memory->readByte(this->getCombined(RegisterPairs::HL)) >> 1) | static_cast<uint8_t>(bit7 << 7);
        this->memory->writeByte(this->getCombined(RegisterPairs::HL), result);
        break;
    
    default:
        throw std::runtime_error("Invalid target at the RL instruction.");
        break;
    }

    changeFlag(result == 0, false, false, bit0);

    return this->regs.PC+2;
}

uint16_t CPU::SRL(const RotateTarget& target){
    
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
        bit0 = (this->memory->readByte(this->getCombined(RegisterPairs::HL))) & 1;
        result = static_cast<uint8_t>(this->memory->readByte(this->getCombined(RegisterPairs::HL)) >> 1);
        this->memory->writeByte(this->getCombined(RegisterPairs::HL), result);
        break;
    
    default:
        throw std::runtime_error("Invalid target at the RL instruction.");
        break;
    }

    changeFlag(result == 0, false, false, bit0);

    return this->regs.PC+2;
}

uint16_t CPU::SWAP(const RotateTarget& target){

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
        upper = (this->memory->readByte(this->getCombined(RegisterPairs::HL)) >> 4) & 0b1111;
        lower = (this->memory->readByte(this->getCombined(RegisterPairs::HL))) & 0b1111;
        result = static_cast<uint8_t>(upper << 4) | lower;
        this->memory->writeByte(this->getCombined(RegisterPairs::HL), result);
        break;
    
    default:
        throw std::runtime_error("Invalid target at SWAP instruction.");
        break;
    }

    changeFlag(result == 0, false, false, false);

    return this->regs.PC+2;

}

// JP instruction that receives a boolean condition to decide if it should jump to the designed address of the following 2 bytes.
uint16_t CPU::JP(bool jump){

    if (jump){
        uint8_t lsByte = this->memory->readByte(this->regs.PC+1);
        uint8_t msByte = this->memory->readByte(this->regs.PC+2);

        return static_cast<uint16_t>(msByte << 8 ) | static_cast<uint16_t>(lsByte);
    }
    else return this->regs.PC+3;
}

uint16_t CPU::JR(bool jump){

    if (jump){
        int8_t offset = static_cast<int8_t>(this->memory->readByte(this->regs.PC+1));
        return this->regs.PC + 2 + offset;

    }else return this->regs.PC+2;
}

uint16_t CPU::JPHL(){

    return this->getCombined(RegisterPairs::HL);
    
}

// LD instruction that receives a LoadByte and do the logic with it.
uint16_t CPU::LD(const LoadByte& loadByte){

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
        sourceValue16 = this->memory->readWord(this->regs.PC+1);
        toReturn += 2;
        break;
    case LoadSource::A16I:
        sourceValue = this->memory->readByte(this->memory->readWord(this->regs.PC+1));
        toReturn += 2;
        break;
    case LoadSource::D8:
        sourceValue = this->memory->readByte(this->regs.PC+1);
        toReturn++;
        break;
    case LoadSource::HLI:
        sourceValue = this->memory->readByte(getCombined(RegisterPairs::HL));
        break;
    case LoadSource::BCI:
        sourceValue = this->memory->readByte(getCombined(RegisterPairs::BC));
        break;
    case LoadSource::DEI:
        sourceValue = this->memory->readByte(getCombined(RegisterPairs::DE));
        break;
    case LoadSource::HLDEC:
        sourceValue = this->memory->readByte(getCombined(RegisterPairs::HL));
        this->setCombined(RegisterPairs::HL, this->getCombined(RegisterPairs::HL)-1);
        break;
    case LoadSource::HLINC:
        sourceValue = this->memory->readByte(getCombined(RegisterPairs::HL));
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
        this->setCombined(RegisterPairs::BC, sourceValue16);
        break;
    case LoadTarget::DE:
        this->setCombined(RegisterPairs::DE, sourceValue16);
        break;
    case LoadTarget::HL:
        this->setCombined(RegisterPairs::HL, sourceValue16);
        break;
    case LoadTarget::D8:
        this->memory->writeByte(this->memory->readWord(this->regs.PC+1), sourceValue);
        toReturn += 1;
        break;
    case LoadTarget::D16:
        this->memory->writeWord(this->memory->readWord(this->regs.PC+1), sourceValue16);
        toReturn += 2;
        break;
    case LoadTarget::HLINC:
        this->memory->writeByte(this->getCombined(RegisterPairs::HL), sourceValue);
        this->setCombined(RegisterPairs::HL, this->getCombined(RegisterPairs::HL)+1);
        break;
    case LoadTarget::HLDEC:
        this->memory->writeByte(this->getCombined(RegisterPairs::HL), sourceValue);
        this->setCombined(RegisterPairs::HL, this->getCombined(RegisterPairs::HL)-1);
        break;
    case LoadTarget::BCI:
        this->memory->writeByte(this->getCombined(RegisterPairs::BC), sourceValue);
        break;
    case LoadTarget::DEI:
        this->memory->writeByte(this->getCombined(RegisterPairs::DE), sourceValue);
        break;
    case LoadTarget::HLI:
        this->memory->writeByte(this->getCombined(RegisterPairs::HL), sourceValue);
        break;
    case LoadTarget::SP:
        this->regs.SP = sourceValue16;
        break;
    case LoadTarget::A16:
        this->memory->writeByte(this->memory->readWord(this->regs.PC+1), (this->regs.SP & 0xFF));
        this->memory->writeByte(this->memory->readWord(this->regs.PC+1)+1, static_cast<uint8_t>(this->regs.SP >> 8));
        toReturn +=2;
        break;
    case LoadTarget::A16I:
        this->memory->writeByte(this->memory->readWord(this->regs.PC+1), this->regs.A);
        toReturn += 2;
        break;
    default:
        throw std::runtime_error("Not implemented target on load.");
    }

    return toReturn;
}

uint16_t CPU::LDHLSP(const LoadByte& loadbyte){
    int8_t value = this->memory->readByte(this->regs.PC+1);

    if (value >= 0){
        auto [result, overflow] = overflowSum(this->regs.SP, value);
        
        changeFlag(result == 0, 0, ((this->regs.SP & 0xF) + (value & 0xF)) > 0xF, overflow);
        
        this->regs.SP = result;
        this->setCombined(RegisterPairs::HL, result);
    }
    else {
        auto [result, overflow] = underflowSub(this->regs.SP, abs(value));

        changeFlag(result == 0, 0, ((this->regs.SP & 0xF) < (abs(value) & 0xF)), overflow);
        
        this->regs.SP = result;
        this->setCombined(RegisterPairs::HL, result);
    }

    return this->regs.PC+2;

}

uint16_t CPU::LDH(const LoadByte& loadbyte){

    uint8_t sourceValue = 0;
    uint16_t toReturn = this->regs.PC+1;

    switch (loadbyte.loadSource)
    {
    case LoadSource::A:
        sourceValue = this->regs.A;
        break;
    case LoadSource::C:
        sourceValue = this->memory->readByte(0xFF00 + static_cast<uint16_t>(this->regs.C));
        break;
    case LoadSource::D8:
        sourceValue = this->memory->readByte(0xFF00 + static_cast<uint16_t>(this->memory->readByte(this->regs.PC+1)));
        toReturn++;
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
        this->memory->writeByte(0xFF00+ static_cast<uint16_t>(this->regs.C), sourceValue);
        break;
    case LoadTarget::D8:
        this->memory->writeByte(0xFF00 + static_cast<uint16_t>(this->memory->readByte(this->regs.PC+1)), sourceValue);
        toReturn++;
        break;
    default:
        throw std::runtime_error("Invalid load target at LDH instruction.");
        break;
    }

    return toReturn;
}

void CPU::PUSH(uint16_t value){

    this->regs.SP -= 1;
    this->memory->writeByte(this->regs.SP, (value & 0xFF00)>>8);

    this->regs.SP -= 1;
    this->memory->writeByte(this->regs.SP, (value & 0xFF));

}

uint16_t CPU::POP(){

    uint16_t lsByte = this->memory->readByte(this->regs.SP);
    this->regs.SP += 1;
    
    uint16_t msByte = this->memory->readByte(this->regs.SP);
    this->regs.SP += 1;

    return (msByte << 8) | lsByte;
    
}

uint16_t CPU::POPAF(){

    uint8_t popped = this->memory->readByte(this->regs.SP);
    changeFlag((popped >> 7) & 1, (popped >> 6) & 1, (popped >> 5) & 1, (popped >> 4) & 1);
    this->regs.SP ++;
    this->regs.A = this->memory->readByte(this->regs.SP);
    this->regs.SP ++;

    return this->regs.PC+1;
}

uint16_t CPU::POP16(const RegisterPairs& pair){

    if (pair == RegisterPairs::AF) return this->POPAF();
    uint8_t popped = this->memory->readByte(this->regs.SP);
    switch (pair)
    {
    case RegisterPairs::BC:
        this->regs.C = popped;
        break;
    case RegisterPairs::DE:
        this->regs.E = popped;
        break;
    case RegisterPairs::HL:
        this->regs.L = popped;
        break;
    
    default:
        break;
    }

    this->regs.SP ++;
    popped = this->memory->readByte(this->regs.SP);
    
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
    this->regs.SP ++;

    return this->regs.PC+1;
}


uint16_t CPU::PUSH16(const RegisterPairs& pair){

    this->regs.SP --;
    switch (pair)
    {
    case RegisterPairs::AF:
        this->memory->writeByte(this->regs.SP, this->regs.A);
        break;
    case RegisterPairs::BC:
        this->memory->writeByte(this->regs.SP, this->regs.B);
        break;
    case RegisterPairs::DE:
        this->memory->writeByte(this->regs.SP, this->regs.D);
        break;
    case RegisterPairs::HL:
        this->memory->writeByte(this->regs.SP, this->regs.H);
        break;
    
    default:
        break;
    }
    this->regs.SP --;
    switch (pair)
    {
    case RegisterPairs::AF:
        this->memory->writeByte(this->regs.SP, this->regs.F);
        break;
    case RegisterPairs::BC:
        this->memory->writeByte(this->regs.SP, this->regs.C);
        break;
    case RegisterPairs::DE:
        this->memory->writeByte(this->regs.SP, this->regs.E);
        break;
    case RegisterPairs::HL:
        this->memory->writeByte(this->regs.SP, this->regs.L);
        break;
    
    default:
        break;
    }

    return this->regs.PC+1;
}

uint16_t CPU::CALL(bool jump){
    uint16_t nextPC = this->regs.PC + 3;
    if (jump){
        this->PUSH(nextPC);
        return this->memory->readWord(this->regs.PC+1);
    } else return nextPC;
}

uint16_t CPU::RET(bool jump){
    if (jump) return (this->POP());
    else return this->regs.PC+1;
}

uint16_t CPU::RETI(){
    this->EI();
    return (this->POP());
}

uint16_t CPU::RST(const Vectors& vector){

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

uint16_t CPU::CCF(){

    changeFlag((this->regs.F >> 7) & 1, false, true, ~((this->regs.F >> 4) & 1));

    return this->regs.PC+1;
}

uint16_t CPU::SCF(){

    changeFlag((this->regs.F >> 7) & 1, false, false, true);

    return this->regs.PC+1;
}

uint16_t CPU::DI(){
    this->IME = false;
    return this->regs.PC+1;
}

uint16_t CPU::EI(){
    this->IME = true;
    return this->regs.PC+1;
}

uint16_t CPU::HALT(){

    uint8_t pendingInterrupts = this->getHardwareRegister(HardwareRegisters::IE) & this->getHardwareRegister(HardwareRegisters::IF) & 0x1F;

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

uint16_t CPU::DAA(){

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

uint16_t CPU::STOP(){

    uint8_t op = this->memory->readByte(this->regs.PC+1);

    this->isStopped = true;

    /* The following code will be used if i someday am able to add Gameboy Color support:

    if (this->isGameBoyColor){
        if (this->key1Reg & 0x01){
            this->speedDoubler = !this->speedDoubler;

            this->key1Reg &= 0xFE;

            if (this->speedDoubler){
                this->key1Reg |= 0x80;
            }
            else {
                this->key1Reg &= 0x7F;
            }
        }
    }

    */

    return this->regs.PC+1;
}

uint16_t CPU::getCombined(RegisterPairs pair){
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

void CPU::setCombined(RegisterPairs pair, uint16_t value){
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

void CPU::changeFlag(bool zero, bool subtract, bool halfCarry, bool carry){

    zero? regs.F |= 0b10000000 : regs.F &= ~0b10000000;
    subtract? regs.F |= 0b01000000 : regs.F &= ~0b01000000;
    halfCarry? regs.F |= 0b00100000 : regs.F &= ~0b00100000;
    carry? regs.F |= 0b00010000 : regs.F &= ~0b00010000;

}

uint16_t CPU::execute(const Instruction& instruction){

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
            jumpCondition = !((this->regs.F >> 7 & 1) == 1);
            break;
        case JumpTest::Zero:
            jumpCondition = (this->regs.F>>7 & 1) == 1;
            break;
        case JumpTest::NotCarry:
            jumpCondition = !((this->regs.F >> 4  & 1) == 1);
            break;
        case JumpTest::Carry:
            jumpCondition = (this->regs.F>>4 & 1) == 1;
            break;
        case JumpTest::Always:
            jumpCondition = true;
            break;
        }
        jumpCondition ? this->clock->addCycles(3 * MCYCLESIZE) : this->clock->addCycles(2 * MCYCLESIZE);
        return JP(jumpCondition);

    case InstructionType::LD:
        return LD(instruction.loadByte);

    case InstructionType::PUSH:

        return this->PUSH16(instruction.pair);
    
    case InstructionType::POP:
        
        return this->POP16(instruction.pair);

    case InstructionType::CALL:
        switch (instruction.jumpTest)
        {
        case JumpTest::NotZero:
            jumpCondition = !((this->regs.F >> 7 & 1) == 1);
            break;
        case JumpTest::Zero:
            jumpCondition = (this->regs.F>>7 & 1) == 1;
            break;
        case JumpTest::NotCarry:
            jumpCondition = !((this->regs.F >> 4  & 1) == 1);
            break;
        case JumpTest::Carry:
            jumpCondition = (this->regs.F>>4 & 1) == 1;
            break;
        case JumpTest::Always:
            jumpCondition = true;
            break;
        }
        jumpCondition ? this->clock->addCycles(6 * MCYCLESIZE) : this->clock->addCycles(3 * MCYCLESIZE);
        return this->CALL(jumpCondition);

    case InstructionType::RET:
        switch (instruction.jumpTest)
        {
        case JumpTest::NotZero:
            jumpCondition = !((this->regs.F>>7 & 1) == 1);
            break;
        case JumpTest::Zero:
            jumpCondition = (this->regs.F>>7 & 1) == 1;
            break;
        case JumpTest::NotCarry:
            jumpCondition = !((this->regs.F>>4 & 1) == 1);
            break;
        case JumpTest::Carry:
            jumpCondition = (this->regs.F>>4 & 1) == 1;
            break;
        case JumpTest::Always:
            jumpCondition = true;
            break;
        }
        jumpCondition ? this->clock->addCycles(5 * MCYCLESIZE) : this->clock->addCycles(2 * MCYCLESIZE);
        return this->RET(jumpCondition);

    case InstructionType::NOP:
        return this->regs.PC+1;

    case InstructionType::LDH:
        return this->LDH(instruction.loadByte);
    case InstructionType::INC:
        return this->INC(instruction.arithmetic);
    case InstructionType::DEC:
        return this->DEC(instruction.arithmetic);
    case InstructionType::RLCA:
        return this->RLCA();
    case InstructionType::RRCA:
        return this->RRCA();
    case InstructionType::STOP:
        return this->STOP();
    case InstructionType::RLA:
        return this->RLA();
    case InstructionType::JR:
        switch (instruction.jumpTest)
        {
        case JumpTest::NotZero:
            jumpCondition = !((this->regs.F>>7 & 1) == 1);
            break;
        case JumpTest::Zero:
            jumpCondition = (this->regs.F>>7 & 1) == 1;
            break;
        case JumpTest::NotCarry:
            jumpCondition = !((this->regs.F>>4 & 1) == 1);
            break;
        case JumpTest::Carry:
            jumpCondition = (this->regs.F>>4 & 1) == 1;
            break;
        case JumpTest::Always:
            jumpCondition = true;
            break;
        }
        jumpCondition ? this->clock->addCycles(3 * MCYCLESIZE) : this->clock->addCycles(2 * MCYCLESIZE);
        return this->JR(jumpCondition);
    case InstructionType::RRA:
        return this->RRA();
    case InstructionType::DAA:
        return this->DAA();
    case InstructionType::CPL:
        return this->CPL();
    case InstructionType::SCF:
        return this->SCF();
    case InstructionType::CCF:
        return this->CCF();
    case InstructionType::AND:
        return this->AND(instruction.bitSource);
    case InstructionType::XOR:
        return this->XOR(instruction.bitSource);
    case InstructionType::OR:
        return this->OR(instruction.bitSource);
    case InstructionType::CP:
        return this->CP(instruction.bitSource);
    case InstructionType::RST:
        return this->RST(instruction.vector);
    case InstructionType::RETI:
        return this->RETI();
    case InstructionType::DI:
        return this->DI();
    case InstructionType::RLC:
        return this->RLC(instruction.rotateTarget);
    case InstructionType::RRC:
        return this->RRC(instruction.rotateTarget);
    case InstructionType::RL:
        return this->RL(instruction.rotateTarget);
    case InstructionType::RR:
        return this->RR(instruction.rotateTarget);
    case InstructionType::SLA:
        return this->SLA(instruction.rotateTarget);
    case InstructionType::SRA:
        return this->SRA(instruction.rotateTarget);
    case InstructionType::SWAP:
        return this->SWAP(instruction.rotateTarget);
    case InstructionType::SRL:
        return this->SRL(instruction.rotateTarget);
    case InstructionType::BIT:
        return this->BIT(instruction.bitFlag);
    case InstructionType::RES:
        return this->RES(instruction.bitFlag);
    case InstructionType::SET:
        return this->SET(instruction.bitFlag);
    case InstructionType::JPHL:
        return this->JPHL();
    case InstructionType::LDHLSP:
        return this->LDHLSP(instruction.loadByte);
    case InstructionType::EI:
        return this->EI();
    default:
        throw std::runtime_error("Not implemented instruction.");
        return this->regs.PC+1;
    }
}

void CPU::step(){

    if (this->isStopped){

        this->memory->writeByte(this->getHardwareRegister(HardwareRegisters::DIV), 0);

        /* Will use this when i am able to identify input;

        if (checkKeyInterrupt()){
            this->isStopped = false;
        }
        */

        #ifdef DEBUG

        std::cout << "CPU is stopped.";

        #endif

        return;

    }

    if (this->isHalted){

        #ifdef DEBUG

        std::cout << "CPU is halted.";

        #endif

        uint8_t activeInterrupts = this->getHardwareRegister(HardwareRegisters::IE) & this->getHardwareRegister(HardwareRegisters::IF) & 0x1F;

        if (activeInterrupts > 0){
            this->isHalted = false;
        }
        else {
            return;
        }
    }

    uint16_t instructionByte = this->memory->readByte(this->regs.PC);
    bool prefixed = (instructionByte == 0xCB);


    if (prefixed){
        instructionByte = this->memory->readByte(this->regs.PC+1);
    }

    auto instructionOpt = Instruction::getFromByte(instructionByte, prefixed);

    #ifdef DEBUG

    std::cout << "step() function called with PC register set as: "<< this->regs.PC << std::endl;
    std::cout << "Instruction byte: 0x" << std::hex << instructionByte << std::endl;

    logFile << std::hex << instructionByte << " instruction at PC " << this->regs.PC << std::endl;

    #endif

    #ifdef BLARGGTESTROMLOG

    logFile << "A:" << std::hex << std::setfill('0') << std::uppercase << std::setw(2) << +this->regs.A;
    logFile << " F:" << std::hex << std::setfill('0') << std::uppercase<< std::setw(2) << +this->regs.F;
    logFile << " B:" << std::hex << std::setfill('0') << std::uppercase<< std::setw(2) << +this->regs.B;
    logFile << " C:" << std::hex << std::setfill('0') << std::uppercase<< std::setw(2) << +this->regs.C;
    logFile << " D:" << std::hex << std::setfill('0') << std::uppercase<< std::setw(2) << +this->regs.D;
    logFile << " E:" << std::hex << std::setfill('0') << std::uppercase<< std::setw(2) << +this->regs.E;
    logFile << " H:" << std::hex << std::setfill('0') << std::uppercase<< std::setw(2) << +this->regs.H;
    logFile << " L:" << std::hex << std::setfill('0') << std::uppercase<< std::setw(2) << +this->regs.L;
    logFile << " SP:" << std::hex << std::setfill('0') << std::uppercase<< std::setw(4) << +this->regs.SP;
    logFile << " PC:" << std::hex << std::setfill('0') << std::uppercase<< std::setw(4) << +this->regs.PC;
    logFile << " PCMEM:" << std::hex << std::setfill('0') << std::uppercase<< std::setw(2) << +this->memory->readByte(this->regs.PC) << "," << std::hex << std::setfill('0') << std::uppercase<< std::setw(2) << +this->memory->readByte(this->regs.PC+1) << "," << std::hex << std::setfill('0') << std::uppercase<< std::setw(2) << +this->memory->readByte(this->regs.PC+2) << "," << std::hex << std::setfill('0') << std::uppercase<< std::setw(2) << +this->memory->readByte(this->regs.PC+3) << std::endl;;

    #endif

    if (instructionOpt.has_value()){
        this->clock->addCycles(instructionOpt.value().mcycles * MCYCLESIZE);
        if (this->haltBug){
            this->haltBug = false;

            execute(instructionOpt.value());
        } else {
            this->regs.PC = execute(instructionOpt.value());
        }
    }
    else {
        throw std::runtime_error("Unknown instruction: 0x" + std::to_string(instructionByte));
    }

}