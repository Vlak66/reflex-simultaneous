/**
  ******************************************************************************
  * @file    audio_usb_playback_session.c
  * @author  MCD Application Team
  * @brief   Реализация сессии воспроизведения аудио через USB.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019  STMicroelectronics.
  * Все права защищены.</center></h2>
  *
  * Этот программный модуль лицензирован компанией ST по лицензии Ultimate Liberty
  * SLA0044, "License"; Вы можете использовать этот файл только в соответствии с
  * условиями лицензии. Полный текст лицензии можно найти по адресу:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/*
*******************************************************************************
*
* Изменён ЗАО «ЧИП и ДИП» для поддержки нескольких режимов Alternate Settings,
* чтобы динамически переключаться между разрешением 16 бит и 24 бита, 2019 год.
*
* Modified by ChipDip to support several Alternate Settings
* to change 16 bit and 24 bit audio resolution on the fly, 2019.
*
*******************************************************************************
*/


/* Includes ------------------------------------------------------------------*/
#include "usbd_audio.h"
#include "usb_audio.h"
#include "audio_speaker_node.h"
#include "audio_sessions_usb.h"
#include "usb_audio_descriptors.h"

#if USE_USB_AUDIO_PLAYBACK
/* Private defines -----------------------------------------------------------*/
#define AUDIO_USB_PLAYBACK_ALTERNATE 0x01

/* Private typedef -----------------------------------------------------------*/
/* External variables --------------------------------------------------------*/
#if USE_AUDIO_PLAYBACK_RECORDING_SHARED_CLOCK_SRC
#endif /* USE_AUDIO_PLAYBACK_RECORDING_SHARED_CLOCK_SRC */
/* Private macros ------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Play usb session callbacks */
static int8_t  USB_AudioPlaybackSessionStart(AUDIO_USBSession_t* session);
static int8_t  USB_AudioPlaybackSessionStop(AUDIO_USBSession_t* session);
static int8_t  USB_AudioPlaybackSetAudioStreamingInterfaceAlternateSetting( uint8_t alternate,  uint32_t session_handle);
static int8_t  USB_AudioPlaybackGetState(uint32_t session_handle);
static int8_t  USB_AudioPlaybackSessionCallback(AUDIO_SessionEvent_t  event,
                                               AUDIO_Node_t* node_handle,
                                               struct    AUDIO_Session* session_handle);
static uint32_t   USB_AudioPlaybackGetFeedback( uint32_t session_handle );
static void  AUDIO_USB_Session_Sof_Received(uint32_t session_handle );


static void UpdateInputNodePackLength(void);

/* Private variables ---------------------------------------------------------*/

