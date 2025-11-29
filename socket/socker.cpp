#include"socket.h"

Socket::Socket():sockfd_(socket(AF_INET,SOCK_STREAM,0)), conn_fd_(0), received_(0), sent_(0) {
    if(sockfd_ < 0) {
        errorMessage("socket failed: ");
    }
    logger.log(LogLevel::DEBUG, "create success!\n");
}

Socket::~Socket() {
    close();
}

void Socket::close() {
    if(sockfd_ > 0) {
        ::close(sockfd_);
        sockfd_ = 0;
    }
    if(conn_fd_ > 0) {
        ::close(conn_fd_);
        conn_fd_ = 0;
    }
}

bool Socket::bind(std::string ip, int port) {
    ip_ = ip;
    port_ = port;
    std::memset(&sock_addr_, 0, sizeof(sock_addr_));
    sock_addr_.sin_family = AF_INET;
    sock_addr_.sin_port = htons(port_);
    if(inet_pton(AF_INET, ip_.c_str(), &sock_addr_.sin_addr) < 0) {
        errorMessage("inet_pton failed: ");
        close();
        return 1;
    }

    if(::bind(sockfd_, reinterpret_cast<sockaddr*>(&sock_addr_), sizeof(sock_addr_)) < 0) {
        errorMessage("bind failed: ");
        close();
        return 1;
    }
    logger.log(LogLevel::DEBUG, "bind success!\n");
    return 0;
}

bool Socket::listen(size_t num) {
    if(::listen(sockfd_, num) < 0) {
        errorMessage("listen failed: ");
        close();
        return 1;
    }
    logger.log(LogLevel::DEBUG, "server listening...\n");
    return 0;
}

bool Socket::accept() {
    sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    conn_fd_ = ::accept(sockfd_, reinterpret_cast<sockaddr *>(&client_addr), &client_len);
    if(conn_fd_ < 0) {
        errorMessage("connect failed: ");
        close();
        return 1;
    }

    inet_ntop(AF_INET, &client_addr.sin_addr, client_ip_, sizeof(client_ip_));
    client_port_ = ntohs(client_addr.sin_port);
    logger.log(LogLevel::DEBUG, "accept success!\n");
    return 0;
}

bool Socket::connect(const char *server_ip, int server_port) {
    server_ip_ = server_ip;
    server_port_ = server_port;
    std::memset(&sock_addr_, 0, sizeof(sock_addr_));
    sock_addr_.sin_family = AF_INET;
    sock_addr_.sin_port = htons(server_port_);
    if(inet_pton(AF_INET, server_ip_, &sock_addr_.sin_addr) < 0) {
        errorMessage("inet_pton() failed: error = ");
        close();
        return 1;
    }

    if(::connect(sockfd_, reinterpret_cast<sockaddr*>(&sock_addr_), sizeof(sock_addr_)) < 0) {
        errorMessage("connect failed: error = ");
        close();
        return 1;
    }
    logger.log(LogLevel::DEBUG, "connect success!\n");
    return 0;
}

ssize_t Socket::recv() {
    received_ = ::recv(conn_fd_, recv_buf_, BUF_SIZE, 0);
    if(received_ < 0) {
        errorMessage("recv failed: ");
        close();
        return -1;
    }
    else if(received_ == 0) {
        logger.log(LogLevel::WARN, "client disconnect\n");
        close();
        return -1;
    }
    return received_;
}

bool Socket::send(const char *msg, size_t len) {
    sent_ = 0;
    while(sent_ < len) {
        ssize_t n = ::send(conn_fd_, msg + sent_, len - sent_, 0);
        if(n <= 0) break;
        sent_ += n;
    }
    if(sent_ < len) {
        errorMessage("send failed: ");
        close();
        return 1;
    }
}

void Socket::errorMessage(std::string_view err_t) {
    std::ostringstream oss;
    oss << err_t << strerror(errno) << '\n';
    logger.log(LogLevel::ERROR, oss.str());
}