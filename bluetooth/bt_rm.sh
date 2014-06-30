#!/bin/bash
#name: bt_rm.sh
#author: Leeplus
#date: 2014-03-07
#description: rmmod bt modules from kernel
rmmod hci_uart
rmmod bnep
rmmod sco
rmmod rfcomm
rmmod l2cap
rmmod bluetooth
lsmod
