#if 0


#else 


#include "VoiceManager.h"

#define 	DangerVoiceFile		"/mnt/data/home/opt/data/voice/danger.wav"

int main(void)
{
	printf("this is a test for voice play\r\n");
	CVoiceManager::GetInstance()->TestSound((char *)DangerVoiceFile);
	return 0;
}


#endif