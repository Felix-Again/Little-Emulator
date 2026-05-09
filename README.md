# Little-Emulator

Little project to emulate a specific vintage console cpu and how it works. <br />

This project is a GameBoy DMG-01 Emulation project written purely in C++. <br />

I do not intend to harm any copyrighted content. Because of that, this work is simply a "GameBoy ASM emulator", which is able to receive an asm file with a compiled binary saved as a ".gb" extension file. I will not distribute, nor facilitate the distribution in any kind, of pirated ROMs. This means that anyone that wants to test this emulator must have their own, physical copy of the ROMs and use their own ROMs acquired from their own personal GameBoys in order to play this. <br />

Have fun if you want to download this early stage code or want to see me find out how to implement everything that I need to. Thanks! <br />
I aim to make this program portable to both Linux and Windows. For that, i am using WSL to compile the code with G++ and am using the Visual Studio compiler on Windows itself.

## Implementation course so far

| Component | Status |
| :---- | :---- |
| **CPU** | OPs Implemented. Passed all 11 Individual Blargg Tests. |
| **Graphics** | WIP. Trying to comprehend how it works. Made with SDL3. |
| **Audio** | Not implemented. Will do so with SDL3.|
| **Input**| Not implemented. Will do so with SDL3.

## Build specs

This project now uses CMake to compile. It is really easier than the last approach that I had.

CMake works on any OS, as long as you have any C++ compiler and the SDL3 library.

On Linux, follow these steps:

### 1. Install CMake and SDL3 on Linux

First, verify if you have CMake and SDL3 already installed and on your path. <br/>
To find out if you have CMake, use the following command:
```bash
cmake --version
```

If the output is not `CMake 3.x.x`, you should install/update CMake. <br/>
To do so, use your distro package manager. On Ubuntu, you can install it with:
```bash
sudo apt update
sudo apt install cmake
```

To verify if SDL3 is correctly installed, use <br/>
```bash
cmake --find-package -DNAME=SDL3 -DCOMPILER_ID=GNU -DLANGUAGE=CXX -DMODE=EXIT
```

If the output is `SDL3 found`, you can continue. Otherwise, please follow this [Installation guide](https://github.com/libsdl-org/SDL/blob/main/INSTALL.md). <br/>

### 2. Build the program

Now that the setup is done, you can install this repository source code the way that you like. GitHub has a "Install .zip file" option, use that if you want. If you would like, cloning the repository is also an option. <br/>
For that, open your desired folder and use the following command:
```bash
git clone https://github.com/Felix-Again/Little-Emulator.git
```
This will clone my source code to your computer. <br/>
After downloading the source code, ensure you are on the same file that contains the "src" folder and this "README.md" file. <br/>

Then, open your terminal and use the following commands:
```bash
mkdir build && cd build
cmake ..
make
```

Alternatively to using `make`, you can use `cmake --build .`. <br/>

Now, inside your build folder, you should find the "DMG-Emulator" file. This is the executable file that you can use. Make sure that you have a valid ".gb" file on the folder where you are calling your executable from, as not having any ROM file will make the emulator quit automatically. <br/>

### 1. Build by installing CMake and SDL3 on Windows

To verify if CMake is installed, run powershell and execute this command:
```powershell
cmake --version
```

If you see this error: 
```powershell
cmake: The term 'cmake' is not recognized as a name of a cmdlet, function, script file, or executable program.
Check the spelling of the name, or if a path was included, verify that the path is correct and try again.
```
It means that you need to install CMake. For that, get the corresponding installer for your windows platform on their [official website](https://cmake.org/download/).<br/>

Install SDL3 on [their official github repository](https://github.com/libsdl-org/SDL/releases/tag/release-3.4.8)<br/>
If you need, search for an online tutorial to set up SDL3 on your Windows installation, as it can be a bit confusing<br/>

After having everything ready, make sure to add the **SDL3 path** to your Windows PATH.

### 2. Build the executable on windows

You can then clone the repository on your desired folder with
```powershell
git clone https://github.com/Felix-Again/Little-Emulator.git
```
Then, use the following commands on your main directory:
```powershell
mkdir build && cd build
cmake ..
cmake --build .
```

Your executable will probably be located at `build/Degug/DMG-Emulator.exe`. This is the executable file that you can use. Make sure that you have a valid ".gb" file on the folder where you are calling your executable from, as not having any ROM file will make the emulator quit automatically. <br/>

If any errors occur, you might not have installed correctly your dependencies. <br/>
If you can't build this on Windows for some reason (Something wrong on my CMakeLists.txt or on your computer), I will soon create a "Releases" tab on the github repository to download the compiled executables for Windows.


### Technical and Theoretical Base

I used the following websites as a theorical base of what to do: <br/>
[Rylev Gameboy Emulator Book](https://rylev.github.io/DMG-01/public/book) <br />
[Meganesu OPCodes page](https://meganesu.github.io/generate-gb-opcodes/) <br />
[RGBDS Instructions list and descriptions](https://rgbds.gbdev.io/docs/v1.0.1/gbz80.7) <br />
[The Pandocs](https://gbdev.io/pandocs/) <br />

For testing, I am using Blargg Test ROMS, that I got from [this github repository](https://github.com/retrio/gb-test-roms) <br />
For debugging, I am using the Gameboy Doctor, please check it out at [this github repository](https://github.com/robert/gameboy-doctor) <br />
This program uses SDL3. Please check it out at [this link](https://github.com/libsdl-org/SDL)

# GPL License

This project and its contents are protected under the [GNU General Public License v3 (GPLv3)](LICENSE)<br/>

# "Gameboy" and "Nintendo" are trademarks of Nintendo. This project is not affiliated or endorsed by Nintendo.
I hold no copyright of the Gameboy games or its contents.