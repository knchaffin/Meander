/*  Copyright (C) 2019-2024 Ken Chaffin  
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.
*/      

#include <rack.hpp>      
   

#include "plugin.hpp"   
#include <sstream> 
#include <iomanip>
#include <time.h>

#include <iostream>
#include <fstream> 
#include <string>

#include "ModeScaleQuant.hpp"  



struct ModeScaleQuant : Module  
{
	// poly quant vars and functions
	
	bool polyQuant_scaleNotes[12];
	int polyQuant_searchSpaces[24];
	bool polyQuant_outputNotes[24];

	int ModeScaleQuantScale[7]; // heptatonic scale
	
	void onResetScale()
	{
		    // send ROOT output here to make sure it is initalized and updated
			outputs[OUT_EXT_ROOT_OUTPUT].setVoltage(root_key/12.0);
			// send Poly External Scale to output  
						
			if (scale_out_mode == HEPTATONIC_CHROMATIC_12CH) // aria's poly ext. scale  . This is still default to match V1
			{
				outputs[OUT_EXT_POLY_SCALE_OUTPUT].setChannels(12);  // set polyphony to 12 channels
				for (int i=0; i<12; ++i)
				{
					outputs[OUT_EXT_POLY_SCALE_OUTPUT].setVoltage(0.0,i);  // (not scale note, channel) 
				}
				for (int i=0;i<MSQ_mode_step_intervals[mode][0];++i)
				{
					int note=(int)(notes[i]%MAX_NOTES);  
					if (note==root_key)
						outputs[OUT_EXT_POLY_SCALE_OUTPUT].setVoltage(10.0,(int)note);  // (scale note, channel) 
					else
						outputs[OUT_EXT_POLY_SCALE_OUTPUT].setVoltage(8.0,(int)note);  // (scale note, channel) 
				}
			}

			if (scale_out_mode == HEPTATONIC_DIATONIC_STD_7CH)   // traditional heptatonic scale with root first
			{
				outputs[OUT_EXT_POLY_SCALE_OUTPUT].setChannels(7);  // set polyphony to 7 channels
				for (int i=0;i<MSQ_mode_step_intervals[mode][0];++i)
				{
					int note=(int)(notes[i]%(MAX_NOTES));  
					outputs[OUT_EXT_POLY_SCALE_OUTPUT].setVoltage((float)note/12.0,i);  // (scale note, channel) 
				}
			}
			
			if ((scale_out_mode == PENTATONIC_5CH)&&((mode==0)||(mode==1)||(mode==2)))   // major modes
			{
				outputs[OUT_EXT_POLY_SCALE_OUTPUT].setChannels(5);  // set polyphony to 5 channels
				int scale_note_index=0;
				for (int i=0;i<MSQ_mode_step_intervals[mode][0];++i)
				{
					int note=(int)(notes[i]%(MAX_NOTES)); 
					if ((i!=3)&&(i!=6)&&(scale_note_index<5))  // remove 4th and 7th (i=3,6)
					{
						outputs[OUT_EXT_POLY_SCALE_OUTPUT].setVoltage((float)note/12.0,scale_note_index);  // (scale note, channel) 
						++scale_note_index;
					}
				}
			}
		
			if ((scale_out_mode == PENTATONIC_5CH)&&((mode==3)||(mode==4)||(mode==5)||(mode==6)))   // minor modes
			{
				outputs[OUT_EXT_POLY_SCALE_OUTPUT].setChannels(5);  // set polyphony to 5 channels
				int scale_note_index=0;
				for (int i=0;i<MSQ_mode_step_intervals[mode][0];++i)
				{
					int note=(int)(notes[i]%(MAX_NOTES));  
					if ((i!=1)&&(i!=5)&&(scale_note_index<5))  // remove 2nd and  6th (i=1,5)
					{
						outputs[OUT_EXT_POLY_SCALE_OUTPUT].setVoltage((float)note/12.0,scale_note_index);  // (scale note, channel) 
						++scale_note_index;
					}
				}
			}	

			// if pentatonic major
			if ((scale_out_mode == PENTATONIC_CHROMATIC_12CH)&&((mode==0)||(mode==1)||(mode==2)))   // major modes  Lydian, Ionian, Mixolydian
			{
				outputs[OUT_EXT_POLY_SCALE_OUTPUT].setChannels(12);  // set polyphony to 12 channels
				for (int i=0; i<12; ++i)
				{
					outputs[OUT_EXT_POLY_SCALE_OUTPUT].setVoltage(0.0,i);  // set to 0
				}
				for (int i=0;i<MSQ_mode_step_intervals[mode][0];++i)
				{
					int note=(int)(notes[i]%(MAX_NOTES)); 
					if ((i!=3)&&(i!=6))  // remove 4th and 7th (i=3,6)
					{
							if (note==root_key)
							outputs[OUT_EXT_POLY_SCALE_OUTPUT].setVoltage(10.0,(int)note);  // (scale note, channel) 
						else
							outputs[OUT_EXT_POLY_SCALE_OUTPUT].setVoltage(8.0,(int)note);  // (scale note, channel) 
					}
				}
			}
		
			// if pentatonic minor
			if ((scale_out_mode == PENTATONIC_CHROMATIC_12CH)&&((mode==3)||(mode==4)||(mode==5)||(mode==6)))   // minor modes: Dorian, Aeolian, Phrygian, Locrian
			{
				outputs[OUT_EXT_POLY_SCALE_OUTPUT].setChannels(12);  // set polyphony to 12 channels
				for (int i=0; i<12; ++i)
				{
					outputs[OUT_EXT_POLY_SCALE_OUTPUT].setVoltage(0.0,i);  // set to 0
				}
				for (int i=0;i<MSQ_mode_step_intervals[mode][0];++i)
				{
					int note=(int)(notes[i]%(MAX_NOTES)); 
					if ((i!=1)&&(i!=5))  // remove 2nd and  6th (i=1,5)
					{
						if (note==root_key)
							outputs[OUT_EXT_POLY_SCALE_OUTPUT].setVoltage(10.0,(int)note);  // (scale note, channel) 
						else
							outputs[OUT_EXT_POLY_SCALE_OUTPUT].setVoltage(8.0,(int)note);  // (scale note, channel) 
					}
				}
			}
	}
	

	void onResetQuantizer() 
	{
		// get ModeScaleQuant scale
		for (int i=0;((i<7)&&(i<MSQ_mode_step_intervals[mode][0]));++i)
		{
			int note=(int)(notes[i]%12);  
			ModeScaleQuantScale[i]=note;
		}

		for (int i = 0; i < 12; i++) // initialize clear scale notes
		{
			polyQuant_scaleNotes[i] = false;
		} 

		// get ModeScaleQuant heptatonic scale
		
		for (int i=0;i<7;++i)
		{
			int scale_note=ModeScaleQuantScale[i];
			polyQuant_scaleNotes[scale_note] = true;
		}
			
		polyQuant_determine_searchSpaces();
	}

	void polyQuant_determine_searchSpaces() 
	{
		for (int i = 0; i < 24; i++) 
		{
			int bestNote = 0;
			int leastError = 1000;
			for (int testNote = -12; testNote <= 24; ++testNote) 
			{
				int testError = std::abs((i + 1) / 2 - testNote);
				if (!polyQuant_scaleNotes[eucMod(testNote, 12)]) 
					continue;
				
				if (testError < leastError) 
				{
					bestNote = testNote;
					leastError = testError;
				}
				else 
					break;
				
			}
			polyQuant_searchSpaces[i] = bestNote;
		}
	}
	// end poly quant vars and functions
	
	enum ParamIds 
	{
		CONTROL_ROOT_KEY_PARAM,
		CONTROL_SCALE_PARAM,
		BUTTON_CIRCLESTEP_C_PARAM,
		BUTTON_CIRCLESTEP_G_PARAM,
		BUTTON_CIRCLESTEP_D_PARAM,
		BUTTON_CIRCLESTEP_A_PARAM,
		BUTTON_CIRCLESTEP_E_PARAM,
		BUTTON_CIRCLESTEP_B_PARAM,
		BUTTON_CIRCLESTEP_GBFS_PARAM,
		BUTTON_CIRCLESTEP_DB_PARAM,
		BUTTON_CIRCLESTEP_AB_PARAM,
		BUTTON_CIRCLESTEP_EB_PARAM,
		BUTTON_CIRCLESTEP_BB_PARAM,
		BUTTON_CIRCLESTEP_F_PARAM,
			
		NUM_PARAMS
	};

	enum InputIds 
	{
		IN_ROOT_KEY_EXT_CV,
		IN_SCALE_EXT_CV,
	    IN_POLY_QUANT_EXT_CV,
		NUM_INPUTS
		
	};
	
	
	enum OutputIds 
	{
		OUT_HARMONY_GATE_OUTPUT,
		OUT_HARMONY_CV_OUTPUT,
		OUT_EXT_POLY_SCALE_OUTPUT,
		OUT_EXT_POLY_QUANT_OUTPUT,
		OUT_EXT_ROOT_OUTPUT,
		OUT_EXT_SCALE_OUTPUT,
		OUT_EXT_POLY_QUANT_TRIGGER_OUTPUT,
		NUM_OUTPUTS
	};

	enum LightIds 
	{
	
		LIGHT_LEDBUTTON_CIRCLESTEP_1,
		LIGHT_LEDBUTTON_CIRCLESTEP_2,
		LIGHT_LEDBUTTON_CIRCLESTEP_3,
		LIGHT_LEDBUTTON_CIRCLESTEP_4,
		LIGHT_LEDBUTTON_CIRCLESTEP_5,
		LIGHT_LEDBUTTON_CIRCLESTEP_6,
		LIGHT_LEDBUTTON_CIRCLESTEP_7,
		LIGHT_LEDBUTTON_CIRCLESTEP_8,
		LIGHT_LEDBUTTON_CIRCLESTEP_9,
		LIGHT_LEDBUTTON_CIRCLESTEP_10,
		LIGHT_LEDBUTTON_CIRCLESTEP_11,
		LIGHT_LEDBUTTON_CIRCLESTEP_12,
		LIGHT_CIRCLE_ROOT_KEY_POSITION_1_LIGHT,
		LIGHT_CIRCLE_ROOT_KEY_POSITION_2_LIGHT,
		LIGHT_CIRCLE_ROOT_KEY_POSITION_3_LIGHT,
		LIGHT_CIRCLE_ROOT_KEY_POSITION_4_LIGHT,
		LIGHT_CIRCLE_ROOT_KEY_POSITION_5_LIGHT,
		LIGHT_CIRCLE_ROOT_KEY_POSITION_6_LIGHT,
		LIGHT_CIRCLE_ROOT_KEY_POSITION_7_LIGHT,
		LIGHT_CIRCLE_ROOT_KEY_POSITION_8_LIGHT,
		LIGHT_CIRCLE_ROOT_KEY_POSITION_9_LIGHT,
		LIGHT_CIRCLE_ROOT_KEY_POSITION_10_LIGHT,
		LIGHT_CIRCLE_ROOT_KEY_POSITION_11_LIGHT,
		LIGHT_CIRCLE_ROOT_KEY_POSITION_12_LIGHT,
	
		
		NUM_LIGHTS
	};

	#include "MSQ_data_and_functions.hpp" // for module vars 

	enum ScaleOutMode {
		HEPTATONIC_CHROMATIC_12CH,
		HEPTATONIC_DIATONIC_STD_7CH,
		PENTATONIC_5CH,
		PENTATONIC_CHROMATIC_12CH
	};

	ScaleOutMode scale_out_mode = HEPTATONIC_CHROMATIC_12CH;//  aria's poly ext. scale is default to match V1

	// Clock code adapted from Strum and AS

	struct ModeScaleQuantLFOGenerator 
	{
		float phase = 0.0f;
		float pw = 0.5f;
		float freq = 1.0f;
		dsp::SchmittTrigger resetTrigger;
		void setFreq(float freq_to_set)
		{
			freq = freq_to_set;
		}		
		void step(float dt) 
		{
			float deltaPhase = fminf(freq * dt, 0.5f);
			phase += deltaPhase;
			if (phase >= 1.0f)
				phase -= 1.0f;
		}
		void setReset(float reset) 
		{
			if (resetTrigger.process(reset)) 
			{
				phase = 0.0f;
			}
		}
		float sqr() 
		{
			float sqr = (phase < pw) ? 1.0f : -1.0f;
			return sqr;
		}
	};  // struct ModeScaleQuantLFOGenerator 

	void userPlaysCirclePositionHarmony(int circle_position, float octaveOffset)  // C=0   play immediate
	{
		theModeScaleQuantState.last_harmony_chord_root_note=circle_of_fifths[circle_position];

		bar_note_count=0;  // for this module we do noty keep up with the bar caount so reset it each time a circle chord is played

		if (octaveOffset>9)
			octaveOffset=9;

		valid_current_circle_degree=false;

		for (int i=0; i<7; ++i) // melody and bass will use this to accompany 
		{
			if  (theCircleOf5ths.theDegreeSemiCircle.degreeElements[i].CircleIndex==circle_position)
			{
				int theDegree=theCircleOf5ths.theDegreeSemiCircle.degreeElements[i].Degree;
				if ((theDegree<1)||(theDegree>7))
			     	  theDegree=1;  // force to a valid degree to avoid a crash
				else
				   valid_current_circle_degree=true;  // used in panel circle update
				current_circle_degree = theDegree;
				for (int j=0;j<MAX_STEPS;++j)
				{
					if (theHarmonyTypes[harmony_type].harmony_steps[j]==theDegree)
					{
						theModeScaleQuantState.last_harmony_step=j;
						break;
					}
				}
				break;
			} 
		} 

		theModeScaleQuantState.theMelodyParms.last_step=theModeScaleQuantState.last_harmony_step;
		int note_index=	(int)(theModeScaleQuantState.theMelodyParms.note_avg*num_step_chord_notes[theModeScaleQuantState.last_harmony_step]);		// not sure this is necessary
		note_index=clamp(note_index, 0, num_step_chord_notes[theModeScaleQuantState.last_harmony_step]-1);
		theModeScaleQuantState.theMelodyParms.last_chord_note_index= note_index;
		 
		int current_chord_note=0;
		int root_key_note=circle_of_fifths[circle_position]; 
		int circle_chord_type= theCircleOf5ths.Circle5ths[circle_position].chordType;
		theModeScaleQuantState.theHarmonyParms.last_chord_type=circle_chord_type;
		int num_chord_members=chord_type_num_notes[circle_chord_type];
		
		outputs[OUT_HARMONY_CV_OUTPUT].setChannels(3);  // set polyphony

		if (valid_current_circle_degree)
		{	
			for (int j=0;j<num_chord_members;++j) 
			{
				current_chord_note=(int)((int)root_key_note+(int)chord_type_intervals[circle_chord_type][j]);
				int note_to_play=current_chord_note+(octaveOffset*12);
				note_to_play=clamp(note_to_play, root_key, 108+root_key); // clamp to MIDI range root0 to (C8+root)
				outputs[OUT_HARMONY_CV_OUTPUT].setVoltage((note_to_play/12.0)-4.0,j);  // (note, channel) 
						
				if (j<4)
				{
					theModeScaleQuantState.theHarmonyParms.last[j].note=note_to_play;
					theModeScaleQuantState.theHarmonyParms.last[j].noteType=NOTE_TYPE_CHORD;
					theModeScaleQuantState.theHarmonyParms.last[j].length=theModeScaleQuantState.theHarmonyParms.note_length_divisor;  
					theModeScaleQuantState.theHarmonyParms.last[j].time32s=barts_count;
					theModeScaleQuantState.theHarmonyParms.last[j].countInBar=bar_note_count;
					theModeScaleQuantState.theHarmonyParms.last[j].isPlaying=true;
					if (bar_note_count<256)
					played_notes_circular_buffer[bar_note_count++]=theModeScaleQuantState.theHarmonyParms.last[j];
				}
			}
		}
		
		if (valid_current_circle_degree)
		{
			harmonyGatePulse.reset();  // kill the pulse in case it is active
	    	harmonyGatePulse.trigger(0.1);  // use a shorter gate so chord can be manually retriggerd quicker
		}
	}

	
	
   
	ModeScaleQuantLFOGenerator LFOclock;

	// PULSES FOR TRIGGER OUTPUTS INSTEAD OF GATES
	dsp::PulseGenerator clockPulse32ts;
	bool pulse32ts = false;
	dsp::PulseGenerator clockPulse16ts;
	bool pulse16ts = false;
	dsp::PulseGenerator clockPulse8ts;
	bool pulse8ts = false; 
	dsp::PulseGenerator clockPulse4ts;
	bool pulse4ts = false;
	dsp::PulseGenerator clockPulse2ts;
	bool pulse2ts = false;
	dsp::PulseGenerator clockPulse1ts;
	bool pulse1ts = false;

	dsp::PulseGenerator quantTriggerPulse;
	bool pulseQuant = false;

	float trigger_length = 0.0001f;
	
	const float lightLambda = 0.075f;

	bool running = true;
		
	int bar_count = 0;  // number of bars running count
	
	int i16ts_count = 0;  // counted 32s notes per sixteenth note
	int i8ts_count = 0;  // counted 32s notes per eighth note
	int i4ts_count = 0; // counted 32s notes per quarter note
	int i2ts_count = 0; // counted 32s notes per half note
	int barts_count = 0;     // counted 32s notes per bar

	float tempo =120.0f;
	float fintempo =120.0f;
	float frequency = 2.0f;

	
	int i16ts_count_limit = 2;// 32s notes per sixteenth note
	int i8ts_count_limit = 4;   // 32s notes per eighth note
	int i4ts_count_limit = 8;  // 32s notes per quarter note
	int i2ts_count_limit =16;  // 32s notes per half note
	int barts_count_limit = 32;     // 32s notes per bar
	
	float min_bpm = 3.75f;  // corresponds to CV=-5
	float max_bpm = 960.0f; // corresponds to CV=3.0

	float extHarmonyIn=-99;
	float extMelodyIn=-99;
  

	// end of clock **************************

	dsp::ClockDivider lowFreqClock;
	dsp::ClockDivider sec1Clock;
	dsp::ClockDivider lightDivider;
	
	float phase = 0.f;
 
	int circle_step_index=0;
		
	dsp::SchmittTrigger CircleStepToggles[MAX_STEPS];

	bool CircleStepStates[MAX_STEPS]={};

	rack::dsp::PulseGenerator barTriggerPulse; 
	rack::dsp::PulseGenerator harmonyGatePulse; 
	rack::dsp::PulseGenerator melodyGatePulse; 
	rack::dsp::PulseGenerator bassGatePulse; 
	rack::dsp::PulseGenerator barGaterPulse; 
		
	int lastPlayedCircleDegree=1;
	int lastPlayedCircleOctave=0;
	int lastPlayedCirclePosition=1;

	rack::dsp::PulseGenerator extPolyQuantTriggerPulse[16]; 
		
	void onRandomize(const RandomizeEvent& e) override {
					
		// Call super method if you wish to include default behavior
	    Module::onRandomize(e);
	}

    // save button states
	json_t *dataToJson() override {
		json_t *rootJ = json_object();

		json_object_set_new(rootJ, "scale_out_mode", json_integer(scale_out_mode));

		json_object_set_new(rootJ, "paneltheme", json_integer(MSQ_panelTheme));
		json_object_set_new(rootJ, "panelcontrast", json_real(MSQ_panelContrast));

		// new in 2.0.29
		json_object_set_new(rootJ, "modalmode", json_real(mode));
		json_object_set_new(rootJ, "modaroot", json_real(root_key));

       
		return rootJ;
	}

