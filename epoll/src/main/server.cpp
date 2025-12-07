#include"server/serversocket.h"
#include<signal.h>
#include<sys/epoll.h>

void Exit(int sig) {
    _exit(0);
}

int main() {
    ServerSocket server("127.0.0.1", 8080);
    signal(SIGINT, Exit);

    int sock_fd = server.get_fd();

    int epoll_fd=epoll_create1(0);

    epoll_event ev;
    ev.data.fd=sock_fd;
    ev.events=EPOLLIN;

    epoll_ctl(epoll_fd,EPOLL_CTL_ADD,sock_fd,&ev);

    epoll_event evs[10];

    while(true) {

        int in_fds=epoll_wait(epoll_fd,evs,10,-1);

        if(in_fds < 0) {
            perror("epoll_wait failed");
            break;
        }
        else if(in_fds == 0) {
            continue;
        }
        else{
            std::cout<<"事件数："<<in_fds<<'\n';
            for (int i = 0; i < in_fds; i++) {
                int fd = evs[i].data.fd;
                std::cout<<"文件描述符："<<fd<<'\n';
                if(fd == sock_fd) {
                    int conn_fd=server.accept();
                    
                    ev.data.fd = conn_fd;
                    ev.events = EPOLLIN;
                    epoll_ctl(epoll_fd,EPOLL_CTL_ADD,conn_fd,&ev);
                }
                else {
                    std::vector<char> recv_buffer = server.recv(fd, 8192);
                    if(recv_buffer.empty()) {
                        epoll_ctl(epoll_fd,EPOLL_CTL_DEL,fd,nullptr);
                        close(fd);
                        continue;
                    }
                    std::string recv_msg(recv_buffer.begin(), recv_buffer.end());
                    std::cout << recv_msg << '\n';

                    std::string send_msg = "hello client\n";
                    std::vector<char> send_buffer(send_msg.begin(), send_msg.end());
                    server.send(fd, send_buffer);
                }
            }
        }
    }
    
    return 0;
}