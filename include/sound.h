#ifndef _SOUND_H_
#define _SOUND_H_

#include <alsa/asoundlib.h>

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief 初始化PCM音频设备（播放）
     * @return 成功返回0，失败返回-1
     */
    int snd_pcm_init(void);

    /**
     * @brief 设置PCM数据格式
     * @param format 音频格式（如SND_PCM_FORMAT_S16_LE）
     */
    void snd_pcm_set_format(snd_pcm_format_t format);

    /**
     * @brief 设置PCM采样率
     * @param rate 采样率（如44100、48000）
     */
    void snd_pcm_set_rate(unsigned int rate);

    /**
     * @brief 设置PCM声道数
     * @param channels 声道数（1=单声道，2=双声道）
     */
    void snd_pcm_set_channels(unsigned int channels);

    /**
     * @brief 获取PCM周期大小（单位：帧）
     * @return 周期大小
     */
    unsigned int snd_pcm_get_period_size(void);

    /**
     * @brief 设置PCM周期大小
     * @param size 周期大小（单位：帧）
     */
    void snd_pcm_set_period_size(unsigned int size);

    /**
     * @brief 获取PCM设备句柄
     * @return PCM句柄（snd_pcm_t*）
     */
    snd_pcm_t *snd_pcm_get(void);

    /**
     * @brief 关闭PCM设备并释放资源
     */
    void snd_pcm_dev_close(void);

    /**
     * @brief 初始化混音器（控制音量）
     * @return 成功返回0，失败返回-1
     */
    int snd_mixer_init(void);

    /**
     * @brief 关闭混音器并释放资源
     */
    void snd_mixer_dev_close(void);

    /**
     * @brief 获取当前音量和最大音量
     * @param cur 输出参数：当前音量
     * @param max 输出参数：最大音量
     */
    void snd_get_volume(int *cur, int *max);

    /**
     * @brief 设置音量
     * @param v 目标音量（范围：0~max_volume）
     */
    void snd_set_volume(int v);

    #ifdef __cplusplus
}
#endif

#endif