/* Список используемых аудио-нод */
static AUDIO_USBInputOutputNode_t PlaybackUSBInputNode;
static AUDIO_Description_t PlaybackAudioDescription;
static AUDIO_USB_CF_NodeTypeDef PlaybackFeatureUnitNode;
static AUDIO_SpeakerNode_t PlaybackSpeakerOutputNode;
/* Синхронизация воспроизведения: оценка частоты */
static uint8_t PlaybackSynchroFirstSofReceived = 0;
static uint32_t PlaybackSynchroEstimatedCodecFrequency = 0;

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  AUDIO_PlaybackSessionInit
  *         Инициализирует сессию воспроизведения.
  * @param  as_desc(OUT): обратные вызовы аудио-потока для связи с модулем класса USB Audio
  * @param  controls_desc(OUT): список управления, сессия указывает необходимые элементы управления
  * @param  control_count(IN/OUT): количество элементов управления
  * @param  session_handle(IN): дескриптор сессии, который должен быть выделен
  * @retval  : 0 при отсутствии ошибок
  */
 int8_t  AUDIO_PlaybackSessionInit(USBD_AUDIO_AS_InterfaceTypeDef* as_desc,
                                    USBD_AUDIO_ControlTypeDef* controls_desc,
                                    uint8_t* control_count, uint32_t session_handle)
{
  AUDIO_USBSession_t *play_session;
  AUDIO_USBFeatureUnitDefaults_t controller_defaults;

   play_session = (AUDIO_USBSession_t*)session_handle;
   memset( play_session, 0, sizeof(AUDIO_USBSession_t));

   play_session->interface_num = CONFIG_AUDIO_STREAMING_INTERFACE;
   play_session->alternate = 0;
   //play_session->SessionDeInit = USB_AudioPlaybackSessionDeInit;
   play_session->session.SessionCallback = USB_AudioPlaybackSessionCallback;
   play_session->buffer.size = USB_AUDIO_CONFIG_PLAY_BUFFER_SIZE;
   play_session->buffer.data = malloc( USB_AUDIO_CONFIG_PLAY_BUFFER_SIZE);
   if(! play_session->buffer.data)
   {
    Error_Handler();
   }
    /*настройка параметров аудио*/
  PlayDescriptionInit(&PlaybackAudioDescription);

  *control_count = 0;

   /* создаём входной узел USB */
  USB_AudioStreamingInputInit(&as_desc->data_ep,  &PlaybackAudioDescription,  &play_session->session,  (uint32_t)&PlaybackUSBInputNode);
   play_session->session.node_list = (AUDIO_Node_t*)&PlaybackUSBInputNode;
  /* инициализируем узел управления громкостью */
  controller_defaults.audio_description = &PlaybackAudioDescription;
    /* установите здесь значения по умолчанию для громкости динамика */
  controller_defaults.max_volume = VOLUME_SPEAKER_MAX_DB_256;
  controller_defaults.min_volume = VOLUME_SPEAKER_MIN_DB_256;
  controller_defaults.res_volume = VOLUME_SPEAKER_RES_DB_256;
  USB_AudioStreamingFeatureUnitInit( controls_desc,  &controller_defaults,  CONFIG_UNIT_FEATURE_ID, (uint32_t)&PlaybackFeatureUnitNode);
  (*control_count)++;
  PlaybackUSBInputNode.node.next = (AUDIO_Node_t*)&PlaybackFeatureUnitNode;
  AUDIO_SpeakerInit(&PlaybackAudioDescription, &play_session->session, (uint32_t)&PlaybackSpeakerOutputNode);
  PlaybackFeatureUnitNode.node.next = (AUDIO_Node_t*)&PlaybackSpeakerOutputNode;

/* инициализация параметров синхронизации */

  as_desc->synch_enabled = 1;
  as_desc->synch_ep.ep_num = CONFIG_AUDIO_EP_SYNC;
  as_desc->synch_ep.GetFeedback = USB_AudioPlaybackGetFeedback;
  as_desc->synch_ep.private_data = (uint32_t) play_session;
  as_desc->SofReceived = AUDIO_USB_Session_Sof_Received;
  /* устанавливаем обратные вызовы класса USB AUDIO */
  as_desc->interface_num =  play_session->interface_num;
  as_desc->alternate = 0;
  as_desc->max_alternate = AUDIO_USB_PLAYBACK_ALTERNATE;
  as_desc->private_data = session_handle;
  as_desc->SetAS_Alternate = USB_AudioPlaybackSetAudioStreamingInterfaceAlternateSetting;
  as_desc->GetState = USB_AudioPlaybackGetState;

  /* инициализируем рабочий буфер */
  uint16_t buffer_margin = (PlaybackUSBInputNode.max_packet_length > PlaybackUSBInputNode.packet_length)?PlaybackUSBInputNode.max_packet_length:0;
  USB_AudioStreamingInitializeDataBuffer(&play_session->buffer, USB_AUDIO_CONFIG_PLAY_BUFFER_SIZE,
                                  AUDIO_MS_PACKET_SIZE_FROM_AUD_DESC(&PlaybackAudioDescription) , buffer_margin);
  play_session->session.state = AUDIO_SESSION_INITIALIZED;

  return 0;
}

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  USB_AudioPlaybackSessionStart
  *         Запускает сессию воспроизведения
  * @param  play_session(IN): дескриптор сессии
  * @retval  : 0 при отсутствии ошибок
  */
static int8_t  USB_AudioPlaybackSessionStart(AUDIO_USBSession_t*  play_session)
{
  if(( play_session->session.state == AUDIO_SESSION_INITIALIZED)
     ||(play_session->session.state == AUDIO_SESSION_STOPPED))
  {
        AUDIO_USBFeatureUnitCommands_t commands;
    /* запускаем входной узел */
    PlaybackUSBInputNode.IOStart(& play_session->buffer,   play_session->buffer.size/2,  (uint32_t)&PlaybackUSBInputNode);
    commands.private_data = (uint32_t)&PlaybackSpeakerOutputNode;
    commands.SetMute = PlaybackSpeakerOutputNode.SpeakerMute;
    commands.SetCurrentVolume = PlaybackSpeakerOutputNode.SpeakerSetVolume;
    PlaybackFeatureUnitNode.CFStart(&commands,(uint32_t)&PlaybackFeatureUnitNode);
    play_session->session.state = AUDIO_SESSION_STARTED;
  }

  return 0;
}

/**
  * @brief  USB_AudioPlaybackSessionStop
  *         Останавливает сессию воспроизведения
  * @param  play_session:
  * @retval 0 при отсутствии ошибок
  */
