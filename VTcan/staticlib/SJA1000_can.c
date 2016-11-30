// Reset the CAN device. Issuing this IOCTL causes the
// device to stop and reset. After the device has been
// reset, the device must be reconfigured and explicitly set
// to run.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <pthread.h> 

typedef struct can_rx_dual_filter
{
        int  acr_filter1_id;           // ID    (11bit)
        int  acr_filter1_rtr;          // RTR   (1bit)

        int  amr_filter1_id;           // ID    (11bit)
        int  amr_filter1_rtr;          // RTR   (1bit)

        int  acr_filter2_id;           // ID    (11bit)
        int  acr_filter2_rtr;          // RTR   (1bit)

        int  amr_filter2_id;           // ID    (11bit)
        int  amr_filter2_rtr;          // RTR   (1bit)

        int  acr_data1;                        // Data1 (8bit)
        int  amr_data1;                        // Data1 (8bit)

} can_rx_dual_filter_t;

typedef struct can_rx_single_standard_filter
{
        int acr_id;            // ID    (11bit)
        int acr_rtr;           // RTR   (1bit)
        int acr_data1;         // Data1 (8bit)
        int acr_data2;         // Data2 (8bit)

        int amr_id;            // ID    (11bit)
        int amr_rtr;           // RTR   (1bit)
        int amr_data1;         // Data1 (8bit)
        int amr_data2;         // Data2 (8bit)

} can_rx_single_standard_filter_t;

typedef struct can_rx_single_extended_filter
{
        int acr_id;            // ID    (29bit)
        int acr_rtr;           // RTR   (1bit)

        int amr_id;            // ID    (29bit)
        int amr_rtr;           // RTR   (1bit)

} can_rx_single_extended_filter_t;
#define CAN_MSG_DATA_LEN                        (8)                     // CAN Msg data length


typedef struct can_msg
{
        unsigned short ide;                                                     // Standard/extended msg
        unsigned int id;                                                        // 11 or 29 bit msg id
        unsigned short dlc;                                                     // Size of data
        unsigned char data[CAN_MSG_DATA_LEN];           // Message pay load
        unsigned short rtr;                                                     // RTR message
} can_msg_t;


//******************************************************************
// global variable
//*****************************************************************
pthread_t a_thread;
int cancel=1,critical_read=1,critical_write=1;
int point_read=0,point_write=0,read_write=1;
can_msg_t ReadBuffer[1000];
can_msg_t WriteBuffer[1000];

void Clear_Buffer(int fh){
	
	while (critical_write==0) {}
        if (critical_write==1) {
		critical_write=0;
                while (critical_read==0) {}
		if (point_read==0){
                        critical_read=0;
			int dwDATA=0;
                        read(fh,&dwDATA,0x01); //pfnGetPortVal(0x0E70, &dwDATA, nByteSize );
                        dwDATA=dwDATA & 0xfd;
                        write(fh,&dwDATA,0x01);
                        dwDATA=dwDATA | 0x02;
                        write(fh,&dwDATA,0x01);
			point_read=0;
			point_write=0;
		}
	}
	critical_write==1;
	critical_read ==1;
}


void *thread_function(void *arg)
{
	int fh=0,nResult=0;
	fh = (int*) arg;
	can_msg_t c_msg;
        memset(&c_msg, 0, sizeof(c_msg));
	while( cancel )
        {
          //      printf("1\n");
		nResult = CAN_Receive_and_PushToBuffer(fh,&c_msg);
	//	printf("2\n");
                if( (1 == nResult) )
                {
//                       	printf("Read => IDe(%d), ID(%x), Len(%d), Data1~8(0x%x)(0x%x)(0x%x)(0x%x)(0x%x)(0x%x)(0x%x)(0x%x), RTR(%d)\n", c_msg.ide, c_msg.id, c_msg.dlc, c_msg.data[0], c_msg.data[1], c_msg.data[2], c_msg.data[3], c_msg.data[4], c_msg.data[5], c_msg.data[6], c_msg.data[7], c_msg.rtr);

			// push CAN message to buffer
                       	PushMSGinReceiveVector(&c_msg);
			read_write==0 ;
		//	printf("4\n");
//			printf("now is receiver date.\n");
			usleep(5000);
		//	printf("5\n");
                }
		// check whether has CAN message need to send
		int dwTS = 0x0;
                read(fh,&dwTS,0x02);//pfnGetPortVal(0x0E72, &dwTS, nByteSize );
		//printf("6\n");
                if( dwTS & 0x020 ){}
			//printf("There is a message on CAN device.\n"); 
                else if ( 1 == PopMSGinTransmitVector(&c_msg) )
                {
                       	nResult=CAN_Transmission_when_Idle(fh,&c_msg);
                 	//printf("The value of CAN_Transmission_when_Idle is %d.\n",nResult);
			
		}
                
        	usleep(5000);	
	}

	pthread_exit("Exit thread\n");

}

//****************************************************************
int PopMSGinTransmitVector(can_msg_t *pMSG)
{
	while (critical_write==0) {}
	if (critical_write==1) {
                critical_write=0;

                if (point_write==0){
                        critical_write=1;
                        return -1;
                }
                else{
                       // printf("Write message to buffer.\n");
			//printf("pop dlc is %d\n",WriteBuffer[point_write-1].dlc);
			//printf("pop id is %x\n",WriteBuffer[point_write-1].id);
			pMSG->ide = WriteBuffer[point_write-1].ide;
                        pMSG->id  = WriteBuffer[point_write-1].id ;
                        pMSG->dlc = WriteBuffer[point_write-1].dlc;
                        pMSG->data[0] = WriteBuffer[point_write-1].data[0];
                        pMSG->data[1] = WriteBuffer[point_write-1].data[1];
			pMSG->data[2] = WriteBuffer[point_write-1].data[2];
			pMSG->data[3] = WriteBuffer[point_write-1].data[3];
			pMSG->data[4] = WriteBuffer[point_write-1].data[4];
			pMSG->data[5] = WriteBuffer[point_write-1].data[5];
			pMSG->data[6] = WriteBuffer[point_write-1].data[6];
			pMSG->data[7] = WriteBuffer[point_write-1].data[7];
			pMSG->rtr  = WriteBuffer[point_write-1].rtr;
			point_write=point_write-1;
                        critical_write=1;
                }
        }
        return 1;
}

