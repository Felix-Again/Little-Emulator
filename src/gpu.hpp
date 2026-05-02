#ifndef GPU_HPP
#define GPU_HPP
#include "systembus.hpp"
#include "cpu.hpp"
#include "memorybus.hpp"
#include "globalConstants.hpp"
#include "cyclecounter.hpp"
#include <vector>

class CPU;
class MemoryBus;
class CycleCounter;
class SystemBus;

enum class TilePixel{
    Zero,
    One, 
    Two,
    Three
};

struct Tile{
    TilePixel tiles[8][8];
};

class GPU{
    
    public:
        CycleCounter* clock;
        CPU* cpu;
        MemoryBus* memory;
        SystemBus* systemBus;

        bool vramModified;

        GPU(SystemBus* systemBus);
        void linkPointers(CPU* cpu, MemoryBus* memoryBus, CycleCounter* cycleCounter);

        uint8_t readVram(uint16_t address);

        uint8_t readOam(uint16_t address);

        void writeVram(uint16_t address, uint8_t value);

        void writeOam(uint16_t address, uint8_t value);

        enum class Layers;

        Tile tileset[384];

        Tile emptyTile();

        void update();

    private:

        std::vector<uint8_t> vram;

        std::vector<uint8_t> oam;

};
#endif