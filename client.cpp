#include "utils.hpp"


int main(int argc, char *argv[]) {

    sockaddr_in     serv_addr;
    socklen_t       socket_len = sizeof(struct sockaddr_in);
    char*           id = argv[1];
    int             server_socket;
    uint16_t        port;
    vector<pollfd>  poll_fds;

    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        die(__LINE__, "%s: Error! Unable to create socket",argv[0]);
    }
    if (sscanf(argv[3], "%hu", &port) != 1) {
        die(__LINE__, "%s: Error! Given port is invalid",argv[0]);
    } 

    /*Deactivate Nagle's alogrithm*/
    int flag = 1;
    int result = setsockopt(server_socket, IPPROTO_TCP, TCP_NODELAY, (char *)&flag, sizeof(int));
    //
    setbuf(stdout, NULL);

    //set server address
    memset(&serv_addr, 0, socket_len);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, argv[2], &serv_addr.sin_addr.s_addr) <= 0) {
        die(__LINE__, "%s: Error! Given ip is invalid",argv[0]);
    }
    if (connect(server_socket, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        die(__LINE__, "%s: Error connecting to server",argv[0]);
    }

    Message send_id;
    memset(&send_id, 0, sizeof(send_id));
    strncpy(send_id.id, id, sizeof(id));
    send(server_socket, &send_id, sizeof(send_id), 0);

    poll_fds.push_back({server_socket, POLLIN, 0});
    poll_fds.push_back({STDIN_FILENO, POLLIN, 0});

    /*DO NOT FORGET TO CLOSE ITSELF WHEN RECEIVING TYPE 1*/
    while(true) {
        if (poll(poll_fds.data(),poll_fds.size(), -1) < 0) {
            die(__LINE__, "Poll error");
        }
        for (auto &fd : poll_fds) {
            if (fd.revents & POLLIN) {
                if (fd.fd == server_socket) {
                    Message packet;
                    int nbytes = recv(server_socket, &packet, sizeof(packet), 0);
                    if (nbytes < sizeof(packet)) {
                        die(__LINE__, "%s: Error packet dropped",argv[0]);
                    }

                    if (packet.type_of_command == 1) {
                        close(server_socket);
                        return 0;
                    } else if (packet.type_of_command == 2) {
                        //print message
                        //handle packet
                        print_received_packet(packet);
                    }
                    
                } else if (fd.fd == STDIN_FILENO) {
                    Message packet;
                    string input;

                    getline(cin, input);
                    memset(&packet, 0, sizeof(packet));

                    if (input.rfind("subscribe", 0) == 0) { // pos=0 limits the search to the prefix
                        string topic = input.substr(10, input.size() - 10 - 2);
                        if (topic.size() > 50) {
                            die(__LINE__, "%s: Topic size is too big", argv[0]);
                        }
                        packet.type_of_command = 3;
                        strcpy(packet.topic, topic.c_str());

                        if (input[input.size() - 1] - '0' == 0)  {
                            packet.sf = 0;
                        } else if (input[input.size() - 1] - '0' == 1)  {
                            packet.sf = 1;
                        } else {
                            die(__LINE__, "%s: SF other than 1/0", argv[0]);
                        }

                        send(server_socket, &packet, sizeof(packet), 0);

                        cout << "Subscribed to topic.\n";

                    } else if (input.rfind("unsubscribe", 0) == 0) {
                        string topic = input.substr(12, input.size() - 12);
                        if (topic.size() > 10) {
                            die(__LINE__, "%s: Topic size is too big", argv[0]);
                        }
                        packet.type_of_command = 4;
                        strcpy(packet.topic, topic.c_str());

                        send(server_socket, &packet, sizeof(packet), 0);

                        cout << "Unsubscribed from topic.\n";

                    } else if (input.rfind("exit", 0) == 0) {
                        /*send exit packet*/
                        packet.type_of_command = 5;
                        send(server_socket, &packet, sizeof(packet), 0);
                        close(server_socket);
                        return 0;
                    } else {
                        die(__LINE__, "%s: Command other sub/unsub/exit", argv[0]);
                    }
                }
            } 
        }
    }

}
    // Inchidem conexiunea si socketul creat
    // close(server_socket);