#include "VoiceManager.h"



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

ssize_t CVoiceManager::SNDWAV_P_SaveRead(int fd, void *buf, size_t count)
{
	ssize_t result = 0, res;

	while (count > 0) {
		if ((res = read(fd, buf, count)) == 0)
			break;
		if (res < 0)
			return result > 0 ? result : res;
		count -= res;
		result += res;
		buf = (char *)buf + res;
	}
	return result;
}



void CVoiceManager::SNDWAV_Play(SNDPCMContainer_t *sndpcm, WAVContainer_t *wav, int fd)
{
	int load, ret;
	off64_t written = 0;
	off64_t c;
	off64_t count = LE_INT(wav->chunk.length);

	load = 0;
	while (written < count) {
		/* Must read [chunk_bytes] bytes data enough. */
		do {
			c = count - written;
			if (c > sndpcm->chunk_bytes)
				c = sndpcm->chunk_bytes;
			c -= load;

			if (c == 0)
				break;
			ret = SNDWAV_P_SaveRead(fd, sndpcm->data_buf + load, c);
			if (ret < 0) {
				fprintf(stderr, "Error safe_read/n");
				exit(-1);
			}
			if (ret == 0)
				break;
			load += ret;
		} while ((size_t)load < sndpcm->chunk_bytes);

		/* Transfer to size frame */
		load = load * 8 / sndpcm->bits_per_frame;
		ret = SNDWAV_WritePcm(sndpcm, load);
		if (ret != load)
			break;

		ret = ret * sndpcm->bits_per_frame / 8;
		written += ret;
		load = 0;
	}
}


int CVoiceManager::SNDWAV_P_GetFormat(WAVContainer_t *wav, snd_pcm_format_t *snd_format)
{
	if (LE_SHORT(wav->format.format) != WAV_FMT_PCM)
		return -1;

	switch (LE_SHORT(wav->format.sample_length)) {
	case 16:
		*snd_format = SND_PCM_FORMAT_S16_LE;
		break;
	case 8:
		*snd_format = SND_PCM_FORMAT_U8;
		break;
	default:
		*snd_format = SND_PCM_FORMAT_UNKNOWN;
		break;
	}

	return 0;
}

ssize_t CVoiceManager::SNDWAV_ReadPcm(SNDPCMContainer_t *sndpcm, size_t rcount)
{
	ssize_t r;
	size_t result = 0;
	size_t count = rcount;
	uint8_t *data = sndpcm->data_buf;

	if (count != sndpcm->chunk_size) {
		count = sndpcm->chunk_size;
	}

	while (count > 0) {
		r = snd_pcm_readi(sndpcm->handle, data, count);

		if (r == -EAGAIN || (r >= 0 && (size_t)r < count)) {
			snd_pcm_wait(sndpcm->handle, 1000);
		}
		else if (r == -EPIPE) {
			snd_pcm_prepare(sndpcm->handle);
			fprintf(stderr, "<<<<<<<<<<<<<<< Buffer Underrun >>>>>>>>>>>>>>>/n");
		}
		else if (r == -ESTRPIPE) {
			fprintf(stderr, "<<<<<<<<<<<<<<< Need suspend >>>>>>>>>>>>>>>/n");
		}
		else if (r < 0) {
			fprintf(stderr, "Error snd_pcm_writei: [%s]", snd_strerror(r));
			exit(-1);
		}

		if (r > 0) {
			result += r;
			count -= r;
			data += r * sndpcm->bits_per_frame / 8;
		}
	}
	return rcount;
}

ssize_t CVoiceManager::SNDWAV_WritePcm(SNDPCMContainer_t *sndpcm, size_t wcount)
{
	ssize_t r;
	ssize_t result = 0;
	uint8_t *data = sndpcm->data_buf;

	if (wcount < sndpcm->chunk_size) {
		snd_pcm_format_set_silence(sndpcm->format,
			data + wcount * sndpcm->bits_per_frame / 8,
			(sndpcm->chunk_size - wcount) * sndpcm->channels);
		wcount = sndpcm->chunk_size;
	}
	while (wcount > 0) {
		r = snd_pcm_writei(sndpcm->handle, data, wcount);
		if (r == -EAGAIN || (r >= 0 && (size_t)r < wcount)) {
			snd_pcm_wait(sndpcm->handle, 1000);
		}
		else if (r == -EPIPE) {
			snd_pcm_prepare(sndpcm->handle);
			fprintf(stderr, "<<<<<<<<<<<<<<< Buffer Underrun >>>>>>>>>>>>>>>/n");
		}
		else if (r == -ESTRPIPE) {
			fprintf(stderr, "<<<<<<<<<<<<<<< Need suspend >>>>>>>>>>>>>>>/n");
		}
		else if (r < 0) {
			fprintf(stderr, "Error snd_pcm_writei: [%s]", snd_strerror(r));
			exit(-1);
		}
		if (r > 0) {
			result += r;
			wcount -= r;
			data += r * sndpcm->bits_per_frame / 8;
		}
	}
	return result;
}