static int8_t  USB_AudioPlaybackSessionStop(AUDIO_USBSession_t*  play_session)
{

  if( play_session->session.state == AUDIO_SESSION_STARTED)
  {
    PlaybackUSBInputNode.IOStop((uint32_t)&PlaybackUSBInputNode);
    PlaybackFeatureUnitNode.CFStop((uint32_t)&PlaybackFeatureUnitNode);
    PlaybackSpeakerOutputNode.SpeakerStop((uint32_t)&PlaybackSpeakerOutputNode);
    play_session->session.state = AUDIO_SESSION_STOPPED;
  }

  return 0;
}

/**
  * @brief  USB_AudioPlaybackSessionCallback
  *         Обратный вызов сессии воспроизведения. Получает события от аудио-нод.
  * @param  event(IN): событие, полученное от ноды
  * @param  node(IN): указатель на источник события
  * @param  session_handle: дескриптор сессии воспроизведения
  * @retval : 0 при отсутствии ошибок
  */
static int8_t  USB_AudioPlaybackSessionCallback(AUDIO_SessionEvent_t  event,
                                               AUDIO_Node_t* node,
                                               struct    AUDIO_Session* session_handle)
{
  AUDIO_USBSession_t * play_session = (AUDIO_USBSession_t *)session_handle;

  switch(event)
  {
  case AUDIO_THRESHOLD_REACHED:  // Достигнут порог заполнения буфера — можно начинать воспроизведение

    if(node->type == AUDIO_INPUT)
    {
      PlaybackSpeakerOutputNode.SpeakerStart(&play_session->buffer, (uint32_t)&PlaybackSpeakerOutputNode);
      PlaybackSynchroFirstSofReceived = 0;   // Перезапуск синхронизации
    }
    break;

  case AUDIO_PACKET_RECEIVED:
    // Пакет данных успешно принят — обработка не требуется
    break;

  case AUDIO_FREQUENCY_CHANGED: // Изменилась частота аудио
    {
      // Перенастраиваем динамик под новую частоту
      PlaybackSpeakerOutputNode.SpeakerChangeFrequency((uint32_t)&PlaybackSpeakerOutputNode);

      // Пересчитываем размер буфера
      uint16_t buffer_margin = (PlaybackUSBInputNode.max_packet_length > PlaybackUSBInputNode.packet_length) ? PlaybackUSBInputNode.max_packet_length : 0;
      USB_AudioStreamingInitializeDataBuffer(&play_session->buffer, USB_AUDIO_CONFIG_PLAY_BUFFER_SIZE,
                                  AUDIO_MS_PACKET_SIZE_FROM_AUD_DESC(&PlaybackAudioDescription), buffer_margin);

      PlaybackSynchroFirstSofReceived = 0;
      PlaybackSynchroEstimatedCodecFrequency = 0;
    }
    break;

  case AUDIO_OVERRUN: // Буфер переполнен
  case AUDIO_UNDERRUN: // Буфер опустошён
    {
     // Останавливаем вывод, перезапускаем ввод
     PlaybackSpeakerOutputNode.SpeakerStop((uint32_t)&PlaybackSpeakerOutputNode);
     PlaybackSynchroFirstSofReceived = 0;
     PlaybackSynchroEstimatedCodecFrequency = 0;

     if( play_session->session.state == AUDIO_SESSION_STARTED)
     {
       PlaybackUSBInputNode.IORestart((uint32_t)&PlaybackUSBInputNode);
     }
      break;
    }

  default :
   break;
  }

  return 0;
}


/**
  * @brief  USB_AudioPlaybackSetAudioStreamingInterfaceAlternateSetting
  *         Установка альтернативного режима интерфейса потокового аудио.
  *         Вызывается модулем класса USB Audio.
  * @param  alternate(IN): номер альтернативного режима
  * @param  session_handle(IN): дескриптор сессии
  * @retval : 0 при отсутствии ошибок
  */
static int8_t  USB_AudioPlaybackSetAudioStreamingInterfaceAlternateSetting( uint8_t alternate , uint32_t session_handle)
{
  AUDIO_USBSession_t * play_session;

   play_session = (AUDIO_USBSession_t*)session_handle;

  if(alternate == 0)
  {
    if( play_session->alternate != 0)
    {
       USB_AudioPlaybackSessionStop(play_session); // Останавливаем воспроизведение
       play_session->alternate = 0; // Переходим в режим по умолчанию
    }
  }
  else
  {
    if( play_session->alternate == 0)
    {
      switch(alternate)
      {
        case ALTERNATE_SETTING_16_BIT:
        default:
          play_session->session.node_list->audio_description->resolution = CONFIG_RES_BYTE_16;
        break;
      }

      UpdateInputNodePackLength(); // Обновляем длину пакета
      AUDIO_SpeakerChangeResolution((uint32_t)&PlaybackSpeakerOutputNode); // Меняем разрешение динамика

      USB_AudioPlaybackSessionStart(play_session); // Запускаем сессию с новыми параметрами
      play_session->alternate = alternate;
    }
  }

  return 0;
}


