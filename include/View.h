#pragma once

#include "../libs/lvgl/lvgl.h"
#include "../utils/lv_ext/lv_obj_ext_func.h"
#include "../utils/lv_ext/lv_anim_timeline_wrapper.h"
#include <functional>

namespace Page
{

#define LYRIC_SHOW_LINES 6

    typedef enum
    {
        PlayMode_ListLoop = 0, // 列表循环
        PlayMode_SingleLoop,   // 单曲循环
        PlayMode_Random,       // 随机播放
    } PlayMode;

    using ExitCb = std::function<void(void)>;
    using GetStateCb = std::function<int(void)>;
    using PauseCb = std::function<void(void)>;
    using PlayCb = std::function<void(const char *, int)>;
    using SetCurCb = std::function<void(int)>;
    using GetVolumeCb = std::function<void(int *, int *)>;
    using SetVolumeCb = std::function<void(int)>;
    using GetModeCb = std::function<PlayMode(void)>;
    using SetModeCb = std::function<void(PlayMode)>;

    struct Operations
    {
        ExitCb exitCb;
        GetStateCb getStateCb;   // 获取当前音乐状态回调函数
        PauseCb pauseCb;         // 当前音乐暂停回调函数
        PlayCb playCb;           // 音乐播放回调函数
        SetCurCb setCurCb;       // 音乐设置进度回调函数
        GetVolumeCb getVolumeCb; // 获取音乐音量回调函数
        SetVolumeCb setVolumeCb; // 设置音乐音量回调函数
        GetModeCb getModeCb;     // 获取当前播放模式回调函数
        SetModeCb setModeCb;     // 获取当前播放模式回调函数
    };

    class View
    {
    private:
        Operations _opts;                       // View回调函数集
        lv_obj_t *_playingMusicBtn = nullptr;   // 保存当前播放的音乐列表btn
        bool _isPlaying = false;                // 是否音乐播放标志位
        PlayMode _playMode = PlayMode_ListLoop; // 列表播放模式

    public:
        struct
        {
            lv_obj_t *lyricRoller; // 歌词滚轮

            lv_obj_t *cont;
            lv_obj_t *name;
            struct
            {
                lv_ft_info_t font16; // 自定义字体
                lv_ft_info_t font20; // 自定义字体
            } fontCont;
            struct
            {
                lv_obj_t *cont;
                lv_obj_t *btn;       // 播放键 / 暂停键
                lv_obj_t *slider;    // 进度条
                lv_obj_t *timeLabel; // 时间戳
            } btnCont;

            struct
            {
                lv_obj_t *cont;
                lv_obj_t *prevBtn; // 上一首
                lv_obj_t *nextBtn; // 下一首
                lv_obj_t *funcBtn; // Function Button
            } funcCont;

            struct
            {
                lv_obj_t *cont;
                lv_obj_t *slider; // 音量条
            } sliderCont;

            struct
            {
                lv_obj_t *cont;
            } listCont;

            struct
            {
                lv_obj_t *cont;
                lv_obj_t *cancelBtn; // to cancel this app

                lv_obj_t *musicNameLabel; // to show the music name which is playing
            } topCont;

            lv_anim_timeline_t *anim_timeline;
            lv_anim_timeline_t *anim_timelineClick;
            lv_anim_timeline_t *anim_timelineVolume;
            lv_anim_timeline_t *anim_timelineTop;
        } ui;

        void create(Operations &opts);
        void release(void);
        void appearAnimStart(bool reverse = false);
        void appearAnimClick(bool reverse = false);
        void appearAnimVolume(bool reverse = false);
        void appearAnimTop(bool reverse = false);

        void addMusicList(const char *name);
        void setPlayProgress(int cur, int total);
        void loadLyric(const char *lyric);
        void setLyricId(int id, bool isAnim);
        const char *getMusicName(int index);
        void showMusicName(const char *name);

    private:
        void AttachEvent(lv_obj_t *obj);

        void contCreate(lv_obj_t *obj);
        void btnContCreate(lv_obj_t *obj);
        void listContCreate(lv_obj_t *obj);
        void rollerContCreate(lv_obj_t *obj);
        void volumeSliderContCreate(lv_obj_t *obj);
        void funcContCreate(lv_obj_t *obj);
        void topContCreate(lv_obj_t *obj);
        void fontCreate(void);

        static void onEvent(lv_event_t *event);
        static void buttonEventHandler(lv_event_t *event);
        static void listBtnEventHandler(lv_event_t *event);
        static void sliderEventHandler(lv_event_t *event);

        lv_obj_t *roundRectCreate(lv_obj_t *par, lv_coord_t x_ofs, lv_coord_t y_ofs);
        lv_obj_t *btnCreate(lv_obj_t *par, const void *img_src, lv_coord_t x_ofs, lv_coord_t y_ofs, lv_coord_t w = 50, lv_coord_t h = 50);
        lv_obj_t *sliderCreate(lv_obj_t *par, const void *img_src, lv_coord_t x_ofs = 0, lv_coord_t y_ofs = 0, int32_t min = 0, int32_t max = 255, int32_t val = 0);
        lv_obj_t *listCreate(const char *name, const void *img_src);
    };

}