#!/bin/bash

# 브릿지 추가
sudo ovs-vsctl add-br br30
sudo ovs-vsctl add-br br40

# OpenFlow 버전 설정
sudo ovs-vsctl set bridge br40 protocols=OpenFlow10,OpenFlow11,OpenFlow12,OpenFlow13
sudo ovs-vsctl set bridge br30 protocols=OpenFlow10,OpenFlow11,OpenFlow12,OpenFlow13

# 컨트롤러 설정
sudo ovs-vsctl set-controller br30 tcp:127.0.0.1:6653
sudo ovs-vsctl set-controller br40 tcp:127.0.0.1:6653

# 포트 추가
sudo ovs-vsctl add-port br30 p30t00
sudo ovs-vsctl add-port br40 p40t00

# 패치 인터페이스 설정
sudo ovs-vsctl set interface p04t00 type=patch options:peer=p03t00
sudo ovs-vsctl set interface p03t00 type=patch options:peer=p04t00

echo "OVS setup complete."
