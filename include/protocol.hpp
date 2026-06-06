#pragma once

#include <cstdint>


enum class CommandType : uint32_t {
    PlayGif = 1,
    Exit = 2,
};

struct CommandHeader {
    CommandType type;
    uint32_t payloadSize;
};
