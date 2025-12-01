#include "client/clientsocket.h"

int main() {
    ClientSocket client("127.0.0.1", 8080);

    for(int i=0;i<3;i++){
        sleep(3);
        std::string msg = "hello server";
        std::vector<char> send_buffer(msg.begin(), msg.end());
        client.send(send_buffer);

        std::vector<char> recv_buffer = client.recv(8192);
        std::string recv_msg(recv_buffer.begin(), recv_buffer.end());
        std::cout << recv_msg << '\n';
    }
    return 0;
}