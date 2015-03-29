#include "xbeetransmit.h"

int rx_buffer_size = 0;
int rx_buffer_head = 0;
int rx_buffer_tail = 0;
int tx_buffer_size = 0;
int tx_buffer_tail = 0;
int tx_buffer_head = 0;

int ACK_response = 0;

int block_status = 0;

char data_UART[DATA_SIZE];

xbee_packet packet_buffer[BUFFER_SIZE];
xbee_packet transmit_packet_buffer[BUFFER_SIZE];
char block_text[15] = "MEDIUM BUSY";
char unblock_text[15] = "MEDIUM FREE";
char recieve_text[15] = "RECIEVE";
char transmit_text[15] = "TRANSMITTING";

void init_new_packet(xbee_packet *tx){
	
	tx->length = 0;
	tx->api_identifier = 0;
	tx->DH = 0;
	tx->DL = 0;
	tx->api_identifier = 0;
	tx->source_address = 0;
	tx->rssi = 0;
	tx->option = 0;
	tx->frame_id = 0;
	//tx->AT_com[1] = 0;
//	tx->status = 0;
	tx->data[20] = 0;
	tx->data_len = 0;	
	tx->checksum = 0;	
	
}

/******************************************************************************
* Function name: check_checksum
* Description  : This function checks the checksum in the received API packet. 
				If the checksum doesn't match, the packet is discarded.
* Arguments    : xbee_packet pointer
* Return value : integer
******************************************************************************/
int check_checksum(xbee_packet *pkt){

	int sum = 0, count;
	switch(pkt->api_identifier)
	{
	
		case 0x81:
			/* CHECKSUM check for RX 16-Bit */
			sum = pkt->api_identifier + pkt->source_address + pkt->rssi + pkt->option;
			
			for(count = 0; count< pkt->data_len; count++)
			{
				sum += pkt->data[count];
			}
			sum &= 0x00FF;
			sum = 0xFF - sum;
			
			if(pkt->checksum == sum)
				return 1;
			else
				return 0;
			
			break;
			
		case 0x83:
			/* CHECKSUM check for RX 16-Bit IO */
			sum = pkt->api_identifier + pkt->source_address + pkt->rssi + pkt->option;
			
			for(count = 0; count< pkt->data_len; count++)
			{
				sum += pkt->data[count];
			}
			sum &= 0x00FF;
			sum = 0xFF - sum;
			
			if(pkt->checksum == sum)
				return 1;
			else
				return 0;
			
			break;
			
		default:
			break;
	}// End of Switch
	
	
}


/******************************************************************************
* Function name: checksum
* Description  : This function generates the checksum for the API packet generated
* Arguments    : xbee_packet pointer
* Return value : none
******************************************************************************/
void checksum(xbee_packet *pkt){
	
	int sum, count;
	
	switch(pkt->api_identifier)
	{
		
		case 0x01:
			/* CHECKSUM for TX 16-Bit Request */
			sum = pkt->api_identifier + pkt->frame_id + pkt->DL + pkt->DH + pkt->option;
	
	
			for(count = 0; count< pkt->data_len; count++)
			{
				sum += pkt->data[count];
			}
	
			sum &= 0x00FF;
					
			sum = 0xFF - sum;
			pkt->checksum = sum;
			
			break;
			
		default:
			break;
			
	}// End of Switch
		
			
	
	
}


/******************************************************************************
* Function name: create_packet_16bit
* Description  : This function creates an API packet for TX Request (16-bit address).
				The paramaters passed in the argument are used to generate the packet.
* Arguments    : xbee_packet pointer, long, character pointer, character
* Return value : none
******************************************************************************/
void create_packet_16bit(xbee_packet *tx, long addr, char *data, char frameId){

	int addr_low, count;

	tx->api_identifier = 0x01;
	
	tx->DL = addr & 0x00FF;
		
	
	tx->DH = addr>>8;
	/*	sprintf(tt, "%x", tx->DH);
	lcd_display(LCD_LINE7, tt);*/

	
	tx->option = 0x01;
	tx->frame_id = 00;
	
	for(count = 0; count< strlen(data); count++)
	{		
		tx->data[count] = data[count];
	}
	
	tx->data_len = count;
	

	/*Entering length:
		API + FRAME ID+ DEST ADDR + OPTN + DATA LENGTH */
	tx->length = 1+1+2+1+count;
	
	checksum(tx);

}
/********************************************************************************/


