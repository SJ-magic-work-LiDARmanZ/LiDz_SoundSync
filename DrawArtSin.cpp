/************************************************************
************************************************************/
#include <DrawArtSin.h>

/************************************************************
************************************************************/

/******************************
******************************/
DRAW_ARTSIN::DRAW_ARTSIN()
{
	font_L.load("font/RictyDiminished-Regular.ttf", 30, true, true, false, 0.3f, 72);
	font_S.load("font/RictyDiminished-Regular.ttf", 20, true, true, false, 0.3f, 72);
}

/******************************
******************************/
DRAW_ARTSIN::~DRAW_ARTSIN()
{
}

/******************************
******************************/
void DRAW_ARTSIN::setup(const AUDIO_FFT& fft)
{
	/********************
	********************/
	print_separatoin();
	printf("> setup : Draw ArtSin 2D\n");
	fflush(stdout);
	
	// img_Frame.load("img_frame/Frame.png");
	img_cursor_left.load("img_frame/Cursor_left.png");
	img_cursor_down.load("img_frame/Cursor_down.png");
	
	img_cursor_left.setAnchorPercent(0, 0.5);
	img_cursor_down.setAnchorPercent(0.5, 1.0);
	
	/********************
	********************/
	VboSet_N.setup(AUDIO_BUF_SIZE * 4);
	VboSet_A.setup(AUDIO_BUF_SIZE * 4);
	VboSet_MixDown.setup(AUDIO_BUF_SIZE * 4);
	VboSet_energy.setup(4);
	
	if(Gui_Global->b_DrawArtSin_Overlay)	Refresh_vboVerts(fft, true);
	else									Refresh_vboVerts(fft);
	
	Refresh_vboColor();
	
	/********************
	********************/
	SetupUdp();
}

/******************************
******************************/
void DRAW_ARTSIN::SetupUdp(){
	{
		ofxUDPSettings settings;
		settings.sendTo("127.0.0.1", 12348);
		settings.blocking = false;
		
		udp_Send_.Setup(settings);
	}
}

/******************************
1--2
|  |
0--3
******************************/
void DRAW_ARTSIN::Refresh_vboVerts(const AUDIO_FFT& fft, bool b_Overlay)
{
	/********************
	********************/
	const float win_h = 350;
	
	/********************
	VboSet_A
	********************/
	const float width_of_bar = 1;
	const float space_per_bar = 3;
	float ScreenHeightOfOne = Gui_Global->DrawArtSin_ScreenHeightOfOne;
	glm::vec2 ofs = glm::vec2(192, 400);
	
	for(int i = 0; i < AUDIO_BUF_SIZE; i++){
		float _y = sj_max( float(ofs.y - fft.get_artSin_A(i) * ScreenHeightOfOne), float(ofs.y - win_h) );
		
		VboSet_A.set_vboVerts( i * 4 + 0, ofs.x + i * space_per_bar,				ofs.y );
		VboSet_A.set_vboVerts( i * 4 + 1, ofs.x + i * space_per_bar,				_y );
		VboSet_A.set_vboVerts( i * 4 + 2, ofs.x + i * space_per_bar + width_of_bar,	_y);
		VboSet_A.set_vboVerts( i * 4 + 3, ofs.x + i * space_per_bar + width_of_bar,	ofs.y );
	}
	
	VboSet_A.update_vertex();
	
	/********************
	VboSet_N
	********************/
	const float devide = Gui_Global->DrawEnergy_devide;
	
	const int min_id = AUDIO_BUF_SIZE / devide;
	const int max_id = AUDIO_BUF_SIZE - AUDIO_BUF_SIZE / devide;
	float max_occupied = 0;
	float occupied = 0;
	
	if(!b_Overlay) ofs = glm::vec2(192, 800);
	
	for(int i = 0; i < AUDIO_BUF_SIZE; i++){
		float height = sj_min( float(fft.get_artSin_N(i) * ScreenHeightOfOne), win_h);
		
		if( (min_id <= i) && (i < max_id) ){
			max_occupied += win_h;
			occupied += height;
		}
		
		float _y = ofs.y - height;
		
		VboSet_N.set_vboVerts( i * 4 + 0, ofs.x + i * space_per_bar,				ofs.y );
		VboSet_N.set_vboVerts( i * 4 + 1, ofs.x + i * space_per_bar,				_y );
		VboSet_N.set_vboVerts( i * 4 + 2, ofs.x + i * space_per_bar + width_of_bar,	_y);
		VboSet_N.set_vboVerts( i * 4 + 3, ofs.x + i * space_per_bar + width_of_bar,	ofs.y );
	}
	
	float energy_ratio_N = (max_occupied == 0) ? 0 : occupied / max_occupied;
	VboSet_N.update_vertex();
	
	/********************
	VboSet_MixDown
	********************/
	max_occupied = 0;
	occupied = 0;
	
	if(b_Overlay) ofs = glm::vec2(192, 800);
    
	for(int i = 0; i < AUDIO_BUF_SIZE; i++){
		float height = sj_min( float(fft.get_artSin_MixDown(i) * ScreenHeightOfOne), win_h);
		
		if( (min_id <= i) && (i < max_id) ){
			max_occupied += win_h;
			occupied += height;
		}
		
		float _y = ofs.y;
		if(b_Overlay) _y = ofs.y - height;
		
		VboSet_MixDown.set_vboVerts( i * 4 + 0, ofs.x + i * space_per_bar,					ofs.y );
		VboSet_MixDown.set_vboVerts( i * 4 + 1, ofs.x + i * space_per_bar,					_y );
		VboSet_MixDown.set_vboVerts( i * 4 + 2, ofs.x + i * space_per_bar + width_of_bar,	_y);
		VboSet_MixDown.set_vboVerts( i * 4 + 3, ofs.x + i * space_per_bar + width_of_bar,	ofs.y );
	}
	
	float energy_ratio_MixDown = (max_occupied == 0) ? 0 : occupied / max_occupied;
	VboSet_MixDown.update_vertex();
	
	/********************
	Energy
	********************/
	ofs = glm::vec2(1782, 800);
	
	if(b_Overlay)	energy_ratio_ = energy_ratio_MixDown * Gui_Global->DrawEnergy_lpf + energy_ratio_ * (1 - Gui_Global->DrawEnergy_lpf);
	else			energy_ratio_ = energy_ratio_N * Gui_Global->DrawEnergy_lpf + energy_ratio_ * (1 - Gui_Global->DrawEnergy_lpf);
	
	VboSet_energy.set_vboVerts(0, ofs.x,					ofs.y);
	VboSet_energy.set_vboVerts(1, ofs.x,					ofs.y - energy_ratio_ * win_h);
	VboSet_energy.set_vboVerts(2, ofs.x + 22/* width */,	ofs.y - energy_ratio_ * win_h);
	VboSet_energy.set_vboVerts(3, ofs.x + 22/* width */,	ofs.y);
	
	VboSet_energy.update_vertex();
	
	/* */
	EnergyStateChart();
}

