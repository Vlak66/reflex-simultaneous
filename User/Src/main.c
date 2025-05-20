/*
********************************************************************************
* COPYRIGHT(c) ЗАО «ЧИП и ДИП», 2019, 2020
*
* Программное обеспечение предоставляется на условиях «как есть» (as is).
* При распространении указание автора обязательно.
********************************************************************************
*/

#include "usb_audio.h"
#include "audio_configuration.h"
#include "usbd_audio_if.h"
#include "usbd_desc.h"
#include "SPDIF_RX.h"
#include "SAI.h"

USBD_HandleTypeDef USBD_Device;
extern USBD_AUDIO_InterfaceCallbacksfTypeDef audio_class_interface;

int main(void)
{
  USB_I2S_Init();  // Инициализация USB и аудиоинтерфейса

  while (1)
  {
    __WFI();  // Переход в режим ожидания прерывания
  }
}

//------------------------------------------------------------------------------
// Инициализация системы, USB и аудио
void USB_I2S_Init(void)
{
  // Настройка приоритетов NVIC и системного таймера
  NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);
  HAL_InitTick(TICK_INT_PRIORITY);  // Инициализация таймера для HAL

  // Настройка тактирования (HSE + PLL для 168 МГц)
  FLASH->ACR = FLASH_ACR_LATENCY_5WS | FLASH_ACR_PRFTEN | FLASH_ACR_ICEN | FLASH_ACR_DCEN;
  RCC->CR |= RCC_CR_HSEON;  // Включение HSE
  while(!(RCC->CR & RCC_CR_HSERDY));  // Ожидание готовности HSE

  // Конфигурация PLL:
  // PLLSRC = HSE (8 МГц)
  // PLLM = 8 (8 МГц / 8 = 1 МГц)
  // PLLN = 168 (1 МГц * 168 = 168 МГц)
  // PLLP = 2 (168 МГц / 2 = 84 МГц для системной шины)
  // PLLQ = 7 (168 МГц / 7 = 24 МГц для USB OTG FS)
  RCC->PLLCFGR = RCC_PLLCFGR_PLLR_1 | RCC_PLLCFGR_PLLQ_0 | RCC_PLLCFGR_PLLQ_1 | RCC_PLLCFGR_PLLQ_2 |
                 RCC_PLLCFGR_PLLSRC | RCC_PLLCFGR_PLLN_4 | RCC_PLLCFGR_PLLN_6 | RCC_PLLCFGR_PLLN_8 | RCC_PLLCFGR_PLLM_3;

  // Настройка предделителей шин:
  // AHB: SYSCLK / 1 = 168 МГц
  // APB1: AHB / 4 = 42 МГц (макс. 42 МГц)
  // APB2: AHB / 2 = 84 МГц (макс. 84 МГц)
  RCC->CFGR = RCC_CFGR_PPRE2_DIV2 | RCC_CFGR_PPRE1_DIV4;

  RCC->CR |= RCC_CR_PLLON;  // Включение PLL
  while(!(RCC->CR & RCC_CR_PLLRDY));  // Ожидание готовности PLL

  RCC->CFGR |= RCC_CFGR_SW_PLL;  // Выбор PLL как источника системной частоты
  while((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);  // Ожидание переключения

  // Включение тактирования GPIO портов A, B, C
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN | RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOCEN;
  while(!(RCC->AHB1ENR & (RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOBEN | RCC_AHB1ENR_GPIOCEN)));

  // Инициализация GPIO (USB, I2S, индикаторы)
  ConfigGPIOs_Init();
  if (CONFIG_GPIO->IDR & (1 << CONFIG_1_PIN))
  {
    SetMode(USB_MODE);
    // Инициализация USB устройства
    USBD_Init(&USBD_Device, &AUDIO_Desc, 0);  // Основные дескрипторы
    AudioConfig_Init();  // Настройка аудиопараметров
    OUTClk_Init();       // Настройка тактирования для аудиоинтерфейса

    // Регистрация класса USB Audio и пользовательских обработчиков
    USBD_RegisterClass(&USBD_Device, USBD_AUDIO_CLASS);
    USBD_AUDIO_RegisterInterface(&USBD_Device, &audio_class_interface);
    USBD_Start(&USBD_Device);  // Запуск USB стека
  } else {
    SetMode(SPDIF_MODE);
    // SPDIF input mode
    AudioConfig_Init();
    OUTClk_Init();
    InitSPDIF();
    AudioOutInit(USB_AUDIO_CONFIG_FREQ_44_1_K, CONFIG_RES_BIT_24);
    SAI_MasterMute(1);
  }
}

//------------------------------------------------------------------------------
// Обработчик ошибок (пустой, требуется реализация)
void Error_Handler(void) { }

//------------------------------------------------------------------------------
// Обработчик ошибок USB (вызывает общий обработчик)
void USBD_error_handler(void) { Error_Handler(); }