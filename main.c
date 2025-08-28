#include "mcc_generated_files/mcc.h"                                                                    //transmittor code

// ================= NRF24L01 Defines =================
#define R_RX_PAYLOAD    0x61
#define W_TX_PAYLOAD    0xA0
#define FLUSH_RX        0xE2
#define FLUSH_TX        0xE1
#define W_REGISTER      0x20
#define R_REGISTER      0x00
#define CONFIG          0x00
#define EN_AA           0x01
#define RF_CH           0x05
#define RF_SETUP        0x06
#define STATUS          0x07
#define RX_ADDR_P0      0x0A
#define TX_ADDR         0x10
#define SETUP_AW        0x03
#define RX_PW_P0        0x11

// ================= Macros =================
#define NRF_CE_HIGH()      LATCbits.LATC5 = 1
#define NRF_CE_LOW()       LATCbits.LATC5 = 0
#define NRF_CSN_HIGH()     LATAbits.LATA4 = 1
#define NRF_CSN_LOW()      LATAbits.LATA4 = 0

// ================= SPI Helpers =================
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
    for (uint8_t i = 0; i < len; i++) NRF_SPI(data[i]);
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
    for (uint8_t i = 0; i < len; i++) data[i] = NRF_SPI(0xFF);
    NRF_CSN_HIGH();

    NRF_CSN_LOW();
    NRF_SPI(FLUSH_RX);
    NRF_CSN_HIGH();
}

void NRF_SendPayload(uint8_t *data, uint8_t len) {
    NRF_CSN_LOW();
    NRF_SPI(W_TX_PAYLOAD);
    for (uint8_t i = 0; i < len; i++) NRF_SPI(data[i]);
    NRF_CSN_HIGH();
}

void NRF_ClearStatus() {
    NRF_WriteRegister(STATUS, 0x70);
}

// ================= Mode Control =================
void NRF_SetModeRX() {
    uint8_t addr[5] = {'A','A','A','A','A'};
    NRF_CE_LOW();
    NRF_WriteRegister(CONFIG, 0x0B); // PWR_UP=1, PRIM_RX=1
    NRF_WriteRegister(EN_AA, 0x01);
    NRF_WriteRegister(RF_CH, 0x02);
    NRF_WriteRegister(RF_SETUP, 0x06);
    NRF_WriteRegister(SETUP_AW, 0x03);
    NRF_WriteRegister(RX_PW_P0, 1);
    NRF_WriteRegisterMulti(RX_ADDR_P0, addr, 5);
    NRF_ClearStatus();
    NRF_CSN_LOW(); NRF_SPI(FLUSH_RX); NRF_CSN_HIGH();
    __delay_us(150);
    NRF_CE_HIGH();
}

void NRF_SetModeTX() {
    uint8_t addr[5] = {'A','A','A','A','A'};
    NRF_CE_LOW();
    NRF_WriteRegister(CONFIG, 0x0A); // PWR_UP=1, PRIM_RX=0
    NRF_WriteRegister(RF_CH, 0x02);
    NRF_WriteRegister(RF_SETUP, 0x06);
    NRF_WriteRegister(SETUP_AW, 0x03);
    NRF_WriteRegisterMulti(TX_ADDR, addr, 5);
    NRF_WriteRegisterMulti(RX_ADDR_P0, addr, 5);
    NRF_ClearStatus();
    NRF_CSN_LOW(); NRF_SPI(FLUSH_TX); NRF_CSN_HIGH();
    __delay_us(150);
}

void main(void) {
    SYSTEM_Initialize();
    TRISAbits.TRISA2 = 0;  // LED output
    TRISCbits.TRISC3 = 1;  // S1 button input

    NRF_SetModeRX(); // start in RX mode

    uint8_t payload[1];

    while (1) {
        // Button pressed → transmit 0x55
        if (PORTCbits.RC3 == 1) {
            __delay_ms(20); // debounce
            if (PORTCbits.RC3 == 1) {
                uint8_t data[] = {0x55};
                EUSART_Write(data[0]);   // send raw byte
                NRF_SetModeTX();
                NRF_SendPayload(data, 1);
                NRF_CE_HIGH(); __delay_ms(2); NRF_CE_LOW();
                __delay_us(150);
                NRF_SetModeRX(); // back to listen
                while(PORTCbits.RC3 == 1); // wait for release
            }
        }

        // If data received
        if (NRF_ReadRegister(STATUS) & 0x40) {
            NRF_ReadPayload(payload, 1);
            NRF_ClearStatus();
            if (payload[0] == 0x44) { // from S2
                EUSART_Write(payload[0]);   // send raw byte
                LATAbits.LATA2 = 1;
                __delay_ms(100);
                LATAbits.LATA2 = 0;
            }
        }
    }
}
