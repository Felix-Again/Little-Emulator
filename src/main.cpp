#include <iostream>
#include <fstream>
#include <filesystem>
#include <stdexcept>
#include <cstdint>
#include <bitset>
#include <map>
#include <vector>
#include <functional>
#include <string>
#include <sstream>
#include <optional>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <iomanip>
#include <chrono>
#include "cpu.hpp"
#include "gpu.hpp"
#include "memorybus.hpp"
#include "cyclecounter.hpp"
#include "systembus.hpp"


int main(int argc, char* argv[]){

    SystemBus systemBus;

    CPU* cpu = systemBus.cpu.get();
    GPU* gpu = systemBus.gpu.get();
    MemoryBus* memory = systemBus.memory.get();
    CycleCounter* clock = systemBus.cycleCounter.get();

    std::vector<std::string> romPaths;
    std::string romPath;
    unsigned int selected = 0;

    for (auto entry : std::filesystem::directory_iterator("./")){
        if (entry.path().extension() == ".gb"){
            romPaths.push_back(entry.path().string());
        }
    }

    if (romPaths.size() < 1){
        std::cerr << "No available rom found." << std::endl;
        return 1;
    }

    std::cout << "Please, select your desired ROM." << std::endl;
    for (size_t i = 0; i < romPaths.size(); i++){
        std::cout << "[" <<  i << "]" << " " << romPaths[i] << std::endl;
    }

    std::cout << "ROM selected: " << std::endl;
    std::cin >> selected;

    if (selected < 0 || selected > romPaths.size()-1){
        throw std::runtime_error("Invalid selection: out of bounds.");
    }

    romPath = romPaths[selected];

    /*
    if (!SDL_Init(SDL_INIT_VIDEO)){
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }


    SDL_Window* window = NULL;
    SDL_Renderer* renderer = NULL;
    
    if (!SDL_CreateWindowAndRenderer("SDL3 Window", 160*PIXEL_SIZE, 144*PIXEL_SIZE, 0, &window, &renderer)){
        std::cerr << "Window/Renderer Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    SDL_SetRenderLogicalPresentation(renderer, 160, 144, SDL_LOGICAL_PRESENTATION_LETTERBOX);
    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 160, 144);

    SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);

    Uint32 pixelBuffer[160 * 144];

    for (int i = 0; i < 160*144; i++){
        pixelBuffer[i] = ScreenColor[3];
    }*/

    bool isRunning = true;

    // SDL_Event event;

    while (isRunning){

        /*
        while (SDL_PollEvent(&event)){
            if (event.type == SDL_EVENT_QUIT){
                isRunning = false;
            }
        }*/

        if (!cpu->booted){
            std::cout << "Booting" << std::endl;
            cpu->setPostBoot();
            continue;
        } 
        else if (!cpu->loadedROM){
            std::cout << "Loading" << std::endl;
            cpu->loadROM(romPath);
            continue;
        }
        
        cpu->executeASM();
        

        #ifdef BLARGGTESTROM

        if (memory->readByte(0xFF01) != 0 && memory->readByte(0xFF02) != 0){
            std::cout << "BLARGG TEST ROM: " << (memory->readByte(0xFF01)) << std::endl;
            std::cout << "BLARGG TEST ROM: " << (memory->readByte(0xFF02)) << std::endl;

            memory->writeByte(0xFF01, 0);
            memory->writeByte(0xFF02, 0);
        }

        #endif

        //if (clock.frameComplete()){
        /*
            SDL_UpdateTexture(texture, NULL, pixelBuffer, 160*sizeof(Uint32));

            SDL_RenderClear(renderer);
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 0);

            SDL_RenderTexture(renderer, texture, NULL, NULL);

            SDL_RenderPresent(renderer);*/

            clock->sync();
        //}

    }
    /*
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    */
    return 0;
}