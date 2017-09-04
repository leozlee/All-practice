#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define VOICE_CS "/dev/voice_cs"

int main(void)
{
	int fd = 0;
	fd = open(VOICE_CS,O_RDWR);
	if(fd < 0)
	{
		perror("open fail/r/n");
	}
	else
	{
		printf("fd:%d\r\n",fd);
	}

	int ret = -1;
	char state = 1;

	state = 0;
	ret = write(fd,&state,1);
	printf("ret:%d\r\n",ret);
	if(ret < 0)
	{
		perror("write fail\r\n");
	}
	else
	{
		printf("i write 0 to pin\r\n");
	}

	close(fd);


}
