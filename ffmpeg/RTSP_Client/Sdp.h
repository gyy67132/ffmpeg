#pragma once

#include <string.h>
#include <stdio.h>

#include <iostream>
#include <map>
#include <vector>

using namespace std;

#define TRACK_MAX_NUM 2

class SdpTrack
{
public:
	SdpTrack() {};
	SdpTrack(const SdpTrack& track);
	SdpTrack(const SdpTrack* track);
	bool isAlive = false;
	int playload = -1;//����Ƶ�����ţ�96,97
	char control[30] = { 0 };//streamid=0
	char control_name[30] = { 0 };//streamid
	int control_id = -1;//0

	char codec[20] = {0};//�����ʽ H264
	int timebase = -1;//ʱ���/��ƵƵ�� 90000
	int audio_channel = -1;//��Ƶ����
	int audio_as = -1;//

	bool is_send_setup = false;
};

class Sdp
{
public:
	int parse(char* buff, size_t size);
	SdpTrack* popTrack();

	SdpTrack tracks[TRACK_MAX_NUM];
};

