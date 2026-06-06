
#include <iostream>
#include <filesystem>

#include "raylib.h"
#include "stb_image.h"

#ifndef RESOURCE_PATH
#define RESOURCE_PATH "./resources/"
#endif


int main(int argc, char *argv[]) {

    std::string gifName;
    if (argc > 1) {
        gifName = argv[1];
    } else {
        gifName = "babyboo-pokko.gif";
    }
    std::filesystem::path gifPath = std::filesystem::path(RESOURCE_PATH) / gifName;

    SetConfigFlags(FLAG_WINDOW_UNDECORATED | FLAG_WINDOW_RESIZABLE);

    InitWindow(GetMonitorWidth(0), GetMonitorHeight(0), "Pijector");

    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();

    int screenCenterX = screenWidth / 2;
    int screenCenterY = screenHeight / 2;


    int gifFrames = 0;
    Image gifImage = LoadImageAnim(gifPath.string().c_str(), &gifFrames);
    Texture2D gifTexture = LoadTextureFromImage(gifImage);

    int gifX = screenCenterX - (gifImage.width / 2);
    int gifY = screenCenterY - (gifImage.height / 2);

    int currentFrame = 0;
    int frameCounter = 0;
    int frameDelay = 2;

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        frameCounter++;
        if (frameCounter >= frameDelay) {
            currentFrame++;
            if (currentFrame >= gifFrames) {
                //currentFrame = 0;
                // Exit on gif finish
                break;
            }
            int nextFrameDataOffset = currentFrame * gifImage.width * gifImage.height * 4; // Assuming RGBA format

            UpdateTexture(gifTexture, ((unsigned char *)gifImage.data) + nextFrameDataOffset);

            frameCounter = 0;
        }
        BeginDrawing();
        ClearBackground(BLACK);

        DrawTexture(gifTexture, gifX, gifY, WHITE);

        EndDrawing();
    }

    UnloadImage(gifImage);
    UnloadTexture(gifTexture);

    CloseWindow();
    return 0;
}
