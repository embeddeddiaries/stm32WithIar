
#define ONEPAGE         16
#define EEPROMDEVADDR   0xA0

void i2cEepromInit(void);
HAL_StatusTypeDef eepromWriteIO(uint16_t DevAddress, uint16_t MemAddress, uint8_t *pData, uint16_t len);
HAL_StatusTypeDef eepromReadIO(uint16_t DevAddress, uint16_t MemAddress, uint8_t *pData, uint16_t len);

