Am impelmentat un protocol peste tcp cu ajutorul unei structuri de mesaje.

struct Message {
    char      msg[1500];
    char      topic[50];
    char      id[10];
    uint32_t  ip_udp_sender;
    uint16_t  port_udp_sender;
    uint8_t   type_of_command;
    uint8_t   sf;
    uint8_t   data_type;

} 
Aceasta structura se foloseste pentru a trimite de la udp la clientii abonati,
dar si pentru comunicare dintre clienti si server.
Exista 5 tipuri de mesaje si se seteaza in "type_of_command":
    1 - mesajul de tipul destroy (cand serverul ii spune
         unui client sa se autodistruga)
    2 - message with topic (serverul trimite mesaj pe care
        l-a primit de la un client udp si il trimite unui
        client tcp care e abonat la topicul respectiv)
    3 - subscribe (un client tcp il instiinteaza pe server
        la ce topic vrea sa fie abonat)
    4 - unsubscribe (un client tcp il instiinteaza pe server
        la ce topic vrea sa fie dezabonat)
    5 - exit command from client (clientul il instiinteaza
        pe server ca vrea sa dea log out)

Flow-ul serverului: 
        Serverul deschide 2 conexiuni una tcp si una udp. Cu ajutorul conexiunii tcp 
    serverul primeste noi cereri de conexiune de la clienti. Iar clientii udp 
    ii trimit mesaje pe care acesta trebuie sa le redirectioneze la clientii tcp
    care sunt abonati la topicul la care este atribuit mesajul.
        Pentru a facilita implementarea am folosit o structura numita ServerInfo
    care pastreaza toate informatiile necesare pentru server precum vectorul de file
    descriptori si vectorul de utilizatori.
        De asemenea am folosti si o structura pentru fiecare utilizator tcp. Aceasta
    contine fd-ul socketului prin care serverul este conectat cu utilizatorul, statutul
    acestuia, topic-urile pe care le urmareste, id ul sau si un vector de mesaje. Vectorul
    de mesaje stocheaza mesajele pe care utilizatorul le primeste cat timp nu este logat in
    caz ca are setat valoarea sf pentru anumite topic uri.