	void dataFromJson(json_t *rootJ) override {

         json_t *modeJ = json_object_get(rootJ, "scale_out_mode");
        if(modeJ) scale_out_mode = (ScaleOutMode) json_integer_value(modeJ);

		json_t *panelthemeJ = json_object_get(rootJ, "paneltheme");
        if (panelthemeJ)MSQ_panelTheme = json_integer_value(panelthemeJ);
	
		json_t *panelcontrastJ = json_object_get(rootJ, "panelcontrast");
        if (panelcontrastJ) MSQ_panelContrast = json_real_value(panelcontrastJ);

		// new in 2.0.29
		json_t *modalmodeJ = json_object_get(rootJ, "modalmode");
        if (modalmodeJ) mode = json_real_value(modalmodeJ);

		json_t *modalrootJ = json_object_get(rootJ, "modalroot");
        if (modalrootJ) root_key = json_real_value(modalrootJ);
	 
		circleChanged=true;

	}

	    	
	void process(const ProcessArgs &args) override 
	{
		if (!moduleVarsInitialized)
			return;
		
		// handle poly quant
		int poly_quant_channels = inputs[IN_POLY_QUANT_EXT_CV].getChannels();  // handle poly quantizer inport
		outputs[OUT_EXT_POLY_QUANT_OUTPUT].setChannels(poly_quant_channels);
		outputs[OUT_EXT_POLY_QUANT_TRIGGER_OUTPUT].setChannels(poly_quant_channels);
				
		for (int channel = 0; channel < poly_quant_channels; ++channel) 
		{
			float fnote = inputs[IN_POLY_QUANT_EXT_CV].getVoltage(channel);
			int searchSpace = std::floor( fnote * 24);
			int oct = eucDiv(searchSpace, 24);
			searchSpace -= oct * 24;
			int inote = polyQuant_searchSpaces[searchSpace] + oct * 12;
			polyQuant_outputNotes[eucMod(inote, 12)] = true;
			fnote = float(inote) / 12;
			outputs[OUT_EXT_POLY_QUANT_OUTPUT].setVoltage( fnote, channel);

			//  Set trigger pulse timers here
	
			if (fnote!=last_poly_quant_value[channel]) // if poly quant note changed 
			{
				 extPolyQuantTriggerPulse[channel].reset();  // kill the pulse in case it is active
			     last_poly_quant_value[channel]=fnote;
			     extPolyQuantTriggerPulse[channel].trigger(1e-3f);  // start the 1ms trigger puleses in duration of seconds
			}
		}
		//*************** 

		//Run
		
		int channel = 0;
		for (channel = 0; channel < poly_quant_channels; ++channel) 
		{
	    	if ( extPolyQuantTriggerPulse[channel].process(1.0 / args.sampleRate)) // end the trigger if pulse timer has expired 
		    {
		       outputs[OUT_EXT_POLY_QUANT_TRIGGER_OUTPUT].setVoltage(CV_MAX10, channel); 
	 	    }
	        else
		    {
		       outputs[OUT_EXT_POLY_QUANT_TRIGGER_OUTPUT].setVoltage(0, channel); 
		    }
		}
					
		// these should be done in initialization rather than every process() call
		LFOclock.setFreq(frequency*(32/time_sig_bottom));	  // for 32ts	should not hurt top call this each sample
		barts_count_limit = (32*time_sig_top/time_sig_bottom);
		//************************************************************************
						 
		LFOclock.step(1.0 / args.sampleRate);
							
	
		pulse1ts = clockPulse1ts.process(1.0 / args.sampleRate);
		pulse2ts = clockPulse2ts.process(1.0 / args.sampleRate);
		pulse4ts = clockPulse4ts.process(1.0 / args.sampleRate);
		pulse8ts = clockPulse8ts.process(1.0 / args.sampleRate);
		pulse16ts = clockPulse16ts.process(1.0 / args.sampleRate);
		pulse32ts = clockPulse32ts.process(1.0 / args.sampleRate);

		
	
		// end the gate if pulse timer has expired 
	
		if (harmonyGatePulse.process( 1.0 / APP->engine->getSampleRate()))
		{
			outputs[OUT_HARMONY_GATE_OUTPUT].setVoltage(CV_MAX10);
		}
		else
		{
			outputs[OUT_HARMONY_GATE_OUTPUT].setVoltage(0);
			theModeScaleQuantState.theHarmonyParms.last_chord_playing=false;
		}
		
		//***************
		
		for (int i=0; i<12; ++i) 
		{
			if (CircleStepToggles[i].process(params[BUTTON_CIRCLESTEP_C_PARAM+i].getValue()))  // circle button clicked
			{
				int current_circle_position=i;
								
				for (int j=0; j<12; ++j) 
				{
					if (j!=current_circle_position) 
					{
						CircleStepStates[j] = false;
						lights[LIGHT_LEDBUTTON_CIRCLESTEP_1+j].setBrightness(CircleStepStates[j] ? 1.0f : 0.0f);
					}
				}

				if (!CircleStepStates[current_circle_position])
					CircleStepStates[current_circle_position] = true;
			
				lights[LIGHT_LEDBUTTON_CIRCLESTEP_1+current_circle_position].setBrightness(CircleStepStates[current_circle_position] ? 1.0f : 0.0f);	
			
				userPlaysCirclePositionHarmony(current_circle_position, theModeScaleQuantState.theHarmonyParms.target_octave); 
										
				theModeScaleQuantState.userControllingHarmonyFromCircle=true;
				theModeScaleQuantState.theHarmonyParms.enabled=false;
						
				//find this in circle
				
				for (int j=0; j<7; ++j) 
				{
					if  (theCircleOf5ths.theDegreeSemiCircle.degreeElements[j].CircleIndex==current_circle_position)
					{
						int theDegree=theCircleOf5ths.theDegreeSemiCircle.degreeElements[j].Degree;
						if ((theDegree<1)||(theDegree>7))
		  					theDegree=1;  // force to a valid degree to avoid a crash
						if ((theDegree>=1)&&(theDegree<=7))
						{
							theModeScaleQuantState.circleDegree=theDegree; 
						}
						break;
					}
				} 
			}
		}

		
		float fvalue=0;
  

		//**************************
		if (lightDivider.process())
		{
		}
			
		if (lowFreqClock.process())
		{
			// check controls for changes
					
			frequency = tempo/60.0f;  // BPS
			
		   	if ((fvalue=std::round(params[CONTROL_ROOT_KEY_PARAM].getValue()))!=circle_root_key)
			{
				circle_root_key=(int)fvalue;
				root_key=circle_of_fifths[circle_root_key];
				for (int i=0; i<12; ++i)
					lights[LIGHT_CIRCLE_ROOT_KEY_POSITION_1_LIGHT+i].setBrightness(0.0f);
				lights[LIGHT_CIRCLE_ROOT_KEY_POSITION_1_LIGHT+circle_root_key].setBrightness(1.0f);
				circleChanged=true;
			}
			outputs[OUT_EXT_ROOT_OUTPUT].setVoltage(root_key/12.0); // do it always so it is sent after load. But, this is in if (lowFreqClock.process()){}
		
			if ((fvalue=std::round(params[CONTROL_SCALE_PARAM].getValue()))!=mode)
			{
				mode = fvalue;
				if (true) // modern modal music behavior.  Each mode defaults to a white key scale.  The mode root can be transposed manually.
				{ 
					switch (mode)
					{
                         
                        case 0:  // Lydian
					     root_key=5; // F
						 params[CONTROL_ROOT_KEY_PARAM].setValue(11.0);
					     break;
					   case 1:  //Ionian
					     root_key=0; // C
						 params[CONTROL_ROOT_KEY_PARAM].setValue(0.0);
					     break;
					   case 2:  //Mixolydian
					     root_key=7; // G
						 params[CONTROL_ROOT_KEY_PARAM].setValue(1.0);
					     break;
					   case 3:  //Dorian
					     root_key=2; // D
						 params[CONTROL_ROOT_KEY_PARAM].setValue(2.0);
					     break;
					   case 4:  //Aeolian
					     root_key=9; // A
						 params[CONTROL_ROOT_KEY_PARAM].setValue(3.0);
					     break;
					   case 5:  //Phrygian
					     root_key=4; // E
						 params[CONTROL_ROOT_KEY_PARAM].setValue(4.0);
					     break;
					   case 6:  //Locrian
					     root_key=11; // B
						 params[CONTROL_ROOT_KEY_PARAM].setValue(5.0);
					     break;
					};
				}
				circleChanged=true;
			}
			outputs[OUT_EXT_SCALE_OUTPUT].setVoltage(mode);  // do it always so it is sent after load  But, this is in if (lowFreqClock.process()){}
			

			// check input ports for change

			for (int i=0; i<ModeScaleQuant::NUM_INPUTS; ++i)
			{
				if (inputs[i].isConnected())
				{
					float fvalue=inputs[i].getVoltage();
					
					if ((i==IN_ROOT_KEY_EXT_CV)||(i==IN_SCALE_EXT_CV)||(fvalue!=inportStates[i].lastValue))  // don't do anything unless input changed or certain inputs
					{
						if ((i!=IN_ROOT_KEY_EXT_CV)&&(i!=IN_SCALE_EXT_CV))
							inportStates[i].lastValue=fvalue;
						switch (i)
						{
							// process misc input ports
							
						    // handle mode and root input changes
						  
						    case IN_ROOT_KEY_EXT_CV: 
							if (!theModeScaleQuantState.RootInputSuppliedByRootOutput)  // ModeScaleQuant root key input is NOT fed by ModeScaleQuant root key output
								{
									if (fvalue>=.01)
										{
											float ratio=(fvalue/10.0);
											int newValue=(int)(ratio*11);
											newValue=clamp(newValue, 0, 11);
											if (newValue!=circle_root_key)
											{
												circle_root_key=(int)newValue;
												root_key=circle_of_fifths[circle_root_key];
												params[CONTROL_ROOT_KEY_PARAM].setValue(circle_root_key);
												for (int i=0; i<12; ++i)
													lights[LIGHT_CIRCLE_ROOT_KEY_POSITION_1_LIGHT+i].setBrightness(0.0f);
												lights[LIGHT_CIRCLE_ROOT_KEY_POSITION_1_LIGHT+circle_root_key].setBrightness(1.0f);
												circleChanged=true;
											}
										}
								}
								else
								{
									int newValue=0;
									
									if ((std::abs(fvalue-0.583f)<.01f)) 
										newValue=1;
									else
									if ((std::abs(fvalue-0.167f)<.01f)) 
										newValue=2;
									else
									if ((std::abs(fvalue-0.750f)<.01f)) 
										newValue=3;
									else
									if ((std::abs(fvalue-0.333f)<.01f)) 
										newValue=4;
									else
									if ((std::abs(fvalue-0.917f)<.01f)) 
										newValue=5;
									else
									if ((std::abs(fvalue-0.500f)<.01f)) 
										newValue=6;
									else
									if ((std::abs(fvalue-0.083f)<.01f)) 
										newValue=7;
									else
									if ((std::abs(fvalue-0.667f)<.01f)) 
										newValue=8;
									else
									if ((std::abs(fvalue-0.250f)<.01f)) 
										newValue=9;
									else
									if ((std::abs(fvalue-0.833f)<.01f)) 
										newValue=10;
									else
									if ((std::abs(fvalue-0.417f)<.01f)) 
										newValue=11;
									else
									  newValue=0;
									
									newValue=clamp(newValue, 0, 11);

									if (newValue!=circle_root_key)
									{
										circle_root_key=(int)newValue;
										root_key=circle_of_fifths[circle_root_key];
										params[CONTROL_ROOT_KEY_PARAM].setValue(circle_root_key);
										for (int i=0; i<12; ++i)
											lights[LIGHT_CIRCLE_ROOT_KEY_POSITION_1_LIGHT+i].setBrightness(0.0f);
										lights[LIGHT_CIRCLE_ROOT_KEY_POSITION_1_LIGHT+circle_root_key].setBrightness(1.0f);
										circleChanged=true;
									}
								}
					        	break;

							case IN_SCALE_EXT_CV:
								if (!theModeScaleQuantState.ModeInputSuppliedByModeOutput)  // ModeScaleQuant mode scale input is NOT fed by ModeScaleQuant mode scale  key output
								{   
									float ratio=(fvalue/10.0);
									int newValue=(int)(ratio*6);
									newValue=clamp(newValue, 0, 6);
									if (newValue!=mode)
									{
										mode=(int)newValue;
									//	mode=std::round(newValue);
										params[CONTROL_SCALE_PARAM].setValue(mode);
										if (true) // modern modal music behavior.  Each mode defaults to a white key scale.  The mode root can be transposed manually.
										{
											switch (mode)
											{
										   		case 0:  // Lydian
													root_key=5; // F
													params[CONTROL_ROOT_KEY_PARAM].setValue(11.0);
													break;
												case 1:  //Ionian
													root_key=0; // C
													params[CONTROL_ROOT_KEY_PARAM].setValue(0.0);
													break;
												case 2:  //Mixolydian
													root_key=7; // G
													params[CONTROL_ROOT_KEY_PARAM].setValue(1.0);
													break;
												case 3:  //Dorian
													root_key=2; // D
													params[CONTROL_ROOT_KEY_PARAM].setValue(2.0);
													break;
												case 4:  //Aeolian
													root_key=9; // A
													params[CONTROL_ROOT_KEY_PARAM].setValue(3.0);
													break;
												case 5:  //Phrygian
													root_key=4; // E
													params[CONTROL_ROOT_KEY_PARAM].setValue(4.0);
													break;
												case 6:  //Locrian
													root_key=11; // B
													params[CONTROL_ROOT_KEY_PARAM].setValue(5.0);
													break;
											};  // end switch
										}  // end if (true)
										circleChanged=true;
									}
								}
								else  // ModeScaleQuant mode scale input IS fed by ModeScaleQuant mode scale  key output
								{
									int newValue=(int)fvalue;
									newValue=clamp(newValue, 0, 6);
									if (newValue!=mode)
									{
										mode=(int)newValue;
										params[CONTROL_SCALE_PARAM].setValue(mode);
										if (true) // modern modal music behavior.  Each mode defaults to a white key scale.  The mode root can be transposed manually.
										{
											switch (mode)
											{
												case 0:  // Lydian
												root_key=5; // F
												params[CONTROL_ROOT_KEY_PARAM].setValue(11.0);
												break;
												case 1:  //Ionian
													root_key=0; // C
													params[CONTROL_ROOT_KEY_PARAM].setValue(0.0);
													break;
												case 2:  //Mixolydian
													root_key=7; // G
													params[CONTROL_ROOT_KEY_PARAM].setValue(1.0);
													break;
												case 3:  //Dorian
													root_key=2; // D
													params[CONTROL_ROOT_KEY_PARAM].setValue(2.0);
													break;
												case 4:  //Aeolian
													root_key=9; // A
													params[CONTROL_ROOT_KEY_PARAM].setValue(3.0);
													break;
												case 5:  //Phrygian
													root_key=4; // E
													params[CONTROL_ROOT_KEY_PARAM].setValue(4.0);
													break;
												case 6:  //Locrian
													root_key=11; // B
													params[CONTROL_ROOT_KEY_PARAM].setValue(5.0);
													break;
											};  // end switch
										}  // end if (true)
										circleChanged=true;
									}
									
								}

								outputs[OUT_EXT_SCALE_OUTPUT].setVoltage(mode);
					        	break;

						};  // end switch
					}
				}
			}
		
			// harmony params
       
			// reconstruct initially and when dirty
			if (circleChanged)  
			{	
				notate_mode_as_signature_root_key=((root_key-(mode_natural_roots[mode_root_key_signature_offset[mode]]))+12)%12;
							
				if ((notate_mode_as_signature_root_key==1)   // Db
				  ||(notate_mode_as_signature_root_key==3)   // Eb
				  ||(notate_mode_as_signature_root_key==5)   // F
				  ||(notate_mode_as_signature_root_key==8)   // Ab
				  ||(notate_mode_as_signature_root_key==10)) // Bb
				{
					for (int i=0; i<12; ++i)
						strcpy(note_desig[i], MSQ_note_desig_flats[i]);
				}
				else
				{
					for (int i=0; i<12; ++i)
						strcpy(note_desig[i], MSQ_note_desig_sharps[i]);
				} 
				
				ConstructCircle5ths(circle_root_key, mode);
				ConstructDegreesSemicircle(circle_root_key, mode); //int circleroot_key, int mode)
			
				init_notes();  // depends on mode and root_key		
											
				circleChanged=false;
			
				onResetScale();
				onResetQuantizer();
		
			}
		
		}	

		if (sec1Clock.process())
		{
		}
		
		 	     
	}  // end module process()

	~ModeScaleQuant() 
	{
	}
  
 
	ModeScaleQuant() 
	{
				
		lowFreqClock.setDivision(512);  // every 86 samples, 2ms
		sec1Clock.setDivision(44000);
		lightDivider.setDivision(512);  // every 86 samples, 2ms
					
		ConfigureModuleVars();
		ModeScaleQuantMusicStructuresInitialize();  // sets module moduleVarsInitialized=true

			
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

		for (int i=0; i<12; ++i)
			lights[LIGHT_CIRCLE_ROOT_KEY_POSITION_1_LIGHT+i].setBrightness(0.0f);
		lights[LIGHT_CIRCLE_ROOT_KEY_POSITION_1_LIGHT+root_key].setBrightness(1.0f);  // loaded root_key might not be 0/C
		
		CircleStepStates[0]=1.0f;
		lights[LIGHT_LEDBUTTON_CIRCLESTEP_1].setBrightness(1.0f);
		
	//****************** 
	
		configInput(IN_ROOT_KEY_EXT_CV, "Mono CV Ext. Root Set: 0.1v-10v=C,G,D,A,E,B,F#,Db,Ab,Eb,Bb,F :");
		configInput(IN_SCALE_EXT_CV, "Mono CV Ext. Mode Set: 0.1v-10v=Lydian,Ionian,Mixolydian,Dorian,Aeolian,Phrygian,Locrian :");
		configInput(IN_POLY_QUANT_EXT_CV, "Poly External Note(s) To Quantize In : v/oct");

//**************** 
   
		configOutput(OUT_HARMONY_CV_OUTPUT, "Poly Harmony Chord Notes Out: v/oct :");
		configOutput(OUT_EXT_POLY_SCALE_OUTPUT, "Poly Scale Out: v/oct Out: 7, 5 or 12 notes");
		configOutput(OUT_EXT_POLY_QUANT_OUTPUT, "Poly  External Note(s) Quantized Out : v/oct :");
		configOutput(OUT_EXT_ROOT_OUTPUT, "Mono Scale Root Note Out: v/oct :");
		configOutput(OUT_EXT_POLY_QUANT_TRIGGER_OUTPUT, "Poly quantized out :");
	
//**************** 
								
    	configParam(CONTROL_ROOT_KEY_PARAM, 0, 11, 0.f, "Root/Key");
		getParamQuantity(CONTROL_ROOT_KEY_PARAM)->randomizeEnabled=false;
		configParam(CONTROL_SCALE_PARAM, 0.f, MSQ_num_modes-1, 1.f, "Mode");
		getParamQuantity(CONTROL_SCALE_PARAM)->randomizeEnabled=false;
		
		configButton(BUTTON_CIRCLESTEP_C_PARAM,  "C");
		configButton(BUTTON_CIRCLESTEP_G_PARAM,  "G");
		configButton(BUTTON_CIRCLESTEP_D_PARAM,  "D");
		configButton(BUTTON_CIRCLESTEP_A_PARAM,  "A");
		configButton(BUTTON_CIRCLESTEP_E_PARAM,  "E");
		configButton(BUTTON_CIRCLESTEP_B_PARAM,  "B");
		configButton(BUTTON_CIRCLESTEP_GBFS_PARAM,  "Gb/F#");
		configButton(BUTTON_CIRCLESTEP_DB_PARAM,  "Db");
		configButton(BUTTON_CIRCLESTEP_AB_PARAM,  "Ab");
		configButton(BUTTON_CIRCLESTEP_EB_PARAM,  "Eb");
		configButton(BUTTON_CIRCLESTEP_BB_PARAM,  "Bb");
		configButton(BUTTON_CIRCLESTEP_F_PARAM,  "F");

	
		onResetScale();
		onResetQuantizer(); 

	}  // end ModeScaleQuant()
	
};  // end of struct ModeScaleQuant

