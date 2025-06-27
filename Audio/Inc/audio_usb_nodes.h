/**
 ******************************************************************************
  * @file    audio_usb_nodes.h
  * @author  MCD Application Team
  * @brief   Заголовочный файл для audio_usb_nodes.c — содержит определения структур,
  *          констант и функций для работы с USB-аудио узлами.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * Все права защищены.</center></h2>
  *
  * Этот программный компонент лицензирован компанией ST под лицензией Ultimate Liberty
  * SLA0044, "License"; использовать этот файл можно только в соответствии с условиями лицензии.
  * Полный текст лицензии доступен по адресу: www.st.com/SLA0044
  *
  ******************************************************************************
  */

/* Защита от повторного включения -------------------------------------------*/
#ifndef __AUDIO_USB_NODES_H
#define __AUDIO_USB_NODES_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Подключаемые заголовочные файлы ------------------------------------------*/
#include  "usbd_audio.h"
#include  "audio_node.h"

/* Экспортируемые константы -------------------------------------------------*/

/* Максимальное количество поддерживаемых каналов аудио */
#define AUDIO_MAX_SUPPORTED_CHANNEL_COUNT 2    /* Поддержка стереозвука (2 канала) */

/* Флаги, используемые для управления потоком данных */
#define AUDIO_IO_BEGIN_OF_STREAM          0x01 /* Начало потока — устанавливается при получении первого пакета */
#define AUDIO_IO_RESTART_REQUIRED         0x40 /* Требуется перезапуск USB-узла, например, после изменения частоты */
#define AUDIO_IO_THRESHOLD_REACHED        0x08 /* Порог заполнения буфера достигнут — сигнал для начала чтения данных потребителем */

/* Экспортируемые типы данных -----------------------------------------------*/

/* Специфичные параметры входного USB-узла */
typedef struct
{
    uint16_t threshold; /* После начала воспроизведения USB-вход начинает получать пакеты и записывать их в циклический буфер.
                           Когда объём записанных данных достигает порога threshold, генерируется событие для сессии воспроизведения */
}AUDIO_USBInputSpecifcParams_t;

/* Специфичные параметры выходного USB-узла */
typedef struct
{
  uint8_t* alt_buff; /* Буфер, заполненный нулями, используется для отправки хосту, когда данные не готовы */
  
#if USB_AUDIO_CONFIG_RECORD_USE_FREQ_44_1_K
  uint8_t packet_44_counter;  /* Счётчик для отправки 9 пакетов по 44 семпла, затем одного пакета из 45 семплов */
#endif /* USB_AUDIO_CONFIG_RECORD_FREQ_44_1_K */
}AUDIO_USBOutputSpecifcParams_t;

/* Объединённая структура для входного/выходного USB-узла */
typedef struct
{
  AUDIO_Node_t               node; /* Общая структура узла — должна быть первой */
  uint8_t                    flags;/* Флаги состояния USB-ввода/вывода */
  AUDIO_CircularBuffer_t*    buf; /* Циклический буфер аудио-данных */
  uint16_t                   max_packet_length; /* Максимальная длина пакета для чтения */
  uint16_t                   packet_length; /* Нормальная длина пакета */
  
  /* Функции обратного вызова */
  int8_t  (*IODeInit) (uint32_t); /* Деинициализация узла */
  int8_t  (*IOStart) (AUDIO_CircularBuffer_t*, uint16_t, uint32_t); /* Запуск узла */
  int8_t  (*IORestart) (uint32_t); /* Перезапуск узла */
  int8_t  (*IOStop) (uint32_t); /* Остановка узла */

  /* Специфические параметры для входного или выходного узла */
  union
  {
    AUDIO_USBInputSpecifcParams_t  input;
    AUDIO_USBOutputSpecifcParams_t output;
  }specific;
}
AUDIO_USBInputOutputNode_t;

/* Структура команд для модуля управления функциями USB-аудио */
typedef struct
{
  int8_t  (*SetMute)    (uint16_t, uint8_t, uint32_t);     /* Установка состояния mute */
  uint32_t private_data;                                   /* Данные контекста */
} AUDIO_USBFeatureUnitCommands_t;

/* Структура значений по умолчанию для модуля управления громкостью */
typedef struct
{  
  AUDIO_Description_t       *audio_description; /* Указатель на описание аудио (частота, разрядность и т.д.) */
} AUDIO_USBFeatureUnitDefaults_t;

/* Структура узла модуля управления USB-функциями */
typedef struct
{
  AUDIO_Node_t node;                       /* Общая структура узла — должна быть первой */
  uint8_t unit_id;                         /* Идентификатор UNIT для описания USB-функции */
  USBD_AUDIO_FeatureControlCallbacksTypeDef usb_control_callbacks; /* Список callback-функций */
  AUDIO_USBFeatureUnitCommands_t control_cbks; /* Команды управления */
  
  /* Функции инициализации и управления */
  int8_t  (*CFInit)    (USBD_AUDIO_ControlTypeDef*, AUDIO_USBFeatureUnitDefaults_t*, uint8_t, uint32_t);
  int8_t  (*CFDeInit)  (uint32_t);
  int8_t  (*CFStart)   (AUDIO_USBFeatureUnitCommands_t*, uint32_t);
  int8_t  (*CFStop)    (uint32_t);
  int8_t  (*CFSetMute)    (uint16_t, uint8_t, uint32_t);
}
AUDIO_USB_CF_NodeTypeDef;

/* Макрос для расчёта максимальной длины пакета с учётом коррекции частоты */
#define AUDIO_MAX_PACKET_WITH_FEEDBACK_LENGTH(audio_desc) \
  AUDIO_USB_MAX_PACKET_SIZE((audio_desc)->frequency + 1, (audio_desc)->channels_count, (audio_desc)->resolution)

/* Экспортируемые функции --------------------------------------------------- */

/* Инициализация входящего потока аудио через USB (воспроизведение) */
#if USE_USB_AUDIO_PLAYBACK
int8_t  USB_AudioStreamingInputInit(USBD_AUDIO_EP_DataTypeDef* data_ep,
                                              AUDIO_Description_t* audio_desc,
                                              AUDIO_Session_t* session_handle,  uint32_t node_handle);
#endif /* USE_USB_AUDIO_PLAYBACK */

/* Инициализация узла управления функциями USB-аудио */
int8_t USB_AudioStreamingFeatureUnitInit(USBD_AUDIO_ControlTypeDef* usb_control_feature,
                                   AUDIO_USBFeatureUnitDefaults_t* audio_defaults, uint8_t unit_id,
                                   uint32_t node_handle);

/* Инициализация буфера данных для потоковой передачи аудио через USB */
void USB_AudioStreamingInitializeDataBuffer(AUDIO_CircularBuffer_t* buf, uint32_t buffer_size,
                                     uint16_t packet_size, uint16_t margin);

#ifdef __cplusplus
}
#endif

#endif  /* __AUDIO_USB_NODES_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/