#include "cyclecounter.hpp"

CycleCounter::CycleCounter(int targetFPS = 60, uint32_t frequency = 4194304) {
    this->currentCycles = 0;
    this->cyclesPerFrame = frequency / targetFPS;
    this->targetFrameTime = 1000/targetFPS;
    this->lastTick = SDL_GetTicks();
}

void CycleCounter::addCycles(uint32_t cycles){
    this->currentCycles += cycles;
}

bool CycleCounter::frameComplete() const {
    return this->currentCycles >= this->cyclesPerFrame;
}

void CycleCounter::sync(){
    Uint64 currentTick = SDL_GetTicks();
    Uint32 elaspsed = static_cast<Uint32>(currentTick - this->lastTick);

    if (elaspsed < this->targetFrameTime){
        SDL_Delay(this->targetFrameTime - elaspsed);
    }

    this->lastTick = SDL_GetTicks();

    this->currentCycles = (this->currentCycles > this->cyclesPerFrame) ? (this->currentCycles - this->cyclesPerFrame) : 0;
}

uint32_t CycleCounter::getCycles() const {
    return this->currentCycles;
}