#ifndef GPU_HPP
#define GPU_HPP
#include "systembus.hpp"
#include "cpu.hpp"
#include "memorybus.hpp"
#include "globalConstants.hpp"
#include "cyclecounter.hpp"
#include <vector>
#include <math.h>
#include <SDL3/SDL.h>

class CPU;
class MemoryBus;
class CycleCounter;
class SystemBus;

enum class TilePixel
{
    Zero,
    One,
    Two,
    Three
};

struct Tile
{
    TilePixel tiles[8][8];
};

class GPU
{

public:
    CycleCounter *clock;
    CPU *cpu;
    MemoryBus *memory;
    SystemBus *systemBus;

    uint32_t totalCycles;
    uint8_t currentMode;

    bool vramModified;
    bool oamModified;
    bool previousInterruptLine;

    uint8_t scanlineCounter = 0;
    uint8_t scanlineObjectCounter = 0;

    GPU(SystemBus *systemBus);
    void linkPointers(CPU *cpu, MemoryBus *memoryBus, CycleCounter *cycleCounter);

    uint8_t readVram(uint16_t address);

    uint8_t readOam(uint16_t address);

    void writeVram(uint16_t address, uint8_t value);

    void writeOam(uint16_t address, uint8_t value);

    enum class Layers;

    Tile tileset[384];

    Tile emptyTile();

    void update();

    void updatePixelBuffer(Uint32 *pixelBuffer);

    void step(uint32_t cycles, Uint32 *pixelBuffer);

private:
    std::vector<uint8_t> vram;

    std::vector<uint8_t> oam;
};
#endif