/******************************
******************************/
void DRAW_ARTSIN::IfTimePassed_SendUdp_CutChange(int t_interval){
	/********************
	********************/
	int now = ofGetElapsedTimeMillis();
	
	/********************
	********************/
	if( t_interval < now - t_last_change ){
		/********************
		********************/
		t_last_change = now;
		
		/********************
		********************/
		char buf[100];
		
		snprintf(buf, std::size(buf), "/SoundSyncCutChange");
		string message = buf;
		
		udp_Send_.Send(message.c_str(), message.length());
		
		/********************
		for debug
		********************/
		beat_counter_++;
		if(100 < beat_counter_) beat_counter_ = 0;
	}
}

/******************************
******************************/
void DRAW_ARTSIN::EnergyStateChart(){
	float th_calm, th_l, th_h;
	
	std::vector<float> v = {Gui_Global->DrawEnergy_th[0], Gui_Global->DrawEnergy_th[1], Gui_Global->DrawEnergy_th[2]};
	std::sort(v.begin(), v.end());
	
	th_calm = v[0];
	th_l = v[1];
	th_h = v[2];
	
	switch(state_){
		case State::kEnergy_calm:
			if( th_h < energy_ratio_ ){
				state_ = State::kEnergy_h;
				// IfTimePassed_SendUdp_CutChange( (int)(Gui_Global->DrawEnergy_cut_change_interval * 1000 / 2 /* calm->hは感度上げる */) );
				IfTimePassed_SendUdp_CutChange( (int)(Gui_Global->DrawEnergy_cut_change_interval * 1000 ) );
			}else{
				// IfTimePassed_SendUdp_CutChange( (int)(Gui_Global->DrawEnergy_cut_change_interval * 1000 * 2 /* stay calm時はslowに */) );
			}
			break;
			
		case State::kEnergy_l:
			if( th_h < energy_ratio_ ){
				state_ = State::kEnergy_h;
				IfTimePassed_SendUdp_CutChange( (int)(Gui_Global->DrawEnergy_cut_change_interval * 1000) );
			}else if( energy_ratio_ < th_calm ){
				state_ = State::kEnergy_calm;
				// IfTimePassed_SendUdp_CutChange( (int)(Gui_Global->DrawEnergy_cut_change_interval * 1000) );
			}
			break;
			
		case State::kEnergy_h:
			if( energy_ratio_ < th_calm ){ // calmが先
				state_ = State::kEnergy_calm;
				// IfTimePassed_SendUdp_CutChange( (int)(Gui_Global->DrawEnergy_cut_change_interval * 1000) );
			}else if( energy_ratio_ < th_l ){
				state_ = State::kEnergy_l;
			}
			break;
	}
}

/******************************
******************************/
void DRAW_ARTSIN::Refresh_vboColor()
{
	VboSet_N.set_singleColor(Gui_Global->DrawArtSin_col__N);
	VboSet_N.update_color();
	
	VboSet_A.set_singleColor(Gui_Global->DrawArtSin_col__A);
	VboSet_A.update_color();
	
	VboSet_MixDown.set_singleColor(Gui_Global->DrawArtSin_col__MixDown);
	VboSet_MixDown.update_color();
	
	VboSet_energy.set_singleColor(Gui_Global->DrawEnergy_col);
	VboSet_energy.update_color();
	
	return;
}

