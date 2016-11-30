#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
//#include <curses.h>
#include <sys/ioctl.h>
#include <termios.h>
#include "SJA1000_CAN.h" 
//****************************************************************
void START(fh){
	int res;
	printf("fh value in main is %d",fh);
	res = CAN_Run(fh);
	if (res!=0)
		printf("CAN Run is ok\n");
	else 
		printf("CAN Run is fail.\n");
}
//******************************************************************
void Receive_message(){
	int res;
	printf("Enter to Receive message.\n");
	can_msg_t Message;
	do {
	 	printf("Enter to while loop of Receive message.\n");
		res = CAN_Receive_nonblock(&Message);
		if (res == -1)
			printf("The receive buffer is empty.\n");
		else {
			printf("Receive => IDe(%d), ID(0x%x), Len(%d), Data1~8(0x%x)(0x%x)(0x%x)(0x%x)(0x%x)(0x%x)(0x%x)(0x%x), RTR(%d)\n", Message.ide, Message.id, Message.dlc, Message.data[0], Message.data[1], Message.data[2], Message.data[3], Message.data[4], Message.data[5], Message.data[6], Message.data[7], Message.rtr);
		 	printf("There are %d CAN message in the buffer.\n",res);
		}

	} while( res > 0);
}
//*********************************************************************
void STOP(){
        int res;
        res = CAN_Stop();
        if (res!=0)
                printf("CAN Run is ok\n");
        else
                printf("CAN Run is fail.\n");
}
//***********************************************************************
void Set_BitRate(int fh){
	int BaudRate=0;
	printf("Please key in baudrate (ex:125000):");
        scanf(" %d", &BaudRate);
	CAN_Reset(fh);
	CAN_Initial(fh,BaudRate);
}
//********************************************************************
void Edit_send_message(){

        printf("Edit CAM message.\n");
        can_msg_t Message;
	int ide,rtr,dlc,data;
        printf("IDe (0:standard message(11bits) ; 1:extend message (29bits)) : ");
        scanf(" %d", &ide);
	Message.ide= (short unsigned int) ide;
        printf("Please key in message ID (11 bit 0r 29bit) 0x");
        scanf(" %x", &Message.id);	
	printf("Please key in frame type (remote :1 and data:0):");
        scanf(" %d", &rtr);
	Message.rtr = (short unsigned int) rtr;
	if (Message.rtr==0){
        	printf("Please key in Data Length (0~8) : ");
        	scanf(" %d", &dlc);
		Message.dlc= (short unsigned int) dlc;
        	int i=0;
        	for( i ; i<8 && i<Message.dlc ; i++)
        	{
                 	printf("Data[%x] : 0x", i);
                  	scanf(" %x", &data);
			Message.data[i]=(unsigned char) data;
        	}
	}
        int nResult = CAN_Transmission(&Message);
        if( nResult == -1 )
		printf("The Transmit Buffer is full.\n");
	else{
        	printf("Send => IDe(%d), ID(%d), Len(%d), Data1~8(0x%x)(0x%x)(0x%x)(0x%x)(0x%x)(0x%x)(0x%x)(0x%x), RTR(%d)\n", Message.ide, Message.id, Message.dlc, Message.data[0], Message.data[1], Message.data[2], Message.data[3], Message.data[4], Message.data[5], Message.data[6], Message.data[7], Message.rtr);
        	printf("There are %d Transmit message on Buffer\n",nResult);
        }
}
//**********************************************************************

