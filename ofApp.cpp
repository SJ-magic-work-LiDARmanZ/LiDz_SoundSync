/************************************************************
************************************************************/
#include <ofApp.h>

/************************************************************
************************************************************/

/******************************
******************************/
ofApp::ofApp(int _soundStream_Input_DeviceId, int _soundStream_Output_DeviceId, int _AUDIO_IN_CHS, int _AUDIO_OUT_CHS)
: b_EnableAudioOut(false)
, soundStream_Input_DeviceId(_soundStream_Input_DeviceId)
, soundStream_Output_DeviceId(_soundStream_Output_DeviceId)
, AUDIO_IN_CHS(_AUDIO_IN_CHS)
, AUDIO_OUT_CHS(_AUDIO_OUT_CHS)
{
	/********************
	********************/
	if( (AUDIO_IN_CHS != 1) || (AUDIO_IN_CHS != 2) )	{ AUDIO_IN_CHS = 2; }
	if( (AUDIO_OUT_CHS != 1) || (AUDIO_OUT_CHS != 2) )	{ AUDIO_OUT_CHS = 2; }
}

/******************************
******************************/
ofApp::~ofApp()
{
	if(Gui_Global) delete Gui_Global;
	
	if(fp_Log)	fclose(fp_Log);
}


/******************************
******************************/
void ofApp::exit(){
	soundStream->close();
	delete soundStream;
	
	printf("> Good-bye\n");
	fflush(stdout);
}

/******************************
******************************/
void ofApp::setup(){
	/********************
	********************/
	ofSetBackgroundAuto(true);
	
	ofSetWindowTitle("Audio FFT");
	ofSetVerticalSync(true);
	ofSetFrameRate(30);
	ofSetWindowShape(MAIN_WINDOW_W, MAIN_WINDOW_H);
	ofSetEscapeQuitsApp(false);
	
	/********************
	********************/
	ofSeedRandom();
	
	/********************
	********************/
	fp_Log = fopen("../../../data/Log.csv", "w");
	
	setup_Gui();
	LoadGuiSettingFile("gui.xml");
	
	fbo_out.allocate(MAIN_WINDOW_W, MAIN_WINDOW_H, GL_RGBA, 4);
	ClearFbo(fbo_out);
	
	img_Frame.load("img_frame/Frame.png");
	
	/********************
	********************/
	if(AUDIO_IN_CHS == 2)	AudioFFT.set_AnalyzeCh(AUDIO_FFT::ANALYZE_CH__STEREO);
	else					AudioFFT.set_AnalyzeCh(AUDIO_FFT::ANALYZE_CH__L);
	
	update_AudioFFT_ParamForFFTCal();
	AudioFFT.setup(AUDIO_BUF_SIZE, AUDIO_SAMPLERATE);
	
	/********************
	********************/
	print_separatoin();
	printf("> sound Device\n");
	fflush(stdout);
	
	/*
	settings.setInListener(this);
	settings.setOutListener(this);
	settings.sampleRate = 44100;
	settings.numInputChannels = 2;
	settings.numOutputChannels = 2;
	settings.bufferSize = bufferSize;
	
	soundStream.setup(settings);
	*/
	soundStream = new ofSoundStream();
	soundStream->printDeviceList();
	
	if( soundStream_Input_DeviceId == -1 ){
		ofExit();
		return;
		
	}else{
		/********************
			soundStream->setup()
		の直後、audioIn()/audioOut()がstartする.
		これらのmethodは、AudioFFTにaccessするので、この前に、AudioFFTが初期化されていないと、不正accessが発生してしまう.
		********************/
		setup_sound();
	}
	
	/********************
	********************/
	DrawFFT.setup(AudioFFT);
	DrawArtSin.setup(AudioFFT);
	
	/********************
	Set whether or not the aspect ratio of this camera is forced to a non-default setting.
	The camera's aspect ratio, by default, is the aspect ratio of your viewport. If you have set a non-default value (with ofCamera::setAspectRatio()), you can toggle whether or not this value is applied.
	********************/
	// camera.setForceAspectRatio(true);
	
	// set_CamParam();
	
	/********************
	********************/
	udp.Setup();
	
	/********************
	********************/
	printf("> start process\n");
	fflush(stdout);
}