/******************************
******************************/
void DRAW_ARTSIN::update(const AUDIO_FFT& fft)
{
	if(Gui_Global->b_DrawArtSin_Overlay)	Refresh_vboVerts(fft, true);
	else									Refresh_vboVerts(fft);
	
	Refresh_vboColor();
}

/******************************
******************************/
void DRAW_ARTSIN::draw(ofFbo& fbo)
{
	fbo.begin();
		/********************
		********************/
		ofEnableAntiAliasing();
		ofEnableBlendMode(OF_BLENDMODE_ADD); // OF_BLENDMODE_DISABLED, OF_BLENDMODE_ALPHA, OF_BLENDMODE_ADD
		
		/********************
		Graph
		********************/
		ofFill();
		
		ofSetColor(255);
		// glPointSize(Gui_Global->particleSize);
		
		VboSet_N.draw(GL_QUADS);
		VboSet_A.draw(GL_QUADS);
		
		if(Gui_Global->b_DrawArtSin_Overlay)	VboSet_MixDown.draw(GL_QUADS);
		
		/********************
		energy
		********************/
		VboSet_energy.draw(GL_QUADS);
		
		const float win_h = 350;
		glm::vec2 ofs = glm::vec2(1813, 800);
		
		for(int i = 0; i < 3; i++){
			float height = Gui_Global->DrawEnergy_th[i] * win_h;
			img_cursor_left.draw(ofs.x, ofs.y - height);
		}
		
		const float win_w = 1536;
		ofs = glm::vec2(192, 450);
		img_cursor_down.draw(ofs.x + win_w / Gui_Global->DrawEnergy_devide, ofs.y);
		img_cursor_down.draw(ofs.x + win_w * (1 - 1/ Gui_Global->DrawEnergy_devide), ofs.y);
		
		
		{
			char buf[BUF_SIZE_S];
			snprintf(buf, std::size(buf), "%.2f", energy_ratio_);
			font_S.drawString(buf, 1816, 468);
			
			snprintf(buf, std::size(buf), "%3d", beat_counter_);
			font_S.drawString(buf, 1825, 497);
			
			/*
			if(state_ == State::kEnergy_calm)	font_S.drawString("calm", 1870, 468);
			else if(state_ == State::kEnergy_l)	font_S.drawString("   l", 1870, 468);
			else if(state_ == State::kEnergy_h)	font_S.drawString("   h", 1870, 468);
			*/
			
			snprintf(buf, std::size(buf), "%3d", ((int)ofGetElapsedTimeMillis() - t_last_change) / 1000);
			font_S.drawString(buf, 1870, 497);
		}
		
		/********************
		mask
		********************/
		if(Gui_Global->b_DrawArtSin_MaskUpper){
			ofEnableBlendMode(OF_BLENDMODE_ALPHA); // OF_BLENDMODE_DISABLED, OF_BLENDMODE_ALPHA, OF_BLENDMODE_ADD
			ofSetColor(Gui_Global->col_ClearFbo);
			ofDrawRectangle(192, 50, 1536, 350);
			
			ofEnableBlendMode(OF_BLENDMODE_ADD); // 戻す.
		}
		
		if(Gui_Global->b_DrawArtSin_MaskLower){
			ofEnableBlendMode(OF_BLENDMODE_ALPHA); // OF_BLENDMODE_DISABLED, OF_BLENDMODE_ALPHA, OF_BLENDMODE_ADD
			ofSetColor(Gui_Global->col_ClearFbo);
			ofDrawRectangle(192, 450, 1536, 350);
			
			ofEnableBlendMode(OF_BLENDMODE_ADD); // 戻す.
		}
		
		/********************
		目盛り
		********************/
		{
			const float window_H = 350.0;
			float top_val_of_the_window = get_RawVal_top_of_artsin_window(window_H, Gui_Global->DrawArtSin_ScreenHeightOfOne);
			
			ofSetColor(255);
			char buf[BUF_SIZE_S];
			snprintf(buf, std::size(buf), "%.2f", top_val_of_the_window);
			font_S.drawString(buf, 141, 55);
			font_S.drawString(buf, 141, 456);
		}
		
		/********************
		Graph title
		********************/
		if(Gui_Global->b_DrawArtSin_Overlay){
			char buf[BUF_SIZE_S];
			snprintf(buf, std::size(buf), "Overlay");
			font_L.drawString(buf, 74, 234);
			
			snprintf(buf, std::size(buf), "MixDown");
			font_L.drawString(buf, 74, 634);
		}else{
			char buf[BUF_SIZE_S];
			snprintf(buf, std::size(buf), "FilterA");
			font_L.drawString(buf, 74, 234);
			
			snprintf(buf, std::size(buf), "Normal");
			font_L.drawString(buf, 74, 634);
		}
		
	fbo.end();
}