int CAN_Transmission(can_msg_t *msg)
{
        return PushMSGinTransmitVector(msg) ;
        
}
int PushMSGinTransmitVector(can_msg_t *pMSG)
{
        while(critical_write==0){}
        if (critical_write==1) {
                critical_write=0;
                if (point_write>1000){
                        critical_write=1;
                        return -1;
                }
                else{
                        
//			printf("Push write.\n");
			WriteBuffer[point_write].ide = pMSG->ide;
                        WriteBuffer[point_write].id  = pMSG->id ;
                        WriteBuffer[point_write].dlc = pMSG->dlc;
                        WriteBuffer[point_write].data[0] = pMSG->data[0];
                        WriteBuffer[point_write].data[1] = pMSG->data[1];
                        WriteBuffer[point_write].data[2] = pMSG->data[2];
                        WriteBuffer[point_write].data[3] = pMSG->data[3];
                        WriteBuffer[point_write].data[4] = pMSG->data[4];
                        WriteBuffer[point_write].data[5] = pMSG->data[5];
                        WriteBuffer[point_write].data[6] = pMSG->data[6];
                        WriteBuffer[point_write].data[7] = pMSG->data[7];
                        WriteBuffer[point_write].rtr = pMSG->rtr;
                        point_write=point_write+1;
                        critical_write=1;
                }
        }
        return point_write;

}
//***************************************************************************
int PushMSGinReceiveVector(can_msg_t* pMSG)
{
        while(critical_read==0){}
	if (critical_read==1) {
		critical_read=0;
        	if (point_read>1000){
			critical_read=1;	
			return -1;
		}
		else{
			ReadBuffer[point_read].ide    =pMSG->ide;
			ReadBuffer[point_read].id     =pMSG->id;
			ReadBuffer[point_read].dlc    =pMSG->dlc;
			ReadBuffer[point_read].data[0]=pMSG->data[0];
			ReadBuffer[point_read].data[1]=pMSG->data[1];
			ReadBuffer[point_read].data[2]=pMSG->data[2];
			ReadBuffer[point_read].data[3]=pMSG->data[3];	
			ReadBuffer[point_read].data[4]=pMSG->data[4];
			ReadBuffer[point_read].data[5]=pMSG->data[5];
			ReadBuffer[point_read].data[6]=pMSG->data[6];
			ReadBuffer[point_read].data[7]=pMSG->data[7];
			ReadBuffer[point_read].rtr    =pMSG->rtr;
			point_read=point_read+1;
			critical_read=1;
		}
	}
        return 1;
}

//t CAN_Receive_nonblock(can_msg_t *msg)
//
        // 檢查有無資料, 無資料立即返回
//      return PopMSGinReceiveVector(msg);
//

int CAN_Receive_nonblock(can_msg_t *pMSG)
{
        //printf("The critical_read is %d\n",critical_read);
	while(critical_read==0){}
        if (critical_read==1) {
                critical_read=0;
        	if (point_read==0){
                        critical_read=1;
                        return -1;
		}
		else{
			pMSG->ide = ReadBuffer[point_read-1].ide;
                        pMSG->id  = ReadBuffer[point_read-1].id ;
                        pMSG->dlc = ReadBuffer[point_read-1].dlc;
                        pMSG->data[0] = ReadBuffer[point_read-1].data[0];
			pMSG->data[1] = ReadBuffer[point_read-1].data[1];
			pMSG->data[2] = ReadBuffer[point_read-1].data[2];
			pMSG->data[3] = ReadBuffer[point_read-1].data[3];
			pMSG->data[4] = ReadBuffer[point_read-1].data[4];
			pMSG->data[5] = ReadBuffer[point_read-1].data[5];
			pMSG->data[6] = ReadBuffer[point_read-1].data[6];
			pMSG->data[7] = ReadBuffer[point_read-1].data[7];
			pMSG->rtr = ReadBuffer[point_read-1].rtr;
			point_read=point_read-1;
                        critical_read=1;

		}
	}

        return point_read;
}


int CAN_Receive_and_PushToBuffer(int fh,can_msg_t *msg)
{
        {
                // 檢查是否為 Operation Mode
              
		Check_is_Operation_Mode(fh);
		
                // 檢查有無接收資料
                // 若有資料進來 0E72 的 bit0 變 1 (RBS), 0E73 的 bit 0 變 1 (RI) (此為接收中斷, 需啟動接收中斷的功能)
                // 0E8D 為資料計數器(RMC), 只要計數不為 0 則 0E72 的 bit0 永遠為 1 (RBS), 0E73 的 bit0 永遠為 1 (RI)
                // 0E71 bit2 設為 1 (RRB), 則 0E8D 計數器減 1 (RMC), 0E8E 的值改變 (RX buffer start address 變為下一個位址)(RBSA)
                // RX buffer 只有 64 Byte, 每筆資料 13 byte, 可存放5筆資料 (應只有4筆才對, 5筆要65Byte, 但計數器可累加至5)
                // RX buffer 存滿5筆資料, 若有資料進來則 0E73 bit3 (DOI) 瞬間變為 1 後再變回 0

                int dwRBS = 0x0;
                read(fh,&dwRBS,0x02);//pfnGetPortVal(0x0E72, &dwRBS, nByteSize );
		if ( dwRBS & 0x01 )
                {
			int dwRBSA = 0x0;
                        read(fh,&dwRBSA,0x1e);//pfnGetPortVal(0x0E8E, &dwRBSA, nByteSize );

                        // 取出資料
                        int dwDATA = 0x0;

                        // Extended Frame, Data Length Code bit.
                        
                        read(fh,&dwDATA,0x10);//pfnGetPortVal(0x0E80, &dwDATA, nByteSize );

                                // Read Extended Frame
                                // 0 : SFF; standard frame format will be transmitted by the CAN controller
                                // 1 : EFF; extended frame format will be transmitted by the CAN controller
                        if( dwDATA & 0x080 )
                                 msg->ide = 1; // Extended msg
                        else
                                 msg->ide = 0; // Standard msg

                                // Read RTR message
                                // 1 : remote; remote frame will be transmitted by the CAN controller
                                // 0 : data; data frame will be transmitted by the CAN controller
                        if( dwDATA & 0x040 )
                                 msg->rtr = 1;
                        else
                                 msg->rtr = 0;

                                // Read Data Length Code bit.
                        msg->dlc = dwDATA & 0x0F;
                        
                        // Read ID
                        if( msg->ide == 0 )
                        {
                                // Standard msg
                                // ID.28 to ID.18
                                int ID_13_20 = 0x0;
                                int ID_21_28 = 0x0;

                                read(fh,&ID_13_20,0x12);//pfnGetPortVal(0x0E82, &ID_13_20, nByteSize );
                                read(fh,&ID_21_28,0x11);//pfnGetPortVal(0x0E81, &ID_21_28, nByteSize );

                                msg->id = 0;
 				msg->id = msg->id | (ID_13_20 >> 5);
                                msg->id = msg->id | (ID_21_28 << 3);
        /*
                                // RTR
                                if( ID_13_20 & 0x010 )
                                        msg->rtr = 1;
                                else
                                        msg->rtr = 0;
        */
                        }
                        else
                        {
                                // Extended msg
                                // ID.28 to ID.0
                                int ID_00_04 = 0x0;
                                int ID_05_12 = 0x0;
                                int ID_13_20 = 0x0;
                                int ID_21_28 = 0x0;

                                read(fh,&ID_00_04,0x14);//pfnGetPortVal(0x0E84, &ID_00_04, nByteSize );
                                read(fh,&ID_05_12,0x13);//pfnGetPortVal(0x0E83, &ID_05_12, nByteSize );
                                read(fh,&ID_13_20,0x12);//pfnGetPortVal(0x0E82, &ID_13_20, nByteSize );
                                read(fh,&ID_21_28,0x11);//pfnGetPortVal(0x0E81, &ID_21_28, nByteSize );

                                msg->id = 0;
                                msg->id = msg->id | (ID_00_04 >> 3);
                                msg->id = msg->id | (ID_05_12 << 5);
                                msg->id = msg->id | (ID_13_20 << 13);
                                msg->id = msg->id | (ID_21_28 << 21);
        /*
                                // RTR
                                if( ID_00_04 & 0x04 )
                                        msg->rtr = 1;
                                else
                                        msg->rtr = 0;
        */
                        }
                        // Read Data
                        if( msg->ide == 0 )
                        {
                                // Standard msg
                                // 8-Bytes DATA
                                int dwCanData;
                                read(fh,&dwCanData,0x13);//pfnGetPortVal(0x0E83, &dwCanData, nByteSize );
                                msg->data[0] = (int)(dwCanData);
                                read(fh,&dwCanData,0x14);//pfnGetPortVal(0x0E84, &dwCanData, nByteSize );
                                msg->data[1] = (int)(dwCanData);
                                read(fh,&dwCanData,0x15);//pfnGetPortVal(0x0E85, &dwCanData, nByteSize );
                                msg->data[2] = (int)(dwCanData);
                                read(fh,&dwCanData,0x16);//pfnGetPortVal(0x0E86, &dwCanData, nByteSize );
                                msg->data[3] = (int)(dwCanData);
                                read(fh,&dwCanData,0x17);//pfnGetPortVal(0x0E87, &dwCanData, nByteSize );
                                msg->data[4] = (int)(dwCanData);
                                read(fh,&dwCanData,0x18);//pfnGetPortVal(0x0E88, &dwCanData, nByteSize );
                                msg->data[5] = (int)(dwCanData);
                                read(fh,&dwCanData,0x19);//pfnGetPortVal(0x0E89, &dwCanData, nByteSize );
                                msg->data[6] = (int)(dwCanData);
                                read(fh,&dwCanData,0x1a);//pfnGetPortVal(0x0E8A, &dwCanData, nByteSize );
                                msg->data[7] = (int)(dwCanData);
                        }
                        else
                        {
                                // Extended msg
                                // 8-Bytes DATA
                                int dwCanData;
                                read(fh,&dwCanData,0x15);//pfnGetPortVal(0x0E85, &dwCanData, nByteSize );
                                msg->data[0] = (int)(dwCanData);
                                read(fh,&dwCanData,0x16);//pfnGetPortVal(0x0E86, &dwCanData, nByteSize );
                                msg->data[1] = (int)(dwCanData);
                                read(fh,&dwCanData,0x17);//pfnGetPortVal(0x0E87, &dwCanData, nByteSize );
                                msg->data[2] = (int)(dwCanData);
                                read(fh,&dwCanData,0x18);//pfnGetPortVal(0x0E88, &dwCanData, nByteSize );
                                msg->data[3] = (int)(dwCanData);
                                read(fh,&dwCanData,0x19);//pfnGetPortVal(0x0E89, &dwCanData, nByteSize );
                                msg->data[4] = (int)(dwCanData);
                                read(fh,&dwCanData,0x1a);//pfnGetPortVal(0x0E8A, &dwCanData, nByteSize );
                                msg->data[5] = (int)(dwCanData);
                                read(fh,&dwCanData,0x1b);//pfnGetPortVal(0x0E8B, &dwCanData, nByteSize );
                                msg->data[6] = (int)(dwCanData);
                                read(fh,&dwCanData,0x1c);//pfnGetPortVal(0x0E8C, &dwCanData, nByteSize );
                                msg->data[7] = (int)(dwCanData);
			}
		
                        // 資料讀完, 需清除 RX Buffer
                        read(fh,&dwDATA,0x01);//pfnGetPortVal(0x0E71, &dwDATA, nByteSize );
                        dwDATA = dwDATA | 0x04;
                        write(fh,&dwDATA,0x01);//pfnSetPortVal(0x0E71, dwDATA, nByteSize );
                        //usleep(1000);

                        int bTimeOut = 1;

                        // 接著等待 RBSA 改變
                        // 0E8E 的 RX buffer start address 變為下一個位址
                        int i =0;
			for ( i=0 ; i<1000 ; i++ )
                        {
                                int dwRBSA_Next = 0x0;
                                read(fh,&dwRBSA_Next,0x1e);//pfnGetPortVal(0x0E8E, &dwRBSA_Next, nByteSize );
                                if( dwRBSA != dwRBSA_Next )
                                {
                                        bTimeOut = 0;
                                        break;
                                }

                                //usleep(1000);
                        }

#ifdef _DEBUG
                        if( bTimeOut )
                                printf("Time out!!!\n");
#endif
                        return 1;
                
		}
        }

        return -1;
}