int CVoiceManager::SNDWAV_SetParams(SNDPCMContainer_t *sndpcm, WAVContainer_t *wav)
{
	snd_pcm_hw_params_t *hwparams;
	snd_pcm_format_t format;
	uint32_t exact_rate;
	uint32_t buffer_time, period_time;

	/* Allocate the snd_pcm_hw_params_t structure on the stack. */
	snd_pcm_hw_params_alloca(&hwparams);

	/* Init hwparams with full configuration space */
	if (snd_pcm_hw_params_any(sndpcm->handle, hwparams) < 0) {
		fprintf(stderr, "Error snd_pcm_hw_params_any/n");
		goto ERR_SET_PARAMS;
	}

	if (snd_pcm_hw_params_set_access(sndpcm->handle, hwparams, SND_PCM_ACCESS_RW_INTERLEAVED) < 0) {
		fprintf(stderr, "Error snd_pcm_hw_params_set_access/n");
		goto ERR_SET_PARAMS;
	}

	/* Set sample format */
	if (SNDWAV_P_GetFormat(wav, &format) < 0) {
		fprintf(stderr, "Error get_snd_pcm_format/n");
		goto ERR_SET_PARAMS;
	}
	if (snd_pcm_hw_params_set_format(sndpcm->handle, hwparams, format) < 0) {
		fprintf(stderr, "Error snd_pcm_hw_params_set_format/n");
		goto ERR_SET_PARAMS;
	}
	sndpcm->format = format;

	/* Set number of channels */
	if (snd_pcm_hw_params_set_channels(sndpcm->handle, hwparams, LE_SHORT(wav->format.channels)) < 0) {
		fprintf(stderr, "Error snd_pcm_hw_params_set_channels/n");
		goto ERR_SET_PARAMS;
	}
	sndpcm->channels = LE_SHORT(wav->format.channels);

	/* Set sample rate. If the exact rate is not supported */
	/* by the hardware, use nearest possible rate.         */
	exact_rate = LE_INT(wav->format.sample_rate);
	if (snd_pcm_hw_params_set_rate_near(sndpcm->handle, hwparams, &exact_rate, 0) < 0) {
		fprintf(stderr, "Error snd_pcm_hw_params_set_rate_near/n");
		goto ERR_SET_PARAMS;
	}
	if (LE_INT(wav->format.sample_rate) != exact_rate) {
		fprintf(stderr, "The rate %d Hz is not supported by your hardware./n ==> Using %d Hz instead./n",
			LE_INT(wav->format.sample_rate), exact_rate);
	}

	if (snd_pcm_hw_params_get_buffer_time_max(hwparams, &buffer_time, 0) < 0) {
		fprintf(stderr, "Error snd_pcm_hw_params_get_buffer_time_max/n");
		goto ERR_SET_PARAMS;
	}
	if (buffer_time > 500000) buffer_time = 500000;
	period_time = buffer_time / 4;

	if (snd_pcm_hw_params_set_buffer_time_near(sndpcm->handle, hwparams, &buffer_time, 0) < 0) {
		fprintf(stderr, "Error snd_pcm_hw_params_set_buffer_time_near/n");
		goto ERR_SET_PARAMS;
	}

	if (snd_pcm_hw_params_set_period_time_near(sndpcm->handle, hwparams, &period_time, 0) < 0) {
		fprintf(stderr, "Error snd_pcm_hw_params_set_period_time_near/n");
		goto ERR_SET_PARAMS;
	}

	/* Set hw params */
	if (snd_pcm_hw_params(sndpcm->handle, hwparams) < 0) {
		fprintf(stderr, "Error snd_pcm_hw_params(handle, params)/n");
		goto ERR_SET_PARAMS;
	}

	snd_pcm_hw_params_get_period_size(hwparams, &sndpcm->chunk_size, 0);
	snd_pcm_hw_params_get_buffer_size(hwparams, &sndpcm->buffer_size);
	if (sndpcm->chunk_size == sndpcm->buffer_size) {
		fprintf(stderr, ("Can't use period equal to buffer size (%lu == %lu)/n"), sndpcm->chunk_size, sndpcm->buffer_size);
		goto ERR_SET_PARAMS;
	}

	sndpcm->bits_per_sample = snd_pcm_format_physical_width(format);
	sndpcm->bits_per_frame = sndpcm->bits_per_sample * LE_SHORT(wav->format.channels);

	sndpcm->chunk_bytes = sndpcm->chunk_size * sndpcm->bits_per_frame / 8;

	/* Allocate audio data buffer */
	sndpcm->data_buf = (uint8_t *)malloc(sndpcm->chunk_bytes);
	if (!sndpcm->data_buf) {
		fprintf(stderr, "Error malloc: [data_buf]/n");
		goto ERR_SET_PARAMS;
	}

	return 0;

ERR_SET_PARAMS:
	return -1;
}

