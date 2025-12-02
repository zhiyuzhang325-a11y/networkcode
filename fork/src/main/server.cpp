#include"server/serversocket.h"
#include <sys/types.h>
#include <sys/wait.h>
#include<signal.h>
#include <unistd.h>

void FathEXIT(int sig) {
    std::cout << "父进程退出， sig = " << sig << '\n';
    // kill(0, SIGTERM);
    while(waitpid(-1, nullptr, WNOHANG) > 0);
    _exit(0); // 在信号处理函数中，使用 _exit() 比 exit() 更安全
}
void ChildEXIT(int sig) {
    std::cout << "子进程退出， sig = " << sig << '\n';
    _exit(0);
}
void handler(int sig) {
    waitpid(-1, nullptr, WNOHANG);
}

int main() {
    ServerSocket server("127.0.0.1", 8080);

    signal(SIGTERM, FathEXIT);  // SIGTERM 15   kill
    signal(SIGINT, FathEXIT);   // SIGINT 2     Ctrl + c
    signal(SIGCHLD, handler);


    while(true) {
        int conn_fd = server.accept();
        if(conn_fd < 0) continue;

        /*
            监听 socket 是少数而轻量的，不需要 N 个进程监听
            accept 的任务非常轻：只接受连接，不做逻辑
            主要的 CPU 资源在 业务数据读写（conn_fd），不是 accept
            所以把 accept 集中到一个进程做、其他进程专心处理数据，是最合理的分工。
            
            如果多个子进程同时 accept，会冲突
            Linux 对 accept 的逻辑：
            同一个监听 fd，被多个进程 accept，会产生激烈锁竞争（惊群现象）。
            以前 Linux 2.6 非常严重，会把所有子进程全部唤醒。
            例如 100 个子进程同时监听：
            有 1 个新连接
            100 进程全部被唤醒 → 抢锁 → 99 个失败 → 资源浪费
        */

        int pid = fork();
        if(pid < 0) {
            std::cerr << "fork faild: " << strerror(errno) << '\n';
            return -1;
        }
        else if(pid == 0) {
            server.closeListen();

            signal(SIGTERM, ChildEXIT);
            signal(SIGINT, SIG_IGN);

            while (true) {
                std::vector<char> recv_buffer = server.recv(8192);
                if(recv_buffer.empty()) {
                    std::cout << "client disconnected\n";
                    break;
                }
                std::string recv_msg(recv_buffer.begin(), recv_buffer.end());
                std::cout << recv_msg << '\n';

                std::string send_msg = "hello client\n";
                std::vector<char> send_buffer(send_msg.begin(), send_msg.end());
                server.send(send_buffer);
            }
            return 0;
        }
        else if(pid > 0) {
            server.closeConnect();
            continue;
        }
    }

    return 0;
}