struct MinMaxQuantity : Quantity { 
	float *contrast = NULL;
	std::string label = "Contrast";

	MinMaxQuantity(float *_contrast, std::string _label) {
		contrast = _contrast;
		label = _label;
	}
	void setValue(float value) override {
		*contrast = math::clamp(value, getMinValue(), getMaxValue());
	}
	float getValue() override {
		return *contrast;
	}

	float getMinValue() override { return 0.50f; }
	float getMaxValue() override { return 1.0f; }
	float getDefaultValue() override {return MSQ_panelContrastDefault;}
	std::string getLabel() override { return label; }
	std::string getUnit() override { return " "; }
};    


struct ModeScaleQuantPanelThemeItem : MenuItem {    
    
		ModeScaleQuant  *module;
        int theme;
 
        void onAction(const event::Action &e) override {
         	MSQ_panelTheme = theme;  
		
        };

        void step() override {
        	rightText = (MSQ_panelTheme == theme)? "✔" : "";
        };
    
	};

struct MinMaxSliderItem : ui::Slider {
		MinMaxSliderItem(float *value, std::string label) {
			quantity = new MinMaxQuantity(value, label);
		}
		~MinMaxSliderItem() {
			delete quantity;
		}
    };

struct ModeScaleQuantScaleOutModeItem : MenuItem {   
    
        ModeScaleQuant *module;
        ModeScaleQuant::ScaleOutMode mode;

 
        void onAction(const event::Action &e) override {
                module->scale_out_mode = mode;
				module->onResetScale();
        };

        void step() override {
                rightText = (module->scale_out_mode == mode)? "✔" : "";
        };
    
	};
	
 
struct ModeScaleQuantRootKeySelectLineDisplay : LightWidget {

	ModeScaleQuant *module=NULL;
	int *val = NULL;
	
	ModeScaleQuantRootKeySelectLineDisplay() {
	
	} 

	void draw(const DrawArgs &args) override {

		if (!module)
			return; 
		
		std::shared_ptr<Font> font = APP->window->loadFont(asset::system("res/fonts/Nunito-Bold.ttf"));  // load a Rack font: an sans serif bold
							
		Vec textpos = Vec(19,11); 
		
		// Background
		NVGcolor backgroundColor = nvgRGB(0x0, 0x0, 0x0);
		NVGcolor borderColor = nvgRGB(0x10, 0x10, 0x10);
		nvgBeginPath(args.vg);
		nvgRoundedRect(args.vg, 0.0, 0.0, box.size.x, box.size.y, 4.0);
		nvgFillColor(args.vg, backgroundColor);
		nvgFill(args.vg);
		nvgStrokeWidth(args.vg, 1.0);
		nvgStrokeColor(args.vg, borderColor);
		nvgStroke(args.vg);
	
		nvgFontSize(args.vg,20 );
		if (font)
			nvgFontFaceId(args.vg, font->handle);
		nvgTextLetterSpacing(args.vg, -1);
		nvgTextAlign(args.vg,NVG_ALIGN_CENTER|NVG_ALIGN_MIDDLE);
		nvgFillColor(args.vg, MSQ_paramTextColor);
		nvgStrokeWidth(args.vg, 3.0);

		char text[128];
		
		snprintf(text, sizeof(text), "%s", MSQ_root_key_names[*val]);
		nvgText(args.vg, textpos.x, textpos.y, text, NULL);
				
		nvgClosePath(args.vg);
	}

};

struct ModeScaleQuantScaleSelectLineDisplay : LightWidget {

	ModeScaleQuant *module=NULL;
	int *val = NULL;
	
	ModeScaleQuantScaleSelectLineDisplay() {
       
	}

	void draw(const DrawArgs &args) override { 

		if (!module)
			return;

		

		std::shared_ptr<Font> font = APP->window->loadFont(asset::system("res/fonts/Nunito-Bold.ttf"));  // load a Rack font: a sans-serif bold
		
		Vec textpos = Vec(68,12); 
		
		// Background
		NVGcolor backgroundColor = nvgRGB(0x0, 0x0, 0x0);
		NVGcolor borderColor = nvgRGB(0x10, 0x10, 0x10);
		nvgBeginPath(args.vg);
		nvgRoundedRect(args.vg, 0.0, 0.0, box.size.x, box.size.y, 4.0);
	
		nvgFillColor(args.vg, backgroundColor);
		nvgFill(args.vg);
		nvgStrokeWidth(args.vg, 1.0);
		nvgStrokeColor(args.vg, borderColor);
		nvgStroke(args.vg);
	 
	
		nvgFontSize(args.vg, 20);
		if (font)
		nvgFontFaceId(args.vg, font->handle);
		nvgTextLetterSpacing(args.vg, -1);
		nvgTextAlign(args.vg,NVG_ALIGN_CENTER|NVG_ALIGN_MIDDLE);
		nvgFillColor(args.vg, MSQ_paramTextColor);  
	
		if (module)  
		{
			char text[128];
			
			snprintf(text, sizeof(text), "%s", MSQ_mode_names[*val]);
			nvgText(args.vg, textpos.x, textpos.y, text, NULL);

			// add on the scale notes display out of this box
			nvgFontSize(args.vg, 16);
			nvgFillColor(args.vg, MSQ_panelTextColor);
			strcpy(text,"");
			for (int i=0;i<MSQ_mode_step_intervals[*val][0];++i)
			{
				strcat(text,module->note_desig[module->notes[i]%MAX_NOTES]);  
				strcat(text," ");
			}
			
			nvgText(args.vg, textpos.x, textpos.y+20, text, NULL);
			strcpy(module->MSQscaleText, text);  // save for module instance use
		}
		
		nvgClosePath(args.vg);	
	} 

};

////////////////////////////////////
struct ModeScaleQuantBpmDisplayWidget : LightWidget {

  ModeScaleQuant *module=NULL;	
  float *val = NULL;

 
};


 
struct ModeScaleQuantWidget : ModuleWidget  
{
	SvgPanel* svgPanel;
	SvgPanel* darkPanel;
	
	rack::math::Rect  ParameterRect[MAX_PARAMS];  // warning, don't exceed the dimension
    rack::math::Rect  InportRect[MAX_INPORTS];  // warning, don't exceed the dimension
    rack::math::Rect  OutportRect[MAX_OUTPORTS];  // warning, don't exceed the dimension
 
	ParamWidget* paramWidgets[ModeScaleQuant::NUM_PARAMS]={0};  // keep track of all ParamWidgets as they are created so they can be moved around later  by the enum parmam ID
	LightWidget* lightWidgets[ModeScaleQuant::NUM_LIGHTS]={0};  // keep track of all LightWidgets as they are created so they can be moved around later  by the enum parmam ID

	PortWidget* outPortWidgets[ModeScaleQuant::NUM_OUTPUTS]={0};  // keep track of all output TPortWidgets as they are created so they can be moved around later  by the enum parmam ID
	PortWidget* inPortWidgets[ModeScaleQuant::NUM_INPUTS]={0};  // keep track of all output TPortWidgets as they are created so they can be moved around later  by the enum parmam ID
	
	struct CircleOf5thsDisplay : TransparentWidget 
	{
		ModeScaleQuant* module;
		rack::math::Rect*  ParameterRectLocal;   // warning, don't exceed the dimension
		rack::math::Rect*  InportRectLocal; 	 // warning, don't exceed the dimension
		rack::math::Rect*  OutportRectLocal;     // warning, don't exceed the dimension
			
		CircleOf5thsDisplay(ModeScaleQuant* module)  
		{
		   	this->module = module;  //  most plugins do not do this.  It was introduced in singleton implementation
		}
 
		void DrawCircle5ths(const DrawArgs &args, int root_key) 
		{
			if (!module)
				return;

			std::shared_ptr<Font> textfont = APP->window->loadFont(asset::plugin(pluginInstance, "res/Ubuntu Condensed 400.ttf"));
							
			for (int i=0; i<MAX_CIRCLE_STATIONS; ++i)
			{
					// draw root_key annulus sector

					int relativeCirclePosition = ((i - module->circle_root_key + module->mode)+12) % MAX_CIRCLE_STATIONS;
					
					nvgBeginPath(args.vg);
					float opacity = 128;
			
					nvgStrokeColor(args.vg, nvgRGBA(0x0, 0x0, 0x0, 0xff));
					nvgStrokeWidth(args.vg, 2);
				
					if((relativeCirclePosition == 0)||(relativeCirclePosition == 1)||(relativeCirclePosition == 2)) 
						nvgFillColor(args.vg, nvgRGBA(0xff, 0x20, 0x20, (int)opacity));  // reddish
					else
					if((relativeCirclePosition == 3)||(relativeCirclePosition == 4)||(relativeCirclePosition == 5)) 
						nvgFillColor(args.vg, nvgRGBA(0x20, 0x20, 0xff, (int)opacity));  //bluish
					else
					if(relativeCirclePosition == 6)
						nvgFillColor(args.vg, nvgRGBA(0x20, 0xff, 0x20, (int)opacity));  // greenish
					else	
						nvgFillColor(args.vg, nvgRGBA(0x20, 0x20, 0x20, (int)opacity));  // grayish
				
					nvgArc(args.vg,module->theCircleOf5ths.CircleCenter.x,module->theCircleOf5ths.CircleCenter.y,module->theCircleOf5ths.MiddleCircleRadius,module->theCircleOf5ths.Circle5ths[i].startDegree,module->theCircleOf5ths.Circle5ths[i].endDegree,NVG_CW);
					nvgLineTo(args.vg,module->theCircleOf5ths.Circle5ths[i].pt3.x,module->theCircleOf5ths.Circle5ths[i].pt3.y);
					nvgArc(args.vg,module->theCircleOf5ths.CircleCenter.x,module->theCircleOf5ths.CircleCenter.y,module->theCircleOf5ths.InnerCircleRadius,module->theCircleOf5ths.Circle5ths[i].endDegree,module->theCircleOf5ths.Circle5ths[i].startDegree,NVG_CCW);
					nvgLineTo(args.vg,module->theCircleOf5ths.Circle5ths[i].pt2.x,module->theCircleOf5ths.Circle5ths[i].pt2.y);
								
					nvgFill(args.vg);
					nvgStroke(args.vg);
					
					nvgClosePath(args.vg);	
								
					// draw text
					nvgFontSize(args.vg, 12);
					if (textfont)
					nvgFontFaceId(args.vg, textfont->handle);	
					nvgTextLetterSpacing(args.vg, -1);
					nvgFillColor(args.vg, MSQ_panelTextColor);
					char text[32];
					snprintf(text, sizeof(text), "%s", MSQ_CircleNoteNames[i]);
					Vec TextPosition=module->theCircleOf5ths.CircleCenter.plus(module->theCircleOf5ths.Circle5ths[i].radialDirection.mult(module->theCircleOf5ths.MiddleCircleRadius*.93f));
					nvgTextAlign(args.vg,NVG_ALIGN_CENTER|NVG_ALIGN_MIDDLE);
					nvgText(args.vg, TextPosition.x, TextPosition.y, text, NULL);

			}		
		};

		void DrawDegreesSemicircle(const DrawArgs &args, int root_key) 
		{
			if (!module)
				return;

			std::shared_ptr<Font> textfont = APP->window->loadFont(asset::plugin(pluginInstance, "res/Ubuntu Condensed 400.ttf"));
								
			int chord_type=0; 

			for (int i=0; i<MAX_HARMONIC_DEGREES; ++i)
			{
				// draw degree annulus sector (yellow)

					nvgBeginPath(args.vg);
					float opacity = 128;

					nvgStrokeColor(args.vg, nvgRGBA(0x0, 0x0, 0x0, 0xff));
					nvgStrokeWidth(args.vg, 2);

					chord_type=module->theCircleOf5ths.theDegreeSemiCircle.degreeElements[i].chordType;
					nvgFillColor(args.vg, nvgRGBA(0xf9, 0xf9, 0x20, (int)opacity));  // yellowish
						
					nvgArc(args.vg,module->theCircleOf5ths.CircleCenter.x,module->theCircleOf5ths.CircleCenter.y,module->theCircleOf5ths.OuterCircleRadius,module->theCircleOf5ths.theDegreeSemiCircle.degreeElements[i].startDegree,module->theCircleOf5ths.theDegreeSemiCircle.degreeElements[i].endDegree,NVG_CW);
					nvgLineTo(args.vg,module->theCircleOf5ths.theDegreeSemiCircle.degreeElements[i].pt3.x,module->theCircleOf5ths.theDegreeSemiCircle.degreeElements[i].pt3.y);
					nvgArc(args.vg,module->theCircleOf5ths.CircleCenter.x,module->theCircleOf5ths.CircleCenter.y,module->theCircleOf5ths.MiddleCircleRadius,module->theCircleOf5ths.theDegreeSemiCircle.degreeElements[i].endDegree,module->theCircleOf5ths.theDegreeSemiCircle.degreeElements[i].startDegree,NVG_CCW);
					nvgLineTo(args.vg,module->theCircleOf5ths.theDegreeSemiCircle.degreeElements[i].pt2.x,module->theCircleOf5ths.theDegreeSemiCircle.degreeElements[i].pt2.y);
					
					nvgFill(args.vg);
					nvgStroke(args.vg);
					
					nvgClosePath(args.vg);	
								
					// draw text
					nvgFontSize(args.vg, 10);
					if (textfont)
					nvgFontFaceId(args.vg, textfont->handle);	
					nvgTextLetterSpacing(args.vg, -1); // as close as possible
					nvgFillColor(args.vg, nvgRGBA(0x00, 0x00, 0x00, 0xff));
					char text[32];
				
					chord_type=module->theCircleOf5ths.theDegreeSemiCircle.degreeElements[i].chordType;
					
					if (chord_type==0) // major
						snprintf(text, sizeof(text), "%s", MSQ_circle_of_fifths_degrees_UC[(i - module->theCircleOf5ths.theDegreeSemiCircle.RootKeyCircle5thsPosition+7)%7]);
					else
					if ((chord_type==1)||(chord_type==6)) // minor or diminished
						snprintf(text, sizeof(text), "%s", MSQ_circle_of_fifths_degrees_LC[(i - module->theCircleOf5ths.theDegreeSemiCircle.RootKeyCircle5thsPosition+7)%7]);
					
					
					Vec TextPosition=module->theCircleOf5ths.CircleCenter.plus(module->theCircleOf5ths.theDegreeSemiCircle.degreeElements[i].radialDirection.mult(module->theCircleOf5ths.OuterCircleRadius*.92f));
					nvgTextAlign(args.vg,NVG_ALIGN_CENTER|NVG_ALIGN_MIDDLE);
					nvgText(args.vg, TextPosition.x, TextPosition.y, text, NULL);
					if (i==6) // draw diminished
					{
						Vec TextPositionBdim=Vec(TextPosition.x+9, TextPosition.y-4);
						snprintf(text, sizeof(text), "o");
						nvgTextAlign(args.vg,NVG_ALIGN_CENTER|NVG_ALIGN_MIDDLE);
						nvgFontSize(args.vg, 8);
						nvgText(args.vg, TextPositionBdim.x, TextPositionBdim.y, text, NULL);
					}

			}	
			
		};

		Vec convertSVGtoNVG(float x, float y, float w, float h)
		{
			Vec nvgPos=mm2px(Vec(x+(w/2.0), (128.93-y-(h/2.0))));  // this writes in the center
			return nvgPos;
		}

		
		void drawLabelAbove(const DrawArgs &args, Rect rect, const char* label, float fontSize)  
		{
			std::shared_ptr<Font> textfont = APP->window->loadFont(asset::plugin(pluginInstance, "res/Ubuntu Condensed 400.ttf"));
								    	
			nvgBeginPath(args.vg);
			nvgFillColor(args.vg, MSQ_panelTextColor);
			nvgFontSize(args.vg, fontSize);
			if (textfont)
			nvgFontFaceId(args.vg, textfont->handle);
			nvgTextLetterSpacing(args.vg, -1);
			nvgTextAlign(args.vg,NVG_ALIGN_CENTER|NVG_ALIGN_MIDDLE);
			nvgText(args.vg, rect.pos.x+rect.size.x/2.,rect.pos.y-8, label, NULL);
		}

		void drawLabelRight(const DrawArgs &args, Rect rect, const char* label)  
		{
			std::shared_ptr<Font> textfont = APP->window->loadFont(asset::plugin(pluginInstance, "res/Ubuntu Condensed 400.ttf"));
								    	
			nvgBeginPath(args.vg);
			nvgFillColor(args.vg, MSQ_panelTextColor);
			nvgFontSize(args.vg, 14);
			if (textfont)
			nvgFontFaceId(args.vg, textfont->handle);
			nvgTextLetterSpacing(args.vg, -1);
			nvgTextAlign(args.vg,NVG_ALIGN_LEFT|NVG_ALIGN_MIDDLE);
		//	nvgTextAlign(args.vg,NVG_ALIGN_RIGHT|NVG_ALIGN_MIDDLE);
			nvgText(args.vg, rect.pos.x+rect.size.x+2, rect.pos.y+rect.size.y/2., label, NULL);
		}

		void drawLabelLeft(const DrawArgs &args, Rect rect, const char* label, float xoffset)  
		{
			std::shared_ptr<Font> textfont = APP->window->loadFont(asset::plugin(pluginInstance, "res/Ubuntu Condensed 400.ttf"));
								    	
			nvgBeginPath(args.vg);
			nvgFillColor(args.vg, MSQ_panelTextColor);
			nvgFontSize(args.vg, 14);
			if (textfont)
			nvgFontFaceId(args.vg, textfont->handle);
			nvgTextLetterSpacing(args.vg, -1);
			nvgTextAlign(args.vg,NVG_ALIGN_LEFT|NVG_ALIGN_MIDDLE);
			nvgText(args.vg, rect.pos.x-rect.size.x-2+xoffset, rect.pos.y+rect.size.y/2., label, NULL);
		}

		void drawLabelOffset(const DrawArgs &args, Rect rect, const char* label, float xoffset, float yoffset)  
		{
			std::shared_ptr<Font> textfont = APP->window->loadFont(asset::plugin(pluginInstance, "res/Ubuntu Condensed 400.ttf"));
								    	
			nvgBeginPath(args.vg);
			nvgFillColor(args.vg, MSQ_panelTextColor);
			nvgFontSize(args.vg, 14);
			if (textfont)
			nvgFontFaceId(args.vg, textfont->handle);
			nvgTextLetterSpacing(args.vg, -1);
			nvgTextAlign(args.vg,NVG_ALIGN_LEFT|NVG_ALIGN_MIDDLE);
			nvgText(args.vg, rect.pos.x+xoffset, rect.pos.y+yoffset, label, NULL);
		}

		void drawOutport(const DrawArgs &args, Vec OutportPos, const char* label, float value, int valueDecimalPoints, float scale=1.0)  // scale is vertical only
		{
			std::shared_ptr<Font> textfont = APP->window->loadFont(asset::plugin(pluginInstance, "res/Ubuntu Condensed 400.ttf"));
								
			Vec displayRectPos= OutportPos.plus(Vec(-3, -scale*15));  // specific for 30x43 size
			nvgBeginPath(args.vg);
		    nvgRoundedRect(args.vg, displayRectPos.x,displayRectPos.y, 30.f, scale*43.f, 3.f);
			nvgFillColor(args.vg, nvgRGBA( 0x0,  0x0, 0x0, 0xff));
			nvgFill(args.vg);
			nvgFontSize(args.vg, 10);
			nvgFillColor(args.vg, MSQ_panelTextColor);
			if (textfont)
			nvgFontFaceId(args.vg, textfont->handle);
			nvgTextLetterSpacing(args.vg, -1);
			nvgFillColor(args.vg, nvgRGBA(0xff, 0xff, 0xff, 0xFF));
			nvgTextAlign(args.vg,NVG_ALIGN_CENTER|NVG_ALIGN_MIDDLE);
			if (scale==1.0)
				nvgText(args.vg, OutportPos.x+12, OutportPos.y-8, label, NULL);
			else
				nvgText(args.vg, OutportPos.x+12, OutportPos.y-6, label, NULL);  // move Out label a bit
		}

