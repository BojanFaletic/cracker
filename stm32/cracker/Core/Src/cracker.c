#include "cracker.h"

/*Try not to write blocking functions.*/

uint32_t transmit_recieve_byte(UART_HandleTypeDef *huart, uint8_t transmitedByte, uint8_t expectedRxByte){
	/* Transmits a uart byte and checks if the response matches the desired one.*/
	uint8_t transmitBuffer[4];
	uint8_t recieveBuffer[4];

	transmitBuffer[0] = transmitedByte;
	if(HAL_UART_Transmit(huart, transmitBuffer, 1, TX_TIMEOUT) != HAL_OK){
		return TRANSMIT_ERR;
	}
	if(HAL_UART_Receive(huart, recieveBuffer, 1, RX_TIMEOUT) != HAL_OK){
		return NO_RESPONSE;
	}
	if(recieveBuffer[0] != expectedRxByte)
	{
		return INCORRECT_TARGET_RESPONSE;
	}
	return RX_TX_OK;
}

uint32_t init_target_connection(UART_HandleTypeDef *huart){
	/* This functions initializes the uart connection to target device. */

	uint8_t transmitBuffer[4];
	uint8_t recieveBuffer[4];

	// First set the uart peripheral baud rate to 9600.
	huart->Init.BaudRate = 9600;
	if (HAL_UART_Init(huart) != HAL_OK){
	  return HW_BAUDRATE_CHANGE_ERR;
	}

	// Send the first 16x 0x00 bytes
	int i;
	for(i =0; i < 16; i++){
		transmitBuffer[0] = 0x00;
		if(HAL_UART_Transmit(huart, transmitBuffer, 1, TX_TIMEOUT) != HAL_OK){
			return TRANSMIT_ERR;
		}
		// Wait the required 20ms
		HAL_Delay(20);
	}

	// After sending another 0x00 the target should respond with a random byte
	if(HAL_UART_Transmit(huart, transmitBuffer, 1, TX_TIMEOUT) != HAL_OK){
		return TRANSMIT_ERR;
	}

	if(HAL_UART_Receive(huart, recieveBuffer, 1, RX_TIMEOUT) != HAL_OK){
		return NO_RESPONSE;
	}
	return CON_INIT_OK;
}

uint32_t set_baudrate(UART_HandleTypeDef *huart, uint32_t baudrate){
	/*Set the desired baud rate.*/
	uint32_t tx_rx_status;
	// Initiate the baud rate change
	tx_rx_status = transmit_recieve_byte(huart, 0xB0, 0xB0);
	if(tx_rx_status != RX_TX_OK){
		return tx_rx_status;
	}

	switch(baudrate){
		case 9600:
			tx_rx_status = transmit_recieve_byte(huart, 0xB0, 0xB0);
			break;

		case 19200:
			tx_rx_status = transmit_recieve_byte(huart, 0xB1, 0xB1);
			break;

		case 38400:
			tx_rx_status = transmit_recieve_byte(huart, 0xB2, 0xB2);
			break;

		case 57600:
			tx_rx_status = transmit_recieve_byte(huart, 0xB3, 0xB3);
			break;

		case 115200:
			tx_rx_status = transmit_recieve_byte(huart, 0xB4, 0xB4);
			break;

		default:
			return INCORRECT_BAUDRATE;
	}

	// Check if baud rate change handshake was successful.
	if(tx_rx_status != RX_TX_OK){
		return BAURATE_HANDSHAKE_FAILED;
	}
	// Now change the uart baud rate.
	huart->Init.BaudRate = baudrate;
	if (HAL_UART_Init(huart) != HAL_OK){
	  return HW_BAUDRATE_CHANGE_ERR;
	}

	// Check if the baud rate switch was successful by asking target for bootloader version.
	// Bootloader version is returned as a string e.g. "VER.1.48", so the first responded byte should be 'V' or hex 0x56.
	tx_rx_status = transmit_recieve_byte(huart, 0xFB, 0x56);
	if(tx_rx_status != RX_TX_OK){
		return ERROR_AFTER_BAUDRATE_SWITCH;
	}

	return BAUDRATE_CHANGE_OK;
}

