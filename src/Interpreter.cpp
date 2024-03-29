#include <cstdlib>
#include <ctime>
#include <string>
#include <SDL_keycode.h>
#include "Interpreter.hpp"

#define GetRegister(opcode) ( (opcode & 0x0F00) >> 8 )

/**
    Default Constructor
 */
Interpreter::Interpreter() : m_delayTimer(0x00), m_soundTimer(0x00), m_stackPointer(0xFF), m_screenSize(ScreenSize::Chip8), m_programCounter(0x0200), m_I(0x0000)
{
    /**
        Initialize the screen buffer
     */
    m_pScreenBuffer.reset();
	
	/**
		Zero all bits in arrays
	*/
	m_memory.fill(0x00);
	m_registerV.fill(0x00);
	m_fontset.fill(0x00);
	m_stack.fill(0x0000);
    
    // Initializr std::rand()
    std::srand(std::time(nullptr));
};

/**
    Default Destructor
 */
Interpreter::~Interpreter()
{
};

/**
    Initializes the Interpreter to open, validate and place the loaded ROM in the allocated 4K memory.

    @param[in] filePath Path to the ROM file to load.
	@param[in] screenSize Size of the screen.
    @return true or false depending on initialization of emulator RAM and loading of the ROM
 */
bool Interpreter::Initialize(const char* filePath, ScreenSize screenSize)
{
	m_screenSize = screenSize;

    if (!InitializeEmulatorRAM())
    {
        printf("Error: Failed to allocate RAM for Chip8!\n");
        return false;
    };
    
    if (!InitializeEmulatorKeyboard())
    {
        printf("Error: Failed to initialize keyboard for Chip8!\n");
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
    uint16_t pc = m_programCounter + g_chipInstructionSize;
    
    switch (opcode & 0xF000) {
            
        case 0x000:
            
            switch (opcode & 0x000F)
            {
                    
                /**
                    0x0000\n
                        Clear the display.
                */
                case 0x0000:
                {
                    uint16_t pixels = GetEmulatorWidth() * GetEmulatorHeight();
                    std::fill(
                              m_pScreenBuffer.get(),
                              m_pScreenBuffer.get() + pixels,
                              0x00);
                }
                    break;
                    
                /**
                    0x000E\n
                        Return from subroutine.
                */
                case 0x000E:
                    pc = m_stack[m_stackPointer] + g_chipInstructionSize;
                    m_stackPointer--;
                    break;
            };
            break;

			/**
				1nnn\n
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
				3xkk\n
					Compare register x to kk, if equal increment program counter by 2
			 */
		case 0x3000:
			pc += m_registerV[GetRegister(opcode)] == (opcode & 0x00FF) ? g_chipInstructionSize : 0;
			break;

			/**
				4xkk\n
					Compare register x to kk, is not equal increment program counter by 2
			 */
		case 0x4000:
			pc += m_registerV[GetRegister(opcode)] != (opcode & 0x00FF) ? g_chipInstructionSize : 0;
			break;

			/**
				5xy0\n
					Compare register x and y, if x and y is equal increment program counter by 2
			 */
		case 0x5000:
			pc += m_registerV[(opcode & 0x0F00) >> 8] == m_registerV[(opcode & 0x00F0 >> 4)] ? g_chipInstructionSize : 0;
			break;

			/**
				6xkk\n
					Store kk in register x
			 */
		case 0x6000:
			m_registerV[GetRegister(opcode)] = opcode & 0x00FF;
			break;

			/**
				7xkk\n
					Set register x to x + kk
			 */
		case 0x7000:
			m_registerV[GetRegister(opcode)] += (opcode & 0x00FF);
			break;

            /**
                8xyn\n
                    n determines which opcode we should use.
             */
		case 0x8000:
            
            switch (opcode & 0x000F)
            {
                    
                    /**
                        8xy0\n
                            Set register Vx to Vy
                     */
                case 0x0000:
                    m_registerV[(opcode & 0x0F00) >> 8] = m_registerV[(opcode & 0x00F0) >> 4];
                    break;
                    
                    /**
                        8xy1\n
                            Set register Vx to Vx OR (|) Vy
                     */
                case 0x0001:
                    m_registerV[(opcode & 0x0F00) >> 8] |= m_registerV[(opcode & 0x00F0) >> 4];
                    break;
                    
                    /**
                        8xy2\n
                            Set register Vx to Vx AND (&) Vy
                     */
                case 0x0002:
                    m_registerV[(opcode & 0x0F00) >> 8] &= m_registerV[(opcode & 0x00F0) >> 4];
                    break;
                    
                    /**
						8xy3\n
                     		Set register Vx XOR Vy
                     */
                case 0x0003:
                    m_registerV[(opcode & 0x0F00) >> 8] ^= m_registerV[(opcode & 0x00F0) >> 4];
                    break;
                    
                    /**
                        8xy4\n
                            Set register Vx to Vx + Vy.
                            VF is set to carry
                            Set register VF to carry (Vx + Vy > 255, carry is equal to 1 otherwise 0)
                     */
                case 0x0004:
                    m_registerV[(opcode & 0x0F00) >> 8] += m_registerV[(opcode & 0x00F0) >> 4];
                    m_registerV[0x0F] = m_registerV[(0x00F0) >> 4] > (0xFF - m_registerV[(0x0F00 & opcode) >> 8]);
                    break;
                
                    /**
                        8xy5\n
                            Set register Vx to Vx - Vy
                            VF is set to NOT borrow
                     */
                case 0x0005:
                    m_registerV[0x0F] = m_registerV[(opcode & 0x0F00) >> 8] > m_registerV[(opcode & 0x00F0) >> 4] ? 0x01 : 0x00;
                    m_registerV[(opcode & 0x0F00) >> 8] -= m_registerV[(opcode & 0x00F0) >> 4];
                    break;
                    
                    /**
                        8xy6\n
                            If the last bit of Vx is 1 set VF  to 1, otherwise 0.
                            Divide register Vx by two.
                     */
                case 0x0006:
                    m_registerV[0x0F] = m_registerV[(opcode & 0x0F00) >> 8] & 0x01;
                    m_registerV[(opcode & 0x0F00) >> 8] >>= 1;
                    break;
                    
                    /**
                        8xy7\n
                            Set register Vx to Vx - Vy.
                            If Vy is greater than Vx set VF to 1, otherwise 0.
                            Register VF is set to NOT BORROW.
                     */
                case 0x0007:
                    m_registerV[0x0F] = m_registerV[(opcode & 0x00F0) >> 4] > m_registerV[(0x0F00) >> 8] ? 0x01 : 0x00;
                    m_registerV[(opcode & 0x0F00) >> 8] = m_registerV[(opcode & 0x00F0) >> 4] - m_registerV[(opcode & 0x0F00) >> 8];
                    break;
                    
                    /**
                        8xyE\n
                            Check if most-significant bit is 1, if so set VF to 1 otherwise 0.
                            Multiply register Vx by 2.
                     */
                case 0x000E:
                    m_registerV[0x0F] = m_registerV[(opcode & 0x0F00) >> 8] & 0x80 ? 0x01 : 0x00;
                    m_registerV[(opcode & 0x0F00) >> 8] <<= 1;
                    break;
            }
            
			break;

			/**
				9xy0\n
					Skip next instruction if register x is equal to register y
			 */
		case 0x9000:
			pc += m_registerV[(opcode & 0x0F00) >> 8] != m_registerV[(opcode & 0x00F0) >> 4] ? g_chipInstructionSize : 0;
			break;

			/**
				Annn\n
					Set I to nnn
					Program counter is set to value nnn
			 */
		case 0xA000:
			m_I = opcode & 0x0FFF;
			break;
            
            /**
                Bnnn\n
                    Set program counter to nnn + value of register V0.
             */
        case 0xB000:
            pc = (opcode & 0x0FFF) + m_registerV[0x00];
            break;
            
            /**
                Cxkk\n
                    Set Vx to random byte AND kk
             */
        case 0xC000:
            m_registerV[(opcode & 0x0F00) >> 8] = (std::rand() % 255) + (opcode & 0x00FF);
            break;

			/**
				Dxyn\n
					Display n-byte sprite starting at location of I
					x - positionX from Vx
					y - positionY from Vy
					n - read n bytes from memory also used as height
			 */
		case 0xD000:
        {
            uint16_t posX = m_registerV[(opcode & 0x0F00) >> 8];
            uint16_t posY = m_registerV[(opcode & 0x00F0) >> 4];
            uint16_t width = GetEmulatorWidth();
            uint16_t height = opcode & 0x000F;
            uint16_t pixel;
            
            // Set register 15 (0x0F) to 0 for no collision detected.
            m_registerV[0x0F] = 0;
            for(uint16_t y = 0; y < height; y++)
            {
                
                // Get start address
                pixel = m_memory[m_I + y];
                for(uint16_t x = 0; x < 8; x++)     // 8-bits is the maximum width of a sprite
                {
                    
                    if((pixel & (0x80 >> x)) != 0)
                    {
                        
                        // Did we collide with something?
                        if(m_pScreenBuffer[(posX + x + ((posY + y) * width))] == 1)
                        {
                            m_registerV[0x0F] = 0x01;
                        };
                        
                        m_pScreenBuffer[posX + x + ((posY + y) * width)] ^= 1;
                    };
                };
            };
        }
            break;

			/**
				E000\n
					Handle input.
			*/
		case 0xE000:
			
			switch (opcode & 0x00FF)
			{
					/**
						Ex9E\n
							Skip next instruction if the key with value Vx is pressed.
					*/
				case 0x009E:
                    pc += m_keyboard[m_registerV[(opcode & 0x0F00) >> 8]] != 0 ? g_chipInstructionSize : 0;
					break;

					/**
						ExA1\n
							Skip the next instruction if the key with value Vx is released.
					*/
				case 0x00A1:
                    pc += m_keyboard[m_registerV[(opcode & 0x0F00) >> 8]] == 0 ? g_chipInstructionSize : 0;
					break;
			}
			break;
            
            /**
                FxII\n
                    Two last nibbles are used to determine which instruction to use.
             */
        case 0xF000:
            
            switch (opcode & 0x00FF)
            {
                    
                    /**
                        Fx007\n
                            Store delay timer in register Vx.
                     */
                case 0x0007:
                    m_registerV[(opcode & 0x0F00) >> 8] = m_delayTimer;
                    break;
                    
                    /**
                        Fx0A\n
                            Wait for a key press and then store it in register Vx.
                     */
                case 0x000A:
                {
                    // A check for if we've pressed a key.
                    bool isKeyPressed = false;
                    
                    // Look for the pressed key.
                    for (int i = 0; i < g_chipKeyboardSize; i++)
                    {
                        // Check if the key is pressed.
                        if (m_keyboard[i] == 0x01)
                        {
                            // Store in register Vxz®
                            m_registerV[(opcode & 0x0F00) >> 8] = i;
                            isKeyPressed = true; // Say that we've pressed the key.
                        };
                    };
                    
                    // Break the loop until we've pressed a key.
                    if (!isKeyPressed)
                    {
                        return;
                    };
                };
                    break;
                    
                    /**
                        Fx15\n
                            Set delay timer to x.
                     */
                case 0x0015:
                    m_delayTimer = ((opcode & 0x0F00) >> 8);
                    break;
                    
                    /**
                        Fx18\n
                            Set sound timer to x.
                     */
                case 0x0018:
                    m_soundTimer = ((opcode & 0x0F00) >> 8);
                    break;
                    
                    /**
                        Fx1E\n
                            Set I to I + register Vx.
                     */
                case 0x001E:
                    m_I += m_registerV[(opcode & 0x0F00) >> 8];
                    break;
                    
                    /**
                        Fx29\n
                            Set I to corresponding sprite for digit at Vx.
                     */
                case 0x0029:
                    // Multiply value by five (5) since a font sprite has a length of five (5).
					m_I = m_registerV[(opcode & 0x0F00) >> 8] * 5;
                    break;
                    
                    /**
                        Fx33\n
                            Store the BCD (Binary Coded Decimal) of Vx in memory location starting at I.\n
                            I = one hundredth digit of Vx \n
                            I + 1 = one tenth digit of Vx \n
                            I + 2 = one digit of Vx \n
                     */
                case 0x0033:
                {
                    uint16_t value = m_registerV[(opcode & 0x0F00) >> 8];
                    m_memory[m_I] = (value / 100) % 10;
                    m_memory[m_I + 1] = (value / 10) % 10;
                    m_memory[m_I + 2] = value % 10;
                }
                    break;

					/**
						Fx55\n
							Load register V0 through Vx in to memory.
					 */
				case 0x0055:
					{
						for (uint8_t i = 0; i < ((opcode & 0x0F00) >> 8); i++)
						{
							m_memory[m_I + i] = m_registerV[i];
						};
					};
					break;
                    
                    /**
                        Fx65\n
                            Reads registers V0 to Vx from memory starting at I.
                     */
                case 0x0065:
                    for (int i = 0; i < ((opcode & 0x0F00) >> 8); i++)
                    {
                        m_registerV[i] = m_memory[m_I + i];
                    };
                    break;
            };
            break;

		default:
			printf("Unknown");
			break;
    };
    m_programCounter = pc;
    
    // Subtract one from the delay timer.
    // (Should be at a rate of 1 per 60Hz)
    if (m_delayTimer > 0)
    {
        --m_delayTimer;
    };
    
    // Subtract one from the sound timer.
    // (Should be at a rate of 1 per 60Hz)
    if (m_soundTimer > 0)
    {
        if (m_soundTimer == 1)
        {
            // TODO: Implement audio beep here.
        };
        --m_soundTimer;
    };
};

/**
    Draw pixels to the screen
 
    @param[in] pScreen An array of all pixels available on the screen passed.
    @param[in] windowWidth Width of the window
    @param[in] windowHeight Height of the window
 */
void Interpreter::Draw(uint32_t* pScreen, uint32_t windowWidth, uint32_t windowHeight)
{
    // Get screen width.
    uint16_t screenWidth = GetEmulatorWidth();
    
    for(uint16_t y = 0; y < windowHeight; y++)
    {
        for(uint16_t x = 0; x < windowWidth; x++)
        {
            // Since the window is 10 times bigger than the emulators
            // screen we divide x and y values by to more accuratly map them.
            pScreen[x + (windowWidth * y)] = m_pScreenBuffer[(x / 10) + (y / 10) * screenWidth] ? 0xFFFFFFFF : 0x00000000;
        };
    };
};

/**
 	Sets pressed key to 0x01

	@param[in] keyIndex of key in keyboard array.
 */
void Interpreter::OnKeyPressed(uint8_t keyIndex)
{
    m_keyboard[keyIndex] = 0x01;
};

/**
 	Sets the released key to 0x00

	 @param[in] keyIndex of key in keyboard array.
 */
void Interpreter::OnKeyReleased(uint8_t keyIndex)
{
    m_keyboard[keyIndex] = 0x00;
};

/**
    Allocates 4096 KB to emulate Chip8's amount of RAM and the screen buffer

 
    @return Successful state if we initialize the RAM and screen buffer
 */
bool Interpreter::InitializeEmulatorRAM()
{
    m_memory.fill(0x00);
    
    //  Retrieve the two higher nibbles for the width then multiply
    //  the two retrieved higher nibbles with the two lower to get
    //  the resolution of the Chip8 screen.
     
	uint16_t pixels = (static_cast<uint16_t>(m_screenSize) >> 8) * (0x00FF & static_cast<uint16_t>(m_screenSize));
    m_pScreenBuffer.reset(new uint8_t[pixels]);
    std::fill(
              m_pScreenBuffer.get(),
              m_pScreenBuffer.get() + pixels,
              0x00);
    
    return !m_memory.empty() && m_pScreenBuffer != nullptr;
};

/**
 	Initialize the keys in keyboard to 0x00
 */
bool Interpreter::InitializeEmulatorKeyboard()
{
    m_keyboard.fill(0x00);
    return true;
};

/**
    Initializes the fontset for Chip8
 
    @return Successfull state if we fill the fontset with keys '0' through 'F'
 */
bool Interpreter::InitializeFontset()
{   
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

	std::memcpy(&m_memory[0x00], &m_fontset[0x00], g_chipFontsetSize);
    
    return !m_fontset.empty();
};

/**
    Opens and loads the requested ROM file.\n
    Incase of being unable to locate or place the ROM in memory it will fail.

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
    if (romSize < 0 && romSize > (g_chipRamSize - 0x1FF))
    {
        printf("File to large, RAM size is %u\n", g_chipRamSize);
        file.close();
        return false;
    };
    
	uint16_t fileIndex = 0;
    file.seekg(0, std::ifstream::beg);

	// Read from file one byte at the time.
	while (file)
	{
		// Add bytes from byte 512 until the end of the file.
		m_memory[0x0200 + fileIndex] = file.get();
		fileIndex++; // Increase the index after each byte.
	};
    file.close();
    
    return !file.is_open();
};

/**
	Retrieve emulator screen width from screenSize
*/
uint16_t Interpreter::GetEmulatorWidth() const
{
	return static_cast<uint16_t>(m_screenSize) >> 8;
};

/**
	Retrieve emulator screen height from screenSize
*/
uint16_t Interpreter::GetEmulatorHeight() const
{
	return (0x00FF & static_cast<uint16_t>(m_screenSize));
};