//*******************************************************************************************








int  CAN_Run(int fh)
{
        int res;
	cancel =1;
	critical_read=1;
	critical_write=1;
	point_read=0;
	point_write=0;
	printf("creat thread\n");
	res = pthread_create(&a_thread,NULL,thread_function,(void*)fh);
	if (res!=0){
		printf("The thread creat fail.\n");
                return -1;
	}
	else {
        	return 1;
	//	printf("The thread create success\n");
	}
}
int CAN_Stop()
{
       	int res;
	void *thread_result;
	cancel=0;
	res = pthread_join(a_thread,&thread_result);
	if (res!=0){
 		//printf("The thread join fail.\n");
                return -1;
        }
	else{
		//printf("Join result is %s\n",(char*) thread_result);                
		return 1;
	}
}


void CAN_Reset(int fh)
{

	{
		// 若不是 Reset Mode 就切換為 Reset Mode
		int dwDATA = 0x0;
		read(fh,&dwDATA,0x0); //pfnGetPortVal(0x0E70, &dwDATA, nByteSize );
		if( (dwDATA & 0x01) == 0 )
		{
			dwDATA = dwDATA | 0x01;
			write(fh,&dwDATA,0x00);//pfnSetPortVal(0x0E70, dwDATA, nByteSize );
			//sleep(1);
		}
	}
}

int Error_Detect(int fh)
{

        {
                int dwDATA = 0x0;
                read(fh,&dwDATA,0x0c); //pfnGetPortVal(0x0E70, &dwDATA, nByteSize );
   		return dwDATA;             
        }
}

void Abort_Transmission(int fh)
{

        {
                int dwDATA = 0x0;
		while(critical_write==0){}
		if (critical_write==1){
			critical_write=0;
			point_write=0;
			read(fh,&dwDATA,0x01); //pfnGetPortVal(0x0E70, &dwDATA, nByteSize );
                	dwDATA=dwDATA & 0xfd;
			write(fh,&dwDATA,0x01);
			dwDATA=dwDATA | 0x02;
                	write(fh,&dwDATA,0x01);
		}
        }
}


