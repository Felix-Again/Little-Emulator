#ifndef MEMORYBUS_HPP
#define MEMORYBUS_HPP
#include "systembus.hpp"
#include "cpu.hpp"
#include "gpu.hpp"
#include "globalConstants.hpp"
#include "cyclecounter.hpp"
#include <vector>

class GPU;
class CPU;
class CycleCounter;
class SystemBus;

class MemoryBus{

    public:
        CycleCounter* clock;
        GPU* gpu;
        CPU* cpu;
        SystemBus* systemBus;

        MemoryBus(SystemBus* systembus);

        void linkPointers(CPU* cpu, GPU* gpu, CycleCounter* cycleCounter);

        uint8_t readByte(uint16_t address);

        uint16_t readWord(uint16_t address);

        void OamDmaTransfer(uint8_t value);

        void writeByte(uint16_t address, uint8_t value);

        void writeWord(uint16_t address, uint16_t value);

    private:

        std::vector<uint8_t> memory;
        
};
#endif