#include<arpa/inet.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<unistd.h>
#include<cstring>
#include<iostream>

int main() {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        std::cerr << "socket create failed: error: " << strerror(errno) << '\n';
        close(sockfd);
        return 1;
    }

    std::string ip = "127.0.0.1";
    int port = 8080;
    sockaddr_in sock_addr;
    std::memset(&sock_addr, 0, sizeof(sock_addr));
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip.c_str(), &sock_addr.sin_addr) < 0) {
        std::cerr << "inet_pton() failed: error = " << strerror(errno) << '\n';
        close(sockfd);
        return 1;
    }

    // 0 成功   -1 失败
    if (connect(sockfd, reinterpret_cast<sockaddr *>(&sock_addr), sizeof(sock_addr)) < 0) {
        std::cerr << "connect failed: error = " << strerror(errno) << '\n';
        return 1;
    }
    std::cout << "connect success!\n";

    std::string date = "Hello World";
    ssize_t date_len = date.size();
    ssize_t sent = 0;
    while(sent < date_len) {
        ssize_t n = send(sockfd, date.c_str() + sent, date_len - sent, 0);
        if (n < 0) break;
        sent += n;
    }
    if(sent < date_len) {
            std::cerr << "send fail: error = " << strerror(errno) << '\n';
            close(sockfd);
            return 1;
    }
    std::cout << "send success!\n";

    const int BUF_SIZE = 4096;
    char buf[BUF_SIZE + 1];
    ssize_t received = 0;
    while(received < date_len) {
        ssize_t n = recv(sockfd, buf + received, date_len - received, 0);
        if(n < 0) break;
        else if(n == 0) {
            std::cerr << "server closed connection\n";
            close(sockfd);
            return 1;
        }
        received += n;
    }
    if(received < date_len) {
        std::cerr << "recv fail: error = " << strerror(errno) << '\n';
        close(sockfd);
        return 1;
    }
    buf[received] = '\0';
    std::cout << "recv success!\n"
              << buf << '\n';

    close(sockfd);
    return 0;
}