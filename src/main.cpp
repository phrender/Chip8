/*! \file
		Entry point for Chip8 Emulator
 */

#include <fstream>
#include <memory>
#include "Interpreter.hpp"
#include <SDL.h>

#undef main		// Undef main so we don't use SDL's main()

/**
 * SDL Window pointer
 */
SDL_Window* g_pWindow = nullptr;

/**
 * SDL Surface pointer
 */
SDL_Surface* g_pSurface = nullptr;

/**
    Bool value for when to exit the emulator.
 */
bool g_quit = false;

/**
	uint16_t to store screen width and height.
	Store width (64) in two upper nibbles and height (32) in the two lower nibbles.
*/
constexpr uint16_t g_screenSize = 0x4020;

/**
    Emulator interpreter pointer.
 */
std::unique_ptr<Interpreter> g_pInterpreter = nullptr;

/**
    Emulator key map.
 */
std::array<uint8_t, CHIP8_KEYBOARD_SIZE> g_keyboardMap = {};

/**
    Initialize SDL for Chip8 emulator

    @param[in] windowName  nameof the window as a std::string
    @param[in] windowWidth width of the window
    @param[in] windowHeight height of the window

    @return sucessful state if SDL is initialized.
 */
bool InitializeSDL(const std::string& windowName, uint32_t windowWidth, uint32_t windowHeight);

/**
    Handle input for the emulator
 */
void HandleInput();

/**
    Shutdown SDL when exiting the emulator
 */
void ShutdownSDL();

int main(int argc, char** argv)
{
    g_pInterpreter = std::make_unique<Interpreter>();
    
    if (InitializeSDL("Chip8", 640, 320) &&
        argc == 2 &&
        g_pInterpreter != nullptr &&
        g_pInterpreter->Initialize(argv[1], g_screenSize))
    {
        uint32_t* pScreen = static_cast<uint32_t*>(g_pSurface->pixels);
        while (!g_quit)
        {
            g_pInterpreter->Run();
            HandleInput();

            SDL_LockSurface(g_pSurface);
            std::memset(pScreen, 0x00000000, (g_pSurface->w * g_pSurface->h * sizeof(uint32_t)));
            
            g_pInterpreter->Draw(pScreen, 640, 320);
                
            SDL_UnlockSurface(g_pSurface);

            SDL_UpdateWindowSurface(g_pWindow);
        };
        ShutdownSDL();
            
        return 0;
    };
    
    printf("Failed to initialize Chip8 Emulator!\n");
    return -1;
};

bool InitializeSDL(const std::string& windowName, uint32_t windowWidth, uint32_t windowHeight)
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS  | SDL_INIT_AUDIO) != 0)
    {
        printf("Unable to initialize SDL: %s\n", SDL_GetError());
        return false;
    };
    
    g_pWindow = SDL_CreateWindow(
                                 windowName.c_str(),        // Window title as c-string
                                 SDL_WINDOWPOS_CENTERED,    // X position of the window
                                 SDL_WINDOWPOS_CENTERED,    // Y position of the window
                                 windowWidth,               // Width of the window
                                 windowHeight,              // HEight of the window
                                 SDL_WINDOW_SHOWN);         // SDL window flags
    
    if (!g_pWindow)
    {
        printf("Failed to initialize SDL_Window: %s\n", SDL_GetError());
        return false;
    }
    
    g_pSurface = SDL_GetWindowSurface(g_pWindow);
    
    g_keyboardMap =
    {
        SDLK_x, SDLK_1, SDLK_2, SDLK_3,     // 1, 2, 3, C
        SDLK_q, SDLK_w, SDLK_e, SDLK_a,     // 4, 5, 6, D
        SDLK_s, SDLK_d, SDLK_z, SDLK_c,     // 7, 8, 9, E
        SDLK_4, SDLK_r, SDLK_f, SDLK_v      // A, 0, B, F
    };
    
    return true;
};

/**
    Handles input via SDL
 */
void HandleInput()
{
    SDL_Event e;
    
    while (SDL_PollEvent(&e) != 0)
    {
        switch (e.type)
        {
            case SDL_QUIT:
                g_quit = true;
                break;
                
            case SDL_KEYDOWN:
                
                if (e.key.keysym.sym == SDLK_ESCAPE)
                {
                    g_quit = true;
                    break;
                };
                
                for (uint8_t keyIndex = 0; keyIndex < g_keyboardMap.size(); keyIndex++)
                {
                    if (e.key.keysym.sym == g_keyboardMap[keyIndex])
                    {
                        g_pInterpreter->OnKeyPressed(keyIndex);
                    };
                };
                
                break;
                
            case SDL_KEYUP:
                for (uint8_t keyIndex = 0; keyIndex < g_keyboardMap.size(); keyIndex++)
                {
                    if (e.key.keysym.sym == g_keyboardMap[keyIndex])
                    {
                        g_pInterpreter->OnKeyReleased(keyIndex);
                    };
                };
                break;
        };
    };
};

/**
    Shutsdown SDL for the emulator
 */
void ShutdownSDL()
{
    if (g_pSurface)
    {
        SDL_FreeSurface(g_pSurface);
        g_pSurface = nullptr;
    };
    
    if (g_pWindow)
    {
        SDL_DestroyWindow(g_pWindow);
        g_pWindow = nullptr;
    };
    
    if (g_pInterpreter)
    {
        g_pInterpreter.reset();
        g_pInterpreter = nullptr;
    };
    
    SDL_Quit();
};
