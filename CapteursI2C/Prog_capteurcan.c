#define osObjectsPublic   							// define objects in main module 
#include "osObjects.h"                      // RTOS object definitions

#include "Driver_I2C.h"                 // ::CMSIS Driver:I2C
#include "stdio.h"
#include "Board_GLCD.h"                 // ::Board Support:Graphic LCD
#include "GLCD_Config.h"                // Keil.MCB1700::Board Support:Graphic LCD
#include "LPC17xx.h"
#include "Board_LED.h"                  // ::Board Support:LED
#include "RTE_Device.h"                 // Keil::Device:Startup
#include "Driver_CAN.h"                 // ::CMSIS Driver:CAN



extern GLCD_FONT GLCD_Font_16x24;
extern GLCD_FONT GLCD_Font_6x8;
extern ARM_DRIVER_I2C Driver_I2C2;
extern ARM_DRIVER_I2C Driver_I2C0;
extern   ARM_DRIVER_CAN         Driver_CAN1;
extern   ARM_DRIVER_CAN         Driver_CAN2;


#define SLAVE_I2C_ADDR1       0x70		// Adresse esclave sur 7 bits
#define SLAVE_I2C_ADDR2       0x71		// Adresse esclave sur 7 bits

uint8_t DeviceAddr1;
uint8_t DeviceAddr2;
static char presence1;
static char presence2;
char trame;

osThreadId id_CANthreadR;
void CANthreadR (void const *argument);       
osThreadDef(CANthreadR,osPriorityNormal,1,0);

osThreadId id_CANthreadT;
void CANthreadT (void const *argument);       
osThreadDef(CANthreadT,osPriorityNormal,1,0);

osThreadId id_Recupcapt;
void Recupcapt (void const *argument);
osThreadDef(Recupcapt,osPriorityNormal,1,512);



 void myCAN2_callback(uint32_t obj_idx, uint32_t event)
{
      switch (event)
    {
    case ARM_CAN_EVENT_RECEIVE:
        /*  Message was received successfully by the obj_idx object. */
       osSignalSet(id_CANthreadR, 0x001);
        break;
    }
}


    void CANthreadR (void const *argument)
		{
			uint8_t data_buf[1];
			int id;
			char taille;
			char textr[10];
			ARM_CAN_MSG_INFO   rx_msg_info;
			while(1)
			{
		LED_Off(6);
		osSignalWait(0x001,osWaitForever);
		Driver_CAN2.MessageRead(0,&rx_msg_info,data_buf,0);
		id = rx_msg_info.id;
		taille = rx_msg_info.dlc;
		sprintf(textr,"ID: %03x Size: %d",rx_msg_info.id,rx_msg_info.dlc);
		GLCD_DrawString(10,150,textr);
		osSignalSet(id_CANthreadT,0x001);
		LED_On(6);
		}
	}
		
		
    void CANthreadT (void const *argument)
		{
		uint8_t envoi_buf[1];
		char julien;	
		char texts[10];
		ARM_CAN_MSG_INFO   tx_msg_info;
		tx_msg_info.id = ARM_CAN_STANDARD_ID(0x5F7);
		tx_msg_info.rtr = 0;

		while(1)
			{
		julien =0x00;
		LED_Off(4);
		osSignalWait(0x001,osWaitForever);
		if (presence1 == 1 ) julien |= (1<<0);
		else julien |= (0<<0);
		//if (presence2 == 1 )  julien |= (1<<7);
		//else julien |= (0<<7);
    envoi_buf[0] = julien;
		Driver_CAN2.MessageSend(1,&tx_msg_info,envoi_buf,1);
		sprintf(texts,"%01X",envoi_buf[0]);
		GLCD_DrawString(10,100,texts);
		LED_On(4);
			}
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

void InitCan2 (void)
	{
	Driver_CAN2.Initialize(NULL,myCAN2_callback);
	Driver_CAN2.PowerControl(ARM_POWER_FULL);
	Driver_CAN2.SetMode(ARM_CAN_MODE_INITIALIZATION);
	Driver_CAN2.SetBitrate( ARM_CAN_BITRATE_NOMINAL,
													125000,
													ARM_CAN_BIT_PROP_SEG(5U)   |         // Set propagation segment to 5 time quanta
                          ARM_CAN_BIT_PHASE_SEG1(1U) |         // Set phase segment 1 to 1 time quantum (sample point at 87.5% of bit time)
                          ARM_CAN_BIT_PHASE_SEG2(1U) |         // Set phase segment 2 to 1 time quantum (total bit is 8 time quanta long)
                          ARM_CAN_BIT_SJW(1U));                // Resynchronization jump width is same as phase segment 2
                          
	// Mettre ici les filtres ID de réception sur objet 0
	Driver_CAN2.ObjectSetFilter(0,ARM_CAN_FILTER_ID_EXACT_ADD,ARM_CAN_STANDARD_ID(0x5F7),0);
	Driver_CAN2.ObjectConfigure(1,ARM_CAN_OBJ_TX);	
	Driver_CAN2.ObjectConfigure(0,ARM_CAN_OBJ_RX);				// Objet 0 du CAN1 pour réception
	
	Driver_CAN2.SetMode(ARM_CAN_MODE_NORMAL);					// fin init
}
	


void Recupcapt (void const *argument)
{
	char tab3[30];
	while (1)
	{
		LED_Off(3);
		presence1=read1byte0(0x70,0x03);
		presence2=read1byte2(0x71,0x03);
		if (presence1 <= 15) presence1 = 1;
		else presence1 = 0;
		if (presence2 <= 15) presence2 = 1;
		else presence2 = 0;
		if (presence1 == 1) osSignalSet(id_CANthreadT,0x001);
		sprintf(tab3,"Valeur : %02d%02d", presence1,presence2);
	  GLCD_DrawString(10,10,tab3);
		write1byte0(0x70,0x00,0x51);
		write1byte2(0x71,0x00,0x51);
		LED_On(3);
			osDelay(100);

	}

}

int main (void){
	osKernelInitialize ();    
	Init_I2C2();
	Init_I2C0();
	InitCan2();
	GLCD_Initialize();
	GLCD_ClearScreen();
	GLCD_SetFont(&GLCD_Font_6x8);
	LED_Initialize();		

	DeviceAddr1 = SLAVE_I2C_ADDR1;
	DeviceAddr2 = SLAVE_I2C_ADDR2;
 
    // L'IT Timer doit être de priorité plus faible que I2C et LCD
	id_Recupcapt = osThreadCreate (osThread(Recupcapt), NULL);
	id_CANthreadR = osThreadCreate (osThread(CANthreadR), NULL);
	id_CANthreadT = osThreadCreate (osThread(CANthreadT), NULL);
	
	 osKernelStart ();                         // start thread execution 
	 osDelay(osWaitForever);
}
