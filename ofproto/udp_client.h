#ifndef UDP_CLIENT_H
#define UDP_CLIENT_H

/*#include "ofproto/ofproto-provider.h" // ofport 구조체 포함

struct network_metrics {
    double bandwidth;
    double loss;
    double delay;
};

struct client_args {
    const char *server_ip;
    int port_number;
    struct ofport *port;
};

struct network_metrics measure_performance(const char *server_ip, int port_number);
void *udp_client_thread(void *arg);
void start_udp_client(const char *server_ip, int port_number, struct ofport *ofp);

#endif // UDP_CLIENT_H

*/


#include <stdint.h>

uint32_t measure_performance(const char *server_ip, int port_number);
uint32_t start_udp_client(const char *server_ip, int port_number);

#endif // UDP_CLIENT_H