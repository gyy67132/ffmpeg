#include<stdio.h>
#include <malloc.h>

int YUV2y2u2v(const char* path, int width, int height)
{
	FILE* fp = fopen(path, "rb+");
	FILE* fp2 = fopen("YUV420.y", "wb+");
	FILE* fp3 = fopen("YUV420.u", "wb+");
	FILE* fp4 = fopen("YUV420.v", "wb+");
	if (!fp || !fp2 || !fp3 || !fp4)
		return -1;
	unsigned char* p = (unsigned char*)malloc(width * height * 3 / 2);

	int i = 1;
	while (i)
	{
		size_t ret = fread(p, 1, width * height * 3 / 2, fp);
		if (ret < width * height * 3 / 2)
			break;
		fwrite(p, 1, width * height, fp2);
		fwrite(p + width * height, 1, width * height / 4, fp3);
		fwrite(p + width * height * 5 / 4, 1, width * height / 4, fp4);
		i = 0;
	}
	fclose(fp);
	fclose(fp2);
	fclose(fp3);
	fclose(fp4);
	
	return 0;
}

int main()
{
	//YUV2y2u2v("./dump-1280x720.yuv", 1280, 720);
	YUV2y2u2v("11.yuv", 720, 480);

	return 0;
}