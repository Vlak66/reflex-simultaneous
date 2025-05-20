/**
  ******************************************************************************
  * @file    usbd_desc.c
  * @author  MCD Application Team
  * @brief   Этот файл предоставляет дескрипторы USB устройства и методы форматирования строк.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019  STMicroelectronics.
  * Все права защищены.</center></h2>
  *
  * Данный программный компонент лицензирован ST под Ultimate Liberty license
  * SLA0044, "Лицензией"; Вы не можете использовать этот файл иначе как в
  * соответствии с Лицензией. Вы можете получить копию Лицензии по адресу:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */

/* Подключаемые файлы ----------------------------------------------------------*/
#include "usbd_core.h"
#include "usbd_desc.h"
#include "usbd_conf.h"
#include "usb_audio.h"
#include "usb_audio_descriptors.h"
#include "audio_configuration.h"

/* Приватные определения типов ------------------------------------------------*/
/* Приватные определения констант ---------------------------------------------*/
#define USBD_VID                      0x0483  // Идентификатор производителя
#define USBD_PID                      0xA210  // Идентификатор продукта
#define USBD_LANGID_STRING            0x409   // Идентификатор языка (английский)
#define USBD_MANUFACTURER_STRING      "ChipDip." // Строка производителя
#define USBD_CONFIGURATION_FS_STRING  "AUDIO Config"    // Строка конфигурации
#define USBD_INTERFACE_FS_STRING      "AUDIO Interface" // Строка интерфейса

#define  USB_SIZ_STRING_SERIAL       0x1A // Размер серийного номера 26 байт

/* Приватные макросы ---------------------------------------------------------*/
/* Прототипы приватных функций -----------------------------------------------*/
uint8_t *USBD_AUDIO_DeviceDescriptor(USBD_SpeedTypeDef speed, uint16_t *length);
uint8_t *USBD_AUDIO_LangIDStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length);
uint8_t *USBD_AUDIO_ManufacturerStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length);
uint8_t *USBD_AUDIO_ProductStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length);
uint8_t *USBD_AUDIO_SerialStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length);
uint8_t *USBD_AUDIO_ConfigStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length);
uint8_t *USBD_AUDIO_InterfaceStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length);
#ifdef USB_SUPPORT_USER_STRING_DESC
uint8_t *USBD_AUDIO_USRStringDesc(USBD_SpeedTypeDef speed, uint8_t idx, uint16_t *length);
#endif /* USB_SUPPORT_USER_STRING_DESC */

/* Приватные переменные ------------------------------------------------------*/
// Структура с указателями на функции получения дескрипторов
USBD_DescriptorsTypeDef AUDIO_Desc = {
  USBD_AUDIO_DeviceDescriptor,          // Дескриптор устройства
  USBD_AUDIO_LangIDStrDescriptor,       // Строковый дескриптор языка
  USBD_AUDIO_ManufacturerStrDescriptor, // Строковый дескриптор производителя
  USBD_AUDIO_ProductStrDescriptor,      // Строковый дескриптор продукта
  USBD_AUDIO_SerialStrDescriptor,       // Строковый дескриптор серийного номера
  USBD_AUDIO_ConfigStrDescriptor,       // Строковый дескриптор конфигурации
  USBD_AUDIO_InterfaceStrDescriptor,    // Строковый дескриптор интерфейса
};

