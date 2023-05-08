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

    struct sockaddr_in client_addr;
    socklen_t client_addrlen = sizeof(client_addr);

    //if fills client_addr
    int client_socket = accept(server->tcp_socket, (struct sockaddr *)&client_addr, &client_addrlen);

    Message packet;
    int nbytes = recvfrom(client_socket, &packet, sizeof(packet), 0, NULL, NULL);

    // we have to check if the client with this id already exists
    // if yes then we update its file descriptor and its status
    if (server->users.find(packet.id) != server->users.end()) {
        User existing_user = server->users[packet.id];

        if (existing_user.status == 0) {
            /*User exists but not active; Update user*/
            existing_user.file_descriptor = { client_socket, POLLIN, 0 };
            existing_user.status = 1;

            cout << "New client " << existing_user.id 
                << " connected from "  
                << inet_ntoa(client_addr.sin_addr)
                << ":" << client_addr.sin_port << ".\n";

            server->poll_fds.push_back({ client_socket, POLLIN, 0 });
        } else {
            /*Telling illegal client to destroy itself*/
            Message destroy_message;
            memset(&destroy_message, 0, sizeof(destroy_message));

            destroy_message.type_of_command = 1;
            send(client_socket,&destroy_message, sizeof(destroy_message), 0);

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


