#include "View.h"

using namespace Page;

void View::create(Operations &opts)
{
    // 获取View回调函数集
    _opts = opts;

    // 初始化字体
    fontCreate();
    // 总画布的创建
    contCreate(lv_scr_act());
    // 播放列表画布的创建
    listContCreate(ui.cont);
    // 创建歌词滚筒
    rollerContCreate(ui.cont);
    // 功能按钮画布的创建
    funcContCreate(ui.cont);
    // 按钮画布的创建
    btnContCreate(ui.cont);
    // 顶部画布的创建
    topContCreate(ui.cont);
    // 音量条画布的创建
    volumeSliderContCreate(ui.cont);

    // 为当前屏幕添加事件回调函数
    AttachEvent(lv_scr_act());
    // 为播放/暂停键添加事件回调函数
    lv_obj_add_event_cb(ui.btnCont.btn, buttonEventHandler, LV_EVENT_ALL, this);
    lv_obj_add_event_cb(ui.funcCont.prevBtn, buttonEventHandler, LV_EVENT_ALL, this);
    lv_obj_add_event_cb(ui.funcCont.nextBtn, buttonEventHandler, LV_EVENT_ALL, this);
    lv_obj_add_event_cb(ui.funcCont.funcBtn, buttonEventHandler, LV_EVENT_ALL, this);
    lv_obj_add_event_cb(ui.topCont.cancelBtn, buttonEventHandler, LV_EVENT_ALL, this);
    // 为进度条添加事件回调函数
    lv_obj_add_event_cb(ui.btnCont.slider, sliderEventHandler, LV_EVENT_ALL, this);
    // 为音量条添加事件回调函数
    lv_obj_add_event_cb(ui.sliderCont.slider, sliderEventHandler, LV_EVENT_ALL, this);

    // 动画的创建
    ui.anim_timeline = lv_anim_timeline_create();
    ui.anim_timelineClick = lv_anim_timeline_create();
    ui.anim_timelineVolume = lv_anim_timeline_create();
    ui.anim_timelineTop = lv_anim_timeline_create();

#define ANIM_DEF(start_time, obj, attr, start, end) \
    {start_time, obj, LV_ANIM_EXEC(attr), start, end, 500, lv_anim_path_ease_out, true}

#define ANIM_OPA_DEF(start_time, obj) \
    ANIM_DEF(start_time, obj, opa_scale, LV_OPA_COVER, LV_OPA_TRANSP)

    lv_anim_timeline_wrapper_t wrapper[] =
        {
            ANIM_DEF(0, ui.btnCont.cont, height, 20, lv_obj_get_height(ui.btnCont.cont)),
            ANIM_DEF(0, ui.btnCont.cont, width, 20, lv_obj_get_width(ui.btnCont.cont)),
            ANIM_DEF(100, ui.funcCont.cont, y, 480, lv_obj_get_y_aligned(ui.funcCont.cont)),
            LV_ANIM_TIMELINE_WRAPPER_END // 这个标志着结构体成员结束，不能省略，在下面函数lv_anim_timeline_add_wrapper的轮询中做判断条件
        };
    lv_anim_timeline_add_wrapper(ui.anim_timeline, wrapper);

    lv_anim_timeline_wrapper_t wrapperVolume[] =
        {
            ANIM_DEF(0, ui.sliderCont.cont, x, lv_obj_get_x_aligned(ui.sliderCont.cont), 100),
            ANIM_DEF(0, ui.sliderCont.cont, y, lv_obj_get_y_aligned(ui.sliderCont.cont), -50),

            LV_ANIM_TIMELINE_WRAPPER_END // 这个标志着结构体成员结束，不能省略，在下面函数lv_anim_timeline_add_wrapper的轮询中做判断条件
        };
    lv_anim_timeline_add_wrapper(ui.anim_timelineVolume, wrapperVolume);

    lv_anim_timeline_wrapper_t wrapperTop[] =
    {
        ANIM_DEF(0, ui.topCont.cont, y, -40, lv_obj_get_x_aligned(ui.topCont.cont)),
        ANIM_DEF(0, ui.topCont.cont, width, 20, lv_obj_get_width(ui.topCont.cont)),

        LV_ANIM_TIMELINE_WRAPPER_END // 这个标志着结构体成员结束，不能省略，在下面函数lv_anim_timeline_add_wrapper的轮询中做判断条件
    };
    lv_anim_timeline_add_wrapper(ui.anim_timelineTop, wrapperTop);

    lv_coord_t xOriginal = lv_obj_get_x_aligned(lv_obj_get_child(ui.btnCont.cont, 1));
    lv_coord_t yOriginal = lv_obj_get_y_aligned(lv_obj_get_child(ui.btnCont.cont, 1));

    lv_anim_timeline_wrapper_t wrapperForClick[] =
        {
            ANIM_DEF(0, lv_obj_get_child(ui.btnCont.cont, 1), x, xOriginal, xOriginal - 30),
            ANIM_OPA_DEF(300, lv_obj_get_child(ui.btnCont.cont, 1)),
            {300, lv_obj_get_child(ui.btnCont.cont, 1), (lv_anim_exec_xcb_t)lv_obj_set_shadow_opa_scale, LV_OPA_COVER, LV_OPA_TRANSP, 500, lv_anim_path_ease_out, true},

            ANIM_DEF(0, lv_obj_get_child(ui.btnCont.cont, 2), x, xOriginal, xOriginal - 21),
            ANIM_DEF(0, lv_obj_get_child(ui.btnCont.cont, 2), y, yOriginal, yOriginal - 21),
            ANIM_OPA_DEF(300, lv_obj_get_child(ui.btnCont.cont, 2)),
            {300, lv_obj_get_child(ui.btnCont.cont, 2), (lv_anim_exec_xcb_t)lv_obj_set_shadow_opa_scale, LV_OPA_COVER, LV_OPA_TRANSP, 500, lv_anim_path_ease_out, true},

            ANIM_DEF(0, lv_obj_get_child(ui.btnCont.cont, 3), y, yOriginal, yOriginal - 30),
            ANIM_OPA_DEF(300, lv_obj_get_child(ui.btnCont.cont, 3)),
            {300, lv_obj_get_child(ui.btnCont.cont, 3), (lv_anim_exec_xcb_t)lv_obj_set_shadow_opa_scale, LV_OPA_COVER, LV_OPA_TRANSP, 500, lv_anim_path_ease_out, true},

            ANIM_DEF(0, lv_obj_get_child(ui.btnCont.cont, 4), x, xOriginal, xOriginal + 21),
            ANIM_DEF(0, lv_obj_get_child(ui.btnCont.cont, 4), y, yOriginal, yOriginal - 21),
            ANIM_OPA_DEF(300, lv_obj_get_child(ui.btnCont.cont, 4)),
            {300, lv_obj_get_child(ui.btnCont.cont, 4), (lv_anim_exec_xcb_t)lv_obj_set_shadow_opa_scale, LV_OPA_COVER, LV_OPA_TRANSP, 500, lv_anim_path_ease_out, true},

            ANIM_DEF(0, lv_obj_get_child(ui.btnCont.cont, 5), x, xOriginal, xOriginal + 30),
            ANIM_OPA_DEF(300, lv_obj_get_child(ui.btnCont.cont, 5)),
            {300, lv_obj_get_child(ui.btnCont.cont, 5), (lv_anim_exec_xcb_t)lv_obj_set_shadow_opa_scale, LV_OPA_COVER, LV_OPA_TRANSP, 500, lv_anim_path_ease_out, true},

            ANIM_DEF(0, lv_obj_get_child(ui.btnCont.cont, 6), x, xOriginal, xOriginal + 21),
            ANIM_DEF(0, lv_obj_get_child(ui.btnCont.cont, 6), y, yOriginal, yOriginal + 21),
            ANIM_OPA_DEF(300, lv_obj_get_child(ui.btnCont.cont, 6)),
            {300, lv_obj_get_child(ui.btnCont.cont, 6), (lv_anim_exec_xcb_t)lv_obj_set_shadow_opa_scale, LV_OPA_COVER, LV_OPA_TRANSP, 500, lv_anim_path_ease_out, true},

            ANIM_DEF(0, lv_obj_get_child(ui.btnCont.cont, 7), y, yOriginal, yOriginal + 30),
            ANIM_OPA_DEF(300, lv_obj_get_child(ui.btnCont.cont, 7)),
            {300, lv_obj_get_child(ui.btnCont.cont, 7), (lv_anim_exec_xcb_t)lv_obj_set_shadow_opa_scale, LV_OPA_COVER, LV_OPA_TRANSP, 500, lv_anim_path_ease_out, true},

            ANIM_DEF(0, lv_obj_get_child(ui.btnCont.cont, 8), x, xOriginal, xOriginal - 21),
            ANIM_DEF(0, lv_obj_get_child(ui.btnCont.cont, 8), y, yOriginal, yOriginal + 21),
            ANIM_OPA_DEF(300, lv_obj_get_child(ui.btnCont.cont, 8)),
            {300, lv_obj_get_child(ui.btnCont.cont, 8), (lv_anim_exec_xcb_t)lv_obj_set_shadow_opa_scale, LV_OPA_COVER, LV_OPA_TRANSP, 500, lv_anim_path_ease_out, true},

            LV_ANIM_TIMELINE_WRAPPER_END // 这个标志着结构体成员结束，不能省略，在下面函数lv_anim_timeline_add_wrapper的轮询中做判断条件
        };
    lv_anim_timeline_add_wrapper(ui.anim_timelineClick, wrapperForClick);

    // 开始动画
    appearAnimStart();
    appearAnimVolume();
    appearAnimTop();
}

