#include "gpu.hpp"


GPU::GPU(SystemBus* systemBus) : systemBus(systemBus), vram(VRAM_SIZE), oam(OAM_SIZE){

    this->vramModified = false;
}

void GPU::linkPointers(CPU* cpu, MemoryBus* memoryBus, CycleCounter* cycleCounter){

    this->cpu = cpu;
    this->memory = memoryBus;
    this->clock = cycleCounter;

}

uint8_t GPU::readVram(uint16_t address){
    if (address >= VRAM_BEGIN) address -= VRAM_BEGIN;
    return this->vram[address];
}

uint8_t GPU::readOam(uint16_t address){
    if (address >= OAM_BEGIN) address -= OAM_BEGIN;
    return this->oam[address];
}

void GPU::writeVram(uint16_t address, uint8_t value){

    if (address >= VRAM_BEGIN) address -= VRAM_BEGIN;

    if (address >= 0x1800) return;
    
    this->vramModified = true;

    this->vram[address] = value;

}

void GPU::writeOam(uint16_t address, uint8_t value){
    if (address >= OAM_BEGIN) address -= OAM_BEGIN;

    // bool priorityBit = (value >> 7) & 1;
    // bool yFlipBit = (value >> 6) & 1;
    // bool xFlipBit = (value >> 5) & 1;
    // bool DMGPalette = (value >> 4) & 1;
    // bool bankBit = (value >> 3) & 1;

    this->oam[address] = value;
    //this->clock->addCycles(MCYCLESIZE);

}

enum class GPU::Layers{
    Background,
    Window,
    Objects
};

Tile GPU::emptyTile(){
    Tile tile;

    for (int i = 0; i < 8; i ++){
        for (int j = 0; j < 8; j++){
            tile.tiles[i][j] = TilePixel::Zero;
        }
    }

    return tile;
}

void GPU::update(){
    this->vramModified = true;
}