// Set the timing (baud rate) using one of the pre-defined
// recommended timings.
int CAN_SetBaud(int fh,int nBaudRate)
{
		CAN_Reset(fh);

		int dwDATA = 0;

		// Baud Rate
		double BRP = 0;
		int BRP_check = 0;
		int TSEG1 = 1;
		int TSEG2 = 0;

		int nSAM = 0;

		// 公式1
		// TSEG1 = 1, TSEG2 = 0 => SAM = 0
		TSEG1 = 1;
		TSEG2 = 0;

		BRP = ( ((double)8000000) / (nBaudRate*(3+TSEG1+TSEG2)) ) -1;
#ifdef _DEBUG
		long a_1= 3+TSEG1+TSEG2;
		long b_1= nBaudRate*a_1;
		double c_1= (double)8000000/b_1;
		double BRP_1 = c_1-1;
#endif
               
		BRP_check = (int)(BRP);
		// BRP 不能整除 或 BRP > 63 則 BRP 無效
		if(    (BRP-BRP_check) != 0
			|| BRP > 63
			|| BRP <= 0)
		{
			// 公式2
			// TSEG1 = 5, TSEG2 = 2 => SAM = 1
			TSEG1 = 5;
			TSEG2 = 2;
			BRP = ( ((double)8000000) / (nBaudRate*(3+TSEG1+TSEG2)) ) -1;
#ifdef _DEBUG
			long a_2= 3+TSEG1+TSEG2;
			long b_2= nBaudRate*a_2;
			double c_2= (double)8000000/b_2;
			double BRP_2 = c_2-1;
#endif
			BRP_check = (int)(BRP);
			// BRP 不能整除 或 BRP > 63 則 BRP 無效
			if(    (BRP-BRP_check) != 0
				|| BRP > 63
				|| BRP <= 0)
			{
				// 公式3
				// TSEG1 = 15, TSEG2 = 7 => SAM = 1
				TSEG1 = 15;
				TSEG2 = 7;
				BRP = ( ((double)8000000) / (nBaudRate*(3+TSEG1+TSEG2)) ) -1;
#ifdef _DEBUG
				long a_3= 3+TSEG1+TSEG2;
				long b_3= nBaudRate*a_3;
				double c_3= (double)8000000/b_3;
				double BRP_3 = c_3-1;
#endif
				BRP_check = (int)(BRP);
				// BRP 不能整除 或 BRP > 29 則 BRP 無效
				if(    (BRP-BRP_check) != 0
					|| BRP > 29
					|| BRP <= 0)
				{
					// 不支援此 nBaudRate
					return -1;
				}
				else
				{
					nSAM = 1;
				}
			}
			else
			{
				nSAM = 1;
			}
		}
		else
		{
			nSAM = 0;
		}

		// Set BRP
		int dwBRP = (int)(BRP);
		read(fh,&dwDATA,0x06);//pfnGetPortVal(0x0E76, &dwDATA, nByteSize );
		dwDATA = dwDATA & 0xC0;
		dwDATA = dwDATA | dwBRP;
		write(fh,&dwDATA,0x06);//pfnSetPortVal(0x0E76, dwDATA, nByteSize );

		// Set TSEG1, TSEG2
		int dwTSEG = 0x0;
		int dwTSEGtemp1 = (int)(TSEG1) & 0x0F;
		int dwTSEGtemp2 = (int)(TSEG2) & 0x07;
		dwTSEG = dwTSEGtemp1 | (dwTSEGtemp2 << 4);

		// Set SAM
		if( nSAM == 1 )
		{
			dwTSEG = dwTSEG | 0x80;
			write(fh,&dwTSEG,0x07);//pfnSetPortVal(0x0E77, dwTSEG, nByteSize );
		}
		else
		{
			dwTSEG = dwTSEG & 0x7F;
			write(fh,&dwTSEG,0x07);//pfnSetPortVal(0x0E77, dwTSEG, nByteSize );
		}

		return 1;
	


}




// Set the receive filter for a particular receive buffer.
void CAN_Dual_Filter(int fh,can_rx_dual_filter_t *filter)
{
	{
		CAN_Reset(fh);
		//printf("into dual filter\n");
		int ACR1_temp = 0x0;
		int ACR3_temp = 0x0;

		int AMR1_temp = 0x0;
		int AMR3_temp = 0x0;

		// AFM : Acceptance Filter Mode (1:single 0:dual)
		{
			// dual
			int dwDATA = 0x0;
			read(fh,&dwDATA,0x0);//pfnGetPortVal(0x0E70, &dwDATA, nByteSize );
			dwDATA = dwDATA & 0xF7;
			write(fh,&dwDATA,0x0);//pfnSetPortVal(0x0E70, dwDATA, nByteSize );
		}

		// Set Filter 1 ACR
		{
			// ACR0 bit7~bit0, ACR1 bit7~bit5
			int ACR1 = 0x0;
			int ACR0 = 0x0;
			//printf("acr_filter1_id =%x\n",filter->acr_filter1_id);
			ACR1 = (int)(filter->acr_filter1_id << 5);
			ACR0 = (int)(filter->acr_filter1_id >> 3);
			//printf("ACR0 =%x\n",ACR0);
			//printf("ACR1 =%x\n",ACR1);
			// Set RTR (ACR1 bit4)
			if( filter->acr_filter1_rtr != 0 )
				ACR1 = ACR1 | 0x010;
			//printf("ACR0 =%x\n",ACR0);
                        //printf("ACR1 =%x\n",ACR1);

			write(fh,&ACR1,0x11);//pfnSetPortVal(0x0E81, ACR1, nByteSize );
			write(fh,&ACR0,0x10);//pfnSetPortVal(0x0E80, ACR0, nByteSize );

			ACR1_temp = ACR1;
		}

		// Set Filter 1 AMR
		{
			// AMR0 bit7~bit0, AMR1 bit7~bit5
			int AMR1 = 0x0;
			int AMR0 = 0x0;

			AMR1 = (int)(filter->amr_filter1_id << 5);
			AMR0 = (int)(filter->amr_filter1_id >> 3);

			// Set RTR (AMR1 bit4)
			if( filter->amr_filter1_rtr != 0 )
				AMR1 = AMR1 | 0x010;

			write(fh,&AMR1,0x15);//pfnSetPortVal(0x0E85, AMR1, nByteSize );
			write(fh,&AMR0,0x14);//pfnSetPortVal(0x0E84, AMR0, nByteSize );

			AMR1_temp = AMR1;
		}

		// Set Filter 2 ACR
		{
			// ACR2 bit7~bit0, ACR3 bit7~bit5
			int ACR3 = 0x0;
			int ACR2 = 0x0;

			ACR3 = (int)(filter->acr_filter2_id << 5);
			ACR2 = (int)(filter->acr_filter2_id >> 3);

			// Set RTR (ACR3 bit4)
			if( filter->acr_filter2_rtr != 0 )
				ACR3 = ACR3 | 0x010;

			write(fh,&ACR3,0x13);//pfnSetPortVal(0x0E83, ACR3, nByteSize );
			write(fh,&ACR2,0x12);//pfnSetPortVal(0x0E82, ACR2, nByteSize );

			ACR3_temp = ACR3;
		}

		// Set Filter 2 AMR
		{
			// AMR2 bit7~bit0, AMR3 bit7~bit5
			int AMR3 = 0x0;
			int AMR2 = 0x0;

			AMR3 = (int)(filter->amr_filter2_id << 5);
			AMR2 = (int)(filter->amr_filter2_id >> 3);

			// Set RTR (AMR3 bit4)
			if( filter->amr_filter2_rtr != 0 )
				AMR3 = AMR3 | 0x010;

			write(fh,&AMR3,0x17);//pfnSetPortVal(0x0E87, AMR3, nByteSize );
			write(fh,&AMR2,0x16);//pfnSetPortVal(0x0E86, AMR2, nByteSize );

			AMR3_temp = AMR3;
		}

		// Set ACR Data
		{
			// ACR1 bit3~bit0, ACR3 bit3~bit0
			int ACR3 = 0x0;
			int ACR1 = 0x0;

			ACR3 = (int)(filter->acr_data1 & 0x0F);
			ACR1 = (int)(filter->acr_data1 >> 4);

			ACR3 = ACR3 | ACR3_temp;
			ACR1 = ACR1 | ACR1_temp;

			write(fh,&ACR3,0x13);//pfnSetPortVal(0x0E83, ACR3, nByteSize );
			write(fh,&ACR1,0x11);//pfnSetPortVal(0x0E81, ACR1, nByteSize );
		}

		// Set AMR Data
		{
			// AMR1 bit3~bit0, AMR3 bit3~bit0
			int AMR3 = 0x0;
			int AMR1 = 0x0;

			AMR3 = (int)(filter->amr_data1 & 0x0F);
			AMR1 = (int)(filter->amr_data1 >> 4);

			AMR3 = AMR3 | AMR3_temp;
			AMR1 = AMR1 | AMR1_temp;

			write(fh,&AMR3,0x17);//pfnSetPortVal(0x0E87, AMR3, nByteSize );
			write(fh,&AMR1,0x15);//pfnSetPortVal(0x0E85, AMR1, nByteSize );
		}
	}
}

