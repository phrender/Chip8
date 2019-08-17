#include <string>
#include "Interpreter.hpp"

#define GetRegister(opcode) ( (opcode & 0x0F00) >> 8 )
#define CHIP8_JUMP_TWO_INCSTUCTIONS 4

/**
    Default Constructor
 */
Interpreter::Interpreter() : m_stackPointer(0xFF), m_programCounter(0x200), m_I(0x000)
{
    /**
        Clears any data obtained during initialization of 'm_stack' and 'm_registerV'
     */
    m_stack = {};
    m_registerV = {};
};

/**
    Default Destructor
 */
Interpreter::~Interpreter()
{
};

/**
    Initializes the Interpreter to open, validate and place the loaded ROM in the allocated 4K memory.

    @param[in] filePath Path to the ROM file to load
    @return true or false depending on initialization of emulator RAM and loading of the ROM
 */
bool Interpreter::Initialize(const char* filePath)
{
    if (!InitializeEmulatorRAM())
    {
        printf("Error: Failed to allocate RAM for Chip8!\n");
        return false;
    };
    
    if (!InitializeFontset())
    {
        printf("Error: Failed to initialize fontset for Chip8!\n");
        return false;
    }
    
    if (!OpenAndLoadFile(filePath))
    {
        printf("Error: Failed to open and load requested file!\n");
        return false;
    };
    
    return true;
};

/**
    Runs the interpreter.
 */
void Interpreter::Run()
{
    uint16_t opcode = (m_memory[m_programCounter] << 8 | m_memory[m_programCounter + 1]);
    uint16_t pc = m_programCounter + CHIP8_INSTRUCTION_SIZE;
    
    switch (opcode & 0xF000) {
        case 0x000:
            // Execute SYS addr
            
            /**
                0x00EE
                    return from a subroutine.
             */
            if (opcode == 0x00EE)
            {
                pc = m_stack[m_stackPointer];
                m_stack[m_stackPointer] = 0x0000;
                m_stackPointer--;
            };
            
            break;
            
            /**
                1nnn:
                    Jump to location nnn.
             */
        case 0x1000:
            pc = opcode & 0x0FFF;
            break;
            
            /**
                2nnn:
                    Call subroutine at nnn.
             */
        case 0x2000:
            m_stackPointer++;
            m_stack[m_stackPointer] = m_programCounter;
            pc = (opcode & 0x0FFF);
            break;
            
            /**
                3xkk
                    Compare register x to kk, if equal increment program counter by 2
             */
        case 0x3000:
            pc += m_registerV[GetRegister(opcode)] == (opcode & 0x00FF) ? CHIP8_INSTRUCTION_SIZE : 0;
            break;
            
            /**
                4xkk
                    Compare register x to kk, is not equal increment program counter by 2
             */
        case 0x4000:
            pc += m_registerV[GetRegister(opcode)] != (opcode & 0x00FF) ? CHIP8_INSTRUCTION_SIZE : 0;
            break;
            
            /**
                5xy0
                    Compare register x and y, if x and y is equal increment program counter by 2
             */
        case 0x5000:
            pc += m_registerV[(opcode & 0x0F00) >> 8] == m_registerV[(opcode & 0x00F0 >> 4)] ? CHIP8_INSTRUCTION_SIZE : 0;
            break;
            
            /**
                6xkk
                    Store kk in register x
             */
        case 0x6000:
            m_registerV[GetRegister(opcode)] = opcode & 0x00FF;
            break;
            
            /**
                7xkk
                    Set register x to x + kk
             */
        case 0x7000:
            m_registerV[GetRegister(opcode)] += (opcode & 0x00FF);
            break;
            
        case 0x8000:
            break;
            
            /**
                9xy0
                    Skip next instruction if register x is equal to register y
             */
        case 0x9000:
            pc += m_registerV[(opcode & 0x0F00) >> 8] != m_registerV[(opcode & 0x00F0) >> 4] ? CHIP8_INSTRUCTION_SIZE : 0;
            break;
            
            /**
                Annn
                    Set I to nnn
                    Program counter is set to value nnn
             */
        case 0xA000:
            m_I = opcode & 0x0FFF;
            break;
            
            /**
                Dxyn
                    Display n-byte sprite starting at location of I
                    x - positionX?
                    y - positionY?
             */
        case 0xD000:
            // Draw code here?
            break;
            
        default:
            break;
    }
    m_programCounter = pc;
};

/**
    Allocates 4096 KB to emulate Chip8's amount of RAM,
 */
bool Interpreter::InitializeEmulatorRAM()
{
    m_memory.fill(0x00);
    
    return !m_memory.empty();
};

/**
    Initializes the fontset for Chip8
 
    @return Successfull state if we fill the fontset with keys '0' through 'F'
 */
bool Interpreter::InitializeFontset()
{
    // Clear the array
    m_fontset = {};
    
    // Insert the fonset.
    m_fontset =
    {
        0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
        0x20, 0x60, 0x20, 0x20, 0x70, // 1
        0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
        0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
        0x90, 0x90, 0xF0, 0x10, 0x10, // 4
        0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
        0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
        0xF0, 0x10, 0x20, 0x40, 0x40, // 7
        0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
        0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
        0xF0, 0x90, 0xF0, 0x90, 0x90, // A
        0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
        0xF0, 0x80, 0x80, 0x80, 0xF0, // C
        0xE0, 0x90, 0x90, 0x90, 0xE0, // D
        0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
        0xF0, 0x80, 0xF0, 0x80, 0x80  // F
    };
    
    // Validate font set(?)
    for (auto font : m_fontset)
    {
        if (font == 0x00)
        {
            return false;
        };
    };
    
    return !m_fontset.empty();
};

/**
    Opens and loads the requested ROM file.
    Incase of being unable to locate or place
    the ROM in memory it will fail.

    @param[in] filePath Path to the ROM file to load
    @return Success if we can successfully load, validate and place the ROM in memory
 */
bool Interpreter::OpenAndLoadFile(const char* filePath)
{
    if (filePath == nullptr || strlen(filePath) == 0)
    {
        printf("File path cannot be null or empty!\n");
        return false;
    };
    
    std::ifstream file;
    file.open(filePath, std::ifstream::in | std::ifstream::binary);
    
    if (!file.is_open())
    {
        printf("Failed to open %s\n", filePath);
        return false;
    };
    
    file.seekg(0, file.end);
    size_t romSize = file.tellg();
    if (romSize < 0 && romSize > (CHIP8_RAM_SIZE - 0x1FF))
    {
        printf("File to large, RAM size is %u\n", CHIP8_RAM_SIZE);
        file.close();
        return false;
    };
    
    uint8_t* pEmulatorRom = new uint8_t[romSize];
    file.seekg(0, std::ifstream::beg);
    file.read((char*)pEmulatorRom, romSize);
    file.close();
    
    memcpy(&m_memory[0x200], pEmulatorRom, romSize);
    delete[] pEmulatorRom;
    pEmulatorRom = nullptr;
    
    return !file.is_open() && pEmulatorRom == nullptr;
};
