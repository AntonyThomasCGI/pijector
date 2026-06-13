#pragma once

#include <cstdint>


enum class CommandType : uint32_t {
    PlayGif = 1,
    AutoPlayGif = 2,
    Shutdown = 3,
};

struct CommandHeader {
    CommandType type;
    uint32_t payloadSize;
};