/**
  * @brief  USB_AudioPlaybackGetState
  *         Возвращает текущее состояние интерфейса потокового аудио.
  * @param  session_handle: дескриптор сессии
  * @retval : всегда 0 (интерфейс активен)
  */
static int8_t  USB_AudioPlaybackGetState(uint32_t session_handle)
{
  return 0;
}

/**
  * @brief  USB_AudioPlaybackGetFeedback
  *         Возвращает количество проигранных семплов (для обратной связи по синхронизации).
  * @param  session_handle: дескриптор сессии
  * @retval : частота семплирования с поправкой на переполнение/опустошение буфера
  */
static uint32_t USB_AudioPlaybackGetFeedback( uint32_t session_handle )
{
 if((PlaybackSpeakerOutputNode.node.state == AUDIO_NODE_STARTED))
 {
    if(PlaybackSynchroEstimatedCodecFrequency)
    {
      return PlaybackSynchroEstimatedCodecFrequency ;
    }
    else
    {
     AUDIO_CircularBuffer_t *buffer = &((AUDIO_USBSession_t*)session_handle)->buffer;
     int32_t wr_distance;

     wr_distance = AUDIO_BUFFER_FREE_SIZE(buffer);

     // Корректируем частоту в зависимости от состояния буфера
     if(wr_distance <= (buffer->size >> 2))
     {
       return PlaybackAudioDescription.frequency - 1000;
     }
     if( wr_distance >= (buffer->size - (buffer->size >> 2)))
     {
       return PlaybackAudioDescription.frequency + 1000;
     }
    }
 }

 return PlaybackAudioDescription.frequency;
}


/**
  * @brief  AUDIO_USB_Session_Sof_Received
  *         Обработка события SOF (Start of Frame) для синхронизации аудио.
  * @param  session_handle: дескриптор сессии
  * @retval : нет
  */
static void  AUDIO_USB_Session_Sof_Received(uint32_t session_handle )
 {
   static uint16_t sof_counter = 0;
#ifdef USE_USB_HS
   static uint8_t micro_sof_counter = 0;
#endif /* USE_USB_HS */
   static uint32_t total_received_sub_samples = 0;
    AUDIO_USBSession_t *session;
    uint16_t read_samples_per_channel ;

  session = (AUDIO_USBSession_t*)session_handle;

  if( session->session.state == AUDIO_SESSION_STARTED)
  {
   if(PlaybackSynchroFirstSofReceived)
   {
#ifdef USE_USB_HS
     if(micro_sof_counter !=7)
     {
       micro_sof_counter++;
     }
     else
     {
#endif /* USE_USB_HS */
        read_samples_per_channel = PlaybackSpeakerOutputNode.SpeakerGetReadCount((uint32_t)&PlaybackSpeakerOutputNode);
        total_received_sub_samples += read_samples_per_channel;

        if(++sof_counter == 1000)
        {
          PlaybackSynchroEstimatedCodecFrequency = ((total_received_sub_samples)>>1);
          sof_counter = 0;
          total_received_sub_samples = 0;
        }

#ifdef USE_USB_HS
        micro_sof_counter = 0;
     }
#endif /* USE_USB_HS */
   }
   else
   {
       PlaybackSpeakerOutputNode.SpeakerStartReadCount((uint32_t)&PlaybackSpeakerOutputNode);
       sof_counter = 0;
#ifdef USE_USB_HS
       micro_sof_counter = 0;
#endif /* USE_USB_HS */
       total_received_sub_samples = 0;
       PlaybackSynchroFirstSofReceived = 1;
    }
  }
  else
  {
    PlaybackSynchroFirstSofReceived = 0;
  }
 }

#endif /*USE_USB_AUDIO_PLAYBACK*/


/**
  * @brief  UpdateInputNodePackLength
  *         Обновляет длину пакета входного узла в соответствии с текущими аудио-настройками.
  */
 void UpdateInputNodePackLength(void)
 {
   PlaybackUSBInputNode.packet_length = AUDIO_USB_PACKET_SIZE_FROM_AUD_DESC(&PlaybackAudioDescription);

   PlaybackUSBInputNode.max_packet_length = AUDIO_MAX_PACKET_WITH_FEEDBACK_LENGTH(&PlaybackAudioDescription);
 }

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/