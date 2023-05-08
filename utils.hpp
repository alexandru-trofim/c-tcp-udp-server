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

using namespace std;

typedef struct Message {
    char msg[1500];
    char topic[50];
    char id[10];
    uint32_t ip_udp_sender;
    uint16_t len_msg;
    uint16_t port_udp_sender;
    uint8_t type_of_command;
    uint8_t len_topic;

} Message;

typedef struct Client {
    pollfd file_descriptor;
    string ID;
    vector<Message> messages_to_send;    
    int status;
    map<string, int> followed_topics;

} Client;





void die (int line_number, const char * format, ...);