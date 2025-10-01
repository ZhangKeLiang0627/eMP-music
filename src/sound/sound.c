#include "sound.h"
#include <string.h>

// PCM设备和混音器配置（适配Linux ALSA）
#define PCM_PLAYBACK_DEV "default"  // 默认PCM播放设备
#define MIXER_DEV "default"         // 默认混音器设备
#define DEFAULT_VOLUME_RATIO 0.8f   // 默认音量比例（80%）

// PCM相关全局变量
static snd_pcm_t *pcm = NULL;                // PCM设备句柄
static snd_pcm_uframes_t period_size = 1152; // 周期大小（单位：帧）
static unsigned int periods = 4;             // 周期数（驱动层缓冲区大小）
static snd_pcm_hw_params_t *hwparams = NULL; // PCM硬件参数对象

// 混音器相关全局变量
static int max_volume = 0;                    // 最大音量值
static snd_mixer_t *mixer = NULL;             // 混音器句柄
static snd_mixer_elem_t *playback_vol_elem = NULL; // 音量控制元素（非NULL表示初始化成功）

/**
 * @brief 初始化PCM音频设备
 * @return 成功返回0，失败返回-1
 */
int snd_pcm_init(void)
{
    int ret;

    // 1. 打开PCM播放设备
    ret = snd_pcm_open(&pcm, PCM_PLAYBACK_DEV, SND_PCM_STREAM_PLAYBACK, 0);
    if (ret < 0)
    {
        fprintf(stderr, "[PCM] snd_pcm_open failed: %s (dev: %s)\n",
                snd_strerror(ret), PCM_PLAYBACK_DEV);
        return -1;
    }

    // 2. 初始化硬件参数对象
    snd_pcm_hw_params_malloc(&hwparams);
    ret = snd_pcm_hw_params_any(pcm, hwparams);
    if (ret < 0)
    {
        fprintf(stderr, "[PCM] snd_pcm_hw_params_any failed: %s\n", snd_strerror(ret));
        goto err_free_hwparams;
    }

    // 3. 设置PCM核心参数（交错模式、16位小端、44.1kHz、双声道）
    // 3.1 访问模式：交错模式（左右声道数据交替存储）
    ret = snd_pcm_hw_params_set_access(pcm, hwparams, SND_PCM_ACCESS_RW_INTERLEAVED);
    if (ret < 0)
    {
        fprintf(stderr, "[PCM] set access failed: %s\n", snd_strerror(ret));
        goto err_free_hwparams;
    }

    // 3.2 数据格式：16位有符号小端（主流音频格式）
    ret = snd_pcm_hw_params_set_format(pcm, hwparams, SND_PCM_FORMAT_S16_LE);
    if (ret < 0)
    {
        fprintf(stderr, "[PCM] set format failed: %s\n", snd_strerror(ret));
        goto err_free_hwparams;
    }

    // 3.3 采样率：44100Hz（CD音质标准）
    ret = snd_pcm_hw_params_set_rate(pcm, hwparams, 44100, 0);
    if (ret < 0)
    {
        fprintf(stderr, "[PCM] set rate failed: %s\n", snd_strerror(ret));
        goto err_free_hwparams;
    }

    // 3.4 声道数：双声道（立体声）
    ret = snd_pcm_hw_params_set_channels(pcm, hwparams, 2);
    if (ret < 0)
    {
        fprintf(stderr, "[PCM] set channels failed: %s\n", snd_strerror(ret));
        goto err_free_hwparams;
    }

    // 3.5 周期大小：按配置设置（自动适配硬件支持的大小）
    ret = snd_pcm_hw_params_set_period_size_near(pcm, hwparams, &period_size, 0);
    if (ret < 0)
    {
        fprintf(stderr, "[PCM] set period size failed: %s\n", snd_strerror(ret));
        goto err_free_hwparams;
    }
    printf("[PCM] period size(frame): %lu\n", period_size);

    // 3.6 周期数：驱动层缓冲区包含4个周期
    ret = snd_pcm_hw_params_set_periods(pcm, hwparams, periods, 0);
    if (ret < 0)
    {
        fprintf(stderr, "[PCM] set periods failed: %s\n", snd_strerror(ret));
        goto err_free_hwparams;
    }

    // 4. 应用硬件参数（使配置生效）
    ret = snd_pcm_hw_params(pcm, hwparams);
    if (ret < 0)
    {
        fprintf(stderr, "[PCM] apply hw params failed: %s\n", snd_strerror(ret));
        goto err_close_pcm;
    }

    printf("[PCM] init success\n");
    return 0;

// 错误处理流程
err_free_hwparams:
    snd_pcm_hw_params_free(hwparams);
    hwparams = NULL;
err_close_pcm:
    snd_pcm_close(pcm);
    pcm = NULL;
    return -1;
}

/**
 * @brief 设置PCM数据格式
 * @param format 音频格式（如SND_PCM_FORMAT_S16_LE）
 */
