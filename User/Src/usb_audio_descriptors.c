/*
********************************************************************************
* COPYRIGHT(c) ЗАО «ЧИП и ДИП», 2019, 2020
*
* Программное обеспечение предоставляется на условиях «как есть» (as is).
* При распространении указание автора обязательно.
********************************************************************************
*/

#include "usbd_def.h"
#include "usbd_audio.h"
#include "usb_audio_descriptors.h"
#include "audio_configuration.h"

/*
********************************************************************************
* Этот файл содержит дескрипторы USB аудио для различных конфигураций.
* Включает в себя следующие конфигурации:
* - Стерео 2.0 (16 бит)
*
* Дескрипторы включают в себя стандартные и специфические для класса аудио
* интерфейсы, а также терминалы ввода и вывода.
*
* Файл также содержит определения размеров дескрипторов и идентификаторов
* терминалов и интерфейсов.
********************************************************************************
*/

//------------------------------Stereo 2.0 16---------------------------
const uint8_t USBD_AUDIO_Config_2_0_Stereo[CONFIG_2_0_STEREO_DESCRIPTOR_SIZE] =
{
  /* Конфигурация 1 */
  /* bLength (длина дескриптора) */
  0x09,
  /* bDescriptorType (тип дескриптора: конфигурация) */
  USB_DESC_TYPE_CONFIGURATION,
  /* wTotalLength (общая длина дескриптора, младший байт) */
  LOBYTE(CONFIG_2_0_STEREO_DESCRIPTOR_SIZE),
  /* wTotalLength (общая длина дескриптора, старший байт) */
  HIBYTE(CONFIG_2_0_STEREO_DESCRIPTOR_SIZE),
  /* bNumInterfaces (количество интерфейсов) */
  0x02,
  /* bConfigurationValue (значение конфигурации) */
  0x01,
  /* iConfiguration (индекс строки конфигурации) */
  0x00,
  /* bmAttributes (атрибуты: питание от шины) */
  0x80,
  /* bMaxPower (максимальная потребляемая мощность = 100 мА) */
  0x64,
  /* 09 byte (длина дескриптора в байтах) */

  /* Стандартный дескриптор интерфейса AC: интерфейс управления аудио */
  /* bLength (длина дескриптора) 9 байт - usbd_audio.h */
  USBD_AUDIO_STANDARD_INTERFACE_DESC_SIZE,
  /* bDescriptorType (тип дескриптора: интерфейс) */
  USB_DESC_TYPE_INTERFACE,
  /* bInterfaceNumber (номер интерфейса) */
  0x00,
  /* bAlternateSetting (альтернативная настройка) */
  0x00,
  /* bNumEndpoints (количество конечных точек) */
  0x00,
  /* bInterfaceClass (класс интерфейса: аудио) =0x01 - usbd_audio.h */
  USBD_AUDIO_CLASS_CODE,
  /* bInterfaceSubClass (подкласс интерфейса: управление аудио =0x01 - usbd_audio.h) */
  USBD_AUDIO_INTERFACE_SUBCLASS_AUDIOCONTROL,
  /* bInterfaceProtocol (протокол интерфейса: не определен) =0x00 - usbd_audio.h */
  USBD_AUDIO_INTERFACE_PROTOCOL_UNDEFINED,
  /* iInterface (индекс строки интерфейса) */
  0x00,
  /* 09 byte (длина дескриптора в байтах) */

  /* Классовый дескриптор заголовка интерфейса AC */
  /* bLength (длина дескриптора) */
  0x09,
  /* bDescriptorType (тип дескриптора: специфичный для класса аудио интерфейс) */
  /* =0x24 - usbd_audio.h */
  USBD_AUDIO_DESC_TYPE_CS_INTERFACE,
  /* bDescriptorSubtype (подтип дескриптора - Header Descriptor) */
  /* Header Descriptor служит "вводной частью" для всех класс-специфических
     дескрипторов интерфейса, связанных с аудиоустройством. Он предоставляет
     хосту общую информацию о:

     - Версии USB Audio Class
     - Общей структуре аудиоинтерфейса
     - Количестве и идентификаторах интерфейсов, участвующих в аудиопотоке
     Это позволяет хосту правильно интерпретировать остальные дескрипторы
     и настроить взаимодействие с устройством. */
  /* =0x01 - usbd_audio.h */
  USBD_AUDIO_CS_AC_SUBTYPE_HEADER,
  /* bcdADC (версия спецификации) версия 1.0 - usbd_audio.h */
  /* Поле bcdADC указывает версию USB Audio Class, которую поддерживает устройство:
   * 0x0100 - USB Audio Class 1.0 (UAC1)
   * 0x0200 - USB Audio Class 2.0 (UAC2)
   * 0x0300 - USB Audio Class 3.0 (UAC3)
   *
   * Хост использует эту информацию для выбора подходящего драйвера и протокола:
   * - UAC1: базовые аудиофункции с ограниченным набором возможностей
   * - UAC2/UAC3: поддержка высоких частот дискретизации и многоканального звука
   */
  LOBYTE(USBD_AUDIO_ADC_BCD),
  HIBYTE(USBD_AUDIO_ADC_BCD),
  /* wTotalLength (общая длина дескриптора, младший байт) */
  /* =40 - usb_audio_descriptors.h */
  LOBYTE(CONFIG_2_0_STEREO_AC_TOTAL_SIZE),
  /* wTotalLength (общая длина дескриптора, старший байт) */
  HIBYTE(CONFIG_2_0_STEREO_AC_TOTAL_SIZE),
  /* bInCollection (количество потоковых интерфейсов) */
  0x01,
  /* baInterfaceNr (номер потокового интерфейса для воспроизведения) */
  /* =1 - usb_audio_descriptors.h */
  CONFIG_AUDIO_STREAMING_INTERFACE,
  /* 9 byte (длина дескриптора в байтах) */

  /* USB OUT Терминал для сессии воспроизведения */
  /* Дескриптор входного терминала */
  /* bLength (длина дескриптора) =12 - usbd_audio.h*/
  USBD_AUDIO_INPUT_TERMINAL_DESC_SIZE,
  /* bDescriptorType (тип дескриптора) */
  USBD_AUDIO_DESC_TYPE_CS_INTERFACE,
  /* bDescriptorSubtype (подтип дескриптора Input Terminal Descriptor) */
  /* Input Terminal Descriptor предоставляет информацию о входном терминале
     аудиоустройства. Он описывает:

     - Тип источника сигнала (например, микрофон, линейный вход)
     - Количество каналов и их конфигурацию
     - Характеристики сигнала (например, частота дискретизации)

     Этот дескриптор позволяет хосту (например, компьютеру) понять, какой тип
     аудиосигнала поступает от устройства, и настроить взаимодействие с ним. */
  USBD_AUDIO_CS_AC_SUBTYPE_INPUT_TERMINAL,
  /* bTerminalID (идентификатор терминала) */
  /* =0x12 - usb_audio_descriptors.h */
  CONFIG_TERMINAL_INPUT_ID,
  /* wTerminalType (тип терминала - потоковый USB) 0x0101 */
  LOBYTE(USBD_AUDIO_TERMINAL_IO_USB_STREAMING),
  HIBYTE(USBD_AUDIO_TERMINAL_IO_USB_STREAMING),
  /* bAssocTerminal (связанный терминал) */
  /* 0 - связь отсутствует */
  0x00,
  /* bNrChannels (количество каналов) 2 канала - audio_configuration.h */
  CONFIG_2_0_STEREO_CHANNEL_COUNT,
  /* wChannelConfig (конфигурация каналов) */
  /* 3 - два канала левый и правый - audio_configuration.h*/
  LOBYTE(CONFIG_2_0_STEREO_CHANNEL_MAP),
  HIBYTE(CONFIG_2_0_STEREO_CHANNEL_MAP),
  /* iChannelNames (индекс имен каналов) */
  /* 0 - не используется */
  0x00,
  /* iTerminal (индекс строки терминала) */
  /* 0 - не используется */
  0x00,
  /* 12 байт */

  /* Управление воспроизведением USB */
  /* Дескриптор блока управления (Feature Unit Descriptor) */
  /* bLength (длина дескриптора в байтах) */
  10,
  /* bDescriptorType (тип дескриптора) */
  USBD_AUDIO_DESC_TYPE_CS_INTERFACE,
  /* bDescriptorSubtype (подтип дескриптора - Feature Unit Descriptor) */
  /* Feature Unit Descriptor предоставляет информацию о возможностях управления
     аудиоустройством. Он описывает:

     - Какие функции управления доступны (например, громкость, mute)
     - Количество каналов, к которым применяются эти функции
     - Диапазоны значений для каждой функции (например, минимальная и максимальная громкость)

     Этот дескриптор позволяет хосту (например, компьютеру) управлять аудиоустройством,
     предоставляя пользователю доступ к настройкам громкости, баланса и других параметров. */
  USBD_AUDIO_CS_AC_SUBTYPE_FEATURE_UNIT,
  /* bUnitID (идентификатор блока управления) */
  CONFIG_UNIT_FEATURE_ID,
  /* bSourceID (идентификатор источника - входной терминал) */
  CONFIG_TERMINAL_INPUT_ID,
  /* bControlSize (размер поля управления в байтах) */
  0x01,
  /* bmaControls(0) (управление отключением звука) общее */
  /* Массив битовых масок, описывающих функции управления для каждого канала.
     Каждый элемент массива соответствует одному каналу. Например:

     Бит 0: Mute (отключение звука).
     Бит 1: Volume (регулировка громкости).
     Бит 2: Bass (басы).
     Бит 3: Mid (средние частоты).
     Бит 4: Treble (высокие частоты). */
  USBD_AUDIO_CONTROL_FEATURE_UNIT_MUTE,
  /* bmaControls(1) (управление для канала 1 - не используется) */
  0,
  /* bmaControls(2) (управление для канала 2 - не используется) */
  0,
  /* iTerminal (индекс строки описания - не используется) */
  0x00,
  /* Всего 10 байт */

  /* USB Воспроизведение: Терминал динамика */
  /* Дескриптор выходного терминала */
  /* bLength (длина дескриптора в байтах) */
  USBD_AUDIO_OUTPUT_TERMINAL_DESC_SIZE,
  /* bDescriptorType (тип дескриптора Output Terminal Descriptor) */
  USBD_AUDIO_DESC_TYPE_CS_INTERFACE,
  /* Дескриптор выходного терминала предоставляет информацию
     о выходном терминале аудиоустройства. Он описывает:

     - Тип выходного терминала (например, динамик, наушники).
     - Источник сигнала (например, блок управления Feature Unit).
     - Ассоциированный терминал (если есть).

     Этот дескриптор позволяет хосту (например, компьютеру) понять,
     какой тип аудиосигнала выходит из устройства,
     и настроить взаимодействие с ним.
  */
  /* bDescriptorSubtype (подтип дескриптора) */
  USBD_AUDIO_CS_AC_SUBTYPE_OUTPUT_TERMINAL,
  /* bTerminalID (идентификатор терминала) */
  CONFIG_TERMINAL_OUTPUT_ID,
  /* wTerminalType (тип терминала)  0x0301 */
  /* Тип терминала. Определяет назначение выходного терминала. Например:

    0x0301 — Speaker (динамик).
    0x0302 — Headphones (наушники).
    0x0603 — Line Connector (линейный выход).
  */
  LOBYTE(USBD_AUDIO_TERMINAL_O_SPEAKER),
  HIBYTE(USBD_AUDIO_TERMINAL_O_SPEAKER),
  /* bAssocTerminal (идентификатор ассоциированного терминала) */
  /* 0 - не связан */
  0x00,
  /* bSourceID (идентификатор источника FU 06) */
  CONFIG_UNIT_FEATURE_ID,
  /* iTerminal (индекс строки терминала) */
  0x00,
  /* Всего 09 байт */

  /* Дескриптор стандартного интерфейса AS для воспроизведения аудио - Нулевая полоса пропускания */
  /*
    Zero Bandwidth (нулевая полоса пропускания) - это состояние аудиоинтерфейса,
    при котором поток данных не активен, и интерфейс не использует ресурсы USB.
    Используется для экономии пропускной способности USB, когда устройство
    не передает или не принимает аудиоданные.
    Характеризуется отсутствием активных эндпоинтов (bNumEndpoints = 0x00)
    и альтернативной настройкой bAlternateSetting = 0x00.
  */
  /* Размер дескриптора в байтах 9 байт */
  USBD_AUDIO_STANDARD_INTERFACE_DESC_SIZE,
  /* Тип дескриптора (Standard Interface Descriptor) */
  USB_DESC_TYPE_INTERFACE,
  /* Номер интерфейса 1 - usb_audio_descriptors.h */
  CONFIG_AUDIO_STREAMING_INTERFACE,          /* bInterfaceNumber */
  /* Альтернативная настройка (Zero Bandwidth) */
  0x00,                                         /* bAlternateSetting */
  /* Количество конечных точек (нет активных эндпоинтов в Zero Bandwidth) */
  0x00,                                         /* bNumEndpoints */
  /* Класс интерфейса (Audio Class) =0x01 usbd_audio.h */
  USBD_AUDIO_CLASS_CODE,                       /* bInterfaceClass */
  /* Подкласс интерфейса (Audio Streaming) */
  USBD_AUDIO_INTERFACE_SUBCLASS_AUDIOSTREAMING,                /* bInterfaceSubClass */
  /* Протокол интерфейса (не определен) */
  USBD_AUDIO_INTERFACE_PROTOCOL_UNDEFINED,                     /* bInterfaceProtocol */
  /* Индекс строки описания интерфейса (строка не используется) */
  0x00,                                         /* iInterface */
  /* Всего 9 байт */

  //-------------------------16 bit---------------------------------------------

  /* Дескриптор стандартного интерфейса AS для воспроизведения аудио - Рабочий режим потоковой передачи */
  /* Размер дескриптора в байтах 9 байт*/
  USBD_AUDIO_STANDARD_INTERFACE_DESC_SIZE,
  /* Тип дескриптора (Standard Interface Descriptor) */
  USB_DESC_TYPE_INTERFACE,
  /* Номер интерфейса =1 usb_audio_descriptors.h */
  CONFIG_AUDIO_STREAMING_INTERFACE,
  /* Альтернативная настройка =1 (рабочий режим, 16-битный аудиопоток) */
  ALTERNATE_SETTING_16_BIT,
  /* Количество конечных точек (Endpoints) */
   /*  (две конечные точки: для данных и обратной связи) */
  0x02,
  /* Класс интерфейса (Audio Class) */
  USBD_AUDIO_CLASS_CODE,
  /* Подкласс интерфейса (Audio Streaming) */
  USBD_AUDIO_INTERFACE_SUBCLASS_AUDIOSTREAMING,
  /* Протокол интерфейса (не определен) */
  USBD_AUDIO_INTERFACE_PROTOCOL_UNDEFINED,
  /* Индекс строки описания интерфейса (строка не используется) */
  0x00,
  /* Всего 9 байт */

  /* Дескриптор класс-специфического интерфейса AS */
  /* Размер дескриптора в байтах 7 байт */
  USBD_AUDIO_AS_CS_INTERFACE_DESC_SIZE,
  /* (тип дескриптора Output Terminal Descriptor) */
  USBD_AUDIO_DESC_TYPE_CS_INTERFACE,
  /* Подтип дескриптора =1 (General AS Interface Descriptor) */
  USBD_AUDIO_CS_SUBTYPE_AS_GENERAL,
  /* Идентификатор связанного терминала (Input Terminal ID) */
  CONFIG_TERMINAL_INPUT_ID,
  /* Задержка в миллисекундах (обычно 1 мс) */
  0x01,
  /* Формат аудиоданных 0x001 (PCM - импульсно-кодовая модуляция) */
  LOBYTE(USBD_AUDIO_FORMAT_TYPE_PCM),
  HIBYTE(USBD_AUDIO_FORMAT_TYPE_PCM),
  /* Всего 7 байт */

  /* Дескриптор формата аудио типа I */
  /* Размер дескриптора в байтах (зависит от количества поддерживаемых частот дискретизации =6) */
  USBD_USBD_AUDIO_FORMAT_TYPE_I_DESC_SIZE(CONFIG_2_0_STEREO_16_BIT_FREQ_COUNT),
  /* Тип дескриптора (Class-Specific Interface Descriptor) */
  USBD_AUDIO_DESC_TYPE_CS_INTERFACE,
  /* Подтип дескриптора (Format Type Descriptor) */
  USBD_AUDIO_CS_SUBTYPE_AS_FORMAT_TYPE,
  /* Тип формата аудио (Type I - стандартный PCM) */
  USBD_AUDIO_FORMAT_TYPE_I,
  /* Количество каналов (например, 2 для стерео) */
  CONFIG_2_0_STEREO_CHANNEL_COUNT,
  /* Размер подкадра в байтах (например, 2 байта для 16-битного звука) */
  CONFIG_RES_BYTE_16,
  /* Разрядность аудиоданных (например, 16 бит на выборку) */
  CONFIG_RES_BIT_16,
  /* Количество поддерживаемых частот дискретизации */
  CONFIG_2_0_STEREO_16_BIT_FREQ_COUNT,
  /* Частоты дискретизации, закодированные в 3 байта каждая */
  AUDIO_SAMPLE_FREQ(USB_AUDIO_CONFIG_FREQ_44_1_K), /* 44.1 кГц */
  AUDIO_SAMPLE_FREQ(USB_AUDIO_CONFIG_FREQ_48_K),   /* 48 кГц */
  AUDIO_SAMPLE_FREQ(USB_AUDIO_CONFIG_FREQ_88_2_K), /* 88.2 кГц */
  AUDIO_SAMPLE_FREQ(USB_AUDIO_CONFIG_FREQ_96_K),   /* 96 кГц */
  AUDIO_SAMPLE_FREQ(USB_AUDIO_CONFIG_FREQ_176_4_K),/* 176.4 кГц */
  AUDIO_SAMPLE_FREQ(USB_AUDIO_CONFIG_FREQ_192_K),  /* 192 кГц */
  /* Общий размер дескриптора: (0x08 + количество частот(6) * 3 байта) */

  /* Дескриптор конечной точки для воспроизведения аудио */
  /* Стандартный дескриптор изохронной конечной точки для передачи аудиоданных */
  /* Размер дескриптора в байтах 9 байт */
  USBD_AUDIO_STANDARD_ENDPOINT_DESC_SIZE,
  /* Тип дескриптора (Endpoint Descriptor) */
  USB_DESC_TYPE_ENDPOINT,
  /* Адрес конечной точки (1-я выходная конечная точка) */
  CONFIG_AUDIO_EP_OUT,
  /* Атрибуты конечной точки (изохронная, асинхронная) */
  USBD_EP_TYPE_ISOC|USBD_EP_ATTR_ISOC_ASYNC,
  /* Максимальный размер пакета в байтах (Частота * 2 (стерео) * 2 (размер выборки в байтах)) */
  LOBYTE(CONFIG_2_0_STEREO_16_BIT_MAX_PACKET),  /* (младший байт) */
  HIBYTE(CONFIG_2_0_STEREO_16_BIT_MAX_PACKET),  /* (старший байт) */
  /* Интервал опроса конечной точки (обычно 1 для USB High Speed) */
  0x01,
  /* Частота обновления (не используется, значение 0x00) */
  0x00,
  /* Адрес конечной точки синхронизации (обратная связь) */
  CONFIG_AUDIO_EP_SYNC,
  /* Всего 9 байт */

  /* Дескриптор класс-специфической изохронной конечной точки для передачи аудиоданных */
  /* Размер дескриптора в байтах 7 байт */
  USBD_AUDIO_SPECIFIC_DATA_ENDPOINT_DESC_SIZE,
  /* Тип дескриптора (Class-Specific Endpoint Descriptor) */
  USBD_AUDIO_DESC_TYPE_CS_ENDPOINT,
  /* Подтип дескриптора (General Endpoint Descriptor) */
  USBD_AUDIO_SPECIFIC_EP_DESC_SUBTYPE_GENERAL,
  /* Атрибуты управления: поддержка изменения частоты дискретизации */
  USBD_AUDIO_AS_CONTROL_SAMPLING_FREQUENCY,
  /* Единицы измерения задержки блокировки (не используется, значение 0x00) */
  0x00,
  /* Задержка блокировки (не используется, значение 0x0000) */
  0x00,                                         /*  (младший байт) */
  0x00,                                         /*  (старший байт) */
  /* Всего 7 байт */
  /* Следующий дескриптор специфичен для конечной точки синхронизации */
  /* Дескриптор конечной точки обратной связи для воспроизведения аудио */
  /* Стандартный дескриптор изохронной конечной точки для передачи данных обратной связи */
  /* Размер дескриптора в байтах 9 байт */
  USBD_AUDIO_STANDARD_ENDPOINT_DESC_SIZE,
  /* Тип дескриптора (Endpoint Descriptor) */
  USB_DESC_TYPE_ENDPOINT,
  /* Адрес конечной точки (конечная точка синхронизации) 0x81 */
  CONFIG_AUDIO_EP_SYNC,
  /* Атрибуты конечной точки (изохронная) */
  USBD_EP_TYPE_ISOC,
  /* Максимальный размер пакета в байтах (зависит от частоты дискретизации) */
  LOBYTE(AUDIO_FEEDBACK_EP_PACKET_SIZE),        /* (младший байт) */
  HIBYTE(AUDIO_FEEDBACK_EP_PACKET_SIZE),        /* (старший байт) */
  /* Интервал опроса конечной точки (обычно 1 для USB High Speed) */
  0x01,
  /* Частота обновления =0x07 (количество обновлений за период кадра) */
  CONFIG_AUDIO_FEEDBACK_REFRESH,
  /* Адрес конечной точки синхронизации (не используется, значение 0x00) */
  0,
  /* Всего 9 байт */
};

