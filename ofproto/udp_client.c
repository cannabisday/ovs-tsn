#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/time.h>
#include "udp_client.h"
//#include "ofproto/ofproto-provider.h" // ofport 구조체 포함





#define BUFFER_SIZE 1024
#define DURATION 10




uint32_t measure_performance(const char *server_ip, int port_number) {
    int sockfd;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    int packets_sent = 0, packets_received = 0;
    double total_time = 0.0;
    struct timeval start, end;
    socklen_t len = sizeof(server_addr);

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port_number);
    server_addr.sin_addr.s_addr = inet_addr(server_ip);

    gettimeofday(&start, NULL);

    for (int i = 0; i < DURATION; ++i) {
        sendto(sockfd, buffer, BUFFER_SIZE, 0, (const struct sockaddr *) &server_addr, sizeof(server_addr));
        packets_sent++;

        if (recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *) &server_addr, &len) > 0) {
            packets_received++;
        }

        usleep(1000000);
    }

    gettimeofday(&end, NULL);

    total_time = (end.tv_sec - start.tv_sec) * 1000.0 + (end.tv_usec - start.tv_usec) / 1000.0;

    double bandwidth = (packets_sent * BUFFER_SIZE * 8) / (DURATION * 1000000.0); // Mbps

    close(sockfd);
    return (uint32_t)bandwidth;
}

uint32_t start_udp_client(const char *server_ip, int port_number) {
    return measure_performance(server_ip, port_number);
}

/*
struct network_metrics measure_performance(const char *server_ip, int port_number) {
    int sockfd;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    int packets_sent = 0, packets_received = 0;
    struct timeval start, end;
    double total_time = 0.0;
    socklen_t len;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port_number);
    inet_pton(AF_INET, server_ip, &server_addr.sin_addr);

    gettimeofday(&start, NULL);
    for (int i = 0; i < DURATION; i++) {
        sendto(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
        packets_sent++;

        len = sizeof(server_addr);
        if (recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&server_addr, &len) > 0) {
            packets_received++;
        }

        gettimeofday(&end, NULL);
        total_time += (end.tv_sec - start.tv_sec) * 1000.0;
        total_time += (end.tv_usec - start.tv_usec) / 1000.0;
    }

    struct network_metrics metrics;
    metrics.bandwidth = (packets_sent * BUFFER_SIZE * 8) / (DURATION * 1000000.0); // Mbps
    metrics.loss = ((packets_sent - packets_received) / (double)packets_sent) * 100; // percentage
    metrics.delay = total_time / packets_sent; // ms

    close(sockfd);
    return metrics;
}

void *udp_client_thread(void *arg) {
    struct client_args *args = (struct client_args *)arg;
    struct network_metrics metrics = measure_performance(args->server_ip, args->port_number);

    args->port->pp.curr_speed = (unsigned int)metrics.bandwidth;
    printf("Port %d Current Speed: %u\n", args->port->pp.port_no, args->port->pp.curr_speed);

    pthread_exit(NULL);
}

void start_udp_client(const char *server_ip, int port_number, struct ofport *ofp) {
    pthread_t thread_id;
    struct client_args *args = malloc(sizeof(struct client_args));
    args->server_ip = server_ip;
    args->port_number = port_number;
    args->port = ofp;

    pthread_create(&thread_id, NULL, udp_client_thread, (void *)args);
    pthread_detach(thread_id);
}
*/