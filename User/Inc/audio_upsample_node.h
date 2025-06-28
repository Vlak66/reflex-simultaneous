/**
 ******************************************************************************
  * @file    audio_upsample_node.h
  * @author  HAND AUDIO
  * @brief   Upsampling
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
#ifndef __AUDIO_UPSAMPLE_NODE_H
#define __AUDIO_UPSAMPLE_NODE_H

#include <stdint.h>
#include "audio_node.h"


void AUDIO_UpsampleNodeInit(AUDIO_UpsampleNode_t* up_node, AUDIO_Description_t* desc, uint8_t factor);
void AUDIO_UpsampleProcess(AUDIO_UpsampleNode_t* up_node, uint8_t* in_buf, uint32_t in_samples, uint8_t* out_buf, uint32_t* out_samples);

#endif // __AUDIO_UPSAMPLE_NODE_H