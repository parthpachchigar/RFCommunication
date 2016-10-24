/*
 ===============================================================================
 Name        : LISA.c
 Author      : Parth Pachchigar
 Version     : 1
 Description : Implementation of Linear Invariant Synchronization Algorithm
 ===============================================================================
 */

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif

#include <cr_section_macros.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#define S_BUFFER_SIZE 440
#define R_BUFFER_SIZE 1024
static int kernel[32][8] = { { 0, 1, 0, 1, 0, 0, 0, 0 }, { 0, 1, 0, 1, 0, 0, 0,
		1 }, { 0, 1, 0, 1, 0, 0, 1, 0 }, { 0, 1, 0, 1, 0, 0, 1, 1 }, { 0, 1, 0,
		1, 0, 1, 0, 0 }, { 0, 1, 0, 1, 0, 1, 0, 1 }, { 0, 1, 0, 1, 0, 1, 1, 0 },
		{ 0, 1, 0, 1, 0, 1, 1, 1 }, { 0, 1, 0, 1, 1, 0, 0, 0 }, { 0, 1, 0, 1, 1,
				0, 0, 1 }, { 0, 1, 0, 1, 1, 0, 1, 0 },
		{ 0, 1, 0, 1, 1, 0, 1, 1 }, { 0, 1, 0, 1, 1, 1, 0, 0 }, { 0, 1, 0, 1, 1,
				1, 0, 1 }, { 0, 1, 0, 1, 1, 1, 1, 0 },
		{ 0, 1, 0, 1, 1, 1, 1, 1 }, { 1, 0, 1, 0, 0, 0, 0, 0 }, { 1, 0, 1, 0, 0,
				0, 0, 1 }, { 1, 0, 1, 0, 0, 0, 1, 0 },
		{ 1, 0, 1, 0, 0, 0, 1, 1 }, { 1, 0, 1, 0, 0, 1, 0, 0 }, { 1, 0, 1, 0, 0,
				1, 0, 1 }, { 1, 0, 1, 0, 0, 1, 1, 0 },
		{ 1, 0, 1, 0, 0, 1, 1, 1 }, { 1, 0, 1, 0, 1, 0, 0, 0 }, { 1, 0, 1, 0, 1,
				0, 0, 1 }, { 1, 0, 1, 0, 1, 0, 1, 0 },
		{ 1, 0, 1, 0, 1, 0, 1, 1 }, { 1, 0, 1, 0, 1, 1, 0, 0 }, { 1, 0, 1, 0, 1,
				1, 0, 1 }, { 1, 0, 1, 0, 1, 1, 1, 0 },
		{ 1, 0, 1, 0, 1, 1, 1, 1 } };

void delay_ms(uint32_t delayInMs) {
	LPC_TIM0->TCR = 0x02;
	LPC_TIM0->PR = 0x00;
	LPC_TIM0->MR0 = delayInMs * 0.5 * (9000000 / 1000 - 1);
	LPC_TIM0->IR = 0xff;
	LPC_TIM0->MCR = 0x04;
	LPC_TIM0->TCR = 0x01;

	while (LPC_TIM0->TCR & 0x01)
		;
	return;
}
//Initialize the port and pin as inputs.
void GPIOinitIn(uint8_t portNum, uint32_t pinNum) {
	if (portNum == 0) {
		LPC_GPIO0->FIODIR &= ~(1 << pinNum);
	} else if (portNum == 1) {
		LPC_GPIO1->FIODIR &= ~(1 << pinNum);
	} else if (portNum == 2) {
		LPC_GPIO2->FIODIR &= ~(1 << pinNum);
	} else {
		puts("Not a valid port!\n");
	}
}

//Initialize the port and pin as outputs.
void GPIOinitOut(uint8_t portNum, uint32_t pinNum) {
	if (portNum == 0) {
		LPC_GPIO0->FIODIR |= (1 << pinNum);
	} else if (portNum == 1) {
		LPC_GPIO1->FIODIR |= (1 << pinNum);
	} else if (portNum == 2) {
		LPC_GPIO2->FIODIR |= (1 << pinNum);
	} else {
		puts("Not a valid port!\n");
	}
}

