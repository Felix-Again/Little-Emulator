#ifndef CPU_HPP
#define CPU_HPP
#include "systembus.hpp"
#include "gpu.hpp"
#include "memorybus.hpp"
#include "cyclecounter.hpp"
#include <cstdint>
#include <stdexcept>
#include <iomanip>
#include <optional>
#include <iostream>
#include <bitset>

class GPU;
class MemoryBus;
class CycleCounter;
class SystemNus;

class CPU{
    public:
        CycleCounter* clock;
        GPU* gpu;
        MemoryBus* memory;
        SystemBus* systemBus;

        CPU(SystemBus* systemBus);
        
        void linkPointers(GPU* gpu, MemoryBus* memoryBus, CycleCounter* cycleCounter);
        
        void executeASM();

        enum class HardwareRegisters;

        uint8_t getHardwareRegister(const HardwareRegisters& hardwareRegister);

    private:

        bool isHalted;
        bool isStopped;
        bool IME;
        bool haltBug;

        struct Registers {
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

        enum class Vectors;
        
        enum class RegisterPairs : uint16_t;

        enum class InstructionType;

        enum class Target;

        enum class ArithmeticTarget;

        enum class ArithmeticSource;

        enum class BitwiseSource;

        enum class BitFlagSource;

        enum class JumpTest;
        
        enum class LoadTarget;

        enum class LoadSource;
        
        enum class StackTarget;

        enum class RotateTarget;

        struct Arithmetic;

        struct LoadByte;

        struct BitFlag;

        struct Instruction;

        uint8_t* get8BitReg(const std::string& name);

        uint8_t* get8BitReg(const Target& target);

        uint16_t* get16BitReg(const std::string& name);

        uint16_t* get16BitReg(const Target& target);

        uint16_t extract16BitReg(const std::string& name);

        uint16_t extract16BitReg(const Target& target);

        std::pair<uint16_t, bool> overflowSum(uint16_t val1, uint16_t val2);

        std::pair<uint8_t, bool> overflowSum(uint8_t val1, uint8_t val2);

        std::pair<uint16_t, bool> overflowSub(uint16_t val1, uint16_t val2);
        
        std::pair<uint8_t, bool> overflowSub(uint8_t val1, uint8_t val2);

        uint16_t ADD(const Arithmetic& arithmetic);
        uint16_t ADC(const Arithmetic& arithmetic);
        uint16_t SUB(const Arithmetic& arithmetic);
        uint16_t SBC(const Arithmetic& arithmetic);
        uint16_t CP(const BitwiseSource& bitsource);
        uint16_t DEC(const Arithmetic& arithmetic);
        uint16_t INC(const Arithmetic& arithmetic);
        uint16_t AND(const BitwiseSource& bitsource);
        uint16_t CPL();
        uint16_t OR(const BitwiseSource& bitsource);
        uint16_t XOR(const BitwiseSource& bitsource);
        uint16_t BIT(const BitFlag& bitFlag);
        uint16_t RES(const BitFlag& bitFlag);
        uint16_t SET(const BitFlag& bitFlag);
        uint16_t RL(const RotateTarget& target);
        uint16_t RLC(const RotateTarget& target);
        uint16_t RR(const RotateTarget& target);
        uint16_t RRC(const RotateTarget& target);
        uint16_t SLA(const RotateTarget& target);
        uint16_t SRA(const RotateTarget& target);
        uint16_t SRL(const RotateTarget& target);
        uint16_t SWAP(const RotateTarget& target);
        uint16_t JP(bool jump);
        uint16_t JR(bool jump);
        uint16_t JPHL();
        uint16_t LD(const LoadByte& loadByte);
        uint16_t LDHLSP(const LoadByte& loadByte);
        uint16_t LDH(const LoadByte& loadByte);
        void PUSH(uint16_t value);
        uint16_t POP();
        uint16_t POPAF();
        uint16_t POP16(const RegisterPairs& pair);
        uint16_t PUSH16(const RegisterPairs& pair);
        uint16_t CALL(bool jump);
        uint16_t RET(bool jump);
        uint16_t RETI();
        uint16_t RST(const Vectors& vector);
        uint16_t CCF();
        uint16_t SCF();
        uint16_t DI();
        uint16_t EI();
        uint16_t HALT();
        uint16_t DAA();
        uint16_t STOP();
        uint16_t getCombined(RegisterPairs pair);
        void setCombined(RegisterPairs pair, uint16_t value);
        void changeFlag(bool zero, bool subtract, bool halfCarry, bool carry);
        uint16_t execute(const Instruction& instruction);
        void step();
};
#endif