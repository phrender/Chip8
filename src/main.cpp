#include <fstream>
#include <memory>
#include "Interpreter.hpp"
#include <SDL.h>

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
 * Initialize SDL for Chip8 emulator
 *
 * @param[in] windowName  nameof the window as a std::string
 * @param[in] windowWidth width of the window
 * @param[in] windowHeight height of the window
 *
 * @return sucessful state if SDL is initialized.
 */
bool InitializeSDL(const std::string& windowName, uint32_t windowWidth, uint32_t windowHeight);
void HandleInput();
void ShutdownSDL();

int main(int argc, char** argv)
{
    if (auto data = std::make_unique<Interpreter>())
    {
        if (argc == 2 && data->Initialize(argv[1]) && InitializeSDL("Chip8", 640, 320))
        {
            uint32_t* pScreen = static_cast<uint32_t*>(g_pSurface->pixels);
            while (!g_quit)
            {
                data->Run();
                HandleInput();
                
                SDL_LockSurface(g_pSurface);
                std::memset(pScreen, 0x00000000, (g_pSurface->w * g_pSurface->h * sizeof(uint32_t)));
                SDL_UnlockSurface(g_pSurface);
                
                SDL_UpdateWindowSurface(g_pWindow);
            }
            ShutdownSDL();
            
            return 0;
        };
    };
    
    printf("Failed to initialize Chip8 Emulator!\n");
    return -1;
};

bool InitializeSDL(const std::string& windowName, uint32_t windowWidth, uint32_t windowHeight)
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
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
    
    return true;
};

/**
    Handles input via SDL
 */
void HandleInput()
{
    SDL_Event e;
    
    while (SDL_PollEvent(&e) != 0) {
        switch (e.type) {
            case SDL_QUIT:
                g_quit = true;
                break;
                
            default:
                break;
        }
    }
};

/**
    Shutsdown SDL for the emulator
 */
void ShutdownSDL()
{
    if (g_pSurface != nullptr)
    {
        SDL_FreeSurface(g_pSurface);
        g_pSurface = nullptr;
    };
    
    if (g_pWindow != nullptr)
    {
        SDL_DestroyWindow(g_pWindow);
        g_pWindow = nullptr;
    };
    
    SDL_Quit();
};
