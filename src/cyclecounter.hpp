#ifndef CYCLECOUNTER_HPP
#define CYCLECOUNTER_HPP
#include "systembus.hpp"
#include "cpu.hpp"
#include "gpu.hpp"
#include "memorybus.hpp"
#include "globalConstants.hpp"

class GPU;
class MemoryBus;
class CPU;
class SystemBus;

class CycleCounter{

    public:

        CycleCounter(int targetFPS, uint32_t frequency);

        void addCycles(uint32_t cycles);

        bool frameComplete() const;

        void sync();

        uint32_t getCycles() const;

    private:

        Uint64 lastTick;
        uint32_t currentCycles;
        uint32_t cyclesPerFrame;
        uint32_t targetFrameTime;
};
#endif