int main(int argc ,char *argv[])
{
//******************* initial CAN driver, then get DEVICE ID on "fh"****		
		int nRetCode = 0,fh=0;
        	fh = open("/dev/NEXCOM_CAN1", O_RDWR);
		if(fh >=0 ) {
               	 	printf("CAN driver operation is normal\n");
        	}
        	else {
                	printf("CAN driver cannot open.\n");
                return -1;
        	}	
//************************************************************************
		system("clear");
//************************ initial CAN device (SJA1000)******************	 	
		
		Clear_Buffer(fh); // clear CAN R/T buffer
		int nResult = CAN_Initial(fh,CAN_BAUD_62500);// You can set baudrate what you want.
		if ( nResult == 1 )
			printf("Initial success.\n");
		else
		{
			printf("Initial false!!!\n");
			//system("PAUSE");
			return -1;
		}
//*************************************************************************
//************************* Set filter content to SJA1000 *****************
		int nFilterMode = 0;
		printf("=== Set filter mode ====\n");
		printf("Filter mode (0:Dual 1:Single 2:Don't care) : ");
		scanf("%d", &nFilterMode );

		if( nFilterMode == 0 )
		{ // Dual mode

			printf("\n");
			printf("===Dual filter configuration===\n");

			can_rx_dual_filter_t dual_filter;

			printf("Filter 1 ACR ID (11bit) : 0x");
			scanf("%x", &dual_filter.acr_filter1_id );
			printf("Filter 1 ACR RTR (0 or 1) : ");
			scanf("%d", &dual_filter.acr_filter1_rtr );

			printf("Filter 1 AMR ID (11bit) : 0x");
			scanf("%x", &dual_filter.amr_filter1_id );
			printf("Filter 1 AMR RTR (0 or 1) : ");
			scanf("%d", &dual_filter.amr_filter1_rtr );

			printf("Filter 2 ACR ID (11bit) : 0x");
			scanf("%x", &dual_filter.acr_filter2_id );
			printf("Filter 2 ACR RTR (0 or 1) : ");
			scanf("%d", &dual_filter.acr_filter2_rtr );

			printf("Filter 2 AMR ID (11bit) : 0x");
			scanf("%x", &dual_filter.amr_filter2_id );
			printf("Filter 2 AMR RTR (0 or 1) : ");
			scanf("%d", &dual_filter.amr_filter2_rtr );

			printf("Filter data 1 ACR (8bit) : 0x");
			scanf("%x", &dual_filter.acr_data1 );
			printf("Filter data 1 AMR (8bit) : 0x");
			scanf("%x", &dual_filter.amr_data1 );

			CAN_Dual_Filter(fh,&dual_filter);
		}
		else if( nFilterMode == 1 )
		{ // Single mode
			int nID_Mode = 0;
			printf("ID? (0:Standard 1:Extended) : ");
			scanf("%d", &nID_Mode );

			if( nID_Mode == 0 )
			{ // Standard

				printf("\n");
				printf("===Single Standard filter configuration===\n");

				can_rx_single_standard_filter_t single_standard_filter;

				printf("ACR ID (11bit) : 0x");
				scanf("%x", &single_standard_filter.acr_id );
				printf("ACR RTR (0 or 1) : ");
				scanf("%d", &single_standard_filter.acr_rtr );

				printf("AMR ID (11bit) : 0x");
				scanf("%x", &single_standard_filter.amr_id );
				printf("AMR RTR (0 or 1) : ");
				scanf("%d", &single_standard_filter.amr_rtr );

				printf("Data 1 ACR (8bit) : 0x");
				scanf("%x", &single_standard_filter.acr_data1);
				printf("Data 1 AMR (8bit) : 0x");
				scanf("%x", &single_standard_filter.amr_data1);

				printf("Data 2 ACR (8bit) : 0x");
				scanf("%x", &single_standard_filter.acr_data2);
				printf("Data 2 AMR (8bit) : 0x");
				scanf("%x", &single_standard_filter.amr_data2);

				CAN_Single_Standard_Filter(fh,&single_standard_filter);
			}
			else
			{ // Extended

				printf("\n");
				printf("===Single Extended filter configuration===\n");

				can_rx_single_extended_filter_t single_extended_filter;

				printf("ACR ID (29bit) : 0x");
				scanf("%x", &single_extended_filter.acr_id );
				printf("The filter.acr_id is %x\n",single_extended_filter.acr_id);
				printf("ACR RTR (0 or 1) : ");
				scanf("%d", &single_extended_filter.acr_rtr );

				printf("AMR ID (29bit) : 0x");
				scanf("%x", &single_extended_filter.amr_id );
				printf("AMR RTR (0 or 1) : ");
				scanf("%d", &single_extended_filter.amr_rtr );

				CAN_Single_Extended_Filter(fh,&single_extended_filter);
			}
		}
		else {

                        can_rx_dual_filter_t dual_filter;

                        dual_filter.amr_filter1_id =0xffff;
                        dual_filter.amr_filter1_rtr=0xffff;

                        dual_filter.amr_filter2_id=0xffff; 
                        dual_filter.amr_filter2_rtr=0xffff;

                        dual_filter.amr_data1=0xffff;

                        CAN_Dual_Filter(fh,&dual_filter);

		}

//********************************************************************************************

//******************** Demo program for CAN bus function (receive & Transmit CAN message)*****

		
		while(1)
		{
			printf("Philips SJA1000 CAN Bus\n");
			printf("========================================================\n");
			printf("1. Start :Please set CAN bus run at first time.\n");
			printf("2. Automatic send CAN message.\n");
			printf("3. Receive CAN message.\n");
			printf("4. Edit CAN message and send.\n");
			printf("5. Abort current Transmission.\n");
			printf("6. Change bit Rate of CAN.\n");
			printf("7. STOP CAN :Please stop CAN bus when you exit this program.\n");
  			printf("8. Exit \n");
			printf("========================================================\n");
			printf(">>");
		//	Abort_Transmission(fh);
			char nChar = 0;
			//nChar = getchar();
			scanf(" %c",&nChar);
			system("clear");
			switch(nChar)
			{
			case '1':
                                START(fh);
                                break;
			case '2':
				Automatic_Transmit_CAN_message();
				break;
			case '3':
				Receive_message();
				break;
			case '4':
				//Edit_CAN_message_and_Transmit(fh);
				Edit_send_message();
				break;
			case '5':
                                Abort_Transmission(fh);
                                break;

			case '6':
                                Set_BitRate(fh);
                                break;
			
			case '7':
                                STOP(fh);
                                break;

			case '8':
				CAN_Reset(fh);
                                return 1;
                                break;

			default :
				printf("Wrong selection!!\n ");
				break;
			}
		
		}

		STOP(fh);
		return nRetCode;
}
