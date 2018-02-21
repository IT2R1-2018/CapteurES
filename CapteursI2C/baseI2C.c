#include "Driver_I2C.h"                 // ::CMSIS Driver:I2C
#include "stdio.h"
#include "Board_GLCD.h"                 // ::Board Support:Graphic LCD
#include "GLCD_Config.h"                // Keil.MCB1700::Board Support:Graphic LCD

#define SLAVE_I2C_ADDR       0x1D			// Adresse esclave sur 7 bits

extern ARM_DRIVER_I2C Driver_I2C0;

uint8_t DeviceAddr;

void write1byte(unsigned char composant, unsigned char registre, unsigned char valeur)
{ 
	unsigned char tab1[2];
	tab1[0]=registre;
	tab1[1]=valeur;
	Driver_I2C0.MasterTransmit(composant,tab1,2,false);
	while (Driver_I2C0.GetStatus().busy == 1);
}


unsigned char read1byte (unsigned char composant, unsigned char registre)
{
	unsigned char retour;
	Driver_I2C0.MasterTransmit(composant,&registre,1,true);
	while (Driver_I2C0.GetStatus().busy == 1);
	
	Driver_I2C0.MasterReceive (composant, &retour, 1, false);		// false = avec stop
	while (Driver_I2C0.GetStatus().busy == 1);	// attente fin transmission
	return retour;
}
void Init_I2C(void){
	Driver_I2C0.Initialize(NULL);
	Driver_I2C0.PowerControl(ARM_POWER_FULL);
	Driver_I2C0.Control(	ARM_I2C_BUS_SPEED,				// 2nd argument = débit
							ARM_I2C_BUS_SPEED_STANDARD  );	// 100 kHz
	Driver_I2C0.Control(	ARM_I2C_BUS_CLEAR,
							0 );
}
extern GLCD_FONT GLCD_Font_16x24;
extern GLCD_FONT GLCD_Font_6x8;

#include "LPC17xx.h"

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
    LPC_TIM0->IR |= 1<<0;   /* clear interrupt bit */
	char VitX,VitY,posX,posY;
	unsigned char commande = 0x20;
	unsigned char Y1, Y2;
	unsigned char Ya1, Ya2;
	unsigned char Xa1, Xa2;
	short X,Y;
	unsigned char X1;
	unsigned char X2;
	char tab2[100];
	char tab3[100];
	char tab4[100];
	char tab5[100];
	tab2[0]=X1;
	tab2[1]=X2;
	tab3[0]=Y1;
	tab3[1]=Y2;
	tab4[0]=Xa1;
	tab4[1]=Xa2;
	tab5[0]=Ya1;
	tab5[1]=Ya2;
	write1byte(0x1D, 0x20,0x57);
	write1byte(0x1D, 0x21,0x00);
	Xa1=read1byte(0x1D,0x28);
	Xa2=read1byte(0x1D,0x29);
	Ya1=read1byte(0x1D,0x2A);
	Ya2=read1byte(0x1D,0x2B);
	X=Xa2<<8|Xa1;
	Y=Ya2<<8|Ya1;
	posX=posX+X/200;
	posY=posY+Y/200;
    GLCD_DrawRectangle (posX, posY, 20, 20);
}



int main (void){
	uint8_t tab[10], maValeur;
	

	
	Init_I2C();
	GLCD_Initialize();
	GLCD_ClearScreen();
	GLCD_SetFont(&GLCD_Font_16x24);
	// Adresse module esclave sur 7 bits
	DeviceAddr = SLAVE_I2C_ADDR;
	 
	 Timer_Init(1, 624999);          // 20 Hz

    // L'IT Timer doit être de priorité plus faible que I2C et LCD
    NVIC_EnableIRQ(TIMER0_IRQn);
    NVIC_SetPriority(TIMER0_IRQn,2);

    NVIC_SetPriority(I2C0_IRQn,0);
    NVIC_SetPriority(SSP1_IRQn,0);
		
	while (1)
	{
//			write1byte(0x1D, 0x25,0x20);
//			X1=read1byte(0x1D,0x08);
//			X2=read1byte(0x1D,0x09);
//			write1byte(0x1D, 0x26,0x00);
//			sprintf(tab2,"mag X = %06d ",(short)(X2<<8|X1));
//			GLCD_DrawString(1,1,(unsigned char*)tab2);
//		
//			write1byte(0x1D, 0x25,0x20);
//			Y1=read1byte(0x1D,0x0A);
//			Y2=read1byte(0x1D,0x0B);
//			write1byte(0x1D, 0x26,0x00);
//			sprintf(tab3,"mag Y = %06d ",(short)(Y2<<8|Y1));
//			GLCD_DrawString(1,50,(unsigned char*)tab3);
		
		
		
			
//			sprintf(tab4,"mag Xa = %06d ",(short)(Xa2<<8|Xa1));
//			GLCD_DrawString(1,100,(unsigned char*)tab4);
//			
//			sprintf(tab5,"mag Ya = %06d ",(short)(Ya2<<8|Ya1));
//			GLCD_DrawString(1,150,(unsigned char*)tab5);
//			
	}
	return 0;
}

