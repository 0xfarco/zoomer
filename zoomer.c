#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <stdlib.h>

void TakeScreenshot(unsigned char** out_data, int *out_width, int *out_height) {
    Display* display = XOpenDisplay(NULL);
    if (!display) {
        printf("Cannot open display\n");
        return;
    }

    int screen = DefaultScreen(display);
    Window root = RootWindow(display, screen);
    int width = DisplayWidth(display, screen);
    int height = DisplayHeight(display, screen);

    *out_width = width;
    *out_height = height;

    XImage* img = XGetImage(display, root, 0, 0, width, height, AllPlanes, ZPixmap);
    if (!img) {
        printf("Cannot get image\n");
        return;
    }

    *out_data = (unsigned char*)malloc(3 * width * height);
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            long pixel = XGetPixel(img, x, y);
            (*out_data)[(y * width + x) * 3 + 0] = (pixel & img->red_mask) >> 16;
            (*out_data)[(y * width + x) * 3 + 1] = (pixel & img->green_mask) >> 8;
            (*out_data)[(y * width + x) * 3 + 2] = (pixel & img->blue_mask);
        }
    }

    XDestroyImage(img);
    XCloseDisplay(display);
}

void ShowImageWithSDL(unsigned char* image_data, int width, int height) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("SDL_Init Error: %s\n", SDL_GetError());
        return;
    }

    SDL_Window *win = SDL_CreateWindow("Screenshot Viewer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_SHOWN);
    if (!win) {
        printf("SDL_CreateWindow Error: %s\n", SDL_GetError());
        SDL_Quit();
        return;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        printf("SDL_CreateRenderer Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(win);
        SDL_Quit();
        return;
    }

    SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING, width, height);
    SDL_UpdateTexture(texture, NULL, image_data, width * 3);

    float zoom = 1.0f;
    SDL_Rect destRect = {0, 0, width, height};
    int cursor_x, cursor_y;
    int dragging = 0;
    int last_x, last_y;

    int quit = 0;
    SDL_Event event;
    while (!quit) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = 1;
            } else if (event.type == SDL_MOUSEWHEEL && (SDL_GetModState() & KMOD_CTRL)) {
                SDL_GetMouseState(&cursor_x, &cursor_y);
                float prev_zoom = zoom;
                zoom *= (event.wheel.y > 0) ? 1.1f : 1.0f / 1.1f;
                
                destRect.w = (int)(width * zoom);
                destRect.h = (int)(height * zoom);
                
                destRect.x = cursor_x - (cursor_x - destRect.x) * (zoom / prev_zoom);
                destRect.y = cursor_y - (cursor_y - destRect.y) * (zoom / prev_zoom);
            } else if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
                dragging = 1;
                last_x = event.button.x;
                last_y = event.button.y;
            } else if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_LEFT) {
                dragging = 0;
            } else if (event.type == SDL_MOUSEMOTION && dragging) {
                destRect.x += event.motion.x - last_x;
                destRect.y += event.motion.y - last_y;
                last_x = event.motion.x;
                last_y = event.motion.y;
            }
        }
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, &destRect);
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(win);
    SDL_Quit();
    free(image_data);
}

int main() {
    int width, height;
    unsigned char *image_data;

    TakeScreenshot(&image_data, &width, &height);
    ShowImageWithSDL(image_data, width, height);

    return 0;
}

