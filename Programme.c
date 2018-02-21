#include "rl_net.h"                     // Keil.MDK-Pro::Network:CORE
#include "GPIO_LPC17xx.h"               // Keil::Device:GPIO
#include "GLCD_Config.h"                // Keil.MCB1700::Board Support:Graphic LCD
#include "GPDMA_LPC17xx.h"              // Keil::Device:GPDMA
#include "Board_LED.h"                  // ::Board Support:LED



uint32_t udp_cb_func(int32_t socket, const NET_ADDR *addr, const uint8_t *buf, uint32_t len);
void send_udp_data (void);

int32_t udp_sock;





int main (void)
{
	netInitialize();
	LED_Initialize();
	
	udp_sock = netUDP_GetSocket(udp_cb_func);
	if (udp_sock >0) 
	{
		netUDP_Open(udp_sock, 2001);
	}

return 0;
}




uint32_t udp_cb_func(int32_t socket, const NET_ADDR *addr, const uint8_t *buf, uint32_t len)
{
	uint8_t *ptr;
	ptr = netUDP_GetBuffer(len);
	ptr[0]= buf[0];
	ptr[1]= buf[1];
	if (buf[0] == 0x01 ) 
	{
		 LED_On(3);
	}
	else
	{
			LED_Off(3);
		}
		send_udp_data ();
		return 0;
	}




	void send_udp_data (void) {
	 
		if (udp_sock > 0) {

			// IPv4 address: 192.168.0.1
			NET_ADDR addr = { NET_ADDR_IP4, 2000, 192, 168, 0, 5 };
		 
			uint8_t *sendbuf;
	 
    sendbuf = netUDP_GetBuffer (2);
    sendbuf[0] = 0x01;
    sendbuf[1] = 0xAA;
 
    netUDP_Send (udp_sock, &addr, sendbuf, 2);
   
  }
}
 