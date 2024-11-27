#include <config.h>
#include <string.h>

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>
#include <string.h>  // 추가: memset을 사용하기 위해 필요

#define BUFFER_SIZE 2764800
#define DURATION 10

// 성능 측정 함수
uint32_t measure_performance(const char *server_ip, int port_number) {
    int sockfd;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    int packets_sent = 0, packets_received = 0;
    double total_time = 0.0;
    struct timeval start, end, tv;
    socklen_t len = sizeof(server_addr);

    // 소켓 생성
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }
    printf("Socket created successfully.\n");

    // 타임아웃 설정 (5초)
    tv.tv_sec = 5;  // 타임아웃 시간 (초)
    tv.tv_usec = 0; // 타임아웃 시간 (마이크로초)
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
        perror("Error setting socket receive timeout");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port_number);
    server_addr.sin_addr.s_addr = inet_addr(server_ip);  // 서버의 IP 주소 명확히 설정

    // 서버 주소 출력
    printf("Sending data to server at IP: %s, Port: %d\n", server_ip, port_number);

    //gettimeofday(&start, NULL);

    for (int i = 0; i < DURATION; ++i) {
        // 전송할 데이터 준비
        snprintf(buffer, BUFFER_SIZE, "Packet number: %d", i + 1);

        // 서버로 데이터 전송
        if (sendto(sockfd, buffer, strlen(buffer), 0, (const struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
            perror("Failed to send packet");
        } else {
            printf("Sent: %s to server IP: %s, Port: %d\n", buffer, inet_ntoa(server_addr.sin_addr), ntohs(server_addr.sin_port));
            packets_sent++;
        }

        // 서버로부터 데이터 수신 (타임아웃 적용됨)
        int n = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *) &server_addr, &len);
        if (n > 0) {
            buffer[n] = '\0';
            printf("Received: %s\n", buffer);
            packets_received++;
            if (packets_received == 1) {
                gettimeofday(&start, NULL); // 첫 번째 패킷 수신 시점 기록
            }
        } else if (n == 0) {
            printf("No data received from server (connection closed).\n");
        } else {
            perror("Receive failed or timeout");
            printf("Receive failed or timeout: Resource temporarily unavailable\n");  // 추가된 로그
        }

        usleep(1000000/DURATION);  // 1초 대기
    }

    gettimeofday(&end, NULL);

    total_time = (end.tv_sec - start.tv_sec) * 1.0 + (end.tv_usec - start.tv_usec) / 1000000.0;
    double bandwidth = ((packets_sent-1) * BUFFER_SIZE * 8) / (total_time) /10000000; // Mbps

    close(sockfd);
    printf("Total packets sent: %d, Total packets received: %d\n", packets_sent, packets_received);
    printf("Total time: %.2f s, Bandwidth: %.2f Mbps\n", total_time, bandwidth);

    return (uint32_t)bandwidth;
}

// UDP 클라이언트 시작 함수
uint32_t start_udp_client(const char *server_ip, int port_number) {
    return measure_performance(server_ip, port_number);
}