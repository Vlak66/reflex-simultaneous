/**
  ******************************************************************************
  * @file    audio_speaker_node.c
  * @author  MCD Application Team
  * @brief   реализация узла динамика.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Авторские права (c) 2019  STMicroelectronics.
  * Все права защищены.</center></h2>
  *
  * Этот программный компонент лицензирован ST по лицензии Ultimate Liberty
  * SLA0044, "Лицензия"; Вы не можете использовать этот файл, кроме как в соответствии с
  * Лицензией. Вы можете получить копию Лицензии по адресу:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */

/*
*******************************************************************************
*
* Modified by HAND AUDIO to support апсемплинга
*
*******************************************************************************
*/


/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include "usbd_audio.h"
#include "audio_speaker_node.h"
#include "audio_configuration.h"
#include "usb_audio.h"
#include "board.h"
#include "dsp_upsampling.h" // <-- НОВОЕ: Включение заголовка для DSP апсемплинга

/* Private defines -----------------------------------------------------------*/
#define SPEAKER_CMD_STOP                1
#define SPEAKER_CMD_EXIT                2
#define SPEAKER_CMD_CHANGE_FREQUENCE    4
#define SPEAKER_CMD_CHANGE_RESOLUTION   (1 << 3)
#define VOLUME_DB_256_TO_PERCENT(volume_db_256) \
    ((uint8_t)((((int)(volume_db_256) - VOLUME_SPEAKER_MIN_DB_256) * 100) / \
    (VOLUME_SPEAKER_MAX_DB_256 - VOLUME_SPEAKER_MIN_DB_256)))

/* alt buffer max size */
/*#define SPEAKER_ALT_BUFFER_SIZE ((USB_AUDIO_CONFIG_PLAY_FREQ_MAX+999)/1000)*2*4*2 старое*/
#define IN_CHUNK_SAMPLES_STEREO         256 // Количество стерео-сэмплов, обрабатываемых за раз
#define SPEAKER_ALT_BUFFER_SIZE         (IN_CHUNK_SAMPLES_STEREO * 4 * 2 * sizeof(int16_t)) // Макс. размер после x4 апсемплинга

#ifdef DEBUG_SPEAKER_NODE
#define SPEAKER_DEBUG_BUFFER_SIZE 1000
#endif /*DEBUG_SPEAKER_NODE*/

/* Private function prototypes -----------------------------------------------*/

static int8_t  AUDIO_SpeakerStart(AUDIO_CircularBuffer_t* buffer, uint32_t node_handle);
static int8_t  AUDIO_SpeakerStop( uint32_t node_handle);
static int8_t  AUDIO_SpeakerMute( uint16_t channel_number,  uint8_t mute , uint32_t node_handle);
static int8_t  AUDIO_SpeakerSetVolume( uint16_t channel_number,  int volume ,  uint32_t node_handle);
static void    AUDIO_SpeakerInitInjectionsParams( AUDIO_SpeakerNode_t* speaker);
static int8_t  AUDIO_SpeakerStartReadCount( uint32_t node_handle);
static uint16_t AUDIO_SpeakerGetLastReadCount( uint32_t node_handle);
static void ProcessAudio(void);

/* Приватные типы -----------------------------------------------------------*/
#ifdef DEBUG_SPEAKER_NODE
typedef struct
{
  uint32_t time;
  uint16_t injection_size;
  uint16_t read;
  uint16_t dump;
  uint8_t* data;
} AUDIO_SpeakerNodeBufferStats_t;
#endif /* DEBUG_SPEAKER_NODE*/

/* Приватные макросы ------------------------------------------------------------*/
/* Внешние переменные --------------------------------------------------------*/
//extern SAI_HandleTypeDef         haudio_out_sai_1;
#ifdef DEBUG_SPEAKER_NODE
extern __IO uint32_t uwTick;
#endif /* DEBUG_SPEAKER_NODE*/

/* Приватные переменные -----------------------------------------------------------*/
static AUDIO_SpeakerNode_t *AUDIO_SpeakerHandler = 0;
#ifdef DEBUG_SPEAKER_NODE
static AUDIO_SpeakerNodeBufferStats_t AUDIO_SpeakerDebugStats[SPEAKER_DEBUG_BUFFER_SIZE];
static  int AUDIO_SpeakerDebugStats_count =0;
#endif /* DEBUG_SPEAKER_NODE*/

