#!/bin/bash

# 종료할 프로세스 이름들
PROCESSES=("ovsdb-server" "ovs-vswitchd")

#sudo ovs-vsctl del-br br03
#sudo ovs-vsctl del-br br04
# 종료할 프로세스를 검색하고 종료
#for PROC in "${PROCESSES[@]}"; do
#    PID=$(pgrep $PROC)
#    if [ -n "$PID" ]; then
#        echo "Killing $PROC with PID: $PID"
#        sudo kill -9 $PID
#    else
#        echo "$PROC not running."
#    fi
#done

# OVS 빌드 및 설치
echo "Building Open vSwitch..."
sudo make -j 10 && sudo make install

if [ $? -ne 0 ]; then
    echo "Build or install failed. Exiting."
    exit 1
fi

echo "Build and install completed successfully."

# OVS 데몬들 재시작
echo "Starting ovsdb-server..."
sudo ovsdb-server --remote=punix:/usr/local/var/run/openvswitch/db.sock \
--remote=db:Open_vSwitch,Open_vSwitch,manager_options \
--private-key=db:Open_vSwitch,SSL,private_key \
--certificate=db:Open_vSwitch,SSL,certificate \
--bootstrap-ca-cert=db:Open_vSwitch,SSL,ca_cert \
--pidfile --detach --log-file

echo "Starting ovs-vswitchd..."
sudo /usr/local/share/openvswitch/scripts/ovs-ctl start

echo "Open vSwitch services restarted successfully."
#sudo ovs-vsctl add-br br03
#sudo ovs-vsctl add-br br04