/******************************************************************************
* Function name: tx_request_16_bit
* Description  : This function adds appropriate broadcast/data fields and address 
				in the generated packet. These packets are then stored in the buffer
				for transmission.
* Arguments    : xbee_packet pointer
* Return value : none
******************************************************************************/
void tx_request_16_bit(xbee_packet *tx){
	
	char address_MSB = tx->DH;
	char address_LSB = tx->DL;
	char broadcast_DATA[3];
	
	/* data for boradcast BLOCK */
	broadcast_DATA[0] = OPTION_BLOCK;
	broadcast_DATA[1] = address_MSB;
	broadcast_DATA[2] = address_LSB;
	
	xbee_packet tx_bc_blk;
	xbee_packet tx_bc_unblk;
	
	/* CREATING BROADCAST BLOCK */
	create_packet_16bit(&tx_bc_blk, 0xFFFF, broadcast_DATA, 7);
	transmit_add_packet_buffer(&tx_bc_blk); // BROADCAST_BLOCK --> BUFFER
	
	/* ADD DATA TO BUFFER */	
	transmit_add_packet_buffer(tx);// DATA_BLOCK --> BUFFER
	
	/* CREATING BROADCAST UNBLOCK */
	broadcast_DATA[0] = OPTION_UNBLOCK;
	create_packet_16bit(&tx_bc_unblk, 0xFFFF, broadcast_DATA, 7);

	transmit_add_packet_buffer(&tx_bc_unblk);// BROADCAST_BLOCK --> BUFFER
	
	/* IF MEDIUM IS FREE, TRANSMIT FROM BUFFER ELSE LET IT BE IN BUFFER */
	if(block_status == UNBLOCK)
	{
		
		transmit_buffer();
	}			
    
}// end of function
/********************************************************************************/

/******************************************************************************
* Function name: transmit_add_packet_buffer
* Description  : This function adds the received packet into buffer.
* Arguments    : xbee_packet pointer
* Return value : none
******************************************************************************/
void transmit_add_packet_buffer(xbee_packet *pkt){
	 
 	if(tx_buffer_size != BUFFER_SIZE)
	{
			transmit_packet_buffer[tx_buffer_tail++] = *pkt;
			tx_buffer_tail %= BUFFER_SIZE; // circular loop
			tx_buffer_size++;
	}		
}// end of function
/********************************************************************************/


/******************************************************************************
* Function name: transmit_get_packet_buffer
* Description  : This function dequeues the packet from buffer.
* Arguments    : none
* Return value : xbee_packet
******************************************************************************/
/* DEQUEUE FROM TRANSMIT BUFFER */
xbee_packet transmit_get_packet_buffer(){
		
		xbee_packet pkt;
		pkt = transmit_packet_buffer[tx_buffer_head];
		tx_buffer_head++;
		
		tx_buffer_head %= BUFFER_SIZE; // circular loop
		tx_buffer_size--;
		
		return pkt;
		
}// end of function
/********************************************************************************/


