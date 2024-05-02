/************************************************************
************************************************************/
#include <udp.h>

/************************************************************
************************************************************/

/******************************
******************************/
MyUdp::MyUdp(){
}

/******************************
******************************/
MyUdp::~MyUdp(){
}

/******************************
******************************/
void MyUdp::Setup(){
	{
		ofxUDPSettings settings;
		settings.sendTo("127.0.0.1", 12346);
		// settings.sendTo("10.0.0.10", 12346);
		settings.blocking = false;
		
		udp_Send_.Setup(settings);
	}
}

/******************************
******************************/
void MyUdp::Send(const AUDIO_FFT& fft){
	/********************
	********************/
	if(!Gui_Global->b_send_udp) return;
	
	/********************
	********************/
	const int BUF_SIZE = 100;
	char buf[BUF_SIZE];
	
	float unity_sound_wave_h			= Gui_Global->unity_sound_wave_h;
	float unity_sound_wave_space		= Gui_Global->unity_sound_wave_space;
	glm::vec3 unity_sound_wave_center	= Gui_Global->unity_sound_wave_center;
	
	//                                        center.xyz     h    space
	snprintf(buf, std::size(buf), "/SoundWave,%.2f,%.2f,%.2f,%.2f,%.2f,", unity_sound_wave_center.x, unity_sound_wave_center.y, unity_sound_wave_center.z,  unity_sound_wave_h, unity_sound_wave_space);
	string message = buf;

	for(int i = 0; i < AUDIO_BUF_SIZE; i++){
		if( (int)Gui_Global->unity_sound_wave_type == 0 )		snprintf(buf, std::size(buf), "%.4f", fft.get_artSin_N(i)); 
		else if( (int)Gui_Global->unity_sound_wave_type == 1 )	snprintf(buf, std::size(buf), "%.4f", fft.get_artSin_A(i)); 
		else													snprintf(buf, std::size(buf), "%.4f", fft.get_artSin_MixDown(i)); 
		
		message += buf;
		
		if(i < AUDIO_BUF_SIZE - 1) message += ",";
	}
	
	/*
	if(counter == 0){
		printf("%s\n", message.c_str());
		fflush(stdout);
		counter++;
	}
	*/
	
	udp_Send_.Send(message.c_str(), message.length());
}







