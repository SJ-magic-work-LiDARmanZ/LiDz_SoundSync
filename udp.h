/************************************************************
************************************************************/
#pragma once

/************************************************************
************************************************************/
#include <stdio.h>
#include <ofMain.h>
#include <ofxNetwork.h>
#include <ofxAudioFFT.h>

#include "sj_common.h"


/************************************************************
************************************************************/
class MyUdp{
private:
	/****************************************
	****************************************/
	ofxUDPManager udp_Send_;
	int counter = 0;
	
	
public:
	/****************************************
	****************************************/
	MyUdp();
	~MyUdp();
	void Setup();
	void Send(const AUDIO_FFT& fft);
	void StartSending();
	void StopSending();
};