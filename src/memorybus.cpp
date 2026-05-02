#include "memorybus.hpp"

MemoryBus::MemoryBus(SystemBus* systemBus) : systemBus(systemBus), memory(0x10000) {
    
}

void MemoryBus::linkPointers(CPU* cpu, GPU* gpu, CycleCounter* cycleCounter){

    this->cpu = cpu;
    this->gpu = gpu;
    this->clock = cycleCounter;

}

uint8_t MemoryBus::readByte(uint16_t address){


    if (VRAM_BEGIN <= address && address <= VRAM_END){
        return this->gpu->readVram(address);
    }

    if (OAM_BEGIN <= address && address <= OAM_END){
        return this->gpu->readOam(address);
    }
    
    if (ECHORAM_BEGIN <= address && address <= ECHORAM_END){
        return this->memory[address - (ECHORAM_BEGIN-WRAM_BEGIN)];
    }

    return this->memory[address];
}

uint16_t MemoryBus::readWord(uint16_t address){
    uint8_t lsByte = this->readByte(address);

    uint16_t next = address + 1;
    uint8_t msByte = this->readByte(next);

    return (static_cast<uint16_t>(msByte)) << 8 | static_cast<uint16_t>(lsByte);
}

void MemoryBus::OamDmaTransfer(uint8_t value){
    uint16_t sourceAddress = static_cast<uint16_t>(value) << 8;

    for (int i = 0; i < 160; i++){
        uint8_t data = this->memory[sourceAddress + i];
        this->gpu->writeOam(0xFE00 + i, data);
    }
}

void MemoryBus::writeByte(uint16_t address, uint8_t value){

    if (address == OAM_DMA_TRANSFER_BYTE){
        this->OamDmaTransfer(value);
        return;
    }

    if (VRAM_BEGIN <= address && address <= VRAM_END){
        this->gpu->writeVram(address, value);
        return;
    }
    if (OAM_BEGIN <= address && address <= OAM_END){
        this->gpu->writeOam(address, value);
        return;
    }

    if (WRAM_BEGIN <= address && address <= WRAM_END){
        this->memory[address] = value;

        if (address + (ECHORAM_BEGIN - WRAM_BEGIN) <= ECHORAM_END){
            this->memory[address + (ECHORAM_BEGIN - WRAM_BEGIN)] = value;
        }
    }

    if (ECHORAM_BEGIN <= address && address <= ECHORAM_END){
        this->memory[address] = value;
        this->memory[address - (ECHORAM_BEGIN - WRAM_BEGIN)] = value;
        return;
    }

    this->memory[address] = value;
}

void MemoryBus::writeWord(uint16_t address, uint16_t value){
    this->writeByte(address, value & 0xFF);
    this->writeByte(address+1, (value >> 8) & 0xFF);
}

