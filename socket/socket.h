#include <iostream>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/socket.h>
#include <cstring>

#include "log.h"

Logger &logger = Logger::instance();

class Socket {
public:
    Socket();

    ~Socket();
    void close();
    bool bind(std::string, int);
    bool listen(size_t num);

    bool accept();
    bool connect(const char *, int);
    ssize_t recv();
    bool send(const char *, size_t);
private:
    void errorMessage(std::string_view);
    int sockfd_;
    int conn_fd_;
    std::string ip_;
    const char *server_ip_;
    char client_ip_[INET_ADDRSTRLEN];
    int port_;
    int server_port_;
    int client_port_;
    sockaddr_in sock_addr_;
    static const int BUF_SIZE = 4096;
    char recv_buf_[BUF_SIZE + 1];
    char send_buf_[BUF_SIZE + 1];
    ssize_t received_;
    ssize_t sent_;
};