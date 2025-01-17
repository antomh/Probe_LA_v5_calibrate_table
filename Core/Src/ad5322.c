/*******************************************************************************
 * @File: DAC_AD5322.c
 * @Author: Kozlov-dev, A.
 * @Project: Probe_LA.c
 * @Microcontroller: STM32F103C8T
 * @Device: Probe_LA
 * @Date: 07.04.2020
 * @Purpose: Управление цифро-аналоговым преобразователем DAC_AD5322
 * @Datasheet : https://www.analog.com/media/en/technical-documentation/data-sheets/AD5302_5312_5322.pdf
 *******************************************************************************/

#include <stm32f1xx_hal.h>

#include <main.h>
#include <ad5322.h>


void ToggleLDAC() {
	HAL_GPIO_WritePin(AD5312_LDAC_GPIO_Port, AD5312_LDAC_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(AD5312_LDAC_GPIO_Port, AD5312_LDAC_Pin, GPIO_PIN_SET);
}



void SendSPI(SPI_HandleTypeDef *pSPI,uint16_t out) {
  HAL_GPIO_WritePin(AD5312_SYNC_GPIO_Port, AD5312_SYNC_Pin, GPIO_PIN_RESET);

  // Передача значений в цап
  //out	= 0b0100000111111111;
  HAL_SPI_Transmit(pSPI, (uint8_t*)(&out), 1, 10);

  // запепрет передачи CS
  HAL_GPIO_WritePin(AD5312_SYNC_GPIO_Port, AD5312_SYNC_Pin, GPIO_PIN_SET);
}



// Запуск цифро-аналогового преобразования канала А
void DAC_AD5322_Ch1(SPI_HandleTypeDef *pSPI, uint16_t data_ch1) {

	if (data_ch1 > 0x0FFF)	data_ch1	= 0x0FFF;

  uint16_t chan 		= 0;	// bit 15: 0 для канала A, 1 для канала B.
  uint16_t bufferVref = 1;	// bit 14: усилитель VREF?
  uint16_t PD1_Mode 	= 0;	// bit 13: PD1/PD0 Operating Modes   0  Normal Operation
  uint16_t PD0_Mode 	= 0;	// bit 12: PD1/PD0 Operating Modes   0  Normal Operation
  uint16_t out, tv;

  tv	= (chan << 15) | (bufferVref << 14) | (PD1_Mode << 13) | (PD0_Mode << 12);
  out = (tv & 0xF000) | (data_ch1 & 0x0FFF);
  SendSPI(pSPI,out);
  SendSPI(pSPI,out);
  ToggleLDAC();
}



// Запуск цифро-аналогового преобразования канала В
void DAC_AD5322_Ch2(SPI_HandleTypeDef *pSPI, uint16_t data_ch2) {

	if (data_ch2 > 0x0FFF)	data_ch2	= 0x0FFF;

  uint16_t chan 		= 1;	// bit 15: 0 для канала A, 1 для канала B.
  uint16_t bufferVref = 1;	// bit 14: усилитель VREF?
  uint16_t PD1_Mode 	= 0;	// bit 13: PD1/PD0 Operating Modes   0  Normal Operation
  uint16_t PD0_Mode 	= 0;	// bit 12: PD1/PD0 Operating Modes   0  Normal Operation
  uint16_t out, tv;

  tv	= (chan << 15) | (bufferVref << 14) | (PD1_Mode << 13) | (PD0_Mode << 12);
  out = (tv & 0xF000) | (data_ch2 & 0x0FFF);
  SendSPI(pSPI,out);
//  SendSPI(pSPI,out);
  ToggleLDAC();
}



void DAC_AD5322_Ch1Ch2(SPI_HandleTypeDef *pSPI, uint16_t data_ch1, uint16_t data_ch2) {

	if (data_ch1 > 0x0FFF)	data_ch1	= 0x0FFF;
	if (data_ch2 > 0x0FFF)	data_ch2	= 0x0FFF;

  uint16_t chan 		= 0;	  // bit 15: 0 для канала A, 1 для канала B.
  uint16_t bufferVref = 1;	// bit 14: усилитель VREF?
  uint16_t PD1_Mode 	= 0;	// bit 13: PD1/PD0 Operating Modes    0  Normal Operation
  uint16_t PD0_Mode 	= 0;	// bit 12: PD1/PD0 Operating Modes    0  Normal Operation
  uint16_t out, tv;

  tv	= (chan << 15) | (bufferVref << 14) | (PD1_Mode << 13) | (PD0_Mode << 12);
	out = (tv & 0xF000) | (data_ch1 & 0x0FFF);

  SendSPI(pSPI,out);
  SendSPI(pSPI,out);
  //--------------------------------------------------------------------------
  chan 		= 1;	    // bit 15: 0 для канала A, 1 для канала B.
  bufferVref 	= 1;	// bit 14: усилитель VREF?
  PD1_Mode 	= 0;	  // bit 13: PD1/PD0 Operating Modes    0  Normal Operation
  PD0_Mode 	= 0;	  // bit 12: PD1/PD0 Operating Modes    0  Normal Operation

  tv	= (chan << 15) | (bufferVref << 14) | (PD1_Mode << 13) | (PD0_Mode << 12);
  out = (tv & 0xF000) | (data_ch2 & 0x0FFF);
  SendSPI(pSPI,out);
  SendSPI(pSPI,out);
  ToggleLDAC();
}












