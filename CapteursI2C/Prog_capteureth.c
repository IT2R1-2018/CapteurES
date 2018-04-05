#include "Driver_I2C.h"                 // ::CMSIS Driver:I2C
#include "stdio.h"
#include "Board_GLCD.h"                 // ::Board Support:Graphic LCD
#include "GLCD_Config.h"                // Keil.MCB1700::Board Support:Graphic LCD
#include "LPC17xx.h"
#include "Board_LED.h"                  // ::Board Support:LED
#include "Board_ADC.h"
#include "rl_net.h"    


extern GLCD_FONT GLCD_Font_16x24;
extern GLCD_FONT GLCD_Font_6x8;
extern ARM_DRIVER_I2C Driver_I2C2;
extern ARM_DRIVER_I2C Driver_I2C0;
int32_t udp_sock; 


#define SLAVE_I2C_ADDR1       0x71		// Adresse esclave sur 7 bits
#define SLAVE_I2C_ADDR2       0x70		// Adresse esclave sur 7 bits

uint8_t DeviceAddr1;
uint8_t DeviceAddr2;
static char presence1;
static char presence2;

void send_udp_data (void) {
 
  if (udp_sock > 0) {
     
    // IPv4 address: 192.168.0.2
			NET_ADDR addr = { NET_ADDR_IP4, 2000 , 192, 168, 0, 2 };
    uint8_t *sendbuf;
		char julien=0;	
		if (presence1 == 1 ) julien |= (1<<0);
		else julien |= (0<<0);
		if (presence2 == 1 )  julien |= (1<<4);
		else julien |= (0<<4);
    sendbuf = netUDP_GetBuffer (2);
    sendbuf[0] = 0xBB;
    sendbuf[1] = julien;

    netUDP_Send (udp_sock, &addr, sendbuf, 2);
    
  }
}

uint32_t udp_cb_func (int32_t socket, const  NET_ADDR *addr, const uint8_t *buf, uint32_t len) {
 
  // Data received
  
  if (buf[0] == 0xBB) {
    LED_On (0);
		send_udp_data();
  }
	else LED_On(buf[1]);
	//send_udp_data();
  
  return (0);
}


void write1byte2(unsigned char composant, unsigned char registre, unsigned char valeur)
{ 
	unsigned char tab1[2];
	tab1[0]=registre;
	tab1[1]=valeur;
	Driver_I2C2.MasterTransmit(composant,tab1,2,false);
	while (Driver_I2C2.GetStatus().busy == 1);
}
void write1byte0(unsigned char composant, unsigned char registre, unsigned char valeur)
{ 
	unsigned char tab0[2];
	tab0[0]=registre;
	tab0[1]=valeur;
	Driver_I2C0.MasterTransmit(composant,tab0,2,false);
	while (Driver_I2C0.GetStatus().busy == 1);
}


unsigned char read1byte2 (unsigned char composant, unsigned char registre)
{
	unsigned char retour;
	Driver_I2C2.MasterTransmit(composant,&registre,1,true);
	while (Driver_I2C2.GetStatus().busy == 1);

	
	Driver_I2C2.MasterReceive (composant, &retour, 1, false);		// false = avec stop
	while (Driver_I2C2.GetStatus().busy == 1);	// attente fin transmission
	
	return retour;
}
unsigned char read1byte0 (unsigned char composant, unsigned char registre)
{
	unsigned char retour;
	Driver_I2C0.MasterTransmit(composant,&registre,1,true);
	while (Driver_I2C0.GetStatus().busy == 1);

	
	Driver_I2C0.MasterReceive (composant, &retour, 1, false);		// false = avec stop
	while (Driver_I2C0.GetStatus().busy == 1);	// attente fin transmission
	
	return retour;
}

void Init_I2C2(void){
	Driver_I2C2.Initialize(NULL);
	Driver_I2C2.PowerControl(ARM_POWER_FULL);
	Driver_I2C2.Control(	ARM_I2C_BUS_SPEED,				// 2nd argument = débit
							ARM_I2C_BUS_SPEED_STANDARD  );	// 100 kHz
	Driver_I2C2.Control(	ARM_I2C_BUS_CLEAR,0 );
}
void Init_I2C0(void){
	Driver_I2C0.Initialize(NULL);
	Driver_I2C0.PowerControl(ARM_POWER_FULL);
	Driver_I2C0.Control(	ARM_I2C_BUS_SPEED,				// 2nd argument = débit
							ARM_I2C_BUS_SPEED_STANDARD  );	// 100 kHz
	Driver_I2C0.Control(	ARM_I2C_BUS_CLEAR,0 );
}


void Timer_Init(unsigned int prescaler, unsigned int valeur)
{
LPC_SC->PCONP |= (1<<1); //allume le timer 0 (facultatif, déjà allumé après un reset)

LPC_TIM0->PR =  prescaler;
LPC_TIM0->MR0 = valeur;
LPC_TIM0->MCR = 3;  /*reset counter si MR0=COUNTER + interrupt*/

 LPC_TIM0->TCR = 1; //démarre le comptage
}

void TIMER0_IRQHandler(void)
{
		char tab3[30];
    LPC_TIM0->IR |= 1<<0;   /* clear interrupt bit */
		
		LED_Off(3);
		presence1=read1byte0(0x71,0x03);
		presence2=read1byte2(0x70,0x03);
		if (presence1 <= 15) presence1 = 1;
		else presence1 = 0;
		if (presence2 <= 15) presence2 = 1;
		else presence2 = 0;
		sprintf(tab3,"Valeur1 : %02d%02d", presence1,presence2);
	  GLCD_DrawString(10,10,tab3);
		write1byte0(0x71,0x00,0x51);
		write1byte2(0x70,0x00,0x51);
		LED_On(3);
}

int main (void){
	Init_I2C2();
	Init_I2C0();
	GLCD_Initialize();
	GLCD_ClearScreen();
	netInitialize();
	
	GLCD_SetFont(&GLCD_Font_16x24);
	
	LED_Initialize();
	
		udp_sock = netUDP_GetSocket (udp_cb_func);
  if (udp_sock > 0) {
    netUDP_Open (udp_sock, 2000);}
	
	// Adresse module esclave sur 7 bits
	DeviceAddr1 = SLAVE_I2C_ADDR1;
	DeviceAddr2 = SLAVE_I2C_ADDR2;
  Timer_Init(1,12500000/2-1);          // 12 Hz

    // L'IT Timer doit être de priorité plus faible que I2C et LCD
    NVIC_EnableIRQ(TIMER0_IRQn);
    NVIC_SetPriority(TIMER0_IRQn,2);
		
	
}
