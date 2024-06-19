#include "Rtsp_client.h"

int sendCmdOptions(int seq)
{

}

int main()
{
	Rtsp_client client;
	if (client.initWinSock() != 0)
		return -1;
	if (client.connectServer() != 0)
		return -1;
}