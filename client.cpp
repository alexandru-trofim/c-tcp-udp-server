#include "utils.hpp"




int main(int argc, char *argv[]) {

    sockaddr_in serv_addr;
    socklen_t   socket_len = sizeof(struct sockaddr_in);
    char*       id = argv[1];
    int         server_socket;
    uint16_t    port;

    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        die(__LINE__, "%s: Error! Unable to create socket",argv[0]);
    }
    if (sscanf(argv[3], "%hu", &port) != 1) {
        die(__LINE__, "%s: Error! Given port is invalid",argv[0]);
    } 

    //set server address
    memset(&serv_addr, 0, socket_len);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, argv[2], &serv_addr.sin_addr.s_addr) <= 0) {
        die(__LINE__, "%s: Error! Given ip is invalid",argv[0]);
    }
    //connect to the server 
    if (connect(server_socket, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        die(__LINE__, "%s: Error connecting to server",argv[0]);
    }

    Message send_id;
    memset(&send_id, 0, sizeof(send_id));
    strncpy(send_id.id,id, sizeof(id));
    send(server_socket, &send_id, sizeof(send_id), 0);

    /*DO NOT FORGET TO CLOSE ITSELF WHEN RECEIVING TYPE 1*/
    while(true) {
    }
    // Inchidem conexiunea si socketul creat
    // close(server_socket);

}