		void drawGrid(const DrawArgs &args)
		{
			
				float panelWidth=mm2px(406);
				float panelHeight=mm2px(129);
				int gridWidthDivisions=(int)(panelWidth/10.0);  // 112
				int gridHeightDivisions=(int)(panelHeight/10.0); // 38
				float gridWidth=10.0;
				float gridHeight=10.0;

				
				nvgStrokeColor(args.vg, nvgRGB(0, 0, 0));
				nvgStrokeWidth(args.vg, 1.0);
				
				for (int x=0; x<gridWidthDivisions; ++x)
				{
					if (x%10==0)
					{
						nvgStrokeWidth(args.vg, 2.0);
						nvgStrokeColor(args.vg, nvgRGBA(0, 0, 0xff, 0x80));
					}
					else
					{
						nvgStrokeWidth(args.vg, 1.0);
						nvgStrokeColor(args.vg, nvgRGBA(0xff, 0, 0, 0x80));
					}

					nvgBeginPath(args.vg);
					nvgMoveTo(args.vg, x*gridWidth, 0);
					nvgLineTo(args.vg, x*gridWidth,  gridHeightDivisions*gridHeight);
					nvgStroke(args.vg);
				}
				for (int y=0; y<gridHeightDivisions; ++y)
				{
					if (y%10==0)
					{
						nvgStrokeWidth(args.vg, 2.0);
						nvgStrokeColor(args.vg, nvgRGBA(0, 0, 0xff, 0x80));
					}
					else
					{
						nvgStrokeWidth(args.vg, 1.0);
						nvgStrokeColor(args.vg, nvgRGBA(0xff, 0, 0, 0x80));
					}

					nvgBeginPath(args.vg);
					nvgMoveTo(args.vg, 0, y*gridHeight);
					nvgLineTo(args.vg, gridWidthDivisions*gridWidth, y*gridHeight);
					nvgStroke(args.vg);
				}
		}

		void drawKey(const DrawArgs &args, int k, bool state, int source) // state true means key pressed. Source 0=generic, 1=harmony chord, 2=melody, 3=arp, 4= bass, 5=drone
		{
			if ((k<21)||(k>108)) 
			  return;

			NVGcolor whiteNoteOnColor;
			NVGcolor whiteNoteOffColor;
			NVGcolor blackNoteOnColor;
			NVGcolor blackNoteOffColor;

			whiteNoteOffColor=nvgRGBA( 0xff,  0xff, 0xff, 0xff);  // treat all scale notes the same, regardless of "source"
			blackNoteOffColor=nvgRGBA( 0x0,  0x0, 0x0, 0xff);
			if (source==0)
			{
				whiteNoteOnColor=MSQ_panelHarmonyKeyboardColor;
				blackNoteOnColor=MSQ_panelHarmonyKeyboardColor;
			}
			if (source==1)
			{
				whiteNoteOnColor=MSQ_panelMelodyKeyboardColor;
				blackNoteOnColor=MSQ_panelMelodyKeyboardColor;
			}
			if (source==2)
			{
				whiteNoteOnColor=MSQ_panelArpKeyboardColor;
				blackNoteOnColor=MSQ_panelArpKeyboardColor;
			}
			if (source==3)
			{
				whiteNoteOnColor=MSQ_panelBassKeyboardColor;
				blackNoteOnColor=MSQ_panelBassKeyboardColor;
			}
		
			int numWhiteKeys=52;
		
			float beginLeftEdge = 215.0;
			float begintopEdge = 345.0;
			float keyboardLength=240.0;
			float whiteKeySpacing=keyboardLength/numWhiteKeys;
			float whiteKeyWidth=0.80*whiteKeySpacing;
			float whiteKeyLength=keyboardLength/8.0;
			float blackKeyLength=(whiteKeyLength*0.66);
			float blackKeyWidth=(0.5*whiteKeyWidth);

			float lineWidth=0.80;
			
		    int octave=(int)((k-12)/12);
			float octaveLeftEdge=octave*7.0*whiteKeySpacing;
			float semitoneOffset=0;

			int semitone=k%12;
			bool isWholeNote=false;
						
			switch (semitone)
			{
				case 0:  //C
					isWholeNote=true;
					semitoneOffset=0;
				   break;
				case 1:  //C#
					semitoneOffset=1;
				  break;
				case 2:  //D
					isWholeNote=true;
					semitoneOffset=1;
				  break;
				case 3:  //D#
					semitoneOffset=2;
				  break;
				case 4:  //E
					isWholeNote=true;
					semitoneOffset=2;
				  break;
				case 5:  //F
					isWholeNote=true;
					semitoneOffset=3;
				  break;
				case 6:  //F#
					semitoneOffset=4;
				  break;
				case 7:  //G
					isWholeNote=true;
					semitoneOffset=4;
				  break;
				case 8:  //G#
					semitoneOffset=5;
				  break;
				case 9:  //A
					isWholeNote=true;
					semitoneOffset=5;
				  break;
				case 10:  //A#
					semitoneOffset=6;
				  break;
				case 11:  //B
					isWholeNote=true;
					semitoneOffset=6;
				  break;
				
			}
		
		
			if (beginLeftEdge > 0) 
			{
				if ((true)&&(k==108))  // draw white key with no notches
				{
					nvgBeginPath(args.vg);
					float whiteKeyBeginLeftEdge= beginLeftEdge + octaveLeftEdge + semitoneOffset*whiteKeySpacing;
					nvgMoveTo(args.vg, whiteKeyBeginLeftEdge, begintopEdge);
					nvgLineTo(args.vg, whiteKeyBeginLeftEdge, (begintopEdge+whiteKeyLength));  //1
					nvgLineTo(args.vg, whiteKeyBeginLeftEdge+whiteKeyWidth, (begintopEdge+whiteKeyLength));//2
					nvgLineTo(args.vg, whiteKeyBeginLeftEdge+whiteKeyWidth, begintopEdge);//3
					nvgLineTo(args.vg, whiteKeyBeginLeftEdge, begintopEdge);//4
					nvgClosePath(args.vg);  //4

					nvgStrokeColor(args.vg, nvgRGB(0, 0, 0));
					nvgStrokeWidth(args.vg, lineWidth);
					nvgStroke(args.vg);
				
					if (!state)
						nvgFillColor(args.vg, whiteNoteOffColor);
					else
						nvgFillColor(args.vg, whiteNoteOnColor);

					nvgFill(args.vg);
				}
				else
				if ((true)&&((k==21)||(semitone==0)||(semitone==5)))  // draw a white key with right notch
				{
					nvgBeginPath(args.vg);
					float whiteKeyBeginLeftEdge= beginLeftEdge + octaveLeftEdge + semitoneOffset*whiteKeySpacing;
					nvgMoveTo(args.vg, whiteKeyBeginLeftEdge, begintopEdge);
					nvgLineTo(args.vg, whiteKeyBeginLeftEdge, (begintopEdge+whiteKeyLength));  //1
					nvgLineTo(args.vg, whiteKeyBeginLeftEdge+whiteKeyWidth, (begintopEdge+whiteKeyLength));//2
					nvgLineTo(args.vg, whiteKeyBeginLeftEdge+whiteKeyWidth, begintopEdge+(1.05*blackKeyLength));//3
					nvgLineTo(args.vg, whiteKeyBeginLeftEdge+whiteKeyWidth-(blackKeyWidth/2.0), begintopEdge+(1.05*blackKeyLength));//4
					nvgLineTo(args.vg, whiteKeyBeginLeftEdge+whiteKeyWidth-(blackKeyWidth/2.0), begintopEdge);//5
					nvgLineTo(args.vg, whiteKeyBeginLeftEdge, begintopEdge);//5
					nvgClosePath(args.vg); //6
					nvgStrokeColor(args.vg, nvgRGB(0, 0, 0));
					nvgStrokeWidth(args.vg, lineWidth);
					nvgStroke(args.vg);
				
					if (!state)
						nvgFillColor(args.vg, whiteNoteOffColor);
					else
						nvgFillColor(args.vg, whiteNoteOnColor);

					nvgFill(args.vg);
				}
				else  
				if ((true)&&((semitone==4)||(semitone==11))) // draw a white key with left notch
				{
					nvgBeginPath(args.vg);
					float whiteKeyBeginLeftEdge= beginLeftEdge + octaveLeftEdge + semitoneOffset*whiteKeySpacing +(blackKeyWidth/2.0);
					nvgMoveTo(args.vg, whiteKeyBeginLeftEdge, begintopEdge);
					nvgLineTo(args.vg, whiteKeyBeginLeftEdge, begintopEdge+blackKeyLength);//1
					nvgLineTo(args.vg, whiteKeyBeginLeftEdge-(blackKeyWidth/2.0), begintopEdge+(1.05*blackKeyLength));//2
					nvgLineTo(args.vg, whiteKeyBeginLeftEdge-(blackKeyWidth/2.0), begintopEdge+whiteKeyLength);//3
					nvgLineTo(args.vg, whiteKeyBeginLeftEdge-(blackKeyWidth/2.0)+whiteKeyWidth, begintopEdge+whiteKeyLength);//4
					nvgLineTo(args.vg, whiteKeyBeginLeftEdge-(blackKeyWidth/2.0)+whiteKeyWidth, begintopEdge);//5
					nvgLineTo(args.vg, whiteKeyBeginLeftEdge, begintopEdge);//6
					nvgClosePath(args.vg);  //6

					nvgStrokeColor(args.vg, nvgRGB(0, 0, 0));
					nvgStrokeWidth(args.vg, lineWidth);
					nvgStroke(args.vg);
				
					if (!state)
						nvgFillColor(args.vg, whiteNoteOffColor);
					else
						nvgFillColor(args.vg, whiteNoteOnColor);

					nvgFill(args.vg);
				}
				else
				if ((true)&&((semitone==2)||(semitone==7)||(semitone==9)))  // draw a white key with left and right notch
				{
					nvgBeginPath(args.vg);
				
					// left notch 4 segments
					float whiteKeyBeginLeftEdge1= beginLeftEdge + octaveLeftEdge + semitoneOffset*whiteKeySpacing +(blackKeyWidth/2.0);
					nvgMoveTo(args.vg, whiteKeyBeginLeftEdge1, begintopEdge);
					nvgLineTo(args.vg, whiteKeyBeginLeftEdge1, begintopEdge+blackKeyLength);//1
					nvgLineTo(args.vg, whiteKeyBeginLeftEdge1-(blackKeyWidth/2.0), begintopEdge+(1.05*blackKeyLength));//2
					nvgLineTo(args.vg, whiteKeyBeginLeftEdge1-(blackKeyWidth/2.0), begintopEdge+whiteKeyLength);//3
					nvgLineTo(args.vg, whiteKeyBeginLeftEdge1-(blackKeyWidth/2.0)+whiteKeyWidth, begintopEdge+whiteKeyLength);//4
					
					// right notch 4 segments
					float whiteKeyBeginLeftEdge2= beginLeftEdge + octaveLeftEdge + semitoneOffset*whiteKeySpacing;
					nvgLineTo(args.vg, whiteKeyBeginLeftEdge2+whiteKeyWidth, begintopEdge+blackKeyLength);//5
					nvgLineTo(args.vg, whiteKeyBeginLeftEdge2+whiteKeyWidth-(blackKeyWidth/2.0), begintopEdge+(1.05*blackKeyLength));//6
					nvgLineTo(args.vg, whiteKeyBeginLeftEdge2+whiteKeyWidth-(blackKeyWidth/2.0), begintopEdge);//7
					nvgLineTo(args.vg, whiteKeyBeginLeftEdge1, begintopEdge);//8
					nvgClosePath(args.vg);  //8
										
					nvgStrokeColor(args.vg, nvgRGB(0, 0, 0));
					

					nvgStrokeWidth(args.vg, lineWidth);
					nvgStroke(args.vg);
			
					if (!state)
						nvgFillColor(args.vg, whiteNoteOffColor);
					else
						nvgFillColor(args.vg, whiteNoteOnColor);

					nvgFill(args.vg);
				}
				
				if ((true)&&(!isWholeNote)) // draw a black key
				{
					nvgBeginPath(args.vg);
					float blackKeyBeginLeftEdge= beginLeftEdge + octaveLeftEdge + (semitoneOffset*whiteKeySpacing) -blackKeyWidth ;
					nvgMoveTo(args.vg, blackKeyBeginLeftEdge, begintopEdge);
					nvgLineTo(args.vg, blackKeyBeginLeftEdge, (begintopEdge+blackKeyLength));  //1
					nvgLineTo(args.vg, blackKeyBeginLeftEdge+blackKeyWidth, (begintopEdge+blackKeyLength)); //2
					nvgLineTo(args.vg, blackKeyBeginLeftEdge+blackKeyWidth, begintopEdge);  //3
					nvgMoveTo(args.vg, blackKeyBeginLeftEdge, begintopEdge);//4
					nvgClosePath(args.vg);  //4
					nvgStrokeColor(args.vg, nvgRGB(0, 0, 0));
					nvgStrokeWidth(args.vg, lineWidth);
					nvgStroke(args.vg);
				
					if (!state)
						nvgFillColor(args.vg, blackNoteOffColor);
					else
						nvgFillColor(args.vg, blackNoteOnColor);

					nvgFill(args.vg);
				}
			}
		}
		
		void drawKeyboard(const DrawArgs &args) // draw a piano keyboard
		{
			if (!module)
				return;

			for (int k=21; k<109; ++k) 
				drawKey(args, k, false, 0);
        }		