/******************************
description
	memoryを確保は、app start後にしないと、
	segmentation faultになってしまった。
******************************/
void ofApp::setup_Gui()
{
	/********************
	********************/
	Gui_Global = new GUI_GLOBAL;
	Gui_Global->setup("param", "gui.xml", 10, 10);
}

/******************************
******************************/
void ofApp::LoadGuiSettingFile(string file_name){
	char buf[100];
	snprintf(buf, std::size(buf), "../../../data/%s", file_name.c_str());
	
	if(is_FileExist(buf)){
		Gui_Global->gui.loadFromFile(file_name);
		printf("> load gui setting\n\n");
		fflush(stdout);
	}else{
		printf("> GUI setting file not exists\n\n");
		fflush(stdout);
	}
}

/******************************
******************************/
bool ofApp::is_FileExist(string str_FileName){
	std::ifstream in{str_FileName, std::ios::in};
	if(in.is_open()){
		in.close();
		return true;
	}else{
		return false;
	}
	
	/*
	FILE* fp = fopen(str_FileName.c_str(), "r");
	if(fp){
		fclose(fp);
		return true;
	}else{
		return false;
	}
	*/
}

/******************************
******************************/
void ofApp::setup_sound(){
	/********************
	********************/
	vector<ofSoundDevice> devices = soundStream->getDeviceList();
	
	ofSoundStreamSettings settings;
	
	settings.setInDevice(devices[soundStream_Input_DeviceId]);
	if(devices[soundStream_Input_DeviceId].name.find("Native Instruments") != std::string::npos){
		/********************
		"name" でDeviceを選択する場合は、
		このblockを参考にしてください。
		********************/
		printf("> name match : Native Instruments selected\n");
		fflush(stdout);
	}
	
	if( soundStream_Output_DeviceId != -1 ){
		settings.setOutDevice(devices[soundStream_Output_DeviceId]);
		b_EnableAudioOut = true;
	}

	settings.setInListener(this);
	settings.numInputChannels = AUDIO_IN_CHS;
	
	if(b_EnableAudioOut){
		settings.setOutListener(this); /* Don't forget this */
		settings.numOutputChannels = AUDIO_OUT_CHS;
	}else{
		settings.numOutputChannels = 0;
	}
	
	settings.sampleRate = AUDIO_SAMPLERATE;
	settings.bufferSize = AUDIO_BUF_SIZE;
	settings.numBuffers = AUDIO_BUFFERS; // 使ってないっぽい
	
	/********************
		soundStream->setup()
	の直後、audioIn()/audioOut()がstartする.
	これらのmethodは、AudioFFTにaccessするので、この前に、AudioFFTが初期化されていないと、不正accessが発生してしまう.
	********************/
	soundStream->setup(settings);
}

/******************************
******************************/
void ofApp::update_AudioFFT_ParamForFFTCal(){
	
	AudioFFT.update_ParamForFFTCal(	Gui_Global->FFT__SoftGain,
									Gui_Global->FFT__k_smooth,
									Gui_Global->FFT__dt_smooth_2_N,
									Gui_Global->FFT__dt_smooth_2_A,
									Gui_Global->FFT__b_HanningWindow,
									Gui_Global->FFT__Afilter_0dB_at_Hz,
									Gui_Global->ArtSin_Band_min__N,
									Gui_Global->ArtSin_Band_max__N,
									Gui_Global->ArtSin_Band_min__A,
									Gui_Global->ArtSin_Band_max__A,
									Gui_Global->ArtSin_PhaseMap_k,
									Gui_Global->b_ArtSin_PhaseMap_MoreDynamic_N,
									Gui_Global->b_ArtSin_PhaseMap_MoreDynamic_A,
									Gui_Global->b_ArtSin_abs,
									Gui_Global->b_TukeyWindow_artSin,
									Gui_Global->Tukey_alpha);
	
}

