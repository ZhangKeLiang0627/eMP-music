#include "Model.h"

#define MUSIC_DIR "/mnt/UDISK/music/"
#define LYRIC_DIR "/mnt/UDISK/music/lyrics/"

using namespace Page;

/**
 * @brief Model构造函数
 *
 * @param exitCb
 * @param mutex
 */
Model::Model(std::function<void(void)> exitCb, pthread_mutex_t &mutex)
{
    _threadExitFlag = false;
    _mutex = &mutex;

    // 设置UI回调函数
    Operations uiOpts = {0};

    uiOpts.exitCb = exitCb;
    uiOpts.getStateCb = std::bind(&Model::getState, this);
    uiOpts.pauseCb = std::bind(&Model::pause, this);
    uiOpts.playCb = std::bind(&Model::play, this, std::placeholders::_1, std::placeholders::_2);
    uiOpts.setCurCb = std::bind(&Model::setCur, this, std::placeholders::_1);
    uiOpts.setVolumeCb = snd_set_volume;
    uiOpts.getVolumeCb = snd_get_volume;
    uiOpts.getModeCb = std::bind(&Model::getMode, this);
    uiOpts.setModeCb = std::bind(&Model::setMode, this, std::placeholders::_1);

    _view.create(uiOpts);

    // 这里设置一个1000ms的定时器，软定时器，用于在onTimerUpdate里update
    _timer = lv_timer_create(onTimerUpdate, 1000, this);
    // 创建执行线程，传递this指针
    pthread_create(&_pthread, NULL, threadProcHandler, this);
}

Model::~Model()
{
    _threadExitFlag = true;

    lv_timer_del(_timer);

    _view.release();
}

/**
 * @brief 定时器更新函数
 *
 */
void Model::onTimerUpdate(lv_timer_t *timer)
{
    Model *instance = (Model *)timer->user_data;

    instance->update();
}

/**
 * @brief 更新UI等事务
 *
 */
void Model::update(void)
{
    if (_musicObj != nullptr)
    {
        _view.setPlayProgress(_musicObj->getCurTime(), _musicObj->getTotalTime());
    }
}

/**
 * @brief 线程处理函数
 *
 * @return void*
 */
void *Model::threadProcHandler(void *arg)
{
    Model *model = static_cast<Model *>(arg); // 将arg转换为Model指针

    int tick = 0;

    model->_musicNum = model->searchMusic(MUSIC_DIR);

    while (!model->_threadExitFlag)
    {
        pthread_mutex_lock(model->_mutex);

        if (model->_musicLyric != nullptr)
        {
            if (model->_musicObj != nullptr)
            {
                int id = lyric_getid_by_time(model->_musicLyric, model->_musicObj->getCurTime() * 1000);
                
                // 修复歌词滚动问题
                if (model->_musicObj->getCurTime() * 1000 < 1 * 1000)
                    id = 0;

                model->_view.setLyricId(id, true);
            }
        }

        // 播放完后继续播放下一首歌
        if (model->_musicObj != nullptr && model->_musicObj->isOver() == true)
        {
            model->changeMusic();
        }

        pthread_mutex_unlock(model->_mutex);

        usleep(50000);
    }
}

/**
 * @brief 搜索某个目录下的音频文件，支持 MP3、WAV
 * @param path 目录路径
 * @return 搜索到的音乐个数
 */
int Model::searchMusic(string path)
{
    int count = 0;
    bool legalMusic = false;
    std::string filePath;

    struct dirent *ent;
    DIR *dir = opendir(path.c_str());

    for (int i = 0;; i++)
    {
        ent = readdir(dir);
        if (ent == nullptr)
            break;

        if (ent->d_type == DT_REG)
        {
            const char *pfile = strrchr(ent->d_name, '.');
            if (pfile != nullptr)
            {
                filePath = path + std::string(ent->d_name);

                if (strcasecmp(pfile, ".wav") == 0)
                {
                    printf("wav file\n");
                    legalMusic = true;
                }
                else if (strcasecmp(pfile, ".mp3") == 0)
                {
                    printf("mp3 file\n");
                    legalMusic = true;
                }
            }
        }
        if (legalMusic == true)
        {
            legalMusic = false;

            pthread_mutex_lock(_mutex);
            _view.addMusicList(ent->d_name);
            pthread_mutex_unlock(_mutex);

            count++;
        }

        // usleep(50000);
    }

    closedir(dir);

    return count;
}

