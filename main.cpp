#include "stdio.h"
#include "stdlib.h"
#include "FrameExtractor/FrameExtractor.h"
#include "FrameExtractor/H264Frames.h"
#define INPUT_FILE "d:\\work\\windows\\FrameExtractor_test\\slamtv60.264"
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

int main()
{
	FRAMEX_CTX *pCTX;
	FILE *fp;
	FRAMEX_STRM_PTR frame_stream;
	unsigned char *buf;
	int len;
	unsigned char *ptr;

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

	len = ExtractConfigStreamH264(pCTX, (unsigned char *)&frame_stream, buf, BUF_SIZE, NULL);
	if (len>0)
	{
		dump_to_file(OUTPUT_FILE, (char*)buf, len);
	}

	while((len = NextFrameH264(pCTX, (unsigned char *)&frame_stream, (unsigned char*)buf, BUF_SIZE, NULL)) >0)
	{
		dump_to_file(OUTPUT_FILE, (char*)buf, len);
	}
	
	printf("finish...\n");
//	getchar();
	return 0;
}