// Set the receive filter for a particular receive buffer.
void CAN_Single_Standard_Filter(int fh,can_rx_single_standard_filter_t *filter)
{
	{
		CAN_Reset(fh);

		// AFM : Acceptance Filter Mode (1:single 0:dual)
		{
			// single
			int dwDATA = 0x0;
			read(fh,&dwDATA,0x00);//pfnGetPortVal(0x0E70, &dwDATA, nByteSize );
			dwDATA = dwDATA | 0x08;
			write(fh,&dwDATA,0x00);//pfnSetPortVal(0x0E70, dwDATA, nByteSize );
		}

		// Set ACR
		{
			// ACR0 bit7~bit0, ACR1 bit7~bit5
			int ACR1 = 0x0;
			int ACR0 = 0x0;

			ACR1 = (int)(filter->acr_id << 5);
			ACR0 = (int)(filter->acr_id >> 3);

			// Set RTR (ACR1 bit4)
			if( filter->acr_rtr != 0 )
				ACR1 = ACR1 | 0x010;

			write(fh,&ACR1,0x11);//pfnSetPortVal(0x0E81, ACR1, nByteSize );
			write(fh,&ACR0,0x10);//pfnSetPortVal(0x0E80, ACR0, nByteSize );
		}

		// Set ACR Data
		{
			// ACR2 bit7~bit0, ACR3 bit7~bit0
			int ACR3 = 0x0;
			int ACR2 = 0x0;

			ACR3 = (int)(filter->acr_data2);
			ACR2 = (int)(filter->acr_data1);

			write(fh,&ACR3,0x13);//pfnSetPortVal(0x0E83, ACR3, nByteSize );
			write(fh,&ACR2,0x12);//pfnSetPortVal(0x0E82, ACR2, nByteSize );
		}

		// Set AMR
		{
			// AMR0 bit7~bit0, AMR1 bit7~bit5
			int AMR1 = 0x0;
			int AMR0 = 0x0;

			AMR1 = (int)(filter->amr_id << 5);
			AMR0 = (int)(filter->amr_id >> 3);

			// Set RTR (AMR1 bit4)
			if( filter->amr_rtr != 0 )
				AMR1 = AMR1 | 0x010;

			write(fh,&AMR1,0x15);//pfnSetPortVal(0x0E85, AMR1, nByteSize );
			write(fh,&AMR0,0x14);//pfnSetPortVal(0x0E84, AMR0, nByteSize );
		}

		// Set AMR Data
		{
			// AMR2 bit7~bit0, AMR3 bit7~bit0
			int AMR3 = 0x0;
			int AMR2 = 0x0;

			AMR3 = (int)(filter->amr_data2);
			AMR2 = (int)(filter->amr_data1);

			write(fh,&AMR3,0x17);//pfnSetPortVal(0x0E87, AMR3, nByteSize );
			write(fh,&AMR2,0x16);//pfnSetPortVal(0x0E86, AMR2, nByteSize );
		}
	}
}

// Set the receive filter for a particular receive buffer.
void CAN_Single_Extended_Filter(int fh,can_rx_single_extended_filter_t *filter)
{
	{
		CAN_Reset(fh);

	// AFM : Acceptance Filter Mode (1:single 0:dual)
		{
			// single
			int dwDATA = 0x0;
			read(fh,&dwDATA,0x00);//pfnGetPortVal(0x0E70, &dwDATA, nByteSize );
			dwDATA = dwDATA | 0x08;
			write(fh,&dwDATA,0x00);//pfnSetPortVal(0x0E70, dwDATA, nByteSize );
		}

		// Set ACR
		{
			// ACR0 bit7~bit0, ACR1 bit7~bit0, ACR2 bit7~bit0, ACR3 bit7~bit3
			int ACR3 = 0x0;
			int ACR2 = 0x0;
			int ACR1 = 0x0;
			int ACR0 = 0x0;

			ACR3 = (int)(filter->acr_id << 3);
			ACR2 = (int)(filter->acr_id >> 5);
			ACR1 = (int)(filter->acr_id >> 13);
			ACR0 = (int)(filter->acr_id >> 21);

			// Set RTR (ACR3 bit2)
			if( filter->acr_rtr != 0 )
				ACR3 = ACR3 | 0x04;

			write(fh,&ACR3,0x13);//pfnSetPortVal(0x0E83, ACR3, nByteSize );
			write(fh,&ACR2,0x12);//pfnSetPortVal(0x0E82, ACR2, nByteSize );
			write(fh,&ACR1,0x11);//pfnSetPortVal(0x0E81, ACR1, nByteSize );
			write(fh,&ACR0,0x10);//pfnSetPortVal(0x0E80, ACR0, nByteSize );
		}

		// Set AMR
		{
			// AMR0 bit7~bit0, AMR1 bit7~bit0, AMR2 bit7~bit0, AMR3 bit7~bit3
			int AMR3 = 0x0;
			int AMR2 = 0x0;
			int AMR1 = 0x0;
			int AMR0 = 0x0;

			AMR3 = (int)(filter->amr_id << 3);
			AMR2 = (int)(filter->amr_id >> 5);
			AMR1 = (int)(filter->amr_id >> 13);
			AMR0 = (int)(filter->amr_id >> 21);

			// Set RTR (AMR3 bit2)
			if( filter->amr_rtr != 0 )
				AMR3 = AMR3 | 0x04;

			write(fh,&AMR3,0x17);//pfnSetPortVal(0x0E87, AMR3, nByteSize );
			write(fh,&AMR2,0x16);//pfnSetPortVal(0x0E86, AMR2, nByteSize );
			write(fh,&AMR1,0x15);//pfnSetPortVal(0x0E85, AMR1, nByteSize );
			write(fh,&AMR0,0x14);//pfnSetPortVal(0x0E84, AMR0, nByteSize );
		}
	}
}





void Check_is_Operation_Mode(int fh)
{
	{
		// 檢查是否為 Operation Mode
		int dwDATA = 0x0;
		int setZero=0x00;
		read(fh,&dwDATA,0x00);//pfnGetPortVal(0x0E70, &dwDATA, nByteSize );
		if( dwDATA & 0x01 )
		{
			// Reset Mode
			//printf("TX RX error conuter Reset.\n");
			write(fh,&setZero,0x0e);//pfnSetPortVal(0x0E7E, 0x00, nByteSize ); // TX error conuter Reset
			write(fh,&setZero,0x0f);//pfnSetPortVal(0x0E7F, 0x00, nByteSize ); // RX error conuter Reset
			//usleep(1000);

			// Operation Mode
			//printf("Enter Operation Mode.\n");
			dwDATA = dwDATA & 0xFE;
			write(fh,&dwDATA,0x00);//pfnSetPortVal(0x0E70, dwDATA, nByteSize ); // Operation Mode
			//usleep(1000);
		}
	}
}


// Abort Transmission.
/*int CAN_Abort_Transmission(int fh)
{
	{
		int dwAT = 0x0;
		read(fh,&dwAT,0x01);//pfnGetPortVal(0x0E71, &dwAT, nByteSize );
		dwAT = dwAT | 0x02;
		write(fh,&dwAT,0x01);//pfnSetPortVal(0x0E71, dwAT, nByteSize );

	}

	return 0;
}*/


