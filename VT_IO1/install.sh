#!/bin/bash
cp NEXCOM_IO.ko /lib/modules/`uname -r`/kernel/drivers/char/
insmod NEXCOM_IO.ko
depmod -a
cp /etc/modules /etc/modules.old
sed '$a NEXCOM_IO' /etc/modules.old > /etc/modules	
rm /etc/modules.old
echo "Install success please reset your computer."
