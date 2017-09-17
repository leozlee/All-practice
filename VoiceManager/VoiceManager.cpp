#include "VoiceManager.h"

#define Audio_Device 	"/dev/snd/pcmC0D0p"
#define Sample_Size 	16
#define Sample_Rate 	44100 //sampling rate
#define CHANNEL			2


CVoiceManager* CVoiceManager::mInstance = NULL;


CVoiceManager::CVoiceManager()
{

}

CVoiceManager::~CVoiceManager()
{

}


CVoiceManager* CVoiceManager::GetInstance()
{
	if(!mInstance)
		mInstance = new CVoiceManager();
	return mInstance;
}


bool CVoiceManager::HasInstance()
{
	if(!mInstance)
		return false;
	return true;
}

int CVoiceManager::TestSound(char *filename)
{
    struct stat stat_buf;
    char *buf = NULL;
    int handler;
    int result;
    int arg,status;

    //打开声音文件，将文件读入内存
    mFd=open(filename,O_RDONLY);
    if(mFd<0) 
     return -1;
    if(fstat(mFd,&stat_buf))
    {
        close(mFd);
        return -1;
    }

    if(!stat_buf.st_size)
    {
        close(mFd);
        return -1;
   }
   //根据文件爱你大小申请内存
   buf=new char[stat_buf.st_size];
   if(!buf)
   {
       close(mFd);
       return -1;
   }
   //将音频文件存放在buf中
   if(read(mFd,buf,stat_buf.st_size)<0)
   {
       free(buf);
       close(mFd);
       return -1;
   }

   //打开声卡设备，并设置声卡播放参数，这些参数必须与声音文件参数一致
   handler=open(Audio_Device,O_WRONLY);
   if(-1 == handler)
   {
       perror("open Audio_Device fail");
       return -1;
   } 
   arg=Sample_Rate;
   status=ioctl(handler,SOUND_PCM_WRITE_RATE,&arg);
   if(-1 == status)
   {
       perror("error from SOUND_PCM_WRITE_RATE ioctl");
       return -1;
   }

   arg=Sample_Size;
   status=ioctl(handler,SOUND_PCM_WRITE_BITS,&arg);
   if(-1 == status)
   {
       perror("error from SOUND_PCM_WRITE_BITS ioctl");
       return -1;
   }

  //向端口写入，播放
   result=write(handler,buf,stat_buf.st_size);
   if(-1 == result)
   {
       perror("Fail to play the sound!");
       return -1;
   }
   free(buf);
   close(mFd);
   close(handler);
  return result;

}