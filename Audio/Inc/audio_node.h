/**
 ******************************************************************************
  * @file    audio_node.h
  * @author  MCD Application Team
  * @brief   Определяет аудио-узлы
 ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019  STMicroelectronics.
  * Все права защищены.</center></h2>
  *
  * Этот программный компонент лицензирован компанией ST по лицензии Ultimate Liberty
  * SLA0044, "License"; Вы не можете использовать этот файл, кроме как в соответствии с
  * Лицензией. Вы можете получить копию Лицензии по адресу:
  *                             www.st.com/SLA0044
  *
 ******************************************************************************
  */

/* Защита от повторного включения -------------------------------------*/
#ifndef __AUDIO_NODE_H
#define __AUDIO_NODE_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>

/* Экспортированные константы --------------------------------------------------------------*/
/* Экспортированные типы ------------------------------------------------------------------*/
/** 
  * @brief  Основная структура аудио-буфера (кольцевой буфер)
  */
typedef struct
{
  uint8_t*                   data; /* Указатель на данные кольцевого буфера */
  uint16_t                   rd_ptr;  /* Смещение чтения из кольцевого буфера */
  uint16_t                   wr_ptr;   /* Смещение записи в кольцевой буфер */
  uint16_t                   size;   /* Размер сегмента буфера, где могут быть прочитаны или записаны данные. 
                                      Может быть меньше реального размера буфера */
}
AUDIO_CircularBuffer_t;

/**
  * @brief  Состояния аудио-узла
  */
typedef enum
{
  AUDIO_NODE_OFF, /* Узел не инициализирован */
  AUDIO_NODE_INITIALIZED,
  AUDIO_NODE_STARTED,/* Узел запущен */
  AUDIO_NODE_STOPPED,
  AUDIO_NODE_ERROR
}
 AUDIO_NodeState_t;

/**
  * @brief Свойства аудио-потока
  */
typedef struct
{
  uint32_t           frequency;
  uint8_t            channels_count;   /* Количество аудиоканалов */
  uint16_t           channels_map;    /* Карта аудиоканалов в пространстве, см. спецификацию USB Audio класса */
  uint16_t           audio_type;
  int                audio_volume_db_256;/* Громкость, масштабированная по определению USB Audio */
  uint8_t            audio_mute;
  uint8_t            resolution;
}AUDIO_Description_t;

/**
  * @brief Перечисление типов узлов
  */
typedef enum
{
  AUDIO_INPUT,      /* Входной узел */
  AUDIO_OUTPUT,     /* Выходной узел */
  AUDIO_CONTROL,    /* Управляющий узел */
  AUDIO_PROCESSING  /* Обрабатывающий узел */
}
AUDIO_NodeType_t;

/**
  * @brief Базовая структура аудио-узла
  */
typedef struct    AUDIO_Node
{
  AUDIO_NodeState_t    state;
  AUDIO_Description_t* audio_description;
  AUDIO_NodeType_t     type;
  struct AUDIO_Session*     session_handle;  /* Основная сессия, к которой принадлежит этот узел */
  struct AUDIO_Node*        next;
}
AUDIO_Node_t;

/**
  * @brief События, генерируемые узлами в рамках сессии
  */
typedef enum
{
  AUDIO_THRESHOLD_REACHED,          /* Достигнут порог кольцевого буфера — достаточно данных для чтения */
  AUDIO_BEGIN_OF_STREAM,            /* Записан первый пакет в буфер */
  AUDIO_PACKET_RECEIVED,            /* Пакет получен с хоста через USB */
  AUDIO_PACKET_PLAYED,              /* Пакет воспроизведён динамиком */
  AUDIO_OVERRUN,                    /* Произошёл переполнение кольцевого буфера */
  AUDIO_UNDERRUN,                   /* Произошёл недоход кольцевого буфера */
  AUDIO_OVERRUN_TH_REACHED,         /* Достигнут порог переполнения — скоро произойдёт переполнение */
  AUDIO_UNDERRUN_TH_REACHED,        /* Достигнут порог недохода — скоро произойдёт недоход */
  AUDIO_FREQUENCY_CHANGED           /* Хост запросил изменение частоты дискретизации — нужно перезапустить узлы и сбросить буфер */
} AUDIO_SessionEvent_t;

/**
  * @brief Состояния аудио-сессии
  */
typedef enum
{
  AUDIO_SESSION_OFF,
  AUDIO_SESSION_INITIALIZED,
  AUDIO_SESSION_STARTED,
  AUDIO_SESSION_STOPPED,
  AUDIO_SESSION_ERROR
}
 AUDIO_SessionState_t;

/**
  * @brief Основная структура аудио-сессии
  */
