#include "gpu.hpp"

GPU::GPU(SystemBus *systemBus) : systemBus(systemBus), vram(VRAM_SIZE), oam(OAM_SIZE)
{

    this->vramModified = false;
    this->oamModified = false;
    this->previousInterruptLine = false;

    this->totalCycles = 0;
    this->currentMode = 1;
}

void GPU::linkPointers(CPU *cpu, MemoryBus *memoryBus, CycleCounter *cycleCounter)
{

    this->cpu = cpu;
    this->memory = memoryBus;
    this->clock = cycleCounter;
}

uint8_t GPU::readVram(uint16_t address)
{
    if (address >= VRAM_BEGIN)
        address -= VRAM_BEGIN;
    return this->vram[address];
}

uint8_t GPU::readOam(uint16_t address)
{
    if (address >= OAM_BEGIN)
        address -= OAM_BEGIN;

    // Byte 0 = Y position
    // Byte 1 = X position
    // Byte 2 = Tile index
    // Byte 3 = Attributes/Flags

    return this->oam[address];
}

void GPU::writeVram(uint16_t address, uint8_t value)
{
    this->vramModified = true;

    if (address >= VRAM_BEGIN)
        address -= VRAM_BEGIN;

    this->vram[address] = value;
}

void GPU::writeOam(uint16_t address, uint8_t value)
{
    if (address >= OAM_BEGIN)
        address -= OAM_BEGIN;

    // bool priorityBit = (value >> 7) & 1;
    // bool yFlipBit = (value >> 6) & 1;
    // bool xFlipBit = (value >> 5) & 1;
    // bool DMGPalette = (value >> 4) & 1;
    // bool bankBit = (value >> 3) & 1;

    this->oam[address] = value;
    // this->clock->addCycles(MCYCLESIZE);
}

enum class GPU::Layers
{
    Background,
    Window,
    Objects
};

Tile GPU::emptyTile()
{
    Tile tile;

    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            tile.tiles[i][j] = TilePixel::Zero;
        }
    }

    return tile;
}

void GPU::update()
{
    this->vramModified = true;
}

void GPU::updatePixelBuffer(Uint32 *pixelBuffer)
{

    uint8_t LCDC = this->cpu->getHardwareRegister(CPU::HardwareRegisters::LCDC);
    uint8_t LY = this->cpu->getHardwareRegister(CPU::HardwareRegisters::LY);

    this->clock->addCycles(1 * TCYCLESIZE);

    uint16_t WindowTileMapAreaStart = (((LCDC >> 6) & 0x1) ? 0x9C00 : 0x9800);
    bool WindowEnable = (LCDC >> 5) & 0x1;
    uint16_t BGWindowTileMapDataArea = (LCDC >> 4) & 0x1;
    uint16_t BGtileAreaStart = (((LCDC >> 3) & 0x1) ? 0x9C00 : 0x9800);
    uint8_t OBJheight = (((LCDC >> 2) & 0x1) ? 8 : 16); // 0 = 8x8; 1 = 8x16
    bool OBJenable = (LCDC >> 1) & 0x1;
    bool BGWindowEnable = LCDC & 0x1;

    // uint16_t BGtileAreaEnd = (BGtileAreaStart == TILEMAP1_BEGIN) ? TILEMAP1_END : TILEMAP2_END;

    if (BGWindowEnable)
    {

        uint8_t SCY = this->cpu->getHardwareRegister(CPU::HardwareRegisters::SCY);
        uint8_t SCX = this->cpu->getHardwareRegister(CPU::HardwareRegisters::SCX);

        uint8_t WY = this->cpu->getHardwareRegister(CPU::HardwareRegisters::WY);
        uint8_t WX = this->cpu->getHardwareRegister(CPU::HardwareRegisters::WX);

        uint8_t byte1 = 0;
        uint8_t byte2 = 0;

        uint8_t tileIndex = 0;

        for (int x = 0; x < 160; x++)
        {

            uint16_t mapAddress;
            uint8_t tileX, tileY, tileLine;

            bool isWindow = (WindowEnable && LY >= WY && x >= (WX - 7));

            if (isWindow)
            {
                tileX = (x - (WX - 7)) / 8;
                tileY = (LY - WY) / 8;
                tileLine = (LY - WY) % 8;
                mapAddress = WindowTileMapAreaStart + (tileY * 32) + tileX;
            }
            else
            {
                uint8_t Ypos = (LY + SCY) % 256;
                uint8_t Xpos = (x + SCX) % 256;
                tileX = Xpos / 8;
                tileY = Ypos / 8;
                tileLine = Ypos % 8;
                mapAddress = BGtileAreaStart + (tileY * 32) + tileX;
            }

            tileIndex = this->memory->readByte(mapAddress);
            uint16_t tileDataAddress;

            if (BGWindowTileMapDataArea & 0x1)
            {
                tileDataAddress = 0x8000 + (tileIndex * 16) + (tileLine * 2);
            }
            else
            {
                int8_t signedIndex = static_cast<int8_t>(tileIndex);
                tileDataAddress = 0x9000 + (signedIndex * 16) + (tileLine * 2);
            }

            uint8_t colorIndex;
            int bitShift;

            if (x == 0 || tileX % 8 == 0)
            {

                byte1 = this->memory->readByte(tileDataAddress);
                byte2 = this->memory->readByte(tileDataAddress + 1);
            }
            
            bitShift = 7 - (isWindow ? (x - (WX - 7)) % 8 : (x + SCX) % 8);

            colorIndex = (((byte2 >> bitShift) & 0x1) << 1) | ((byte1 >> bitShift) & 0x1);

            uint8_t BGP = this->cpu->getHardwareRegister(CPU::HardwareRegisters::BGP);

            uint8_t paletteColor = (BGP >> (colorIndex * 2)) & 0x3;

            pixelBuffer[LY * 160 + x] = ScreenColor[paletteColor];
        }
    }

    if (OBJenable)
    {
        for (int i = OAM_BEGIN; i < OAM_END; i += 4)
        {
            uint8_t byte0 = this->readOam(i);     // Y position + 16
            uint8_t byte1 = this->readOam(i + 1); // X position + 8
            uint8_t byte2 = this->readOam(i + 2); // Tile index
            uint8_t byte3 = this->readOam(i + 3); // Attributes

            if (byte1 == 0 || byte1 >= 168)
            {
                continue;
            }

            const Uint32 *pallete = ((byte3 >> 4) & 0x1 ? OBJ_PALLETE_1 : OBJ_PALLETE_0);

            uint8_t lsTileByte = this->memory->readByte(0x8000 + 16 * byte2 + 16 * LY);
            uint8_t msTileByte = this->memory->readByte(0x8000 + 16 * byte2 + 16 * LY + 8);

            if ((byte0 + OBJheight > LY + 16) && (byte0 + OBJheight < LY + 24))
            {
                for (int j = 0; j < 8; j++)
                {
                    uint8_t colorOption = (((msTileByte >> (7 - j)) & 0x1) << 1) | (((lsTileByte >> (7 - j)) & 0x1));
                    pixelBuffer[LY * WINDOW_WIDTH + (byte1 - 8) + j] = pallete[colorOption];
                }
            }
        }
    }
}