void snd_pcm_set_format(snd_pcm_format_t format)
{
    if (pcm == NULL || hwparams == NULL)
    {
        fprintf(stderr, "[PCM] set format failed: PCM not initialized\n");
        return;
    }

    int ret = snd_pcm_hw_params_set_format(pcm, hwparams, format);
    if (ret < 0)
    {
        fprintf(stderr, "[PCM] set format failed: %s\n", snd_strerror(ret));
        return;
    }

    ret = snd_pcm_hw_params(pcm, hwparams);
    if (ret < 0)
    {
        fprintf(stderr, "[PCM] apply format failed: %s\n", snd_strerror(ret));
    }
}

/**
 * @brief 设置PCM采样率
 * @param rate 采样率（如44100、48000）
 */
void snd_pcm_set_rate(unsigned int rate)
{
    if (pcm == NULL || hwparams == NULL)
    {
        fprintf(stderr, "[PCM] set rate failed: PCM not initialized\n");
        return;
    }

    int ret = snd_pcm_hw_params_set_rate(pcm, hwparams, rate, 0);
    if (ret < 0)
    {
        fprintf(stderr, "[PCM] set rate failed: %s\n", snd_strerror(ret));
        return;
    }

    ret = snd_pcm_hw_params(pcm, hwparams);
    if (ret < 0)
    {
        fprintf(stderr, "[PCM] apply rate failed: %s\n", snd_strerror(ret));
    }
}

/**
 * @brief 设置PCM声道数
 * @param channels 声道数（1=单声道，2=双声道）
 */
void snd_pcm_set_channels(unsigned int channels)
{
    if (pcm == NULL || hwparams == NULL)
    {
        fprintf(stderr, "[PCM] set channels failed: PCM not initialized\n");
        return;
    }

    int ret = snd_pcm_hw_params_set_channels(pcm, hwparams, channels);
    if (ret < 0)
    {
        fprintf(stderr, "[PCM] set channels failed: %s\n", snd_strerror(ret));
        return;
    }

    ret = snd_pcm_hw_params(pcm, hwparams);
    if (ret < 0)
    {
        fprintf(stderr, "[PCM] apply channels failed: %s\n", snd_strerror(ret));
    }
}

/**
 * @brief 获取PCM周期大小（单位：帧）
 * @return 周期大小
 */
unsigned int snd_pcm_get_period_size(void)
{
    return period_size;
}

/**
 * @brief 设置PCM周期大小
 * @param size 周期大小（单位：帧）
 */
void snd_pcm_set_period_size(unsigned int size)
{
    if (pcm == NULL || hwparams == NULL)
    {
        fprintf(stderr, "[PCM] set period size failed: PCM not initialized\n");
        return;
    }

    period_size = size;
    int ret = snd_pcm_hw_params_set_period_size(pcm, hwparams, period_size, 0);
    if (ret < 0)
    {
        fprintf(stderr, "[PCM] set period size failed: %s\n", snd_strerror(ret));
        return;
    }

    ret = snd_pcm_hw_params(pcm, hwparams);
    if (ret < 0)
    {
        fprintf(stderr, "[PCM] apply period size failed: %s\n", snd_strerror(ret));
    }
}

/**
 * @brief 获取PCM设备句柄
 * @return PCM句柄（snd_pcm_t*）
 */
snd_pcm_t *snd_pcm_get(void)
{
    return pcm;
}

/**
 * @brief 关闭PCM设备并释放资源
 */
void snd_pcm_dev_close(void)
{
    if (hwparams != NULL)
    {
        snd_pcm_hw_params_free(hwparams);
        hwparams = NULL;
    }
    if (pcm != NULL)
    {
        snd_pcm_close(pcm);
        pcm = NULL;
    }
    printf("[PCM] device closed\n");
}

/**
 * @brief 初始化混音器（查找系统实际存在的音量控制项）
 * @return 成功返回0，失败返回-1
 */