/******************************************************************************
* Function name: transmit_buffer
* Description  : This function is used to serially transmit the packet after some delay.
				It checks whether the medium is free or not. If the medium is busy, then
				it will bypass the if condition. This function will be called from 
				check_apiIdentifier() as soon as the medium is free.				
* Arguments    : none
* Return value : none
******************************************************************************/
void transmit_buffer(void){

	xbee_packet pkt;
	int length, count, i;
	char *frame;
	char display_text[15];	
	ACK_response = 0;	
	
	int ki = 0;
	char temp[2];

	for(count=0; count<ADDRESS-1000; count++)
		for(i=0; i<3200; i++);
			
  if(block_status == UNBLOCK)
  {
	
	sprintf(display_text, "%s", transmit_text);
	lcd_display(LCD_LINE1, display_text);
	//IEN(SCI6, RXI6) = 0;
	

	while(tx_buffer_size)
	{
		ki++;
		/*GET PACKET FROM BUFFER*/
			pkt = transmit_get_packet_buffer();
			
		
			length = pkt.length + 4; // ADDING START DELIMITER + LENGTH + CHECKUM.
	

			// CREATING FRAME SIZE..
			frame = (char *)calloc(length, sizeof(char));
	
			// LENGTH [MSB]
			frame[0] = 0x7E;
			frame[1] = pkt.length >> 8;
	
			// LENGTH [LSB]
			frame[2] = pkt.length & 0x00FF;

			// API IDENTIFIER [1 BYTE]
			frame[3] = pkt.api_identifier;
		
			// FRAME ID [1 BYTE]
			frame[4] = pkt.frame_id;
		
			// DESTINATION ADDRESS [MSB|LSB]
			frame[5] = pkt.DH;
			frame[6] = pkt.DL;
	
			// OPTION [1 BYTE]
			frame[7] = pkt.option;
		
			// DATA BYTE
			for(count = 0; count < pkt.data_len; count++)
			{
				frame[8+count] = pkt.data[count];
			}
	
			// CHECKSUM [1 BYTE]
			frame[8+count] = pkt.checksum;
			
			//////////////////////////////////////////////////////////
			/* SERIALLY TRANSMIT THE DATA */
			//if(block_status == BLOCK )
			
			
			for(i=0; i <= 8+count; i++)
			{	
				sci_put_char(frame[i]);
			//	printf("%x ", frame[i]);
			}
			//////////////////////////////////////////////////////////
		
			free(frame); // free memory for MALLOC created variable	
			
			for(count=0; count<32000; count++)
				for(int i=0; i<SPEED_TO_TRANSFER; i++);
	}
	sprintf(display_text, "%s", unblock_text);
	lcd_display(LCD_LINE1, display_text);
  }//end of IF
	//IEN(SCI6, RXI6) = 1;
}// end of function

/********************************************************************************/


/******************************************************************************
* Function name: get_packet_UART
* Description  : This function is used dequeue packet from received buffer. This
				function is generally called when rx_buffer_empty() returns 1.
				When a packet is received, it is stored in buffer.				
* Arguments    : none
* Return value : none
******************************************************************************/
void get_packet_UART(){

	
	int len, counter;
	/*msb*/
	while(!newDataAvailable());
	data_UART[0] = rx_UART();
	/*lsb*/
	while(!newDataAvailable());
	//data_UART[1] = rx_UART();
	len = rx_UART();
	
	for(counter = 2; counter <len+2; counter++)
	{
		while(!newDataAvailable());
		data_UART [counter] = rx_UART();
	}
	
	while(!newDataAvailable());
	data_UART[counter] = rx_UART();
	
	data_UART[1] = len;
	
	//check_apiIdentifier();
	char temp1[15];	
	sprintf(temp1, "%x %x %x ", data_UART[5], data_UART[6], data_UART[7]);
	lcd_display(LCD_LINE8, temp1);
}

/********************************************************************************/


