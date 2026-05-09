#ifndef SYSTEMBUS_HPP
#define SYSTEMBUS_HPP
#include "cpu.hpp"
#include "gpu.hpp"
#include "memorybus.hpp"
#include "globalConstants.hpp"
#include "cyclecounter.hpp"
#include <memory>
#include <cstdint>

class GPU;
class MemoryBus;
class CycleCounter;
class CPU;

class SystemBus{
    public:
    std::unique_ptr<CPU> cpu;
    std::unique_ptr<GPU> gpu;
    std::unique_ptr<MemoryBus> memory;
    std::unique_ptr<CycleCounter> cycleCounter;

    SystemBus();
    ~SystemBus();
};
#endif