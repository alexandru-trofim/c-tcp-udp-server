#include "utils.hpp"

using namespace std;

int main (int argc, char* argv[]) {
    // argv[1] - portul doriot
    //deschidem doi socketi unul pentru udp si unul pentru tcp pe portul primit ca param
    //portul asteapta conexiuni datagrame de pe orice ip local

    unsigned short port;
    int tcp_socket, udp_socket;
    struct sockaddr_in addr;
    vector<pollfd> poll_file_descriptors;

    // we deactivate the buffering for low latency
    setvbuf(stdout, NULL, _IONBF, BUFSIZ);

    if ((tcp_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        die(__LINE__, "%s: Error while creating the tcp_socket",argv[0]);
    }

    if ((udp_socket = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        die(__LINE__, "%s: Error while creating the upd_socket",argv[0]);
    }

    if ((port = atoi(argv[1])) == 0) {
        die(__LINE__, "%s: Cannot convert cmd argument \"%s\" to port number", argv[0],argv[1]);
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    if(bind(tcp_socket, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        die(__LINE__, "%s: Cannot bind tcp_socket",argv[0]);
    }

    if(bind(udp_socket, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        die(__LINE__, "%s: Cannot bind udp_socket",argv[0]);
    }

    if (listen(tcp_socket, SOMAXCONN) < 0) {
        die(__LINE__, "%s: Error listen tcp_socket",argv[0]);
    }

    cout << port <<" tcp: "<< tcp_socket << " udp: "<< udp_socket <<  endl;

    //add tcp udp sockets to poll file descriptors
    poll_file_descriptors.push_back({ tcp_socket, POLLIN, 0 }); // TCP socket
    poll_file_descriptors.push_back({ udp_socket, POLLIN, 0 }); // UDP socket

    while (true) {

        if (poll(poll_file_descriptors.data(),poll_file_descriptors.size(), -1) < 0) {
            die(__LINE__, "Poll error");
        }

        for (auto &fd : poll_file_descriptors) {
            if (fd.revents & POLLIN) {
                if (fd.fd == tcp_socket) {

                    struct sockaddr_in client_addr;
                    socklen_t client_addrlen = sizeof(client_addr);

                    //if fills client_addr
                    int client_socket = accept(tcp_socket, (struct sockaddr *)&client_addr, &client_addrlen);

                    Message packet;
                    int nbytes = recvfrom(client_socket, &packet, sizeof(packet), 0, NULL, NULL);
                    printf("%s\n", packet.id);


                    // aici ai acceptat o conexiune TCP
                    poll_file_descriptors.push_back({ client_socket, POLLIN, 0 });









                } else if (fd.fd == udp_socket) {
                    char buffer[1024];
                    struct sockaddr_in sender_addr;
                    socklen_t sender_addrlen = sizeof(sender_addr);
                    int nbytes = recvfrom(udp_socket, buffer, sizeof(buffer), 0, (struct sockaddr *)&sender_addr, &sender_addrlen);
                    // aici ai primit un datagram UDP
                    cerr << buffer << '\n';
                } else {

                }
            } 
        }


    }

    close(tcp_socket);
    // close(udp_socket);

}