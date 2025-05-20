/**
  ******************************************************************************
  * @file    usbd_audio.h
  * @author  MCD Application Team
  * @brief   заголовочный файл для файла usbd_audio.c, это новая реализация класса USB Audio,
  * поддерживающая больше функций.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * Все права защищены.</center></h2>
  *
  * Этот программный компонент лицензирован компанией ST по лицензии Ultimate Liberty
  * SLA0044, "License"; Вы можете использовать этот файл только в соответствии с
  * лицензией. Копию лицензии можно получить по адресу:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* Определение для предотвращения рекуррентного включения -------------------------------------*/
#ifndef __USB_AUDIO_H
#define __USB_AUDIO_H
#ifdef __cplusplus
 extern "C" {
#endif
/* Includes ------------------------------------------------------------------*/
#include  "usbd_ioreq.h"
/** @addtogroup STM32_USB_DEVICE_LIBRARY
  * @{
  */
/** @defgroup USBD_AUDIO
  * @brief Это заголовочный файл для usbd_audio.c
  * @{
  */
/** @defgroup USBD_AUDIO_Exported_Defines
  * @{
  */
 #define USBD_AUDIO_ADC_BCD                                           0x0100
 #define USBD_AUDIO_CLASS_CODE                                        0x01
/* Коды подклассов аудиоинтерфейса */
#define USBD_AUDIO_INTERFACE_SUBCLASS_AUDIOCONTROL                   0x01
#define USBD_AUDIO_INTERFACE_SUBCLASS_AUDIOSTREAMING                 0x02
#define USBD_AUDIO_INTERFACE_SUBCLASS_MIDISTREAMING                  0x03
/* Коды протоколов аудиоинтерфейса  */
#define USBD_AUDIO_INTERFACE_PROTOCOL_UNDEFINED                      0x00
/* Таблица A-2: Битовые распределения формата данных аудио типа I */
#define USBD_AUDIO_FORMAT_TYPE_PCM                                   0x0001
/* Таблица A-1: Коды типов форматов */
#define USBD_AUDIO_FORMAT_TYPE_I                                     0x01
#define USBD_AUDIO_FORMAT_TYPE_II                                    0x02
#define USBD_AUDIO_FORMAT_TYPE_III                                   0x03
/* Типы дескрипторов аудиоустройств */
#define USBD_AUDIO_DESC_TYPE_CS_DEVICE                               0x21
#define USBD_AUDIO_DESC_TYPE_CS_INTERFACE                            0x24
#define USBD_AUDIO_DESC_TYPE_CS_ENDPOINT                             0x25
#define USBD_AUDIO_DESC_TYPE_INTERFACE_ASSOC                         0x0B /* Дескриптор ассоциации интерфейсов */
    /* размер специфичных аудиодескрипторов */
#define USBD_AUDIO_STANDARD_INTERFACE_DESC_SIZE                      0x09
#define USBD_AUDIO_INTERFACE_ASSOC_DESC_SIZE                         0x08
#define USBD_AUDIO_DESC_SIZ                                           0x09
#define USBD_AUDIO_STANDARD_ENDPOINT_DESC_SIZE                       0x09
#define USBD_AUDIO_SPECIFIC_DATA_ENDPOINT_DESC_SIZE                  0x07
#define USBD_AUDIO_INPUT_TERMINAL_DESC_SIZE                          0x0C
#define USBD_AUDIO_FEATURE_UNIT_DESC_SIZE(CH_NB,CTRSIZE)            (0x07+(((CH_NB)+1)*(CTRSIZE)))
#define USBD_AUDIO_OUTPUT_TERMINAL_DESC_SIZE                         0x09
#define USBD_AUDIO_AC_CS_INTERFACE_DESC_SIZE(AS_CNT)                 (0x08 + (AS_CNT))
#define USBD_AUDIO_AS_CS_INTERFACE_DESC_SIZE                         0x07
#define USBD_USBD_AUDIO_FORMAT_TYPE_I_DESC_SIZE(NBFREQ)              (0x08 + (NBFREQ)*3)
/* Класс-специфичные поля bmAttributes изохронного аудио-ендпоинта AS */
#define USBD_AUDIO_AS_CONTROL_SAMPLING_FREQUENCY             0x0001 /* D0 = 1 */
#define USBD_AUDIO_AS_CONTROL_PITCH                          0x0002 /* D1 = 1 */
#define USBD_AUDIO_AS_CONTROL_MAX_PACKET_ONLY                0x0080 /* D7 = 1 */
/* Подтипы класс-специфичных дескрипторов конечных точек аудио */
#define USBD_AUDIO_SPECIFIC_EP_DESC_SUBTYPE_GENERAL                  0x01 /* EP_GENERAL */
#define USBD_EP_ATTR_ISOC_NOSYNC                          0x00 /* режим без синхронизации */
#define USBD_EP_ATTR_ISOC_ASYNC                           0x04 /* режим с обратной связью */
#define USBD_EP_ATTR_ISOC_ADAPT                           0x08 /* адаптивная синхронизация */
#define USBD_EP_ATTR_ISOC_SYNC                            0x0C /* строгая синхронизация */
/* ЗАПРОСЫ КЛАССА USB AUDIO BREQUEST TYPES */
#define USBD_AUDIO_REQ_SET_CUR                             0x01
#define USBD_AUDIO_REQ_GET_CUR                             0x81
#define USBD_AUDIO_REQ_SET_MIN                             0x02
#define USBD_AUDIO_REQ_GET_MIN                             0x82
#define USBD_AUDIO_REQ_SET_MAX                             0x03
#define USBD_AUDIO_REQ_GET_MAX                             0x83
#define USBD_AUDIO_REQ_SET_RES                             0x04
#define USBD_AUDIO_REQ_GET_RES                             0x84
#define USBD_AUDIO_REQ_SET_MEM                             0x05
#define USBD_AUDIO_REQ_GET_MEM                             0x85
#define USBD_AUDIO_REQ_GET_STAT                            0xFF
/* Управление Feature Unit */
#define USBD_AUDIO_CONTROL_FEATURE_UNIT_MUTE          0x01
#define USBD_AUDIO_CONTROL_FEATURE_UNIT_VOLUME        0x02
  /* Селекторы управления Feature Unit */
#define USBD_AUDIO_FU_MUTE_CONTROL                                    0x01
#define USBD_AUDIO_FU_VOLUME_CONTROL                                  0x02
/* определение управляющих элементов конечной точки */
#define USBD_AUDIO_CONTROL_EP_SAMPL_FREQ               0x01 /* Управление частотой дискретизации */
#define USBD_AUDIO_CONTROL_EP_PITCH                   0x02 /* Управление тоном/высотой звука */
/* Конфигурация текущей реализации аудиокласса */
#define USBD_AUDIO_AS_INTERFACE_COUNT 1//0x02
#define USBD_AUDIO_MAX_IN_EP  2 //5 including EP0
#define USBD_AUDIO_MAX_OUT_EP 2 //5 including EP0
//#define USBD_AUDIO_MAX_AS_INTERFACE 2
//#define USBD_AUDIO_EP_MAX_CONTROL 3
#define USBD_AUDIO_CONFIG_CONTROL_UNIT_COUNT 1//0x02
//#define USBD_AUDIO_FEATURE_MAX_CONTROL 2
#define AUDIO_FEEDBACK_EP_PACKET_SIZE                 0x03
/**
  * @}
  */
/** @defgroup USBD_AUDIO_Exported_TypesDefinitions
  * @{
  */
/* Подтипы дескрипторов интерфейса управления аудио */
typedef enum
{
  USBD_AUDIO_CS_AC_SUBTYPE_UNDEFINED                               = 0x00,
  USBD_AUDIO_CS_AC_SUBTYPE_HEADER                                  = 0x01,
  USBD_AUDIO_CS_AC_SUBTYPE_INPUT_TERMINAL                          = 0x02,
  USBD_AUDIO_CS_AC_SUBTYPE_OUTPUT_TERMINAL                         = 0x03,
  USBD_AUDIO_CS_AC_SUBTYPE_MIXER_UNIT                              = 0x04,
  USBD_AUDIO_CS_AC_SUBTYPE_SELECTOR_UNIT                           = 0x05,
  USBD_AUDIO_CS_AC_SUBTYPE_FEATURE_UNIT                            = 0x06,
  USBD_AUDIO_CS_AC_SUBTYPE_PROCESSING_UNIT                         = 0x07,
  USBD_AUDIO_CS_AC_SUBTYPE_EXTENSION_UNIT                          = 0x08,
}USBD_AUDIO_SpecificACInterfaceDescSubtypeTypeDef;
typedef enum
{
  USBD_AUDIO_TERMINAL_IO_USB_UNDEFINED                             = 0x0100 ,
  USBD_AUDIO_TERMINAL_IO_USB_STREAMING                             = 0x0101 ,
  USBD_AUDIO_TERMINAL_IO_USB_VENDOR_SPECIFIC                       = 0x01FF ,
  USBD_AUDIO_TERMINAL_I_UNDEFINED                                  = 0x0200 ,
  USBD_AUDIO_TERMINAL_I_MICROPHONE                                 = 0x0201 ,
  USBD_AUDIO_TERMINAL_I_DESKTOP_MICROPHONE                         = 0x0202 ,
  USBD_AUDIO_TERMINAL_O_UNDEFINED                                  = 0x0300 ,
  USBD_AUDIO_TERMINAL_O_SPEAKER                                    = 0x0301 ,
  USBD_AUDIO_TERMINAL_O_HEADPHONES                                 = 0x0302
}USBD_AUDIOTerminalTypeDef;
/* Подтипы дескрипторов потокового аудиоинтерфейса */
typedef enum
{
  USBD_AUDIO_CS_SUBTYPE_AS_UNDEFINED                               = 0x00,
  USBD_AUDIO_CS_SUBTYPE_AS_GENERAL                                 = 0x01,
  USBD_AUDIO_CS_SUBTYPE_AS_FORMAT_TYPE                             = 0x02,
  USBD_AUDIO_CS_SUBTYPE_AS_FORMAT_SPECIFIC                         = 0x03
}USBD_AUDIO_SpecificASInterfaceDescSubtypeTypeDef;
/* Колбэки блока управления */
typedef struct
{
   int8_t  (*GetMute)    (uint16_t /*канал*/, uint8_t* /*mute*/, uint32_t /* приватные данные */);
   int8_t  (*SetMute)    (uint16_t /*канал*/, uint8_t /*mute*/, uint32_t /* приватные данные */);
   int8_t  (*SetCurVolume)    (uint16_t /*канал*/, uint16_t /*громкость*/, uint32_t /* приватные данные */);
   int8_t  (*GetCurVolume)    (uint16_t /*канал*/, uint16_t* /*громкость*/, uint32_t /* приватные данные */);
   uint16_t MaxVolume;
   uint16_t MinVolume;
   uint16_t ResVolume;
   int8_t  (*GetStatus)     (uint32_t /*приватные данные */);
}USBD_AUDIO_FeatureControlCallbacksTypeDef;
/* Колбэки модуля управления, вызываются при запросах Get_Cur, Set_Cur и т.д. */
typedef union
{
   USBD_AUDIO_FeatureControlCallbacksTypeDef* feature_control;
}USBD_AUDIO_ControlCallbacksTypeDef;
/** Аудиоблок: поддерживаемые команды и соответствующие колбэки */
/* Следующая структура описывает аудиоблок */
typedef struct
{
    uint8_t id; /* Идентификатор блока */
    USBD_AUDIO_SpecificACInterfaceDescSubtypeTypeDef type; /* Тип блока */
    uint16_t control_req_map; /* Карта разрешенных запросов: GET_CUR, GET_MAX и т.д. */
    uint16_t control_selector_map; /* Список поддерживаемых управляющих элементов, например Mute и Volume */
    USBD_AUDIO_ControlCallbacksTypeDef Callbacks; /* список колбэков */
    uint32_t  private_data; /* используется как последний аргумент каждого колбэка */
}USBD_AUDIO_ControlTypeDef;
#ifdef USBD_SUPPORT_AUDIO_MULTI_FREQUENCIES
/* Колбэки управления конечной точкой */
typedef struct
{
   int8_t  (*GetCurFrequency)    (uint32_t* /*частота*/, uint32_t /* приватные данные */);
   int8_t  (*SetCurFrequency)    (uint32_t /*частота*/, uint8_t* /* restart_req*/ , uint32_t /* приватные данные */);
   uint32_t MaxFrequency;
   uint32_t MinFrequency;
   uint32_t ResFrequency;
}USBD_AUDIO_EndPointControlCallbacksTypeDef;
#endif /* USBD_SUPPORT_AUDIO_MULTI_FREQUENCIES */
/* Структура описывает конечную точку данных и её колбэки */
 typedef struct
 {
   uint8_t ep_num;
   uint16_t control_name_map; /* битовая карта поддерживаемых команд: get_cur|set_cur ... */
   uint16_t control_selector_map; /* битовая карта поддерживаемых управляющих параметров: частота, шаг и т.д. */
   uint8_t* buf;
   uint16_t length;
   int8_t  (*DataReceived)     ( uint16_t/* длина данных */, uint32_t/* приватные данные */); /* вызывается для OUT EP при получении данных */
   uint8_t*  (*GetBuffer)    (uint32_t /* приватные данные */, uint16_t* packet_length); /* вызывается для IN и OUT EP для получения буфера */
   uint16_t  (*GetMaxPacketLength)    (uint32_t /*приватные данные */); /* Вызывается перед открытием EP для получения максимальной длины пакета */
   int8_t  (*GetState)     (uint32_t/*приватные данные */);
#if USBD_SUPPORT_AUDIO_MULTI_FREQUENCIES
   USBD_AUDIO_EndPointControlCallbacksTypeDef control_cbk;
#endif /*USBD_SUPPORT_AUDIO_MULTI_FREQUENCIES*/
   uint32_t  private_data;/* используется как последний аргумент каждого колбэка */
 }  USBD_AUDIO_EP_DataTypeDef;
#if USBD_SUPPORT_AUDIO_OUT_FEEDBACK
 /* Структура описывает конечную точку обратной связи */
 typedef struct
 {
   uint8_t  ep_num; /* номер конечной точки */
   uint8_t feedback_data[AUDIO_FEEDBACK_EP_PACKET_SIZE]; /* буфер для отправки обратной связи */
   uint32_t      (*GetFeedback)     (  uint32_t/* приватные данные */); /* возвращает количество воспроизведенных семплов с момента последнего ResetRate */
   uint32_t private_data;
 }  USBD_AUDIO_EP_SynchTypeDef;
#endif /*USBD_SUPPORT_AUDIO_OUT_FEEDBACK */
/* Структура описывает аудиопотоковый интерфейс */
typedef struct USBD_AUDIO_AS_Interface
{
    uint8_t interface_num; /* номер аудиопотокового интерфейса */
    uint8_t max_alternate; /* наибольший альтернативный номер интерфейса */
    uint8_t alternate;/* текущий альтернативный номер интерфейса */
    USBD_AUDIO_EP_DataTypeDef data_ep; /* основная конечная точка данных интерфейса */
#if USBD_SUPPORT_AUDIO_OUT_FEEDBACK
    uint8_t synch_enabled;
    USBD_AUDIO_EP_SynchTypeDef synch_ep; /* описание синхронизационной конечной точки */
#endif /*USBD_SUPPORT_AUDIO_OUT_FEEDBACK */
    void  (*SofReceived)     ( uint32_t/*приватные данные */);
    int8_t  (*SetAS_Alternate)     ( uint8_t/*альтернатива*/, uint32_t/*приватные данные */);
    int8_t  (*GetState)     (uint32_t/*приватные данные */);
    uint32_t  private_data; /* используется как последний аргумент каждого колбэка */
}USBD_AUDIO_AS_InterfaceTypeDef;
/* Структура описывает всю аудиофункцию, которую должен инициализировать пользователь */
typedef struct
{
  uint8_t control_count; /* количество управляющих блоков */
  uint8_t as_interfaces_count;/* количество аудиопотоковых интерфейсов */
  USBD_AUDIO_ControlTypeDef controls[USBD_AUDIO_CONFIG_CONTROL_UNIT_COUNT]; /* список управляющих блоков */
  USBD_AUDIO_AS_InterfaceTypeDef as_interfaces[USBD_AUDIO_AS_INTERFACE_COUNT];/* список аудиопотоковых интерфейсов */
}USBD_AUDIO_FunctionDescriptionfTypeDef;
/* Структура описывает аудиоинтерфейс */
typedef struct
{
    int8_t  (*Init)         (USBD_AUDIO_FunctionDescriptionfTypeDef* /* описание AS */ , uint32_t /*приватные данные */);
    int8_t  (*DeInit)       (USBD_AUDIO_FunctionDescriptionfTypeDef* /* описание AS */, uint32_t /*приватные данные */);
    int8_t  (*GetConfigDesc) (uint8_t ** /*pdata*/, uint16_t * /*psize*/, uint32_t /*private_data*/);
    int8_t  (*GetState)     (uint32_t privatedata);
    uint32_t private_data;
}USBD_AUDIO_InterfaceCallbacksfTypeDef;
/**
  * @}
  */
/** @defgroup USBD_CORE_Exported_Macros
  * @{
  */
 /* макросы для использования в дескрипторе конфигурации */
#define AUDIO_SAMPLE_FREQ(frq)      (uint8_t)(frq), (uint8_t)(((frq) >> 8)), (uint8_t)(((frq) >> 16))
#define AUDIO_FREQ_TO_DATA(frq , bytes)      do{\
                                                  (bytes)[0]= (uint8_t)(frq);\
                                                  (bytes)[1]= (uint8_t)(((frq) >> 8));\
                                                  (bytes)[2]= (uint8_t)(((frq) >> 16));\
                                               }while(0);
#define AUDIO_FREQ_FROM_DATA(bytes)      (((uint32_t)((bytes)[2]))<<16)| (((uint32_t)((bytes)[1]))<<8)| (((uint32_t)((bytes)[0])))
/**
  * @}
  */
/** @defgroup USBD_CORE_Exported_Variables
  * @{
  */
extern USBD_ClassTypeDef  USBD_AUDIO;
#define USBD_AUDIO_CLASS    &USBD_AUDIO
/**
  * @}
  */
/** @defgroup USB_CORE_Exported_Functions
  * @{
  */
uint8_t  USBD_AUDIO_RegisterInterface  (USBD_HandleTypeDef   *pdev,
                                        USBD_AUDIO_InterfaceCallbacksfTypeDef *aifc);
/**
  * @}
  */
#ifdef __cplusplus
}
#endif
#endif  /* __USB_AUDIO_H */
/**
  * @}
  */
/**
  * @}
  */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
