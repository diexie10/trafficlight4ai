#include <cctype>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <string>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>

static std::string computeDefaultSocketPath()
{
    // Priority: $XDG_RUNTIME_DIR/trafficlight4ai.sock, fallback /tmp/trafficlight4ai-$UID.sock
    const char *xdg = std::getenv("XDG_RUNTIME_DIR");
    if (xdg && xdg[0] != '\0')
        return std::string(xdg) + "/trafficlight4ai.sock";
    return "/tmp/trafficlight4ai-" + std::to_string(getuid()) + ".sock";
}

int main(int argc, char *argv[])
{
    // Drain stdin non-blocking to avoid broken pipe from callers (e.g. Codex hooks)
    int stdinFlags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, stdinFlags | O_NONBLOCK);
    char buf[4096];
    while (read(STDIN_FILENO, buf, sizeof(buf)) > 0) {}
    fcntl(STDIN_FILENO, F_SETFL, stdinFlags);

    if (argc < 2)
        return 0;

    // Determine socket path: env var > default (XDG_RUNTIME_DIR > /tmp with UID)
    const char *envPath = std::getenv("TL4AI_SOCKET");
    std::string defaultPath = computeDefaultSocketPath();
    const char *socketPath = envPath ? envPath : defaultPath.c_str();

    // Build command: uppercase the argument, append newline
    std::string command = argv[1];
    for (auto &c : command)
        c = static_cast<char>(toupper(static_cast<unsigned char>(c)));
    command += '\n';

    // Create socket
    int fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd < 0)
        return 0;

    // Set non-blocking for connect timeout
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);

    struct sockaddr_un addr{};
    addr.sun_family = AF_UNIX;
    if (strlen(socketPath) >= sizeof(addr.sun_path)) {
        close(fd);
        return 0; // path too long, cannot connect
    }
    strncpy(addr.sun_path, socketPath, sizeof(addr.sun_path) - 1);

    // Connect (non-blocking)
    int ret = connect(fd, reinterpret_cast<struct sockaddr *>(&addr), sizeof(addr));
    if (ret < 0 && errno == EINPROGRESS) {
        struct pollfd pfd{};
        pfd.fd = fd;
        pfd.events = POLLOUT;
        // 100ms timeout
        if (poll(&pfd, 1, 100) <= 0) {
            close(fd);
            return 0;
        }
        int err = 0;
        socklen_t len = sizeof(err);
        getsockopt(fd, SOL_SOCKET, SO_ERROR, &err, &len);
        if (err != 0) {
            close(fd);
            return 0;
        }
    } else if (ret < 0) {
        close(fd);
        return 0;
    }

    // Restore blocking for send
    fcntl(fd, F_SETFL, flags);

    // Send command (retry on partial write)
    size_t sent = 0;
    while (sent < command.size()) {
        ssize_t n = write(fd, command.c_str() + sent, command.size() - sent);
        if (n <= 0)
            break;
        sent += static_cast<size_t>(n);
    }
    close(fd);
    return 0;
}