// <-- НОВЫЕ: Буферы для DSP обработки
// dspIn: буфер для данных, взятых из кольцевого USB-буфера перед апсемплингом.
// Размер: IN_CHUNK_SAMPLES_STEREO * 2 (для стерео) * sizeof(int16_t)
static int16_t dsp_input_buffer[IN_CHUNK_SAMPLES_STEREO * 2] __attribute__((aligned(4)));
// dspOut: буфер для данных после апсемплинга.
// Размер: IN_CHUNK_SAMPLES_STEREO * 4 (для макс. x4) * 2 (для стерео) * sizeof(int16_t)
static int16_t dsp_output_buffer[IN_CHUNK_SAMPLES_STEREO * 4 * 2] __attribute__((aligned(4)));


/* Экспортируемые функции ---------------------------------------------------------*/

/**
  * @brief  AUDIO_SpeakerInit
  *         Инициализирует узел аудиодинамика, устанавливает обратные вызовы и запускает кодек. Поскольку данные не готовы,
  *         SAI заполняется из альтернативного буфера (заполненного нулями)
  * @param  audio_description(ВХ): информация об аудио
  * @param  session_handle(ВХ):   дескриптор сессии
  * @param  node_handle(ВХ):      дескриптор узла динамика должен быть выделен
  * @retval 0 в случае отсутствия ошибок
  */
 int8_t  AUDIO_SpeakerInit(AUDIO_Description_t* audio_description,  AUDIO_Session_t* session_handle,
                           uint32_t node_handle)
{
  AUDIO_SpeakerNode_t* speaker;

  speaker = (AUDIO_SpeakerNode_t*)node_handle;
  memset(speaker, 0, sizeof(AUDIO_SpeakerNode_t));
  speaker->node.type = AUDIO_OUTPUT;
  speaker->node.state = AUDIO_NODE_INITIALIZED;
  speaker->node.session_handle = session_handle;
  speaker->node.audio_description = audio_description;
  speaker->specific.alt_buffer = malloc(SPEAKER_ALT_BUFFER_SIZE);
  if(speaker->specific.alt_buffer == 0)
  {
    Error_Handler();
  }
  AUDIO_SpeakerInitInjectionsParams( speaker);

  /* установить обратные вызовы */
  //speaker->SpeakerDeInit = AUDIO_SpeakerDeInit;
  speaker->SpeakerStart = AUDIO_SpeakerStart;
  speaker->SpeakerStop = AUDIO_SpeakerStop;
  speaker->SpeakerChangeFrequency = AUDIO_SpeakerChangeFrequency;
  speaker->SpeakerMute = AUDIO_SpeakerMute;
  speaker->SpeakerSetVolume = AUDIO_SpeakerSetVolume;
  speaker->SpeakerStartReadCount = AUDIO_SpeakerStartReadCount;
  speaker->SpeakerGetReadCount = AUDIO_SpeakerGetLastReadCount;

  SetAudioConfigDependedFuncs(speaker);

  // <-- ИЗМЕНЕНО: Передаем в AudioOutInit целевую частоту с учетом апсемплинга
  uint32_t initial_frequency = speaker->node.audio_description->frequency;
  if (IsUpsamplingEnabled())
  {
      // Определяем коэффициент апсемплинга для начальной частоты
      // (полагаем, что начальная частота будет 44.1/48/88.2/96кГц)
      if (initial_frequency < USB_AUDIO_CONFIG_FREQ_88_2_K) // 44.1/48 кГц
      {
          initial_frequency *= UP_FACTOR_X4;
      }
      else // 88.2/96 кГц
      {
          initial_frequency *= UP_FACTOR_X2;
      }
  }

  AudioOutInit(initial_frequency, audio_description->resolution << 3);
  ExtPowerDisable();

/*
  BSP_AUDIO_OUT_Init_Ext(OUTPUT_DEVICE_AUTO,
                     VOLUME_DB_256_TO_PERCENT(VOLUME_SPEAKER_DEFAULT_DB_256),
                     speaker->node.audio_description->frequency, audio_description->resolution<<3 );
*/


 // <-- ИЗМЕНЕНО: На начальной стадии заполняем буфер для воспроизведения нулями.
  // Если апсемплинг включен, то размер буфера должен соответствовать выходу апсемплинга.
  uint16_t initial_play_data_size = AUDIO_SpeakerHandler->specific.injection_size;
  if (IsUpsamplingEnabled()) {
      // Это просто заглушка нулями, поэтому здесь не принципиально применять DSP,
      // но для корректного размера выходного буфера нужно учесть upsampling
      // Предполагаем, что specific.injection_size уже отражает апсемплинг
      // после вызова AUDIO_SpeakerInitInjectionsParams
  }

  speaker->SpeakerPlay((uint16_t *)speaker->specific.data,
                       initial_play_data_size, // Используем скорректированный размер
                       speaker->node.audio_description->resolution);


  //BSP_AUDIO_OUT_Play((uint16_t *)speaker->specific.data ,speaker->specific.data_size );


  //начиная с версии 1.6
  //если USB-устройство инициализировано - программный сброс
  //нужно для корректного функционирования при перезагрузке ОС
  //и при включении ПК
  if (AUDIO_SpeakerHandler != 0)
  {
    //Soft reset
    __DSB();

    SCB->AIRCR  = ((0x5FA << SCB_AIRCR_VECTKEY_Pos)      |
                   (SCB->AIRCR & SCB_AIRCR_PRIGROUP_Msk) |
                   SCB_AIRCR_SYSRESETREQ_Msk);
    __DSB();
    while(1);
  }

  AUDIO_SpeakerHandler = speaker;
  return 0;
}


