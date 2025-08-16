#include "mcc_generated_files/mcc.h"

// NRF24L01 Commands
#define R_RX_PAYLOAD        0x61
#define FLUSH_RX            0xE2
#define W_REGISTER          0x20
#define R_REGISTER          0x00

// Registers
#define CONFIG              0x00
#define EN_AA               0x01
#define RF_CH               0x05
#define RF_SETUP            0x06
#define STATUS              0x07
#define RX_ADDR_P0          0x0A
#define SETUP_AW            0x03
#define RX_PW_P0            0x11
////////////////////////////////////////
#define SETUP_RETR_REG      (uint8_t)0x04
/*Re-TX delay of 750 us*/
#define ARC             (uint8_t)0x2A
#define RF_PWR          (uint8_t)6


// Macros
#define NRF_CE_HIGH()      LATCbits.LATC5 = 1
#define NRF_CE_LOW()       LATCbits.LATC5 = 0
#define NRF_CSN_HIGH()     LATAbits.LATA4 = 1
#define NRF_CSN_LOW()      LATAbits.LATA4 = 0

uint8_t NRF_SPI(uint8_t data) {
    return SPI1_ExchangeByte(data);
}

void NRF_WriteRegister(uint8_t reg, uint8_t value) {
    NRF_CSN_LOW();
    NRF_SPI(W_REGISTER | (reg & 0x1F));
    NRF_SPI(value);
    NRF_CSN_HIGH();
}

void NRF_WriteRegisterMulti(uint8_t reg, uint8_t *data, uint8_t len) {
    NRF_CSN_LOW();
    NRF_SPI(W_REGISTER | (reg & 0x1F));
    for (uint8_t i = 0; i < len; i++)
        NRF_SPI(data[i]);
    NRF_CSN_HIGH();
}

uint8_t NRF_ReadRegister(uint8_t reg) {
    NRF_CSN_LOW();
    NRF_SPI(R_REGISTER | (reg & 0x1F));
    uint8_t result = NRF_SPI(0xFF);
    NRF_CSN_HIGH();
    return result;
}

void NRF_ReadPayload(uint8_t *data, uint8_t len) {
    NRF_CSN_LOW();
    NRF_SPI(R_RX_PAYLOAD);
    for (uint8_t i = 0; i < len; i++)
        data[i] = NRF_SPI(0xFF);
    NRF_CSN_HIGH();

    // Flush RX just in case
    NRF_CSN_LOW();
    NRF_SPI(FLUSH_RX);
    NRF_CSN_HIGH();
}

void NRF_ClearStatus() {
    NRF_WriteRegister(STATUS, 0x70); // Clear RX_DR, TX_DS, MAX_RT
}

