gcc -D REENTRANT -c SJA1000_CAN.c -lpthread
ar crv libScan.a SJA1000_CAN.o
cp libScan.a /lib/

