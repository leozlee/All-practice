#ifndef _VOICEMANAGER_H__
#define _VOICEMANAGER_H__

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <stdio.h>
#include <linux/soundcard.h>


#define VOICE_FILE_PATH "/home/opt/data/voice"


enum VoiceSignalId
{
	NO_SIGNAL			= 0,	//无语音信号
	TURN_ON_DEVICE,				//开机语音提示
	WARNING_DANGER,				//报警：danger
	WARNING_DILONG_ONCE,		//报警：长滴声1声
	WARNING_DILONG_3TIMES,		//报警：长滴声3声
	WARNING_DISHORT,			//报警：短滴声1声
	WARNING_DIHIT,				//报警：超短滴1声
	WARNING_OVERSPEED,			//超速语音报警
};

class CVoiceManager
{
public:

	CVoiceManager();
	~CVoiceManager();
	static CVoiceManager* GetInstance();
	bool HasInstance();
	void Destory();
	int TestSound(char *filename);
	bool AudioPlay(const char *path,int nSampleRate,int nChannels,int fmt);
	bool VoiceInit();

	
private:
	int mFd;
	static CVoiceManager* mInstance;



};





#endif