		void updatePanel(const DrawArgs &args)
		{
			if (!module)
				return;

			if(module->randEnqueued)
			{
				APP->engine->randomizeModule(module);
				module->randEnqueued=false;
			}

			std::shared_ptr<Font> textfont = APP->window->loadFont(asset::plugin(pluginInstance, "res/Ubuntu Condensed 400.ttf"));
			std::shared_ptr<Font> musicfont = APP->window->loadFont(asset::plugin(pluginInstance, "res/Bravura.otf"));

			if (true)  // draw rounded corner rects  for input jacks border 
			{
				char labeltext[128];
							
				snprintf(labeltext, sizeof(labeltext), "%s", "Root");
			  	drawLabelRight(args,ParameterRectLocal[ModeScaleQuant::CONTROL_ROOT_KEY_PARAM], labeltext);
							
				snprintf(labeltext, sizeof(labeltext), "%s", "Mode: bright->to darkest");
				drawLabelRight(args,ParameterRectLocal[ModeScaleQuant::CONTROL_SCALE_PARAM], labeltext);
						
				snprintf(labeltext, sizeof(labeltext), "%s", "In");
				drawLabelOffset(args, InportRectLocal[ModeScaleQuant::IN_POLY_QUANT_EXT_CV], labeltext, +2., -12.); 
			
			} 

			if (true)  // draw rounded corner rects  for output jacks border   
			{
				char labeltext[128];
				snprintf(labeltext, sizeof(labeltext), "%s", "1V/Oct");
				drawOutport(args, OutportRectLocal[ModeScaleQuant::OUT_HARMONY_CV_OUTPUT].pos, labeltext, 0, 1);

				snprintf(labeltext, sizeof(labeltext), "%s", "Gate");
				drawOutport(args, OutportRectLocal[ModeScaleQuant::OUT_HARMONY_GATE_OUTPUT].pos, labeltext, 0, 1);

				snprintf(labeltext, sizeof(labeltext), "%s", "Manual Chord Out");
				drawLabelOffset(args, OutportRectLocal[ModeScaleQuant::OUT_HARMONY_CV_OUTPUT], labeltext, -5., -25.0);

			
				snprintf(labeltext, sizeof(labeltext), "%s", "Out");
				drawOutport(args, OutportRectLocal[ModeScaleQuant::OUT_EXT_ROOT_OUTPUT].pos, labeltext, 0, 1, 1);  

				//
				snprintf(labeltext, sizeof(labeltext), "%s", "Poly");
				drawLabelOffset(args, OutportRectLocal[ModeScaleQuant::OUT_EXT_POLY_SCALE_OUTPUT], labeltext, -26., 7.); 
				snprintf(labeltext, sizeof(labeltext), "%s", "Ext.->");
				drawLabelOffset(args, OutportRectLocal[ModeScaleQuant::OUT_EXT_POLY_SCALE_OUTPUT], labeltext, -26., +19.0); 
				snprintf(labeltext, sizeof(labeltext), "%s", "Scale");
				drawLabelOffset(args, OutportRectLocal[ModeScaleQuant::OUT_EXT_POLY_SCALE_OUTPUT], labeltext, -26., +31.0); 
				//
										
				snprintf(labeltext, sizeof(labeltext), "%s", "Out");
				drawOutport(args, OutportRectLocal[ModeScaleQuant::OUT_EXT_POLY_SCALE_OUTPUT].pos, labeltext, 0, 1);

				snprintf(labeltext, sizeof(labeltext), "%s", "Quants");
				drawOutport(args, OutportRectLocal[ModeScaleQuant::OUT_EXT_POLY_QUANT_OUTPUT].pos, labeltext, 0, 1);

				snprintf(labeltext, sizeof(labeltext), "%s", "Trigs");
				drawOutport(args, OutportRectLocal[ModeScaleQuant::OUT_EXT_POLY_QUANT_TRIGGER_OUTPUT].pos, labeltext, 0, 1);

				snprintf(labeltext, sizeof(labeltext), "%s", "|-----Poly Quantizer-----|");
				drawLabelOffset(args, InportRectLocal[ModeScaleQuant::IN_POLY_QUANT_EXT_CV], labeltext, -5., -29.);  
			
				snprintf(labeltext, sizeof(labeltext), "%s", "Out");
				drawOutport(args, OutportRectLocal[ModeScaleQuant::OUT_EXT_SCALE_OUTPUT].pos, labeltext, 0, 1);

							
			}

			
			Vec pos;
			char text[128];
			nvgFontSize(args.vg, 17);
			if (textfont)
			nvgFontFaceId(args.vg, textfont->handle);
			nvgTextLetterSpacing(args.vg, -1);
			nvgFillColor(args.vg, MSQ_panelTextColor);
			
			//************************
			// circle area 
			
			float beginEdge = 295; 
			float beginTop =115;
			float lineWidth=0.75; 
			float stafflineLength=100;
			float barLineVoffset=36.;
			float barLineVlength=60.;
			float yHalfLineSpacing=3.0f;
			int num_sharps1=0;
			int vertical_offset1=0;
			int num_flats1=0;
           
			if (true)  // draw treble and bass clef staves
			{
				// draw bar left vertical edge

				if (beginEdge > 0) {
					nvgBeginPath(args.vg);
					nvgMoveTo(args.vg, beginEdge, beginTop+barLineVoffset);
					nvgLineTo(args.vg, beginEdge, beginTop+(1.60*barLineVlength));
					nvgStrokeColor(args.vg, MSQ_panelLineColor);
					nvgStrokeWidth(args.vg, lineWidth);
					nvgStroke(args.vg);
				}
            
				// draw staff lines
				nvgBeginPath(args.vg);
			
				for (int staff = 36; staff <= 72; staff += 36) {
					for (int y = staff; y <= staff + 24; y += 6) { 
						nvgMoveTo(args.vg, beginEdge, beginTop+y);
						nvgLineTo(args.vg, beginEdge+stafflineLength, beginTop+y);
					}
				}  

				nvgStrokeColor(args.vg, MSQ_panelLineColor);
				nvgStrokeWidth(args.vg, lineWidth);
				nvgStroke(args.vg);

				nvgFontSize(args.vg, 90);
				if (musicfont)
				nvgFontFaceId(args.vg, musicfont->handle);
				nvgTextLetterSpacing(args.vg, -1);
				nvgFillColor(args.vg, MSQ_panelLineColor);
				pos=Vec(beginEdge+12, beginTop+54); 
				snprintf(text, sizeof(text), "%s", MSQ_gClef.c_str());  
				nvgText(args.vg, pos.x, pos.y, text, NULL);

				nvgFontSize(args.vg, 90);
				pos=Vec(beginEdge+12, beginTop+78.5); 
				snprintf(text, sizeof(text), "%s", MSQ_fClef.c_str());  
				nvgText(args.vg, pos.x, pos.y, text, NULL);
				
				nvgFontSize(args.vg, 90);
			
               				
				nvgTextAlign(args.vg,NVG_ALIGN_CENTER|NVG_ALIGN_MIDDLE);
				snprintf(text, sizeof(text), "%s", MSQ_sharp.c_str());  
				
				num_sharps1=0;
				vertical_offset1=0;
				for (int i=0; i<7; ++i)
				{
					nvgBeginPath(args.vg);
					if (MSQ_root_key_signatures_chromaticForder[module->notate_mode_as_signature_root_key][i]==1)
					{
						vertical_offset1=MSQ_root_key_sharps_vertical_display_offset[num_sharps1];
						pos=Vec(beginEdge+24+(num_sharps1*5), beginTop+33+(vertical_offset1*yHalfLineSpacing));
						nvgText(args.vg, pos.x, pos.y, text, NULL);
						++num_sharps1;
					}
					nvgClosePath(args.vg);
				}	
			
				snprintf(text, sizeof(text), "%s", MSQ_flat.c_str());  
				num_flats1=0;
				vertical_offset1=0;
				for (int i=6; i>=0; --i)  
				{
					nvgBeginPath(args.vg);
					if (MSQ_root_key_signatures_chromaticForder[module->notate_mode_as_signature_root_key][i]==-1)
					{
						vertical_offset1=MSQ_root_key_flats_vertical_display_offset[num_flats1];
						pos=Vec(beginEdge+24+(num_flats1*5), beginTop+33+(vertical_offset1*yHalfLineSpacing));
						nvgText(args.vg, pos.x, pos.y, text, NULL);
						++num_flats1;
					}
					nvgClosePath(args.vg);
				}	

				// now do for bass clef

				nvgFontSize(args.vg, 90);
				nvgTextAlign(args.vg,NVG_ALIGN_CENTER|NVG_ALIGN_MIDDLE);
				snprintf(text, sizeof(text), "%s", MSQ_sharp.c_str());  
				
				num_sharps1=0;
				vertical_offset1=0;
				for (int i=0; i<7; ++i)
				{
					nvgBeginPath(args.vg);
					if (MSQ_root_key_signatures_chromaticForder[module->notate_mode_as_signature_root_key][i]==1)
					{
						vertical_offset1=MSQ_root_key_sharps_vertical_display_offset[num_sharps1];
						pos=Vec(beginEdge+24+(num_sharps1*5), beginTop+75+(vertical_offset1*yHalfLineSpacing));
						nvgText(args.vg, pos.x, pos.y, text, NULL);
						++num_sharps1;
					}
					nvgClosePath(args.vg);
				}	
			
				snprintf(text, sizeof(text), "%s", MSQ_flat.c_str());  
				num_flats1=0;
				vertical_offset1=0;
				for (int i=6; i>=0; --i)
				{
					nvgBeginPath(args.vg);
					if (MSQ_root_key_signatures_chromaticForder[module->notate_mode_as_signature_root_key][i]==-1)
					{
						vertical_offset1=MSQ_root_key_flats_vertical_display_offset[num_flats1];
						pos=Vec(beginEdge+24+(num_flats1*5), beginTop+75+(vertical_offset1*yHalfLineSpacing));
						nvgText(args.vg, pos.x, pos.y, text, NULL);
						++num_flats1;
					}
					nvgClosePath(args.vg);
				}	

				// designate Middle C as C4
				nvgFontSize(args.vg, 10);
				if (textfont)
				nvgFontFaceId(args.vg, textfont->handle);
				nvgTextLetterSpacing(args.vg, -1);
				nvgFillColor(args.vg, MSQ_panelTextColor);
				float mid_C_position = 181.0;  // middle C
				pos=Vec(beginEdge-8, mid_C_position);  
				snprintf(text, sizeof(text), "%s", "C4");  
				nvgText(args.vg, pos.x, pos.y, text, NULL);

				// draw notes on staves
				float display_note_position=0; 
				char noteText[128];
		
		        
				if ((module->moduleVarsInitialized)&&(module->valid_current_circle_degree))  // only initialized if Module!=NULL
				{
					nvgFontSize(args.vg, 90);
					if (musicfont)
					nvgFontFaceId(args.vg, musicfont->handle);
					nvgTextLetterSpacing(args.vg, -1);
					nvgFillColor(args.vg,  MSQ_panelTextColor);

					for (int i=0; ((i<module->bar_note_count)&&(i<256)); ++i)
					{
						int display_note=module->played_notes_circular_buffer[i].note;
																
						int scale_note=0;
						if (strstr(module->note_desig[display_note%12],"C"))
							scale_note=0;
						else
						if (strstr(module->note_desig[display_note%12],"D"))
							scale_note=1;
						else
						if (strstr(module->note_desig[display_note%12],"E"))
							scale_note=2;
						else
						if (strstr(module->note_desig[display_note%12],"F"))
							scale_note=3;
						else
						if (strstr(module->note_desig[display_note%12],"G"))
							scale_note=4;
						else
						if (strstr(module->note_desig[display_note%12],"A"))
							scale_note=5;
						else
						if (strstr(module->note_desig[display_note%12],"B"))
							scale_note=6;
											
						int octave=(display_note/12)-2; 
											
						display_note_position = 108.0-(octave*21.0)-(scale_note*3.0);
											
						float note_x_spacing= 230.0/(32*module->time_sig_top/module->time_sig_bottom);  // experimenting with note spacing function of time_signature.  barts_count_limit is not in scope, needs to be global
						pos=Vec(beginEdge+80+(module->played_notes_circular_buffer[i].time32s*note_x_spacing), beginTop+display_note_position);   
						if (true)  // color code notes in staff rendering
						{ 
							if (module->played_notes_circular_buffer[i].noteType==NOTE_TYPE_CHORD)
								nvgFillColor(args.vg,  MSQ_panelHarmonyPartColor); 
							else
							if (module->played_notes_circular_buffer[i].noteType==NOTE_TYPE_MELODY)
								nvgFillColor(args.vg,  MSQ_panelMelodyPartColor); 
							else
							if (module->played_notes_circular_buffer[i].noteType==NOTE_TYPE_ARP)
								nvgFillColor(args.vg,  MSQ_panelArpPartColor); 
							else
							if (module->played_notes_circular_buffer[i].noteType==NOTE_TYPE_BASS)
								nvgFillColor(args.vg,  MSQ_panelBassPartColor); 
							
						}

						
						nvgFontSize(args.vg, 90);
						if (module->played_notes_circular_buffer[i].length==1)
						{
							snprintf(noteText, sizeof(noteText), "%s",  MSQ_noteWhole.c_str());  
						}
						else
						if (module->played_notes_circular_buffer[i].length==2)
						{
							if ((display_note<38)||((display_note>47)&&(display_note<59)))
							{
								snprintf(noteText, sizeof(noteText), "%s",  MSQ_noteHalfUp.c_str()); 
								pos.x+=1.0;  // move up stem notes a bit to right to avoid collisions with down stem notes
							}
							else
							{
								snprintf(noteText, sizeof(noteText), "%s",  MSQ_noteHalfDown.c_str()); 
								pos.x-=1.0;  // move down stem notes a bit to left to avoid collisions with up stem notes
							}
						}
						else
						if (module->played_notes_circular_buffer[i].length==4)
						{
							if ((display_note<38)||((display_note>47)&&(display_note<59)))
							{
								snprintf(noteText, sizeof(noteText), "%s",  MSQ_noteQuarterUp.c_str()); 
								pos.x+=1.0;  // move up stem notes a bit to right to avoid collisions with down stem notes
							}
							else
							{
								snprintf(noteText, sizeof(noteText), "%s",  MSQ_noteQuarterDown.c_str()); 
								pos.x-=1.0;  // move down stem notes a bit to left to avoid collisions with up stem notes
							}
						}
						else
						if (module->played_notes_circular_buffer[i].length==8)
						{
							if ((display_note<38)||((display_note>47)&&(display_note<59)))
							{
								snprintf(noteText, sizeof(noteText), "%s",  MSQ_noteEighthUp.c_str()); 
								pos.x+=1.0;  // move up stem notes a bit to right to avoid collisions with down stem notes
							}
							else
							{
								snprintf(noteText, sizeof(noteText), "%s",  MSQ_noteEighthDown.c_str()); 
								pos.x-=1.0;  // move down stem notes a bit to left to avoid collisions with up stem notes
							}
						}
						else
						if (module->played_notes_circular_buffer[i].length==16)
						{
							if ((display_note<38)||((display_note>47)&&(display_note<59)))
							{
								snprintf(noteText, sizeof(noteText), "%s",  MSQ_noteSixteenthUp.c_str()); 
								pos.x+=1.0;  // move up stem notes a bit to right to avoid collisions with down stem notes
							}
							else
							{
								snprintf(noteText, sizeof(noteText), "%s",  MSQ_noteSixteenthDown.c_str()); 
								pos.x-=1.0;  // move down stem notes a bit to left to avoid collisions with up stem notes
							}
						}
						else
						if (module->played_notes_circular_buffer[i].length==32)
						{
							if ((display_note<38)||((display_note>47)&&(display_note<59)))
							{
								snprintf(noteText, sizeof(noteText), "%s",  MSQ_noteThirtysecondthUp.c_str()); 
								pos.x+=1.0;  // move up stem notes a bit to right to avoid collisions with down stem notes
							}
							else
							{
								snprintf(text, sizeof(noteText), "%s",  MSQ_noteThirtysecondthDown.c_str()); 
								pos.x-=1.0;  // move down stem notes a bit to left to avoid collisions with up stem notes
							}
						}   
					
						// do ledger lines
						int onLineNumberAboveStaves=0;  // value= 1,2,3
						int onLineNumberBetweenStaves=0;// valid=1
						int onLineNumberBelowStaves=0;  // value= 1,2,3
						int onSpaceNumberAboveStaves=0;  // value= 1,2,3
						int onSpaceNumberBetweenStaves=0;// valid=1
						int onSpaceNumberBelowStaves=0;  // value= 1,2,3
					
						// detect notes on lines
						if ((scale_note==3)&&(octave==5))                       //F7
							onLineNumberAboveStaves=7;
						else					
						if ((scale_note==1)&&(octave==5))                       //D7
							onLineNumberAboveStaves=6;
						else					
						if ((scale_note==6)&&(octave==4))                       //B6
							onLineNumberAboveStaves=5;
						else				
						if ((scale_note==4)&&(octave==4))                       //G6
							onLineNumberAboveStaves=4;
						else				
						if ((scale_note==2)&&(octave==4))                       //E6
							onLineNumberAboveStaves=3;
						else
						if ((scale_note==0)&&(octave==4))                       //C6
							onLineNumberAboveStaves=2;
						else
						if ((scale_note==5)&&(octave==3))                       //A5
							onLineNumberAboveStaves=1;
						else
						if ((scale_note==0)&&(octave==2))                       //C4
							onLineNumberBetweenStaves=1;
						else
						if ((scale_note==2)&&(octave==0))                       //E2
							onLineNumberBelowStaves=1;
						else
						if ((scale_note==0)&&(octave==0))                       //C2
							onLineNumberBelowStaves=2;
						else
						if ((scale_note==5)&&(octave==-1))		                //A1
							onLineNumberBelowStaves=3;
						else
						if ((scale_note==3)&&(octave==-1))		                //F1
							onLineNumberBelowStaves=4;
						else
						if ((scale_note==1)&&(octave==-1))		                //D1
							onLineNumberBelowStaves=5;
						else
						if ((scale_note==6)&&(octave==-2))		                //B0
							onLineNumberBelowStaves=6;

						Vec ledgerPos=pos;
						
						if ((onLineNumberAboveStaves!=0)||(onLineNumberBelowStaves!=0)||(onLineNumberBetweenStaves!=0))
						{
							nvgFontSize(args.vg, 90);
							nvgFillColor(args.vg,  MSQ_panelTextColor);
							nvgStrokeWidth(args.vg, lineWidth);
													
							if ((module->played_notes_circular_buffer[i].length==8)||(module->played_notes_circular_buffer[i].length==16)||(module->played_notes_circular_buffer[i].length==32))
								ledgerPos.x -= 2.75;
							else
								ledgerPos.x += 0.25;

							ledgerPos.y += 11.5;
							snprintf(text, sizeof(text), "%s",  MSQ_staff1Line.c_str()); 
							nvgText(args.vg, ledgerPos.x, ledgerPos.y, text, NULL);
							for (int j=onLineNumberAboveStaves; j>1; --j)
							{
								ledgerPos.y += 6.0;
								nvgText(args.vg,ledgerPos.x, ledgerPos.y, text, NULL);
							}
							for (int j=onLineNumberBelowStaves; j>1; --j)
							{
								ledgerPos.y -= 6.0;
								nvgText(args.vg, ledgerPos.x, ledgerPos.y, text, NULL);
							}
						} 

						if ((onLineNumberAboveStaves==0)&&(onLineNumberBelowStaves==0))  // prevent triggering both line and space
						{
							// detect notes on spaces
						
							if ((scale_note==4)&&(octave==5))                   //G7
								onSpaceNumberAboveStaves=8;
							else
							if ((scale_note==2)&&(octave==5))                   //E7
								onSpaceNumberAboveStaves=7;
							else
							if ((scale_note==0)&&(octave==5))                   //C7
								onSpaceNumberAboveStaves=6;
							else
							if ((scale_note==5)&&(octave==4))                   //A6
								onSpaceNumberAboveStaves=5;
							else
							if ((scale_note==3)&&(octave==4))                   //F6
								onSpaceNumberAboveStaves=4;
							else
							if ((scale_note==1)&&(octave==4))                   //D6
								onSpaceNumberAboveStaves=3;
							else
							if ((scale_note==6)&&(octave==3))                   //B5
								onSpaceNumberAboveStaves=2;
							else
							if ((scale_note==4)&&(octave==3))                   //G5
								onSpaceNumberAboveStaves=1;
							else
							if ((scale_note==3)&&(octave==0))                   //F2
								onSpaceNumberBelowStaves=1;
							else
							if ((scale_note==1)&&(octave==0))                   //D2
								onSpaceNumberBelowStaves=2;
							else
							if ((scale_note==6)&&(octave==-1))                  //B1
								onSpaceNumberBelowStaves=3;
							else
							if ((scale_note==4)&&(octave==-1))                  //G1
								onSpaceNumberBelowStaves=4;
							else
							if ((scale_note==2)&&(octave==-1))                  //E1
								onSpaceNumberBelowStaves=5;
							else
							if ((scale_note==0)&&(octave==-1))                  //C1
								onSpaceNumberBelowStaves=6;
							
							// Draw lines for spaces
							if ((onSpaceNumberAboveStaves!=0)||(onSpaceNumberBelowStaves!=0)||(onSpaceNumberBetweenStaves!=0))
							{
								nvgFontSize(args.vg, 90);
								nvgFillColor(args.vg,  MSQ_panelTextColor);
								nvgStrokeWidth(args.vg, lineWidth);
								
								if ((module->played_notes_circular_buffer[i].length==8)||(module->played_notes_circular_buffer[i].length==16)||(module->played_notes_circular_buffer[i].length==32))
									ledgerPos.x -= 2.75;
								else
									ledgerPos.x += 0.25;

								snprintf(text, sizeof(text), "%s",  MSQ_staff1Line.c_str()); 
								for (int j=onSpaceNumberAboveStaves; j>=1; --j)
								{
									ledgerPos.y=55.5-(j*6.0);
									nvgText(args.vg, ledgerPos.x, ledgerPos.y, text, NULL);
								}
								for (int j=onSpaceNumberBelowStaves; j>=1; --j)
								{
									ledgerPos.y=115.70+(j*6.0);
									nvgText(args.vg, ledgerPos.x, ledgerPos.y, text, NULL);
								}
							}
						} 
						if (true)  // color code notes in staff rendering
						{ 
							if (module->played_notes_circular_buffer[i].noteType==NOTE_TYPE_CHORD)
								nvgFillColor(args.vg,  MSQ_panelHarmonyPartColor); 
							else
							if (module->played_notes_circular_buffer[i].noteType==NOTE_TYPE_MELODY)
								nvgFillColor(args.vg,  MSQ_panelMelodyPartColor); 
							else
							if (module->played_notes_circular_buffer[i].noteType==NOTE_TYPE_ARP)
								nvgFillColor(args.vg,  MSQ_panelArpPartColor); 
							else
							if (module->played_notes_circular_buffer[i].noteType==NOTE_TYPE_BASS)
								nvgFillColor(args.vg,  MSQ_panelBassPartColor); 
							
						}
						nvgText(args.vg, pos.x, pos.y, noteText, NULL);  // now draw notes after ledger lines
					}
				}
			}

			nvgFontSize(args.vg, 12);
			
			if (textfont)
			nvgFontFaceId(args.vg, textfont->handle);
			nvgTextLetterSpacing(args.vg, -1);
			nvgFillColor(args.vg, MSQ_panelTextColor);

			nvgFontSize(args.vg, 12);
			

	   
			if (module->moduleVarsInitialized)  // globals fully initialized if Module!=NULL
			{
				nvgFontSize(args.vg, 14);
				if (textfont)
				nvgFontFaceId(args.vg, textfont->handle);
				nvgTextLetterSpacing(args.vg, -1);
				nvgFillColor(args.vg, MSQ_panelTextColor);
				nvgTextAlign(args.vg,NVG_ALIGN_CENTER|NVG_ALIGN_MIDDLE);
			}
		
			pos=Vec(344, 235); 
			nvgFontSize(args.vg, 25);
			nvgTextAlign(args.vg,NVG_ALIGN_CENTER|NVG_ALIGN_MIDDLE);

            if (module->valid_current_circle_degree)
			  nvgFillColor(args.vg, MSQ_panelHarmonyPartColor); 
			else
			  nvgFillColor(args.vg, MSQ_panelTextColor); 


			int last_chord_root=module->theModeScaleQuantState.last_harmony_chord_root_note%12;
						
			char chord_type_desc[16];
			strcpy(chord_type_desc, "");
			if (module->theModeScaleQuantState.theHarmonyParms.last_chord_type==0)
				strcpy(chord_type_desc, "");
			else
			if (module->theModeScaleQuantState.theHarmonyParms.last_chord_type==2)  // dom
				strcpy(chord_type_desc, "dom7");
			else
			if (module->theModeScaleQuantState.theHarmonyParms.last_chord_type==1)
				strcpy(chord_type_desc, "m");
			else
			if (module->theModeScaleQuantState.theHarmonyParms.last_chord_type==3)
				strcpy(chord_type_desc, "7");
			else
			if (module->theModeScaleQuantState.theHarmonyParms.last_chord_type==4)
				strcpy(chord_type_desc, "m7");
			else
			if (module->theModeScaleQuantState.theHarmonyParms.last_chord_type==5)
				strcpy(chord_type_desc, "dim7");
			else
			if (module->theModeScaleQuantState.theHarmonyParms.last_chord_type==6)
				strcpy(chord_type_desc, "dim");
    						   
			if ((module->theModeScaleQuantState.theHarmonyParms.last_chord_type==0)||(module->theModeScaleQuantState.theHarmonyParms.last_chord_type==2)||(module->theModeScaleQuantState.theHarmonyParms.last_chord_type==3))  // major
			{
				if (module->valid_current_circle_degree)
				  snprintf(text, sizeof(text), "%s-%s%s", MSQ_circle_of_fifths_arabic_degrees[module->current_circle_degree], module->note_desig[last_chord_root], chord_type_desc);
			    else
				  snprintf(text, sizeof(text), "%s%s",  module->note_desig[last_chord_root], chord_type_desc);
			}
			else
			if ((module->theModeScaleQuantState.theHarmonyParms.last_chord_type==5)||(module->theModeScaleQuantState.theHarmonyParms.last_chord_type==6)) // diminished
			{
				if (module->valid_current_circle_degree)
				  snprintf(text, sizeof(text), "%s'-%s%s", MSQ_circle_of_fifths_arabic_degrees_LC[module->current_circle_degree], module->note_desig[last_chord_root], chord_type_desc);
				else
				  snprintf(text, sizeof(text), "%s%s",  module->note_desig[last_chord_root], chord_type_desc);
			}
			else  // minor
			{
				if (module->valid_current_circle_degree)
				  snprintf(text, sizeof(text), "%s-%s%s", MSQ_circle_of_fifths_arabic_degrees_LC[module->current_circle_degree], module->note_desig[last_chord_root], chord_type_desc);
				else
				   snprintf(text, sizeof(text), "%s%s",  module->note_desig[last_chord_root], chord_type_desc);
			}
								
			nvgText(args.vg, pos.x, pos.y, text, NULL);  
				
			if (true)
			{ 
				drawKeyboard(args); // redraw full keyboard per frame. clears any key down states
				int octave=4;
				int semitoneOffset=0;
			    int keyboard_offset=12.0; // adjust note range to middle C on piano keyboard
				int colorIndex=1;  // gray
			
			    
				if ((strstr(module->MSQscaleText,"C# "))||(strstr(module->MSQscaleText,"Db ")))
				{
					semitoneOffset=1;
					if (module->root_key>semitoneOffset)  
					    drawKey(args, ((octave+1)*12)+semitoneOffset+keyboard_offset, true, colorIndex); 
					else
				    	drawKey(args, (octave*12)+semitoneOffset+keyboard_offset, true, colorIndex);  
				}
		
				if (strstr(module->MSQscaleText,"C "))
				{
					semitoneOffset=0;
					if (module->root_key>semitoneOffset)  
					   drawKey(args, ((octave+1)*12)+semitoneOffset+keyboard_offset, true, colorIndex); 
					else
				       drawKey(args, (octave*12)+semitoneOffset+keyboard_offset, true, colorIndex); 
				}
			
				if ((strstr(module->MSQscaleText,"D# "))||(strstr(module->MSQscaleText,"Eb ")))
				{
					semitoneOffset=3;
					if (module->root_key>semitoneOffset)  
					   drawKey(args, ((octave+1)*12)+semitoneOffset+keyboard_offset, true, colorIndex); 
					else
					   drawKey(args, (octave*12)+semitoneOffset+keyboard_offset, true, colorIndex);  
				}
			
				if (strstr(module->MSQscaleText,"D "))
				{
					semitoneOffset=2;
					if (module->root_key>semitoneOffset)  
					   drawKey(args, ((octave+1)*12)+semitoneOffset+keyboard_offset, true, colorIndex); 
					else
					   drawKey(args, (octave*12)+semitoneOffset+keyboard_offset, true, colorIndex);  
				}
			
				if (strstr(module->MSQscaleText,"E "))
				{
					semitoneOffset=4;
					if (module->root_key>semitoneOffset)  
					   drawKey(args, ((octave+1)*12)+semitoneOffset+keyboard_offset, true, colorIndex); 
					else
					  drawKey(args, (octave*12)+semitoneOffset+keyboard_offset, true, colorIndex);  
				}
			
				if ((strstr(module->MSQscaleText,"F# "))||(strstr(module->MSQscaleText,"Gb ")))
				{
					semitoneOffset=6;
					if (module->root_key>semitoneOffset)  
					   drawKey(args, ((octave+1)*12)+semitoneOffset+keyboard_offset, true, colorIndex); 
					else
				  	drawKey(args, (octave*12)+semitoneOffset+keyboard_offset, true, colorIndex);  
				}
			
				if (strstr(module->MSQscaleText,"F "))
				{
					semitoneOffset=5;
					if (module->root_key>semitoneOffset)  
					   drawKey(args, ((octave+1)*12)+semitoneOffset+keyboard_offset, true, colorIndex); 
					else
					  drawKey(args, (octave*12)+semitoneOffset+keyboard_offset, true, colorIndex);  
				}
			
				if ((strstr(module->MSQscaleText,"G# "))||(strstr(module->MSQscaleText,"Ab ")))
				{
					semitoneOffset=8;
					if (module->root_key>semitoneOffset)  
					   drawKey(args, ((octave+1)*12)+semitoneOffset+keyboard_offset, true, colorIndex); 
					else
					   drawKey(args, (octave*12)+semitoneOffset+keyboard_offset, true, colorIndex);  
				}
			
				if (strstr(module->MSQscaleText,"G "))
				{
					semitoneOffset=7;
					if (module->root_key>semitoneOffset)  
					   drawKey(args, ((octave+1)*12)+semitoneOffset+keyboard_offset, true, colorIndex); 
					else
					   drawKey(args, (octave*12)+semitoneOffset+keyboard_offset, true, colorIndex);  
				}
			 
				if ((strstr(module->MSQscaleText,"A# "))||(strstr(module->MSQscaleText,"Bb ")))
				{
					semitoneOffset=10;
					if (module->root_key>semitoneOffset)  
					   drawKey(args, ((octave+1)*12)+semitoneOffset+keyboard_offset, true, colorIndex); 
					else
					   drawKey(args, (octave*12)+semitoneOffset+keyboard_offset, true, colorIndex);  
				}
			
				if (strstr(module->MSQscaleText,"A "))
				{
					semitoneOffset=9;
					if (module->root_key>semitoneOffset)  
					   drawKey(args, ((octave+1)*12)+semitoneOffset+keyboard_offset, true, colorIndex); 
					else
					   drawKey(args, (octave*12)+semitoneOffset+keyboard_offset, true, colorIndex);  
				}
			
				if (strstr(module->MSQscaleText,"B "))
				{
					semitoneOffset=11;
					if (module->root_key>semitoneOffset)  
					   drawKey(args, ((octave+1)*12)+semitoneOffset+keyboard_offset, true, colorIndex); 
					else
					   drawKey(args, (octave*12)+semitoneOffset+keyboard_offset, true, colorIndex);  
				}
			}

			// draw a vertucal division line
			nvgBeginPath(args.vg);
			nvgStrokeWidth(args.vg, 2.0);
			nvgStrokeColor(args.vg,nvgRGBA( 0x80,  0x80 , 0x80, 0x80));
			nvgMoveTo(args.vg, 185, 10);
			nvgLineTo(args.vg, 185, 370);
			nvgStroke(args.vg);
			nvgClosePath(args.vg);
			//
					
		}  // end UpdatePanel()

	   
		double smoothedDt=.016;  // start out at 1/60
		int numZeroes=0;
		
