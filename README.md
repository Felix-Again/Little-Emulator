# Little-Emulator

Little project to emulate a specific vintage console cpu and how it works. <br />

This project is a GameBoy DMG-01 Emulation project written purely in C++. <br />

I do not intend to harm any copyrighted content. Because of that, this work is simply a "GameBoy ASM emulator", which is able to receive an asm file with a compiled binary saved as a ".gb" extension file. I will not distribute, nor facilitate the distribution in any kind, of pirated ROMs. This means that anyone that wants to test this emulator must have their own, physical copy of the ROMs and use their own ROMs acquired from their own personal GameBoys in order to play this. <br />

Have fun if you want to download this early stage code or want to see me find out how to implement everything that I need to. Thanks! <br />
I aim to make this program portable to both Linux and Windows. For that, i am using WSL to compile the code with G++ and am using the Visual Studio compiler on Windows itself.

## Implementation course so far

| Component | Status |
| :---- | :---- |
| **CPU** | OPs Implemented. Test 1 passed. Will test others later. |
| **Graphics** | WIP. Trying to comprehend how it works. Made with SDL3. |
| **Audio** | Not implemented. Will do so with SDL3.|
| **Input**| Not implemented. Will do so with SDL3.

I have tested the emulator for the 01-special cpu instruction blargg test rom. After fixing every problem, it finally passed the test, with the correct "Passed" output on the terminal. I used the Gameboy Doctor and it verified that my emulator log has 100% similar to the correct log. <br/>
For this, I changed the cycle counter FPS to 600, because 60 would take a long time and disabling the cycle sync() function for the test made my CPU overheat. So, keep that in mind <br/>

## Build specs

**Language**: C++17.<br/>
**Compiler**: G++/Visual Studio.<br/>
**Graphics/Inputs**: SDL3. <br />

Prerequisites: <br />
On both platforms, you must remember to add the SDL3 libs and other SDL3 files to your include path. <br/>
On windows, Visual Studio Community with the C++ desktop development tools installed. Then, open it and load this code as a project and compile it with `cl.exe`. Remember to have the `SDL3.dll` file on the same folder as the executable.<br/>
On Linux, use g++, linking every .cpp file. The code itself must look like this: <br/>

```bash
g++ ./src/main.cpp ./src/cpu.cpp ./src/cyclecounter.cpp ./src/gpu.cpp ./src/memorybus.cpp ./src/systembus.cpp -o [custom file name] -lSDL3
```

After running the compiler, simply using 
```bash
./[custom file name]
```
Should do the trick and make you able to access the emulator. Please, make sure that you have your own ROMS on the same folder of the executable binary file.
### Technical and Theoretical Base

I used the following websites as a theorical base of what to do:
[Rylev Gameboy Emulator Book](https://rylev.github.io/DMG-01/public/book) <br />
[Meganesu OPCodes page](https://meganesu.github.io/generate-gb-opcodes/) <br />
[RGBDS Instructions list and descriptions](https://rgbds.gbdev.io/docs/v1.0.1/gbz80.7) <br />
[The Pandocs](https://gbdev.io/pandocs/) <br />

For testing, I am using Blargg Test ROMS, that I got from [this github repository](https://github.com/retrio/gb-test-roms) <br />
For debugging, I am using the Gameboy Doctor, please check it out at [this github repository](https://github.com/robert/gameboy-doctor) <br />
This program uses SDL3. Please check it out at [this link](https://github.com/libsdl-org/SDL)

# GLP License

This project and its contents are protected under the [GNU General Public License v3 (GPLv3)](LICENSE)<br/>

# "Gameboy" and "Nintendo" are trademarks of Nintendo. This project is not affiliated or endorsed by Nintendo.
I hold no copyright of the Gameboy games or its contents.