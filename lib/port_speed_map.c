#include <config.h>
#include "openvswitch/shash.h"
#include "port_speed_map.h"
#include <stdlib.h>
#include <stdio.h>  // 에러 로그 출력을 위한 필요 헤더
#include <ifaddrs.h>     // getifaddrs, struct ifaddrs
#include <arpa/inet.h>   // inet_ntop
#include <netinet/in.h>  // struct sockaddr_in
#include <sys/socket.h>  // socket functions
#include <netdb.h>       // getnameinfo, NI_MAXHOST, NI_NUMERICHOST
#include <sys/types.h>
#include "utilities/udp_client.h"
#include "utilities/udp_client.c"
#include "openvswitch/vlog.h"
#define MAX_NODES 15
#define ARP_COMMAND "arp -n |grep 192.168.10|grep -v 192.168.10.1"
#define BASE_PORT 20000
VLOG_DEFINE_THIS_MODULE(port_speed_map); // 고유 이름 사용


struct shash port_speed_map;

// 포트 속도를 설정할 때 사용할 기본값
static uint32_t default_speed = 1;

void init_port_speed_map(void) {
    shash_init(&port_speed_map);
    VLOG_WARN("Port speed map initialized");  // 초기화 확인용 로그

}
//init_port_speed_map();

void set_port_speed(const char *port_name, uint32_t speed) {
    if (!is_port_speed_map_initialized) {
        is_port_speed_map_initialized = true;

        init_port_speed_map();
        VLOG_INFO("ovs_psm)Port speed map initialized once.");
    } else {
        VLOG_INFO("ovs_psm)Port speed map already initialized.");
    }



    if (port_name == NULL) {
        VLOG_WARN("Error: Port name is NULL\n");
        return;
    }
    VLOG_WARN(" for port: %s, speed is none and dspeed is %u\n", port_name, speed);

    uint32_t *speed_ptr = malloc(sizeof(uint32_t));
    if (speed_ptr == NULL) {
        VLOG_WARN("Error: Memory allocation failed for port speed\n");
        return;
    }
    VLOG_WARN("malloc success");
    *speed_ptr = speed;
    VLOG_WARN("ovs_psm)replace start");

        // 포트가 이미 존재하는지 확인
    if (shash_find_data(&port_speed_map, port_name) != NULL) {
        // 이미 존재하므로 replace 수행
        VLOG_WARN("shash_replace start");
        shash_replace(&port_speed_map, port_name, speed_ptr);
        VLOG_WARN("Replaced speed for port %s with speed %u\n", port_name, speed);
    } else {
        // 존재하지 않으므로 add 수행
        VLOG_WARN("repalce add start");

        if (shash_add(&port_speed_map, port_name, speed_ptr) == NULL) {
            VLOG_WARN("ovs_psm)Error: Failed to add port %s, possibly duplicate key\n", port_name);
            free(speed_ptr);  // 오류 시 메모리 해제
        } else {
            VLOG_WARN("ovs_psm)Successfully added port %s with speed %u\n", port_name, *speed_ptr);
        }
    }
    VLOG_WARN("replace success");
    return;
    //uint32_t *found_speed_ptr = shash_find_data(&port_speed_map, port_name);
    //printf("Port %s speed set to %u\n", port_name, *found_speed_ptr);  // 성공적으로 속도 설정 시 출력
}

typedef struct {
    char ip[64];
    uint16_t port;
} NodeInfo;

