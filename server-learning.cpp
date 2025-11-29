#include <arpa/inet.h>  // htons, inet_addr, sockaddr_in
#include <netinet/in.h> // sockaddr_in
#include <sys/socket.h> // socket, bind, listen, accept, setsockopt
#include <unistd.h>     // close, read, write
#include <cstring>      // memset, strerror
#include <iostream>     // std::cout, std::cerr

int main() {
    /* 
       sockfd 是一个int型套接字，存储由 socket() 返回的文件描述符(fd),
       fd 类似于索引，通过它可以操作对应的内核 socket 结构体

       socket同时还会创建内核 socket 结构体，用来储存端口信息
       例如：
            192.0.0.1是IP地址，8080是端口号（Port），192.0.0.1:8080才是完整的端口
            网络概念         类比解释                   	
            IP 地址	                 房子地址   → 哪个房子（哪台主机）
            端口号(Port)               房间号   → 房子里面的具体房间（服务）
            IP:Port	        房子地址 + 房间号   → 具体某个服务的“门牌”

       函数原型：
            int socket(int domain, int type, int protocol);
       使用样例：
            sockfd = socket(AF_INET, SOCK_STREAM, 0);
       参数说明：
            AF_INET     : IPv4的地址，表示Internet Protocol version 4，网络协议4
            SOCK_STREAM  : 套接字类型，表示 TCP 流式套接字
            protocol    : 协议编号，一般传 0，由内核根据 type 自动选择 TCP 协 (IPPROTO_TCP)
            AF = Address Family（地址族/地址类型）

       TCP  : Transmission Control Protocol, 传输控制协议
            TCP 是面向连接的、可靠的流式传输协议，数据以 连续字节流 形式发送
            所以叫SOCK_STEAM
    */
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0){
        std::cerr << "create socket error: errmsy = " << strerror(errno) << std::endl;
        // return 1 → 程序异常退出
        return 1;
    }
    std::cout << "create socket success!\n";

    int opt = 1;
    /* 
       SO_REUSEADDR :
            Socket Option（套接字选项）
            Reuse Address（重用地址）

       当你关闭 TCP 服务器后，端口不会立刻释放，而是进入 TIME_WAIT 状态
       如果你想 立即重启服务器绑定同一个端口，就需要 SO_REUSEADDR

       内核允许你在 TIME_WAIT 状态下再次绑定端口，但不会覆盖正在使用的连接

       函数原型：
            int setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen);
       （帮助 bind 成功）
    */
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // addr 是程序中申请的用户空间结构体，不是内核中的 socket 结构体
    // 它用来存储 IP 地址、端口等信息，通过 bind/connect 系统调用
    // 把这些信息 传递 给内核 socket 结构体，从而初始化内核中的 socket
    std::string ip = "127.0.0.1";
    int port = 8080;
    sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    // addr.sin_addr.s_addr = INADDR_ANY;
    // addr.sin_addr.s_addr = inet_addr(ip.c_str());
    /*
     全称英文：Host TO Network Long/Short
     意思：把 主机字节序 (Host) 的 32 位长整数 (Long) 转换为 网络字节序 (Network)
     0x7F000001   32位
     8080         16位
    */
    addr.sin_port = htons(port);
    if(::inet_pton(AF_INET, ip.c_str(), &addr.sin_addr) <= 0) {
         std::cerr << "inet_pton() failed: error = " << strerror(errno) << '\n';
         close(sockfd);
         return 1;
    }
    //     inet n to p
    //        Internet presentation(可读形式（字符串表示）) to network(网络字节序（大端）)
    /*
       sockaddr_in:
            in 就是 Internet（IPv4）
            sin_family → socket internet family（地址族）（告诉 操作系统“接下来的 socket 地址是用哪种协议的”）
            sin_port → socket internet port（端口号）
            sin_addr → socket internet address（IP 地址）(Internet Protocol 网络协议)

       inet.addr(const char *)
            把字符串形式的 IPv4 地址转成网络字节序（大端）整数
            c_str() → std::string 转成 const char* 指针

       uint16_t htons(uint16_t hostshort);
            int → 网络字节序（大端）16 位整数
            （主机字节序 = 小端）
    */

    if(::bind(sockfd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) < 0) {
        std::cerr << "socket bind error: errmsy =" << strerror(errno) << std::endl;
        close(sockfd);
        return 1;
    }
    /*
       bind 函数原型：
            int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);

       reinterpret_cast<类型> → 强制类型转换
            reinterpret	 重新解释、重新理解	
                   cast	 类型转换、投射
    */
    printf("socket bind success: ip = %s port = %d\n", ip.c_str(), port);

    // 函数原型：int listen(int sockfd, int backlog);
    // backlog：内核在未被 accept 的等待队列中允许的最大连接数
    if(::listen(sockfd, 1024) < 0) {
        std::cerr << "socket listen error: errmsy = " << strerror(errno) << std::endl;
        close(sockfd);
        return 1;
    }
    std::cout << "socket is listening...\n";

    while(true) {
        /*
            ∙ accept 函数是系统调用 → 内核实现

            ∙ 内核在 内部 有  半连接队列  ，里面存储了客户端发来的 SYN 包和地址信息

            ∙ 当调用 accept 时，内核会：
            ∙ 从半连接队列中取一个客户端连接
            ∙ 将客户端的 IP 和端口写入你提供的 client_addr 缓冲区
            ∙ 因为传给 accept 的是 &client_addr（地址），内核直接写入栈空间
            ∙ 所以你上的变量就“收到了”外部客户端的信息

            如果没有就会一直阻塞
        */
        // 所以不用初始化 
        sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int conn_fd = accept(sockfd, reinterpret_cast<sockaddr *>(&client_addr), &client_len);
        // int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
        // 返回值：成功返回新的连接的 socket fd
        
        if(conn_fd < 0) {
            std::cerr << "socket accept error: errmsy = " << strerror(errno) << std::endl;
            continue; // 出错但继续接受下一个连接
        }
        
        // IPv4 地址字符串长度的宏定义
        char client_ip[INET_ADDRSTRLEN]; // inet_addrstrlen（i net _ addr str len）
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));
        /*
           inet n to p
                Internet network(网络字节序（大端）) to presentation(可读形式（字符串表示）)

           const char *inet_ntop(int af, const void *src, char *dst, socklen_t size);
                参数	    类型	            含义
                af	        int	        地址族，AF_INET（IPv4）或 AF_INET6（IPv6）
                src	    const void*	    指向网络字节序地址的指针（如 &client_addr.sin_addr）
                dst	        char*	    存放可读字符串的缓冲区
                size	socklen_t	    缓冲区大小（推荐用 INET_ADDRSTRLEN）（inet_addrstrlen)

                src:根据结构体大小和类型来解析字节，而不是直接解析整数
                void * = 通用指针类型
        */
        std::cout << "Accepted connection from " << client_ip << ":" << ntohs(client_addr.sin_port) << std::endl;
        // ntohs 的作用是 把网络字节序（大端）的 16 位整数转换成主机字节序

        const int BUF_SIZE = 4096;
        char buffer[BUF_SIZE + 1];
        while(true) {
            // ssize_t read(int fd, void *buf, size_t count);
            // ssize_t 有符号整数类型 signed
            // size_t 有符号整数类型
            ssize_t n = recv(conn_fd, buffer, BUF_SIZE, 0);
            if (n < 0) {
                std::cerr << "read failed: " << strerror(errno) << std::endl;
                break; // 停止接收
            }
            else if (n == 0) {
                std::cout << "Client disconnected\n";
                break;
            }

            /*
               TCP 服务器有两种 socket
               (1) 监听 socket
               由 listen_fd = socket(...) 创建
               由服务器专用，用于：

                    监听端口
                    等客户端来连接

                ❌ 不进行数据收发
                ❌ 不能用来 read/write

                (2) 会话 socket
                由 accept() 返回
                每来一个客户端，就新建一个 socket
                    ✔ 可以收发数据
                    ✔ 一个客户端对应一个独立的 socket
                    ✔ 多个客户端就创建多个 socket


               为什么我们调用 read(conn_fd) 和 write(conn_fd)，明明没有传入 client 的 socket，却能读写客户端发来的数据？
                    因为 conn_fd（会话 socket）内部已经在内核中绑定了客户端的 IP、端口、协议状态、TCP 缓冲区等信息，read/write 根据这个 fd 操作的是内核 socket，而不是你传入的参数。

                    read/write 操作的是由 accept 返回的文件描述符对应的内核 socket，内核维护了 TCP 状态和缓冲区，用户态只是访问这个缓冲区


               既然 conn_socket 已经和客户端连上了，为什么还要调用 read/write？
               为什么我还要主动去读？数据不是已经在主机里面了吗？
                    TCP 连接建立后，数据会进入内核的“socket 接收缓冲区”
                    这个缓冲区不是应用程序的，是内核的
                    conn_socket 只是访问这个缓冲区的“钥匙”（fd）
                    数据只有在 read 的时候才会从内核拷贝到的 buffer（用户空间）
            */
            // read/write recv/send 不保证把数据一次性全部读/写完（最好都用循环）
            ssize_t sent = 0;
            while(sent < n) {
                ssize_t m = send(conn_fd, buffer + sent, n - sent, 0);
                if (m < 0) {
                    std::cerr << "write failed: " << strerror(errno) << '\n';
                    break;
                }
                sent += m;
            }
            if (sent < n) break;
        }

        close(conn_fd);
    }

    close(sockfd);
    return 0;
}
/*
& "E:\vscode\network\Cygwin\bin\g++.exe" .\server-learning.cpp -std=c++17 -pthread

netstat -ano|findstr :8080
*/