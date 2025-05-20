/**
  ******************************************************************************
  * @file    USB_Device/AUDIO_EXT_Advanced_Player_Recorder/Inc/stm32f4xx_hal_conf.h
  * @author  MCD Application Team
  * @brief   Конфигурационный файл HAL.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019  STMicroelectronics.
  * Все права защищены.</center></h2>
  *
  * Этот программный компонент лицензирован компанией ST по лицензии Ultimate Liberty,
  * SLA0044, "License"; Вы не можете использовать этот файл, кроме как в соответствии с
  * лицензией. Вы можете получить копию лицензии на следующем сайте:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* Определение для предотвращения рекурсивного подключения -------------------------------------*/
#ifndef __STM32F4xx_HAL_CONF_H
#define __STM32F4xx_HAL_CONF_H
#ifdef __cplusplus
 extern "C" {
#endif
/* Экспортированные типы ------------------------------------------------------------*/
/* Экспортированные константы --------------------------------------------------------*/
/* ########################## Выбор модулей ############################## */
/**
  * @brief Это список модулей, которые будут использоваться в драйвере HAL
  */
// Использовать callbacks
#define USE_HAL_TIM_REGISTER_CALLBACKS 1
#define USE_HAL_SPDIFRX_REGISTER_CALLBACKS 1
#define HAL_MODULE_ENABLED
/* #define HAL_ADC_MODULE_ENABLED */
/* #define HAL_CAN_MODULE_ENABLED */
/* #define HAL_CRC_MODULE_ENABLED */
/* #define HAL_CEC_MODULE_ENABLED */
/* #define HAL_CRYP_MODULE_ENABLED */
/* #define HAL_DAC_MODULE_ENABLED */
/* #define HAL_DCMI_MODULE_ENABLED */
#define HAL_DMA_MODULE_ENABLED
/* #define HAL_DMA2D_MODULE_ENABLED */
/* #define HAL_ETH_MODULE_ENABLED */
//#define HAL_FLASH_MODULE_ENABLED
/* #define HAL_NAND_MODULE_ENABLED */
/* #define HAL_NOR_MODULE_ENABLED */
/* #define HAL_PCCARD_MODULE_ENABLED */
//#define HAL_SRAM_MODULE_ENABLED
/* #define HAL_SDRAM_MODULE_ENABLED */
/* #define HAL_HASH_MODULE_ENABLED */
#define HAL_GPIO_MODULE_ENABLED
//#define HAL_I2C_MODULE_ENABLED
//#define HAL_I2S_MODULE_ENABLED
/* #define HAL_IWDG_MODULE_ENABLED */
/* #define HAL_LTDC_MODULE_ENABLED */
//#define HAL_PWR_MODULE_ENABLED
/* #define HAL_QSPI_MODULE_ENABLED */
#define HAL_RCC_MODULE_ENABLED
/* #define HAL_RNG_MODULE_ENABLED */
/* #define HAL_RTC_MODULE_ENABLED */
//#define HAL_SAI_MODULE_ENABLED
/* #define HAL_SD_MODULE_ENABLED */
/* #define HAL_SPI_MODULE_ENABLED */
#define HAL_TIM_MODULE_ENABLED
//#define HAL_UART_MODULE_ENABLED
/* #define HAL_USART_MODULE_ENABLED */
/* #define HAL_IRDA_MODULE_ENABLED */
/* #define HAL_SMARTCARD_MODULE_ENABLED */
/* #define HAL_WWDG_MODULE_ENABLED */
#define HAL_CORTEX_MODULE_ENABLED
#define HAL_PCD_MODULE_ENABLED
//#define HAL_HCD_MODULE_ENABLED
//#define HAL_FMPI2C_MODULE_ENABLED
#define HAL_SPDIFRX_MODULE_ENABLED
/* ########################## Настройка HSE/HSI ##################### */
/**
  * @brief Задайте значение внешнего высокочастотного осциллятора (HSE), используемого в вашем приложении.
  *        Это значение используется модулем RCC HAL для вычисления системной частоты
  *        (когда HSE используется в качестве источника системной частоты, напрямую или через PLL).
  */
#if !defined  (HSE_VALUE)
  #define HSE_VALUE    (8000000U) /*!< Значение внешнего осциллятора в Гц */
#endif /* HSE_VALUE */
#if !defined  (HSE_STARTUP_TIMEOUT)
  #define HSE_STARTUP_TIMEOUT    (100U)   /*!< Таймаут запуска HSE, в мс */
#endif /* HSE_STARTUP_TIMEOUT */
/**
  * @brief Значение внутреннего высокочастотного осциллятора (HSI).
  *        Это значение используется модулем RCC HAL для вычисления системной частоты
  *        (когда HSI используется в качестве источника системной частоты, напрямую или через PLL).
  */
#if !defined  (HSI_VALUE)
  #define HSI_VALUE    (16000000U) /*!< Значение внутреннего осциллятора в Гц */
#endif /* HSI_VALUE */
/**
  * @brief Значение внутреннего низкочастотного осциллятора (LSI).
  */
#if !defined  (LSI_VALUE)
 #define LSI_VALUE  (32000U)       /*!< Типичное значение LSI в Гц */
#endif /* LSI_VALUE */                      /*!< Реальное значение может отличаться в зависимости от 
                                             изменения напряжения и температуры.*/
/**
  * @brief Значение внешнего низкочастотного осциллятора (LSE).
  */
#if !defined  (LSE_VALUE)
 #define LSE_VALUE  (32768U)    /*!< Значение внешнего низкочастотного осциллятора в Гц */
#endif /* LSE_VALUE */
#if !defined  (LSE_STARTUP_TIMEOUT)
  #define LSE_STARTUP_TIMEOUT    (5000U)   /*!< Таймаут запуска LSE, в мс */
#endif /* LSE_STARTUP_TIMEOUT */
/**
  * @brief Внешний источник тактовой частоты для периферийного устройства I2S
  *        Это значение используется модулем I2S HAL для вычисления источника тактовой частоты I2S
  *        который подключается непосредственно через вывод I2S_CKIN.
  */
#if !defined  (EXTERNAL_CLOCK_VALUE)
  #define EXTERNAL_CLOCK_VALUE    (12288000U) /*!< Значение внешнего осциллятора в Гц */
#endif /* EXTERNAL_CLOCK_VALUE */
/* Совет: Чтобы избежать изменения этого файла каждый раз, когда вам нужно использовать другой HSE,
   ===  вы можете задать значение HSE в препроцессоре вашего компилятора. */
/* ########################### Конфигурация системы ######################### */
/**
  * @brief Здесь находится раздел конфигурации системы HAL
  */
#define  VDD_VALUE                    ((uint32_t)3300) /*!< Значение напряжения питания в мВ */
#define  TICK_INT_PRIORITY            ((uint32_t)0x00) /*!< Приоритет прерывания тика */
#define  USE_RTOS                     0
#define  PREFETCH_ENABLE              1
#define  INSTRUCTION_CACHE_ENABLE     1
#define  DATA_CACHE_ENABLE            1
/* ########################## Выбор утверждений ############################## */
/**
  * @brief Раскомментируйте строку ниже, чтобы расширить макрос "assert_param" в коде
  *        драйверов HAL
  */
/* #define USE_FULL_ASSERT    1 */
/* Includes ------------------------------------------------------------------*/
/**
  * @brief Подключение заголовочных файлов модулей
  */
#ifdef HAL_RCC_MODULE_ENABLED
  #include "stm32f4xx_hal_rcc.h"
#endif /* HAL_RCC_MODULE_ENABLED */
#ifdef HAL_GPIO_MODULE_ENABLED
  #include "stm32f4xx_hal_gpio.h"
#endif /* HAL_GPIO_MODULE_ENABLED */
#ifdef HAL_DMA_MODULE_ENABLED
  #include "stm32f4xx_hal_dma.h"
#endif /* HAL_DMA_MODULE_ENABLED */
#ifdef HAL_CORTEX_MODULE_ENABLED
  #include "stm32f4xx_hal_cortex.h"
#endif /* HAL_CORTEX_MODULE_ENABLED */
#ifdef HAL_ADC_MODULE_ENABLED
  #include "stm32f4xx_hal_adc.h"
#endif /* HAL_ADC_MODULE_ENABLED */
#ifdef HAL_CAN_MODULE_ENABLED
  #include "stm32f4xx_hal_can.h"
#endif /* HAL_CAN_MODULE_ENABLED */
#ifdef HAL_CRC_MODULE_ENABLED
  #include "stm32f4xx_hal_crc.h"
#endif /* HAL_CRC_MODULE_ENABLED */
#ifdef HAL_CRYP_MODULE_ENABLED
  #include "stm32f4xx_hal_cryp.h"
#endif /* HAL_CRYP_MODULE_ENABLED */
#ifdef HAL_DMA2D_MODULE_ENABLED
  #include "stm32f4xx_hal_dma2d.h"
#endif /* HAL_DMA2D_MODULE_ENABLED */
#ifdef HAL_DAC_MODULE_ENABLED
  #include "stm32f4xx_hal_dac.h"
#endif /* HAL_DAC_MODULE_ENABLED */
#ifdef HAL_DCMI_MODULE_ENABLED
  #include "stm32f4xx_hal_dcmi.h"
#endif /* HAL_DCMI_MODULE_ENABLED */
#ifdef HAL_ETH_MODULE_ENABLED
  #include "stm32f4xx_hal_eth.h"
#endif /* HAL_ETH_MODULE_ENABLED */
#ifdef HAL_FLASH_MODULE_ENABLED
  #include "stm32f4xx_hal_flash.h"
#endif /* HAL_FLASH_MODULE_ENABLED */
#ifdef HAL_SRAM_MODULE_ENABLED
  #include "stm32f4xx_hal_sram.h"
#endif /* HAL_SRAM_MODULE_ENABLED */
#ifdef HAL_NOR_MODULE_ENABLED
  #include "stm32f4xx_hal_nor.h"
#endif /* HAL_NOR_MODULE_ENABLED */
#ifdef HAL_NAND_MODULE_ENABLED
  #include "stm32f4xx_hal_nand.h"
#endif /* HAL_NAND_MODULE_ENABLED */
#ifdef HAL_PCCARD_MODULE_ENABLED
  #include "stm32f4xx_hal_pccard.h"
#endif /* HAL_PCCARD_MODULE_ENABLED */
#ifdef HAL_SDRAM_MODULE_ENABLED
  #include "stm32f4xx_hal_sdram.h"
#endif /* HAL_SDRAM_MODULE_ENABLED */
#ifdef HAL_HASH_MODULE_ENABLED
 #include "stm32f4xx_hal_hash.h"
#endif /* HAL_HASH_MODULE_ENABLED */
#ifdef HAL_I2C_MODULE_ENABLED
 #include "stm32f4xx_hal_i2c.h"
#endif /* HAL_I2C_MODULE_ENABLED */
#ifdef HAL_I2S_MODULE_ENABLED
 #include "stm32f4xx_hal_i2s.h"
#endif /* HAL_I2S_MODULE_ENABLED */
#ifdef HAL_IWDG_MODULE_ENABLED
 #include "stm32f4xx_hal_iwdg.h"
#endif /* HAL_IWDG_MODULE_ENABLED */
#ifdef HAL_LTDC_MODULE_ENABLED
 #include "stm32f4xx_hal_ltdc.h"
#endif /* HAL_LTDC_MODULE_ENABLED */
#ifdef HAL_PWR_MODULE_ENABLED
 #include "stm32f4xx_hal_pwr.h"
#endif /* HAL_PWR_MODULE_ENABLED */
#ifdef HAL_RNG_MODULE_ENABLED
 #include "stm32f4xx_hal_rng.h"
#endif /* HAL_RNG_MODULE_ENABLED */
#ifdef HAL_RTC_MODULE_ENABLED
 #include "stm32f4xx_hal_rtc.h"
#endif /* HAL_RTC_MODULE_ENABLED */
#ifdef HAL_SAI_MODULE_ENABLED
 #include "stm32f4xx_hal_sai.h"
#endif /* HAL_SAI_MODULE_ENABLED */
#ifdef HAL_SD_MODULE_ENABLED
 #include "stm32f4xx_hal_sd.h"
#endif /* HAL_SD_MODULE_ENABLED */
#ifdef HAL_SPI_MODULE_ENABLED
 #include "stm32f4xx_hal_spi.h"
#endif /* HAL_SPI_MODULE_ENABLED */
#ifdef HAL_TIM_MODULE_ENABLED
 #include "stm32f4xx_hal_tim.h"
#endif /* HAL_TIM_MODULE_ENABLED */
#ifdef HAL_UART_MODULE_ENABLED
 #include "stm32f4xx_hal_uart.h"
#endif /* HAL_UART_MODULE_ENABLED */
#ifdef HAL_USART_MODULE_ENABLED
 #include "stm32f4xx_hal_usart.h"
#endif /* HAL_USART_MODULE_ENABLED */
#ifdef HAL_IRDA_MODULE_ENABLED
 #include "stm32f4xx_hal_irda.h"
#endif /* HAL_IRDA_MODULE_ENABLED */
#ifdef HAL_SMARTCARD_MODULE_ENABLED
 #include "stm32f4xx_hal_smartcard.h"
#endif /* HAL_SMARTCARD_MODULE_ENABLED */
#ifdef HAL_WWDG_MODULE_ENABLED
 #include "stm32f4xx_hal_wwdg.h"
#endif /* HAL_WWDG_MODULE_ENABLED */
#ifdef HAL_PCD_MODULE_ENABLED
 #include "stm32f4xx_hal_pcd.h"
#endif /* HAL_PCD_MODULE_ENABLED */
#ifdef HAL_HCD_MODULE_ENABLED
 #include "stm32f4xx_hal_hcd.h"
#endif /* HAL_HCD_MODULE_ENABLED */
#ifdef HAL_QSPI_MODULE_ENABLED
 #include "stm32f4xx_hal_qspi.h"
#endif /* HAL_QSPI_MODULE_ENABLED */
#ifdef HAL_CEC_MODULE_ENABLED
 #include "stm32f4xx_hal_cec.h"
#endif /* HAL_CEC_MODULE_ENABLED */
#ifdef HAL_FMPI2C_MODULE_ENABLED
 #include "stm32f4xx_hal_fmpi2c.h"
#endif /* HAL_FMPI2C_MODULE_ENABLED */
#ifdef HAL_SPDIFRX_MODULE_ENABLED
 #include "stm32f4xx_hal_spdifrx.h"
#endif /* HAL_SPDIFRX_MODULE_ENABLED */
/* Экспортированные макросы ------------------------------------------------------------*/
#ifdef  USE_FULL_ASSERT
/**
  * @brief Макрос assert_param используется для проверки параметров функций.
  * @param expr: Если expr равно false, вызывается функция assert_failed,
  *         которая сообщает имя исходного файла и номер строки, где произошла ошибка.
  *         Если expr равно true, возвращается no value.
  * @retval None
  */
  #define assert_param(expr) ((expr) ? (void)0 : assert_failed((uint8_t *)__FILE__, __LINE__))
/* Экспортированные функции ------------------------------------------------------- */
  void assert_failed(uint8_t* file, uint32_t line);
#else
  #define assert_param(expr) ((void)0U)
#endif /* USE_FULL_ASSERT */
#ifdef __cplusplus
}
#endif
#endif /* __STM32F4xx_HAL_CONF_H */
/************************ (C) COPYRIGHT STMicroelectronics *****КОНЕЦ ФАЙЛА****/