void View::release()
{
    if (ui.anim_timeline)
    {
        lv_anim_timeline_del(ui.anim_timeline);
        ui.anim_timeline = nullptr;
    }
    if (ui.anim_timelineClick)
    {
        lv_anim_timeline_del(ui.anim_timelineClick);
        ui.anim_timelineClick = nullptr;
    }
    if (ui.anim_timelineVolume)
    {
        lv_anim_timeline_del(ui.anim_timelineVolume);
        ui.anim_timelineVolume = nullptr;
    }
    // 移除屏幕手势回调函数
    lv_obj_remove_event_cb(lv_scr_act(), onEvent);

    // 释放内存
    lv_obj_t *listBtn = nullptr;
    while ((listBtn = lv_obj_get_child(ui.listCont.cont, -1)) != nullptr)
    {
        char *music_name = (char *)lv_obj_get_user_data(listBtn);
        if (music_name != nullptr)
            delete[] music_name;

        lv_obj_del(listBtn);
    }
}

void View::appearAnimStart(bool reverse) // 开始开场动画
{
    lv_anim_timeline_set_reverse(ui.anim_timeline, reverse);
    lv_anim_timeline_start(ui.anim_timeline);
}

void View::appearAnimClick(bool reverse) // 按钮动画
{
    lv_anim_timeline_set_reverse(ui.anim_timelineClick, reverse);
    lv_anim_timeline_start(ui.anim_timelineClick);
}

