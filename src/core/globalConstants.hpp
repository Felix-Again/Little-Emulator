#ifndef GLOBALCONSTANTS_HPP
#define GLOBALCONSTANTS_HPP
#include <cstdint>
#include <SDL3/SDL.h>

static const uint16_t VRAM_BEGIN = 0x8000;
static const uint16_t VRAM_END = 0x9FFF;
static const uint16_t VRAM_SIZE = VRAM_END - VRAM_BEGIN + 1;

static const uint16_t ROM_BEGIN = 0;
static const uint16_t ROM_END = 0x7FFF;

static const uint16_t TILEMAP1_BEGIN = 0x9800;
static const uint16_t TILEMAP1_END = 0x9BFF;
static const uint16_t TILEMAP2_BEGIN = 0x9C00;
static const uint16_t TILEMAP2_END = 0x9FFF;

static const uint16_t WRAM_BEGIN = 0xC000;
static const uint16_t WRAM_END = 0xDFFF;

static const uint16_t ECHORAM_BEGIN = 0xE000;
static const uint16_t ECHORAM_END = 0xFDFF;

static const uint16_t OAM_BEGIN = 0xFE00;
static const uint16_t OAM_END = 0xFE9F;
static const uint16_t OAM_SIZE = OAM_END - OAM_BEGIN + 1;
static const uint16_t OAM_DMA_TRANSFER_BYTE = 0xFF46;

static const uint64_t TCYCLESIZE = 1;
static const uint64_t MCYCLESIZE = 4 * TCYCLESIZE;
static const uint64_t DIV_FREQUENCY = 16384;
static const uint64_t CPU_FREQUENCY = 4194304;

static const int PIXEL_SIZE = 4;

static const Uint32 ScreenColor[] = {
    0xFF000000,
    0xFF555555,
    0xFFAAAAAA,
    0xFFFFFFFF};

static const Uint32 OBJ_PALLETE_0[] = {
    0xFF000000,
    0xFF555555,
    0xFFAAAAAA,
    0xFFFFFFFF};

static const Uint32 OBJ_PALLETE_1[] = {
    0xFF000000,
    0xFF555555,
    0xFFAAAAAA,
    0xFFFFFFFF};

static const uint16_t VBLANK_INTERRUPT = 0x40;
static const uint16_t STAT_INTERRUPT = 0x48;
static const uint16_t TIMER_INTERRUPT = 0x50;
static const uint16_t SERIAL_INTERRUPT = 0x58;
static const uint16_t JOYPAD_INTERRUPT = 0x60;

static const uint8_t WINDOW_HEIGHT = 144;
static const uint8_t WINDOW_WIDTH = 160;
#endif