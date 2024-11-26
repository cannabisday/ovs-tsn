#include "udp_server.h" // 필요한 경우 헤더 파일 추가
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>  // 에러 코드 출력을 위한 헤더 추가

#define BUFFER_SIZE 4096

// UDP 서버 스레드 함수
void *udp_server_thread(void *arg) {
    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    char buffer[BUFFER_SIZE];
    socklen_t addr_len = sizeof(client_addr);
    int port = *(int *)arg;
    free(arg);

    printf("Starting UDP server thread on port %d...\n", port);
    fflush(stdout); // 강제로 출력 버퍼 비우기

    // 소켓 생성
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        return NULL;
    }
    printf("Socket created successfully.\n");
    fflush(stdout); // 강제로 출력 버퍼 비우기

    memset(&server_addr, 0, sizeof(server_addr));
    memset(&client_addr, 0, sizeof(client_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY; // 모든 인터페이스에서 수신할 수 있도록 설정
    server_addr.sin_port = htons(port);

    // 포트 바인딩
    if (bind(sockfd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        int err = errno;  // 에러 번호 저장
        perror("Bind failed");
        printf("Error code: %d\n", err);  // 에러 번호 출력
        fflush(stdout); // 강제로 출력 버퍼 비우기
        close(sockfd);
        return NULL;
    }
    printf("Socket bound to port %d successfully.\n", port);
    fflush(stdout); // 강제로 출력 버퍼 비우기

    // 수신 타임아웃 설정 (5초)
    struct timeval tv;
    tv.tv_sec = 5;  // 5초 타임아웃
    tv.tv_usec = 0;
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
        perror("Error setting socket receive timeout");
        close(sockfd);
        return NULL;
    }

    while (1) {
        // 여기에서 서버 상태를 확인하기 위한 디버그 메시지를 출력합니다.
        printf("Checking server status before receiving data...\n");
        fflush(stdout); // 강제로 출력 버퍼 비우기

        printf("Waiting to receive data...\n");  // 수신 대기 상태 디버깅 메시지
        fflush(stdout); // 강제로 출력 버퍼 비우기

        int n = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&client_addr, &addr_len);
        if (n < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                printf("Receive timed out, no data received.\n");
                fflush(stdout); // 강제로 출력 버퍼 비우기
            } else {
                perror("Receive failed");
            }
        } else {
            buffer[n] = '\0';
            printf("Received: %s from client IP: %s, Port: %d\n", buffer, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
            fflush(stdout); // 강제로 출력 버퍼 비우기

            // 클라이언트에게 응답 전송
            snprintf(buffer, BUFFER_SIZE, "ACK: Received your packet");
            if (sendto(sockfd, buffer, strlen(buffer), 0, (const struct sockaddr *)&client_addr, addr_len) < 0) {
                perror("Failed to send ACK to client");
            } else {
                printf("Sent ACK to client IP: %s, Port: %d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
                fflush(stdout); // 강제로 출력 버퍼 비우기
            }
        }
    }

    close(sockfd);
    return NULL;
}


// UDP 서버 시작 함수
void start_udp_server(int port) {
    pthread_t thread_id;
    int *server_port = malloc(sizeof(int));
    *server_port = port;
    printf("Preparing to start UDP server on port %d...\n", port);
    
    // 서버 스레드 생성
    if (pthread_create(&thread_id, NULL, udp_server_thread, server_port) != 0) {
        perror("Failed to create server thread");
        free(server_port);
        return;
    }
    pthread_detach(thread_id); // 스레드 분리, 메인 스레드와 독립적으로 동작
    printf("UDP server thread started on port %d.\n", port);
}
