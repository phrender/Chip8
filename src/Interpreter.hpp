#ifndef INTERPRETER_HPP_INCLUDED
#define INTERPRETER_HPP_INCLUDED
#pragma once

#include <cstdint>
#include <fstream>
#include <array>
#include <memory>

/** \def Chip8 RAM size 4096 KB */
#define CHIP8_RAM_SIZE 4096
/** \def SIze of a Chip8 instruction (16 bits) */
#define CHIP8_INSTRUCTION_SIZE 2
/** \def Size of the Chip8 register bank */
#define CHIP8_REGISTER_BANK_SIZE 16
/** \def Size of the Chip8 stack */
#define CHIP8_STACK_SIZE 16
/** \def Number of keys available on a Chip8 */
#define CHIP8_KEYBOARD_SIZE 16
/** \def Chip8 screen width */
#define CHIP8_WIDTH 64
/** \def Chip8 screen height */
#define CHIP8_HEIGHT 32
/** \def Chip8 fonstset size */
#define CHIP8_FONTSET_SIZE 80

class Interpreter
{
	public:
	
		Interpreter();
		~Interpreter();
    
        bool Initialize(const char* filePath, uint16_t windowWidth, uint16_t windowHeight);
        void Run();
    
    private:
    
        bool InitializeEmulatorRAM(uint16_t windowWidth, uint16_t windowHeight);
        bool InitializeFontset();
        bool OpenAndLoadFile(const char* filePath);
    
	private:

        /** Holds the current position of the stack*/
        int8_t m_stackPointer;
        /** Emulator RAM */
        std::array<uint8_t, CHIP8_RAM_SIZE> m_memory;
        /** Emulator program counter, starts at byte 512 */
        uint16_t m_programCounter;
        /** Emulator Index Register */
        uint16_t m_I;
        /** Emulator stack, 16 slots */
        std::array<uint16_t, CHIP8_STACK_SIZE> m_stack;
        /** Emulator register, 16 8-bit, slots */
        std::array<uint8_t, CHIP8_REGISTER_BANK_SIZE> m_registerV;
        /** Emulator fontset */
        std::array<uint8_t, CHIP8_FONTSET_SIZE> m_fontset;
        /** Emulator screen buffer (64*32) */
        std::unique_ptr<uint8_t[]> m_pScreenBuffer;

}; // Interpreter

#endif // INTERPRETER_HPP_INCLUDED
