

#define MP3_CTRL_FLAG_PLAY      0x1
#define MP3_CTRL_FLAG_PAUSE     0x2
#define MP3_CTRL_FLAG_STOP      0x4
#define MP3_CTRL_FLAG_CONTINUOUS  0x8

#define MP3_STATE_STOPPED       0x0
#define MP3_STATE_PLAYING       0x1
#define MP3_STATE_PAUSED        0x2

void Mp3PlaybackTask(void* pdata);
OS_FLAG_GRP* GetMP3Flags(void);
uint8_t getMp3PlayerStatus();