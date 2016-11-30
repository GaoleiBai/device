typedef enum
{
        CAN_BAUD_12500  = 12500,                    // 12.5  kbps
        CAN_BAUD_12800  = 12800,                                        // 12.8  kbps
        CAN_BAUD_16000  = 16000,                                        // 16    kbps
        CAN_BAUD_20000  = 20000,                                        // 20    kbps
        CAN_BAUD_25000  = 25000,                    // 25    kbps
        CAN_BAUD_31250  = 31250,                    // 31.25 kbps
        CAN_BAUD_32000  = 32000,                                        // 32    kbps
        CAN_BAUD_40000  = 40000,                                        // 40    kbps
        CAN_BAUD_50000  = 50000,                    // 50    kbps
        CAN_BAUD_62500  = 62500,                    // 62.5  kbps
        CAN_BAUD_64000  = 64000,                                        // 64    kbps
        CAN_BAUD_80000  = 80000,                                        // 80    kbps
        CAN_BAUD_100000 = 100000,                   // 100   kbps
        CAN_BAUD_125000 = 125000,                   // 125   kbps
        CAN_BAUD_160000 = 160000,                                       // 160   kbps
        CAN_BAUD_200000 = 200000,                   // 200   kbps
        CAN_BAUD_250000 = 250000,                   // 250   kbps
        CAN_BAUD_400000 = 400000,                   // 400   kbps
        CAN_BAUD_500000 = 500000,                   // 500   kbps
        CAN_BAUD_1000000 = 1000000,                 // 1000  kbps
} can_baud_t;

typedef struct can_error
{
        unsigned int Direction;                     // 0 TX; error occurred during transmission
                                                    // 1 RX; error occurred during reception
        unsigned int error_stat;                    // 
                                                    // 00 = bit error
                                                    // 01 = form error
                                                    // 02 = stuff error
                                                    // 03 = other type of error
        unsigned int rx_err_cnt;                                        // Rx counter
        unsigned int tx_err_cnt;                                        // Tx counter
} can_error_t;

typedef struct can_msg
{
        unsigned short ide;                                                     // Standard/extended msg
        unsigned int id;                                                        // 11 or 29 bit msg id
        unsigned short dlc;                                                     // Size of data
        unsigned char data[8];           // Message pay load
        unsigned short rtr;                                                     // RTR message
} can_msg_t;

typedef struct can_rx_dual_filter
{
        unsigned int  acr_filter1_id;           // ID    (11bit)
        unsigned int  acr_filter1_rtr;          // RTR   (1bit)

        unsigned int  amr_filter1_id;           // ID    (11bit)
        unsigned int  amr_filter1_rtr;          // RTR   (1bit)

        unsigned int  acr_filter2_id;           // ID    (11bit)
        unsigned int  acr_filter2_rtr;          // RTR   (1bit)

        unsigned int  amr_filter2_id;           // ID    (11bit)
        unsigned int  amr_filter2_rtr;          // RTR   (1bit)

        unsigned int  acr_data1;                        // Data1 (8bit)
        unsigned int  amr_data1;                        // Data1 (8bit)

} can_rx_dual_filter_t;

typedef struct can_rx_single_standard_filter
{
        unsigned int acr_id;            // ID    (11bit)
        unsigned int acr_rtr;           // RTR   (1bit)
        unsigned int acr_data1;         // Data1 (8bit)
        unsigned int acr_data2;         // Data2 (8bit)

        unsigned int amr_id;            // ID    (11bit)
        unsigned int amr_rtr;           // RTR   (1bit)
        unsigned int amr_data1;         // Data1 (8bit)
        unsigned int amr_data2;         // Data2 (8bit)

} can_rx_single_standard_filter_t;

typedef struct can_rx_single_extended_filter
{
        unsigned int acr_id;            // ID    (29bit)
        unsigned int acr_rtr;           // RTR   (1bit)

        unsigned int amr_id;            // ID    (29bit)
        unsigned int amr_rtr;           // RTR   (1bit)

} can_rx_single_extended_filter_t;

void CAN_Reset(int);
int  CAN_SetBaud(int ,int );
void CAN_Dual_Filter(int ,can_rx_dual_filter_t *);
void CAN_Single_Standard_Filter(int,can_rx_single_standard_filter_t *);
void CAN_Single_Extended_Filter(int,can_rx_single_extended_filter_t *);
void Abort_Transmission(int);
int  CAN_Transmission(can_msg_t *);
int  CAN_Receive_nonblock(can_msg_t *);
int  CAN_Error_Stats_Get(int,can_error_t *);
void Automatic_Transmit_CAN_message();
int  CAN_Initial(int,int);
int  CAN_Run(int);
int  CAN_Stop();
void Clear_Buffer(int);
