#include "utils.hpp"

void die (int line_number, const char * format, ...) {
    va_list vargs;
    va_start (vargs, format);
    fprintf (stderr, "%d: ", line_number);
    vfprintf (stderr, format, vargs);
    fprintf (stderr, ".\n");
    va_end (vargs);
    exit (1);
}

void setup_server(ServerInfo* server) {

    //create sockets 
    if ((server->tcp_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        die(__LINE__, " Error while creating the tcp_socket");
    }
    if ((server->udp_socket = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        die(__LINE__, "Error while creating the upd_socket");
    }
    /*Deactivate Nagle's alogrithm*/
    int flag = 1;
    int result = setsockopt(server->tcp_socket, IPPROTO_TCP, TCP_NODELAY, (char *)&flag, sizeof(int));

    //make them reusable
    int enable = 1;
    if (setsockopt(server->tcp_socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
        die(__LINE__, "Error while creating the upd_socket");
    }
    if (setsockopt(server->udp_socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
        die(__LINE__, "Error while creating the upd_socket");
    }

    //set server address
    memset(&server->addr, 0, sizeof(server->addr));
    server->addr.sin_family = AF_INET;
    server->addr.sin_port = htons(server->port);
    server->addr.sin_addr.s_addr = INADDR_ANY;

    //bind sockets
    if(bind(server->tcp_socket, (sockaddr*)&server->addr, sizeof(server->addr)) < 0) {
        die(__LINE__, "Cannot bind tcp_socket");
    }
    if(bind(server->udp_socket, (sockaddr*)&server->addr, sizeof(server->addr)) < 0) {
        die(__LINE__, "Cannot bind udp_socket");
    }

    if (listen(server->tcp_socket, SOMAXCONN) < 0) {
        die(__LINE__, "Error listen tcp_socket");
    }

    //add tcp udp stdin sockets to poll file descriptors
    server->poll_fds.push_back({ server->tcp_socket, POLLIN, 0 }); // TCP socket
    server->poll_fds.push_back({ server->udp_socket, POLLIN, 0 }); // UDP socket
    server->poll_fds.push_back({ STDIN_FILENO, POLLIN, 0 }); // Standard input
}

void handle_new_conn(ServerInfo* server) {

    Message      packet;
    sockaddr_in  client_addr;
    socklen_t    client_addrlen = sizeof(client_addr);

    /*accept new conn and get the user's id*/
    int client_socket = accept(server->tcp_socket, (sockaddr *)&client_addr, &client_addrlen);
    int nbytes = recvfrom(client_socket, &packet, sizeof(packet), 0, NULL, NULL);

    /*Check if user already exists and/or is active*/
    if (server->users.find(packet.id) != server->users.end()) {
        User existing_user = server->users[packet.id];

        if (existing_user.status == 0) {
            /*User exists but not active; Update user*/
            server->users[packet.id].file_descriptor = { client_socket, POLLIN, 0 };
            server->users[packet.id].status = 1;

            cout << "New client " << existing_user.id 
                << " connected from "  
                << inet_ntoa(client_addr.sin_addr)
                << ":" << client_addr.sin_port << ".\n";

            send_sf_messages(&server->users[packet.id]);

            server->poll_fds.push_back({ client_socket, POLLIN, 0 });
        } else {
            /*Telling illegal client to destroy itself*/
            Message destroy_message;
            memset(&destroy_message, 0, sizeof(destroy_message));

            destroy_message.type_of_command = 1;
            send(client_socket,&destroy_message, sizeof(destroy_message), 0);
            close(client_socket);

            cout << "Client " << existing_user.id << " already connected.";
        }
    } else {
        /*New user have to be created*/
        User new_user;
        new_user.file_descriptor = { client_socket, POLLIN, 0 };
        strcpy(new_user.id, packet.id);
        new_user.status = 1;

        /*add user to database*/
        server->users[new_user.id] = new_user;

        cout << "New client " << new_user.id 
            << " connected from "  
            << inet_ntoa(client_addr.sin_addr)
            << ":" << client_addr.sin_port << ".\n";

        server->poll_fds.push_back({ client_socket, POLLIN, 0 });
    }
}

void exec_subscribe_action(ServerInfo* server, Message packet, int fd) {
    for (auto it = server->users.begin(); it != server->users.end(); ++it) {
        if (it->second.file_descriptor.fd == fd) {
            /*found the user we want to update subscription*/
            string topic(packet.topic);
            it->second.followed_topics.insert({topic, packet.sf});

            break;
        }
    }
}

void exec_unsubscribe_action(ServerInfo* server, Message packet, int fd) {
    for (auto it = server->users.begin(); it != server->users.end(); ++it) {
        User user = it->second;
        if (user.file_descriptor.fd == fd) {
            /*found the user we want to update subscription*/
            string topic(packet.topic);
            user.followed_topics.erase(topic);

            break;
        }
    }
}


void recv_udp_send_clients (ServerInfo* server) {
    char buffer[1551];
    char topic[50];
    Message packet;
    uint8_t type;
    char msg[1500];

    struct sockaddr_in sender_addr;
    socklen_t sender_addrlen = sizeof(sender_addr);
    int nbytes = recvfrom(server->udp_socket, buffer, 
                          sizeof(buffer),
                          0,
                          (sockaddr *)&sender_addr,
                          &sender_addrlen);

    /*Setup packet we have to send*/
    memset(&packet, 0, sizeof(packet));
    strncpy(packet.topic, buffer, 50);
    memcpy(packet.msg, buffer + 51, nbytes - 51);
    packet.data_type = *(buffer + 50);
    packet.ip_udp_sender = sender_addr.sin_addr.s_addr;
    packet.port_udp_sender = sender_addr.sin_port;
    packet.type_of_command = 2;

    string string_topic(packet.topic);

    for (auto it = server->users.begin(); it != server->users.end(); ++it) {
        User user = it->second;
        if (user.followed_topics.find(string_topic) != user.followed_topics.end()){
            if (user.status == 1 ) {
                send(user.file_descriptor.fd, &packet, sizeof(packet), 0);
                //user active
            } else if (user.status == 0 && user.followed_topics[string_topic] == 1) {
                //store and forward
                it->second.messages_to_send.push_back(packet);
            }
        }
    }

}

void send_sf_messages (User* user) {
    for (int i = 0; i < user->messages_to_send.size(); ++i) { 
        Message packet = user->messages_to_send[i];
        send(user->file_descriptor.fd, &packet, sizeof(packet), 0);
    }

}