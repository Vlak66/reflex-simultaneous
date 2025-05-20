/*
********************************************************************************
* COPYRIGHT(c) ЗАО «ЧИП и ДИП», 2019, 2020
*
* Программное обеспечение предоставляется на условиях "как есть" (as is).
* При распространении указание автора обязательно.
********************************************************************************
*/


#ifndef __AUDIO_CONFIGURATION_H
#define __AUDIO_CONFIGURATION_H


#include "board.h"
#include "audio_node.h"
#include "usb_audio_constants.h"
#include "audio_speaker_node.h"

/**
 * @brief Включение поддержки USB Audio Class 1.0
 */
#define USE_USB_AUDIO_CLASS_10                 1

/**
 * @brief Альтернативные режимы настройки аудио интерфейса по битности
 */
#define ALTERNATE_SETTING_16_BIT               1
#define ALTERNATE_SETTING_24_BIT               2

/**
 * @brief Конфигурации разрядности аудиосигнала
 */
#define CONFIG_RES_BIT_16                      16   /*!< 16 битов на отсчёт */
#define CONFIG_RES_BYTE_16                     2    /*!< 2 байта на отсчёт */

#define CONFIG_RES_BIT_24                      24   /*!< 24 бита на отсчёт */
#define CONFIG_RES_BYTE_24                     3    /*!< 3 байта на отсчёт */

/**
 * @brief Конфигурация стереофонического звука (2.0)
 */
#define CONFIG_2_0_STEREO_CHANNEL_COUNT        2     /*!< Кол-во каналов: 2 */
#define CONFIG_2_0_STEREO_CHANNEL_MAP          0x03  /*!< Карта каналов: FL (фронтальный левый), FR (фронтальный правый) */
#define CONFIG_2_0_STEREO_16_BIT_FREQ_COUNT    6     /*!< Поддерживаемые частоты при 16 битах: 6 вариантов */
#define CONFIG_2_0_STEREO_24_BIT_FREQ_COUNT    4     /*!< Поддерживаемые частоты при 24 битах: 4 варианта */
#define CONFIG_2_0_STEREO_16_BIT_MAX_PACKET    ((192 + 2) * CONFIG_2_0_STEREO_CHANNEL_COUNT * CONFIG_RES_BYTE_16)
#define CONFIG_2_0_STEREO_24_BIT_MAX_PACKET    ((96 + 2) * CONFIG_2_0_STEREO_CHANNEL_COUNT * CONFIG_RES_BYTE_24)

#define CONFIG_2_0_SAI_COUNT                   1     /*!< Используется один SAI интерфейс */
#define CONFIG_2_0_FREQUENCY_DEFAULT           USB_AUDIO_CONFIG_FREQ_48_K /*!< Частота по умолчанию: 48 кГц */

/**
 * @brief Адреса уникальных ID чипа (для генерации серийного номера устройства)
 */
#define DEVICE_ID1                             ((uint32_t)0x1FFF7A10)
#define DEVICE_ID2                             ((uint32_t)0x1FFF7A14)
#define DEVICE_ID3                             ((uint32_t)0x1FFF7A18)

/**
 * @brief Настройки буферизации и максимальной пропускной способности
 */
#define USB_AUDIO_CONFIG_SAI_MAX_COUNT         CONFIG_2_0_SAI_COUNT
#define USB_AUDIO_CONFIG_PLAY_BUFFER_SIZE      ((1024 * 10) * USB_AUDIO_CONFIG_SAI_MAX_COUNT) /*!< Размер буфера воспроизведения */


/**
 * @brief Типы аудиоконфигураций устройства
 */
#define AUDIO_CONFIG_2_0_STEREO                0

/**
 * @brief Режимы синхронизации устройства
 */
enum SYNC_MODES
{
  MASTER_INT_SYNC,   /*!< Внутренняя синхронизация как мастер */
  MASTER_EXT_SYNC,   /*!< Внешняя синхронизация как мастер */
  SLAVE_SYNC         /*!< Устройство работает в режиме слэва */
};

// =====================================================================
// Функции управления аудиоустройством
// =====================================================================

void SetAudioConfigDependedFuncs(AUDIO_SpeakerNode_t *speaker); /*!< Установка функций, зависящих от текущей конфигурации динамика */
void Play_SAIMaster(uint16_t *Data, uint16_t Size, uint8_t ResByte); /*!< Воспроизведение через SAI в режиме мастера */
void AudioChangeFrequency(uint32_t AudioFrequency); /*!< Изменение частоты воспроизведения */
void ExtSyncSelectSource(uint32_t AudioFrequency); /*!< Настройка источника внешней синхронизации */
void AudioChangeResolution(uint8_t AudioResolution); /*!< Смена битности аудиосигнала */
void AudioOutMute(uint8_t MuteFlag); /*!< Включение/выключение заглушения звука */
uint16_t GetRemainingTxSize(void); /*!< Получить размер оставшихся данных для передачи */
uint16_t GetLastTxSize(void); /*!< Получить размер последней отправленной порции */
void AudioOutInit(uint32_t AudioFrequency, uint8_t AudioResolution); /*!< Инициализация аудиовыхода */
void PlayDescriptionInit(AUDIO_Description_t *Description); /*!< Инициализация описания проигрывания */
uint8_t GetAudioConfiguration(void); /*!< Получить текущую аудиоконфигурацию */
void MakeSerialNumber(uint32_t *Buffer); /*!< Генерация серийного номера из UID */
void AudioConfig_Init(void); /*!< Инициализация аудиоконфигурации */
void OUTClk_Init(void); /*!< Инициализация тактового выхода */
void ExtPowerDisable(void); /*!< Отключение внешнего питания */
void ConfigGPIOs_Init(void); /*!< Инициализация GPIO для аудиорежимов */

#endif // __AUDIO_CONFIGURATION_H

/***************************** END OF FILE ************************************/