/**
  * @brief  BSP_AUDIO_OUT_Error_CallBack
  *         Manages the DMA error event.
  * @param  None
  * @retval None
  */
void BSP_AUDIO_OUT_Error_CallBack(void)
{
  Error_Handler();
}

/**
  * @brief  BSP_AUDIO_OUT_TransferComplete_CallBack
  *         Управляет событием полного завершения передачи DMA.
  * @param  Нет
  * @retval Нет
  */
void BSP_AUDIO_OUT_TransferComplete_CallBack(void)
{
  uint16_t wr_distance; // Переменная для расстояния записи

  if ((AUDIO_SpeakerHandler) &&
      (AUDIO_SpeakerHandler->node.state != AUDIO_NODE_OFF))
  {
    // выполняется, если получена команда остановки
    if (AUDIO_SpeakerHandler->specific.cmd & SPEAKER_CMD_EXIT)
    {
      AUDIO_SpeakerHandler->specific.cmd = 0;
      return;
    }

    // <-- ИЗМЕНЕНО: Логика смены частоты с учетом апсемплинга
    if (AUDIO_SpeakerHandler->specific.cmd & SPEAKER_CMD_CHANGE_FREQUENCE)
    {
      AUDIO_SpeakerHandler->node.state = AUDIO_NODE_STOPPED;
      AUDIO_SpeakerInitInjectionsParams(AUDIO_SpeakerHandler); // Обновить параметры инъекций

      uint32_t host_frequency = AUDIO_SpeakerHandler->node.audio_description->frequency;
      uint32_t target_sai_frequency = host_frequency; // По умолчанию равна частоте хоста

      if (IsUpsamplingEnabled()) // Если апсемплинг включен
      {
          if (host_frequency == USB_AUDIO_CONFIG_FREQ_44_1_K)
          {
              target_sai_frequency = USB_AUDIO_CONFIG_FREQ_176_4_K; // 44100 -> 176400 (x4)
              DSP_UpsampleInit(UP_FACTOR_X4, GetUpsampleAlgo()); // Инициализируем DSP
          }
          else if (host_frequency == USB_AUDIO_CONFIG_FREQ_48_K)
          {
              target_sai_frequency = USB_AUDIO_CONFIG_FREQ_192_K; // 48000 -> 192000 (x4)
              DSP_UpsampleInit(UP_FACTOR_X4, GetUpsampleAlgo()); // Инициализируем DSP
          }
          else if (host_frequency == USB_AUDIO_CONFIG_FREQ_88_2_K)
          {
              target_sai_frequency = USB_AUDIO_CONFIG_FREQ_176_4_K; // 88200 -> 176400 (x2)
              DSP_UpsampleInit(UP_FACTOR_X2, GetUpsampleAlgo()); // Инициализируем DSP
          }
          else if (host_frequency == USB_AUDIO_CONFIG_FREQ_96_K)
          {
              target_sai_frequency = USB_AUDIO_CONFIG_FREQ_192_K; // 96000 -> 192000 (x2)
              DSP_UpsampleInit(UP_FACTOR_X2, GetUpsampleAlgo()); // Инициализируем DSP
          }
          // Для частот 176.4k и 192k апсемплинг не нужен, они уже на максимуме,
          // поэтому target_sai_frequency останется равным host_frequency.
          // DSP_UpsampleInit(UP_FACTOR_X1, 0); // Инициализация на UP_FACTOR_X1 (без апсемплинга)
      } else {
          // Если апсемплинг выключен, инициализируем DSP как "без апсемплинга" (UP_FACTOR_X1)
          // DSP_UpsampleInit(UP_FACTOR_X1, 0); // Эта функция может не требовать вызова, если нет апсемплинга
                                             // или если она всегда вызывается с фактическим фактором >1
      }
      AudioChangeFrequency(target_sai_frequency); // Вызываем с целевой частотой для SAI
      AUDIO_SpeakerHandler->specific.cmd &= ~SPEAKER_CMD_CHANGE_FREQUENCE;
    }

    if (AUDIO_SpeakerHandler->specific.cmd & SPEAKER_CMD_STOP)
    {
      // ... (Остальной код для SPEAKER_CMD_STOP остается без изменений) ...
      AUDIO_SpeakerHandler->specific.data =
        AUDIO_SpeakerHandler->specific.alt_buffer;
      AUDIO_SpeakerHandler->specific.data_size =
        AUDIO_SpeakerHandler->specific.injection_size;
      AUDIO_SpeakerHandler->specific.offset = 0;
      memset(AUDIO_SpeakerHandler->specific.data, 0,
        AUDIO_SpeakerHandler->specific.data_size);
      AUDIO_SpeakerHandler->node.state = AUDIO_NODE_STOPPED;
      AUDIO_SpeakerHandler->specific.cmd ^= SPEAKER_CMD_STOP;
    }

    if (AUDIO_SpeakerHandler->specific.cmd &
        SPEAKER_CMD_CHANGE_RESOLUTION)
    {
      AUDIO_SpeakerHandler->node.state = AUDIO_NODE_STOPPED;
      AUDIO_SpeakerInitInjectionsParams(AUDIO_SpeakerHandler);
      AudioChangeResolution(
        AUDIO_SpeakerHandler->node.audio_description->resolution
        << 3);
      AUDIO_SpeakerHandler->specific.cmd &=
        ~SPEAKER_CMD_CHANGE_RESOLUTION;
    }

    // <-- ИЗМЕНЕНО: Вызываем Play_SAIMaster с данными, которые будут подготовлены ProcessAudio
    // speaker->SpeakerPlay((uint16_t *)AUDIO_SpeakerHandler->specific.data, ...);
    // Теперь data и data_size устанавливаются в ProcessAudio.
    // Вызов SpeakerPlay() перемещен в конец ProcessAudio или после него, чтобы использовались
    // уже подготовленные DSP-данные.

    /* если динамик был запущен, подготовьте следующие данные */
    if (AUDIO_SpeakerHandler->node.state == AUDIO_NODE_STARTED)
    {
#ifdef DEBUG_SPEAKER_NODE
      AUDIO_SpeakerDebugStats[AUDIO_SpeakerDebugStats_count].time =
        uwTick;
#endif /* DEBUG_SPEAKER_NODE */

      /* уведомить сессию о том, что пакет воспроизведен */
      AUDIO_SpeakerHandler->node.session_handle->SessionCallback(
        AUDIO_PACKET_PLAYED,
        (AUDIO_Node_t *)AUDIO_SpeakerHandler,
        AUDIO_SpeakerHandler->node.session_handle);

      // <-- НОВОЕ: Вызываем нашу функцию ProcessAudio()
      ProcessAudio();

      wr_distance = AUDIO_BUFFER_FILLED_SIZE(AUDIO_SpeakerHandler->buf);
      if (wr_distance < AUDIO_SpeakerHandler->packet_length) // Здесь проверяем наличие "оригинального" размера данных
      {
        /** информировать сессию о том, что произошел недозапись */
        AUDIO_SpeakerHandler->node.session_handle->
          SessionCallback(AUDIO_UNDERRUN,
          (AUDIO_Node_t *)AUDIO_SpeakerHandler,
          AUDIO_SpeakerHandler->node.session_handle);

        // Если недозапись, то для предотвращения артефактов
        // можно заполнить dsp_input_buffer нулями
        memset(dsp_input_buffer, 0, sizeof(dsp_input_buffer));
        AUDIO_SpeakerHandler->specific.data = (uint8_t*)dsp_output_buffer; // Убедимся, что отправляем буфер DSP
        AUDIO_SpeakerHandler->specific.data_size = IN_CHUNK_SAMPLES_STEREO * 4 * 2 * sizeof(int16_t); // Макс размер выхлопа
      }
      else
      {
        // После вызова ProcessAudio(), AUDIO_SpeakerHandler->specific.data
        // и data_size уже содержат правильные указатели и размеры.
        // Здесь нет необходимости в дополнительных копированиях или условиях
        // типа #if USB_AUDIO_CONFIG_PLAY_USE_FREQ_44_1_K
        // Так как вся логика подготовки данных перенесена в ProcessAudio.

        // Обновляем указатель чтения в кольцевом буфере на основе
        // исходного количества потребленных сэмплов (packet_length).
        AUDIO_SpeakerHandler->buf->rd_ptr += AUDIO_SpeakerHandler->packet_length;
        if (AUDIO_SpeakerHandler->buf->rd_ptr >=
            AUDIO_SpeakerHandler->buf->size)
        {
          AUDIO_SpeakerHandler->buf->rd_ptr -=
            AUDIO_SpeakerHandler->buf->size;
        }
      }

      // <-- ИЗМЕНЕНО: Вызов SpeakerPlay перемещен сюда, после подготовки данных
      // speaker->specific.data и speaker->specific.data_size уже установлены в ProcessAudio.
      AUDIO_SpeakerHandler->SpeakerPlay(
          (uint16_t *)AUDIO_SpeakerHandler->specific.data,
          (uint16_t)AUDIO_SpeakerHandler->specific.data_size,
          AUDIO_SpeakerHandler->node.audio_description->resolution);

#ifdef DEBUG_SPEAKER_NODE
      AUDIO_SpeakerDebugStats[AUDIO_SpeakerDebugStats_count].
        data = AUDIO_SpeakerHandler->specific.data;
      AUDIO_SpeakerDebugStats[AUDIO_SpeakerDebugStats_count].
        injection_size =
        AUDIO_SpeakerHandler->specific.data_size;
      AUDIO_SpeakerDebugStats[AUDIO_SpeakerDebugStats_count].
        read = AUDIO_SpeakerHandler->buf->rd_ptr;
#endif /* DEBUG_SPEAKER_NODE */
    } /* AUDIO_NODE_STARTED */

#ifdef DEBUG_SPEAKER_NODE
      if (++AUDIO_SpeakerDebugStats_count ==
          SPEAKER_DEBUG_BUFFER_SIZE)
      {
        AUDIO_SpeakerDebugStats_count = 0;
      }
#endif /* DEBUG_SPEAKER_NODE */
  } // END if ((AUDIO_SpeakerHandler) && (AUDIO_SpeakerHandler->node.state != AUDIO_NODE_OFF))
}


