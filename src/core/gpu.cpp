#include "gpu.hpp"


GPU::GPU(SystemBus* systemBus) : systemBus(systemBus), vram(VRAM_SIZE), oam(OAM_SIZE){

    this->vramModified = false;
    this->oamModified = false;
    this->previousInterruptLine = false;
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
    

    this->vram[address] = value;

    if (0 <= address && address <= 0x7FF & !this->cpu->booting){
        this->vramModified = true;

        uint8_t index = (address-(address%16))/16;

        TilePixel pixelValue = TilePixel::Zero;
        uint8_t byte1 = this->vram[index];
        uint8_t byte2 = this->vram[index+1];
  
        for (int i = 0; i < 8; i++){
            for (int j = 0; j < 8; j++){

                uint8_t bit1 = (byte1 >> (7 - i));
                uint8_t bit2 = (byte2 >> (7 - j));

                if (bit1 == 0 && bit2 == 0){
                    pixelValue = TilePixel::Zero;
                }
                else if (bit1 == 1 && bit2 == 0){
                    pixelValue = TilePixel::One;
                }
                else if (bit1 == 0 && bit2 == 1){
                    pixelValue = TilePixel::Two;
                }
                else if (bit1 == 1 && bit2 == 1){
                    pixelValue = TilePixel::Three;
                }

                this->tileset[index].tiles[i][j] = pixelValue;
            }
        }

        #ifdef DEBUGGPU
        std::cout << "Modified the TILE in the following address: " << std::hex <<address << " with value of byte1: "<< +byte1 << " and byte2: " << +byte2 << std::endl;
        #endif

    }

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

void GPU::updatePixelBuffer(Uint32* pixelBuffer){

    Uint32 color;
    #ifdef DEBUGGPU
    std::cout << "Updating pixelBuffer" << std::endl;
    #endif

    for (int i = 0; i < 360 ; i++){
        for (int row = 0; row < 8; row ++){
            for (int col = 0; col < 8; col++){
                switch (this->tileset[i].tiles[row][col])
                {
                case TilePixel::Zero:
                    color = ScreenColor[0];
                    break;
                case TilePixel::One:
                    color = ScreenColor[1];
                    break;
                case TilePixel::Two:
                    color = ScreenColor[2];
                    break;
                case TilePixel::Three:
                    color = ScreenColor[3];
                    break;
                
                default:
                    break;
                }

                uint8_t originX = 8*(i % 20);
                uint8_t originY = 8*(i - (i % 20))/20;
                pixelBuffer[((row+originY)*160)+col+originX] = color;
                
            }
        }
        
    }
    #ifdef DEBUGGPU
    std::cout << "Finished updating the pixel buffer" << std::endl;
    #endif

    this->vramModified = false;

}
