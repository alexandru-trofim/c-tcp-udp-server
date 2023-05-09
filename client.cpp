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

    /*Send id to server*/
    Message send_id;
    memset(&send_id, 0, sizeof(send_id));
    strncpy(send_id.id, id, sizeof(id));
    send(server_socket, &send_id, sizeof(send_id), 0);

    poll_fds.push_back({server_socket, POLLIN, 0});
    poll_fds.push_back({STDIN_FILENO, POLLIN, 0});

    while(true) {
        if (poll(poll_fds.data(),poll_fds.size(), -1) < 0) {
            die(__LINE__, "Poll error");
        }
        for (auto &fd : poll_fds) {
            if (fd.revents & POLLIN) {
                if (fd.fd == server_socket) {
                    int res = process_packet_from_server(server_socket);

                    if (res == -1) {
                        return 0;
                    }
                    
                } else if (fd.fd == STDIN_FILENO) {

                    Message packet;
                    string input;

                    getline(cin, input);
                    memset(&packet, 0, sizeof(packet));

                    if (input.rfind("subscribe", 0) == 0) { 

                        subscribe_client(server_socket, input);

                    } else if (input.rfind("unsubscribe", 0) == 0) {

                        unsubscribe_client(server_socket, input);
                        
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