// Copy message from user space and transmit.
int CAN_Transmission_when_Idle(int fh,can_msg_t *msg)
{
	{
		// 檢查是否為 Operation Mode
		Check_is_Operation_Mode(fh);

		// 檢查是否有資料傳送中
		// TS 為 1 表示有資料在 TX Buffer 內未傳送出去 (沒接線, 或對方在 Reset mode)
		int dwTS = 0x0;
		read(fh,&dwTS,0x02);//pfnGetPortVal(0x0E72, &dwTS, nByteSize );
		if( dwTS & 0x020 )
		{
#ifdef _DEBUG
			//printf("TS => 1.\n");
#endif
			return -3;
		}

		int dwDATA = 0x0;

		// Extended Frame, Data Length Code bit.
		{
			// Set Extended Frame
			// 0 : SFF; standard frame format will be transmitted by the CAN controller
			// 1 : EFF; extended frame format will be transmitted by the CAN controller
			if( msg->ide == 0 )
				dwDATA = 0x000; // Standard msg
			else
				dwDATA = 0x080; // Extended msg

			// Set RTR message
			// 1 : remote; remote frame will be transmitted by the CAN controller
			// 0 : data; data frame will be transmitted by the CAN controller
			if( msg->rtr != 0 )
				dwDATA = dwDATA | 0x40;

			// Set Data Length Code bit.
			
			int dlc = msg->dlc & 0x0F;
			//printf("The dlc is %d\n",dlc);
			dwDATA = dwDATA | dlc;
			write(fh,&dwDATA,0x10);//pfnSetPortVal(0x0E80, dwDATA, nByteSize );
		}

		// Set ID
		if( msg->ide == 0 )
		{
			// Standard msg
			// ID.28 to ID.18
			int ID_13_20 = 0x0;
			int ID_21_28 = 0x0;

			ID_13_20 = (int)(msg->id << 5);
			ID_21_28 = (int)(msg->id >> 3);

			// Set RTR message
			// 1 : remote; remote frame will be transmitted by the CAN controller
			// 0 : data; data frame will be transmitted by the CAN controller
			if( msg->rtr != 0 )
				ID_13_20 = ID_13_20 | 0x010;

			write(fh,&ID_13_20,0x12);//pfnSetPortVal(0x0E82, ID_13_20, nByteSize );
			write(fh,&ID_21_28,0x11);//pfnSetPortVal(0x0E81, ID_21_28, nByteSize );
		}
		else
		{
			// Extended msg
			// ID.28 to ID.0
			int ID_00_04 = 0x0;
			int ID_05_12 = 0x0;
			int ID_13_20 = 0x0;
			int ID_21_28 = 0x0;

			ID_00_04 = (int)(msg->id << 3);
			ID_05_12 = (int)(msg->id >> 5);
			ID_13_20 = (int)(msg->id >> 13);
			ID_21_28 = (int)(msg->id >> 21);

			// Set RTR message
			// 1 : remote; remote frame will be transmitted by the CAN controller
			// 0 : data; data frame will be transmitted by the CAN controller
			if( msg->rtr != 0 )
				ID_00_04 = ID_00_04 | 0x04;

			write(fh,&ID_00_04,0x14);//pfnSetPortVal(0x0E84, ID_00_04, nByteSize );
			write(fh,&ID_05_12,0x13);//pfnSetPortVal(0x0E83, ID_05_12, nByteSize );
			write(fh,&ID_13_20,0x12);//pfnSetPortVal(0x0E82, ID_13_20, nByteSize );
			write(fh,&ID_21_28,0x11);//pfnSetPortVal(0x0E81, ID_21_28, nByteSize );
		}

		// Set Data
		if( msg->ide == 0 )
		{
			// Standard msg
			// 8-Bytes DATA
			int TX_DATA_1 = msg->data[0];
			int TX_DATA_2 = msg->data[1];
			int TX_DATA_3 = msg->data[2];
			int TX_DATA_4 = msg->data[3];
			int TX_DATA_5 = msg->data[4];
			int TX_DATA_6 = msg->data[5];
			int TX_DATA_7 = msg->data[6];
			int TX_DATA_8 = msg->data[7];

			write(fh,&TX_DATA_1,0x13);//pfnSetPortVal(0x0E83, TX_DATA_1, nByteSize );
			write(fh,&TX_DATA_2,0x14);//pfnSetPortVal(0x0E84, TX_DATA_2, nByteSize );
			write(fh,&TX_DATA_3,0x15);//pfnSetPortVal(0x0E85, TX_DATA_3, nByteSize );
			write(fh,&TX_DATA_4,0x16);//pfnSetPortVal(0x0E86, TX_DATA_4, nByteSize );
			write(fh,&TX_DATA_5,0x17);//pfnSetPortVal(0x0E87, TX_DATA_5, nByteSize );
			write(fh,&TX_DATA_6,0x18);//pfnSetPortVal(0x0E88, TX_DATA_6, nByteSize );
			write(fh,&TX_DATA_7,0x19);//pfnSetPortVal(0x0E89, TX_DATA_7, nByteSize );
			write(fh,&TX_DATA_8,0x1a);//pfnSetPortVal(0x0E8A, TX_DATA_8, nByteSize );
		}
		else
		{
			// Extended msg
			// 8-Bytes DATA
			int TX_DATA_1 = msg->data[0];
			int TX_DATA_2 = msg->data[1];
			int TX_DATA_3 = msg->data[2];
			int TX_DATA_4 = msg->data[3];
			int TX_DATA_5 = msg->data[4];
			int TX_DATA_6 = msg->data[5];
			int TX_DATA_7 = msg->data[6];
			int TX_DATA_8 = msg->data[7];

			write(fh,&TX_DATA_1,0x15);//pfnSetPortVal(0x0E85, TX_DATA_1, nByteSize );
			write(fh,&TX_DATA_2,0x16);//pfnSetPortVal(0x0E86, TX_DATA_2, nByteSize );
			write(fh,&TX_DATA_3,0x17);//pfnSetPortVal(0x0E87, TX_DATA_3, nByteSize );
			write(fh,&TX_DATA_4,0x18);//pfnSetPortVal(0x0E88, TX_DATA_4, nByteSize );
			write(fh,&TX_DATA_5,0x19);//pfnSetPortVal(0x0E89, TX_DATA_5, nByteSize );
			write(fh,&TX_DATA_6,0x1a);//pfnSetPortVal(0x0E8A, TX_DATA_6, nByteSize );
			write(fh,&TX_DATA_7,0x1b);//pfnSetPortVal(0x0E8B, TX_DATA_7, nByteSize );
			write(fh,&TX_DATA_8,0x1c);//pfnSetPortVal(0x0E8C, TX_DATA_8, nByteSize );
		}

		// Transmission Request

		{
			read(fh,&dwDATA,0x01);//pfnGetPortVal(0x0E71, &dwDATA, nByteSize );
			dwDATA = dwDATA | 0x01;
			write(fh,&dwDATA,0x01);//pfnSetPortVal(0x0E71, dwDATA, nByteSize );
		}
//		printf("Write message\n");
		//sleep(1);
		int i=0;
		for (i ; i<1000 ; i++ )
		{
			int dwIR = 0x0;
			read(fh,&dwIR,0x03);//pfnGetPortVal(0x0E73, &dwIR, nByteSize ); 
#ifdef _DEBUG
                        //printf("The data of 0x03 is %x   ",dwIR);

#endif

			if( dwIR & 0x80 )
			{
				// 有 Error
				int error;
#ifdef _DEBUG
				//printf("Transmit Error!!!\n");
#endif
				read(fh,&error,0x0c);//pfnGetPortVal(0x0E73, &dwIR, nByteSize ); 
				return error;
			}

			if( dwIR & 0x02 )
			{
				// 傳送中
#ifdef _DEBUG  	
				//printf("Transmit ing\n");
#endif
				//sleep(1);
				continue;
			}

			int dwSR = 0x0;
			read(fh,&dwSR,0x02);//pfnGetPortVal(0x0E72, &dwSR, nByteSize );
			/*if ( dwSR & 0x02 )
			{
#ifdef _DEBUG 
				//printf("Full of RX Buffer!!!\n");
				// 對方 RX Buffer 已滿
#endif
				return -2;
			}*/

			if ( dwSR & 0x0c )
			{
				// 傳送成?
#ifdef _DEBUG				
				//printf("The data no error!\n");
#endif
				return -1;
			}
		}
	}

	return -3;
}