int snd_mixer_init(void)
{
    snd_mixer_elem_t *elem = NULL;
    const char *elem_name = NULL;
    long vol_min, vol_max;
    int ret;

    // 1. 打开混音器
    ret = snd_mixer_open(&mixer, 0);
    if (ret < 0)
    {
        fprintf(stderr, "[Mixer] snd_mixer_open failed: %s\n", snd_strerror(ret));
        goto err_return;
    }

    // 2. 关联混音器到声卡设备
    ret = snd_mixer_attach(mixer, MIXER_DEV);
    if (ret < 0)
    {
        fprintf(stderr, "[Mixer] snd_mixer_attach failed: %s (dev: %s)\n",
                snd_strerror(ret), MIXER_DEV);
        goto err_close_mixer;
    }

    // 3. 注册混音器元素（使用默认配置）
    ret = snd_mixer_selem_register(mixer, NULL, NULL);
    if (ret < 0)
    {
        fprintf(stderr, "[Mixer] snd_mixer_selem_register failed: %s\n", snd_strerror(ret));
        goto err_close_mixer;
    }

    // 4. 加载混音器（获取所有控制项）
    ret = snd_mixer_load(mixer);
    if (ret < 0)
    {
        fprintf(stderr, "[Mixer] snd_mixer_load failed: %s\n", snd_strerror(ret));
        goto err_close_mixer;
    }

    // 5. 遍历控制项，优先查找 "PCM Playback Volume"（适配你的系统）
    // 若PCM控制项不存在，再查找 "Master Playback Volume"（双重保险）
    for (elem = snd_mixer_first_elem(mixer); elem != NULL; elem = snd_mixer_elem_next(elem))
    {
        elem_name = snd_mixer_selem_get_name(elem);
        if (elem_name == NULL)
            continue;

        // 调试：打印所有找到的控制项（可选，方便排查）
        // printf("[Mixer] found control: %s\n", elem_name);

        // 优先匹配 PCM 音量控制（推荐，不影响系统全局音量）
        if (strcmp(elem_name, "PCM Playback Volume") == 0)
        {
            playback_vol_elem = elem;
            break;
        }
        // 备用：匹配 Master 音量控制（系统全局音量）
        else if (strcmp(elem_name, "Master Playback Volume") == 0 && playback_vol_elem == NULL)
        {
            playback_vol_elem = elem;
            break;
        }
    }

    // 6. 检查是否找到有效控制项
    if (playback_vol_elem == NULL)
    {
        fprintf(stderr, "[Mixer] no valid volume control found (need PCM/Master Playback Volume)\n");
        goto err_close_mixer;
    }

    // 7. 初始化音量：设置为最大音量的80%，并记录最大音量
    ret = snd_mixer_selem_get_playback_volume_range(playback_vol_elem, &vol_min, &vol_max);
    if (ret < 0)
    {
        fprintf(stderr, "[Mixer] get volume range failed: %s\n", snd_strerror(ret));
        goto err_close_mixer;
    }
    max_volume = vol_max;

    // 设置默认音量（80%）
    long default_vol = (vol_max - vol_min) * DEFAULT_VOLUME_RATIO + vol_min;
    ret = snd_mixer_selem_set_playback_volume_all(playback_vol_elem, default_vol);
    if (ret < 0)
    {
        fprintf(stderr, "[Mixer] set default volume failed: %s\n", snd_strerror(ret));
        goto err_close_mixer;
    }

    // 打印初始化信息
    printf("[Mixer] init success\n");
    printf("[Mixer] control name: %s\n", snd_mixer_selem_get_name(playback_vol_elem));
    printf("[Mixer] volume range: %ld ~ %ld\n", vol_min, vol_max);
    printf("[Mixer] default volume: %ld (%.0f%%)\n", default_vol, DEFAULT_VOLUME_RATIO * 100);
    return 0;

// 错误处理流程
err_close_mixer:
    snd_mixer_close(mixer);
    mixer = NULL;
err_return:
    fprintf(stderr, "[Mixer] init failed\n");
    return -1;
}

/**
 * @brief 关闭混音器并释放资源
 */
void snd_mixer_dev_close(void)
{
    if (mixer != NULL)
    {
        snd_mixer_close(mixer);
        mixer = NULL;
        playback_vol_elem = NULL;
        max_volume = 0;
        printf("[Mixer] device closed\n");
    }
}

/**
 * @brief 获取当前音量和最大音量
 * @param cur 输出参数：当前音量
 * @param max 输出参数：最大音量
 */
void snd_get_volume(int *cur, int *max)
{
    long vol;

    // 空指针检查（避免断言失败）
    if (playback_vol_elem == NULL || cur == NULL || max == NULL)
    {
        fprintf(stderr, "[Mixer] get volume failed: invalid pointer\n");
        *cur = 0;
        *max = 0;
        return;
    }

    // 获取左声道音量（通常左右声道音量一致）
    if (snd_mixer_selem_get_playback_volume(playback_vol_elem, SND_MIXER_SCHN_FRONT_LEFT, &vol) < 0)
    {
        fprintf(stderr, "[Mixer] get current volume failed\n");
        *cur = 0;
    }
    else
    {
        *cur = (int)vol;
    }
    *max = max_volume;
}

/**
 * @brief 设置音量
 * @param v 目标音量（范围：0~max_volume）
 */
void snd_set_volume(int v)
{
    // 空指针检查
    if (playback_vol_elem == NULL)
    {
        fprintf(stderr, "[Mixer] set volume failed: mixer not initialized\n");
        return;
    }

    // 音量范围限制（避免超出硬件支持范围）
    if (v < 0)
        v = 0;
    else if (v > max_volume)
        v = max_volume;

    // 设置所有声道音量
    if (snd_mixer_selem_set_playback_volume_all(playback_vol_elem, v) < 0)
    {
        fprintf(stderr, "[Mixer] set volume to %d failed\n", v);
    }
    else
    {
        printf("[Mixer] volume set to %d (%.0f%%)\n", v, (float)v / max_volume * 100);
    }
}
    