#include"server/serversocket.h"

Logger &logger = Logger::instance();

ServerSocket::ServerSocket():sockfd_(socket(AF_INET,SOCK_STREAM,0)), conn_fd_(0) {
    if(sockfd_ < 0) {
        errorMessage("socket failed: ");
    }
    logger.log(LogLevel::DEBUG, "create success!\n");
}

ServerSocket::ServerSocket(const std::string &ip, size_t port):ServerSocket() {
    // set_non_blocking();
    set_recv_buffer(10 * 1024);
    set_send_buffer(10 * 1024);
    // set_linger(true, 0);
    set_keepalive();
    set_reuesaddr();

    bind(ip, port);
    listen(1024);
}

ServerSocket::~ServerSocket() {
    close();
}

void ServerSocket::bind(const std::string ip, int port) {
    sockaddr_in sock_addr;
    std::memset(&sock_addr, 0, sizeof(sock_addr));
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_port = htons(port);
    if(inet_pton(AF_INET, ip.c_str(), &sock_addr.sin_addr) < 0) {
        errorMessage("inet_pton failed: ");
    }

    if(::bind(sockfd_, reinterpret_cast<sockaddr*>(&sock_addr), sizeof(sock_addr)) < 0) {
        errorMessage("bind failed: ");
    }
    logger.log(LogLevel::DEBUG, "bind success!\n");
}

void ServerSocket::listen(size_t num) {
    if(::listen(sockfd_, num) < 0) {
        errorMessage("listen failed: ");
    }
    logger.log(LogLevel::DEBUG, "server listening...\n");
}

int ServerSocket::accept() {
    sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    conn_fd_ = ::accept(sockfd_, reinterpret_cast<sockaddr *>(&client_addr), &client_len);
    if(conn_fd_ < 0) {
        errorMessage("connect failed: ");
        return -1;
    }

    std::vector<char> client_ip(INET_ADDRSTRLEN);
    int client_port;
    inet_ntop(AF_INET, &client_addr.sin_addr, client_ip.data(), sizeof(client_ip));
    client_port = ntohs(client_addr.sin_port);

    // std::string(client_ip.data(), client_ip.size())
    // 结果 字符串后面包含多个空字符 \0，导致日志系统可能 截断或显示异常
    std::string client_ip_str(client_ip.data());
    std::ostringstream oss;
    oss << "accept conntection from " << client_ip_str << '\n';
    logger.log(LogLevel::DEBUG, oss.str());
    return conn_fd_;
}

std::vector<char> ServerSocket::recv(size_t buf_size) {
    std::vector<char> buf(buf_size);
    ssize_t received = ::recv(conn_fd_, buf.data(), buf_size, 0);
    if(received < 0) {
        errorMessage("recv failed: ");
        return {};
    }
    else if(received == 0) {
        logger.log(LogLevel::WARN, "client disconnect\n");
        return {};
    }
    logger.log(LogLevel::DEBUG, "server recv success!\n");
    return buf;
}

size_t ServerSocket::send(const std::vector<char> &data) {
    size_t total_sent = 0;
    size_t to_send = data.size();
    const char *data_ptr = data.data();
    while(total_sent < to_send) {
        ssize_t n = ::send(conn_fd_, data_ptr + total_sent, to_send - total_sent, 0);
        if(n <= 0) break;
        total_sent += n;
    }
    if(total_sent < to_send) {
        errorMessage("send failed: ");
    }
    return total_sent;
}

void ServerSocket::close() {
    if(sockfd_ > 0) {
        ::close(sockfd_);
        sockfd_ = 0;
    }
    if(conn_fd_ > 0) {
        ::close(conn_fd_);
        conn_fd_ = 0;
    }
}

void ServerSocket::closeConnect() {
    if(conn_fd_ > 0) {
        ::close(conn_fd_);
        conn_fd_ = 0;
    }
}

void ServerSocket::closeListen() {
    if(sockfd_ > 0) {
        ::close(sockfd_);
        sockfd_ = 0;
    }
}

void ServerSocket::errorMessage(std::string_view err_t) {
    std::ostringstream oss;
    oss << err_t << strerror(errno) << '\n';
    logger.log(LogLevel::ERROR, oss.str());
}

void ServerSocket::set_non_blocking() {
    int flags = fcntl(sockfd_, F_GETFL, 0); // file control
    if(flags < 0) {
        errorMessage("get_non_blocking failed: ");
        return;
    }
    flags |= O_NONBLOCK;
    if(fcntl(sockfd_, F_SETFL,flags) < 0) {
        errorMessage("set_non_blocking failed: ");
    }
}

void ServerSocket::set_send_buffer(size_t size)
{
    if(setsockopt(sockfd_, SOL_SOCKET, SO_SNDBUF, &size, sizeof(size)) < 0) {
        errorMessage("set_send_buffer failed: ");
    }
}

void ServerSocket::set_recv_buffer(size_t size) {
    if(setsockopt(sockfd_, SOL_SOCKET, SO_RCVBUF, &size, sizeof(size)) < 0) {
        errorMessage("set_recv_buffer failed: ");
    }
}
void ServerSocket::set_linger(bool active, size_t seconds) {
    linger l;
    std::memset(&l, 0, sizeof(l));
    l.l_onoff = active ? 1 : 0;
    l.l_linger = seconds;
    if(setsockopt(sockfd_, SOL_SOCKET, SO_LINGER, &l, sizeof(l)) < 0) {
        errorMessage("set_linger failed: ");
    }
}

void ServerSocket::set_keepalive() {
    int flag = 1;
    if(setsockopt(sockfd_, SOL_SOCKET, SO_KEEPALIVE, &flag, sizeof(flag)) < 0) {
        errorMessage("set_keepalive failed: ");
    }
}

void ServerSocket::set_reuesaddr() {
    int flag = 1;
    if(setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag)) < 0) {
        errorMessage("set_reuseaddr failed: ");
    }
}