// Read and copy CAN messages to user space.
/*int CAN_Receive(int fh,can_msg_t *msg)
{
	
		// 檢查是否為 Operation Mode
		Check_is_Operation_Mode(fh);
		int dwRBS = 0x0;
		read(fh,&dwRBS,0x02);//pfnGetPortVal(0x0E72, &dwRBS, nByteSize );
		if( dwRBS & 0x01 )
		{
			// 有資料
			printf("recieve ready\n");
			// 取出 RBSA 
			int dwRBSA = 0x0;
			read(fh,&dwRBSA,0x1d);//pfnGetPortVal(0x0E8E, &dwRBSA, nByteSize );
			 printf("RMC is %x\n",dwRBSA);
			 
                        read(fh,&dwRBSA,0x1e);//pfnGetPortVal(0x0E8E, &dwRBSA, nByteSize );
                         printf("dwRBSA is %x\n",dwRBSA);

			// 取出資料
			int dwDATA = 0x0;

			// Extended Frame, Data Length Code bit.
			{
				read(fh,&dwDATA,0x10);//pfnGetPortVal(0x0E80, &dwDATA, nByteSize );

				// Read Extended Frame
				// 0 : SFF; standard frame format will be transmitted by the CAN controller
				// 1 : EFF; extended frame format will be transmitted by the CAN controller
				if( dwDATA & 0x080 )
					msg->ide = 1; // Extended msg
				else
					msg->ide = 0; // Standard msg

				// Read RTR message
				// 1 : remote; remote frame will be transmitted by the CAN controller
				// 0 : data; data frame will be transmitted by the CAN controller
				if( dwDATA & 0x040 )
					msg->rtr = 1;
				else
					msg->rtr = 0;

				// Read Data Length Code bit.
				msg->dlc = dwDATA & 0x0F;
			}

			// Read ID
			if( msg->ide == 0 )
			{
				// Standard msg
				// ID.28 to ID.18
				int ID_13_20 = 0x0;
				int ID_21_28 = 0x0;

				read(fh,&ID_13_20,0x12);//pfnGetPortVal(0x0E82, &ID_13_20, nByteSize );
				read(fh,&ID_21_28,0X11);//pfnGetPortVal(0x0E81, &ID_21_28, nByteSize );

				msg->id = 0;
				msg->id = msg->id | (ID_13_20 >> 5);
				msg->id = msg->id | (ID_21_28 << 3);
	
				// RTR
				if( ID_13_20 & 0x010 )
	//				msg->rtr = 1;
	//			else
	//				msg->rtr = 0;
	
			}
			else
			{
				// Extended msg
				// ID.28 to ID.0
				int ID_00_04 = 0x0;
				int ID_05_12 = 0x0;
				int ID_13_20 = 0x0;
				int ID_21_28 = 0x0;

				read(fh,&ID_00_04,0x14);//pfnGetPortVal(0x0E84, &ID_00_04, nByteSize );
				read(fh,&ID_05_12,0x13);//pfnGetPortVal(0x0E83, &ID_05_12, nByteSize );
				read(fh,&ID_13_20,0x12);//pfnGetPortVal(0x0E82, &ID_13_20, nByteSize );
				read(fh,&ID_21_28,0x11);//pfnGetPortVal(0x0E81, &ID_21_28, nByteSize );

				msg->id = 0;
				msg->id = msg->id | (ID_00_04 >> 3);
				msg->id = msg->id | (ID_05_12 << 5);
				msg->id = msg->id | (ID_13_20 << 13);
				msg->id = msg->id | (ID_21_28 << 21);
	
				// RTR
	//			if( ID_00_04 & 0x04 )
	//				msg->rtr = 1;
	//			else
	//				msg->rtr = 0;
	
			}

			// Read Data
			if( msg->ide == 0 )
			{
				// Standard msg
				// 8-Bytes DATA
				int dwCanData;
				read(fh,&dwCanData,0x13);//pfnGetPortVal(0x0E83, &dwCanData, nByteSize );
				msg->data[0] = (int)(dwCanData);
				read(fh,&dwCanData,0x14);//pfnGetPortVal(0x0E84, &dwCanData, nByteSize );
				msg->data[1] = (int)(dwCanData);
				read(fh,&dwCanData,0x15);//pfnGetPortVal(0x0E85, &dwCanData, nByteSize );
				msg->data[2] = (int)(dwCanData);
				read(fh,&dwCanData,0x16);//pfnGetPortVal(0x0E86, &dwCanData, nByteSize );
				msg->data[3] = (int)(dwCanData);
				read(fh,&dwCanData,0x17);//pfnGetPortVal(0x0E87, &dwCanData, nByteSize );
				msg->data[4] = (int)(dwCanData);
				read(fh,&dwCanData,0x18);//pfnGetPortVal(0x0E88, &dwCanData, nByteSize );
				msg->data[5] = (int)(dwCanData);
				read(fh,&dwCanData,0x19);//pfnGetPortVal(0x0E89, &dwCanData, nByteSize );
				msg->data[6] = (int)(dwCanData);
				read(fh,&dwCanData,0x1a);//pfnGetPortVal(0x0E8A, &dwCanData, nByteSize );
				msg->data[7] = (int)(dwCanData);
			}
			else
			{
				// Extended msg
				// 8-Bytes DATA
				int dwCanData;
				read(fh,&dwCanData,0x15);//pfnGetPortVal(0x0E85, &dwCanData, nByteSize );
				msg->data[0] = (int)(dwCanData);
				read(fh,&dwCanData,0x16);//pfnGetPortVal(0x0E86, &dwCanData, nByteSize );
				msg->data[1] = (int)(dwCanData);
				read(fh,&dwCanData,0x17);//pfnGetPortVal(0x0E87, &dwCanData, nByteSize );
				msg->data[2] = (int)(dwCanData);
				read(fh,&dwCanData,0x18);//pfnGetPortVal(0x0E88, &dwCanData, nByteSize );
				msg->data[3] = (int)(dwCanData);
				read(fh,&dwCanData,0x19);//pfnGetPortVal(0x0E89, &dwCanData, nByteSize );
				msg->data[4] = (int)(dwCanData);
				read(fh,&dwCanData,0x1a);//pfnGetPortVal(0x0E8A, &dwCanData, nByteSize );
				msg->data[5] = (int)(dwCanData);
				read(fh,&dwCanData,0x1b);//pfnGetPortVal(0x0E8B, &dwCanData, nByteSize );
				msg->data[6] = (int)(dwCanData);
				read(fh,&dwCanData,0x1c);//pfnGetPortVal(0x0E8C, &dwCanData, nByteSize );
				msg->data[7] = (int)(dwCanData);
			}

			// 資料讀完, 需清除 RX Buffer
			read(fh,&dwDATA,0x01);//pfnGetPortVal(0x0E71, &dwDATA, nByteSize );
			dwDATA = dwDATA | 0x04;
			write(fh,&dwDATA,0x01);//pfnSetPortVal(0x0E71, dwDATA, nByteSize );

			//sleep(1);

			int bTimeOut = 0xff;

			// 接著等待 RBSA 改變
			// 0E8E 的 RX buffer start address 變為下一個位址
			int i=0;
			for ( i ; i<1000 ; i++ )
			{
				int dwRBSA_Next = 0x0;
				
                                read(fh,&dwRBSA_Next,0x1d);//pfnGetPortVal(0x0E8E, &dwRBSA_Next, nByteSize );
                                printf("PMC_next is %x\n",dwRBSA_Next);
				read(fh,&dwRBSA_Next,0x1e);//pfnGetPortVal(0x0E8E, &dwRBSA_Next, nByteSize );
				printf("dwRBSA_next is %x\n",dwRBSA_Next);
				if( dwRBSA != dwRBSA_Next )
				{
					bTimeOut = 0x00;
					break;
				}

				//sleep(1);
			}

//#ifdef _DEBUG
			if( bTimeOut ){
				printf("Time out!!!\n");
				return 0;
			}
			else{
				printf("finish recieve data!!!\n");
				return 1;
			}
//#endif 
		}
	
	return 0;

}
*/





