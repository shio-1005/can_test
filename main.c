
#include <xc.h>
#include <stdio.h>
#include <stdlib.h>
#include "spi.h"
#include "MCP2515.h"
//#include "fram.h"
#include "ADS_2.h"

// CONFIG1
#pragma config FOSC = HS        // Oscillator Selection bits (HS oscillator: High-speed crystal/resonator on RA6/OSC2/CLKOUT and RA7/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled and can be enabled by SWDTEN bit of the WDTCON register)
#pragma config PWRTE = ON       // Power-up Timer Enable bit (PWRT enabled)
#pragma config MCLRE = ON       // RE3/MCLR pin function select bit (RE3/MCLR pin function is MCLR)
#pragma config CP = OFF         // Code Protection bit (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Code Protection bit (Data memory code protection is disabled)
#pragma config BOREN = ON       // Brown Out Reset Selection bits (BOR enabled)
#pragma config IESO = OFF       // Internal External Switchover bit (Internal/External Switchover mode is disabled)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enabled bit (Fail-Safe Clock Monitor is disabled)
#pragma config LVP = OFF        // Low Voltage Programming Enable bit (RB3 pin has digital I/O, HV on MCLR must be used for programming)

// CONFIG2
#pragma config BOR4V = BOR21V   // Brown-out Reset Selection bit (Brown-out Reset set to 2.1V)
#pragma config WRT   = OFF        // Flash Program Memory Self Write Enable bits (Write protection off)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#define _XTAL_FREQ      20000000
#define _CAN_BAUDRATE   2

#define _ChargeMode     0b00000001
#define _COMMMode       0b00000010
#define _StandbyMode    0b00000011
#define _MissionMode    0b00000100

#define EID8_MODE       0b00000001
#define EID0_MODE       0b00000000


#define _RXB0D0         0b01100110

#define RXF0SIDH        0b00000000                                                          //フィルタ設定
#define RXF0SIDL        0b00001000
#define RXF0EID8        0b00000000
#define RXF0EID0        0b00000000
#define RXM0SIDH        0b00000000                                                          //マスク設定
#define RXM0SIDL        0b00000000
#define RXM0EID8        0b00000000
#define RXM0EID0        0b00000000

/*--Prtotype--*/
void CONFIG();
//void interrupt timer();

/*--Grobal Variables--*/
volatile enum MODE{Charge, COMM, Standby, Mission} Mode; 
volatile char mode[8] = {0, 0, 0, 0, 0, 0, 0, 0};
volatile char HK_1[8] = {1, 1, 1, 1, 1, 1, 1, 1};
volatile char HK_2[8] = {2, 2, 2, 2, 2, 2, 2, 2};
volatile char HK_3[8] = {3, 3, 3, 3, 3, 3, 3, 3};
volatile char HK_4[8] = {4, 4, 4, 4, 4, 4, 4, 4};
volatile char flag;
volatile char rx_int;
volatile int  cnt = 0;

volatile char data;

void main(void)
{ 
    CONFIG();
    __delay_ms(100);
    
    //Write(_CANINTF, 0b00000000);
    
    /* 送信　
        Write(_TXB0DLC , 0b00001000);                                               //データサイズ8byte
        Load_TX_ID(_F_TXB0SIDH, SIDH_1, SIDL_DATA2, EID8_WRITE, EID0_DATA2);        //HKデータ送信       
        Load_TX_Data(_F_TXB0D0, 8, &HK_1);                                          //データ送信 
        RTS0(2);
      */
    
    
    while(1){
        
        
        /* 受信　*/
        rx_int = Read(_CANINTF);                                                            //受信バッファ確認
        if((rx_int & _Flagbit0) == 0b00000001)
        {   
            Write(_CANINTF, 0b00000000);                                                    //受信フラグクリア
            data = Read(_RXB0D0);
            __delay_ms(1000);
            
            /* 送信　*/
            Write(_TXB0DLC , 0b00000001);                                               //データサイズ8byte
            Load_TX_ID(_F_TXB0SIDH, SIDH_1, SIDL_DATA2, EID8_WRITE, EID0_DATA2);        //HKデータ送信       
            Load_TX_Data(_F_TXB0D0, 1, &data);                                          //データ送信 
            RTS0(2);
        
            /* 送信　*/
            Write(_TXB0DLC , 0b00001000);                                               //データサイズ8byte
            Load_TX_ID(_F_TXB0SIDH, SIDH_1, SIDL_DATA2, EID8_WRITE, EID0_DATA2);        //HKデータ送信       
            Load_TX_Data(_F_TXB0D0, 8, &HK_1);                                          //データ送信 
            RTS0(2);
        }
        
        
        
    }
    
    
}

void CONFIG()
{
    OSCCON = 0b01101000;
    ANSEL  = 0b00000000;
    ANSELH = 0b00000000;
    TRISA  = 0b00000000;
    TRISC  = 0b00000000;
    PORTA  = 0b00000000;
    PORTC  = 0b00000000;
    
    spi_init();
    __delay_ms(100);
    
    MCP2515_init(_CAN_BAUDRATE);                                                
    Write(_TXB0DLC , 0b00001000);                                                               //データサイズ8byte
    Write(_RXM0SIDH , RXM0SIDH);                                                                //マスク設定
    Write(_RXM0SIDL , RXM0SIDL);
    Write(_RXM0EID8 , RXM0EID8);
    Write(_RXM0EID0 , RXM0EID0);
    Write(_RXF0SIDH , RXF0SIDH);                                                                //フィルタ設定
    Write(_RXF0SIDL , RXF0SIDL);
    Write(_RXF0EID8 , RXF0EID8);
    Write(_RXF0EID0 , RXF0EID0);
    MCP2515_Open(0);                                                                            //0にしないと動かなかった．これから考える
    
    /*TMR1設定
    PIR1bits.TMR1IF = 0;                                                                        //TMR1フラグ
    PIE1bits.TMR1IE = 1;                                                                        //TMR1可能
    T1CONbits.T1CKPS1 = 1;                                                                      //TMR1プリスケーラ
    T1CONbits.T1CKPS0 = 1;
    T1CONbits.TMR1CS = 0;                                                                       //TMR1内部クロック使用
    T1CONbits.TMR1ON = 0;                                                                       //TMR1 OFF
    */
}

/*
void interrupt timer(void)
{
    
    PIR1bits.TMR1IF = 0;                                                                        //TMR1 フラグクリア
    TMR1L = 0;                                                                                  //TMR1 ビット
    TMR1H = 0;
    
    cnt++;
}
*/