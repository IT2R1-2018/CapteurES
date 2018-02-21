#include "Driver_I2C.h"                 // ::CMSIS Driver:I2C
#include "stdio.h"
#include "Board_GLCD.h"                 // ::Board Support:Graphic LCD
#include "GLCD_Config.h"                // Keil.MCB1700::Board Support:Graphic LCD
#include "LPC17xx.h"
#include "Board_LED.h"                  // ::Board Support:LED

extern GLCD_FONT GLCD_Font_16x24;
extern GLCD_FONT GLCD_Font_6x8;
extern ARM_DRIVER_I2C Driver_I2C2;

#define SLAVE_I2C_ADDR       0xE2		// Adresse esclave sur 7 bits
uint8_t DeviceAddr;
char presence[2];
char tab3[30];

void write1byte(unsigned char composant, unsigned char registre, unsigned char valeur)
{ 
	unsigned char tab1[2];
	tab1[0]=registre;
	
	
	tab1[1]=valeur;
	Driver_I2C2.MasterTransmit(composant,tab1,2,false);
	while (Driver_I2C2.GetStatus().busy == 1);
}


unsigned char read1byte (unsigned char composant, unsigned char registre)
{
	unsigned char retour;
	Driver_I2C2.MasterTransmit(composant,&registre,1,true);
	LED_On(1);
	while (Driver_I2C2.GetStatus().busy == 1);

	
	Driver_I2C2.MasterReceive (composant, &retour, 1, false);		// false = avec stop
	LED_Off(1);
	while (Driver_I2C2.GetStatus().busy == 1);	// attente fin transmission
	LED_On(0);
	
	return retour;
}
void Init_I2C(void){
	Driver_I2C2.Initialize(NULL);
	Driver_I2C2.PowerControl(ARM_POWER_FULL);
	Driver_I2C2.Control(	ARM_I2C_BUS_SPEED,				// 2nd argument = débit
							ARM_I2C_BUS_SPEED_STANDARD  );	// 100 kHz
	Driver_I2C2.Control(	ARM_I2C_BUS_CLEAR,0 );
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
		char i;
    LPC_TIM0->IR |= 1<<0;   /* clear interrupt bit */
		presence[0]=read1byte(0xE2,0x51);
	  sprintf(tab3,"Valeur : %d", presence[0]);
		GLCD_DrawString(10,10,tab3);
		LED_On(3);
}

int main (void){
	
	Init_I2C();
	GLCD_Initialize();
	GLCD_ClearScreen();
	GLCD_SetFont(&GLCD_Font_16x24);
	LED_Initialize();
	// Adresse module esclave sur 7 bits
	DeviceAddr = SLAVE_I2C_ADDR;
	 
	 Timer_Init(1, 624999);          // 20 Hz

    // L'IT Timer doit être de priorité plus faible que I2C et LCD
    NVIC_EnableIRQ(TIMER0_IRQn);
    NVIC_SetPriority(TIMER0_IRQn,2);
		
	while (1)
	{
   		 
	}
}
