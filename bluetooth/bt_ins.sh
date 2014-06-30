#!/bin/bash
#name: bt_in.sh
#author: Leeplus
#date: 2014-03-07
#decription: insmod for bt modules
insmod /lib/modules/3.4.61+/kernel/net/bluetooth/bluetooth.ko
insmod /lib/modules/3.4.61+/kernel/net/bluetooth/l2cap.ko
insmod /lib/modules/3.4.61+/kernel/net/bluetooth/sco.ko
insmod /lib/modules/3.4.61+/kernel/net/bluetooth/bnep/bnep.ko
insmod /lib/modules/3.4.61+/kernel/net/bluetooth/rfcomm/rfcomm.ko
insmod /lib/modules/3.4.61+/kernel/drivers/bluetooth/hci_uart.ko
lsmod | grep bluetooth