void View::appearAnimVolume(bool reverse) // 音量条动画
{
    lv_anim_timeline_set_reverse(ui.anim_timelineVolume, reverse);
    lv_anim_timeline_start(ui.anim_timelineVolume);
}

void View::appearAnimTop(bool reverse) // topCont动画
{
    lv_anim_timeline_set_reverse(ui.anim_timelineTop, reverse);
    lv_anim_timeline_start(ui.anim_timelineTop);

    // ui.isTopContCollapsed = reverse;
}

void View::AttachEvent(lv_obj_t *obj)
{
    lv_obj_add_event_cb(obj, onEvent, LV_EVENT_ALL, this);
}

// 自定义字体初始化
void View::fontCreate(void)
{
    ui.fontCont.font16.name = "/mnt/UDISK/font/SmileySans.ttf";
    ui.fontCont.font16.weight = 16;
    ui.fontCont.font16.style = FT_FONT_STYLE_NORMAL;
    ui.fontCont.font16.mem = nullptr;
    lv_ft_font_init(&ui.fontCont.font16);

    ui.fontCont.font20.name = "/mnt/UDISK/font/SmileySans.ttf";
    ui.fontCont.font20.weight = 20;
    ui.fontCont.font20.style = FT_FONT_STYLE_NORMAL;
    ui.fontCont.font20.mem = nullptr;
    lv_ft_font_init(&ui.fontCont.font20);
}

