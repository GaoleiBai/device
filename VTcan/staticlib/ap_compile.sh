cp libScan.a /lib/
gcc -D _REENTRANT SJA1000_CAN_AP.c -o CANAP -lScan -lpthread

