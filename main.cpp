#include <assert.h>
#include "stdio.h"
#include "stdlib.h"
#include "FrameExtractor/FrameExtractor.h"
#include "FrameExtractor/H264Frames.h"

#ifdef __cplusplus
extern "C" {
#endif
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavfilter/avfiltergraph.h>
#include <libavfilter/avcodec.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavutil/opt.h>
#include <libavutil/pixdesc.h>
#include <libswscale/swscale.h>
#ifdef __cplusplus
};
#endif

#define INPUT_FILE "z:\\work\\tiny210\\rootfs.friendly\\h264.sps.header"
#define OUTPUT_FILE "d:\\work\\windows\\FrameExtractor_test\\slamtv60.264.new"


static unsigned char h264_delimiter[4] = {0x0, 0x0, 0x0, 0x01};

void dump_to_file(const char *path, char *buf, int len)
{
	static FILE *fp;
	if (!fp)
	{
		remove(path);
		fp = fopen(path, "wb");
	}
	fwrite(buf, len, 1, fp);

}

#define BUF_SIZE 1024*1024*8

int FrameExtractor_test()
{
	FRAMEX_CTX *pCTX;
	FILE *fp;
	FRAMEX_STRM_PTR frame_stream;
	H264_CONFIG_DATA conf_data;
	unsigned char *buf;
	int len;
	unsigned char *ptr;
	unsigned int coding_type;

	buf = (unsigned char*)malloc(BUF_SIZE);
	
	fp = fopen(INPUT_FILE, "rb");
	if (!fp)
		exit(-1);

	fseek(fp, 0, SEEK_END);
	len = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	ptr = (unsigned char*)malloc(len);
	fread(ptr, len, 1, fp);
	fseek(fp, 0, SEEK_SET);
	fclose(fp);

	frame_stream.p_cur = ptr;
	frame_stream.p_end = ptr+len;
	frame_stream.p_start = ptr;

	pCTX = FrameExtractorInit(FRAMEX_IN_TYPE_MEM, h264_delimiter, sizeof(h264_delimiter), 1);
	FrameExtractorFirst(pCTX, (void*)&frame_stream);

	len = ExtractConfigStreamH264(pCTX, (unsigned char *)&frame_stream, buf, BUF_SIZE, &conf_data);
	if (len>0)
	{
		//dump_to_file(OUTPUT_FILE, (char*)buf, len);
	}

	while((len = NextFrameH264(pCTX, (unsigned char *)&frame_stream, (unsigned char*)buf, BUF_SIZE, &coding_type)) >0)
	{
		printf("len:%d coding_type:%d\n", len, coding_type);
		//dump_to_file(OUTPUT_FILE, (char*)buf, len);
	}
	
	printf("finish...\n");
	getchar();
	return 0;
}



void ppm_save(unsigned char *buf, int wrap, int xsize, int ysize,
			  char *filename)
{
	FILE *f;
	int i;

	remove(filename);

	f=fopen(filename, "w+");
	if (f)
	{
		fprintf(f,"P6\n%d %d\n%d\n",xsize,ysize,255);
		for(i=0;i<ysize;i++)
			fwrite(buf + i * wrap,1, wrap,f);
		fclose(f);
	}
}

#define YUV420_FILE "z:\\work\\tiny210\\rootfs.friendly\\yuv420p.mfc"
int main()
{
	AVPicture picture_yuv420p;
	AVPicture picture_rgb888;
	SwsContext *pSwscontext;

	int ret, i;
	unsigned char *ptr;
	unsigned char *mem_y;
	unsigned char *mem_uv;
	unsigned char *mem_v;
	FILE *fp;

	AVPixelFormat yuv_type = AV_PIX_FMT_YUV420P;
	AVPixelFormat rgb_type = AV_PIX_FMT_RGB24;



#define DEBUG_YUV
#ifdef DEBUG_YUV
#define WIDTH 640
#define HEIGHT 480
//	yuv_type = AV_PIX_FMT_NV12;
#else
#define WIDTH 720
#define HEIGHT 400
#endif

	av_register_all();

	mem_y = (unsigned char *)malloc(WIDTH*HEIGHT);
	mem_uv = (unsigned char *)malloc(WIDTH*HEIGHT/2);
	mem_v = (unsigned char *)malloc(WIDTH*HEIGHT/2);

	fp = fopen(YUV420_FILE, "rb");
	assert(fp);

	fread(mem_y, WIDTH*HEIGHT, 1, fp);
	fread(mem_uv, WIDTH*HEIGHT/2, 1, fp);
	fclose(fp);

#ifdef DEBUG_YUV
	pSwscontext = sws_getContext(WIDTH, HEIGHT, yuv_type, 
		WIDTH, HEIGHT, rgb_type, SWS_BILINEAR, NULL, NULL, NULL);
#else
	pSwscontext = sws_getContext(WIDTH, HEIGHT, rgb_type, 
		WIDTH, HEIGHT, yuv_type, SWS_BILINEAR, NULL, NULL, NULL);
#endif

	ret = avpicture_alloc(&picture_rgb888, rgb_type, WIDTH, HEIGHT);
	assert(ret==0);
#ifndef DEBUG_YUV
	ptr = picture_rgb888.data[0];
	for (i=0; i<WIDTH*HEIGHT*3; i+=3)
	{
		ptr[i] = 255;
		ptr[i+1] = 0x0;
		ptr[i+2] = 0x0;
	}
#endif
	
	ret = avpicture_alloc(&picture_yuv420p, yuv_type, WIDTH, HEIGHT);
	assert(ret==0);

#ifdef DEBUG_YUV
	memcpy(picture_yuv420p.data[0], mem_y, WIDTH*HEIGHT);
	memset(picture_yuv420p.data[1], 128, picture_yuv420p.linesize[1]*HEIGHT);
	//memcpy(picture_yuv420p.data[1], mem_uv, WIDTH*HEIGHT/4);
	//memcpy(picture_yuv420p.data[2], mem_uv+WIDTH*HEIGHT/4, WIDTH*HEIGHT/4);

	int h = sws_scale(pSwscontext, picture_yuv420p.data,  picture_yuv420p.linesize, 0, HEIGHT,
		picture_rgb888.data, picture_rgb888.linesize);
#else
	int h = sws_scale(pSwscontext, picture_rgb888.data,  picture_rgb888.linesize, 0, HEIGHT,
		picture_yuv420p.data, picture_yuv420p.linesize);
#endif

	ppm_save(picture_rgb888.data[0], picture_rgb888.linesize[0], WIDTH, HEIGHT, "d:\\tmp\\rgb888.ppm");

	avpicture_free(&picture_rgb888);
	avpicture_free(&picture_yuv420p);
	return 0;
}