#include "CHIP8.h"

#include <SDL2/SDL.h>
#include <fmt/core.h>
#include <chrono>
#include <string>
#include <thread>
#include <tuple>

auto initSDL() -> std::tuple<SDL_Window*, SDL_Renderer*, SDL_Texture*> {
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        fmt::print("error initializing SDL: {}\n", SDL_GetError());
    }

    // Aspect ratio must be 2:1
    auto* window = SDL_CreateWindow(
      "Chip-8 Emulator", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1024, 512, 0);
    if (window == nullptr) { fmt::print("error creating window: {}\n", SDL_GetError()); }

    auto* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == nullptr) { fmt::print("error creating renderer: {}\n", SDL_GetError()); }

    auto* texture =
      SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, 64, 32);
    if (texture == nullptr) { fmt::print("error creating texture: {}\n", SDL_GetError()); }

    return {window, renderer, texture};
}

auto deinitSDL(SDL_Window* window) -> void {
    SDL_DestroyWindow(window);
    SDL_Quit();
}

auto main(int argc, char* argv[]) -> int {
    if (argc < 2) {
        fmt::print("error running emulator: no path to target ROM provided\n");
        return 0;
    }

    auto romPath = std::string{argv[1]};
    auto cpu     = CHIP8{romPath};

    auto [window, renderer, texture] = initSDL();
    auto running                     = true;
    auto pixels                      = std::array<uint32_t, 4096>{};

    while (running) {
        // Run emulator cycle
        cpu.cycle();

        SDL_Event e;
        while (SDL_PollEvent(&e) > 0) {
            switch (e.type) {
                case SDL_QUIT:
                    running = false;
                    deinitSDL(window);
                    break;
            }
        }

        for (int i = 0; i < 2048; i++) {
            if (cpu.graphics[i] == 1) {
                pixels[i] = 0xFFFFFFFF;
            } else {
                pixels[i] = 0x000000FF;
            }
        }

        if (SDL_UpdateTexture(texture, nullptr, &pixels, 64 * sizeof(uint32_t)) != 0) {
            fmt::print("error updating texture: {}\n", SDL_GetError());
        }

        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);

        // Sleep for 16 milliseconds
        std::this_thread::sleep_for(std::chrono::milliseconds{16});
    }

    return 0;
}