#include "utils.hpp"

using namespace std;

int main (int argc, char* argv[]) {

    //allocate server on heap
    ServerInfo server;

    // we deactivate the buffering for low latency
    setvbuf(stdout, NULL, _IONBF, BUFSIZ);
    //do not forget to deactivate naggle algo
    //
    setbuf(stdout, NULL);

    //get port from arguments
    if ((server.port = atoi(argv[1])) == 0) {
        die(__LINE__, "%s: Cannot convert cmd argument \"%s\" to port number", argv[0],argv[1]);
    }
    setup_server(&server);

    while (true) {

        if (poll(server.poll_fds.data(),server.poll_fds.size(), -1) < 0) {
            die(__LINE__, "Poll error");
        }

        for (int i = 0; i < server.poll_fds.size(); ++i) {
            pollfd fd = server.poll_fds[i];

            if (fd.revents & POLLIN) {
                if (fd.fd == server.tcp_socket) {
                    handle_new_conn(&server);

                } else if (fd.fd == server.udp_socket) {
                    recv_udp_send_clients (&server);
                    // char buffer[1024];
                    // struct sockaddr_in sender_addr;
                    // socklen_t sender_addrlen = sizeof(sender_addr);
                    // int nbytes = recvfrom(server.udp_socket, buffer, sizeof(buffer), 0, (struct sockaddr *)&sender_addr, &sender_addrlen);
                    // // aici ai primit un datagram UDP
                    // cerr << buffer << '\n';

                } else if (fd.fd == STDIN_FILENO){
                    string input;
                    getline(cin, input);
                    if (!input.compare("exit")) {
                        cerr << "Trosha: closing sockets" << "\n";
                        close(server.tcp_socket);
                        close(server.udp_socket);
                        //close all tcp clients
                        // free everything
                        return 0;
                    }
                } else {
                    /*other tcp's*/
                    Message packet;
                    recv(fd.fd, &packet, sizeof(packet), 0);
                    if (packet.type_of_command == 3) {
                        /*subscribe command*/
                        exec_subscribe_action(&server, packet, fd.fd);

                    } else if (packet.type_of_command == 4) {
                        exec_unsubscribe_action(&server, packet, fd.fd);
                        
                    } else if (packet.type_of_command == 5) {
                        /*exit command from client*/
                        //delete from pollfd
                        close(fd.fd);
                        //erase current fd entry
                        for (auto it = server.users.begin(); it != server.users.end(); it++)
                        {
                            if (it->second.file_descriptor.fd == fd.fd) {
                                it->second.status = 0;
                                cout << "Client " << it->second.id << " disconnected.\n";
                                break;
                            }
                        }
                        server.poll_fds.erase(server.poll_fds.begin() + i);
                        i--;
                    }
                }
            } 
        }


    }


}