/* Приватные функции ---------------------------------------------------------*/
/**
  * @brief  AUDIO_SpeakerStart
  *         Запустить узел динамика
  * @param  buffer(IN):     буфер, который будет использоваться во время запуска узла
  * @param  node_handle(IN): дескриптор узла динамика должен быть инициализирован
  * @retval 0 в случае отсутствия ошибок
  */
// Функция AUDIO_SpeakerStart инициализирует узел динамика
static int8_t  AUDIO_SpeakerStart(AUDIO_CircularBuffer_t* buffer,  uint32_t node_handle)
{
  AUDIO_SpeakerNode_t* speaker;

  // Привязываем дескриптор узла динамика к переменной speaker
  speaker = (AUDIO_SpeakerNode_t*)node_handle;
  // Устанавливаем буфер для динамика
  speaker->buf = buffer;
  // Сбрасываем команду динамика
  speaker->specific.cmd = 0;
  // Устанавливаем состояние mute
  AUDIO_SpeakerMute( 0,  speaker->node.audio_description->audio_mute , node_handle);
  // Устанавливаем громкость динамика
  AUDIO_SpeakerSetVolume( 0,  speaker->node.audio_description->audio_volume_db_256 , node_handle);
  // Устанавливаем состояние узла динамика в "запущен"
  speaker->node.state = AUDIO_NODE_STARTED;
  return 0; // Возвращаем 0, если ошибок нет
}

 /**
  * @brief  AUDIO_SpeakerStop
  *         Остановить узел динамика. Динамик будет остановлен после
  *         завершения передачи текущего пакета.
  * @param  node_handle(IN): дескриптор узла динамика
  * @retval 0 в случае отсутствия ошибок
  */
