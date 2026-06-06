
#include <iostream>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <cstring>

#include "protocol.hpp"


bool write_all(int fd, const void* data, size_t size)
{
    const char* ptr = static_cast<const char*>(data);

    while (size > 0)
    {
        ssize_t written = write(fd, ptr, size);

        if (written <= 0)
            return false;

        ptr += written;
        size -= written;
    }

    return true;
}


bool send_play_gif(int fd, const std::string& path)
{
    CommandHeader header{
        CommandType::PlayGif,
        static_cast<uint32_t>(path.size())
    };

    if (!write_all(fd, &header, sizeof(header)))
        return false;

    if (!write_all(fd, path.data(), path.size()))
        return false;

    return true;
}


int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <gif_file>" << std::endl;
        return 1;
    }

    const char* socket_path = "/tmp/pijector.sock";

    int fd = socket(AF_UNIX, SOCK_STREAM, 0);

    sockaddr_un addr{};
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path) - 1);

    if (connect(fd,
                reinterpret_cast<sockaddr*>(&addr),
                sizeof(addr)) < 0)
    {
        std::cout << "ERROR: connect" << std::endl;
        return 1;
    }

    send_play_gif(fd, argv[1]);

    close(fd);
}
