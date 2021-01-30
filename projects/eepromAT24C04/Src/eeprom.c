#include "main.h"
#include "stdio.h"
#include <string.h>

#include "eeprom.h"

extern I2C_HandleTypeDef eepromI2c;


/*
 * brief: Used to write data array into specified EEPROM memory location
 * param: DevAddress: EEPROM device address
 * param: MemAddress: Memory address location of EEPROM that data needs to be written to.
 * param: pData: base address of data array that needs to be written in EEPROM
 * param: len: Total number of bytes need to write into EEPROM in bytes
 */

HAL_StatusTypeDef eepromWriteIO(uint16_t DevAddress, uint16_t MemAddress, uint8_t *pData, uint16_t len) 
{
  HAL_StatusTypeDef returnValue = HAL_ERROR;
  uint8_t data[18] = {0};
	
	uint16_t completed = 0;
	if(MemAddress + len < 4000)
	{
		while(len >= 16)
		{
			/* We compute the MSB and LSB parts of the memory address */
			data[1] = (uint8_t) ((MemAddress & 0xFF00) >> 8);
			data[0] = (uint8_t) (MemAddress & 0xFF);

			/* And copy the content of the pData array in the temporary buffer */
			memcpy(data+2, pData+completed, ONEPAGE);
			completed += ONEPAGE;
				
			returnValue = HAL_I2C_Master_Transmit(&eepromI2c, DevAddress, data, ONEPAGE+2, HAL_MAX_DELAY);
			if(returnValue != HAL_OK)
				return returnValue;

			len -= ONEPAGE;
			MemAddress += ONEPAGE;
			
			/* We wait until the EEPROM effectively stores data in memory */
			while(HAL_I2C_Master_Transmit(&eepromI2c, DevAddress, 0, 0, HAL_MAX_DELAY) != HAL_OK);
			returnValue = HAL_OK;
		
		}
		if(len > 0)
		{
			data[1] = (uint8_t) ((MemAddress & 0xFF00) >> 8);
			data[0] = (uint8_t) (MemAddress & 0xFF);

			/* And copy the content of the pData array in the temporary buffer */
			memcpy(data+2, pData+completed, len);
			
			returnValue = HAL_I2C_Master_Transmit(&eepromI2c, DevAddress, data, len+2, HAL_MAX_DELAY);
			if(returnValue != HAL_OK)
				return returnValue;
			
			while(HAL_I2C_Master_Transmit(&eepromI2c, DevAddress, 0, 0, HAL_MAX_DELAY) != HAL_OK);
			returnValue = HAL_OK;
		}
	}	
  return returnValue;
}



/*
 * brief: Used to read data from given EEPROM memory location
 * param: DevAddress: EEPROM device address
 * param: MemAddress: Memory address location of EEPROM where the data needs to be read from.
 * param: pData: Base address of data array where the data read from the EEPROM is stored
 * param: len: Total number of bytes that need to be read from EEPROM in bytes
 */

HAL_StatusTypeDef eepromReadIO(uint16_t DevAddress, uint16_t MemAddress, uint8_t *pData, uint16_t len) 
{
  HAL_StatusTypeDef returnValue;
  uint8_t addr[2];

  /* We compute the MSB and LSB parts of the memory address */
  addr[1] = (uint8_t) ((MemAddress & 0xFF00) >> 8);
  addr[0] = (uint8_t) (MemAddress & 0xFF);

  /* First we send the memory location address where start reading data */
  returnValue = HAL_I2C_Master_Transmit(&eepromI2c, DevAddress, addr, 2, HAL_MAX_DELAY);
  if(returnValue != HAL_OK)
    return returnValue;

  /* Next we can retrieve the data from EEPROM */
	while(HAL_I2C_Master_Receive(&eepromI2c, DevAddress, pData, len, HAL_MAX_DELAY)!= HAL_OK);
  return returnValue;
}


void i2cEepromInit(void)
{
	 /* Peripheral clock enable */
 __HAL_RCC_I2C1_CLK_ENABLE();

 eepromI2c.Instance = I2C1;
 eepromI2c.Init.ClockSpeed = 100000;
 eepromI2c.Init.DutyCycle = I2C_DUTYCYCLE_2;
 eepromI2c.Init.OwnAddress1 = 0x0;
 eepromI2c.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
 eepromI2c.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
 eepromI2c.Init.OwnAddress2 = 0;
 eepromI2c.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
 eepromI2c.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
 HAL_I2C_Init(&eepromI2c);
}

void HAL_I2C_MspInit(I2C_HandleTypeDef* hi2c)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(hi2c->Instance==I2C1)
  {
    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**I2C1 GPIO Configuration    
    PB8     ------> I2C1_SCL
    PB9     ------> I2C1_SDA 
    */
    GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
  }
}