void GPU::step(uint32_t cycles, Uint32 *pixelBuffer)
{
    uint8_t LCDC = this->cpu->getHardwareRegister(CPU::HardwareRegisters::LCDC);

    if (!(LCDC >> 7) & 0x1)
    {
        this->totalCycles = 0;
        this->cpu->setHardwareRegister(CPU::HardwareRegisters::LY, 0);

        return;
    }

    this->totalCycles += cycles;

    uint8_t LY = this->cpu->getHardwareRegister(CPU::HardwareRegisters::LY);
    uint8_t STAT = this->cpu->getHardwareRegister(CPU::HardwareRegisters::STAT);
    uint8_t LYC = this->cpu->getHardwareRegister(CPU::HardwareRegisters::LYC);

    if (LY >= 144)
    {
        // Mode 1 - VBlank
        std::cout << "VBlank" << std::endl;

        if (this->totalCycles >= 456)
        {
            this->totalCycles -= 456;
            LY++;

            if (LY > 153)
            {
                LY = 0;
            }

            this->cpu->setHardwareRegister(CPU::HardwareRegisters::LY, LY);
        }
    }
    else
    {
        if (this->totalCycles <= 80)
        {
            // Mode 2 - OAM search
            std::cout << "OAM search" << std::endl;
        }
        else if (this->totalCycles <= 252)
        {
            // Mode 3 - Pixel Transfer
            std::cout << "Pixel Transfer" << std::endl;
        }
        else
        {
            // Mode 0 - HBlank
            std::cout << "HBlank" << std::endl;
            if (this->currentMode != 0)
            {
                this->updatePixelBuffer(pixelBuffer);

                // set STAT mode to 0
            }

            if (this->totalCycles >= 456)
            {
                this->totalCycles -= 456;
                LY++;
                this->cpu->setHardwareRegister(CPU::HardwareRegisters::LY, LY);

                if (LY == 144)
                {
                    this->cpu->requestInterrupt(0);
                }
            }
        }
    }

    if (LY == LYC && ((STAT >> 7) & 0x1) == 0x1)
    {
        this->cpu->requestInterrupt(1);
    }
}