/******************************************************************************
* Function name: check_apiIdentifier
* Description  : This function is used to store the received API packet into xbee_packet
				data type. The type of API packet is decided by checking the API Identifier.
				
				If the broadcast signal received is BLOCK, this function checks
				whether the desired destination address (in the data feild) matches with 
				the address of this node. If the address does not match, this node is blocked
				by setting block_status = 1.
				If the address matches, this function will broadcast a BLOCK ACKNOWLEDGMENT
				signal.
				
				If the broadcast signal received is UNBLOCK, this functions checks whether
				the desired destination address (in the data feild) matches with the address
				of this node. If the address does not match, this node is unblocked by setting
				block_status = 0.
				If the address matches, this function will broadcast a UNBLOCK ACKNOWLEDGMENT
				signal.
				
				Data [0:1]
				0x00 Broadcast Signal ACKNOWLEDGEMENT BLOCK
				0x01 Broadcast Signal BLOCK
				0x10 Broadcast Signal UNBLOCK
				0x11 Broadcast Signal ACKNOWLEGMENT UNBLOCK
				
* Arguments    : none
* Return value : none
******************************************************************************/
void check_apiIdentifier(){
	
	xbee_packet *rx;
		char temp1[10];	
	char recieve;
	char display_text[12];
	int counter;
	char *frame, *unblock_frame; char addr; int sum;	
	
	/* Recieve MSB of Length */
	rx->length = data_UART[0];
	
	
	/* Recieve LSB of Length */
	//while(!newDataAvailable());
	rx->length = rx->length | data_UART[1];
	
	/* Recieve API_IDENTIFIER */
	//while(!newDataAvailable());
	rx->api_identifier = data_UART[2];
	
/*	sprintf(temp1, "%x", rx->api_identifier);
	lcd_display(LCD_LINE8, temp1);*/


	switch(rx->api_identifier)
	{
	    /* RX PACKET 16 BIT */
      case 0x81:
		
			/* Recieve MSB Source ADDRESS */
			rx->source_address = data_UART[3];
			
			/* Recieve LSB Source ADDRESS */
			rx->source_address = (rx->source_address << 8) | data_UART[4];
			

			/* Recieve RSSI Byte */
			rx->rssi = data_UART[5];
			
			
			/* Recieve OPTION */
			rx->option = data_UART[6];
			
						
			/* Recieve DATA Bytes */
			rx->data_len = rx->length - 5;
			for(counter = 0; counter < rx->data_len; counter++ )
			{
				rx->data[counter] = data_UART[7+counter];
			}
			
			char disp[11];
			

			lcd_display(LCD_LINE6, "BLOCKED BY");
			sprintf(disp,"   %x",rx->source_address);
			lcd_display(LCD_LINE7, disp);
			
			/* Recieve CHECKSUM */
			rx->checksum = data_UART[7+counter];
			
	
			sprintf(disp,"=> %x %x %x",rx->data[0], rx->data[1], rx->data[2]);
			lcd_display(LCD_LINE8, disp);
	
				
				/***************************************
				CHECK FOR BLOCK OR UNBLOCK STATUS
				***************************************/
				/* IF BROADCAST OR NOT.. ELSE ADD TO BUFFER */
				if(rx->option == 0x02) // BROADCAST OR NOT
				{
							
					/* ADDRESS EQUAL TO THIS NODE ADDRESS */
					if((rx->data[1] !=	(ADDRESS>>8) & 0x00FF) || rx->data[2] != (ADDRESS & 0x00FF))
					{
						if(rx->data[0] == OPTION_ACK_OPEN)
						{
							block_status = BLOCK;
							sprintf(display_text, "%s", block_text);
							lcd_display(LCD_LINE1, display_text);
						}
						else if(rx->data[0] == OPTION_ACK_CLOSE)
						{
							block_status = UNBLOCK;
							transmit_buffer();
							sprintf(display_text, "%s", unblock_text);
							lcd_display(LCD_LINE1, display_text);
						}
						else if( rx->data[0] == OPTION_BLOCK)
						{
						
							block_status = BLOCK;
							sprintf(display_text, "%s", block_text);
							lcd_display(LCD_LINE1, display_text);
			
						}// end of if for BLOCK
							
						else if( rx->data[0] == OPTION_UNBLOCK)
						{	/* AS SOON AS MEDIUM IS FREE, SEND ALL PACKETS STORED IN THE BUFFER */
							block_status = UNBLOCK;
							lcd_display(LCD_LINE6, "");
							lcd_display(LCD_LINE7, "");
							transmit_buffer();
							sprintf(display_text, "%s", unblock_text);
							lcd_display(LCD_LINE1, display_text);
						}// end of if for UNBLOCK
					}// end of if for ADDRESS CHECK IN BROADCAST
					
					else /* MY ADDRESS */
					{
						if(rx->data[0] == OPTION_ACK_OPEN)
						{
							ACK_response = 1;
							// for future purpose..!! discard now
						}
						else if(rx->data[0] == OPTION_ACK_CLOSE)
						{
							// for future purpose..!! discard now
						}
						
						else if( rx->data[0] == OPTION_BLOCK)
						{
												
							block_status = BLOCK;
							sprintf(display_text, "%s", recieve_text);
							lcd_display(LCD_LINE1, display_text);
							
							
							/* TRANSMIT BLOCK ACK with data[0] = 0x00 */
													
							frame = (char *)calloc(12, sizeof(char));
							frame[0] = 0x7E;
							frame[1] = 0x00;
							frame[2] = 0x08;
							frame[3] = 0x01;
							frame[4] = 0x00; 
							frame[5] = 0xFF; 
							frame[6] = 0xFF;
							frame[7] = 0x01; 
							frame[8] = 0x00;
							addr = (rx->source_address >> 8);
							frame[9] = addr;
							addr = (rx->source_address || 0x00FF);
							frame[10]= addr;
							
							sum = 0;
							for(counter = 3; counter<= 10; counter++)
							{
								sum += frame[counter];
							}
	
							/* calculating checksum */
							sum &= 0x00FF;
							sum = 0xFF - sum;
							
							frame[11] = sum;
							
							for(counter=0; counter <= 11; counter++)
							{	
								sci_put_char(frame[counter]);
							}
							
							free(frame);
					
						}// end of if for BLOCK
							
						else if( rx->data[0] == OPTION_UNBLOCK)
						{	/* AS SOON AS MEDIUM IS FREE, SEND ALL PACKETS STORED IN THE BUFFER */
														
							/* TRANSMIT UNBLOCK ACK with data[0] = 0x11 */
							
							unblock_frame = (char *)calloc(12, sizeof(char));
							unblock_frame[0] = 0x7E;
							unblock_frame[1] = 0x00;
							unblock_frame[2] = 0x08;
							unblock_frame[3] = 0x01;
							unblock_frame[4] = 0x00;
							unblock_frame[5] = 0xFF; 
							unblock_frame[6] = 0xFF;
							unblock_frame[7] = 0x01; 
							unblock_frame[8] = 0x11;
							addr = (rx->source_address >> 8);
							unblock_frame[9] = addr;
							addr = (rx->source_address || 0x00FF);
							unblock_frame[10]= addr;
							
							sum = 0;
							for(counter = 3; counter<= 10; counter++)
							{
								sum += unblock_frame[counter];
							}
	
							/* calculating checksum */
							sum &= 0x00FF;
							sum = 0xFF - sum;
							
							unblock_frame[11] = sum;
							
							for(counter=0; counter <= 11; counter++)
							{	
								sci_put_char(unblock_frame[counter]);
							//	printf("%x ", frame[i]);
							}
							
							free(unblock_frame);
							
							block_status = UNBLOCK;
							transmit_buffer();
							sprintf(display_text, "%s", unblock_text);
							lcd_display(LCD_LINE1, display_text);
							lcd_display(LCD_LINE6, "");
							lcd_display(LCD_LINE7, "");
						
						}// end of if for UNBLOCK
					}
				}// end of if for BROADCAST or SIMPLE DATA PACKET
				else
				add_packet_buffer(rx);
		//	}
		//	else
		//	{
				/* DISCARD THE PACKET */
		//		lcd_display(LCD_LINE4, "Chcksum ERR");
		//	}
	
		
			break;
			
		/* RX PACKET 16 BIT ADDRESS "IO" */
		case 0x83:
	
			/* Recieve MSB Source ADDRESS */
			while(!newDataAvailable());
			rx->source_address = rx_UART();
			
			/* Recieve LSB Source ADDRESS */
			while(!newDataAvailable());
			rx->source_address = (rx->source_address << 8) | rx_UART();
			
			/* Recieve RSSI Byte */
			while(!newDataAvailable());	
			rx->rssi = rx_UART();
			
			/* Recieve OPTION */
			while(!newDataAvailable());	
			rx->option = rx_UART();
						
			/* Recieve DATA Bytes */
			rx->data_len = rx->length - 5;
			for(counter = 0; counter < rx->data_len; counter++ )
			{
				while(!newDataAvailable());
				rx->data[counter] = rx_UART();
			}
			
			/* Recieve CHECKSUM */
			while(!newDataAvailable());	
			rx->checksum = rx_UART();
			
			/* Verifying CHECKSUM */
			if(check_checksum(&rx))
			{
			//	valid_data_recieve = true;
				lcd_display(LCD_LINE5, "Chcksum OK");
				add_packet_buffer(rx);
				
			}
			else
			{
			//	valid_data_recieve = false;
				lcd_display(LCD_LINE5, "Chcksum ERR");
			}
			
			break;
		
		default:
			break;
			
	}// End of Switch

}// end of function


