#ifndef PORT_SPEED_MAP_H
#define PORT_SPEED_MAP_H

#include "openvswitch/shash.h"

// 전역 port_speed_map 선언
extern struct shash port_speed_map;

// 초기화 및 사용 함수 선언
void init_port_speed_map(void);
void set_port_speed(const char *port_name, uint32_t speed);
uint32_t get_port_speed(const char *port_name);

#endif // PORT_SPEED_MAP_H
