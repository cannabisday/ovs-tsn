#!/bin/bash

# Open vSwitch 데이터베이스 서버를 시작
sudo ovsdb-server --remote=punix:/usr/local/var/run/openvswitch/db.sock \
--remote=db:Open_vSwitch,Open_vSwitch,manager_options \
--private-key=db:Open_vSwitch,SSL,private_key \
--certificate=db:Open_vSwitch,SSL,certificate \
--bootstrap-ca-cert=db:Open_vSwitch,SSL,ca_cert \
--pidfile --detach --log-file

# Open vSwitch 서비스를 시작
sudo /usr/local/share/openvswitch/scripts/ovs-ctl start