		void draw(const DrawArgs &args) override 
		{   
		 	if (!module)  // if there is no module, draw the static panel image, i.e., in the browser
			{
				nvgBeginPath(args.vg);
				nvgRect(args.vg, 0.0, 0.0, box.size.x, box.size.y);
				
				if (MSQ_panelTheme==0)  // light theme
				{
					std::shared_ptr<Image> lightPanelImage = APP->window->loadImage(asset::plugin(pluginInstance,"res/ModeScaleQuant-light.png"));
					if (lightPanelImage) 
					{
						int height=0;
						int width=0;
						nvgImageSize(args.vg, lightPanelImage->handle, &width, &height);
						NVGpaint nvgpaint=nvgImagePattern(args.vg, 0.0, 0.0, width, height, 0.0, lightPanelImage->handle, 1.0);
						nvgFillPaint(args.vg, nvgpaint);
						nvgFill(args.vg);
					}
				}
				else // dark theme
				{
					std::shared_ptr<Image> darkPanelImage = APP->window->loadImage(asset::plugin(pluginInstance,"res/ModeScaleQuant-dark.png"));
					if (darkPanelImage) 
					{
						int height=0;
				        int width=0;
						nvgImageSize(args.vg, darkPanelImage->handle, &width, &height);
						NVGpaint nvgpaint=nvgImagePattern(args.vg, 0.0, 0.0, width, height, 0.0, darkPanelImage->handle, 1.0);
						nvgFillPaint(args.vg, nvgpaint);
						nvgFill(args.vg);
					}
				}
							
				nvgClosePath(args.vg);
			    Widget::draw(args);
			    return;  // do not proceedurally draw panel 
			}

			if (true)  // false to disable most nanovg panel rendering for testing
			{
				            							
				// draw ModeScaleQuant logo and chord legend
				
				std::shared_ptr<Font> textfont = APP->window->loadFont(asset::plugin(pluginInstance, "res/Ubuntu Condensed 400.ttf"));
							
				if (textfont)
				{
					nvgBeginPath(args.vg);
					nvgFontSize(args.vg, 27);
					nvgFontFaceId(args.vg, textfont->handle);
					nvgTextLetterSpacing(args.vg, -1);
					nvgFillColor(args.vg, MSQ_panelTextColor);
					nvgTextAlign(args.vg,NVG_ALIGN_CENTER|NVG_ALIGN_MIDDLE);
					
					char text[128];
					snprintf(text, sizeof(text), "%s", "PS-PurrSoftware");
								
					Vec pos=Vec(90, 37); 
					nvgStrokeWidth(args.vg, 3.0);
					nvgText(args.vg, pos.x, pos.y, text, NULL);

					snprintf(text, sizeof(text), "%s", "ModeScaleQuant");
					pos=Vec(90, 65); 
					nvgStrokeWidth(args.vg, 3.0);
					nvgText(args.vg, pos.x, pos.y, text, NULL);

					snprintf(text, sizeof(text), "%s", "Mode Scale Notes");
					nvgFontSize(args.vg, 11);
					pos=Vec(270, 340); 
					nvgStrokeWidth(args.vg, 3.0);
					nvgText(args.vg, pos.x, pos.y, text, NULL);
				

					snprintf(text, sizeof(text), "%s", "Diatonic Circle of 5ths");  
					nvgFontSize(args.vg, 15);
					pos=Vec(350, 35);  
					nvgStrokeWidth(args.vg, 2.0);
					nvgText(args.vg, pos.x, pos.y, text, NULL); 

					nvgClosePath(args.vg);

					nvgStrokeWidth(args.vg, 1.0);
					nvgStrokeColor(args.vg, MSQ_panelLineColor);

					nvgTextAlign(args.vg,NVG_ALIGN_LEFT|NVG_ALIGN_MIDDLE);
								
					nvgBeginPath(args.vg);
				    float opacity = 128; 
				
					nvgMoveTo(args.vg, 250, 45);
					nvgLineTo(args.vg, 290, 45);
					nvgLineTo(args.vg, 290, 55);
					nvgLineTo(args.vg, 250, 55);
					nvgLineTo(args.vg, 250, 45);

					nvgFillColor(args.vg, nvgRGBA(0xff, 0x20, 0x20, (int)opacity));  // reddish
					nvgStroke(args.vg);
					nvgFill(args.vg);
					snprintf(text, sizeof(text), "%s", "Major"); 
					nvgFillColor(args.vg, MSQ_panelTextColor);
					nvgFontSize(args.vg, 10);
					pos=Vec(261, 50);  
					nvgText(args.vg, pos.x, pos.y, text, NULL);
					nvgClosePath(args.vg);

					nvgBeginPath(args.vg);
				
					nvgMoveTo(args.vg, 325, 45);
					nvgLineTo(args.vg, 365, 45);
					nvgLineTo(args.vg, 365, 55);
					nvgLineTo(args.vg, 325, 55);
					nvgLineTo(args.vg, 325, 45);

					nvgFillColor(args.vg, nvgRGBA(0x20, 0x20, 0xff, (int)opacity));  //bluish
					nvgStroke(args.vg);
					nvgFill(args.vg);
					snprintf(text, sizeof(text), "%s", "Minor"); 
					nvgFillColor(args.vg, MSQ_panelTextColor);
					nvgFontSize(args.vg, 10);
					pos=Vec(336, 50);  
					nvgText(args.vg, pos.x, pos.y, text, NULL);
					nvgClosePath(args.vg);

					nvgBeginPath(args.vg);
				
					nvgMoveTo(args.vg, 400, 45);
					nvgLineTo(args.vg, 440, 45);
					nvgLineTo(args.vg, 440, 55);
					nvgLineTo(args.vg, 400, 55);
					nvgLineTo(args.vg, 400, 45);

					nvgFillColor(args.vg, nvgRGBA(0x20, 0xff, 0x20, (int)opacity));  // greenish
					nvgStroke(args.vg);
					nvgFill(args.vg);
					snprintf(text, sizeof(text), "%s", "Diminished"); 
					nvgFillColor(args.vg, MSQ_panelTextColor);
					nvgFontSize(args.vg, 10);
					pos=Vec(404, 50);  
					nvgText(args.vg, pos.x, pos.y, text, NULL);
					nvgClosePath(args.vg);
					
				} 
				
				DrawCircle5ths(args, module->root_key);  // has to be done each frame as panel redraws as SVG and needs to be blanked and cirecles redrawn
				DrawDegreesSemicircle(args,  module->root_key);
				updatePanel(args);
							
			} 
		}  

		 
	
	};  // end struct CircleOf5thsDisplay  

	float dummytempo=120;  // for module==null browser visibility purposes
	int dummysig=4;        // for module==null browser visibility purposes
	int dummyindex=0;      // for module==null browser visibility purposes

