#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <iostream>

int main() {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << "\n";
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow(
        "Vulkan Window",
        800,
        600,
        SDL_WINDOW_VULKAN
    );

    if (!window) {
        std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << "\n";
        SDL_Quit();
        return 1;
    }

    std::cout << "Window created successfully!\n";

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
