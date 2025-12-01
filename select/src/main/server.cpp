#include"server/serversocket.h"

int main() {
    ServerSocket server("127.0.0.1", 8080);

    int sock_fd = server.get_fd();

    fd_set recv_fds;
    FD_ZERO(&recv_fds);
    FD_SET(sock_fd, &recv_fds);
    int max_fd = sock_fd;

    while(true) {
        std::cout << "最大套接字：" << max_fd << std::endl;

        timeval timeout;
        timeout.tv_sec = 10;
        timeout.tv_usec = 0;

        fd_set tmp_fds = recv_fds;
        int in_fds = select(max_fd + 1, &tmp_fds, nullptr, nullptr, &timeout);

        if(in_fds < 0) {
            std::cerr << "select failed: " << strerror(errno) << '\n';
            break;
        }
        else if(in_fds == 0) {
            continue;
        }
        else{
            for (int event_fd = max_fd; event_fd >= 0; event_fd--) {
                std::cout << "当前套接字号为：" << event_fd << '\n';
                if(FD_ISSET(event_fd, &tmp_fds) == 0) continue;

                if(max_fd == 0 && FD_ISSET(event_fd, &recv_fds)) {
                    max_fd = event_fd;
                    std::cout << "最大套接字被更新为：" << event_fd << '\n';
                }

                if(event_fd == sock_fd) {
                    int conn_fd = server.accept();
                    FD_SET(conn_fd, &recv_fds);
                    max_fd = std::max(max_fd, conn_fd);
                }
                else {
                    std::cout << "当前事件为：读写\n";
                    std::vector<char> recv_buffer = server.recv(event_fd, 8192);
                    if(recv_buffer.empty()) {
                        close(event_fd);
                        FD_CLR(event_fd, &recv_fds);
                        if(event_fd == max_fd) {
                            std::cout << "最大套接字被更新\n";
                            max_fd = 0;
                        }
                        continue;
                    }
                    std::string recv_msg(recv_buffer.begin(), recv_buffer.end());
                    std::cout << recv_msg << '\n';

                    std::string send_msg = "hello client\n";
                    std::vector<char> send_buffer(send_msg.begin(), send_msg.end());
                    server.send(event_fd, send_buffer);
                }
            }
        }
        // std::cout << "最大套接字：" << max_fd << std::endl;
    }
    
    return 0;
}