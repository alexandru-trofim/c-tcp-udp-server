#include <iostream> 
#include <map>
#include <vector>
#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <cstdarg>
#include <string>
#include <cmath>
#include <netinet/tcp.h>

using namespace std;

typedef struct Message {
    char msg[1500];
    char topic[50];
    char id[10];
    uint32_t ip_udp_sender;
    uint16_t port_udp_sender;
    uint8_t type_of_command;
    uint8_t sf;
    uint8_t data_type;

} Message;

typedef struct User{
    char id[10];
    pollfd file_descriptor;
    vector<Message> messages_to_send;    
    map<string, int> followed_topics;
    int status;

} User;

typedef struct ServerInfo {
    uint16_t port;
    int tcp_socket;
    int udp_socket;
    struct sockaddr_in addr;
    vector<pollfd> poll_fds;
    map<string, User> users;
} ServerInfo;



/*Server's utils*/
void die (int line_number, const char * format, ...);
void setup_server(ServerInfo* server);
User* find_user_by_id(ServerInfo* server, char*id);
void handle_new_conn(ServerInfo* server);
void exec_subscribe_action(ServerInfo* server, Message packet, int fd);
void exec_unsubscribe_action(ServerInfo* server, Message packet, int fd);
void exec_close_client(ServerInfo* server, int fd);
void recv_udp_send_clients (ServerInfo* server);
void send_sf_messages (User* user);

/*Client's utils*/
void print_received_packet (Message packet);