typedef struct can_error 
{
	unsigned int Direction;                     // 0 TX; error occurred during transmission
	                                            // 1 RX; error occurred during reception
	unsigned int error_stat;                    // 
	                                            // 00 = bit error
	                                            // 01 = form error
	                                            // 02 = stuff error
	                                            // 03 = other type of error
	unsigned int rx_err_cnt;					// Rx counter
	unsigned int tx_err_cnt;					// Tx counter
} can_error_t;

int CAN_Error_Stats_Get(int fh,can_error_t *can_error)
{
	
	int dwECC = 0x0;
	read(fh,&dwECC,0x0c);//pfnGetPortVal(0x0E7C, &dwECC, nByteSize );
	if( dwECC & 0x020 )
		can_error->Direction = 1;
	else
		can_error->Direction = 0;
	int dwERRC = dwECC >> 6;
	can_error->error_stat = dwERRC;

	int dwRXERR = 0x0;
	read(fh,&dwRXERR,0x0e);//pfnGetPortVal(0x0E7E, &dwRXERR, nByteSize );
	can_error->rx_err_cnt = dwRXERR;
	int dwTXERR = 0x0;
	read(fh,&dwTXERR,0x0f);//pfnGetPortVal(0x0E7F, &dwTXERR, nByteSize );
	can_error->tx_err_cnt = dwTXERR;

	return 1;

}


void Automatic_Transmit_CAN_message()
{
	int nID = 0x0;
	while ( 1 )
	{
		can_msg_t Message;
		Message.ide     = 0;
		Message.id      = nID;
		Message.dlc     = 8;
		Message.data[0] = 0x01;
		Message.data[1] = 0x02;
		Message.data[2] = 0x03;
		Message.data[3] = 0x04;
		Message.data[4] = 0x05;
		Message.data[5] = 0x06;
		Message.data[6] = 0x07;
		Message.data[7] = 0x08;
		Message.rtr     = 0;

		int nResult = CAN_Transmission(&Message);
		if( nResult >= 0  )
		{
			printf("Send => IDe(%d), ID(%d), Len(%d), Data1~8(0x%x)(0x%x)(0x%x)(0x%x)(0x%x)(0x%x)(0x%x)(0x%x), RTR(%d)\n", Message.ide, Message.id, Message.dlc, Message.data[0], Message.data[1], Message.data[2], Message.data[3], Message.data[4], Message.data[5], Message.data[6], Message.data[7], Message.rtr);
			nID++;
		}
		else
		{
			//printf("Send CAN Message failed!!!\n");
			if (nResult < 0){}
				//printf("The error code - is %d\n",nResult);
			//else
				//printf("The error code is %x\n",nResult);
		}
		//printf("There are %d data in the buffer.\n",point_write);
		sleep(1);
	}
}

void Receive_CAN_message(fh)
{
	while ( 1 )
	{
		can_msg_t Message;
		if( CAN_Receive_nonblock(&Message) > 0 )
		{
			printf("==========qqqqqCAN_Message==========\n");
			printf("IDe : %d\n",      Message.ide);
			printf("ID : %d(0x%x)\n",       Message.id, Message.id);
			printf("DataLen : %d\n",  Message.dlc);
			int i=0;
			for ( i ; i<8 && i< Message.dlc ; i++ )
			{
				printf("Data %d : 0x%x\n", i+1, Message.data[i]);
			}

			printf("RTR : %d\n",      Message.rtr);
			printf("\n");
		}

		//sleep(1);
	}
}

void Edit_CAN_message_and_Transmit(int fh)
{
	//while(1)
	{
		printf("\n");
		printf("Edit CAM message.\n");

		can_msg_t Message;

		printf("IDe : ");
		scanf(" %d", &Message.ide);

		printf("ID : 0x");
		scanf(" %x", &Message.id);

		printf("DataLen : ");
		scanf(" %d", &Message.dlc);
		int i=0;
		for( i ; i<8 && i<Message.dlc ; i++)
		{
			printf("Data[%x] : 0x", i);
			scanf("%d", &Message.data[i]);
		}

		printf("RTR : ");
		scanf(" %d", &Message.rtr);

		int nResult = CAN_Transmission(&Message);
		if( nResult == -1 )
			printf("Send => IDe(%d), ID(%d), Len(%d), Data1~8(0x%x)(0x%x)(0x%x)(0x%x)(0x%x)(0x%x)(0x%x)(0x%x), RTR(%d)\n", Message.ide, Message.id, Message.dlc, Message.data[0], Message.data[1], Message.data[2], Message.data[3], Message.data[4], Message.data[5], Message.data[6], Message.data[7], Message.rtr);
		else 
			printf("Send CAN Message failed!!!\n");
	}
}

int  CAN_Initial(int fh,int nBaudRate)
{
	{
		CAN_Reset(fh);

		int dwDATA = 0,reg=0;
		dwDATA=0xDB;
		write(fh,&dwDATA,0x08);//pfnSetPortVal(0x0E78, 0xDB, nByteSize ); // Push-Pull Output
		dwDATA=0xc8;
		printf ("set 0x1f to 0xc8\n");
		reg=31;
                write(fh,&dwDATA,reg);//pfnSetPortVal(0x0E8F, 0xC8, nByteSize ); // Extend Mode, Clock out off
		dwDATA=0x96;
                write(fh,&dwDATA,0x0d);//pfnSetPortVal(0x0E7D, 0x96, nByteSize ); // 
		dwDATA=0x00;
                write(fh,&dwDATA,0x04);//pfnSetPortVal(0x0E74, 0xFF, nByteSize ); // 中斷全啟用
		dwDATA=0x00;
                write(fh,&dwDATA,0x0e);//pfnSetPortVal(0x0E7E, 0x00, nByteSize ); // RX error conuter Reset
		dwDATA=0x00;
                write(fh,&dwDATA,0x0f);//pfnSetPortVal(0x0E7F, 0x00, nByteSize ); // TX error conuter Reset

		return CAN_SetBaud(fh,nBaudRate);
	}

	
}
