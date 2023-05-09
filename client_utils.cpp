#include "utils.hpp" 



void print_received_packet (Message packet) {

    char udp_ip[INET_ADDRSTRLEN + 1];
    auto res = inet_ntop(AF_INET,&packet.ip_udp_sender,udp_ip, sizeof(udp_ip));
    if (res == nullptr) {
        die(__LINE__, "%s: Cannot convert to IPv4");
    }
    
    if (packet.data_type == 0) {
        /*Int*/
        uint32_t number;
        memcpy(&number, packet.msg + 1, sizeof(number));

        int final_num;

        if (packet.msg[0] == 0) {
            final_num = (unsigned int)ntohl(number);
        } else {
            final_num = -(unsigned int)ntohl(number);
        }

        cout << udp_ip << ":" << ntohs(packet.port_udp_sender) 
             << " - " << packet.topic << " - "
             << "INT - " << final_num << "\n";


    } else if (packet.data_type == 1) {
        /*Short-real*/
        uint16_t number;
        memcpy(&number, packet.msg, sizeof(number));
        float float_num = (ntohs(number)) / 100.0;

        cout << udp_ip << ":" << ntohs(packet.port_udp_sender) 
             << " - " << packet.topic << " - "
             << "SHORT_REAL - " << float_num << "\n";

    } else if (packet.data_type == 2) {
        /*Float*/
        uint32_t number;
        memcpy(&number, packet.msg + 1, sizeof(number));

        float float_num = (float)ntohl(number) / pow(10, packet.msg[5]);

        if (packet.msg[0] == 1) { 
            float_num = - float_num;
        }

        cout << udp_ip << ":" << ntohs(packet.port_udp_sender) 
             << " - " << packet.topic << " - "
             << "FLOAT - " << float_num << "\n";

    } else if (packet.data_type == 3) {
        /*String*/
        cout << udp_ip << ":" << ntohs(packet.port_udp_sender) 
            << " - " << packet.topic << " - "
            << "STRING - " << packet.msg << "\n";

    }
}


void subscribe_client(int server_socket, string input) {

    Message packet;

    string topic = input.substr(10, input.size() - 10 - 2);
    if (topic.size() > 50) {
        die(__LINE__, "%s: Topic size is too big");
    }
    packet.type_of_command = 3;
    strcpy(packet.topic, topic.c_str());

    if (input[input.size() - 1] - '0' == 0)  {
        packet.sf = 0;
    } else if (input[input.size() - 1] - '0' == 1)  {
        packet.sf = 1;
    } else {
        die(__LINE__, "%s: SF other than 1/0");
    }

    send(server_socket, &packet, sizeof(packet), 0);

    cout << "Subscribed to topic.\n";
}

void unsubscribe_client(int server_socket, string input) {
    Message packet;

    string topic = input.substr(12, input.size() - 12);
    if (topic.size() > 10) {
        die(__LINE__, "%s: Topic size is too big");
    }
    packet.type_of_command = 4;
    strcpy(packet.topic, topic.c_str());

    send(server_socket, &packet, sizeof(packet), 0);

    cout << "Unsubscribed from topic.\n";

}

int process_packet_from_server (int server_socket) {
    Message packet;

    int nbytes = recv(server_socket, &packet, sizeof(packet), 0);
    if (nbytes < sizeof(packet)) {
        die(__LINE__, "%s: Error packet dropped");
    }

    if (packet.type_of_command == 1) {
        close(server_socket);
        return -1;
    } else if (packet.type_of_command == 2) {
        //print message
        //handle packet
        print_received_packet(packet);
    }

    return 1;
}