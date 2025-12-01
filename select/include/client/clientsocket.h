#include <iostream>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/socket.h>
#include <cstring>
#include<string_view>
#include<sstream>
#include<cerrno>
#include<vector>
#include <fcntl.h> // file control : 意思是对文件描述符进行控制/配置。
#include "utility/log.h"

class ClientSocket {
public:
    ClientSocket();
    ClientSocket(const std::string &, size_t);

    ~ClientSocket();

    ClientSocket(const ClientSocket &) = delete;
    ClientSocket &operator=(const ClientSocket &) = delete;

    ClientSocket(ClientSocket &&other) noexcept;
    ClientSocket &operator=(ClientSocket &&other) noexcept;

    void close();

    void connect(const std::string&, int);
    std ::vector<char> recv(size_t);
    size_t send(const std::vector<char>&);

    void set_non_blocking();
    void set_send_buffer(size_t);
    void set_recv_buffer(size_t);
    void set_linger(bool, size_t); // linger: 徘徊，漫步
    void set_keepalive();
    void set_reuesaddr();

private:
    void errorMessage(std::string_view);
    int sockfd_;
};