char *CVoiceManager::WAV_P_FmtString(uint16_t fmt)
{
	switch (fmt) {
	case WAV_FMT_PCM:
		return "PCM";
		break;
	case WAV_FMT_IEEE_FLOAT:
		return "IEEE FLOAT";
		break;
	case WAV_FMT_DOLBY_AC3_SPDIF:
		return "DOLBY AC3 SPDIF";
		break;
	case WAV_FMT_EXTENSIBLE:
		return "EXTENSIBLE";
		break;
	default:
		break;
	}

	return "NON Support Fmt";
}

void CVoiceManager::WAV_P_PrintHeader(WAVContainer_t *container)
{
	printf("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++/n");
	printf("/n");

	printf("File Magic:         [%c%c%c%c]/n",
		(char)(container->header.magic),
		(char)(container->header.magic >> 8),
		(char)(container->header.magic >> 16),
		(char)(container->header.magic >> 24));
	printf("File Length:        [%d]/n", container->header.length);
	printf("File Type:          [%c%c%c%c]/n",
		(char)(container->header.type),
		(char)(container->header.type >> 8),
		(char)(container->header.type >> 16),
		(char)(container->header.type >> 24));

	printf("/n");

	printf("Fmt Magic:          [%c%c%c%c]/n",
		(char)(container->format.magic),
		(char)(container->format.magic >> 8),
		(char)(container->format.magic >> 16),
		(char)(container->format.magic >> 24));
	printf("Fmt Size:           [%d]/n", container->format.fmt_size);
	printf("Fmt Format:         [%s]/n", WAV_P_FmtString(container->format.format));
	printf("Fmt Channels:       [%d]/n", container->format.channels);
	printf("Fmt Sample_rate:    [%d](HZ)/n", container->format.sample_rate);
	printf("Fmt Bytes_p_second: [%d]/n", container->format.bytes_p_second);
	printf("Fmt Blocks_align:   [%d]/n", container->format.blocks_align);
	printf("Fmt Sample_length:  [%d]/n", container->format.sample_length);

	printf("/n");

	printf("Chunk Type:         [%c%c%c%c]/n",
		(char)(container->chunk.type),
		(char)(container->chunk.type >> 8),
		(char)(container->chunk.type >> 16),
		(char)(container->chunk.type >> 24));
	printf("Chunk Length:       [%d]/n", container->chunk.length);

	printf("/n");
	printf("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++/n");
}

int WAV_P_CheckValid(WAVContainer_t *container)
{
	if (container->header.magic != WAV_RIFF ||
		container->header.type != WAV_WAVE ||
		container->format.magic != WAV_FMT ||
		container->format.fmt_size != LE_INT(16) ||
		(container->format.channels != LE_SHORT(1) && container->format.channels != LE_SHORT(2)) ||
		container->chunk.type != WAV_DATA) {

		fprintf(stderr, "non standard wav file./n");
		return -1;
	}

	return 0;
}

int CVoiceManager::WAV_ReadHeader(int fd, WAVContainer_t *container)
{
	assert((fd >= 0) && container);

	if (read(fd, &container->header, sizeof(container->header)) != sizeof(container->header) ||
		read(fd, &container->format, sizeof(container->format)) != sizeof(container->format) ||
		read(fd, &container->chunk, sizeof(container->chunk)) != sizeof(container->chunk)) {

		fprintf(stderr, "Error WAV_ReadHeader/n");
		return -1;
	}

	if (WAV_P_CheckValid(container) < 0)
		return -1;

#ifdef WAV_PRINT_MSG  
	WAV_P_PrintHeader(container);
#endif  

	return 0;
}

int CVoiceManager::WAV_WriteHeader(int fd, WAVContainer_t *container)
{
	assert((fd >= 0) && container);

	if (WAV_P_CheckValid(container) < 0)
		return -1;

	if (write(fd, &container->header, sizeof(container->header)) != sizeof(container->header) ||
		write(fd, &container->format, sizeof(container->format)) != sizeof(container->format) ||
		write(fd, &container->chunk, sizeof(container->chunk)) != sizeof(container->chunk)) {

		fprintf(stderr, "Error WAV_WriteHeader/n");
		return -1;
	}

#ifdef WAV_PRINT_MSG  
	WAV_P_PrintHeader(container);
#endif  

	return 0;
}
