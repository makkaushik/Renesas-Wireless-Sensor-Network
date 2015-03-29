/****************************************************************

	WIRELESS SENSOR NETWORK
	
		RHYTHM PATEL
		KAUSHIK MAKWANA

****************************************************************/
	
/****************************************************************

	POSITION	: ROUTER NODE
	ADDRESS 	: 0x2001
	VICINITY	: 2

****************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <machine.h>
#include "platform.h"
#include "uart.h"
#include "xbeeTransmit.h"
#include "r_switches.h"
//#include "xbeeTransmit.c"


#define SW1_DESTINATION 0x1001
#define SW2_DESTINATION 0x3001


/* 	ROUTING TABLE 
	DESTINATION, NEXT NODE
*/


/* ROUNTING TABLE */
#define ROUTING_TABLE_SIZE 2
int ROUTING_TABLE[10][10] = {
	{0x1001, 0x1001},
	{0x3001, 0x3001}
};
	



/* global switch flag for detecting switch number */
int switch_flag = 0;
//external variable


void main(void)
{
    uint8_t new_char;
    
    /* Initialize LCD */
    lcd_initialize();
    
    /* Clear LCD */
    lcd_clear();
    
	
	
	char data[DATA_SIZE];
	int destination;
	char temp[20], newchar;
	int data_length = 0;
	char dat[DATA_SIZE];
	int tx_destination;	
	
    /* Display message on LCD */

    lcd_display(LCD_LINE3, "  ROUTER ");    
    lcd_display(LCD_LINE1, "MEDIUM FREE");
    
    
	
    /* Initialize the SCI channel for asynchronous UART communications. */
    sci_uart_init(); 
	
	/* can use below instruction for future purpose */
    //sci_tx_int_enable();
   	// sci_rx_int_enable(); 

    /* The three pushbuttons on the YRDK board are tied to interrupt lines, set them up here */
    R_SWITCHES_Init();

	/* create XBee Packet for transmission/reception */
	xbee_packet tx, r, 	tx1;
	
	/* Create new packet for API */
	init_new_packet(&tx);
	
	/*disable Transmit interrupt */
	sci_tx_int_disable();
	
	char frame_id = 0x01;

/*	
	create_packet_16bit(&tx, 0x3001, data, frame_id);
	
	for(int i=0; i<32000; i++)
		for(int j=0; j<1500; j++);
		tx_request_16_bit(&tx);

	create_packet_16bit(&tx1, 0x3001, "KAU", frame_id);
	for(int i=0; i<32000; i++)
		for(int j=0; j<1500; j++);
		tx_request_16_bit(&tx1);*/
	// CALLING transmit TX REQUEST 16-BIT

	
	// broadcast
/*	create_packet_16bit(&tx, 0xFFFF, "ABC", 3);
	tx_request_16_bit(&tx);*/
	/////////////////////////////////////////////////////////////////////////////////
	
//	sci_rx_int_enable();
	int i, j;
	
	char data_tx[DATA_SIZE] = "ND2";


	//	xbee_packet rx;
	/* enable RX interrupt in ICU */
	 IEN(SCI6, RXI6) = 1;

	/* future purpose */
	//	IEN(SCI6, TXI6) = 0;	 
	  

	 /* initialize port for switches */
	 PORT4.PDR.BIT.B0 = 0;
	 PORT4.PDR.BIT.B1 = 0; 
	 PORT4.PMR.BIT.B0 = 0;
	 PORT4.PMR.BIT.B1 = 0;
	 
	 
	 /* run forever */
	while(1){
		/* if switch 1 pressed */
		if(switch_flag == 1)
		{
			/* THIS IF_ELSE IS FOR DEMONSTRATION PURPOSE ONLY.
			   When switch 1 is pressed, then the packet is 
			   sent to SW1_DESTINATION */
			destination = SW1_DESTINATION;
					
					data[0] = destination>>8;
					data[1] = destination & 0x00FF;
					
					strcat(data, data_tx);
					/* look up in ROUTING TABLE for NEXT NODE */
					for(i = 0; i<ROUTING_TABLE_SIZE; i++)
					{
						/* MATCH FOUND for DESIRED DESTINATION ADDRESS */
						if(destination == ROUTING_TABLE[i][0])
						{
							tx_destination = ROUTING_TABLE[i][1];
							//break;	
						}	
					}
					create_packet_16bit(&tx, tx_destination, data, frame_id);
				
					for( i=0; i<32000; i++)
						for(j=0; j<1500; j++);
						tx_request_16_bit(&tx);
					
					for( i = 0; i<DATA_SIZE; i++)
					data[i] = 0;
					
					switch_flag = 0;
			
			
		}
		
		/* if switch 2 pressed */
		else if(switch_flag == 2)
		{
			/* 	THIS IF_ELSE IS FOR DEMONSTRATION PURPOSE ONLY.
				When switch 2 is pressed, then the packet is 
			   	sent to SW2_DESTINATION */
			destination = SW2_DESTINATION;
					
					data[0] = destination>>8;
					data[1] = destination & 0x00FF;
					
					strcat(data, data_tx);
					/* look up in ROUTING TABLE for NEXT NODE */
					for(i = 0; i<ROUTING_TABLE_SIZE; i++)
					{
						/* MATCH FOUND for DESIRED DESTINATION ADDRESS */
						if(destination == ROUTING_TABLE[i][0])
						{
							tx_destination = ROUTING_TABLE[i][1];
							//break;	
						}	
					}
					create_packet_16bit(&tx, tx_destination, data, frame_id);
				
					for( i=0; i<32000; i++)
						for(j=0; j<1500; j++);
						tx_request_16_bit(&tx);

			
				
				for( i = 0; i<DATA_SIZE; i++)
					data[i] = 0;
					
				switch_flag = 0;
			
		}
	
		/* check in buffer for any RECEIVED DATA */
		if(rx_buffer_empty())
		{
			/* dequeue from buffer */
			r = get_packet_buffer();
		
			/* record the destination address */
			destination  = r.data[0];
			destination = destination << 8;
			destination |= r.data[1];

			/* if data for current node 
				then display its contents */
			if(destination == ADDRESS)
			{
				for(i = 2; i< r.data_len; i++)
				dat[i-2] = r.data[i];
		
				//strcat(temp, dat);
				sprintf(temp, "data %c %c %c", dat[0], dat[1], dat[2]);
	
				lcd_display(LCD_LINE5, temp);
			}
			else /* if data needs to be routed FORWARD */
			{
				/* look up in ROUTING TABLE for NEXT NODE */
				for(i = 0; i<ROUTING_TABLE_SIZE; i++)
					{
						/* MATCH FOUND for DESIRED DESTINATION ADDRESS */
						if(destination == ROUTING_TABLE[i][0])
						{
							tx_destination = ROUTING_TABLE[i][1];
							//break;	
						}	
					}/* end of for */
					
				/* create new packet for routing FORWARD the received data */
				create_packet_16bit(&tx, tx_destination, r.data, r.frame_id);
				
				/* transmit the packet to NEXT NODE */
				for( i=0; i<32000; i++)
					for(j=0; j<1500; j++);
					tx_request_16_bit(&tx);
			}/* end of if-else */
			
		}/* end of if (buffer_empty) */
		
		
	} /* end of while */
	

} /* End of function main() */



/******************************************************************************
* Function name: sw1_callback
* Description  : Callback function that is executed when SW1 is pressed.
*                Called by sw1_isr in r_switches.c
* Arguments    : none
* Return value : none
******************************************************************************/
void sw1_callback(void)
{
    switch_flag = 1; /* Add your code here. Note: this is executing inside an ISR. */
} /* End of function sw1_callback() */


/******************************************************************************
* Function name: sw2_callback
* Description  : Callback function that is executed when SW2 is pressed.
*                Called by sw2_isr in r_switches.c
* Arguments    : none
* Return value : none
******************************************************************************/
void sw2_callback(void)
{
   switch_flag = 2; 
} /* End of function sw2_callback() */