/* Стандартный дескриптор USB устройства */
// Выравнивание данных по 4-байтной границе для оптимизации доступа
__ALIGN_BEGIN uint8_t USBD_DeviceDesc[USB_LEN_DEV_DESC] __ALIGN_END = {
  /* Длина дескриптора 18 байт */
  0x12,
  /* Тип дескриптора 0x01 (Дескриптор устройства) - usb_def.h */
  USB_DESC_TYPE_DEVICE,
  /* Версия USB 2.0 */
  0x00,
  0x02,
  /* Класс устройства 0x00 (класс устройства определяется на уровне интерфейса) */
  0x00,
  /* Подкласс устройства 0x00 (подкласс устройства определяется на уровне интерфейса) */
  0x00,
  /* Протокол устройства 0x00 (протокол устройства определяется на уровне интерфейса) */
  0x00,
  /* Стандартный размер пакета для EP0 64 байта  - usb_def.h*/
  USB_MAX_EP0_SIZE,
  /* Идентификатор производителя */
  LOBYTE(USBD_VID),
  HIBYTE(USBD_VID),
  /* Идентификатор продукта */
  LOBYTE(USBD_PID),
  HIBYTE(USBD_PID),
  /* Версия устройства 2.00 */
  0x00,
  0x02,
  /* Индекс строки производителя 0x01 - usb_def.h   */
  USBD_IDX_MFC_STR,
  /* Индекс строки продукта 0x02 - usb_def.h */
  USBD_IDX_PRODUCT_STR,
  /* Индекс строки серийного номера 0x03 - usb_def.h */
  USBD_IDX_SERIAL_STR,
  /* Максимальное количество конфигураций 0x01 - usb_def.h */
  USBD_MAX_NUM_CONFIGURATION
};

/* Стандартный строковый дескриптор языка USB */
__ALIGN_BEGIN uint8_t USBD_LangIDDesc[USB_LEN_LANGID_STR_DESC] __ALIGN_END = {
  /* Длина дескриптора 4 байт usb_def.h */
  USB_LEN_LANGID_STR_DESC,
  /* Тип дескриптора 0x03 (Строковый дескриптор) - usb_def.h */
  USB_DESC_TYPE_STRING,
  /* Идентификатор языка 0x409 (английский) */
  LOBYTE(USBD_LANGID_STRING),
  HIBYTE(USBD_LANGID_STRING),
};

// Буфер для серийного номера USB устройства длиной 26 байт
uint8_t USBD_StringSerial[USB_SIZ_STRING_SERIAL] =
{
  /* Длина дескриптора 26 байт */
  USB_SIZ_STRING_SERIAL,
  /* Тип дескриптора 0x03 (Строковый дескриптор) - usb_def.h */
  USB_DESC_TYPE_STRING,
};

// Буфер для строковых дескрипторов длиной 256 байт
__ALIGN_BEGIN uint8_t USBD_StrDesc[USBD_MAX_STR_DESC_SIZ] __ALIGN_END;

/* Приватные функции ---------------------------------------------------------*/
static void IntToUnicode (uint32_t value , uint8_t *pbuf , uint8_t len);
static void Get_SerialNum(void);

/**
  * @brief  Возвращает дескриптор устройства.
  * @param  speed: Текущая скорость устройства
  * @param  length: Указатель на переменную длины данных
  * @retval Указатель на буфер дескриптора
  */
uint8_t *USBD_AUDIO_DeviceDescriptor(USBD_SpeedTypeDef speed, uint16_t *length)
{
  (void)speed;
  *length = sizeof(USBD_DeviceDesc);
  return (uint8_t*)USBD_DeviceDesc;
}

/**
  * @brief  Возвращает строковый дескриптор идентификатора языка.
  * @param  speed: Текущая скорость устройства
  * @param  length: Указатель на переменную длины данных
  * @retval Указатель на буфер дескриптора
  */
uint8_t *USBD_AUDIO_LangIDStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length)
{
  (void)speed;
  *length = sizeof(USBD_LangIDDesc);
  return (uint8_t*)USBD_LangIDDesc;
}

/**
  * @brief  Возвращает строковый дескриптор продукта.
  * @param  speed: Текущая скорость устройства
  * @param  length: Указатель на переменную длины данных
  * @retval Указатель на буфер дескриптора
  */
uint8_t *USBD_AUDIO_ProductStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length)
{
  (void)speed;
  /* Получаем строку продукта в Unicode формате в зависимости от текущей конфигурации */
  USBD_GetString(GetProductString(), USBD_StrDesc, length);
  return USBD_StrDesc;
}