void NRF_RX_Init() {
    __delay_ms(100);
    uint8_t rx_address[5] = {'A', 'A', 'A', 'A', 'A'};

    NRF_CE_LOW();

    NRF_WriteRegister(CONFIG, 0x0B);       // PWR_UP, PRIM_RX, CRC enabled  //try  0X0F
    NRF_WriteRegister(EN_AA, 0x01);        // Enable Auto-ACK on pipe 0
    NRF_WriteRegister(RF_CH, 0x02);        // Channel 2
    NRF_WriteRegister(RF_SETUP, 0x06);     // 1Mbps, 0dBm
    NRF_WriteRegister(SETUP_AW, 0x03);     // 5-byte address
    NRF_WriteRegister(RX_PW_P0, 1);        // Payload size = 1 byte
    NRF_WriteRegisterMulti(RX_ADDR_P0, rx_address, 5);


    NRF_ClearStatus();

    NRF_CE_HIGH(); // Start listening
}
//************************************* transmittor******************
///////////////////////////////////////////////////////////////////////////////////////////
#define FLUSH_TX           0xE1
#define W_TX_PAYLOAD       0xA0
#define TX_ADDR            0x10
// NRF24L01 Transmitter Setup
void NRF_TX_Init() {
    __delay_ms(100);

//TX_Addr register in NRF is loaded with this addres  and in reciver side RX_ADDR_P0hast mach TX_ADDR for successful communication
    uint8_t tx_address[5] = {'A', 'A', 'A', 'A', 'A'};

    NRF_CE_LOW();  // Disable before config

    NRF_WriteRegister(CONFIG, 0x0A);         // Power up, TX mode changed 0X0E to 0X0A
    /*Auto ACK handling*/
    NRF_WriteRegister(SETUP_RETR_REG,ARC);
    NRF_WriteRegister(EN_AA, 0X01);          // Enable auto-ack on pipe 0
//    NRF_WriteRegister(SETUP_RETR, 0x00);     // Disable retransmit
    NRF_WriteRegister(RF_CH, 0x02);          // Set channel = 2
//    NRF_WriteRegister(RF_SETUP, 0x06);       // 1 Mbps, 0 dBm
    /*-6 dB RF power (data rate = 1MB)*/
    NRF_WriteRegister(RF_SETUP ,RF_PWR );
    NRF_WriteRegister(SETUP_AW, 0x03);       // 5-byte address width

    NRF_WriteRegisterMulti(TX_ADDR, tx_address, 5);     // Set TX addr
    NRF_WriteRegisterMulti(RX_ADDR_P0, tx_address, 5);  // For auto-ack

    NRF_CE_HIGH();
    __delay_ms(2); // Allow to settle
}

// Clear TX FIFO
void NRF_FlushTX() {
    NRF_CSN_LOW();
    NRF_SPI(FLUSH_TX);
    NRF_CSN_HIGH();
}

// Send payload
void NRF_SendPayload(uint8_t *data, uint8_t len) {
    NRF_CSN_LOW();
    NRF_SPI(W_TX_PAYLOAD);
    for (uint8_t i = 0; i < len; i++)
        NRF_SPI(data[i]);
    NRF_CSN_HIGH();
}

void main(void) {
    SYSTEM_Initialize();

    // Set RA0 as output for debugging (e.g. blink LED)
    TRISAbits.TRISA2 = 0;
    
    //////////////////////tx switch///////////
     TRISCbits.TRISC3=1;  //switch as input
//    __delay_ms(100); // Ensure NRF has time to power up

//    NRF_RX_Init();
          NRF_TX_Init();  // Back to RX mode
           __delay_ms(100); // Ensure NRF has time to power up
//    uint8_t payload[1];

    while (1) {
//        uint8_t status = NRF_ReadRegister(STATUS);
//        if (status & 0x40) {  // RX_DR (data received)
//            NRF_ReadPayload(payload, 1);
//            NRF_ClearStatus();
//
//            if (payload[0] == 0x55) {
//                LATAbits.LATA2 = 1;
//                __delay_ms(100);
//                LATAbits.LATA2 = 0;
//            }
//        }
        
  ///////////////transmittor code/////////////////////////////
         if(PORTCbits.RC3 == 1)
        {
        __delay_ms(50); // debounce
        if (PORTCbits.RC3 == 1) // still pressed
        {
            uint8_t data[] = {0x44}; // data to send

            NRF_CE_LOW();     // Stop TX to load new payload
            NRF_FlushTX();    // Clear old data
            NRF_SendPayload(data, 1);  // Send new byte
            NRF_WriteRegister(STATUS, 0x70); // clear TX_DS, MAX_RT flags
            NRF_CE_HIGH();    // Start transmission
            __delay_ms(15);   // small delay for sending
            NRF_CE_LOW();

            // Optional: small delay to avoid repeated sends while holding
            while (PORTCbits.RC3 == 1); // wait for button release
            __delay_ms(10); // Wait to complete send
//                NRF_TX_Init();  // Back to RX mode
//                __delay_ms(150);

        }

    }
//            NRF_RX_Init();   //again setting the NRF in reciver mode 
            
        }
    }