// 总画布的创建
void View::contCreate(lv_obj_t *obj)
{
    lv_obj_t *cont = lv_obj_create(obj);
    lv_obj_remove_style_all(cont);
    lv_obj_set_size(cont, LV_HOR_RES, LV_VER_RES);
    lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_opa(cont, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(cont, lv_color_hex(0xdddddd), 0);
    // lv_obj_set_style_bg_img_src(cont, "S:./res/icon/main1.bin", 0);
    lv_obj_set_style_bg_img_opa(cont, LV_OPA_COVER, 0);
    lv_obj_align(cont, LV_ALIGN_CENTER, 0, 0);
    ui.cont = cont;

    // lv_obj_t *label = lv_label_create(ui.cont);
    // lv_obj_remove_style_all(label);
    // lv_obj_set_style_text_font(label, ui.fontCont.font20.font, 0);
    // lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 0);
    // lv_label_set_text_fmt(label, "%s", "songName");
    // ui.name = label;
}

// 按钮画布的创建
void View::btnContCreate(lv_obj_t *obj)
{
    lv_obj_t *btnCont = lv_obj_create(obj);
    lv_obj_remove_style_all(btnCont);
    lv_obj_set_size(btnCont, lv_pct(90), LV_VER_RES / 4);
    lv_obj_clear_flag(btnCont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_opa(btnCont, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(btnCont, lv_color_hex(0x6a8d6d), 0);
    lv_obj_align(btnCont, LV_ALIGN_BOTTOM_MID, 0, 50);
    lv_obj_set_style_radius(btnCont, 16, LV_PART_MAIN);
    ui.btnCont.cont = btnCont;

    lv_obj_t *btn = btnCreate(btnCont, LV_SYMBOL_PLAY, 10, -25);
    ui.btnCont.btn = btn;

    /* Render octagon explode */
    lv_obj_t *roundRect_1 = roundRectCreate(btnCont, 30, -25);
    lv_obj_t *roundRect_2 = roundRectCreate(btnCont, 30, -25);
    lv_obj_t *roundRect_3 = roundRectCreate(btnCont, 30, -25);
    lv_obj_t *roundRect_4 = roundRectCreate(btnCont, 30, -25);
    lv_obj_t *roundRect_5 = roundRectCreate(btnCont, 30, -25);
    lv_obj_t *roundRect_6 = roundRectCreate(btnCont, 30, -25);
    lv_obj_t *roundRect_7 = roundRectCreate(btnCont, 30, -25);
    lv_obj_t *roundRect_8 = roundRectCreate(btnCont, 30, -25);

    lv_obj_t *slider = sliderCreate(btnCont, nullptr, 30, -15);
    ui.btnCont.slider = slider;

    lv_obj_t *label = lv_label_create(btnCont);
    lv_obj_remove_style_all(label);
    lv_obj_set_style_text_font(label, ui.fontCont.font16.font, LV_STATE_DEFAULT);
    lv_obj_align(label, LV_ALIGN_TOP_MID, -100, 5);
    lv_label_set_text_fmt(label, "%s", "0:0/0:0");
    ui.btnCont.timeLabel = label;
}

// 功能按键画布的创建
void View::funcContCreate(lv_obj_t *obj)
{
    lv_obj_t *cont = lv_obj_create(obj);
    lv_obj_remove_style_all(cont);
    lv_obj_set_size(cont, lv_pct(26), lv_pct(10));
    lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_opa(cont, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(cont, lv_color_hex(0xff9b5e), 0);
    lv_obj_align(cont, LV_ALIGN_BOTTOM_LEFT, 40, -60);
    lv_obj_set_style_radius(cont, 16, LV_PART_MAIN);

    lv_obj_set_style_shadow_width(cont, 10, 0);
    lv_obj_set_style_shadow_ofs_x(cont, 4, 0);
    lv_obj_set_style_shadow_ofs_y(cont, 2, 0);
    lv_obj_set_style_shadow_color(cont, lv_color_hex(0xe36f47), 0);

    ui.funcCont.cont = cont;

    lv_obj_t *btn = nullptr;
    btn = btnCreate(cont, LV_SYMBOL_PREV, 6, -4, 30, 30);
    ui.funcCont.prevBtn = btn;

    btn = btnCreate(cont, LV_SYMBOL_NEXT, 47, -4, 30, 30);
    ui.funcCont.nextBtn = btn;

    btn = btnCreate(cont, LV_SYMBOL_LOOP, 88, -4, 30, 30);
    ui.funcCont.funcBtn = btn;
}

// 音量条画布的创建
void View::volumeSliderContCreate(lv_obj_t *obj)
{
    lv_obj_t *sliderCont = lv_obj_create(obj);
    lv_obj_remove_style_all(sliderCont);
    lv_obj_set_size(sliderCont, lv_pct(20), lv_pct(40));
    lv_obj_clear_flag(sliderCont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_opa(sliderCont, LV_OPA_80, 0);
    lv_obj_set_style_bg_color(sliderCont, lv_color_hex(0xeeeeee), 0);
    lv_obj_align(sliderCont, LV_ALIGN_TOP_RIGHT, -20, 40);
    lv_obj_set_style_radius(sliderCont, 10, LV_PART_MAIN);

    ui.sliderCont.cont = sliderCont;

    ui.sliderCont.slider = sliderCreate(sliderCont, nullptr, 0, 0);
    lv_obj_set_style_bg_opa(ui.sliderCont.slider, LV_OPA_TRANSP, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui.sliderCont.slider, LV_OPA_TRANSP, LV_PART_KNOB | LV_STATE_PRESSED);
    lv_obj_set_style_border_width(ui.sliderCont.slider, 0, LV_PART_KNOB);
    lv_obj_set_size(ui.sliderCont.slider, lv_pct(50), lv_pct(80));

    int val = 30;
    int max = 100;
    if (_opts.getVolumeCb != nullptr) // 获取当前音量和最大值
        _opts.getVolumeCb(&val, &max);

    lv_slider_set_range(ui.sliderCont.slider, 0, max);
    lv_slider_set_value(ui.sliderCont.slider, val, LV_ANIM_OFF);
}

void View::listContCreate(lv_obj_t *obj)
{
    lv_obj_t *cont = lv_obj_create(obj);
    lv_obj_remove_style_all(cont);
    lv_obj_set_size(cont, lv_pct(45), lv_pct(50));
    lv_obj_set_style_bg_opa(cont, LV_OPA_80, LV_PART_MAIN);
    lv_obj_set_style_bg_color(cont, lv_color_hex(0x9cd1bb), LV_PART_MAIN);

    lv_obj_align(cont, LV_ALIGN_LEFT_MID, 20, 0);
    lv_obj_set_style_radius(cont, 16, LV_PART_MAIN);

    lv_obj_set_style_pad_all(cont, 25, LV_PART_MAIN); // 设置每一个item的宽度
    lv_obj_set_style_pad_row(cont, 20, LV_PART_MAIN); // 设置每一个item的间距

    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);       // 设置弹性布局，item竖着排
    lv_obj_set_scroll_dir(cont, LV_DIR_VER);               // 设置画布滚动方向：垂直滚动
    lv_obj_set_scroll_snap_y(cont, LV_SCROLL_SNAP_NONE); // 设置在垂直滚动结束时捕捉子元素的位置：人话：打开菜单第一个item的位置
    lv_obj_set_style_clip_corner(cont, true, LV_PART_MAIN);

    lv_obj_set_scrollbar_mode(cont, LV_SCROLLBAR_MODE_AUTO);
    lv_obj_set_style_bg_color(cont, lv_color_hex(0xffffff), LV_PART_SCROLLBAR);
    lv_obj_set_style_bg_opa(cont, LV_OPA_50, LV_PART_SCROLLBAR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cont, LV_OPA_COVER, LV_PART_SCROLLBAR | LV_STATE_SCROLLED);
    lv_obj_set_style_width(cont, 5, LV_PART_SCROLLBAR);
    lv_obj_set_style_radius(cont, 255, LV_PART_SCROLLBAR);
    ui.listCont.cont = cont;
}

void View::rollerContCreate(lv_obj_t *obj)
{
    // 创建歌词滚筒
    lv_obj_t *roller = lv_roller_create(obj);
    lv_obj_set_size(roller, lv_pct(45), lv_pct(50));
    lv_obj_set_style_radius(roller, 16, LV_PART_MAIN);
    lv_obj_align(roller, LV_ALIGN_RIGHT_MID, -20, 0);                          // 设置对齐
    lv_obj_clear_flag(roller, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE); // 不可滚动不可点击
    lv_obj_set_style_bg_color(roller, lv_color_hex(0x6a8d6d), 0);
    lv_obj_set_style_bg_opa(roller, LV_OPA_60, LV_STATE_DEFAULT);                                     // 背景透明
    lv_obj_set_style_border_opa(roller, LV_OPA_60, LV_STATE_DEFAULT);                                 // 边框透明
    lv_obj_set_style_bg_opa(roller, LV_OPA_60, LV_PART_SELECTED | LV_STATE_DEFAULT);                  // 选中项背景透明
    lv_obj_set_style_text_font(roller, ui.fontCont.font16.font, LV_STATE_DEFAULT);                    // 非选中项字体
    lv_obj_set_style_text_font(roller, ui.fontCont.font20.font, LV_PART_SELECTED | LV_STATE_DEFAULT); // 选中项字体
    lv_obj_set_style_text_color(roller, lv_color_hex(0xffffff), LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(roller, lv_color_hex(0xe9de9e), LV_PART_SELECTED | LV_STATE_DEFAULT); // 字体颜色
    lv_roller_set_options(roller, "begin~", LV_ROLLER_MODE_NORMAL);
    // lv_roller_set_visible_row_count(roller, LYRIC_SHOW_LINES); // 可见行数6行

    ui.lyricRoller = roller;
}

void View::topContCreate(lv_obj_t *obj)
{
    lv_obj_t *cont = lv_obj_create(obj);
    lv_obj_remove_style_all(cont);
    lv_obj_set_size(cont, lv_pct(90), lv_pct(8));
    lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_opa(cont, LV_OPA_90, 0);
    lv_obj_set_style_bg_color(cont, lv_color_hex(0xeeeeee), 0);
    lv_obj_align(cont, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_radius(cont, 5, LV_PART_MAIN);
    ui.topCont.cont = cont;

    lv_obj_t *btn = btnCreate(cont, nullptr, 0, 0, 30, 30);
    lv_obj_align(btn, LV_ALIGN_TOP_RIGHT, -5, 4);
    lv_obj_set_style_bg_color(btn, lv_color_hex(0xff6056), 0);                // 设置按钮默认的颜色
    lv_obj_set_style_bg_color(btn, lv_color_hex(0xe44543), LV_STATE_PRESSED); // 设置按钮在被按下时的颜色
    lv_obj_set_style_bg_color(btn, lv_color_hex(0xe44543), LV_STATE_FOCUSED); // 设置按钮在被按下时的颜色
    ui.topCont.cancelBtn = btn;
    lv_obj_t *cancelBtnLabel = lv_label_create(ui.topCont.cancelBtn);
    lv_obj_remove_style_all(cancelBtnLabel);
    lv_obj_set_style_text_font(cancelBtnLabel, ui.fontCont.font20.font, LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cancelBtnLabel, lv_color_hex(0xffffff), 0);
    lv_obj_center(cancelBtnLabel);
    lv_label_set_text_fmt(cancelBtnLabel, "%s", "x");

    lv_obj_t *musicNameLabel = lv_label_create(cont);
    lv_obj_remove_style_all(musicNameLabel);
    lv_obj_set_style_text_font(musicNameLabel, ui.fontCont.font20.font, LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(musicNameLabel, lv_color_black(), 0);
    lv_label_set_long_mode(musicNameLabel, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_align(musicNameLabel, LV_ALIGN_CENTER, 0, 0);
    lv_label_set_text_fmt(musicNameLabel, "%s", "musicName"); // 空格是为了居中

    ui.topCont.musicNameLabel = musicNameLabel;
}

/**
 * @brief 添加一个音乐到列表
 * @param name 音乐文件名称
 */
void View::addMusicList(const char *name)
{
    // 添加一个按钮到list
    lv_obj_t *obj = listCreate(name, nullptr);

    if (_playingMusicBtn == nullptr)
    {
        // 保存地一首歌
        _playingMusicBtn = obj;
    }

    int len = strlen(name) + 1;
    char *musicName = new char[len];
    strcpy(musicName, name);

    lv_obj_set_user_data(obj, musicName);

    lv_obj_add_event_cb(obj, listBtnEventHandler, LV_EVENT_SHORT_CLICKED, this);
}

/**
 * @brief 加载歌词到UI
 * @param lyric 使用\n隔开的所有歌词
 */
void View::loadLyric(const char *lyric)
{
    if (lyric != nullptr)
        lv_roller_set_options(ui.lyricRoller, lyric, LV_ROLLER_MODE_NORMAL);
    else
        lv_roller_set_options(ui.lyricRoller, "did not find lyric", LV_ROLLER_MODE_NORMAL);
}

/**
 * @brief 设置当前歌词
 * @param id 歌词id
 */
void View::setLyricId(int id, bool isAnim)
{
    if (id < 0)
        return;

    lv_anim_enable_t anim = id && isAnim ? LV_ANIM_ON : LV_ANIM_OFF;
    lv_roller_set_selected(ui.lyricRoller, id, anim);
}

/**
 * @brief 根据索引获取对应的音乐文件名
 * @param index 索引
 */
const char *View::getMusicName(int index)
{
    const char *name = nullptr;

    lv_obj_t *btn = lv_obj_get_child(ui.listCont.cont, index);
    if (btn != nullptr)
    {
        _playingMusicBtn = btn;
        name = (const char *)lv_obj_get_user_data(btn);
    }

    return name;
}

/**
 * @brief 在底部显示音乐名称
 * @param name 音乐名
 */
void View::showMusicName(const char *name)
{
    if (name != nullptr)
    {
        lv_label_set_text(ui.topCont.musicNameLabel, name);
        printf("songName:%s\n", name);
    }
}

lv_obj_t *View::sliderCreate(lv_obj_t *par, const void *img_src, lv_coord_t x_ofs, lv_coord_t y_ofs, int32_t min, int32_t max, int32_t val)
{
    lv_obj_t *obj = lv_slider_create(par);
    lv_obj_remove_style_all(obj);
    lv_slider_set_mode(obj, LV_SLIDER_MODE_NORMAL);
    lv_slider_set_range(obj, min, max);
    lv_slider_set_value(obj, val, LV_ANIM_OFF);

    lv_obj_set_size(obj, lv_pct(78), lv_pct(20));

    lv_obj_set_style_border_width(obj, 3, LV_PART_KNOB);
    lv_obj_set_style_border_color(obj, lv_color_hex(0xbbbbbb), LV_PART_KNOB);
    lv_obj_set_style_pad_all(obj, 1, LV_PART_KNOB);
    lv_obj_set_style_radius(obj, 10, LV_PART_KNOB);
    lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(obj, LV_OPA_60, LV_PART_KNOB | LV_STATE_PRESSED);
    lv_obj_set_style_bg_color(obj, lv_color_hex(0x445588), LV_PART_KNOB);

    lv_obj_set_style_radius(obj, 8, LV_PART_MAIN);
    lv_obj_set_style_bg_color(obj, lv_color_hex(0x3c9ba6), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, LV_PART_MAIN);

    lv_obj_set_style_radius(obj, 8, LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(obj, lv_color_hex(0xa4d9b2), LV_PART_INDICATOR);
    lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, LV_PART_INDICATOR);
    lv_obj_align(obj, LV_ALIGN_CENTER, x_ofs, y_ofs);

    if (img_src != nullptr)
    {
        lv_obj_t *img = lv_img_create(obj);
        lv_obj_align(img, LV_ALIGN_LEFT_MID, 10, 0);
        lv_img_set_src(img, img_src);
        // lv_obj_set_style_img_opa(img, LV_OPA_50, LV_PART_MAIN);
        // lv_obj_set_style_img_recolor_opa(img, LV_OPA_COVER, LV_PART_MAIN);
        // lv_obj_set_style_img_recolor(img, lv_color_white(), LV_PART_MAIN);
    }

    return obj;
}

lv_obj_t *View::btnCreate(lv_obj_t *par, const void *img_src, lv_coord_t x_ofs, lv_coord_t y_ofs, lv_coord_t w, lv_coord_t h)
{
    lv_obj_t *obj = lv_obj_create(par);
    lv_obj_remove_style_all(obj);
    lv_obj_set_size(obj, w, h);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_align(obj, LV_ALIGN_LEFT_MID, x_ofs, y_ofs);
    lv_obj_set_style_bg_img_src(obj, img_src, 0);

    lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, 0);
    lv_obj_set_style_width(obj, w / 1.5f, LV_STATE_PRESSED);                   // 设置button按下时的宽
    lv_obj_set_style_height(obj, h / 1.5f, LV_STATE_PRESSED);                  // 设置button按下时的长
    lv_obj_set_style_bg_color(obj, lv_color_hex(0x356b8c), 0);                 // 设置按钮默认的颜色
    lv_obj_set_style_bg_color(obj, lv_color_hex(0x242947), LV_STATE_PRESSED);  // 设置按钮在被按下时的颜色
    lv_obj_set_style_bg_color(obj, lv_color_hex(0xf2daaa), LV_STATE_FOCUSED);  // 设置按钮在被聚焦时的颜色
    lv_obj_set_style_bg_color(obj, lv_color_hex(0xa99991), LV_STATE_DISABLED); // 设置按钮失能时的颜色
    lv_obj_set_style_radius(obj, 9, 0);                                        // 按钮画圆角

    static lv_style_transition_dsc_t tran;
    static const lv_style_prop_t prop[] = {LV_STYLE_WIDTH, LV_STYLE_HEIGHT, LV_STYLE_PROP_INV};
    lv_style_transition_dsc_init(
        &tran,
        prop,
        lv_anim_path_ease_out,
        150,
        0,
        NULL);
    lv_obj_set_style_transition(obj, &tran, LV_STATE_PRESSED);
    lv_obj_set_style_transition(obj, &tran, LV_STATE_FOCUSED);

    lv_obj_update_layout(obj);

    return obj;
}

lv_obj_t *View::listCreate(const char *name, const void *img_src)
{
    // lv_obj_t *obj = lv_list_add_btn(ui.listCont.cont, img_src, name);
    // lv_obj_t *obj = lv_list_add_btn(ui.listCont.cont, LV_SYMBOL_PLAY, name);

    lv_obj_t *obj = lv_obj_create(ui.listCont.cont);
    lv_obj_remove_style_all(obj);
    lv_obj_set_size(obj, LV_PCT(98), LV_PCT(24));
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_align(obj, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_opa(obj, LV_OPA_60, LV_STATE_DEFAULT); // 设置背景透明
    lv_obj_set_style_bg_opa(obj, LV_OPA_80, LV_STATE_PRESSED); // 设置背景透明度(按下时)
    lv_obj_set_style_width(obj, lv_pct(95), LV_STATE_PRESSED); // 设置button按下时的长宽
    lv_obj_set_style_height(obj, lv_pct(21), LV_STATE_PRESSED);
    lv_obj_set_style_radius(obj, 9, 0); // 按钮画圆角

    lv_obj_set_style_shadow_width(obj, 10, 0);
    lv_obj_set_style_shadow_ofs_x(obj, 5, 0);
    lv_obj_set_style_shadow_ofs_y(obj, 5, 0);
    lv_obj_set_style_shadow_color(obj, lv_color_hex(0xd6dff5), 0);

    lv_obj_t *img = lv_img_create(obj);
    lv_img_set_src(img, LV_SYMBOL_VIDEO);
    lv_obj_align(img, LV_ALIGN_LEFT_MID, 10, 0);

    lv_obj_t *label = lv_label_create(obj);
    lv_obj_set_size(label, lv_pct(80), LV_SIZE_CONTENT);
    // lv_obj_set_style_text_font(label, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_font(label, ui.fontCont.font20.font, 0);
    lv_label_set_text(label, name);
    lv_label_set_long_mode(label, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_align_to(label, img, LV_ALIGN_OUT_RIGHT_MID, 20, 0);

    // anim
    static lv_style_transition_dsc_t tran;
    static const lv_style_prop_t prop[] = {LV_STYLE_WIDTH, LV_STYLE_HEIGHT, LV_STYLE_PROP_INV};
    lv_style_transition_dsc_init(
        &tran,
        prop,
        lv_anim_path_ease_out,
        150,
        0,
        nullptr);
    lv_obj_set_style_transition(obj, &tran, LV_STATE_PRESSED);
    lv_obj_update_layout(obj);

    return obj;
}

lv_obj_t *View::roundRectCreate(lv_obj_t *par, lv_coord_t x_ofs, lv_coord_t y_ofs)
{
    /* Render octagon explode */
    lv_obj_t *roundRect = lv_obj_create(par);
    lv_obj_remove_style_all(roundRect);
    lv_obj_set_size(roundRect, 10, 10);
    lv_obj_set_style_radius(roundRect, 2, 0);

    lv_obj_set_style_shadow_width(roundRect, 10, 0);
    lv_obj_set_style_shadow_ofs_x(roundRect, 1, 0);
    lv_obj_set_style_shadow_ofs_y(roundRect, 1, 0);
    lv_obj_set_style_shadow_color(roundRect, lv_color_hex(0x5d8c3d), 0);
    lv_obj_set_style_shadow_spread(roundRect, 1, 0);
    lv_obj_set_style_shadow_opa(roundRect, LV_OPA_TRANSP, 0);

    lv_obj_set_style_bg_color(roundRect, lv_color_hex(0x88d35e), 0);
    lv_obj_set_style_bg_opa(roundRect, LV_OPA_TRANSP, 0);
    lv_obj_align(roundRect, LV_ALIGN_LEFT_MID, x_ofs, y_ofs);

    return roundRect;
}

/**
 * @brief 设置音乐播放进度显示
 */
void View::setPlayProgress(int cur, int total)
{
    lv_slider_set_range(ui.btnCont.slider, 0, total);
    if (!lv_obj_has_state(ui.btnCont.slider, LV_STATE_PRESSED)) // 未按下时设置
        lv_slider_set_value(ui.btnCont.slider, cur, LV_ANIM_OFF);

    lv_label_set_text_fmt(ui.btnCont.timeLabel, "%d:%d/%d:%d", cur / 60, cur % 60, total / 60, total % 60);
}

void View::buttonEventHandler(lv_event_t *event)
{
    View *instance = (View *)lv_event_get_user_data(event);
    LV_ASSERT_NULL(instance);

    lv_event_code_t code = lv_event_get_code(event);
    lv_obj_t *obj = lv_event_get_current_target(event);

    if (code == LV_EVENT_SHORT_CLICKED)
    {
        if (obj == instance->ui.btnCont.btn)
        {
            instance->appearAnimClick();

            if (instance->_isPlaying == true)
            {
                if (instance->_opts.pauseCb != nullptr)
                {
                    instance->_isPlaying = false;
                    instance->_opts.pauseCb();
                    lv_obj_set_style_bg_img_src(obj, LV_SYMBOL_PLAY, 0);
                    printf("[View] pause!\n");
                }
            }
            else
            {
                if (instance->_opts.playCb != nullptr)
                {

                    int index = lv_obj_get_index(instance->_playingMusicBtn);

                    instance->_isPlaying = true;
                    instance->_opts.playCb(nullptr, index);
                    lv_obj_set_style_bg_img_src(obj, LV_SYMBOL_PAUSE, 0);
                    printf("[View] play!\n");
                }
            }
        }

        if (obj == instance->ui.funcCont.prevBtn)
        {
            int index = lv_obj_get_index(instance->_playingMusicBtn); // 获取当前musicBtn索引
            int total = lv_obj_get_child_cnt(instance->ui.listCont.cont) - 1;
            index = index < total ? index - 1 : total - 1; // 设置上一个btn索引

            lv_obj_t *nextBtn = lv_obj_get_child(instance->ui.listCont.cont, index); // 获取上一个musicListBtn
            if (nextBtn != nullptr)
            {
                instance->_isPlaying = true;

                instance->_playingMusicBtn = nextBtn;
                const char *musicName = (const char *)lv_obj_get_user_data(nextBtn);
                if (instance->_opts.playCb != nullptr && musicName != nullptr)
                    instance->_opts.playCb(musicName, index);

                lv_obj_set_style_bg_img_src(instance->ui.btnCont.btn, LV_SYMBOL_PAUSE, 0);
                printf("[View] prev!\n");
            }
        }
        else if (obj == instance->ui.funcCont.nextBtn)
        {
            int index = lv_obj_get_index(instance->_playingMusicBtn); // 获取当前musicBtn索引
            int total = lv_obj_get_child_cnt(instance->ui.listCont.cont) - 1;
            index = index < total ? index + 1 : 0; // 设置下一个btn索引

            lv_obj_t *nextBtn = lv_obj_get_child(instance->ui.listCont.cont, index); // 获取下一个musicListBtn
            if (nextBtn != nullptr)
            {
                instance->_isPlaying = true;

                instance->_playingMusicBtn = nextBtn;
                const char *musicName = (const char *)lv_obj_get_user_data(nextBtn);
                if (instance->_opts.playCb != nullptr && musicName != nullptr)
                    instance->_opts.playCb(musicName, index);

                lv_obj_set_style_bg_img_src(instance->ui.btnCont.btn, LV_SYMBOL_PAUSE, 0);
                printf("[View] next!\n");
            }
        }
        else if (obj == instance->ui.funcCont.funcBtn)
        {
            if (instance->_playMode != PlayMode_Random)
                instance->_playMode = (PlayMode)(instance->_playMode + (PlayMode)1);
            else
                instance->_playMode = PlayMode_ListLoop;

            if (instance->_opts.setModeCb != nullptr)
            {
                instance->_opts.setModeCb(instance->_playMode);

                switch (instance->_playMode)
                {
                case PlayMode_ListLoop:
                    lv_obj_set_style_bg_img_src(obj, LV_SYMBOL_LOOP, 0);

                    break;
                case PlayMode_SingleLoop:
                    lv_obj_set_style_bg_img_src(obj, LV_SYMBOL_LIST, 0);

                    break;
                case PlayMode_Random:
                    lv_obj_set_style_bg_img_src(obj, LV_SYMBOL_SHUFFLE, 0);

                    break;
                default:
                    break;
                }

                printf("[View] setMode!\n");
            }
        }
        else if (obj == instance->ui.topCont.cancelBtn)
        {
            if (instance->_opts.exitCb !=nullptr)
                instance->_opts.exitCb();
        }
    }
}

void View::sliderEventHandler(lv_event_t *event)
{
    View *instance = (View *)lv_event_get_user_data(event);
    LV_ASSERT_NULL(instance);

    lv_obj_t *obj = lv_event_get_current_target(event);
    lv_event_code_t code = lv_event_get_code(event);

    if (code == LV_EVENT_VALUE_CHANGED)
    {
        // lv_indev_wait_release(lv_indev_get_act());
    }
    if (code == LV_EVENT_RELEASED)
    {
        if (obj == instance->ui.btnCont.slider)
        {
            int cur = lv_slider_get_value(obj);
            if (instance->_opts.setCurCb != nullptr)
                instance->_opts.setCurCb(cur);
        }

        if (obj == instance->ui.sliderCont.slider)
        {
            int val = lv_slider_get_value(obj);
            if (instance->_opts.setVolumeCb != nullptr)
                instance->_opts.setVolumeCb(val);
        }
    }
}

void View::listBtnEventHandler(lv_event_t *event)
{
    View *instance = (View *)lv_event_get_user_data(event);
    LV_ASSERT_NULL(instance);

    lv_event_code_t code = lv_event_get_code(event);
    lv_obj_t *obj = lv_event_get_current_target(event);
    const char *musicName = (const char *)lv_obj_get_user_data(obj);

    printf("[View] Cb musicName:%s\n", musicName);

    if (code == LV_EVENT_SHORT_CLICKED)
    {
        instance->_isPlaying = true;
        instance->_playingMusicBtn = obj;
        int index = lv_obj_get_index(obj);

        if (instance->_opts.playCb != nullptr)
            instance->_opts.playCb(musicName, index);

        lv_obj_set_style_bg_img_src(instance->ui.btnCont.btn, LV_SYMBOL_PAUSE, 0);
    }
}

void View::onEvent(lv_event_t *event)
{
    View *instance = (View *)lv_event_get_user_data(event);
    LV_ASSERT_NULL(instance);

    lv_obj_t *obj = lv_event_get_current_target(event);
    lv_event_code_t code = lv_event_get_code(event);

    if (code == LV_EVENT_GESTURE)
    {
        switch (lv_indev_get_gesture_dir(lv_indev_get_act()))
        {
        case LV_DIR_LEFT:
            printf("[View] LV_DIR_LEFT!\n");
            instance->appearAnimVolume(true);

            break;
        case LV_DIR_RIGHT:
            printf("[View] LV_DIR_RIGHT!\n");
            instance->appearAnimVolume(false);

            break;
        case LV_DIR_TOP:
            printf("[View] LV_DIR_TOP!\n");

            break;
        case LV_DIR_BOTTOM:
            printf("[View] LV_DIR_BOTTOM!\n");
            // instance->_opts.exitCb();
            break;

        default:
            break;
        }
    }
}