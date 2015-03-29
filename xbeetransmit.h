#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <machine.h>
#include "platform.h"
#include "uart.h"
#include "stdio.h"
#include "string.h"

#define DATA_SIZE 98
#define BUFFER_SIZE 20
#define BLOCK 1
#define UNBLOCK 0

/**************************************************************/
#define VICINITY 2
#define ADDRESS 0x2001
#define BROADCAST 0xFFFF
#define OPTION_DATA 0x00
#define OPTION_BLOCK 0x01
#define OPTION_UNBLOCK 0x10
#define OPTION_ACK_OPEN 0x00
#define OPTION_ACK_CLOSE 0x11

#define SPEED_TO_TRANSFER 750


/**************************************************************/



typedef struct{
	
	int length;
	long DH;
	long DL;
	char api_identifier;
	int source_address;
	char rssi;
	char option;
	char frame_id;
	char AT_com[2];
	char status;
	char data[DATA_SIZE];
	int data_len;	
	char checksum;		
}xbee_packet;

void tx_request_16_bit(xbee_packet *);
void init_new_packet(xbee_packet *);
void create_packet_16bit(xbee_packet *, long , char *, char );
void checksum(xbee_packet *);
int check_checksum(xbee_packet *);
void check_apiIdentifier();
char rx_UART(void);
int newDataAvailable(void);
void transmit_buffer(void); // transmit from buffer

int	rx_buffer_empty(void);
xbee_packet get_packet_buffer();
void add_packet_buffer(xbee_packet *);

void transmit_add_packet_buffer(xbee_packet *);
xbee_packet transmit_get_packet_buffer();


void get_packet_UART();