void setGPIO(uint8_t portNum, uint32_t pinNum) {
	if (portNum == 0) {
		LPC_GPIO0->FIOSET = (1 << pinNum);
		//printf("\nPin 0.%d has been set.", pinNum);
	}
	//Can be used to set pins on other ports for future modification
	else {
		//puts("\nOnly port 0 is used, try again!");
	}
}
//Deactivate the pin
void clearGPIO(uint8_t portNum, uint32_t pinNum) {
	if (portNum == 0) {
		LPC_GPIO0->FIOCLR = (1 << pinNum);
		//printf("\nPin 0.%d has been cleared.", pinNum);
	}
	//Can be used to clear pins on other ports for future modification
	else {
		//puts("\nOnly port 0 is used, try again!");
	}
}
int binToDeci(int binary[]) {
	int i, decimal = 0;
	for (i = 0; i < 8; i++) {

		decimal += binary[i] * pow(2, 7 - i);
	}
	return decimal;
}
void decodePayload(int recData[], int start, int stop) {
	int i, j, byte[8];
	printf("\n");
	for (i = start, j = 0; i < stop; i++, j++) {
		if (j == 8) {
			printf("%c", binToDeci(byte));
			j = 0;
		}
		byte[j] = recData[i];
	}
	printf("%c", binToDeci(byte));
}
int applyLisaEngine(int recData[]) {
	int i, j;
	for (i = 0; i < 31; i++) {
		for (j = 0; j < R_BUFFER_SIZE - 16; j++) {
			if ((kernel[i][0] == recData[j + 0])
					&& (kernel[i][1] == recData[j + 1])
					&& (kernel[i][2] == recData[j + 2])
					&& (kernel[i][3] == recData[j + 3])
					&& (kernel[i][4] == recData[j + 4])
					&& (kernel[i][5] == recData[j + 5])
					&& (kernel[i][6] == recData[j + 6])
					&& (kernel[i][7] == recData[j + 7])
					&& (kernel[i + 1][0] == recData[j + 8])
					&& (kernel[i + 1][1] == recData[j + 9])
					&& (kernel[i + 1][2] == recData[j + 10])
					&& (kernel[i + 1][3] == recData[j + 11])
					&& (kernel[i + 1][4] == recData[j + 12])
					&& (kernel[i + 1][5] == recData[j + 13])
					&& (kernel[i + 1][6] == recData[j + 14])
					&& (kernel[i + 1][7] == recData[j + 15])) {
				printf("Pattern matched (Confidence:2): %x %x",
						binToDeci(kernel[i]), binToDeci(kernel[i + 1]));
				return j + (32 - i) * 8;
			}
		}
	}
	return -1;
}
int payloadEnd(int recData[]) {
	int i, j, end_pattern[16] =
			{ 0, 0, 1, 0, 0, 0, 1, 1, 0, 0, 1, 0, 0, 0, 1, 1 }, flag = 1;
	for (i = 0; i < R_BUFFER_SIZE - 16; i++) {
		for (j = i; j < i + 16; j++) {
			if (end_pattern[j - i] != recData[j]) {
				flag = 0;
				break;
			}
		}
		if (flag == 1) {
			return i;
		} else {
			flag = 1;
		}
	}
	return -1;
}
void receiveAck() {
	GPIOinitIn(0, 2);
	if (LPC_GPIO0->FIOPIN0 & (1 << 2)) {
		printf("\nAck Receieved...");
	}
}
void send() {
	//Set pin 0.2 as output
	GPIOinitOut(0, 2);
	int p, sendData[S_BUFFER_SIZE] = { 0, 1, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0,
			0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 1, 0, 1, 0, 1, 0,
			1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 1, 0, 0, 1, 0, 1,
			0, 1, 1, 1, 0, 1, 0, 1, 1, 0, 0, 0, 0, 1, 0, 1, 1, 0, 0, 1, 0, 1, 0,
			1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 1, 1, 0, 1, 0, 1, 1, 1, 0, 0, 0, 1,
			0, 1, 1, 1, 0, 1, 0, 1, 0, 1, 1, 1, 1, 0, 0, 1, 0, 1, 1, 1, 1, 1, 1,
			0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 1, 1, 0, 1, 0, 0, 0, 1, 0,
			1, 0, 1, 0, 0, 0, 1, 1, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0,
			1, 1, 0, 1, 0, 0, 1, 1, 0, 1, 0, 1, 0, 0, 1, 1, 1, 1, 0, 1, 0, 1, 0,
			0, 0, 1, 0, 1, 0, 1, 0, 0, 1, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1,
			0, 1, 1, 1, 0, 1, 0, 1, 1, 0, 0, 1, 0, 1, 0, 1, 1, 0, 1, 1, 0, 1, 0,
			1, 1, 1, 0, 1, 0, 1, 0, 1, 1, 1, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0,
			0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 1,
			0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0,
			1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 1, 1, 0, 1, 0, 0, 1, 0, 0, 0,
			0, 1, 0, 0, 0, 0, 1, 1, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0,
			1, 0, 1, 0, 0, 0, 1, 1, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0,
			1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1,
			0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 0, 1, 0, 1, 0, 0, 1, 0,
			0, 0, 1, 1, 0, 0, 1, 0, 0, 0, 1, 1 };
	for (p = 0; p < S_BUFFER_SIZE; p++) {
		delay_ms(100);
		if (sendData[p] == 1) {
			setGPIO(0, 2);
		} else {
			clearGPIO(0, 2);
		}
	}
	receiveAck();
	/*printf("\nSent Data=\n");
	 for (p = 0; p < S_BUFFER_SIZE; p++) {
	 printf("%d", sendData[p]);
	 }*/
}
void sendAck() {
	GPIOinitOut(0, 3);
	setGPIO(0, 3);
}
void receive() {
	int payload_start, payload_end;
	//Set pin 0.3 as input
	GPIOinitIn(0, 3);

	int recData[R_BUFFER_SIZE], p;
	for (p = 0; p < R_BUFFER_SIZE; p++) {
		//printf("\nReceiving %d", p);
		delay_ms(10);
		if (LPC_GPIO0->FIOPIN0 & (1 << 3)) {
			recData[p] = 1;
		} else {
			recData[p] = 0;
		}

	}
	sendAck();
	//printf("\nReceived Data=\n");
	/*for (p = 0; p < R_BUFFER_SIZE; p++) {
	 printf("%d", recData[p]);
	 }
	 printf("\n");
	 */
	payload_start = applyLisaEngine(recData);
	if (payload_start != -1 && payload_start < R_BUFFER_SIZE) {
		printf("\nPayload starts at %d", payload_start);
	} else {
		printf("\nPayload Not Found...");
	}
	payload_end = payloadEnd(recData);
	if (payload_end != -1 && payload_end < R_BUFFER_SIZE) {
		printf("\nPayload ends at %d", payload_end);
	} else {
		printf("\nPayload End Not Found...");
	}
	if ((payload_start != -1 && payload_start < R_BUFFER_SIZE)
			&& (payload_end != -1 && payload_end < R_BUFFER_SIZE)) {
		decodePayload(recData, payload_start, payload_end);

	}
}
int main(void) {
	int choice;
	while (1) {
		printf("\nEnter your choice:\n1. Send\n2. Receive\n");
		scanf("%d", &choice);
		switch (choice) {
		case 1:
			send();
			break;
		case 2:
			receive();
			break;
		default:
			printf("\nInvalid Choice");
		}
	}
	//0 should never be returned, due to infinite while loop
	return 0;
}
