/************************************************************
************************************************************/
#pragma once

/************************************************************
************************************************************/
#include <ofMain.h>
#include <ofxAudioFFT.h>
#include <ofxNetwork.h>
#include <ofxVboSet.h>
#include "sj_common.h"

/************************************************************
************************************************************/

/**************************************************
**************************************************/
class DRAW_ARTSIN{
private:
	/****************************************
	****************************************/
	enum class State{
		kEnergy_calm,
		kEnergy_l,
		kEnergy_h,
	};
	State state_ = State::kEnergy_calm;
	
	int t_last_change = 0;
	
	/****************************************
	****************************************/
	ofxUDPManager udp_Send_;
	
	ofx__VBO_SET VboSet_N;
	ofx__VBO_SET VboSet_A;
	ofx__VBO_SET VboSet_MixDown;
	ofx__VBO_SET VboSet_energy;
	
	ofTrueTypeFont font_L;
	ofTrueTypeFont font_S;
	
	float energy_ratio_ = 0;
	ofImage img_cursor_left;
	ofImage img_cursor_down;
	int beat_counter_ = 0;
	int down_counter_ = 0;
	
	/****************************************
	****************************************/
	void SetupUdp();
	void IfTimePassed_SendUdp_CutChange(int t_interval);
	void Refresh_vboVerts(const AUDIO_FFT& fft, bool b_Overlay = false);
	void Refresh_vboColor();
	void EnergyStateChart();
	
public:
	DRAW_ARTSIN();
	~DRAW_ARTSIN();
	
	void setup(const AUDIO_FFT& fft);
	void update(const AUDIO_FFT& fft);
	void draw(ofFbo& fbo);
};

