#include"clientsocket.h"

ClientSocket::ClientSocket():sockfd_(socket(AF_INET,SOCK_STREAM,0)) {
    if(sockfd_ < 0) {
        errorMessage("socket failed: ");
    }
    logger.log(LogLevel::DEBUG, "create success!\n");
}

ClientSocket::~ClientSocket() {
    close();
}

void ClientSocket::close() {
    if(sockfd_ > 0) {
        ::close(sockfd_);
        sockfd_ = 0;
    }
}

void ClientSocket::connect(const std::string &server_ip, int server_port) {
    sockaddr_in server_addr;
    std::memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    if(inet_pton(AF_INET, server_ip.c_str(), &server_addr.sin_addr) < 0) {
        errorMessage("inet_pton() failed: error = ");
        close();
        return;
    }

    if(::connect(sockfd_, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr)) < 0) {
        errorMessage("connect failed: error = ");
        close();
        return;
    }
    logger.log(LogLevel::DEBUG, "connect success!\n");
}

std::vector<char> ClientSocket::recv(size_t buf_size)
{
    std::vector<char> buf(buf_size);
    ssize_t received = ::recv(sockfd_, buf.data(), buf_size, 0);
    if(received < 0) {
        errorMessage("recv failed: ");
    }
    else if(received == 0) {
        logger.log(LogLevel::WARN, "client disconnect\n");
        return {};
    }
    buf.resize(received);
    return buf;
}

size_t ClientSocket::send(const std::vector<char>& data) {
    ssize_t total_sent = 0;
    size_t to_send = data.size();
    const char *data_ptr = data.data();
    while(total_sent < to_send) {
        ssize_t n = ::send(sockfd_, data_ptr + total_sent, to_send - total_sent, 0);
        if(n <= 0) break;
        total_sent += n;
    }
    if(total_sent < to_send) {
        errorMessage("send failed: ");
    }
    return total_sent;
}

void ClientSocket::errorMessage(std::string_view err_t) {
    std::ostringstream oss;
    oss << err_t << strerror(errno) << '\n';
    logger.log(LogLevel::ERROR, oss.str());
}

void ClientSocket::set_non_blocking() {
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

void ClientSocket::set_send_buffer(size_t size)
{
    if(setsockopt(sockfd_, SOL_SOCKET, SO_SNDBUF, &size, sizeof(size)) < 0) {
        errorMessage("set_send_buffer failed: ");
    }
}

void ClientSocket::set_recv_buffer(size_t size) {
    if(setsockopt(sockfd_, SOL_SOCKET, SO_RCVBUF, &size, sizeof(size)) < 0) {
        errorMessage("set_recv_buffer failed: ");
    }
}
void ClientSocket::set_linger(bool active, size_t seconds) {
    linger l;
    std::memset(&l, 0, sizeof(l));
    l.l_onoff = active ? 1 : 0;
    l.l_linger = seconds;
    if(setsockopt(sockfd_, SOL_SOCKET, SO_LINGER, &l, sizeof(l)) < 0) {
        errorMessage("set_linger failed: ");
    }
}

void ClientSocket::set_keepalive() {
    int flag = 1;
    if(setsockopt(sockfd_, SOL_SOCKET, SO_KEEPALIVE, &flag, sizeof(flag)) < 0) {
        errorMessage("set_keepalive failed: ");
    }
}

void ClientSocket::set_reuesaddr() {
    int flag = 1;
    if(setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag)) < 0) {
        errorMessage("set_reuseaddr failed: ");
    }
}