typedef struct    AUDIO_Session
{
  AUDIO_Node_t * node_list; /* Список узлов, используемых сессией */
  AUDIO_SessionState_t state;
  int8_t  (*SessionCallback) (AUDIO_SessionEvent_t /* событие */ ,
                              AUDIO_Node_t* /* дескриптор узла */,
                              struct    AUDIO_Session* /* дескриптор сессии */); /* Callback вызывается узлами при возникновении событий, таких как overrun/underrun */
}AUDIO_Session_t;

// Узел для апсемплинга аудиоданных (увеличение частоты дискретизации)
typedef struct {
    AUDIO_Node_t node; // базовая структура узла
    // параметры для апсемплинга
    uint8_t upsample_factor; // например, 2 или 4
    // можно добавить поле для выбора метода
} AUDIO_UpsampleNode_t;

/* Экспортированные макросы -----------------------------------------------------------*/

/** 
  * @brief  Вычисляет свободное место в кольцевом буфере
  */
#define AUDIO_BUFFER_FREE_SIZE(buff)  (((buff)->wr_ptr>=(buff)->rd_ptr)?(buff)->rd_ptr +(buff)->size -(buff)->wr_ptr : \
                                                                          (buff)->rd_ptr -(buff)->wr_ptr)

/** 
  * @brief  Вычисляет количество занятых байт в кольцевом буфере
  */
#define AUDIO_BUFFER_FILLED_SIZE(buff)  (((buff)->wr_ptr>= (buff)->rd_ptr)?(buff)->wr_ptr -(buff)->rd_ptr : \
(buff)->wr_ptr +(buff)->size -(buff)->rd_ptr)

/** 
  * @brief  Вычисляет номинальный размер (в байтах) аудио-пакета на 1 мс
  * Например: для 48 кГц / 24 бита / стерео = 48 * 3 * 2 байт
  */
#define AUDIO_MS_PACKET_SIZE(freq,channel_count,res_byte) (((uint32_t)((freq) /1000))* (channel_count) * (res_byte))

/** 
  * @brief  Вычисляет максимальный возможный размер пакета на 1 мс (без учёта коррекции синхронизации)
  * Например: для 44.1 кГц / 16 бит / стерео = 45 * 2 * 2 байт
  */
#define AUDIO_MS_MAX_PACKET_SIZE(freq,channel_count,res_byte) AUDIO_MS_PACKET_SIZE(freq+999,channel_count,res_byte)

#ifdef USE_USB_HS
/** 
  * @brief  Вычисляет размер аудио-пакета в режиме High Speed USB (округление вниз)
  */
#define AUDIO_USB_PACKET_SIZE(freq,channel_count,res_byte) (((uint32_t)((freq) /8000))* (channel_count) * (res_byte))

/** 
  * @brief  Вычисляет максимальный размер аудио-пакета в режиме High Speed USB (округление вверх)
  */
#define AUDIO_USB_MAX_PACKET_SIZE(freq,channel_count,res_byte) AUDIO_USB_PACKET_SIZE(freq+7999,channel_count,res_byte)
#else /* USE_USB_HS */
/** 
  * @brief  Вычисляет размер аудио-пакета в режиме Full Speed USB (округление вниз)
  */
#define AUDIO_USB_PACKET_SIZE(freq,channel_count,res_byte) (((uint32_t)((freq) /1000))* (channel_count) * (res_byte))

/** 
  * @brief  Вычисляет максимальный размер аудио-пакета в режиме Full Speed USB (округление вверх)
  */
#define AUDIO_USB_MAX_PACKET_SIZE(freq,channel_count,res_byte) AUDIO_USB_PACKET_SIZE(freq+999,channel_count,res_byte)
#endif /* USE_USB_HS */

/** 
  * @brief  Макросы, использующие структуру AUDIO_Description_t как источник параметров
  */
#define AUDIO_USB_PACKET_SIZE_FROM_AUD_DESC(audio_desc) AUDIO_USB_PACKET_SIZE((audio_desc)->frequency, (audio_desc)->channels_count, (audio_desc)->resolution)
#define AUDIO_USB_MAX_PACKET_SIZE_FROM_AUD_DESC(audio_desc) AUDIO_USB_MAX_PACKET_SIZE((audio_desc)->frequency, (audio_desc)->channels_count, (audio_desc)->resolution)
#define AUDIO_MS_PACKET_SIZE_FROM_AUD_DESC(audio_desc) AUDIO_MS_PACKET_SIZE((audio_desc)->frequency, (audio_desc)->channels_count, (audio_desc)->resolution)
#define AUDIO_MS_MAX_PACKET_SIZE_FROM_AUD_DESC(audio_desc) AUDIO_MS_PACKET_SIZE((audio_desc)->frequency + 999, (audio_desc)->channels_count, (audio_desc)->resolution)

/** 
  * @brief  Вычисляет длину одного аудио-сэмпла в байтах
  */
#define AUDIO_SAMPLE_LENGTH(audio_desc) ( (audio_desc)->channels_count*(audio_desc)->resolution)

#ifdef __cplusplus
}
#endif
#endif  /* __AUDIO_NODE_H */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/