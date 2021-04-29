/*
 * cracker.h
 *
 *  Created on: Apr 24, 2021
 *      Author: matic
 */

#ifndef INC_CRACKER_H_
#define INC_CRACKER_H_

#include "stm32f4xx_hal.h"

#define GET_MIN_CLK_SPEED

#define TEST_TARGET_SPEED 9600

#define TARGET_CLOCK_KHZ 4000
#define MAX_TARGET_CLKSPEED_KHZ 4000

#define TX_TIMEOUT 100 // Timeout for uart TX.
#define RX_TIMEOUT 1000 //Timeout for uart RX.

#define RX_TX_OK 0 //transmit_recieve_byte()  finished successfully.

#define BAUDRATE_CHANGE_OK 0 //set_baudrate() finished successfully.

#define CON_INIT_OK 0 // init_target_connection() finished successfully.
#define TRANSMIT_ERR 1 // Error during UART transmit.
#define NO_RESPONSE 2 // No response from target device.
#define INCORRECT_TARGET_RESPONSE 3 // Incorrect response from target device.
#define BAURATE_HANDSHAKE_FAILED 4 //Baud rate handshake failed
#define INCORRECT_BAUDRATE 5 // Incorrect baud rated selected for communication.
#define HW_BAUDRATE_CHANGE_ERR 6 // Hardware error while switching STM32 baud rate.
#define ERROR_AFTER_BAUDRATE_SWITCH 7 // Communication error after baud rate was switched.

uint32_t set_baudrate(UART_HandleTypeDef *huart, uint32_t baudrate);
uint32_t init_target_connection(UART_HandleTypeDef *huart);
uint32_t transmit_recieve_byte(UART_HandleTypeDef *huart, uint8_t transmitedByte, uint8_t expectedRxByte);
void target_reset(GPIO_TypeDef *port, uint16_t reset_pin, uint16_t mode_pin);
void send_one_key_byte(uint8_t byte, uint8_t byte_pos,  UART_HandleTypeDef *huart, TIM_HandleTypeDef *htim);
uint32_t read_and_reset_timer(TIM_HandleTypeDef *htim);
void set_target_clock_generator(TIM_HandleTypeDef *htim, uint32_t clock_in_KHz);


#endif /* INC_CRACKER_H_ */