static int8_t  AUDIO_SpeakerStop(uint32_t node_handle)
{
  AUDIO_SpeakerNode_t* speaker;

  speaker = (AUDIO_SpeakerNode_t*)node_handle;
  // Устанавливаем команду остановки динамика
  speaker->specific.cmd |= SPEAKER_CMD_STOP;

  return 0; // Возвращаем 0, если ошибок нет
}

 /**
  * @brief  AUDIO_SpeakerChangeFrequency
  *         изменить частоту, затем остановить узел динамика
  * @param  node_handle: дескриптор узла динамика должен быть запущен
  * @retval 0 в случае отсутствия ошибок
  */
int8_t  AUDIO_SpeakerChangeFrequency( uint32_t node_handle)
{
  AUDIO_SpeakerNode_t* speaker;

  speaker = (AUDIO_SpeakerNode_t*)node_handle;
  // Устанавливаем команду изменения частоты
  speaker->specific.cmd |= SPEAKER_CMD_CHANGE_FREQUENCE;
  return 0; // Возвращаем 0, если ошибок нет
}

 /**
  * @brief  AUDIO_SpeakerInitInjectionsParams
  *         Инициализация параметров инъекций динамика
  * @param  speaker(IN): дескриптор узла динамика должен быть запущен
  * @retval 0 в случае отсутствия ошибок
  */

