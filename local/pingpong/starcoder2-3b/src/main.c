#include <SDL2/SDL.h>

int main(int argc, char* argv[]) {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize: %s\n", SDL_GetError());
        return 1;
    }

    // Create window
    SDL_Window* window = SDL_CreateWindow("SDL Hello World",
                                          SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED,
                                          640, 480,
                                          SDL_WINDOW_SHOWN);
    if (window == NULL) {
        printf("Window could not be created: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    // Create renderer
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) {
        printf("Renderer could not be created: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Main loop flag
    bool quit = false;
    SDL_Event e;

    // Clear console (for Windows)
    #ifdef _WIN32
    system("cls");
    #else
    system("clear");
    #endif

    printf("Hello World!\n");

    // Main rendering loop
    while (!quit) {
        // Handle events
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            }
        }

        // Clear screen
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Draw text
        SDL_Texture* texture = SDL_CreateTexture(renderer,
                                                SDL_PIXELFORMAT_RGBA8888,
                                                SDL_TEXTUREACCESS_STREAMING,
                                                256, 128);
        SDL_Surface* textSurface = SDL_CreateRGBSurfaceWithFormat(NULL, 256, 128, 32, SDL_PIXELFORMAT_RGBA8888);
        SDL_FillRect(textSurface, NULL, SDL_MapRGBA(textSurface->format, 0, 0, 0, 255));
        SDL_Color textColor = {255, 255, 255, 255};
        SDL_Rect textRect = {10, 10, 256, 128};
        SDL_FillRect(textSurface, &textRect, SDL_MapRGBA(textSurface->format, 255, 255, 255, 255));
        SDL_RenderCopy(renderer, texture, NULL, &textRect);

        // Render
        SDL_RenderPresent(renderer);

        SDL_Delay(1000); // Delay to prevent busy waiting
    }

    // Cleanup
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}