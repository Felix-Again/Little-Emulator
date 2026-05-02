#ifndef GLOBALCONSTANTS_HPP
#define GLOBALCONSTANTS_HPP
#include <cstdint>
#include <SDL3/SDL.h>

static const uint16_t VRAM_BEGIN = 0x8000;
static const uint16_t VRAM_END = 0x9FFF;
static const uint16_t VRAM_SIZE = VRAM_END - VRAM_BEGIN + 1;

static const uint16_t TILEMAP_BEGIN = 0x9800;
static const uint16_t TILEMAP_END = 0x9BFF;

static const uint16_t WRAM_BEGIN = 0xC000;
static const uint16_t WRAM_END = 0xDFFF;

static const uint16_t ECHORAM_BEGIN = 0xE000;
static const uint16_t ECHORAM_END = 0xFDFF;

static const uint16_t OAM_BEGIN = 0xFE00;
static const uint16_t OAM_END = 0xFE9F;
static const uint16_t OAM_SIZE  = OAM_END - OAM_BEGIN + 1;
static const uint16_t OAM_DMA_TRANSFER_BYTE = 0xFF46;

static const uint64_t TCYCLESIZE = 0;
static const uint64_t MCYCLESIZE = 4 * TCYCLESIZE;

static const int PIXEL_SIZE = 4;

static const Uint32 ScreenColor[] = {
    0xFF081820,
    0xFF88C070,
    0xFF346856,
    0xFFE0F8D0
};
#endif