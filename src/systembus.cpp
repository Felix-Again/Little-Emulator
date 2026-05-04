#include "systembus.hpp"

SystemBus::SystemBus(){

    cpu = std::make_unique<CPU>(this);
    gpu = std::make_unique<GPU>(this);
    memory = std::make_unique<MemoryBus>(this);
    cycleCounter = std::make_unique<CycleCounter>(600, 4194304);

    memory->linkPointers(cpu.get(), gpu.get(), cycleCounter.get());
    cpu->linkPointers(gpu.get(), memory.get(), cycleCounter.get());
    gpu->linkPointers(cpu.get(), memory.get(), cycleCounter.get());
}

SystemBus::~SystemBus() = default;