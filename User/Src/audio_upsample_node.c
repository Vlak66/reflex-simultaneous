// audio_upsample_node.c
#include "audio_upsample_node.h"

void AUDIO_UpsampleNodeInit(AUDIO_UpsampleNode_t* up_node, AUDIO_Description_t* desc, uint8_t factor)
{
    // Обнуляем структуру (на всякий случай)
    memset(up_node, 0, sizeof(AUDIO_UpsampleNode_t));
    // Привязываем описание аудио
    up_node->node.audio_description = desc;
    // Указываем тип узла
    up_node->node.type = AUDIO_PROCESSING;
    // Ставим состояние "инициализирован"
    up_node->node.state = AUDIO_NODE_INITIALIZED;
    // Сохраняем коэффициент апсемплинга
    up_node->upsample_factor = factor;
    // Здесь можно добавить выбор метода (если потребуется)
}


// Linear interpolation for 16-bit stereo
void AUDIO_UpsampleProcess(AUDIO_UpsampleNode_t* up_node, uint8_t* in_buf, uint32_t in_samples, uint8_t* out_buf, uint32_t* out_samples) {
    uint8_t factor = up_node->upsample_factor;
    // Пример для 16 бит, стерео:
    int16_t* src = (int16_t*)in_buf;
    int16_t* dst = (int16_t*)out_buf;
    uint32_t channels = up_node->node.audio_description->channels_count;
    uint32_t out_idx = 0;
    for (uint32_t i = 0; i < in_samples - 1; ++i) {
        for (uint8_t ch = 0; ch < channels; ++ch) {
            int16_t s0 = src[i * channels + ch];
            int16_t s1 = src[(i + 1) * channels + ch];
            for (uint8_t k = 0; k < factor; ++k) {
                dst[out_idx * channels + ch] = s0 + ((s1 - s0) * k) / factor;
            }
        }
        out_idx += factor;
    }
    // Последний сэмпл
    for (uint8_t ch = 0; ch < channels; ++ch)
        dst[out_idx * channels + ch] = src[(in_samples - 1) * channels + ch];
    *out_samples = in_samples * factor;

}