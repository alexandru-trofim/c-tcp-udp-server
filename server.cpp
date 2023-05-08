#include "utils.hpp"

using namespace std;

int main (int argc, char* argv[]) {

    //allocate server on heap
    ServerInfo server;

    // we deactivate the buffering for low latency
    setvbuf(stdout, NULL, _IONBF, BUFSIZ);
    //do not forget to deactivate naggle algo

    //get port from arguments
    if ((server.port = atoi(argv[1])) == 0) {
        die(__LINE__, "%s: Cannot convert cmd argument \"%s\" to port number", argv[0],argv[1]);
    }
    setup_server(&server);

    while (true) {

        if (poll(server.poll_fds.data(),server.poll_fds.size(), -1) < 0) {
            die(__LINE__, "Poll error");
        }

        for (auto &fd : server.poll_fds) {
            if (fd.revents & POLLIN) {
                if (fd.fd == server.tcp_socket) {

                    handle_new_conn(&server);
                    // socklen_t client_addrlen = sizeof(client_addr);

                    // //if fills client_addr
                    // int client_socket = accept(server.tcp_socket, (struct sockaddr *)&client_addr, &client_addrlen);

                    // Message packet;
                    // int nbytes = recvfrom(client_socket, &packet, sizeof(packet), 0, NULL, NULL);

                    // // we have to check if the client with this id already exists
                    // // if yes then we update its file descriptor and its status
                    // if (server.users.find(packet.id) != server.users.end()) {
                    //     //user already exists
                    //     //check if user is active
                    //     //if yes close client_socket
                    //     //if no update the user socket
                    //     User existing_user = server.users[packet.id];
                    //     if (existing_user.status == 0) {
                    //         //user is not active we should make him active update pollfd and add 
                    //         // to pollfd vector
                    //         existing_user.file_descriptor = { client_socket, POLLIN, 0 };
                    //         existing_user.status = 1;
                    //         server.poll_fds.push_back({ client_socket, POLLIN, 0 });

                    //         cout << "New client " << existing_user.id 
                    //             << " connected from "  
                    //             << inet_ntoa(client_addr.sin_addr)
                    //             << ":" << client_addr.sin_port << ".\n";
                    //     } else {
                    //         cout << "user exists and active\n";
                    //     }
                        
                    // } else {
                    //     //user does not exist
                    //     //create new user
                    //     User new_user;
                    //     new_user.file_descriptor = { client_socket, POLLIN, 0 };
                    //     strcpy(new_user.id, packet.id);
                    //     new_user.status = 1;

                    //     //add user to database
                    //     server.users[new_user.id] = new_user;
                    //     cout << "New client " << new_user.id 
                    //         << " connected from "  
                    //         << inet_ntoa(client_addr.sin_addr)
                    //         << ":" << client_addr.sin_port << ".\n";

                    //     server.poll_fds.push_back({ client_socket, POLLIN, 0 });
                    // }

                    // if (user == NULL) {
                    //     cerr << "Trosha: this is a new client";
                    //     //initialize new user 
                    //     User new_user;
                    //     new_user.file_descriptor = {client_socket, POLLIN, 0};
                    //     strcpy(new_user.id, packet.id);
                    //     new_user.status = 1;
                    //     cout << "New client " << new_user.id 
                    //         << " connected from "  
                    //         << inet_ntoa(client_addr.sin_addr)
                    //         << ":" << client_addr.sin_port << ".\n";
                    // } else {
                    //     cout << "user already exists\n";
                    // }

                    // aici ai acceptat o conexiune TCP


                } else if (fd.fd == server.udp_socket) {
                    char buffer[1024];
                    struct sockaddr_in sender_addr;
                    socklen_t sender_addrlen = sizeof(sender_addr);
                    int nbytes = recvfrom(server.udp_socket, buffer, sizeof(buffer), 0, (struct sockaddr *)&sender_addr, &sender_addrlen);
                    // aici ai primit un datagram UDP
                    cerr << buffer << '\n';

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
                }
            } 
        }


    }


}