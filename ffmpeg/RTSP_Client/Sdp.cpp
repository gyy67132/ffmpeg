#include "Sdp.h"

SdpTrack::SdpTrack(const SdpTrack& track)
{
	isAlive = track.isAlive;
	playload = track.playload;
	strcpy(control, track.control);
	strcpy(control_name, track.control_name);
	control_id = track.control_id;

	strcpy(codec, track.codec);
	audio_channel = track.audio_channel;
	audio_as = track.audio_as;
	timebase = track.timebase;

	is_send_setup = track.is_send_setup;
}

SdpTrack::SdpTrack(const SdpTrack* track)
{
	isAlive = track->isAlive;
	playload = track->playload;
	strcpy(control, track->control);
	strcpy(control_name, track->control_name);
	control_id = track->control_id;

	strcpy(codec, track->codec);
	audio_channel = track->audio_channel;
	audio_as = track->audio_as;
	timebase = track->timebase;

	is_send_setup = track->is_send_setup;
}

vector<string> splitString(const string& str, const string &sep)
{
	vector<string> vecstr;
	int sepSize = sep.size();

	int lastPosition = 0;
	int pos = -1;
	while (-1 != (pos = str.find(sep, lastPosition)))
	{
		string tmp = str.substr(lastPosition, pos - lastPosition);
		if (!tmp.empty())
			vecstr.push_back(tmp);
		lastPosition += sepSize + tmp.size();
	}
	string tmp = str.substr(lastPosition);

	if(!tmp.empty())
		vecstr.push_back(tmp);
	return vecstr;
}

int Sdp::parse(char* buff, size_t size)
{
	/*
	* RTSP/1.0 200 OK
	Content-Base: rtsp://127.0.0.1:554/live/test/
	Content-Length: 605
	Content-Type: application/sdp
	CSeq: 2
	Date: Fri, Jun 21 2024 11:55:46 GMT
	Server: ZLMediaKit(git hash:ca7efd5/2023-03-24T16:19:22+08:00,branch:master,build time:2023-03-25T10:26:51)
	Session: YzcbR6Newtk5
	x-Accept-Dynamic-Rate: 1
	x-Accept-Retransmit: our-retransmit

	v=0
	o=- 0 0 IN IP4 0.0.0.0
	s=Streamed by ZLMediaKit(git hash:ca7efd5/2023-03-24T16:19:22+08:00,branch:master,build time:2023-03-25T10:26:51)
	c=IN IP4 0.0.0.0
	t=0 0
	a=range:npt=now-
	a=control:*
	m=video 0 RTP/AVP 96
	b=AS:979
	a=fmtp:96 packetization-mode=1; sprop-parameter-sets=Z2QAKKzZQLQ9v/ACAAGxAAADAAEAAAMAMg8YMZY=,aOvssiw=; profile-level-id=640028
	a=rtpmap:96 H264/90000
	a=control:streamid=0
	m=audio 0 RTP/AVP 97
	b=AS:129
	a=fmtp:97 profile-level-id=1;mode=AAC-hbr;sizelength=13;indexlength=3;indexdeltalength=3; config=121056E500
	a=rtpmap:97 MPEG4-GENERIC/44100/2
	a=control:streamid=1
	*/
	map<string, SdpTrack> SdpTrackMap;
	string currentMediaName;
	bool currentParseMedia = false;
	
	char* line = strtok(buff, "\n");
	while (line)
	{
		if (strstr(line, "m=")) {
			char sdp_name[10] = { 0 };
			int sdp_seq = 0;
			char sdp_transport[10] = { 0 };
			int sdp_playload = 0;
			if (sscanf(line, "m=%s %d %s %d\r\n", sdp_name, &sdp_seq, sdp_transport, &sdp_playload) != 4)
			{
				printf("parse sdp m error\n");
			}
			if (strcmp(sdp_name, "video") == 0)
			{
				currentMediaName = sdp_name;
			}else if (strcmp(sdp_name, "audio") == 0)
			{
				currentMediaName = sdp_name;
			}
			else {
				printf("parse sdp m name error\n");
				break;
			}
			SdpTrack* track = &(SdpTrackMap[currentMediaName]);
			track->isAlive = true;
			track->playload = sdp_playload;
			currentParseMedia = true;
		}
		else if (currentParseMedia)
		{
			SdpTrack* track = &(SdpTrackMap[currentMediaName]);
			if (strstr(line, "a=control:"))
			{	//a=control:streamid=0
				int streamId = -1;
				char val[50] = { 0 };
				if (sscanf(line, "a=control:%s\r\n", val) != 1)
				{
					printf("parse a=control: error\n");
				}
				else {
					memcpy(track->control, val, strlen(val));
					vector<string> vec = splitString(track->control, "=");
					if (vec.size() == 2)
					{
						memcpy(track->control_name, vec[0].data(),strlen(vec[0].data()));
						track->control_id = atoi(vec[1].data());
					}
					else
					{
						printf("parse a=control: val error\n");
					}
				}
			}
			else if (strstr(line, "a=rtpmap:"))
			{
				//a=rtpmap:96 H264/90000

				char val[30] = { 0 };
				if (96 == track->playload) {
					if (sscanf(line, "a=rtpmap:96 %s\r\n", val) != 1)
					{
						printf("parse a=rtpmap:96 val error\n");
					}
					else {
						vector<string> vec = splitString(val, "/");
						if (vec.size() == 2)
						{
							memcpy(track->codec, vec[0].data(), strlen(vec[0].data()) );
							track->timebase = atoi(vec[1].data());
						}
						else
						{
							printf("parse a=rtpmap:96 H264/90000 error\n");
						}
					}
				}else if (97 == track->playload) {
					//a=rtpmap:97 MPEG4-GENERIC/44100/2
					if (sscanf(line, "a=rtpmap:97 %s\r\n", val) != 1)
					{
						printf("parse a=rtpmap:97 val error\n");
					}
					else {
						vector<string> vec = splitString(val, "/");
						if (vec.size() == 3)
						{
							memcpy(track->codec, vec[0].data(), strlen(vec[0].data()));
							track->timebase = atoi(vec[1].data());
							track->audio_channel = atoi(vec[2].data());
						}
						else
						{
							printf("parse a=rtpmap:97 MPEG4-GENERIC/44100/2 error\n");
						}
					}
				}
				else
				{
					printf("parse a=rtpmap: error\n");
				}
			}
			else if (strstr(line, "b="))
			{
				//b=AS:129
				if (sscanf(line, "b=AS:%d\r\n", &track->audio_as) != 1)
				{
					printf("parse b=AS: error\n");
				}
			}
		}
		line = strtok(NULL, "\n");
	}

	for (auto tmp : SdpTrackMap)
	{
		SdpTrack track = tmp.second;
		if (track.control_id < TRACK_MAX_NUM)
			tracks[track.control_id] = track;
	}

	return 0;
}

SdpTrack* Sdp::popTrack()
{
	for (int i = 0; i < TRACK_MAX_NUM; i++)
	{
		if (tracks[i].isAlive && tracks[i].is_send_setup == false)
		{
			tracks[i].is_send_setup = true;
			return &tracks[i];
		}
	}
	return nullptr;
}