/******************************************************************************
* Function name: newDataAvailable
* Description  : This function returns 1 if any data is available in received buffer.
* Arguments    : none
* Return value : integer
******************************************************************************/
int newDataAvailable(void){
	
	//	SCI6.SSR.BIT.ORER = 0; /* Clear overrun error. Repeats until condition cleared. */                          
	
	
	if(SCI6.SSR.BYTE & 0x040)
	{
		return 1;
	}
	else 
	return 0;	
}


/******************************************************************************
* Function name: rx_UART
* Description  : This function returns data byte received in RDR (serial transmit).
* Arguments    : none
* Return value : character
******************************************************************************/
char rx_UART(void){
	//SCI6.SSR.BIT.ORER = 0;
	char data_rx;
	data_rx = SCI6.RDR;
	return data_rx;
}


/******************************************************************************
* Function name: add_packet_buffer
* Description  : This function adds packet to buffer.
* Arguments    : none
* Return value : xbee_packet pointer
******************************************************************************/
void add_packet_buffer(xbee_packet *pkt)
{
	if(rx_buffer_size != BUFFER_SIZE)
	{
			packet_buffer[rx_buffer_tail++] = *pkt;
			rx_buffer_tail %= BUFFER_SIZE; // circular loop
			rx_buffer_size++;
	}
}// end of function


/******************************************************************************
* Function name: get_packet_buffer
* Description  : This function dequeue the packet from buffer.
* Arguments    : none
* Return value : xbee_packet
******************************************************************************/
xbee_packet get_packet_buffer(){
	/*int rx_buffer_size = 0;
	int rx_buffer_head = 0;
	int rx_buffer_tail = 0;
	xbee_packet packet_buffer[BUFFER_SIZE];*/

	xbee_packet pkt;
	
	if(rx_buffer_size != 0)
	{
		pkt = packet_buffer[rx_buffer_head];
		rx_buffer_head++;
		
		rx_buffer_head %= BUFFER_SIZE; // circular loop
		rx_buffer_size--;
		
		return pkt;
	}
	
}// end of function


/******************************************************************************
* Function name: rx_buffer_empty
* Description  : This function check the buffer size and returns 1 if buffer is
				empty.
* Arguments    : none
* Return value : integer
******************************************************************************/
int	rx_buffer_empty(void){
	
	if(!rx_buffer_size)
	{
		return 0;
	}
	else
		return 1;
	
}// end of function
