
#include <iostream>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <cstring>

#include <argparse/argparse.hpp>

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


bool send_shutdown(int fd)
{
    CommandHeader header{
        CommandType::Shutdown,
        0
    };

    return write_all(fd, &header, sizeof(header));
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
    argparse::ArgumentParser parser("pijector", "1.0");

    argparse::ArgumentParser gif_command("gif");
    gif_command.add_description("Play a GIF file on the display service");
    gif_command.add_argument("path")
        .help("Path to the GIF file to play");

    argparse::ArgumentParser shutdown_command("shutdown");
    shutdown_command.add_description("Shutdown the display service");

    parser.add_subparser(gif_command);
    parser.add_subparser(shutdown_command);

    try {
        parser.parse_args(argc, argv);
    } catch (const std::runtime_error& err) {
        std::cerr << err.what() << std::endl;
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
        std::cout << "ERROR: Could not connect to server. Is it running?" << std::endl;
        return 1;
    }

    if (parser.is_subcommand_used(gif_command)) {
        send_play_gif(fd, gif_command.get("path"));
    } else if (parser.is_subcommand_used(shutdown_command)) { 
        send_shutdown(fd);
    }

    close(fd);
}