static void  AUDIO_SpeakerInitInjectionsParams( AUDIO_SpeakerNode_t* speaker)
{
  // Устанавливаем длину пакета на основе аудио описания
  speaker->packet_length = AUDIO_MS_PACKET_SIZE_FROM_AUD_DESC(
    speaker->node.audio_description);

  // <-- ИЗМЕНЕНО: Корректируем packet_length и injection_size
  // Это оригинальный размер пакета, который мы читаем из USB буфера.
   uint32_t original_injection_size;

  // Вычисляем размер инъекции на основе частоты, количества каналов и разрешения
  original_injection_size = AUDIO_MS_PACKET_SIZE( \
    speaker->node.audio_description->frequency, \
    speaker->node.audio_description->channels_count, \
    speaker->node.audio_description->resolution);

  speaker->specific.injection_size = original_injection_size; // По умолчанию
  speaker->specific.alt_buf_half_size = original_injection_size; // По умолчанию

  // Если апсемплинг включен, корректируем injection_size, т.к. это
  // размер данных, подаваемых на SAI.
  if (IsUpsamplingEnabled())
  {
      uint32_t current_freq = speaker->node.audio_description->frequency;
      UpFactor_t upsample_factor;

      if (current_freq < USB_AUDIO_CONFIG_FREQ_88_2_K) // 44.1/48 кГц
      {
          upsample_factor = UP_FACTOR_X4;
      }
      else if (current_freq < USB_AUDIO_CONFIG_FREQ_176_4_K) // 88.2/96 кГц
      {
          upsample_factor = UP_FACTOR_X2;
      }
      else // 176.4/192 кГц (апсемплинг не применяется, или фактор x1)
      {
          upsample_factor = UP_FACTOR_X1; // Или что-то, что сигнализирует не обрабатывать
      }

      // Если upsample_factor > 1, то увеличиваем injection_size
      if (upsample_factor > UP_FACTOR_X1)
      {
          speaker->specific.injection_size = original_injection_size * upsample_factor;
          speaker->specific.alt_buf_half_size = speaker->specific.injection_size; // Альт буфер тоже должен быть увеличен
          // Инициализируем DSP-модуль здесь для соответствующего фактора
          DSP_UpsampleInit(upsample_factor, GetUpsampleAlgo());
      } else {
          // Если upsample_factor = UP_FACTOR_X1 (или апсемплинг выключен),
          // SAI будет работать на оригинальной частоте, injection_size останется оригинальным.
          // Нет необходимости вызывать DSP_UpsampleInit с UP_FACTOR_X1,
          // так как DSP_UpsampleBlock будет вызван только если factor > 1.
      }
  }


  // Инициализация параметров двойного буфера и смещения
  speaker->specific.double_buff = 0;
  speaker->specific.offset = 0;

  // Обнуляем альтернативный буфер (теперь SPEAKER_ALT_BUFFER_SIZE)
  memset(speaker->specific.alt_buffer, 0, SPEAKER_ALT_BUFFER_SIZE);
  speaker->specific.data = speaker->specific.alt_buffer; // Начинаем инъекцию данных
  speaker->specific.data_size = speaker->specific.injection_size; // Устанавливаем размер данных (уже скорректированный)
}


 /**
  * @brief  AUDIO_SpeakerMute
  *         Устанавливает значение заглушки для динамика
  * @param  channel_number(IN): номер канала
  * @param  mute(IN): значение заглушки (0 : заглушить, 1 : включить)
  * @param  node_handle(IN): дескриптор узла динамика
  * @retval  : 0 в случае отсутствия ошибок
  */
