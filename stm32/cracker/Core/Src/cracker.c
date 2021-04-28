#include "cracker.h"

/*Try not to write blocking functions.*/

uint32_t read_and_reset_timer(TIM_HandleTypeDef *htim)
{
	// Read timer only if it is stopped. If it isn't stopped, stop and reset timer.
	if(htim->Instance->CCR1 != 0x00)
	{
		htim->Instance->CCR1 = 0;
		htim->Instance->CNT = 0x00000000;
		return 0;
	}

	uint32_t timer_count;
	timer_count = htim->Instance->CNT;

	//Reset the timer counter;
	htim->Instance->CNT = 0x00000000;
	return timer_count;
}

uint32_t did_timer_overflow(TIM_HandleTypeDef *htim){
	#TODO
	return 0;
}

void clean_rx_buffer(UART_HandleTypeDef *huart){
	/*Cleans (reads) the entire RX buffer*/
	uint8_t recieveBuffer[8];
	HAL_UART_Receive(huart, recieveBuffer, 8, RX_TIMEOUT);
}

uint32_t transmit_recieve_byte(UART_HandleTypeDef *huart, uint8_t transmitedByte, uint8_t expectedRxByte){
	/* Transmits a uart byte and checks if the response matches the desired one.*/
	uint8_t transmitBuffer[4];
	uint8_t recieveBuffer[4];

	clean_rx_buffer(huart);

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

	clean_rx_buffer(huart);

	return RX_TX_OK;
}

uint32_t init_target_connection(UART_HandleTypeDef *huart){
	/* This functions initializes the uart connection to target device. */
	uint32_t tx_rx_status;
	uint8_t transmitBuffer[4];

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

	if(HAL_UART_Transmit(huart, transmitBuffer, 1, TX_TIMEOUT) != HAL_OK){
		return TRANSMIT_ERR;
	}

	// Check if the baud rate switch was successful by asking target for bootloader version.
	// Bootloader version is returned as a string e.g. "VER.1.48", so the first responded byte should be 'V' or hex 0x56.
	tx_rx_status = transmit_recieve_byte(huart, 0xFB, 0x56);
	if(tx_rx_status != RX_TX_OK){
		return tx_rx_status;
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

void target_reset(GPIO_TypeDef *port, uint16_t reset_pin, uint16_t mode_pin){
	/*Reset the target and pull mode pin to low.*/
	HAL_GPIO_WritePin(port, mode_pin, GPIO_PIN_RESET);
//	HAL_Delay(500);
	HAL_GPIO_WritePin(port, reset_pin, GPIO_PIN_RESET);
	HAL_Delay(100);
	HAL_GPIO_WritePin(port, reset_pin, GPIO_PIN_SET);
	HAL_Delay(500);
}

void send_one_key_byte(uint8_t byte, uint8_t byte_pos,  UART_HandleTypeDef *huart, TIM_HandleTypeDef *htim){
	// byte_pos max value is 6!
	uint8_t transmitBuffer[2];

	//Initial comm part
	transmitBuffer[0] = 0x50;
	transmitBuffer[1] = 0x70;
	HAL_UART_Transmit(huart, transmitBuffer, 2, TX_TIMEOUT);
	HAL_Delay(100);
	transmitBuffer[0] = 0x70;
	HAL_UART_Transmit(huart, transmitBuffer, 1, TX_TIMEOUT);
	HAL_Delay(100);

	uint8_t header[] = {0xf5, 0xdf, 0xff, 0x00, 0x07};
	HAL_UART_Transmit(huart, header, sizeof(header), TX_TIMEOUT);

	HAL_Delay(10);
	// Reset the timer
	read_and_reset_timer(htim);

	uint8_t key[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x70};
	key[byte_pos] = byte;
	HAL_UART_Transmit(huart, key, sizeof(key), TX_TIMEOUT);
	HAL_Delay(100);

}

