/**
  ******************************************************************************
  * @file    USB_Device/AUDIO_EXT_Advanced_Player_Recorder/Inc/usbd_conf.h
  * @author  MCD Application Team
  * @brief   Конфигурация низкоуровневого драйвера USB
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Авторские права (c) 2019 STMicroelectronics.
  * Все права защищены.</center></h2>
  *
  * Этот программный компонент лицензирован компанией ST под лицензией Ultimate Liberty license
  * SLA0044, "Лицензия"; Вы не можете использовать этот файл вне рамок соблюдения
  * условий Лицензии. Копию Лицензии можно получить по адресу:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */

/* Определение для предотвращения рекурсивного включения --------------------*/
#ifndef __USBD_CONF_H
#define __USBD_CONF_H

/* Подключаемые файлы ------------------------------------------------------*/
#include "stm32f4xx_hal.h"       // HAL-библиотека для STM32F4
#include "hal_usb_ex.h"          // Расширенные функции USB HAL
#include "usb_audio.h"           // Определения класса USB Audio
#include <stdio.h>               // Стандартный ввод/вывод
#include <stdlib.h>              // Стандартные функции (malloc, free)
#include <string.h>              // Функции работы со строками
#include "audio_configuration.h" // Конфигурация аудиосистемы

/* Экспортируемые константы -----------------------------------------------*/
/* Общие настройки */
#define USBD_MAX_NUM_INTERFACES               1 // Макс. количество интерфейсов (1 для упрощения)
#define USBD_MAX_NUM_CONFIGURATION            1 // Макс. количество конфигураций USB
#define USBD_MAX_STR_DESC_SIZ                 0x100 // Макс. размер строковых дескрипторов
#define USBD_SUPPORT_USER_STRING              0 // Отключена поддержка пользовательских строк
#define USBD_SELF_POWERED                     0 // Устройство не автономное (питание от шины)
#define USBD_DEBUG_LEVEL                      0 // Уровень отладки (0 - без отладки)

// Поддержка обратной связи для аудиовоспроизведения
#define USBD_SUPPORT_AUDIO_OUT_FEEDBACK 1

// Поддержка множественных частот для аудио (только для класса Audio 1.0)
#if USE_USB_AUDIO_CLASS_10
#if (defined USE_AUDIO_USB_PLAY_MULTI_FREQUENCIES) || (defined USE_AUDIO_USB_RECORD_MULTI_FREQUENCIES)
#define USBD_SUPPORT_AUDIO_MULTI_FREQUENCIES 1
#endif
#endif

/* Конфигурация класса AUDIO */
/* Экспортируемые типы данных ---------------------------------------------*/
// В текущей версии дополнительные типы данных не требуются

/* Экспортируемые макросы -------------------------------------------------*/
/* Макросы управления памятью */
#define USBD_malloc               malloc // Выделение памяти
#define USBD_free                 free   // Освобождение памяти
#define USBD_memset               memset // Заполнение памяти
#define USBD_memcpy               memcpy // Копирование памяти

/* Макросы отладки */
#if (USBD_DEBUG_LEVEL > 0)
// Логирование пользователя
#define USBD_UsrLog(...)   printf(__VA_ARGS__);\
                           printf("\n");
#else
#define USBD_UsrLog(...)
#endif

#if (USBD_DEBUG_LEVEL > 1)
// Логирование ошибок
#define USBD_ErrLog(...)   printf("ОШИБКА: ");\
                           printf(__VA_ARGS__);\
                           printf("\n");
#else
#define USBD_ErrLog(...)
#endif

#if (USBD_DEBUG_LEVEL > 2)
// Отладочное логирование
#define USBD_DbgLog(...)   printf("ОТЛАДКА: ");\
                           printf(__VA_ARGS__);\
                           printf("\n");
#else
#define USBD_DbgLog(...)
#endif

/* Экспортируемые функции ----------------------------------------------- */
void USBD_error_handler(void); // Обработчик критических ошибок USB

#endif /* __USBD_CONF_H */