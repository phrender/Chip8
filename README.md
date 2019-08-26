# Chip8
A Chip 8 emulator to practice C++, currently no audio support.

##### Keymap
```
|1|2|3|C|    |1|2|3|4|
|4|5|6|D| => |Q|W|E|R|
|7|8|9|E|    |A|S|D|F|
|A|0|B|F|    |Z|X|C|V|
```

### Requirements
* CMake 3.11
* SDL 2.0.10

### Build instructions
* Create external folder and place SDL inside of it. (`external\SDL`)
* Create build folder and enter it.
* Generate from source
  * For `Microsoft Visual Studio` run `cmake -G "Visual Studio 15 2017" ..`
  * For MacOS/Linux `run cmake ..`
* Build
  * `Windows`
    * Visual Studio - Open Chip8Emu.sln
	* Executable will be created in `build\bin\<configuration>\`
  * `MacOS/Linux` 
    * `cmake --build .`
	* Executable will be created in `build\bin`
* Run
  * Windows 
    * Drag a rom file to the executable and drop it.
	* Run `start Chip8Emu.exe <path-to-rom>`
  * `MacOS\Linux`
    * Run `./Chip8Emu <path-to-rom>`

### Sources
* http://devernay.free.fr/hacks/chip8/C8TECH10.HTM#2.5
* https://en.wikipedia.org/wiki/CHIP-8
