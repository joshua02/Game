/*
  Copyright (C) 1997-2025 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely.
*/
#define SDL_MAIN_USE_CALLBACKS 1  /* use the callbacks instead of main() */
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_vulkan.h>

#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <vulkan/vulkan.h>

#include <iostream>

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;

/* This function runs once at startup. */
SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
    /* Create the window */
    
    SDL_Window* window = SDL_CreateWindow(
        "Vulkan Window",
        800,
        600,
        SDL_WINDOW_VULKAN
    );

    if (!window) {
        std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << "\n";
        SDL_Quit();
        return SDL_APP_FAILURE;
    }

    std::cout << "Window created successfully!\n";

    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

    std::cout << extensionCount << " extensions supported\n";

    glm::mat4 matrix;
    glm::vec4 vec;
    auto test = matrix * vec;

    SDL_DestroyWindow(window);
    return SDL_APP_SUCCESS;

    return SDL_APP_CONTINUE;
}

/* This function runs when a new event (mouse input, keypresses, etc) occurs. */
SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
    if (event->type == SDL_EVENT_KEY_DOWN ||
        event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;  /* end the program, reporting success to the OS. */
    }
    return SDL_APP_CONTINUE;
}

/* This function runs once per frame, and is the heart of the program. */
SDL_AppResult SDL_AppIterate(void *appstate)
{
    return SDL_APP_CONTINUE;
}

/* This function runs once at shutdown. */
void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
}

