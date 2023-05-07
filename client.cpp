#include "utils.hpp"




int main(int argc, char *argv[]) {


    char* id = argv[1];
    return 0;
    int sockfd;
    uint16_t port;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        die(__LINE__, "%s: Error! Unable to create socket",argv[0]);
    }

    struct sockaddr_in serv_addr;
    socklen_t socket_len = sizeof(struct sockaddr_in);

    if (sscanf(argv[2], "%hu", &port) != 1) {
        die(__LINE__, "%s: Error! Given port is invalid",argv[0]);
    } 


    //set server address
    memset(&serv_addr, 0, socket_len);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, argv[1], &serv_addr.sin_addr.s_addr) <= 0) {
        die(__LINE__, "%s: Error! Given ip is invalid",argv[0]);
    }

    //connect to the server 
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        
    }

    run_client(sockfd);

    // Inchidem conexiunea si socketul creat
    close(sockfd);

}