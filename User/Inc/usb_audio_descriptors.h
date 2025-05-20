/*
********************************************************************************
* Права копиригта (c) ЗАО «ЧИП и ДИП», 2019, 2020
*
* Программное обеспечение предоставляется без гарантий (as is).
* При распространении обязательна ссылка на авторство.
********************************************************************************
*/

#ifndef __USB_AUDIO_DESCRIPTORS_H
#define __USB_AUDIO_DESCRIPTORS_H

/* Размеры дескрипторов для разных конфигураций аудиоустройств */
#define CONFIG_2_0_STEREO_DESCRIPTOR_SIZE      (134)       // Стерео 2.0 (16 бит)
#define CONFIG_2_0_STEREO_AC_TOTAL_SIZE        40          // Размер секции Audio Control

/* Идентификаторы терминалов и блоков управления */
#define CONFIG_TERMINAL_INPUT_ID               0x12        // Входной терминал (например, микрофон)
#define CONFIG_UNIT_FEATURE_ID                 0x16        // Блок управления эффектами
#define CONFIG_TERMINAL_OUTPUT_ID              0x14        // Выходной терминал (например, динамики)

/* Конфигурация интерфейсов и эндпоинтов */
#define CONFIG_AUDIO_STREAMING_INTERFACE       1          // Номер интерфейса для потоковой передачи
#define CONFIG_AUDIO_EP_OUT                    0x01        // Адрес эндпоинта вывода (OUT)
#define CONFIG_AUDIO_EP_SYNC                   0x81        // Адрес эндпоинта синхронизации (IN)
#define CONFIG_AUDIO_FEEDBACK_REFRESH          0x07        // Период обновления обратной связи (в мс)

/* Функция для получения строки с названием продукта */
uint8_t* GetProductString(void);

#endif // __USB_AUDIO_DESCRIPTORS_H