/******************************
******************************/
void ofApp::update(){
	/********************
	********************/
	if(Gui_Global->b_Audio_Start)			{ Gui_Global->b_Audio_Start = false; Sound__Start(); }
	if(Gui_Global->b_Audio_Stop)			{ Gui_Global->b_Audio_Stop = false; Sound__Stop(); }
	if(Gui_Global->b_Audio_Reset)			{ Gui_Global->b_Audio_Reset = false; Sound__Reset(); }
	
	if(Gui_Global->b_reload_gui_setting)	{ Gui_Global->b_reload_gui_setting = false; LoadGuiSettingFile("gui.xml"); }
	
	/********************
	********************/
	// set_CamParam();
	
	/********************
	********************/
	update_AudioFFT_ParamForFFTCal();
	AudioFFT.update();
	
	DrawFFT.update(AudioFFT);
	DrawArtSin.update(AudioFFT);
}

/******************************
******************************/
void ofApp::draw(){
	/********************
	********************/
	ClearFbo(fbo_out);
	
	/********************
	********************/
	draw_frame(fbo_out);
	DrawFFT.draw(fbo_out, AudioFFT);
	DrawArtSin.draw(fbo_out);
	
	udp.Send(AudioFFT);
	
	/********************
	********************/
	ofEnableAntiAliasing();
	ofEnableBlendMode(OF_BLENDMODE_DISABLED); // OF_BLENDMODE_DISABLED, OF_BLENDMODE_ALPHA, OF_BLENDMODE_ADD
	
	ofBackground(0, 0, 0, 255);
	ofSetColor(255, 255, 255, 255);
	
	fbo_out.draw(0, 0);
	
	/********************
	********************/
	if(Gui_Global->b_Disp){
		Gui_Global->gui.draw();
		
		ofSetColor(255);
		string info;
		info += "FPS = " + ofToString(ofGetFrameRate(), 2) + "\n";
		ofDrawBitmapString(info, 30, 30);
	}
}

/******************************
******************************/
void ofApp::draw_frame(ofFbo& fbo){
	fbo.begin();
		ofEnableAntiAliasing();
		ofEnableBlendMode(OF_BLENDMODE_ADD); // OF_BLENDMODE_DISABLED, OF_BLENDMODE_ALPHA, OF_BLENDMODE_ADD
		
		ofSetColor(255);
		img_Frame.draw(0, 0, fbo.getWidth(), fbo.getHeight());
	fbo.end();
}

/******************************
******************************/
void ofApp::audioIn(ofSoundBuffer & buffer){
	AudioFFT.SetVol(buffer);
}

/******************************
******************************/
void ofApp::audioOut(ofSoundBuffer & buffer){
	AudioFFT.GetVol(buffer, b_EnableAudioOut);
}

/******************************
******************************/
void ofApp::Sound__Start(){
	soundStream->start();
	printf("> soundStream : start\n");
	fflush(stdout);
}

/******************************
******************************/
void ofApp::Sound__Stop(){
	soundStream->stop();
	printf("> soundStream : stop\n");
	fflush(stdout);
}

/******************************
******************************/
void ofApp::Sound__Reset(){
	soundStream->close();
	delete soundStream;
	
	soundStream = new ofSoundStream();
	setup_sound();
	
	printf("> soundStream : close->restart\n");
	fflush(stdout);
}

/******************************
******************************/
void ofApp::keyPressed(int key){
	switch(key){
		case 'd':
			Gui_Global->b_Disp = !Gui_Global->b_Disp;
			break;
			
		case 'm':
			Gui_Global->gui.minimizeAll();
			break;
	}
}

/******************************
******************************/
void ofApp::keyReleased(int key){
}

/******************************
******************************/
void ofApp::mouseMoved(int x, int y ){

}

/******************************
******************************/
void ofApp::mouseDragged(int x, int y, int button){

}

/******************************
******************************/
void ofApp::mousePressed(int x, int y, int button){

}

/******************************
******************************/
void ofApp::mouseReleased(int x, int y, int button){

}

/******************************
******************************/
void ofApp::mouseEntered(int x, int y){

}

/******************************
******************************/
void ofApp::mouseExited(int x, int y){

}

/******************************
******************************/
void ofApp::windowResized(int w, int h){

}

/******************************
******************************/
void ofApp::gotMessage(ofMessage msg){

}

/******************************
******************************/
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
