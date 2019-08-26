#ifndef INTERPRETER_HPP_INCLUDED
#define INTERPRETER_HPP_INCLUDED
#pragma once

#include <cstdint>
#include <fstream>
#include <array>
#include <memory>

/** Chip8 RAM size 4096 KB */
constexpr uint16_t g_chipRamSize = 0x1000;
/** Size of a Chip8 instruction (16 bits) */
constexpr uint8_t g_chipInstructionSize = 0x10;
/** Size of the Chip8 register bank */
constexpr uint8_t g_chipRegisterBankSize = 0x10;
/** Size of the Chip8 stack */
constexpr uint8_t g_chipStackSize = 0x10;
/** Number of keys available on a Chip8 */
constexpr uint8_t g_chipKeyboardSize = 0x10;
/** Chip8 fonstset size */
constexpr uint8_t g_chipFontsetSize = 0x50;

/**
	Allows for easier handling of multiple screen sizes.
		Chip8 - 64 x 32 pixels
		ETTI - 64 x 48 pixels
 */
enum class ScreenSize : uint16_t
{
	Chip8 = 0x4020,
	ETTI = 0x4030
};

class Interpreter
{
	public:
	
		Interpreter();
		~Interpreter();
    
        bool Initialize(const char* filePath, ScreenSize screenSize);
        void Run();
    
        void Draw(uint32_t* pScreen, uint32_t windowWidth, uint32_t windowHeight);
    
        void OnKeyPressed(uint8_t keyIndex);
        void OnKeyReleased(uint8_t keyIndex);

    private:
    
        bool InitializeEmulatorRAM();
        bool InitializeEmulatorKeyboard();
        bool InitializeFontset();
        bool OpenAndLoadFile(const char* filePath);

		uint16_t GetEmulatorWidth() const;
		uint16_t GetEmulatorHeight() const;
    
	private:

        /** Delay timer */
        uint8_t m_delayTimer;
        /** Sound timer */
        uint8_t m_soundTimer;
        /** Holds the current position of the stack*/
        int8_t m_stackPointer;
		/** Holds the screen size (ex. 0x4020 = 64*32) */
		ScreenSize m_screenSize;
        /** Emulator RAM */
        std::array<uint8_t, g_chipRamSize> m_memory;
        /** Emulator keyboard */
        std::array<uint8_t, g_chipKeyboardSize> m_keyboard;
        /** Emulator program counter, starts at byte 512 */
        uint16_t m_programCounter;
        /** Emulator Index Register */
        uint16_t m_I;
        /** Emulator stack, 16 slots */
        std::array<uint16_t, g_chipStackSize> m_stack;
        /** Emulator register, 16 8-bit, slots */
        std::array<uint8_t, g_chipRegisterBankSize> m_registerV;
        /** Emulator fontset */
        std::array<uint8_t, g_chipFontsetSize> m_fontset;
        /** Emulator screen buffer (ex. 64*32) */
        std::unique_ptr<uint8_t[]> m_pScreenBuffer;

}; // Interpreter

#endif // INTERPRETER_HPP_INCLUDED
