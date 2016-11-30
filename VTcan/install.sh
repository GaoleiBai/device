#!/bin/bash
cp NEXCOM_CAN1.ko /lib/modules/`uname -r`/kernel/drivers/char/
insmod NEXCOM_CAN1.ko
depmod -a
cp /etc/modules /etc/modules.old
sed '$a NEXCOM_CAN1' /etc/modules.old > /etc/modules	
rm /etc/modules.old
echo "Install success please reset your computer."