static int8_t  AUDIO_SpeakerMute( uint16_t channel_number,  uint8_t mute , uint32_t node_handle)
{
  AudioOutMute(mute);
  return 0;
}

 /**
  * @brief  AUDIO_SpeakerSetVolume
  *         Устанавливает значение громкости для динамика
  * @param  channel_number(IN): номер канала
  * @param  volume_db_256(IN):  значение громкости в дБ
  * @param  node_handle(IN):    дескриптор узла динамика
  * @retval 0 в случае отсутствия ошибок
  */
static int8_t  AUDIO_SpeakerSetVolume( uint16_t channel_number,  int volume_db_256 ,  uint32_t node_handle)
{

  return 0;
}


 /**
  * @brief  AUDIO_SpeakerStartReadCount
  *         Запускает счетчик того, сколько байт было прочитано из буфера (передано в SAI)
  * @param  node_handle: дескриптор узла микрофона, который должен быть запущен
  * @retval  : 0 в случае отсутствия ошибок
  */
static int8_t  AUDIO_SpeakerStartReadCount( uint32_t node_handle)
{
     AUDIO_SpeakerNode_t* speaker;

    speaker = (AUDIO_SpeakerNode_t*)node_handle;
    speaker->specific.dma_remaining = GetRemainingTxSize();//SAI_GetRemainingTxSize();//SAI_MASTER_DMA_STREAM->NDTR;//__HAL_DMA_GET_COUNTER(haudio_out_sai_1.hdmatx);
    return 0;
}

 /**
  * @brief  AUDIO_SpeakerGetLastReadCount
  *         возвращает количество байт, которые были прочитаны, и сбрасывает счетчик
  * @param  node_handle: дескриптор узла динамика, который должен быть запущен
  * @retval  :  количество прочитанных байт, 0 в случае ошибки
  */

static uint16_t  AUDIO_SpeakerGetLastReadCount( uint32_t node_handle)
{
  AUDIO_SpeakerNode_t* speaker;
  int cur_waiting_bytes, read_bytes, last_packet_size;

   speaker = (AUDIO_SpeakerNode_t*)node_handle;
   /* считываем оставшееся значение в dma буфере */
    cur_waiting_bytes = GetRemainingTxSize();//SAI_GetRemainingTxSize();//SAI_MASTER_DMA_STREAM->NDTR; //__HAL_DMA_GET_COUNTER(haudio_out_sai_1.hdmatx);
    last_packet_size = GetLastTxSize();//SAI_GetLastTransferSize();//haudio_out_sai_1.XferSize;
    read_bytes = (speaker->specific.dma_remaining >= cur_waiting_bytes) ?
                 (speaker->specific.dma_remaining - cur_waiting_bytes) :
                 (last_packet_size - cur_waiting_bytes) + speaker->specific.dma_remaining;
    if(read_bytes<(last_packet_size>>1))
    {
      read_bytes+=last_packet_size;
    }
   speaker->specific.dma_remaining = cur_waiting_bytes;

    return read_bytes;
}
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

