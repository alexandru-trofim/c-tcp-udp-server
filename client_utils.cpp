#include "utils.hpp" 



void print_received_packet (Message packet) {

    char udp_ip[INET_ADDRSTRLEN + 1];
// char buffer[INET_ADDRSTRLEN + 1];
//       auto result = inet_ntop(AF_INET, &x, buffer, sizeof(buffer));
//       if (result == nullptr) throw std::runtime_error("Can't convert IP4 address");
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