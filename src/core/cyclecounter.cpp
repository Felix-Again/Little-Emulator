#include "cyclecounter.hpp"

CycleCounter::CycleCounter(int targetFPS = 60, uint32_t frequency = CPU_FREQUENCY)
{
    this->currentCycles = 0;
    this->cyclesPerFrame = frequency / targetFPS;
    this->targetFrameTime = 1000 / targetFPS;
    this->targetFrameTimeNS = 1000000000 / targetFPS;
    this->lastTick = SDL_GetTicksNS();
}

void CycleCounter::addCycles(uint32_t cycles)
{
    this->currentCycles += cycles;

    uint8_t DIV = this->cpu->getHardwareRegister(CPU::HardwareRegisters::DIV);
    DIV += (cycles * (double)DIV_FREQUENCY) / (double)CPU_FREQUENCY;

    uint8_t TAC = this->cpu->getHardwareRegister(CPU::HardwareRegisters::TAC);

    bool enable = (TAC & 0x4) != 0;

    if (enable)
    {
        uint8_t clockSelect = TAC & 0x3;
        uint8_t incrementInterval;

        switch (clockSelect)
        {
        case 0:
            incrementInterval = 0xFF;
            break;
        case 1:
            incrementInterval = 0x04;
            break;
        case 2:
            incrementInterval = 0x10;
            break;
        case 3:
            incrementInterval = 0x40;
            break;
        }

        if ((this->currentCycles / MCYCLESIZE) % incrementInterval < (cycles / MCYCLESIZE))
        {
            uint8_t TIMA = this->cpu->getHardwareRegister(CPU::HardwareRegisters::TIMA);
            TIMA++;
            this->cpu->setHardwareRegister(CPU::HardwareRegisters::TIMA, TIMA);
            if (TIMA == 0)
            {
                uint8_t TMA = this->cpu->getHardwareRegister(CPU::HardwareRegisters::TMA);
                this->cpu->setHardwareRegister(CPU::HardwareRegisters::TIMA, TMA);
                this->timerInterrupt();
            }
        }
    }
}

bool CycleCounter::frameComplete() const
{
    return this->currentCycles >= this->cyclesPerFrame;
}

void CycleCounter::linkPointers(CPU *cpu)
{
    this->cpu = cpu;
}

void CycleCounter::sync()
{
    Uint64 currentTick = SDL_GetTicksNS();
    Uint64 elaspsed = currentTick - this->lastTick;

    if (elaspsed < this->targetFrameTimeNS)
    {
        SDL_DelayNS(this->targetFrameTimeNS - elaspsed);
    }

    this->lastTick += this->targetFrameTimeNS;

    if (SDL_GetTicksNS() - this->lastTick > this->targetFrameTimeNS * 2)
    {
        this->lastTick = SDL_GetTicksNS();
    }

    this->currentCycles = ((this->currentCycles > this->cyclesPerFrame) ? (this->currentCycles - this->cyclesPerFrame) : 0);
}

uint32_t CycleCounter::getCycles() const
{
    return this->currentCycles;
}

void CycleCounter::timerInterrupt()
{
    this->cpu->requestInterrupt(2);
}