uint32_t get_port_speed(const char *port_name) {
    /*for client*/
    int udp_client_port = 0; 
    char client_port_str[3] = {port_name[4], port_name[5], '\0' };  // 포트 이름의 마지막 2자리 숫자 추출
    udp_client_port = 20000 + atoi(client_port_str);
    //char new_port_name = "";

    NodeInfo nodes[MAX_NODES];
    int node_count = 0;
    FILE *arp_output;
    char line[256];


    arp_output = popen(ARP_COMMAND, "r");
    if (arp_output == NULL) {
        printf(stderr, "failed to arp command \n");
        return default_speed;
    }

    while (fgets(line, sizeof(line), arp_output) != NULL) {
        char ip[64];

        if (sscanf(line, "%63s",ip) == 1) {
            char *last_octet = strrchr(ip, '.');
            if (last_octet !=NULL) {
                int br_num = atoi(last_octet + 1);
                if (br_num != 1) {
                    nodes[node_count].port = BASE_PORT + br_num;
                    strncpy(nodes[node_count].ip, ip, sizeof(nodes[node_count].ip));
                    node_count++;
                    if (node_count >= MAX_NODES) {
                        break;
                    }

                }
            }
        }
    }
    pclose(arp_output);




    uint32_t new_speed = 2;  // 포트 속도를 설정할 새로운 속도 값


 
    if (port_name == NULL) {
        VLOG_WARN("ovs_psm)Port name is NULL, returning default speed of %u\n", default_speed);
        return default_speed;  // 포트 이름이 없을 경우 기본 속도 반환
    }

    VLOG_WARN("ovs_psm)Looking for port speed for port: %s", port_name);

    uint32_t *speed_ptr = shash_find_data(&port_speed_map, port_name);

    /*for client arpver*/
    if (speed_ptr == NULL) {
        printf("ovs_psm) speed_ptr null ");
        uint32_t bandwidths[MAX_NODES];
        int count = 0;
        for (int i = 0; i <node_count; i++){
            uint32_t bw = start_udp_client(nodes[i].ip, nodes[i].port);
            bandwidths[count++] = bw; 
            printf("tested ip %s , port %d, bw %u \n", nodes[i].ip, nodes[i].port, bw);
        }
 
        int count_1 = 0, count_10 = 0;
        double total_weight = 0 , weight_sum = 0;
        for (int i = 0; i < count; i++) {
            if(bandwidths[i] >= 4) {
                count_1++;
            } else {
               count_10++;
            }
        }
        int total_edges = count_1 + count_10;
        double ratio_1 = (total_edges > 0) ? (double)count_1 / total_edges : 0.0;
        double ratio_10 = (total_edges > 0) ? (double)count_1 / total_edges : 0.0;
        for (int i = 0; i < count; i++) {
            int weight = (bandwidths[i] >= 4) ? 1 : 10;
            if (weight ==1) {
                total_weight += weight * (10*(ratio_1*ratio_1));
                weight_sum += (10*(ratio_1*ratio_1));

            } else {
                total_weight += weight * (1*ratio_10);
                weight_sum += (1*ratio_10);
            }
        }
        uint32_t weighted_bandwidth = (weight_sum > 0) ? (uint32_t)(total_weight / weight_sum) : default_speed;
        printf("weighted bw port %s: %u Mbps \n", port_name, weighted_bandwidth);
        return weighted_bandwidth;

    }



    

    //VLOG_WARN("Looking for the port speed: %u", *speed_ptr);

    /*for client githubver*/
    /*
    if (speed_ptr == NULL) {
        if (strlen(port_name) == 6 && port_name[0] == 'p' && port_name[3] == 't' && port_name[5] != '0') {
        // 클라이언트가 실행될 부분 (포트 이름이 pXXtYY 형식일 때)
            char client_port_str[3] = { port_name[4], port_name[5], '\0' };
            udp_client_port = 20000 + atoi(client_port_str);

        // 클라이언트가 서버로 연결되도록 설정
            printf("Connecting to UDP server at 127.0.0.1, port %d\n", udp_client_port);

        // UDP 클라이언트 시작
            uint32_t bandwidth = start_udp_client("127.0.0.1", udp_client_port);

            printf("Measured bandwidth for client %s: %u Mbps\n", port_name, bandwidth);
            
            return bandwidth;  


        //const char *port_name = "p04t03";  // 임의로 포트 이름 지정
        
        //uint32_t speed = get_port_speed(port_name);  // 포트 속도 가져오기
        //printf("Port %s speed is: %u\n", port_name, speed);
        //joo) psm 1015
    


    } else {
        // pXXtYY 형식이 아닌 경우 처리
        printf("ovs_psm)Port %s does not match the format pXXtYY, skipping UDP client start.\n", port_name);
    }












        VLOG_WARN("ovs_psm)Port speed of %s is not found, returning default speed of %u", port_name, default_speed);
        return default_speed;  
    }*/
    VLOG_WARN("ovs_psm)valid speed ptr is found and set");
    //VLOG_WARN("Port %s speed is %u", port_name, *speed_ptr);
    //return default_speed;
    return *speed_ptr;
}