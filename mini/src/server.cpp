#include<iostream>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<unistd.h>
#include<sys/socket.h>
#include<cstring>

int main() {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        std::cerr << "socket failed: " << strerror(errno) << '\n';
        return 1;
    }

    std::string ip = "127.0.0.1";
    int port = 8080;
    sockaddr_in sock_addr;
    std::memset(&sock_addr, 0, sizeof(sock_addr));
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_port = htons(port);
    if(inet_pton(AF_INET, ip.c_str(), &sock_addr.sin_addr) <= 0) {
        std::cerr << "inet_pton failed: " << strerror(errno) << '\n';
        close(sockfd);
        return 1;
    }

    if(bind(sockfd, reinterpret_cast<sockaddr*>(&sock_addr), sizeof(sock_addr)) < 0) {
        std::cerr << "bind failed: " << strerror(errno) << '\n';
        close(sockfd);
        return 1;
    }
    
    if(listen(sockfd, 1024) < 0) {
        std::cerr << "listen failed: " << strerror(errno) << '\n';
        close(sockfd);
        return 1;
    }
    std::cout << "server listening...\n";

    while(true) {
        sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int conn_fd = accept(sockfd, reinterpret_cast<sockaddr *>(&client_addr), &client_len);
        if(conn_fd < 0) {
            std::cerr << "connect failed: " << strerror(errno) << '\n';
            close(conn_fd);
            break;
        }
        std::cout << "connect success\n";

        char client_ip[INET_ADDRSTRLEN];
        int client_port = ntohs(client_addr.sin_port);
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));
        std::cout << "Accept connect " << client_ip << ":" << client_port << '\n';

        const int BUF_SIZE = 4096;
        char buf[BUF_SIZE + 1];
        ssize_t received = recv(conn_fd, buf, BUF_SIZE, 0);
        if(received < 0) {
            std::cerr << "recv failed: " << strerror(errno) << '\n';
            close(conn_fd);
            break;
        }
        else if(received == 0) {
            std::cerr << "client disconnect\n";
            close(conn_fd);
            break;
        }
        buf[received] = '\0';
        std::cout << "recv success!\n"
                  << buf << '\n';

        ssize_t sent = 0;
        while(sent < received) {
            ssize_t n = send(conn_fd, buf + sent, received - sent, 0);
            if(n < 0) break;
            sent += n;
        }
        if(sent < received) {
            std::cerr << "send failed: " << strerror(errno) << '\n';
            close(conn_fd);
            break;
        }

        close(conn_fd);
    }

    close(sockfd);
    return 0;
}