	ModeScaleQuantWidget(ModeScaleQuant* module)   // all plugins I've looked at use this constructor with module*, even though docs show it deprecated.     
	{ 
		setModule(module);  // most plugins do this 
		this->module = module;  //  most plugins do not do this.  It was introduced in singleton implementation

		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ModeScaleQuant-light.svg")));
		svgPanel=(SvgPanel*)getPanel();
		svgPanel->setVisible((MSQ_panelTheme) == 0);  
					
		MSQ_panelcolor=nvgRGBA((unsigned char)230,(unsigned char)230,(unsigned char)230,(unsigned char)255);
		
		darkPanel = new SvgPanel();
		darkPanel->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ModeScaleQuant-dark.svg")));
		darkPanel->setVisible((MSQ_panelTheme) == 1);  
		addChild(darkPanel);
							
		rack::random::init();  // must be called per thread 

		 if (true)   // must be executed even if module* is null. module* is checked for null below where accessed as it is null in browser preview
		 {
			// create param widgets  Needs to be done even if module is null.
			float RootKey_row=150.;
			float ScaleSelect_row=175.0;
			float PolyExtScale_row=256.0;
			float Quantizer_row=270.0;
						
			ModeScaleQuantRootKeySelectLineDisplay *ModeScaleQuantRootKeySelectDisplay = new ModeScaleQuantRootKeySelectLineDisplay();
			ModeScaleQuantRootKeySelectDisplay->box.pos = Vec(85.,RootKey_row-10.0); 
			ModeScaleQuantRootKeySelectDisplay->box.size = Vec(40, 22); 
			ModeScaleQuantRootKeySelectDisplay->module=module;
			if (module) 
				ModeScaleQuantRootKeySelectDisplay->val = &module->root_key;
			else
			{ 
				ModeScaleQuantRootKeySelectDisplay->val = &dummyindex;  // strictly for browser visibility
			}
			addChild(ModeScaleQuantRootKeySelectDisplay);

			ModeScaleQuantScaleSelectLineDisplay *ModeScaleQuantScaleSelectDisplay = new ModeScaleQuantScaleSelectLineDisplay();
			ModeScaleQuantScaleSelectDisplay->box.pos =Vec(1.0, ScaleSelect_row+10.0); 
			ModeScaleQuantScaleSelectDisplay->box.size = Vec(130, 22); 
			ModeScaleQuantScaleSelectDisplay->module=module;
			if (module) 
				ModeScaleQuantScaleSelectDisplay->val = &module->mode;
			else
			{ 
				ModeScaleQuantScaleSelectDisplay->val = &dummyindex;  // strictly for browser visibility
			}
			
			addChild(ModeScaleQuantScaleSelectDisplay);

			CircleOf5thsDisplay *display = new CircleOf5thsDisplay(module);
			display->ParameterRectLocal=ParameterRect;
			display->InportRectLocal=InportRect;  
			display->OutportRectLocal=OutportRect;  
			display->box.pos = Vec(0, 0);
			display->box.size = Vec(box.size.x, box.size.y);
			addChild(display);
							
			// create params, controls, lights and ports.  Needs to be done even if module* is null.
			
			//*************   Note: Each LEDButton needs its light and that light needs a unique ID, needs to be added to an array and then needs to be repositioned along with the button.  Also needs to be enumed with other lights so lights[] picks it up.
			paramWidgets[ModeScaleQuant::BUTTON_CIRCLESTEP_C_PARAM]=createParamCentered<LEDButton>(mm2px(Vec(116.227, 37.257)), module, ModeScaleQuant::BUTTON_CIRCLESTEP_C_PARAM);
			addParam(paramWidgets[ModeScaleQuant::BUTTON_CIRCLESTEP_C_PARAM]);
			lightWidgets[ModeScaleQuant::LIGHT_LEDBUTTON_CIRCLESTEP_1]=createLightCentered<MediumSimpleLight<GreenLight>>(mm2px(Vec(116.227, 37.257)), module, ModeScaleQuant::LIGHT_LEDBUTTON_CIRCLESTEP_1);
			addChild(lightWidgets[ModeScaleQuant::LIGHT_LEDBUTTON_CIRCLESTEP_1]);
		
			paramWidgets[ModeScaleQuant::BUTTON_CIRCLESTEP_G_PARAM]=createParamCentered<LEDButton>(mm2px(Vec(132.479, 41.32)), module, ModeScaleQuant::BUTTON_CIRCLESTEP_G_PARAM);
			addParam(paramWidgets[ModeScaleQuant::BUTTON_CIRCLESTEP_G_PARAM]);
			lightWidgets[ModeScaleQuant::LIGHT_LEDBUTTON_CIRCLESTEP_2]=createLightCentered<MediumSimpleLight<GreenLight>>(mm2px(Vec(132.479, 41.32)), module, ModeScaleQuant::LIGHT_LEDBUTTON_CIRCLESTEP_2);
			addChild(lightWidgets[ModeScaleQuant::LIGHT_LEDBUTTON_CIRCLESTEP_2]);
		
			paramWidgets[ModeScaleQuant::BUTTON_CIRCLESTEP_D_PARAM]=createParamCentered<LEDButton>(mm2px(Vec(143.163, 52.155)), module, ModeScaleQuant::BUTTON_CIRCLESTEP_D_PARAM);
			addParam(paramWidgets[ModeScaleQuant::BUTTON_CIRCLESTEP_D_PARAM]);
			lightWidgets[ModeScaleQuant::LIGHT_LEDBUTTON_CIRCLESTEP_3]=createLightCentered<MediumSimpleLight<GreenLight>>(mm2px(Vec(143.163, 52.155)), module, ModeScaleQuant::LIGHT_LEDBUTTON_CIRCLESTEP_3);
			addChild(lightWidgets[ModeScaleQuant::LIGHT_LEDBUTTON_CIRCLESTEP_3]);
		
			paramWidgets[ModeScaleQuant::BUTTON_CIRCLESTEP_A_PARAM]=createParamCentered<LEDButton>(mm2px(Vec(147.527, 67.353)), module, ModeScaleQuant::BUTTON_CIRCLESTEP_A_PARAM);
			addParam(paramWidgets[ModeScaleQuant::BUTTON_CIRCLESTEP_A_PARAM]);
			lightWidgets[ModeScaleQuant::LIGHT_LEDBUTTON_CIRCLESTEP_4]=createLightCentered<MediumSimpleLight<GreenLight>>(mm2px(Vec(147.527, 67.353)), module, ModeScaleQuant::LIGHT_LEDBUTTON_CIRCLESTEP_4);
			addChild(lightWidgets[ModeScaleQuant::LIGHT_LEDBUTTON_CIRCLESTEP_4]);
		
			paramWidgets[ModeScaleQuant::BUTTON_CIRCLESTEP_E_PARAM]=createParamCentered<LEDButton>(mm2px(Vec(141.96, 83.906)), module, ModeScaleQuant::BUTTON_CIRCLESTEP_E_PARAM);
			addParam(paramWidgets[ModeScaleQuant::BUTTON_CIRCLESTEP_E_PARAM]);
			lightWidgets[ModeScaleQuant::LIGHT_LEDBUTTON_CIRCLESTEP_5]=createLightCentered<MediumSimpleLight<GreenLight>>(mm2px(Vec(141.96, 83.906)), module, ModeScaleQuant::LIGHT_LEDBUTTON_CIRCLESTEP_5);
			addChild(lightWidgets[ModeScaleQuant::LIGHT_LEDBUTTON_CIRCLESTEP_5]);
		
			paramWidgets[ModeScaleQuant::BUTTON_CIRCLESTEP_B_PARAM]=createParamCentered<LEDButton>(mm2px(Vec(132.931, 94.44)), module, ModeScaleQuant::BUTTON_CIRCLESTEP_B_PARAM);
			addParam(paramWidgets[ModeScaleQuant::BUTTON_CIRCLESTEP_B_PARAM]);
			lightWidgets[ModeScaleQuant::LIGHT_LEDBUTTON_CIRCLESTEP_6]=createLightCentered<MediumSimpleLight<GreenLight>>(mm2px(Vec(132.931, 94.44)), module, ModeScaleQuant::LIGHT_LEDBUTTON_CIRCLESTEP_6);
			addChild(lightWidgets[ModeScaleQuant::LIGHT_LEDBUTTON_CIRCLESTEP_6]);
		
			paramWidgets[ModeScaleQuant::BUTTON_CIRCLESTEP_GBFS_PARAM]=createParamCentered<LEDButton>(mm2px(Vec(116.378, 98.804)), module, ModeScaleQuant::BUTTON_CIRCLESTEP_GBFS_PARAM);
			addParam(paramWidgets[ModeScaleQuant::BUTTON_CIRCLESTEP_GBFS_PARAM]);
			lightWidgets[ModeScaleQuant::LIGHT_LEDBUTTON_CIRCLESTEP_7]=createLightCentered<MediumSimpleLight<GreenLight>>(mm2px(Vec(116.378, 98.804)), module, ModeScaleQuant::LIGHT_LEDBUTTON_CIRCLESTEP_7);
			addChild(lightWidgets[ModeScaleQuant::LIGHT_LEDBUTTON_CIRCLESTEP_7]);
		
			paramWidgets[ModeScaleQuant::BUTTON_CIRCLESTEP_DB_PARAM]=createParamCentered<LEDButton>(mm2px(Vec(101.029, 93.988)), module, ModeScaleQuant::BUTTON_CIRCLESTEP_DB_PARAM);
			addParam(paramWidgets[ModeScaleQuant::BUTTON_CIRCLESTEP_DB_PARAM]);
			lightWidgets[ModeScaleQuant::LIGHT_LEDBUTTON_CIRCLESTEP_8]=createLightCentered<MediumSimpleLight<GreenLight>>(mm2px(Vec(101.029, 93.988)), module, ModeScaleQuant::LIGHT_LEDBUTTON_CIRCLESTEP_8);
			addChild(lightWidgets[ModeScaleQuant::LIGHT_LEDBUTTON_CIRCLESTEP_8]);
		
			paramWidgets[ModeScaleQuant::BUTTON_CIRCLESTEP_AB_PARAM]=createParamCentered<LEDButton>(mm2px(Vec(91.097, 83.906)), module, ModeScaleQuant::BUTTON_CIRCLESTEP_AB_PARAM);
			addParam(paramWidgets[ModeScaleQuant::BUTTON_CIRCLESTEP_AB_PARAM]);
			lightWidgets[ModeScaleQuant::LIGHT_LEDBUTTON_CIRCLESTEP_9]=createLightCentered<MediumSimpleLight<GreenLight>>(mm2px(Vec(91.097, 83.906)), module, ModeScaleQuant::LIGHT_LEDBUTTON_CIRCLESTEP_9);
			addChild(lightWidgets[ModeScaleQuant::LIGHT_LEDBUTTON_CIRCLESTEP_9]);
		
			paramWidgets[ModeScaleQuant::BUTTON_CIRCLESTEP_EB_PARAM]=createParamCentered<LEDButton>(mm2px(Vec(86.282, 68.106)), module, ModeScaleQuant::BUTTON_CIRCLESTEP_EB_PARAM);
			addParam(paramWidgets[ModeScaleQuant::BUTTON_CIRCLESTEP_EB_PARAM]);
			lightWidgets[ModeScaleQuant::LIGHT_LEDBUTTON_CIRCLESTEP_10]=createLightCentered<MediumSimpleLight<GreenLight>>(mm2px(Vec(86.282, 68.106)), module, ModeScaleQuant::LIGHT_LEDBUTTON_CIRCLESTEP_10);
			addChild(lightWidgets[ModeScaleQuant::LIGHT_LEDBUTTON_CIRCLESTEP_10]);
		
			paramWidgets[ModeScaleQuant::BUTTON_CIRCLESTEP_BB_PARAM]=createParamCentered<LEDButton>(mm2px(Vec(89.743, 52.004)), module, ModeScaleQuant::BUTTON_CIRCLESTEP_BB_PARAM);
			addParam(paramWidgets[ModeScaleQuant::BUTTON_CIRCLESTEP_BB_PARAM]);
			lightWidgets[ModeScaleQuant::LIGHT_LEDBUTTON_CIRCLESTEP_11]=createLightCentered<MediumSimpleLight<GreenLight>>(mm2px(Vec(189.743, 52.004)), module, ModeScaleQuant::LIGHT_LEDBUTTON_CIRCLESTEP_11);
			addChild(lightWidgets[ModeScaleQuant::LIGHT_LEDBUTTON_CIRCLESTEP_11]);
		
			paramWidgets[ModeScaleQuant::BUTTON_CIRCLESTEP_F_PARAM]=createParamCentered<LEDButton>(mm2px(Vec(101.781, 40.568)), module, ModeScaleQuant::BUTTON_CIRCLESTEP_F_PARAM);
			addParam(paramWidgets[ModeScaleQuant::BUTTON_CIRCLESTEP_F_PARAM]);
			lightWidgets[ModeScaleQuant::LIGHT_LEDBUTTON_CIRCLESTEP_12]=createLightCentered<MediumSimpleLight<GreenLight>>(mm2px(Vec(101.781, 40.568)), module, ModeScaleQuant::LIGHT_LEDBUTTON_CIRCLESTEP_12);
			addChild(lightWidgets[ModeScaleQuant::LIGHT_LEDBUTTON_CIRCLESTEP_12]);
		
	//*************
			lightWidgets[ModeScaleQuant::LIGHT_CIRCLE_ROOT_KEY_POSITION_1_LIGHT]=createLightCentered<MediumLight<RedLight>>(mm2px(Vec(116.227, 43.878)), module, ModeScaleQuant::LIGHT_CIRCLE_ROOT_KEY_POSITION_1_LIGHT);
			addChild(lightWidgets[ModeScaleQuant::LIGHT_CIRCLE_ROOT_KEY_POSITION_1_LIGHT]);
		
			lightWidgets[ModeScaleQuant::LIGHT_CIRCLE_ROOT_KEY_POSITION_2_LIGHT]=createLightCentered<MediumLight<RedLight>>(mm2px(Vec(129.018, 47.189)), module, ModeScaleQuant::LIGHT_CIRCLE_ROOT_KEY_POSITION_2_LIGHT);
			addChild(lightWidgets[ModeScaleQuant::LIGHT_CIRCLE_ROOT_KEY_POSITION_2_LIGHT]);
		
			lightWidgets[ModeScaleQuant::LIGHT_CIRCLE_ROOT_KEY_POSITION_3_LIGHT]=createLightCentered<MediumLight<RedLight>>(mm2px(Vec(137.295, 56.067)), module, ModeScaleQuant::LIGHT_CIRCLE_ROOT_KEY_POSITION_3_LIGHT);
			addChild(lightWidgets[ModeScaleQuant::LIGHT_CIRCLE_ROOT_KEY_POSITION_3_LIGHT]);
		
			lightWidgets[ModeScaleQuant::LIGHT_CIRCLE_ROOT_KEY_POSITION_4_LIGHT]=createLightCentered<MediumLight<RedLight>>(mm2px(Vec(140.455, 67.654)), module, ModeScaleQuant::LIGHT_CIRCLE_ROOT_KEY_POSITION_4_LIGHT);
			addChild(lightWidgets[ModeScaleQuant::LIGHT_CIRCLE_ROOT_KEY_POSITION_4_LIGHT]);
		
			lightWidgets[ModeScaleQuant::LIGHT_CIRCLE_ROOT_KEY_POSITION_5_LIGHT]=createLightCentered<MediumLight<RedLight>>(mm2px(Vec(137.144, 80.295)), module, ModeScaleQuant::LIGHT_CIRCLE_ROOT_KEY_POSITION_5_LIGHT);
			addChild(lightWidgets[ModeScaleQuant::LIGHT_CIRCLE_ROOT_KEY_POSITION_5_LIGHT]);
		
			lightWidgets[ModeScaleQuant::LIGHT_CIRCLE_ROOT_KEY_POSITION_6_LIGHT]=createLightCentered<MediumLight<RedLight>>(mm2px(Vec(128.868, 88.571)), module, ModeScaleQuant::LIGHT_CIRCLE_ROOT_KEY_POSITION_6_LIGHT);
			addChild(lightWidgets[ModeScaleQuant::LIGHT_CIRCLE_ROOT_KEY_POSITION_6_LIGHT]);
		
			lightWidgets[ModeScaleQuant::LIGHT_CIRCLE_ROOT_KEY_POSITION_7_LIGHT]=createLightCentered<MediumLight<RedLight>>(mm2px(Vec(116.077, 92.183)), module, ModeScaleQuant::LIGHT_CIRCLE_ROOT_KEY_POSITION_7_LIGHT);
			addChild(lightWidgets[ModeScaleQuant::LIGHT_CIRCLE_ROOT_KEY_POSITION_7_LIGHT]);
		
			lightWidgets[ModeScaleQuant::LIGHT_CIRCLE_ROOT_KEY_POSITION_8_LIGHT]=createLightCentered<MediumLight<RedLight>>(mm2px(Vec(104.791, 88.872)), module, ModeScaleQuant::LIGHT_CIRCLE_ROOT_KEY_POSITION_8_LIGHT);
			addChild(lightWidgets[ModeScaleQuant::LIGHT_CIRCLE_ROOT_KEY_POSITION_8_LIGHT]);
		
			lightWidgets[ModeScaleQuant::LIGHT_CIRCLE_ROOT_KEY_POSITION_9_LIGHT]=createLightCentered<MediumLight<RedLight>>(mm2px(Vec(96.213, 80.596)), module, ModeScaleQuant::LIGHT_CIRCLE_ROOT_KEY_POSITION_9_LIGHT);
			addChild(lightWidgets[ModeScaleQuant::LIGHT_CIRCLE_ROOT_KEY_POSITION_9_LIGHT]); 
		
			lightWidgets[ModeScaleQuant::LIGHT_CIRCLE_ROOT_KEY_POSITION_10_LIGHT]=createLightCentered<MediumLight<RedLight>>(mm2px(Vec(92.602, 67.654)), module, ModeScaleQuant::LIGHT_CIRCLE_ROOT_KEY_POSITION_10_LIGHT);
			addChild(lightWidgets[ModeScaleQuant::LIGHT_CIRCLE_ROOT_KEY_POSITION_10_LIGHT]);
		
			lightWidgets[ModeScaleQuant::LIGHT_CIRCLE_ROOT_KEY_POSITION_11_LIGHT]=createLightCentered<MediumLight<RedLight>>(mm2px(Vec(95.912, 55.465)), module, ModeScaleQuant::LIGHT_CIRCLE_ROOT_KEY_POSITION_11_LIGHT);
			addChild(lightWidgets[ModeScaleQuant::LIGHT_CIRCLE_ROOT_KEY_POSITION_11_LIGHT]);
		
			lightWidgets[ModeScaleQuant::LIGHT_CIRCLE_ROOT_KEY_POSITION_12_LIGHT]=createLightCentered<MediumLight<RedLight>>(mm2px(Vec(105.393, 46.587)), module, ModeScaleQuant::LIGHT_CIRCLE_ROOT_KEY_POSITION_12_LIGHT);
			addChild(lightWidgets[ModeScaleQuant::LIGHT_CIRCLE_ROOT_KEY_POSITION_12_LIGHT]);
				  				
			paramWidgets[ModeScaleQuant::CONTROL_ROOT_KEY_PARAM]=createParamCentered<Trimpot>(Vec(23.98, RootKey_row), module, ModeScaleQuant::CONTROL_ROOT_KEY_PARAM); 
			dynamic_cast<Knob*>(paramWidgets[ModeScaleQuant::CONTROL_ROOT_KEY_PARAM])->snap=true;
			addParam(paramWidgets[ModeScaleQuant::CONTROL_ROOT_KEY_PARAM]);
		
			paramWidgets[ModeScaleQuant::CONTROL_SCALE_PARAM]=createParamCentered<Trimpot>(Vec(23.98, 232.31), module, ModeScaleQuant::CONTROL_SCALE_PARAM);
			dynamic_cast<Knob*>(paramWidgets[ModeScaleQuant::CONTROL_SCALE_PARAM])->snap=true;
	 		addParam(paramWidgets[ModeScaleQuant::CONTROL_SCALE_PARAM]);

			//***************Harmony******************
		
		// add input ports
		
		   
			for (int i=0; i<ModeScaleQuant::NUM_INPUTS; ++i)
			{
				inPortWidgets[i]=createInputCentered<TinyPJ301MPort>(mm2px(Vec(10*i,5)), module, i);  // temporarily place them along the top before they are repositioned above
				addInput(inPortWidgets[i]);
			}

	// add output ports		
			
			 		
			outPortWidgets[ModeScaleQuant::OUT_HARMONY_CV_OUTPUT]=createOutputCentered<PJ301MPort>(Vec(596.98f, 342.25), module, ModeScaleQuant::OUT_HARMONY_CV_OUTPUT);
			addOutput(outPortWidgets[ModeScaleQuant::OUT_HARMONY_CV_OUTPUT]);
			
			outPortWidgets[ModeScaleQuant::OUT_HARMONY_GATE_OUTPUT]=createOutputCentered<PJ301MPort>(Vec(596.67, 366.93), module, ModeScaleQuant::OUT_HARMONY_GATE_OUTPUT);
			addOutput(outPortWidgets[ModeScaleQuant::OUT_HARMONY_GATE_OUTPUT]);
					
			outPortWidgets[ModeScaleQuant::OUT_EXT_POLY_SCALE_OUTPUT]=createOutputCentered<PJ301MPort>(Vec(1122.05, 368.60), module, ModeScaleQuant::OUT_EXT_POLY_SCALE_OUTPUT);
			addOutput(outPortWidgets[ModeScaleQuant::OUT_EXT_POLY_SCALE_OUTPUT]);

			outPortWidgets[ModeScaleQuant::OUT_EXT_POLY_QUANT_OUTPUT]=createOutputCentered<PJ301MPort>(Vec(206.69, 1018.70), module, ModeScaleQuant::OUT_EXT_POLY_QUANT_OUTPUT);
			addOutput(outPortWidgets[ModeScaleQuant::OUT_EXT_POLY_QUANT_OUTPUT]);

			outPortWidgets[ModeScaleQuant::OUT_EXT_ROOT_OUTPUT]=createOutputCentered<PJ301MPort>(Vec(109.25, 959.65), module, ModeScaleQuant::OUT_EXT_ROOT_OUTPUT);
			addOutput(outPortWidgets[ModeScaleQuant::OUT_EXT_ROOT_OUTPUT]);
			
			outPortWidgets[ModeScaleQuant::OUT_EXT_SCALE_OUTPUT]=createOutputCentered<PJ301MPort>(Vec(168.31, 1033.46), module, ModeScaleQuant::OUT_EXT_SCALE_OUTPUT);
			addOutput(outPortWidgets[ModeScaleQuant::OUT_EXT_SCALE_OUTPUT]);
					
			outPortWidgets[ModeScaleQuant::OUT_EXT_POLY_QUANT_TRIGGER_OUTPUT]=createOutputCentered<PJ301MPort>(Vec(250.98, 1018.70), module, ModeScaleQuant::OUT_EXT_POLY_QUANT_TRIGGER_OUTPUT);
			addOutput(outPortWidgets[ModeScaleQuant::OUT_EXT_POLY_QUANT_TRIGGER_OUTPUT]);
											
			//**********************************

			// now, procedurally rearrange the control param panel locations
			Vec CircleCenter= Vec(mm2px(116.75),mm2px(67.75));
			float OuterCircleRadius=mm2px(39);
			float InnerCircleRadius3=mm2px(26);
					
			// re-layout the circle BUTTONS and LIGHTS programatically  // does not draw circles and segments and text.  That is done by drawCircleOf5ths()
			for(int i = 0; i<MAX_NOTES;++i) 
			{
				const float rotate90 = (M_PI) / 2.0;
							
				double startDegree = (M_PI * 2.0 * ((double)i - 0.5) / MAX_NOTES) - rotate90;
				double endDegree = (M_PI * 2.0 * ((double)i + 0.5) / MAX_NOTES) - rotate90;
				double x1= cos(startDegree) * InnerCircleRadius3 + CircleCenter.x;
				double y1= sin(startDegree) * InnerCircleRadius3 + CircleCenter.y;
				double x2= cos(endDegree) * InnerCircleRadius3 + CircleCenter.x;
				double y2= sin(endDegree) * InnerCircleRadius3 + CircleCenter.y;
				Vec radialLine1=Vec(x1,y1).minus(CircleCenter);
				Vec radialLine2=Vec(x2,y2).minus(CircleCenter);
				Vec centerLine=(radialLine1.plus(radialLine2)).div(2.);
				
				Vec radialDirection=centerLine.normalize();

								
				Vec controlPosition=CircleCenter.plus(radialDirection.mult(OuterCircleRadius*.78f));
				controlPosition=controlPosition.minus((paramWidgets[ModeScaleQuant::BUTTON_CIRCLESTEP_C_PARAM+i]->box.size).div(2.));  // adjust for box size
				paramWidgets[ModeScaleQuant::BUTTON_CIRCLESTEP_C_PARAM+i]->box.pos=controlPosition;
				
				controlPosition=CircleCenter.plus(radialDirection.mult(OuterCircleRadius*.78f));
				controlPosition=controlPosition.minus((lightWidgets[ModeScaleQuant::LIGHT_LEDBUTTON_CIRCLESTEP_1+i]->box.size).div(2.));  // adjust for box size
				lightWidgets[ModeScaleQuant::LIGHT_LEDBUTTON_CIRCLESTEP_1+i]->box.pos=controlPosition;
				

				controlPosition=CircleCenter.plus(radialDirection.mult(OuterCircleRadius*.61f));
				controlPosition=controlPosition.minus((lightWidgets[ModeScaleQuant::LIGHT_CIRCLE_ROOT_KEY_POSITION_1_LIGHT+i]->box.size).div(2.));
				lightWidgets[ModeScaleQuant::LIGHT_CIRCLE_ROOT_KEY_POSITION_1_LIGHT+i]->box.pos=controlPosition;
			}
 
		
			// relayout all param controls and lights 
			Vec drawCenter=Vec(42., RootKey_row);
			paramWidgets[ModeScaleQuant::CONTROL_ROOT_KEY_PARAM]->box.pos=drawCenter.minus(paramWidgets[ModeScaleQuant::CONTROL_ROOT_KEY_PARAM]->box.size.div(2.));

            
			drawCenter=Vec(42.,  ScaleSelect_row);
			paramWidgets[ModeScaleQuant::CONTROL_SCALE_PARAM]->box.pos=drawCenter.minus(paramWidgets[ModeScaleQuant::CONTROL_SCALE_PARAM]->box.size.div(2.));
													
						
			// re-layout all input ports.  Work around parm and input enum value mismatch due to history
			
			for (int i=0; i<ModeScaleQuant::NUM_INPUTS; ++i)
			{
				if (i==ModeScaleQuant::IN_ROOT_KEY_EXT_CV)
				{
					if ((inPortWidgets[i]!=NULL)&&(paramWidgets[i]!=NULL))
					{
						inPortWidgets[i]->box.pos= paramWidgets[i]->box.pos.minus(Vec(20,-1));
					}
				}
				else
				if (i==ModeScaleQuant::IN_SCALE_EXT_CV)
				{
					if ((inPortWidgets[i]!=NULL)&&(paramWidgets[i]!=NULL))
					{
						inPortWidgets[i]->box.pos= paramWidgets[i]->box.pos.minus(Vec(20,-1));
					}
				}
				else
				if (i==ModeScaleQuant::IN_POLY_QUANT_EXT_CV)
				{
					drawCenter=Vec(20., Quantizer_row);
					inPortWidgets[ModeScaleQuant::IN_POLY_QUANT_EXT_CV]->box.pos=drawCenter.minus(inPortWidgets[ModeScaleQuant::IN_POLY_QUANT_EXT_CV]->box.size.div(2.));
				}
							
			}
			

			// re-layout all output port
		
			drawCenter=Vec(50., 365.);
			outPortWidgets[ModeScaleQuant::OUT_HARMONY_CV_OUTPUT]->box.pos=drawCenter.minus(outPortWidgets[ModeScaleQuant::OUT_HARMONY_CV_OUTPUT]->box.size.div(2.));
			drawCenter=drawCenter.plus(Vec(40,0));
			outPortWidgets[ModeScaleQuant::OUT_HARMONY_GATE_OUTPUT]->box.pos=drawCenter.minus(outPortWidgets[ModeScaleQuant::OUT_HARMONY_GATE_OUTPUT]->box.size.div(2.));
		
			drawCenter=Vec(146., PolyExtScale_row);
			outPortWidgets[ModeScaleQuant::OUT_EXT_POLY_SCALE_OUTPUT]->box.pos=drawCenter.minus(outPortWidgets[ModeScaleQuant::OUT_EXT_POLY_SCALE_OUTPUT]->box.size.div(2.));
										
			drawCenter=Vec(50., Quantizer_row);  
			outPortWidgets[ModeScaleQuant::OUT_EXT_POLY_QUANT_OUTPUT]->box.pos=drawCenter.minus(outPortWidgets[ModeScaleQuant::OUT_EXT_POLY_QUANT_OUTPUT]->box.size.div(2.));
		
			drawCenter=Vec(50.0 + 40.0, Quantizer_row);  
			outPortWidgets[ModeScaleQuant::OUT_EXT_POLY_QUANT_TRIGGER_OUTPUT]->box.pos=drawCenter.minus(outPortWidgets[ModeScaleQuant::OUT_EXT_POLY_QUANT_TRIGGER_OUTPUT]->box.size.div(2.));
		
			drawCenter=Vec(145., RootKey_row);   
			outPortWidgets[ModeScaleQuant::OUT_EXT_ROOT_OUTPUT]->box.pos=drawCenter.minus(outPortWidgets[ModeScaleQuant::OUT_EXT_ROOT_OUTPUT]->box.size.div(2.));
		
			drawCenter=Vec(146., ScaleSelect_row+37); 
			outPortWidgets[ModeScaleQuant::OUT_EXT_SCALE_OUTPUT]->box.pos=drawCenter.minus(outPortWidgets[ModeScaleQuant::OUT_EXT_SCALE_OUTPUT]->box.size.div(2.));
				
			
			//********************
			
			for (int i=0; ((i<ModeScaleQuant::NUM_PARAMS)&&(i<MAX_PARAMS)); ++i)  // get the paramWidget box into a ModeScaleQuantWidget array so it can be accessed as needed
			{
				if (paramWidgets[i]!=NULL) 
					ParameterRect[i]=paramWidgets[i]->box;
			}

			for (int i=0; ((i<ModeScaleQuant::NUM_OUTPUTS)&&(i<MAX_OUTPORTS)); ++i)  // get the paramWidget box into a ModeScaleQuantWidget array so it can be accessed as needed
			{
				if (outPortWidgets[i]!=NULL) 
					OutportRect[i]=outPortWidgets[i]->box;
			}

			for (int i=0; ((i<ModeScaleQuant::NUM_INPUTS)&&(i<MAX_INPORTS)); ++i)  // get the paramWidget box into a ModeScaleQuantWidget array so it can be accessed as needed
			{
				if (inPortWidgets[i]!=NULL) 
					InportRect[i]=inPortWidgets[i]->box;
			}
				
		}  // end if (true)  

	}    // end ModeScaleQuantWidget(ModeScaleQuant* module)  

 
	// create panel theme and contrast control
	