/**
  * @brief  Возвращает строковый дескриптор производителя.
  * @param  speed: Текущая скорость устройства
  * @param  length: Указатель на переменную длины данных
  * @retval Указатель на буфер дескриптора
  */
uint8_t *USBD_AUDIO_ManufacturerStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length)
{
  (void)speed;
  /* Получаем строку производителя в Unicode формате */
  USBD_GetString((uint8_t *)USBD_MANUFACTURER_STRING, USBD_StrDesc, length);
  return USBD_StrDesc;
}

/**
  * @brief  Возвращает строковый дескриптор серийного номера.
  * @param  speed: Текущая скорость устройства
  * @param  length: Указатель на переменную длины данных
  * @retval Указатель на буфер дескриптора
  */
uint8_t *USBD_AUDIO_SerialStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length)
{
  (void)speed;
  *length = USB_SIZ_STRING_SERIAL;
  /* Обновляем строковый дескриптор серийного номера данными из уникального ID */
  Get_SerialNum();
  return (uint8_t*)USBD_StringSerial;
}

/**
  * @brief  Возвращает строковый дескриптор конфигурации.
  * @param  speed: Текущая скорость устройства
  * @param  length: Указатель на переменную длины данных
  * @retval Указатель на буфер дескриптора
  */
uint8_t *USBD_AUDIO_ConfigStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length)
{
  (void)speed;
  /* Получаем строку конфигурации в Unicode формате */
  USBD_GetString((uint8_t *)USBD_CONFIGURATION_FS_STRING, USBD_StrDesc, length);
  return USBD_StrDesc;
}

/**
  * @brief  Возвращает строковый дескриптор интерфейса.
  * @param  speed: Текущая скорость устройства
  * @param  length: Указатель на переменную длины данных
  * @retval Указатель на буфер дескриптора
  */
uint8_t *USBD_AUDIO_InterfaceStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length)
{
  (void)speed;
  /* Получаем строку интерфейса в Unicode формате */
  /* GetProductString() - из audio_configuration.c */
  USBD_GetString(GetProductString(), USBD_StrDesc, length);
  return USBD_StrDesc;
}

/**
  * @brief  Создает строковый дескриптор серийного номера
  * @param  Нет параметров
  * @retval Нет возвращаемого значения
  */
static void Get_SerialNum(void)
{
  /* Временный массив для хранения серийного номера */
  uint32_t deviceserial[2];

  /* Создаем серийный номер  - audio_configuration.c */
  MakeSerialNumber(deviceserial);

  /* Если серийный номер не равен 0 */
  if (deviceserial[0] != 0)
  {
    /* Преобразуем первую часть серии в Unicode */
    IntToUnicode (deviceserial[0], (uint8_t*)&USBD_StringSerial[2] ,8);
    /* Преобразуем вторую часть серии в Unicode */
    IntToUnicode (deviceserial[1], (uint8_t*)&USBD_StringSerial[18] ,4);
  }
}

/**
  * @brief  Преобразует 32-битное шестнадцатеричное значение в символы Unicode
  * @param  value: значение для преобразования
  * @param  pbuf: указатель на буфер
  * @param  len: длина буфера
  * @retval Нет возвращаемого значения
  */
static void IntToUnicode (uint32_t value , uint8_t *pbuf , uint8_t len)
{
  uint8_t idx = 0;

  for( idx = 0; idx < len; idx ++)
  {
    if( ((value >> 28)) < 0xA ) // Если цифра 0-9
    {
      pbuf[ 2* idx] = (value >> 28) + '0';
    }
    else // Если буква A-F
    {
      pbuf[2* idx] = (value >> 28) + 'A' - 10;
    }

    value = value << 4; // Сдвигаем на следующую цифру

    pbuf[ 2* idx + 1] = 0; // Добавляем нулевой байт для Unicode
  }
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
