
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "protocol.hpp"

#include "raylib.h"
#include "stb_image.h"


#ifndef RESOURCE_PATH
#define RESOURCE_PATH "./resources/"
#endif


bool read_all(int fd, void* data, size_t size)
{
    char* ptr = static_cast<char*>(data);

    while (size > 0)
    {
        ssize_t n = read(fd, ptr, size);

        if (n <= 0)
            return false;

        ptr += n;
        size -= n;
    }

    return true;
}


int setup_socket() {
    const char *socketPath = "/tmp/pijector.sock";

    unlink(socketPath);

    int serverFd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (serverFd < 0) {
        throw std::runtime_error("Failed to create socket");
    }

    sockaddr_un addr{};
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, socketPath, sizeof(addr.sun_path) - 1);

    if (bind(serverFd, (sockaddr *)&addr, sizeof(addr)) < 0) {
        close(serverFd);
        throw std::runtime_error("Failed to bind socket");
    }

    if (listen(serverFd, 5) < 0) {
        close(serverFd);
        throw std::runtime_error("Failed to listen on socket");
    }

    std::cout << "Listening on " << socketPath << std::endl;

    return serverFd;
}


std::optional<std::pair<CommandHeader, std::vector<char>>> wait_for_command(int serverFd) {

    int clientFd = accept(serverFd, nullptr, nullptr);
    if (clientFd < 0) {
        std::cerr << "Failed to accept connection" << std::endl;
        return std::nullopt;
    }

    CommandHeader header;

    if (!read_all(clientFd, &header, sizeof(header)))
    {
        close(clientFd);
        return std::nullopt;
    }

    std::vector<char> payload(header.payloadSize);

    if (header.payloadSize > 0)
    {
        if (!read_all(clientFd,
                      payload.data(),
                      payload.size()))
        {
            close(clientFd);
        return std::nullopt;
        }
    }

    return std::make_pair(header, payload);
}


void loop() {
    int serverFd = setup_socket();

    SetConfigFlags(FLAG_WINDOW_UNDECORATED | FLAG_WINDOW_RESIZABLE);

    InitWindow(GetMonitorWidth(0), GetMonitorHeight(0), "Pijector");

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);

        EndDrawing();

        auto result = wait_for_command(serverFd);
        if (!result.has_value()) {
            continue;
        }
        auto [header, payload] = result.value();

        std::cout << "Received command: " << static_cast<uint32_t>(header.type)
                  << " with payload size: " << header.payloadSize
                  << std::endl;

        std::cout << "Payload: " << std::string(payload.data(), payload.size()) << std::endl;

        if (header.type == CommandType::PlayGif) {
            std::filesystem::path gifPath = std::filesystem::path(RESOURCE_PATH) / payload.data();
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
        }

    }

}


int main(int argc, char** argv) {
    try {
        loop();
    } catch (std::exception& e) {
        std::cerr << "ERROR: " << e.what() << std::endl;
    }

    return EXIT_SUCCESS;
};
