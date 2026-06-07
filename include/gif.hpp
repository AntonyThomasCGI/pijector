
#pragma once

#include "raylib.h"
#include "stb_image.h"


#include <fstream>
#include <iostream>
#include <vector>



struct GifFrame
{
    Texture2D texture;
    float delaySeconds;
};


std::vector<unsigned char> read_file(const char* filename)
{
    std::ifstream file(filename, std::ios::binary);

    if (!file)
        throw std::runtime_error("Failed to open file");

    file.seekg(0, std::ios::end);
    size_t size = file.tellg();
    file.seekg(0);

    std::vector<unsigned char> buffer(size);
    file.read(reinterpret_cast<char*>(buffer.data()), size);

    return buffer;
}

std::vector<GifFrame> load_gif(const char* filename)
{
    auto fileData = read_file(filename);

    int* delays = nullptr;

    int width;
    int height;
    int frameCount;
    int comp;

    unsigned char* gifPixels =
        stbi_load_gif_from_memory(
            fileData.data(),
            static_cast<int>(fileData.size()),
            &delays,
            &width,
            &height,
            &frameCount,
            &comp,
            4); // force RGBA

    if (!gifPixels)
    {
        throw std::runtime_error(stbi_failure_reason());
    }

    std::vector<GifFrame> frames;

    const int frameSize = width * height * 4;

    for (int i = 0; i < frameCount; i++)
    {
        unsigned char* framePixels =
            gifPixels + i * frameSize;

        Image image{};
        image.data = framePixels;
        image.width = width;
        image.height = height;
        image.mipmaps = 1;
        image.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;

        Texture2D texture = LoadTextureFromImage(image);

        GifFrame frame;
        frame.texture = texture;

        // stb returns delays in milliseconds
        frame.delaySeconds = delays[i] / 1000.0f;

        // some GIFs contain 0-delay frames
        if (frame.delaySeconds <= 0.0f)
            frame.delaySeconds = 0.1f;

        frames.push_back(frame);
    }

    stbi_image_free(gifPixels);
    stbi_image_free(delays);

    return frames;
}
