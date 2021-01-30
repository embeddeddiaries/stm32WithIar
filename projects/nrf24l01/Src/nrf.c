#include "main.h"
#include "nrf.h"
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

extern SPI_HandleTypeDef hspi2;
void UART_SendStr(char *str);

#define NRF_PWRUP()     nrf_writeConfig(PWR_UP,1);
#define NRF_PWRDWN()    nrf_writeConfig(PWR_UP,0);

#define NRF_ASTX()        nrf_writeConfig(PRIM_RX,0);
#define NRF_ASRX()        nrf_writeConfig(PRIM_RX,1);



void nrf_delay(uint32_t ms){
  HAL_Delay(ms);
}

NRF_STATUS nrf_TransmitLL(uint8_t *data,uint16_t len)
{
  if(HAL_SPI_Transmit(&hspi2,data,len,HAL_MAX_DELAY) != HAL_OK){
      return NRF_FAIL;
  }  
  return NRF_SUCCESS;
}

NRF_STATUS nrf_ReceiveLL(uint8_t *data,uint16_t len)
{  
  if(HAL_SPI_Receive(&hspi2,data,len,HAL_MAX_DELAY) != HAL_OK){
      return NRF_FAIL;      
  }
  return NRF_SUCCESS;
}

NRF_STATUS nrf_cmd(uint8_t cmd){  
  return nrf_TransmitLL(&cmd,1);
}

NRF_STATUS nrf_receive(uint8_t reg,uint8_t *data,uint16_t len)
{  
  uint8_t cmd = reg | R_REGISTER;
  
  NRF_CSNLOW();

  if(!nrf_cmd(cmd)){
    return NRF_FAIL;
  }
  if(nrf_ReceiveLL(data,len) != NRF_SUCCESS)
    return NRF_FAIL;

  NRF_CSNHIGH();
  
  return NRF_SUCCESS;
}

NRF_STATUS nrf_send(uint8_t reg,uint8_t *data,uint16_t len)
{
  uint8_t cmd = reg | W_REGISTER;
  
  NRF_CSNLOW();
  
  if(!nrf_cmd(cmd)){
    return NRF_FAIL;
  }
  if(nrf_TransmitLL(data,len) != NRF_SUCCESS)
    return NRF_FAIL;

  NRF_CSNHIGH();
  
  return NRF_SUCCESS;
}


NRF_STATUS nrf_ReadAddr(uint8_t pipeRegAddr,uint8_t *addr,uint8_t len){
  
  if(nrf_receive(pipeRegAddr,addr,len) != NRF_SUCCESS){    
    return NRF_FAIL;
  }  
  return NRF_SUCCESS;
}


NRF_STATUS nrf_WriteAddr(uint8_t pipeRegAddr,uint8_t *addr,uint8_t len){
  
  if(nrf_send(pipeRegAddr,addr,len) != NRF_SUCCESS)
    return NRF_FAIL;
  
  return NRF_SUCCESS;
}

NRF_STATUS nrf_check(void){  
  NRF_STATUS ret = NRF_SUCCESS;
  
  uint8_t data[5] = {0};
  if(nrf_ReadAddr(RX_ADDR_P0,data,5) != NRF_SUCCESS)
          return NRF_FAIL;
  
//  uint8_t msg[50]={0};
//  for(int i=0;i<5;i++){
//    sprintf(msg,"%d\r\n",data[i]);
//    UART_SendStr(msg);
//  }
//  
  for(int i=0;i<5;i++){
    if(data[i] != 0xE7)
      return NRF_FAIL;
  }
    
  memset(data,0xAB,5);
  if(nrf_WriteAddr(RX_ADDR_P0,data,5) != NRF_SUCCESS)
    return NRF_FAIL;

//      for(int i=0;i<5;i++){
//    sprintf(msg,"%d\r\n",data[i]);
//    UART_SendStr(msg);
//  }

  for(int i=0;i<5;i++){
    if(data[i] != 0xAB)
      return NRF_FAIL;
  }  
  
  return ret;
}

uint8_t msg[100];
void nrf_writeConfig(uint8_t bit,bool en){
  uint8_t configVal;
  
  nrf_receive(CONFIG,&configVal,1);  

//    sprintf(msg,"%d\r\n",configVal);
//    UART_SendStr(msg);

  if(en)
    configVal |= bit;
  else
    configVal &= ~bit;
      
  nrf_send(CONFIG,&configVal,1);

//  nrf_receive(CONFIG,&configVal,1);  
//  sprintf(msg,"%d\r\n",configVal);
//  UART_SendStr(msg);
  
}

void nrfInit(void){
  
  //Spi is initilized
  //Gpio for CS, CSN and IRQ is initilized
  
  if(nrf_check() == NRF_FAIL){
    while(1);
  }
  
}

void nrf(void){
  
    if(nrf_check() != NRF_SUCCESS){
    UART_SendStr("nRF24L01+ Check - FAIL!!!\r\nReset the board\r\n");
    while(1);
  }
  UART_SendStr("nRF24L01+ Check - OKAY\r\n");
  NRF_PWRUP();
  nrf_delay(30);
  NRF_ASTX();
  
}


