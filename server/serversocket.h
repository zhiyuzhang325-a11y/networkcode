#include <iostream>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/socket.h>
#include <cstring>
#include <string_view>
#include <sstream>
#include <cerrno>
#include <vector>
#include <fcntl.h> // file control : 意思是对文件描述符进行控制/配置。
#include "log.h"

Logger &logger = Logger::instance();

class ServerSocket {
public:
    ServerSocket();

    ~ServerSocket();

    ServerSocket(const ServerSocket &) = delete;
    ServerSocket &operator=(const ServerSocket &) = delete;

    ServerSocket(ServerSocket &&other) noexcept;
    ServerSocket &operator=(ServerSocket &&other) noexcept;

    void close();
    void bind(std::string, int);
    void listen(size_t num);

    void accept();
    std::vector<char> recv(size_t);
    size_t send(const std::vector<char>&);

    void set_non_blocking();
    void set_send_buffer(size_t);
    void set_recv_buffer(size_t);
    void set_linger(bool, size_t);  // linger: 徘徊，漫步
    void set_keepalive();
    void set_reuesaddr();

private:
    void errorMessage(std::string_view);
    int sockfd_;
    int conn_fd_;
};