	void appendContextMenu(Menu *menu) override 
	{  
        ModeScaleQuant *module = dynamic_cast<ModeScaleQuant*>(this->module);
		if (module==NULL)
			return;
   
		MenuLabel *panelthemeLabel = new MenuLabel();
        panelthemeLabel->text = "Panel Theme                               ";
        menu->addChild(panelthemeLabel);

		ModeScaleQuantPanelThemeItem *lightpaneltheme_Item = new ModeScaleQuantPanelThemeItem();  // this accomodates json loaded value
        lightpaneltheme_Item->text = "  light";
		lightpaneltheme_Item->module = module;
   	    lightpaneltheme_Item->theme = 0;
	    menu->addChild(lightpaneltheme_Item); 

		ModeScaleQuantPanelThemeItem *darkpaneltheme_Item = new ModeScaleQuantPanelThemeItem();  // this accomodates json loaded value
        darkpaneltheme_Item->text = "  dark";
		darkpaneltheme_Item->module = module;
   	    darkpaneltheme_Item->theme = 1;
        menu->addChild(darkpaneltheme_Item);

		// create contrast control
	
		MinMaxSliderItem *minSliderItem = new MinMaxSliderItem(&MSQ_panelContrast, "Contrast");
		minSliderItem->box.size.x = 200.f;
		menu->addChild(minSliderItem);
	
		//

		MenuLabel *modeLabel = new MenuLabel();
        modeLabel->text = "Scale Out Mode                               ";
        menu->addChild(modeLabel);
		
		
		ModeScaleQuantScaleOutModeItem *chromatic_Item = new ModeScaleQuantScaleOutModeItem();
        chromatic_Item->text = "  Heptatonic Chromatic Scale-12ch";
        chromatic_Item->module = module;
        chromatic_Item->mode = ModeScaleQuant::HEPTATONIC_CHROMATIC_12CH;
        menu->addChild(chromatic_Item);

		ModeScaleQuantScaleOutModeItem *heptatonic_Item = new ModeScaleQuantScaleOutModeItem(); 
        heptatonic_Item->text = "  Heptatonic Diatonic STD-7ch";
        heptatonic_Item->module = module;
        heptatonic_Item->mode = ModeScaleQuant::HEPTATONIC_DIATONIC_STD_7CH;
        menu->addChild(heptatonic_Item);


		ModeScaleQuantScaleOutModeItem *pentatonic_Item = new ModeScaleQuantScaleOutModeItem();
        pentatonic_Item->text = "  Pentatonic-5ch";
        pentatonic_Item->module =module;
        pentatonic_Item->mode = ModeScaleQuant::PENTATONIC_5CH;
        menu->addChild(pentatonic_Item); 
		

		ModeScaleQuantScaleOutModeItem *chromatic_pentatonic_Item = new ModeScaleQuantScaleOutModeItem();
        chromatic_pentatonic_Item->text = "  Pentatonic Chromatic-12ch";
        chromatic_pentatonic_Item->module = module;
        chromatic_pentatonic_Item->mode = ModeScaleQuant::PENTATONIC_CHROMATIC_12CH;
        menu->addChild(chromatic_pentatonic_Item);
		
	}  // end ModeScaleQuantWidget() 

	void step() override   // note, this is a widget step() which is not deprecated and is a GUI call.  This advances UI by one "frame"    
	{  
		ModeScaleQuant *module = dynamic_cast<ModeScaleQuant*>(this->module);  

		if (true) // needs to happen even if module==null
		{
			if (svgPanel)
			    svgPanel->setVisible((MSQ_panelTheme) == 0);    
			if (darkPanel)                             
				darkPanel->setVisible((MSQ_panelTheme) == 1);    
		
			float contrast=MSQ_panelContrast;

			// update the global panel theme vars
			if (MSQ_panelTheme==0)  // light theme
			{
				MSQ_panelcolor=nvgRGBA((unsigned char)230,(unsigned char)230,(unsigned char)230,(unsigned char)255);
				float color =255*(1-contrast);
				MSQ_panelTextColor=nvgRGBA((unsigned char)color,(unsigned char)color,(unsigned char)color,(unsigned char)255);  // black text
				MSQ_panelLineColor=nvgRGBA((unsigned char)color,(unsigned char)color,(unsigned char)color,(unsigned char)255);  // black lines
				color =255*contrast;
				MSQ_paramTextColor=nvgRGBA((unsigned char)color,(unsigned char)color,(unsigned char)0,(unsigned char)255);  // yellow text
			
				{
					float r=MSQ_panelHarmonyPartBaseColor.r; 
					float g=(1-contrast);
					float b=(1-contrast);
					MSQ_panelHarmonyPartColor=nvgRGBA(r*156, g*255, b*255, 255);
				}
				{
					float r=(1-contrast);
					float g=(1-contrast);
					float b=MSQ_panelArpPartBaseColor.b;
					MSQ_panelArpPartColor=nvgRGBA(r*255, g*255, b*255, 255);
				}
				{
					float r=(1-contrast);
					float g=MSQ_panelBassPartBaseColor.g;
					float b=(1-contrast);
					MSQ_panelBassPartColor=nvgRGBA(r*255, g*128, b*255, 255);
				}
				{
					float r=(1-contrast);
					float g=(1-contrast);
					float b=(1-contrast);
					MSQ_panelMelodyPartColor=nvgRGBA(r*255, g*255, b*255, 255);
				}
						
			}
			else  // dark theme
			{
				MSQ_panelcolor=nvgRGBA((unsigned char)40,(unsigned char)40,(unsigned char)40,(unsigned char)255);
				float color = 255*contrast;
				MSQ_panelTextColor=nvgRGBA((unsigned char)color,(unsigned char)color,(unsigned char)color,(unsigned char)255);  // white text
				MSQ_panelLineColor=nvgRGBA((unsigned char)color,(unsigned char)color,(unsigned char)color,(unsigned char)255);  // white lines
				color =255*contrast;
				MSQ_paramTextColor=nvgRGBA((unsigned char)color,(unsigned char)color,(unsigned char)0,(unsigned char)255);  // yellow text

				{
					float r=MSQ_panelHarmonyPartBaseColor.r*contrast;
					float g=0.45;
					float b=0.45;
					MSQ_panelHarmonyPartColor=nvgRGBA(r*255, g*255, b*255, 255);
				}
				{
					float b=MSQ_panelArpPartBaseColor.b*contrast;
					float r=0.45;
					float g=0.45;
					MSQ_panelArpPartColor=nvgRGBA(r*255, g*255, b*255, 255);
				}
				{
					float g=MSQ_panelBassPartBaseColor.g*contrast;
					float r=0.45;
					float b=0.45;
					MSQ_panelBassPartColor=nvgRGBA(r*255, g*255, b*255, 255);
				}
				{
					float r=(contrast);
					float g=(contrast);
					float b=(contrast);
					MSQ_panelMelodyPartColor=nvgRGBA(r*228, g*228, b*228, 255);
				}
			}
		}

	
		// root inport cable handling 
		if (module != NULL)  // not in the browser
		{
            module->onResetScale();  // make sure channels and scale notes outports are initialized for each frame, in case they have not been iniitialized

			for (CableWidget* cwIn : APP->scene->rack->getCablesOnPort(inPortWidgets[ModeScaleQuant::IN_ROOT_KEY_EXT_CV]))  // look at each cable on the root key input port. There should be 0 or 1  cables on an input port.
			{
				if (!cwIn->isComplete())    // the cable connection has NOT been completed
				{        
				   module->theModeScaleQuantState.RootInputSuppliedByRootOutput=false;	
                   continue;
				}

				engine:: Cable* cable=cwIn->getCable();

				if (cable)  // there is a cable connected to IN_ROOT_KEY_EXT_CV
				{
					int inputId = cable->inputId;
					int outputId = cable->outputId;
				
					Module * 	outputModule = cable->outputModule;  // cable output module
					if (outputModule)
					{
						plugin::Model *outputmodel = outputModule->getModel(); 
						if (outputmodel)
						{
						//	if ((outputmodel->slug.substr(0, 14) == std::string("ModeScaleQuant"))|| 
						//	    (outputmodel->slug.substr(0, 7) == std::string("Meander")))  
						    if ((outputmodel->slug.substr(0, 21) == std::string("ModeScaleProgressions")) || 
							    (outputmodel->slug.substr(0, 14) == std::string("ModeScaleQuant")) ||
							    (outputmodel->slug.substr(0, 7) == std::string("Meander")))  
							{
							//	if ((outputId==4)||(outputId==26))  // "cable outputID is OUT_EXT_ROOT_OUTPUT" kludge out of scope ModeScaleQuant variable access
								if ((outputId==4)||(outputId==26)||(outputId==15))  // "cable outputID is OUT_EXT_ROOT_OUTPUT" kludge out of scope ModeScaleQuant variable access
								{
									module->theModeScaleQuantState.RootInputSuppliedByRootOutput=true;  // but may be made false based on cable input
								}
							}
							else // connected to moudule other than ModeScaleQuant
							{
								module->theModeScaleQuantState.RootInputSuppliedByRootOutput=false;
							}
						}
						else  // no outputModel
						{
							module->theModeScaleQuantState.RootInputSuppliedByRootOutput=false;
						}
					}
					else  // no outputModule
					{
						module->theModeScaleQuantState.RootInputSuppliedByRootOutput=false;
					}

					Module * 	inputModule = cable->inputModule;    // cable input module
					if (inputModule)
					{
						if ((inputModule!=outputModule))  //"cable inputModule is NOT equal to cable outputModule"
						{
							plugin::Model *inputmodel = inputModule->getModel(); 	
							if (inputmodel)
							{
							//	if ((inputmodel->slug.substr(0, 14) == std::string("ModeScaleQuant"))||  
							//	    (inputmodel->slug.substr(0, 7) == std::string("Meander")))  
							    if ((inputmodel->slug.substr(0, 21) == std::string("ModeScaleProgressions")) || 
							       (inputmodel->slug.substr(0, 14) == std::string("ModeScaleQuant")) ||
							       (inputmodel->slug.substr(0, 7) == std::string("Meander")))  
								{
									if (inputId==ModeScaleQuant::IN_ROOT_KEY_EXT_CV)  // "cable inputID is  IN_ROOT_KEY_EXT_CV"
									{
									}
									else  // "cable inputID is not IN_ROOT_KEY_EXT_CV"
									{
										module->theModeScaleQuantState.RootInputSuppliedByRootOutput=false;
									}
								}
								else  // connected to moudule other than Maender
								{
									module->theModeScaleQuantState.RootInputSuppliedByRootOutput=false;
								}
							}
							else  // no input model
							{
								module->theModeScaleQuantState.RootInputSuppliedByRootOutput=false;
							}
						}
						else  // "cable inputModule is equal to cable outputModule"
						{
							module->theModeScaleQuantState.RootInputSuppliedByRootOutput=false;
						}
					}
					else  // no input module
					{
						module->theModeScaleQuantState.RootInputSuppliedByRootOutput=false;
					}
				}
				else  // there is no cable attached
				{
					module->theModeScaleQuantState.RootInputSuppliedByRootOutput=false;
				}
			}
		}  

		// add mode inport cable handling 
		if (module != NULL)  // not in the browser
		{
			//"Examine module MODE cables--------------------"
			for (CableWidget* cwIn : APP->scene->rack->getCablesOnPort(inPortWidgets[ModeScaleQuant::IN_SCALE_EXT_CV]))  // look at each cable on the mode scale input port. There should be 0 or 1  cables on an input port.
			{
				if (!cwIn->isComplete())   // the cable connection has NOT been completed    
				{      
				   module->theModeScaleQuantState.ModeInputSuppliedByModeOutput=false;	
                   continue;
				}

				engine:: Cable* cable=cwIn->getCable();

				if (cable)  // there is a cable connected to IN_SCALE_EXT_CV
				{
					int inputId = cable->inputId;
					int outputId = cable->outputId;
				
					Module * 	outputModule = cable->outputModule;  // cable output module
					if (outputModule)
					{
						plugin::Model *outputmodel = outputModule->getModel(); 
						if (outputmodel)
						{
                       	    if ((outputmodel->slug.substr(0, 21) == std::string("ModeScaleProgressions")) || 
							    (outputmodel->slug.substr(0, 14) == std::string("ModeScaleQuant")) ||
							    (outputmodel->slug.substr(0, 7) == std::string("Meander")))  
							{
								if ((outputId==5)||(outputId==27)||(outputId==16))  // "cable outputID is OUT_EXT_SCALE_OUTPUT" kludge out of scope ModeScaleQuant variable access
								{
									module->theModeScaleQuantState.ModeInputSuppliedByModeOutput=true;  // but may be made false based on cable input
								}
							}
							else // connected to moudule other than ModeScaleQuant
							{
								module->theModeScaleQuantState.ModeInputSuppliedByModeOutput=false;
							}
						}
						else  // no outputModel
						{
							module->theModeScaleQuantState.ModeInputSuppliedByModeOutput=false;
						}
					}
					else  // no outputModule
					{
						module->theModeScaleQuantState.ModeInputSuppliedByModeOutput=false;
					}

					Module * 	inputModule = cable->inputModule;    // cable input module
					if (inputModule)
					{
						if ((inputModule!=outputModule))  //"cable inputModule is NOT equal to cable outputModule"
						{
							plugin::Model *inputmodel = inputModule->getModel(); 	
							if (inputmodel)
							{
							    if ((inputmodel->slug.substr(0, 21) == std::string("ModeScaleProgressions")) || 
							       (inputmodel->slug.substr(0, 14) == std::string("ModeScaleQuant")) ||
							       (inputmodel->slug.substr(0, 7) == std::string("Meander")))  
								{
									if (inputId==ModeScaleQuant::IN_SCALE_EXT_CV)  // "cable inputID is  IN_SCALE_EXT_CV"
									{
									}
									else // "cable inputID is not IN_SCALE_EXT_CV"
									{
										module->theModeScaleQuantState.ModeInputSuppliedByModeOutput=false;
									}
								}
								else  // connected to moudule other than Maender
								{
									module->theModeScaleQuantState.ModeInputSuppliedByModeOutput=false;
								}
							}
							else  // no input model
							{
								module->theModeScaleQuantState.ModeInputSuppliedByModeOutput=false;
							}
						}
						else  // "cable inputModule is equal to cable outputModule"
						{
							module->theModeScaleQuantState.ModeInputSuppliedByModeOutput=false;
						}
					}
					else  // no input module
					{
						module->theModeScaleQuantState.ModeInputSuppliedByModeOutput=false;
					}
				}
				else  // there is no cable attached
				{
					module->theModeScaleQuantState.ModeInputSuppliedByModeOutput=false;
				}
			}
		
		} 
		//

		// if in the browser, force a panel redraw per frame with the current panel theme
		if (!module) {
			DirtyEvent eDirty;
			parent->parent->onDirty(eDirty);
		}
		else  // not in the browser
		Widget::step();  // most modules do this rather than ModuleWidget::step()
	
		
	} // end step() 

};  // end struct ModeScaleQuantWidget



Model* modelModeScaleQuant = createModel<ModeScaleQuant, ModeScaleQuantWidget>("ModeScaleQuant");    


