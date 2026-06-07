
#include <filesystem>
#include <iostream>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <vector>

#include "raylib.h"
#include "stb_image.h"

#include "protocol.hpp"
#include "gif.hpp"


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


std::optional<std::pair<CommandHeader, std::string>> wait_for_command(int serverFd) {

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

    std::string payload;
    payload.resize(header.payloadSize);

    if (header.payloadSize > 0)
    {
        if (!read_all(clientFd,
                      payload.data(),
                      header.payloadSize))
        {
            close(clientFd);
        return std::nullopt;
        }
    }

    return std::make_pair(header, payload);
}


void play_gif(const std::filesystem::path& gifPath, unsigned int loops = 1) {
    std::vector<GifFrame> gifFrames;
    try {
        gifFrames = load_gif(gifPath.string().c_str());
    } catch (std::exception& e) {
        std::cerr << "Failed to load GIF: " << e.what() << std::endl;
        return;
    }

    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();
    int screenCenterX = screenWidth / 2;
    int screenCenterY = screenHeight / 2;

    int gifWidth = gifFrames.front().texture.width;
    int gifHeight = gifFrames.front().texture.height;
    int gifX = screenCenterX - (gifWidth / 2);
    int gifY = screenCenterY - (gifHeight / 2);

    unsigned int loopCount = 0;
    int currentFrame = 0;
    double nextFrameTime = GetTime() + gifFrames[currentFrame].delaySeconds;
    while (!WindowShouldClose()) {
        double now = GetTime();

        if (now >= nextFrameTime)
        {
            currentFrame = (currentFrame + 1);
            if (currentFrame >= gifFrames.size())
            {
                if (++loopCount >= loops) {
                    break;
                }
                currentFrame = 0;;
            }
            nextFrameTime += gifFrames[currentFrame].delaySeconds;
        }
        BeginDrawing();
        ClearBackground(BLACK);

        DrawTexture(
            gifFrames[currentFrame].texture,
            gifX,
            gifY,
            WHITE);

        EndDrawing();
    }
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

        std::cout << "Payload: " << payload << std::endl;

        if (header.type == CommandType::Shutdown) {
            std::cout << "Shutting down..." << std::endl;
            break;

        } else if (header.type == CommandType::PlayGif) {
            std::istringstream iss(payload);
            std::string gifName;
            unsigned int loops;
            iss >> gifName;
            iss >> loops;

            std::filesystem::path gifPath = std::filesystem::path(RESOURCE_PATH) / gifName;
            play_gif(gifPath, loops=loops);
        }
    }
}


int main(int argc, char** argv) {
    try {
        loop();
    } catch (std::exception& e) {
        std::cerr << "ERROR: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
};
