/**
 ******************************************************************************
  * @file    audio.h
  * @author  MCD Application Team
  * @version V4.0.1
  * @date    21-Июль-2015
  * @brief   Заголовочный файл содержит общие определения и прототипы функций драйвера аудио.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2015 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Защита от повторного включения -------------------------------------------*/
#ifndef __AUDIO_H
#define __AUDIO_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Подключаемые заголовочные файлы ------------------------------------------*/
#include <stdint.h>

/** @addtogroup BSP
  * @{
  */

/** @addtogroup Components
  * @{
  */

/** @addtogroup AUDIO
  * @{
  */

/** @defgroup AUDIO_Exported_Constants Экспортируемые константы
  * @{
  */

/* Стандарты кодеков */
#define CODEC_STANDARD                0x04 /*!< Используемый стандарт кодека */
#define I2S_STANDARD                  I2S_STANDARD_PHILIPS /*!< Стандарт I2S — Philips */

/**
  * @}
  */

/** @defgroup AUDIO_Exported_Types Экспортируемые типы данных
  * @{
  */

/** @defgroup AUDIO_Driver_structure Структура драйвера аудиоустройств
  * @{
  */

/**
  * @brief Структура драйвера аудиоустройства.
  *        Определяет интерфейс взаимодействия с аудиоустройством.
  */
typedef struct
{
  uint32_t  (*Init)(uint16_t, uint16_t, uint8_t, uint32_t);       /*!< Функция инициализации */
  void      (*DeInit)(void);                                      /*!< Деинициализация устройства */
  uint32_t  (*ReadID)(uint16_t);                                  /*!< Чтение идентификатора устройства */
  uint32_t  (*Play)(uint16_t, uint16_t*, uint16_t);               /*!< Начать воспроизведение */
  uint32_t  (*Pause)(uint16_t);                                   /*!< Пауза воспроизведения */
  uint32_t  (*Resume)(uint16_t);                                  /*!< Возобновить воспроизведение */
  uint32_t  (*Stop)(uint16_t, uint32_t);                          /*!< Остановить воспроизведение */
  uint32_t  (*SetFrequency)(uint16_t, uint32_t);                  /*!< Установка частоты дискретизации */
  uint32_t  (*SetVolume)(uint16_t, uint8_t);                      /*!< Установка громкости */
  uint32_t  (*SetMute)(uint16_t, uint32_t);                       /*!< Включение/выключение mute */
  uint32_t  (*SetOutputMode)(uint16_t, uint8_t);                  /*!< Выбор режима вывода звука */
  uint32_t  (*Reset)(uint16_t);                                   /*!< Сброс устройства */
}AUDIO_DrvTypeDef;

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* __AUDIO_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/