/**
 * @brief 切歌
 */
void Model::changeMusic(void)
{
    int maxIndex = _musicNum - 1;

    switch (_musicMode)
    {
    case PlayMode_ListLoop:
        _musicIndex = _musicIndex < maxIndex ? _musicIndex + 1 : 0; // 顺序递增索引
        break;
    case PlayMode_SingleLoop:
        // _musicIndex = _musicIndex;                  //索引不变
        break;
    case PlayMode_Random:
        _musicIndex = rand() % (maxIndex + 1); // 随机生成索引
        break;
    }

    const char *name = _view.getMusicName(_musicIndex); // 根据索引获取音乐文件名

    play(name, _musicIndex); // 播放音乐
}

/**
 * @brief 获取当前音乐的状态
 * @return 音乐状态 0 - 暂停状态 / 1 - 播放状态 / -1 - 未播放(或播放结束)
 */
int Model::getState(void)
{
    if (_musicObj != nullptr && _musicObj->threadState != false)
    {
        return (int)_musicObj->getPlayState();
    }
    else
    {
        return -1;
    }
}

/**
 * @brief 暂停音乐播放
 */
void Model::pause(void)
{
    if (_musicObj != nullptr)
    {
        _musicObj->pause();
    }
}

/**
 * @brief 播放音乐
 * @param name 切换的音乐文件名，若为nullptr则继续播放当前音乐
 * @param index 切换的音乐索引
 */
void Model::play(const char *name, int index)
{
    if (name == nullptr) // 不需要切歌
    {
        if (_musicObj != nullptr)
            _musicObj->play();
        return;
    }

    _musicIndex = index;       // 保存播放索引
    _view.showMusicName(name); // 设置音乐名显示

    std::string filePath = MUSIC_DIR + std::string(name);
    if (_musicObj != nullptr)
        delete _musicObj; // 删除先前的播放
    _musicObj = nullptr;

    if (_musicLyric != nullptr)
        lyric_free(_musicLyric); // 删除先前的歌词
    _musicLyric = nullptr;

    _musicObj = new MusicObj(filePath);
    _musicObj->play();

    _musicLyricSentences = 0;
    filePath = LYRIC_DIR + std::string(name);
    size_t pos = filePath.find('.');
    if (pos != string::npos)
    {
        filePath.erase(pos);
        filePath += ".lrc";
        _musicLyric = lyric_load_from_file(filePath.c_str());
        if (_musicLyric != nullptr)
        {
            _musicLyricSentences = lyric_get_sentence_cnt(_musicLyric);
            const char *lyric_n = lyric_get_all(_musicLyric, '\n');
            _view.loadLyric(lyric_n); // 装载歌词到UI
            if (lyric_n != nullptr)   // UI内部拷贝，此处可以释放
                free((void *)lyric_n);
        }
        else
        {
            _view.loadLyric(nullptr); // 指示找不到歌词
        }
    }
}

/**
 * @brief 设置音乐播放时间点
 * @param cur_sec 设置的播放时间点，单位sec
 */
void Model::setCur(int cur_sec)
{
    if (_musicObj != nullptr)
        _musicObj->setCurTime(cur_sec);
}

/**
 * @brief 设置音乐播放模式
 * @param mode
 */
void Model::setMode(PlayMode mode)
{
    _musicMode = mode;
}

/**
 * @brief 获取当前音乐播放模式
 * @param mode
 */
PlayMode Model::getMode(void)
{
    return _musicMode;
}