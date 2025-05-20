/**
 ******************************************************************************
  * @file    audio_speaker_node.h
  * @author  MCD Application Team
  * @brief   Заголовочный файл для файла audio_speaker_node.c.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * Все права защищены.</center></h2>
  *
  * Этот программный компонент лицензирован STMicroelectronics по лицензии Ultimate Liberty,
  * SLA0044, "License"; использовать этот файл можно только в соответствии с условиями лицензии.
  * Полный текст лицензии можно найти по адресу: www.st.com/SLA0044
  *
 ******************************************************************************
  */

/* Защита от повторного включения -------------------------------------------*/
#ifndef __AUDIO_SPEAKER_NODES_H
#define __AUDIO_SPEAKER_NODES_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Подключаемые заголовочные файлы ------------------------------------------*/

#if !USE_AUDIO_SPEAKER_DUMMY
#include "audio_user_devices.h"
#endif /* USE_AUDIO_SPEAKER_DUMMY */
#include  "audio_node.h"
#include "usb_audio.h"

/* Макросы конфигурации устройства вывода звука */
#define OUTPUT_DEVICE_AUTO                    ((uint16_t)0x0004)
#define CODEC_PDWN_SW                 2

/* Экспортируемые константы -------------------------------------------------*/
/* VOLUME_SPEAKER_RES_DB_256 — разрешение регулировки громкости в децибелах, см. спецификацию UAC */
#define VOLUME_SPEAKER_RES_DB_256       128     /* 0.5 дБ * 256 = 128 */
#define VOLUME_SPEAKER_DEFAULT_DB_256   0       /* По умолчанию 0 дБ */
#define VOLUME_SPEAKER_MAX_DB_256       1536    /* Максимум +6 дБ == 6*256 = 1536 */
#define VOLUME_SPEAKER_MIN_DB_256       -6400   /* Минимум -25 дБ == -25*256 = -6400 */

/* Настройки управления громкостью через таймер */
#if USE_AUDIO_TIMER_VOLUME_CTRL
/* SPEAKER_CMD_CHANGE_VOLUME — сигнал для таймера: требуется изменить уровень громкости */
#define SPEAKER_CMD_CHANGE_VOLUME  0x10
/* SPEAKER_CMD_MUTE_UNMUTE — сигнал для таймера: требуется изменить состояние mute */
#define SPEAKER_CMD_MUTE_UNMUTE    0x20
/* SPEAKER_CMD_MUTE_FIRST — указывает таймеру сначала применить mute, потом изменить громкость */
#define SPEAKER_CMD_MUTE_FIRST     0x40
#endif /* USE_AUDIO_TIMER_VOLUME_CTRL */

/* Выбор реализации инициализации динамика в зависимости от конфигурации */
#ifdef USE_AUDIO_SPEAKER_DUMMY
#define  AUDIO_SpeakerInit AUDIO_SPEAKER_DUMMY_Init
#else /* USE_AUDIO_SPEAKER_DUMMY */
#define  AUDIO_SpeakerInit AUDIO_SPEAKER_USER_Init
#endif /* USE_AUDIO_SPEAKER_DUMMY */

/* Экспортируемые типы данных -----------------------------------------------*/

/* Структура для работы с фиктивным динамиком */
#ifdef USE_AUDIO_SPEAKER_DUMMY
typedef struct
{
  int8_t dummy;    /* Не используется */
}AUDIO_SpeakerSpecificParms_t;
#endif /* USE_AUDIO_SPEAKER_DUMMY */

/* Основная структура узла динамика */
typedef struct
{
  AUDIO_Node_t              node;            /* Общая структура узла */
  AUDIO_CircularBuffer_t*   buf;             /* Буфер аудио-данных */
  uint16_t                  packet_length;   /* Максимальная длина пакета */
  
  /* Дополнительные поля для частоты 44.1 кГц */
#if USB_AUDIO_CONFIG_PLAY_USE_FREQ_44_1_K
  uint16_t               packet_length_max_44_1; /* Макс. длина пакета при 44.1 кГц */
  uint8_t                injection_44_count;     /* Счётчик для вставки 9 пакетов по 44 семпла */
  uint8_t                injection_45_pos;       /* Позиция пакета из 45 семплов */
#endif /* USB_AUDIO_CONFIG_PLAY_USE_FREQ_44_1_K */

  /* Функции обратного вызова для работы с динамиком */
  int8_t                (*SpeakerDeInit)  (uint32_t); /* Деинициализация */
  int8_t                (*SpeakerStart)   (AUDIO_CircularBuffer_t*, uint32_t);
  int8_t                (*SpeakerStop)    (uint32_t);
  int8_t                (*SpeakerChangeFrequency) (uint32_t);
  int8_t                (*SpeakerMute)    (uint16_t, uint8_t, uint32_t);
  int8_t                (*SpeakerSetVolume)    (uint16_t, int, uint32_t);
  int8_t                (*SpeakerStartReadCount) (uint32_t);
  uint16_t              (*SpeakerGetReadCount)   (uint32_t);

  /* Пользовательские параметры динамика */
  AUDIO_SpeakerSpecificParms_t specific;

  /* Функции воспроизведения и подготовки данных */
  void                  (*SpeakerPlay)    (uint16_t*, uint16_t, uint8_t);
  void                  (*SpeakerPrepareData) (uint8_t*, uint16_t, uint8_t);
}
AUDIO_SpeakerNode_t;

/* Экспортируемые макросы ---------------------------------------------------*/
/* Экспортируемые функции --------------------------------------------------*/
int8_t  AUDIO_SpeakerInit(AUDIO_Description_t* audio_description,
                          AUDIO_Session_t* session_handle,
                          uint32_t node_handle);

#ifdef __cplusplus
}
#endif
#endif  /* __AUDIO_SPEAKER_NODES_H */

/* Прототипы callback-функций BSP */
void BSP_AUDIO_OUT_Error_CallBack(void);
void BSP_AUDIO_OUT_TransferComplete_CallBack(void);

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

/* Дополнительные функции */
void AUDIO_SpeakerChangeResolution(uint32_t node_handle);
int8_t AUDIO_SpeakerChangeFrequency(uint32_t node_handle);