//-------------------------------------------------------------------
/**
  * @brief  AUDIO_SpeakerChangeResolution
  *         Функция для изменения разрешения динамика.
  * @param  node_handle: дескриптор узла динамика, который необходимо изменить
  * @retval None
  */
void AUDIO_SpeakerChangeResolution( uint32_t node_handle)
{
  AUDIO_SpeakerNode_t* speaker;

  speaker = (AUDIO_SpeakerNode_t*)node_handle;
  // Устанавливаем команду изменения разрешения
  speaker->specific.cmd |= SPEAKER_CMD_CHANGE_RESOLUTION;
}


// <-- НОВОЕ: Функция для обработки аудиоданных (апсемплинг или проброс)
static void ProcessAudio(void)
{
    uint32_t current_freq_hz = AUDIO_SpeakerHandler->node.audio_description->frequency;
    UpFactor_t upsample_factor = UP_FACTOR_X1; // По умолчанию x1 (без апсемплинга)

    // Определяем коэффициент апсемплинга, если функция включена
    if (IsUpsamplingEnabled())
    {
        if (current_freq_hz == USB_AUDIO_CONFIG_FREQ_44_1_K || current_freq_hz == USB_AUDIO_CONFIG_FREQ_48_K)
        {
            upsample_factor = UP_FACTOR_X4;
        }
        else if (current_freq_hz == USB_AUDIO_CONFIG_FREQ_88_2_K || current_freq_hz == USB_AUDIO_CONFIG_FREQ_96_K)
        {
            upsample_factor = UP_FACTOR_X2;
        }
    }

    uint32_t bytes_to_read = AUDIO_SpeakerHandler->packet_length; // Всегда читаем оригинальный размер пакета
    uint32_t samples_to_read = bytes_to_read / (AUDIO_SpeakerHandler->node.audio_description->channels_count * sizeof(int16_t));

    // Копируем данные из кольцевого буфера в dsp_input_buffer
    // Убедимся, что читаем не более чем IN_CHUNK_SAMPLES_STEREO
    uint32_t actual_samples_to_read = samples_to_read;
    if (actual_samples_to_read > IN_CHUNK_SAMPLES_STEREO)
    {
        actual_samples_to_read = IN_CHUNK_SAMPLES_STEREO;
        bytes_to_read = actual_samples_to_read * AUDIO_SpeakerHandler->node.audio_description->channels_count * sizeof(int16_t);
    }
    
    
      uint32_t bytes_till_end = AUDIO_SpeakerHandler->buf->size - AUDIO_SpeakerHandler->buf->rd_ptr;

      if (bytes_till_end < bytes_to_read)
      {
        // Данные разделены на две части: одна в конце, другая в начале
        // Копируем первую часть (до конца буфера)
        memcpy(dsp_input_buffer, 
              AUDIO_SpeakerHandler->buf->data + AUDIO_SpeakerHandler->buf->rd_ptr, 
              bytes_till_end);
        // Копируем вторую часть (с начала буфера)
        memcpy((uint8_t*)dsp_input_buffer + bytes_till_end, 
              AUDIO_SpeakerHandler->buf->data, 
              bytes_to_read - bytes_till_end);
      }
      else
      {
        // Данные лежат сплошным блоком
        memcpy(dsp_input_buffer, 
              AUDIO_SpeakerHandler->buf->data + AUDIO_SpeakerHandler->buf->rd_ptr, 
              bytes_to_read);
      }


    if (upsample_factor > UP_FACTOR_X1)
    {
        // Выполняем апсемплинг
        DSP_UpsampleBlock(dsp_input_buffer, actual_samples_to_read, dsp_output_buffer);

        // Устанавливаем указатель данных и размер для SAI на результат апсемплинга
        AUDIO_SpeakerHandler->specific.data = (uint8_t*)dsp_output_buffer;
        AUDIO_SpeakerHandler->specific.data_size = bytes_to_read * upsample_factor;
    }
    else
    {
        // Если апсемплинг не включен или не нужен для этой частоты, просто передаем входной буфер
        AUDIO_SpeakerHandler->specific.data = (uint8_t*)dsp_input_buffer;
        AUDIO_SpeakerHandler->specific.data_size = bytes_to_read;
    }

  
}