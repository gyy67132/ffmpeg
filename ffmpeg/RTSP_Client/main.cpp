#include "Rtsp_client.h"



int main()
{
	/*char buf[100] = "H264/90000";
	int timebase;
	char codec[20];
	int ret = sscanf(buf, "%[^/]/%d", codec, &timebase);*/

	Rtsp_client client("rtsp://127.0.0.1:554/live/test");
	if (client.initWinSock() != 0)
		return -1;
	if (client.connectServer() != 0)
		return -1;
	client.startCMD();
}