// Массив строк, содержащих описание продукта для различных аудиоконфигураций.
// Каждая строка соответствует определенной конфигурации аудиоустройства.
const char* PRODUCT_STRINGS[] = {
  "USB HANDAUDIO Stereo 2.0",             // Стерео 2.0
};

// Массив, содержащий размеры дескрипторов конфигурации для каждой аудиоконфигурации.
// Размеры дескрипторов используются для передачи информации о конфигурации USB-устройства.
const uint16_t DESCRIPTOR_SIZE[] = {
  CONFIG_2_0_STEREO_DESCRIPTOR_SIZE,     // Размер дескриптора для стерео 2.0
};

//------------------------------------------------------------------------------
// Функция возвращает дескриптор конфигурации USB-аудиоустройства
// на основе текущей аудиоконфигурации.
uint16_t USB_AUDIO_GetConfigDescriptor(uint8_t **desc) {
  // Получаем текущую аудиоконфигурацию устройства
  uint8_t AudioConfig = GetAudioConfiguration();

  // Если указатель на дескриптор не равен NULL,
  // выбираем дескриптор конфигурации.
  if (desc) {
    *desc = (uint8_t *)USBD_AUDIO_Config_2_0_Stereo;  // Стерео 2.0 (по умолчанию)
  }

  // Возвращаем размер дескриптора для текущей аудиоконфигурации
  return (DESCRIPTOR_SIZE[AudioConfig]);
}

//------------------------------------------------------------------------------
// Функция возвращает строку описания продукта для текущей аудиоконфигурации.
uint8_t* GetProductString(void) {
  // Используем текущую аудиоконфигурацию как индекс для массива PRODUCT_STRINGS
  return (uint8_t*)PRODUCT_STRINGS[GetAudioConfiguration()];
}
//------------------------------------------------------------------------------
