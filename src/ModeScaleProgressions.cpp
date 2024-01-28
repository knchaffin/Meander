/*  Copyright (C) 2019-2024 Ken ChaffintheArpParms  
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
#include <mutex>

#include "ModeScaleProgressions.hpp"  

struct ModeScaleProgressions : Module  
{
	// poly quant vars and functions
	
	bool polyQuant_scaleNotes[12];
	int polyQuant_searchSpaces[24];
	bool polyQuant_outputNotes[24];

	int ModeScaleProgressionsScale[7]; // heptatonic scale
	
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
				for (int i=0;i<MSP_mode_step_intervals[mode][0];++i)
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
				for (int i=0;i<MSP_mode_step_intervals[mode][0];++i)
				{
					int note=(int)(notes[i]%(MAX_NOTES));  
					outputs[OUT_EXT_POLY_SCALE_OUTPUT].setVoltage((float)note/12.0,i);  // (scale note, channel) 
				}
			}
			
			if ((scale_out_mode == PENTATONIC_5CH)&&((mode==0)||(mode==1)||(mode==2)))   // major modes
			{
				outputs[OUT_EXT_POLY_SCALE_OUTPUT].setChannels(5);  // set polyphony to 5 channels
				int scale_note_index=0;
				for (int i=0;i<MSP_mode_step_intervals[mode][0];++i)
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
				for (int i=0;i<MSP_mode_step_intervals[mode][0];++i)
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
				for (int i=0;i<MSP_mode_step_intervals[mode][0];++i)
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
				for (int i=0;i<MSP_mode_step_intervals[mode][0];++i)
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
		// get ModeScaleProgressions scale
		for (int i=0;((i<7)&&(i<MSP_mode_step_intervals[mode][0]));++i)
		{
			int note=(int)(notes[i]%12);  
			ModeScaleProgressionsScale[i]=note;
		}

		for (int i = 0; i < 12; i++) // initialize clear scale notes
		{
			polyQuant_scaleNotes[i] = false;
		} 

		// get ModeScaleProgressions heptatonic scale
		
		for (int i=0;i<7;++i)
		{
			int scale_note=ModeScaleProgressionsScale[i];
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
		BUTTON_RUN_PARAM,
		BUTTON_RESET_PARAM,

		CONTROL_TEMPOBPM_PARAM,
		CONTROL_TIMESIGNATURETOP_PARAM,
		CONTROL_TIMESIGNATUREBOTTOM_PARAM,

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
	
		BUTTON_HARMONY_SETSTEP_1_PARAM,
		BUTTON_HARMONY_SETSTEP_2_PARAM,
		BUTTON_HARMONY_SETSTEP_3_PARAM,
		BUTTON_HARMONY_SETSTEP_4_PARAM,
		BUTTON_HARMONY_SETSTEP_5_PARAM,
		BUTTON_HARMONY_SETSTEP_6_PARAM,
		BUTTON_HARMONY_SETSTEP_7_PARAM,
		BUTTON_HARMONY_SETSTEP_8_PARAM,
		BUTTON_HARMONY_SETSTEP_9_PARAM,
		BUTTON_HARMONY_SETSTEP_10_PARAM,
		BUTTON_HARMONY_SETSTEP_11_PARAM,
		BUTTON_HARMONY_SETSTEP_12_PARAM,
		BUTTON_HARMONY_SETSTEP_13_PARAM,
		BUTTON_HARMONY_SETSTEP_14_PARAM,
		BUTTON_HARMONY_SETSTEP_15_PARAM,
		BUTTON_HARMONY_SETSTEP_16_PARAM,
    
		BUTTON_ENABLE_HARMONY_PARAM,
		BUTTON_HARMONY_DESTUTTER_PARAM,
		CONTROL_HARMONY_VOLUME_PARAM,
		CONTROL_HARMONY_STEPS_PARAM,
		CONTROL_HARMONY_TARGETOCTAVE_PARAM,
		CONTROL_HARMONY_ALPHA_PARAM,
		CONTROL_HARMONY_RANGE_PARAM,
		CONTROL_HARMONY_DIVISOR_PARAM,
		CONTROL_HARMONYPRESETS_PARAM,
  
		BUTTON_ENABLE_HARMONY_ALL7THS_PARAM,
		BUTTON_ENABLE_HARMONY_V7THS_PARAM,
		BUTTON_ENABLE_HARMONY_STACCATO_PARAM,
		BUTTON_PROG_STEP_PARAM,
		BUTTON_ENABLE_HARMONY_4VOICE_OCTAVES_PARAM,
		BUTTON_ENABLE_HARMONY_TONIC_ON_CH1_PARAM,
		BUTTON_ENABLE_HARMONY_BASS_ON_CH1_PARAM,

		BUTTON_RAND_PARAM,
		
		NUM_PARAMS
	};

	enum InputIds 
	{
		IN_RUN_EXT_CV,
		IN_RESET_EXT_CV,
		IN_TEMPO_EXT_CV,
		IN_TIMESIGNATURETOP_EXT_CV,
		IN_TIMESIGNATUREBOTTOM_EXT_CV,
		IN_ROOT_KEY_EXT_CV,
		IN_SCALE_EXT_CV,
		IN_CLOCK_EXT_CV,
		IN_HARMONY_CIRCLE_DEGREE_EXT_CV,
		IN_HARMONY_CIRCLE_DEGREE_GATE_EXT_CV,

		IN_HARMONY_ENABLE_EXT_CV,
		IN_HARMONY_DESTUTTER_EXT_CV,
		IN_HARMONY_VOLUME_EXT_CV,
		IN_HARMONY_STEPS_EXT_CV,
		IN_HARMONY_TARGETOCTAVE_EXT_CV,
		IN_HARMONY_ALPHA_EXT_CV,
		IN_HARMONY_RANGE_EXT_CV,
		IN_HARMONY_DIVISOR_EXT_CV,
		IN_HARMONYPRESETS_EXT_CV,
    
		IN_ENABLE_HARMONY_ALL7THS_EXT_CV,
		IN_ENABLE_HARMONY_V7THS_EXT_CV,

		IN_ENABLE_HARMONY_STACCATO_EXT_CV,
	
		IN_PROG_STEP_EXT_CV,

		IN_POLY_QUANT_EXT_CV,

		IN_ENABLE_HARMONY_4VOICE_OCTAVES_EXT_CV,

		IN_ENABLE_HARMONY_TONIC_ON_CH1_EXT_CV,
		IN_ENABLE_HARMONY_BASS_ON_CH1_EXT_CV,

		IN_RAND_EXT_CV,
		
		NUM_INPUTS
		
	};
	
	

	enum OutputIds 
	{
		OUT_RUN_OUT,
		OUT_RESET_OUT,
		OUT_TEMPO_OUT,
		OUT_CLOCK_OUT, 
		OUT_HARMONY_GATE_OUTPUT,
		OUT_HARMONY_CV_OUTPUT,
		OUT_CLOCK_BEATX2_OUTPUT,
		OUT_CLOCK_BAR_OUTPUT,
		OUT_CLOCK_BEATX4_OUTPUT,
		OUT_CLOCK_BEATX8_OUTPUT,
		OUT_CLOCK_BEAT_OUTPUT,
		OUT_HARMONY_TRIGGER_OUTPUT,
		OUT_HARMONY_VOLUME_OUTPUT,
		OUT_EXT_POLY_SCALE_OUTPUT,
		OUT_EXT_POLY_QUANT_OUTPUT,
		OUT_EXT_ROOT_OUTPUT,
		OUT_EXT_SCALE_OUTPUT,
		OUT_EXT_HARMONIC_DEGREE_OUTPUT,
		OUT_EXT_POLY_QUANT_TRIGGER_OUTPUT,
		OUT_EXT_HARMONIC_DEGREE_CHORD_TYPE_OUTPUT,
			
		NUM_OUTPUTS
	};

	enum LightIds 
	{
		LIGHT_LEDBUTTON_HARMONY_ENABLE,
		LIGHT_LEDBUTTON_MELODY_ENABLE,
		LIGHT_LEDBUTTON_RUN,
		LIGHT_LEDBUTTON_RESET,
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
		LIGHT_LEDBUTTON_CIRCLESETSTEP_1,
		LIGHT_LEDBUTTON_CIRCLESETSTEP_2,
		LIGHT_LEDBUTTON_CIRCLESETSTEP_3,
		LIGHT_LEDBUTTON_CIRCLESETSTEP_4,
		LIGHT_LEDBUTTON_CIRCLESETSTEP_5,
		LIGHT_LEDBUTTON_CIRCLESETSTEP_6, 
		LIGHT_LEDBUTTON_CIRCLESETSTEP_7,
		LIGHT_LEDBUTTON_CIRCLESETSTEP_8,
		LIGHT_LEDBUTTON_CIRCLESETSTEP_9,
		LIGHT_LEDBUTTON_CIRCLESETSTEP_10,
		LIGHT_LEDBUTTON_CIRCLESETSTEP_11,
		LIGHT_LEDBUTTON_CIRCLESETSTEP_12,
		LIGHT_LEDBUTTON_CIRCLESETSTEP_13,
		LIGHT_LEDBUTTON_CIRCLESETSTEP_14,
		LIGHT_LEDBUTTON_CIRCLESETSTEP_15,
		LIGHT_LEDBUTTON_CIRCLESETSTEP_16,
		LIGHT_CLOCK_IN_LIGHT,
	
		LIGHT_LEDBUTTON_ENABLE_HARMONY_ALL7THS_PARAM,
		LIGHT_LEDBUTTON_ENABLE_HARMONY_V7THS_PARAM,
		LIGHT_LEDBUTTON_ENABLE_HARMONY_STACCATO_PARAM,
		LIGHT_LEDBUTTON_PROG_STEP_PARAM,
		LIGHT_LEDBUTTON_ENABLE_HARMONY_4VOICE_OCTAVES_PARAM,

		LIGHT_LEDBUTTON_ENABLE_HARMONY_TONIC_ON_CH1_PARAM,
		LIGHT_LEDBUTTON_ENABLE_HARMONY_BASS_ON_CH1_PARAM,

		LIGHT_LEDBUTTON_RAND,
		
		NUM_LIGHTS
	};

	#include "MSP_data_and_functions.hpp" // for module vars   

	enum ScaleOutMode {
		HEPTATONIC_CHROMATIC_12CH,
		HEPTATONIC_DIATONIC_STD_7CH,
		PENTATONIC_5CH,
		PENTATONIC_CHROMATIC_12CH
	};

	ScaleOutMode scale_out_mode = HEPTATONIC_CHROMATIC_12CH;//  aria's poly ext. scale is default to match V1

	enum GateOutMode {
		STANDARD_GATE,
		VOLUME_OVER_GATE
	};
	
    GateOutMode gate_out_mode = STANDARD_GATE;

	enum HarmonicDegreeOutCvRangeMode
	{
		RANGE_1to7,
		RANGE_0to6
	};

	HarmonicDegreeOutCvRangeMode harmonic_degree_out_mode = RANGE_1to7;

	// Clock code adapted from Strum and AS

	struct ModeScaleProgressionsLFOGenerator 
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
	};  // struct ModeScaleProgressionsLFOGenerator 

	void userPlaysCirclePositionHarmony(int circle_position, float octaveOffset)  // C=0   play immediate
	{
		theModeScaleProgressionsState.last_harmony_chord_root_note=circle_of_fifths[circle_position];

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
						theModeScaleProgressionsState.last_harmony_step=j;
						break;
					}
				}
				break;
			} 
		} 
		
		theModeScaleProgressionsState.theMelodyParms.last_step=theModeScaleProgressionsState.last_harmony_step;
		int note_index=	(int)(theModeScaleProgressionsState.theMelodyParms.note_avg*num_step_chord_notes[theModeScaleProgressionsState.last_harmony_step]);		// not sure this is necessary
		note_index=clamp(note_index, 0, num_step_chord_notes[theModeScaleProgressionsState.last_harmony_step]-1);
		theModeScaleProgressionsState.theMelodyParms.last_chord_note_index= note_index;
		 
		int current_chord_note=0;
		int root_key_note=circle_of_fifths[circle_position]; 
		int circle_chord_type= theCircleOf5ths.Circle5ths[circle_position].chordType;
		theModeScaleProgressionsState.theHarmonyParms.last_chord_type=circle_chord_type;
		int num_chord_members=chord_type_num_notes[circle_chord_type];

   
		if (theModeScaleProgressionsState.theHarmonyParms.enable_4voice_octaves)
		{
			outputs[OUT_HARMONY_CV_OUTPUT].setChannels(4);  // set polyphony
		} 
		else
		if ((theModeScaleProgressionsState.theHarmonyParms.enable_all_7ths)||(theModeScaleProgressionsState.theHarmonyParms.enable_V_7ths))
		{			
			if ((circle_chord_type==2)
			||  (circle_chord_type==3)
			||  (circle_chord_type==4)
			||  (circle_chord_type==5))
				outputs[OUT_HARMONY_CV_OUTPUT].setChannels(4);  // set polyphony
			else
				outputs[OUT_HARMONY_CV_OUTPUT].setChannels(3);  // set polyphony
		}
		else // default
		{
			outputs[OUT_HARMONY_CV_OUTPUT].setChannels(3);  // set polyphony
		}

		if (valid_current_circle_degree)
	    {
			if (harmonic_degree_out_mode == RANGE_1to7)
				outputs[OUT_EXT_HARMONIC_DEGREE_OUTPUT].setVoltage(theModeScaleProgressionsState.circleDegree);  // for degree= 1-7
			else
				outputs[OUT_EXT_HARMONIC_DEGREE_OUTPUT].setVoltage(theModeScaleProgressionsState.circleDegree-1.0);  // for degree= 0-6

			outputs[OUT_EXT_HARMONIC_DEGREE_CHORD_TYPE_OUTPUT].setVoltage(circle_chord_type);  // chord type for circle scale degree
		}

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
					theModeScaleProgressionsState.theHarmonyParms.last[j].note=note_to_play;
					theModeScaleProgressionsState.theHarmonyParms.last[j].noteType=NOTE_TYPE_CHORD;
					theModeScaleProgressionsState.theHarmonyParms.last[j].length=theModeScaleProgressionsState.theHarmonyParms.note_length_divisor;  
					theModeScaleProgressionsState.theHarmonyParms.last[j].time32s=barts_count;
					theModeScaleProgressionsState.theHarmonyParms.last[j].countInBar=bar_note_count;
					theModeScaleProgressionsState.theHarmonyParms.last[j].isPlaying=true;
					if (bar_note_count<256)
					played_notes_circular_buffer[bar_note_count++]=theModeScaleProgressionsState.theHarmonyParms.last[j];
				}
			}
		}
	
	    if (valid_current_circle_degree)
		{
		  harmonyGatePulse.reset();  // kill the pulse in case it is active
		  harmonyGatePulse.trigger(.1);    // keep it short so re-trigger works quickly
		}

	}

	void doHarmony(int barChordNumber=1, bool playFlag=false)
	{
		outputs[OUT_HARMONY_VOLUME_OUTPUT].setVoltage(theModeScaleProgressionsState.theHarmonyParms.volume);
		
		clock_t current_cpu_t= clock();  // cpu clock ticks since program began
		double current_cpu_time_double= (double)(current_cpu_t) / (double)CLOCKS_PER_SEC;
		
															
		current_melody_note += 1.0/12.0;
		current_melody_note=fmod(current_melody_note, 1.0f);	

		
	
		for (int i=0; i<MAX_CIRCLE_STATIONS; ++i) {
			CircleStepStates[i] = false;
			lights[LIGHT_LEDBUTTON_CIRCLESTEP_1+i].setBrightness(0.0f);
		}

		for (int i=0; i<16; ++i) {
			if (i<theActiveHarmonyType.num_harmony_steps)
				lights[LIGHT_LEDBUTTON_CIRCLESETSTEP_1+i].setBrightness(0.25f);
			else
				lights[LIGHT_LEDBUTTON_CIRCLESETSTEP_1+i].setBrightness(0.0f);
				
		}
	
	
     	int step=(bar_count%theActiveHarmonyType.num_harmony_steps);  // 0-(n-1)
 
 		if ((harmony_type==22)&&(step==0)&&(barChordNumber==0))  // random coming home
		{
			float rnd = rack::random::uniform();
			int temp_num_harmony_steps=1 + (int)((rnd*(theHarmonyTypes[22].num_harmony_steps-1)));
			bar_count += (theHarmonyTypes[22].num_harmony_steps-temp_num_harmony_steps);
		}

		if (randomize_harmony) // this could be used to randomize any progression
		{
			if (barChordNumber==0)
			{
				float rnd = rack::random::uniform();
				step = (int)((rnd*theActiveHarmonyType.num_harmony_steps));
				step=step%theActiveHarmonyType.num_harmony_steps;
			}
			else
			{
				step=theModeScaleProgressionsState.theHarmonyParms.last_circle_step;
			}
		}
		else
		if (harmony_type==22) // random coming home
		{
			if (barChordNumber!=0)
			{
				step=theModeScaleProgressionsState.theHarmonyParms.last_circle_step;
			}
		}
		else
		if (harmony_type==23) // random order
		{
			if (barChordNumber==0)
			{
				float rnd = rack::random::uniform();
				step = (int)((rnd*theActiveHarmonyType.num_harmony_steps));
				step=step%theActiveHarmonyType.num_harmony_steps;
			}
			else
			{
				step=theModeScaleProgressionsState.theHarmonyParms.last_circle_step;
			}
		}
		else
		if ((harmony_type==31)||(harmony_type==42)||(harmony_type==43)||(harmony_type==44)||(harmony_type==45)||(harmony_type==46)||(harmony_type==47)||(harmony_type==48))  // Markov chains
		{   
			if (barChordNumber==0)
			{
				float rnd = rack::random::uniform();
				
				if (theModeScaleProgressionsState.theHarmonyParms.last_circle_step==-1)
				{
					step=0;
				}
				else
				{
					float probabilityTargetBottom[8]={0};  // skip first array index since this is 1 based
					float probabilityTargetTop[8]={0};     // skip first array index since this is 1 based
					float bottom=0;
					step=1;
					for (int i=1; i<8; ++i)  // skip first array index since this is 1 based
					{
						probabilityTargetBottom[i]=bottom;
						if (harmony_type==31)
							probabilityTargetTop[i]=bottom+MarkovProgressionTransitionMatrixBach1[theModeScaleProgressionsState.theHarmonyParms.last_circle_step+1][i];
						else
						if (harmony_type==42)
							probabilityTargetTop[i]=bottom+MarkovProgressionTransitionMatrixBach2[theModeScaleProgressionsState.theHarmonyParms.last_circle_step+1][i];
						else
						if (harmony_type==43)
							probabilityTargetTop[i]=bottom+MarkovProgressionTransitionMatrixMozart1[theModeScaleProgressionsState.theHarmonyParms.last_circle_step+1][i];
						else
						if (harmony_type==44)
							probabilityTargetTop[i]=bottom+MarkovProgressionTransitionMatrixMozart2[theModeScaleProgressionsState.theHarmonyParms.last_circle_step+1][i];
						else
						if (harmony_type==45)
							probabilityTargetTop[i]=bottom+MarkovProgressionTransitionMatrixPalestrina1[theModeScaleProgressionsState.theHarmonyParms.last_circle_step+1][i];
						else
						if (harmony_type==46)
							probabilityTargetTop[i]=bottom+MarkovProgressionTransitionMatrixBeethoven1[theModeScaleProgressionsState.theHarmonyParms.last_circle_step+1][i];
						else
						if (harmony_type==47)
							probabilityTargetTop[i]=bottom+MarkovProgressionTransitionMatrixTraditional1[theModeScaleProgressionsState.theHarmonyParms.last_circle_step+1][i];
						else
						if (harmony_type==48)
							probabilityTargetTop[i]=bottom+MarkovProgressionTransitionMatrix_I_IV_V[theModeScaleProgressionsState.theHarmonyParms.last_circle_step+1][i];
											
						
						bottom=probabilityTargetTop[i];
					}
				
					for (int i=1; i<8; ++i)  // skip first array index since this is 1 based
					{
						if ((rnd>probabilityTargetBottom[i])&&(rnd<= probabilityTargetTop[i]))
						{
							step=i-1;
						
						}
					}
				
				}
			}
			else
			{
				step=theModeScaleProgressionsState.theHarmonyParms.last_circle_step;
			}
			
		}
		 
    			

		int degreeStep=(theActiveHarmonyType.harmony_steps[step])%8;   
		valid_current_circle_degree=false;
		if ((degreeStep<1)||(degreeStep>7))
		  degreeStep=1;  // force to a valid degree to avoid a crash
		else
          valid_current_circle_degree=true;

		current_circle_degree = degreeStep;
		theModeScaleProgressionsState.circleDegree=(int)current_circle_degree;  
			
		theModeScaleProgressionsState.theHarmonyParms.last_circle_step=step;  // used for Markov chain

		//find this in semicircle
		for (int i=0; i<7; ++i)
		{
			if  (theCircleOf5ths.theDegreeSemiCircle.degreeElements[i].Degree==degreeStep)
			{
				current_circle_position = theCircleOf5ths.theDegreeSemiCircle.degreeElements[i].CircleIndex; 
				break;
			}
		}

		int circle_chord_type= theCircleOf5ths.Circle5ths[current_circle_position].chordType;
				
		lights[LIGHT_LEDBUTTON_CIRCLESETSTEP_1+step].setBrightness(1.0f);
		lights[LIGHT_LEDBUTTON_CIRCLESTEP_1+ (current_circle_position)%12].setBrightness(1.0f);
		
		double period=1.0/theModeScaleProgressionsState.theHarmonyParms.period; // 1/seconds
		double fBmarg=theModeScaleProgressionsState.theHarmonyParms.seed + (double)(period*current_cpu_time_double); 
	    double fBmrand=(FastfBm1DNoise(fBmarg,theModeScaleProgressionsState.theHarmonyParms.noctaves) +1.)/2; 
			
		theModeScaleProgressionsState.theHarmonyParms.note_avg = 
			(1.0-theModeScaleProgressionsState.theHarmonyParms.alpha)*theModeScaleProgressionsState.theHarmonyParms.note_avg + 
			theModeScaleProgressionsState.theHarmonyParms.alpha*(theModeScaleProgressionsState.theHarmonyParms.range_bottom + (fBmrand*theModeScaleProgressionsState.theHarmonyParms.r1));
					
		if (theModeScaleProgressionsState.theHarmonyParms.note_avg>theModeScaleProgressionsState.theHarmonyParms.range_top)
		theModeScaleProgressionsState.theHarmonyParms.note_avg=theModeScaleProgressionsState.theHarmonyParms.range_top;
		if (theModeScaleProgressionsState.theHarmonyParms.note_avg<theModeScaleProgressionsState.theHarmonyParms.range_bottom)
		theModeScaleProgressionsState.theHarmonyParms.note_avg=theModeScaleProgressionsState.theHarmonyParms.range_bottom;
					
		int step_chord_type= theCircleOf5ths.Circle5ths[current_circle_position].chordType;

		if (harmonic_degree_out_mode == RANGE_1to7)
			outputs[OUT_EXT_HARMONIC_DEGREE_OUTPUT].setVoltage(theModeScaleProgressionsState.circleDegree);  // for degree= 1-7
		else
			outputs[OUT_EXT_HARMONIC_DEGREE_OUTPUT].setVoltage(theModeScaleProgressionsState.circleDegree-1.0);  // for degree= 0-6

		outputs[OUT_EXT_HARMONIC_DEGREE_CHORD_TYPE_OUTPUT].setVoltage(circle_chord_type);  // chord type for circle scale degree

		if (theModeScaleProgressionsState.theHarmonyParms.enable_4voice_octaves)
		{
			outputs[OUT_HARMONY_CV_OUTPUT].setChannels(4);  // set polyphony
		}
		else
		if ((theModeScaleProgressionsState.theHarmonyParms.enable_all_7ths)||(theModeScaleProgressionsState.theHarmonyParms.enable_V_7ths))
		{			
			if ((theCircleOf5ths.Circle5ths[current_circle_position].chordType==2)
			||  (theCircleOf5ths.Circle5ths[current_circle_position].chordType==3)
			||  (theCircleOf5ths.Circle5ths[current_circle_position].chordType==4)
			||  (theCircleOf5ths.Circle5ths[current_circle_position].chordType==5))
				outputs[OUT_HARMONY_CV_OUTPUT].setChannels(4);  // set polyphony
			else
				outputs[OUT_HARMONY_CV_OUTPUT].setChannels(3);  // set polyphony
		}
		else  // default
		{
			outputs[OUT_HARMONY_CV_OUTPUT].setChannels(3);  // set polyphony
		}
		
		int num_chord_members=chord_type_num_notes[step_chord_type]; 
		
		theModeScaleProgressionsState.theHarmonyParms.last_chord_type=step_chord_type;
		theModeScaleProgressionsState.last_harmony_chord_root_note=circle_of_fifths[current_circle_position];
		theModeScaleProgressionsState.last_harmony_step=step;
					
		bool tonicFound=false; 
	
		for (int j=0;j<num_chord_members;++j) 
		{
				current_chord_notes[j]= step_chord_notes[step][(int)(theModeScaleProgressionsState.theHarmonyParms.note_avg*num_step_chord_notes[step])+j]; // may create inversion
				int note_to_play=current_chord_notes[j]-12;  // drop it an octave to get target octave right
				note_to_play=clamp(note_to_play, root_key, 108+root_key); // clamp to MIDI range root0 to (C8+root)
				
				if (j==0)
					theModeScaleProgressionsState.last_harmony_chord_bass_note=note_to_play;
											
				if (playFlag)  
				{
					if (j<4)
					{
						theModeScaleProgressionsState.theHarmonyParms.last[j].note=note_to_play;
						theModeScaleProgressionsState.theHarmonyParms.last[j].noteType=NOTE_TYPE_CHORD;
						theModeScaleProgressionsState.theHarmonyParms.last[j].length=theModeScaleProgressionsState.theHarmonyParms.note_length_divisor;
						theModeScaleProgressionsState.theHarmonyParms.last[j].time32s=barts_count;
						theModeScaleProgressionsState.theHarmonyParms.last[j].countInBar=bar_note_count;
						theModeScaleProgressionsState.theHarmonyParms.last[j].isPlaying=true;
						if (bar_note_count<256)
						played_notes_circular_buffer[bar_note_count++]=theModeScaleProgressionsState.theHarmonyParms.last[j];
					}
				
					//  get tonic in channel 0
											
					if ( (note_to_play%MAX_NOTES)==(theModeScaleProgressionsState.last_harmony_chord_root_note%MAX_NOTES))
					{
						if (theModeScaleProgressionsState.theHarmonyParms.send_tonic_on_first_channel)
							outputs[OUT_HARMONY_CV_OUTPUT].setVoltage((note_to_play/12.0)-4.0,0);  // (note, channel)
						else
							outputs[OUT_HARMONY_CV_OUTPUT].setVoltage((note_to_play/12.0)-4.0,j);  // (note, channel)
						tonicFound=true;
						if (theModeScaleProgressionsState.theHarmonyParms.enable_4voice_octaves)  // put the root octave note in channel 3 (4th)
						{
							note_to_play+=12;  // raise root an octave 
							note_to_play=clamp(note_to_play, root_key, 108+root_key); // clamp to MIDI range root0 to (C8+root)
							outputs[OUT_HARMONY_CV_OUTPUT].setVoltage((note_to_play/12.0)-4.0,3);  // (note, channel)
							theModeScaleProgressionsState.theHarmonyParms.last[3].note=note_to_play;
							theModeScaleProgressionsState.theHarmonyParms.last[3].noteType=NOTE_TYPE_CHORD;
							theModeScaleProgressionsState.theHarmonyParms.last[3].length=theModeScaleProgressionsState.theHarmonyParms.note_length_divisor;
							theModeScaleProgressionsState.theHarmonyParms.last[3].time32s=barts_count;
							theModeScaleProgressionsState.theHarmonyParms.last[3].countInBar=bar_note_count;
							theModeScaleProgressionsState.theHarmonyParms.last[3].isPlaying=true;
							if (bar_note_count<256)
								played_notes_circular_buffer[bar_note_count++]=theModeScaleProgressionsState.theHarmonyParms.last[3];
						}
					}
					else
					{
						if ((!tonicFound)&&(theModeScaleProgressionsState.theHarmonyParms.send_tonic_on_first_channel))
							outputs[OUT_HARMONY_CV_OUTPUT].setVoltage((note_to_play/12.0)-4.0,j+1);  // (note, channel)
						else
							outputs[OUT_HARMONY_CV_OUTPUT].setVoltage((note_to_play/12.0)-4.0,j);  // (note, channel)
					}
						
				}
		}
		
		if (playFlag)
		{ 
			outputs[OUT_HARMONY_VOLUME_OUTPUT].setVoltage(theModeScaleProgressionsState.theHarmonyParms.volume);
			outputs[OUT_EXT_HARMONIC_DEGREE_OUTPUT].setChannels(1);  // set polyphony  
			outputs[OUT_EXT_HARMONIC_DEGREE_CHORD_TYPE_OUTPUT].setChannels(1);  // chord type for circle scale degree
				
			float durationFactor=1.0;
			if (theModeScaleProgressionsState.theHarmonyParms.enable_staccato)
				durationFactor=0.5;
			else
				durationFactor=0.95;

					
			if (theModeScaleProgressionsState.theHarmonyParms.STEP_inport_connected_to_ModeScaleProgressions_trigger_port==OUT_CLOCK_BAR_OUTPUT)
				durationFactor*=1.0;
			else	
			if (theModeScaleProgressionsState.theHarmonyParms.STEP_inport_connected_to_ModeScaleProgressions_trigger_port==OUT_CLOCK_BEAT_OUTPUT)
				durationFactor*=.25;
			else	
			if (theModeScaleProgressionsState.theHarmonyParms.STEP_inport_connected_to_ModeScaleProgressions_trigger_port==OUT_CLOCK_BEATX2_OUTPUT)
				durationFactor*=.125;
			else	
			if (theModeScaleProgressionsState.theHarmonyParms.STEP_inport_connected_to_ModeScaleProgressions_trigger_port==OUT_CLOCK_BEATX4_OUTPUT)
				durationFactor*=.0625;	
			else
			if (theModeScaleProgressionsState.theHarmonyParms.STEP_inport_connected_to_ModeScaleProgressions_trigger_port==OUT_CLOCK_BEATX8_OUTPUT)
				durationFactor*=.03125;	
			else
			if ( inputs[IN_PROG_STEP_EXT_CV].isConnected()) // something is connected to the circle STEP input but we do not know what. Assume it is an 16X BPM frequency
		  		durationFactor *= .01562;  
			
						
			float note_duration=durationFactor*4/(frequency*theModeScaleProgressionsState.theHarmonyParms.note_length_divisor);

			theModeScaleProgressionsState.theHarmonyParms.last_chord_playing=true;

			harmonyGatePulse.reset();  // kill the pulse in case it is active
			harmonyGatePulse.trigger(note_duration);  
		}

	//	outputs[OUT_FBM_HARMONY_OUTPUT].setVoltage((float)clamp((10.f*fBmrand), 0.f, 10.f) ,0); // rescale fBm output to 0-10V so it can be used better for CV.  Output even if harmony disabled

		++circle_step_index;
		if (circle_step_index>=theActiveHarmonyType.num_harmony_steps)
			circle_step_index=0;

		if ((harmony_type==22)&&(step==0)&&(barChordNumber==0))  // random coming home
		{
			float rnd = rack::random::uniform();
			int temp_num_harmony_steps=1 + (int)((rnd*(theHarmonyTypes[22].num_harmony_steps-1)));
			bar_count += (theHarmonyTypes[22].num_harmony_steps-temp_num_harmony_steps);
		}

	}
   
	ModeScaleProgressionsLFOGenerator ModeScaleProgressionsLFOclock;
	
	dsp::SchmittTrigger ST_32ts_trig;  // 32nd note timer tick

	dsp::SchmittTrigger run_button_trig;
	dsp::SchmittTrigger ext_run_trig;
	dsp::SchmittTrigger reset_btn_trig;
	dsp::SchmittTrigger reset_ext_trig;
	dsp::SchmittTrigger rand_btn_trig;
	dsp::SchmittTrigger rand_ext_trig;
	dsp::SchmittTrigger bpm_mode_trig;
	dsp::SchmittTrigger step_button_trig;

	dsp::PulseGenerator resetPulse;
	bool reset_pulse = false;

	dsp::PulseGenerator randPulse;
	bool rand_pulse = false;

	dsp::PulseGenerator runPulse;
	bool run_pulse = false;

	dsp::PulseGenerator stepPulse;
	bool step_pulse = false;

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
	
	float trigger_length = 0.0001f;

	const float lightLambda = 0.075f;
	float resetLight = 0.0f;
	float randLight = 0.0f;
	float stepLight = 0.0f;

	bool running = true;
	double run_start_cpu_time_double= (double)(clock()) / (double)CLOCKS_PER_SEC;
	
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
  		
	float last_melody_note=0;
	float current_melody_note=0;

	int circle_step_index=0;

	dsp::SchmittTrigger HarmonyEnableToggle;
	dsp::SchmittTrigger HarmonyEnableAll7thsToggle;
	dsp::SchmittTrigger HarmonyEnableV7thsToggle;
	dsp::SchmittTrigger HarmonyEnable4VoiceToggle;
	dsp::SchmittTrigger HarmonyEnableStaccatoToggle;
	dsp::SchmittTrigger HarmonyEnableTonicOnCh1Toggle;
	dsp::SchmittTrigger HarmonyEnableBassOnCh1Toggle;
	dsp::SchmittTrigger BassEnableStaccatoToggle;
	dsp::SchmittTrigger RunToggle;
	dsp::SchmittTrigger CircleStepToggles[MAX_STEPS];
	dsp::SchmittTrigger CircleStepSetToggles[MAX_STEPS];

	bool CircleStepStates[MAX_STEPS]={};
	bool CircleStepSetStates[MAX_STEPS]={};

	rack::dsp::PulseGenerator barTriggerPulse; 
	rack::dsp::PulseGenerator harmonyGatePulse; 
	rack::dsp::PulseGenerator melodyGatePulse; 
	rack::dsp::PulseGenerator bassGatePulse; 
	rack::dsp::PulseGenerator barGaterPulse; 

	rack::dsp::PulseGenerator extPolyQuantTriggerPulse[16]; 

	bool time_sig_changed=false;
	bool reset_enqueued=false;

	int override_step=1;

	int lastPlayedCircleDegree=1;
	int lastPlayedCircleOctave=0;
	int lastPlayedCirclePosition=1;

	int lastPlayedScaleDegree=1;
	int lastPlayedScaleOctave=0;
	
	void onRandomize(const RandomizeEvent& e) override {
		for (int i=0; i<NUM_PARAMS; ++i) 
		{
			if (getParamQuantity(i)->randomizeEnabled)
			{
				getParamQuantity(i)->randomize();
			}
		}
		// Call super method if you wish to include default behavior
	    // Module::onRandomize(e);
	}

    // save button states
	json_t *dataToJson() override {
		json_t *rootJ = json_object();

		// running
		json_object_set_new(rootJ, "running", json_boolean(running)); 

		json_object_set_new(rootJ, "theHarmonyParmsenabled", json_boolean(theModeScaleProgressionsState.theHarmonyParms.enabled));
		json_object_set_new(rootJ, "harmony_staccato_enable", json_boolean(theModeScaleProgressionsState.theHarmonyParms.enable_staccato));
		json_object_set_new(rootJ, "theHarmonyParmsenable_all_7ths", json_boolean(theModeScaleProgressionsState.theHarmonyParms.enable_all_7ths));
		json_object_set_new(rootJ, "theHarmonyParmsenable_V_7ths", json_boolean(theModeScaleProgressionsState.theHarmonyParms.enable_V_7ths));
		json_object_set_new(rootJ, "theHarmonyParmsenable_4voice_octaves", json_boolean(theModeScaleProgressionsState.theHarmonyParms.enable_4voice_octaves));
		json_object_set_new(rootJ, "theHarmonyParmsenable_tonic_on_ch1", json_boolean(theModeScaleProgressionsState.theHarmonyParms.send_tonic_on_first_channel));
		json_object_set_new(rootJ, "theHarmonyParmsenable_bass_on_ch1", json_boolean(theModeScaleProgressionsState.theHarmonyParms.send_bass_on_first_channel));
		json_object_set_new(rootJ, "scale_out_mode", json_integer(scale_out_mode));
		json_object_set_new(rootJ, "gate_out_mode", json_integer(gate_out_mode));
		json_object_set_new(rootJ, "paneltheme", json_integer(MSP_panelTheme));
		json_object_set_new(rootJ, "panelcontrast", json_real(MSP_panelContrast));

		// test code for custom progression save
		if (harmony_type==4) // custom
		{
        	json_object_set_new(rootJ, "customPresetStep1", json_integer(theHarmonyTypes[4].harmony_steps[0]));
			json_object_set_new(rootJ, "customPresetStep2", json_integer(theHarmonyTypes[4].harmony_steps[1]));
			json_object_set_new(rootJ, "customPresetStep3", json_integer(theHarmonyTypes[4].harmony_steps[2]));
			json_object_set_new(rootJ, "customPresetStep4", json_integer(theHarmonyTypes[4].harmony_steps[3]));
		
			json_object_set_new(rootJ, "customPresetStep5", json_integer(theHarmonyTypes[4].harmony_steps[4]));
			json_object_set_new(rootJ, "customPresetStep6", json_integer(theHarmonyTypes[4].harmony_steps[5]));
			json_object_set_new(rootJ, "customPresetStep7", json_integer(theHarmonyTypes[4].harmony_steps[6]));
			json_object_set_new(rootJ, "customPresetStep8", json_integer(theHarmonyTypes[4].harmony_steps[7]));

			json_object_set_new(rootJ, "customPresetStep9", json_integer(theHarmonyTypes[4].harmony_steps[8]));
			json_object_set_new(rootJ, "customPresetStep10", json_integer(theHarmonyTypes[4].harmony_steps[9]));
			json_object_set_new(rootJ, "customPresetStep11", json_integer(theHarmonyTypes[4].harmony_steps[10]));
			json_object_set_new(rootJ, "customPresetStep12", json_integer(theHarmonyTypes[4].harmony_steps[11]));

			json_object_set_new(rootJ, "customPresetStep13", json_integer(theHarmonyTypes[4].harmony_steps[12]));
			json_object_set_new(rootJ, "customPresetStep14", json_integer(theHarmonyTypes[4].harmony_steps[13]));
			json_object_set_new(rootJ, "customPresetStep15", json_integer(theHarmonyTypes[4].harmony_steps[14]));
			json_object_set_new(rootJ, "customPresetStep16", json_integer(theHarmonyTypes[4].harmony_steps[15]));
		}
		//
		
		json_object_set_new(rootJ, "harmonic_degree_out_mode", json_integer(harmonic_degree_out_mode));

		// new in 2.0.29
		json_object_set_new(rootJ, "modalmode", json_real(mode));
		json_object_set_new(rootJ, "modaroot", json_real(root_key));
				
		return rootJ;
	}

	void dataFromJson(json_t *rootJ) override {

		// running
		
		json_t *runningJ = json_object_get(rootJ, "running");
		if (runningJ)
			running = json_is_true(runningJ);
				
		json_t *HarmonyParmsenabledJ = json_object_get(rootJ, "theHarmonyParmsenabled");
		if (HarmonyParmsenabledJ)
			theModeScaleProgressionsState.theHarmonyParms.enabled = json_is_true(HarmonyParmsenabledJ);

		json_t *HarmonyParmsstaccato_enableJ = json_object_get(rootJ, "harmony_staccato_enable");
		if (HarmonyParmsstaccato_enableJ)
			theModeScaleProgressionsState.theHarmonyParms.enable_staccato = json_is_true(HarmonyParmsstaccato_enableJ);

		json_t *HarmonyParmsenable_all_7thsJ = json_object_get(rootJ, "theHarmonyParmsenable_all_7ths");
		if (HarmonyParmsenable_all_7thsJ)
			theModeScaleProgressionsState.theHarmonyParms.enable_all_7ths = json_is_true(HarmonyParmsenable_all_7thsJ);
		
		json_t *HarmonyParmsenable_V_7thsJ = json_object_get(rootJ, "theHarmonyParmsenable_V_7ths");
		if (HarmonyParmsenable_V_7thsJ)
			theModeScaleProgressionsState.theHarmonyParms.enable_V_7ths = json_is_true(HarmonyParmsenable_V_7thsJ);  
				
		json_t *HarmonyParmsenable_4voice_octavesJ = json_object_get(rootJ, "theHarmonyParmsenable_4voice_octaves");
		if (HarmonyParmsenable_4voice_octavesJ)
			theModeScaleProgressionsState.theHarmonyParms.enable_4voice_octaves = json_is_true(HarmonyParmsenable_4voice_octavesJ);

		json_t *HarmonyParmsenable_tonic_on_ch1J = json_object_get(rootJ, "theHarmonyParmsenable_tonic_on_ch1");
		if (HarmonyParmsenable_tonic_on_ch1J)
			theModeScaleProgressionsState.theHarmonyParms.send_tonic_on_first_channel = json_is_true(HarmonyParmsenable_tonic_on_ch1J);

		json_t *HarmonyParmsenable_bass_on_ch1J = json_object_get(rootJ, "theHarmonyParmsenable_bass_on_ch1");
		if (HarmonyParmsenable_bass_on_ch1J)
			theModeScaleProgressionsState.theHarmonyParms.send_bass_on_first_channel = json_is_true(HarmonyParmsenable_bass_on_ch1J);
    
		json_t *modeJ = json_object_get(rootJ, "scale_out_mode");
        if(modeJ) scale_out_mode = (ScaleOutMode) json_integer_value(modeJ);
				
		modeJ = json_object_get(rootJ, "gate_out_mode");
        if(modeJ) gate_out_mode = (GateOutMode) json_integer_value(modeJ);
	
		json_t *panelthemeJ = json_object_get(rootJ, "paneltheme");
        if (panelthemeJ)MSP_panelTheme = json_integer_value(panelthemeJ);
	
		json_t *panelcontrastJ = json_object_get(rootJ, "panelcontrast");
        if (panelcontrastJ)MSP_panelContrast = json_real_value(panelcontrastJ);

		
		json_t *harmonic_degree_out_modeJ = json_object_get(rootJ, "harmonic_degree_out_mode");
        if (harmonic_degree_out_modeJ) harmonic_degree_out_mode = (HarmonicDegreeOutCvRangeMode)json_integer_value(harmonic_degree_out_modeJ);

		// older patches will not have either of these set, so set to default if so
		if ((!theModeScaleProgressionsState.theHarmonyParms.send_tonic_on_first_channel)&&(!theModeScaleProgressionsState.theHarmonyParms.send_bass_on_first_channel))
			theModeScaleProgressionsState.theHarmonyParms.send_tonic_on_first_channel = true;
		
		//  code for custom progression load
		//
	
		bool isCustomPreset=false;
		strcpy(theHarmonyTypes[4].harmony_degrees_desc,"");

		int loadedNumSteps=params[CONTROL_HARMONY_STEPS_PARAM].getValue();  // at this point the patch should have been loaded so param can be read
			
		json_t *stepJ=nullptr;
		if ((stepJ = json_object_get(rootJ, "customPresetStep1")))
		{
			isCustomPreset=true;
			int step=json_integer_value(stepJ);
			theHarmonyTypes[4].harmony_steps[0]=step;
			if (loadedNumSteps>0)
			{
				strcat(theHarmonyTypes[4].harmony_degrees_desc,MSP_circle_of_fifths_arabic_degrees[step]);  
				if (0<(loadedNumSteps-1)) 
				strcat(theHarmonyTypes[4].harmony_degrees_desc,"-");
			}
		}

		if ((stepJ = json_object_get(rootJ, "customPresetStep2")))
		{
			isCustomPreset=true;
			int step=json_integer_value(stepJ);
			theHarmonyTypes[4].harmony_steps[1]=step;
			if (loadedNumSteps>1)
			{
				strcat(theHarmonyTypes[4].harmony_degrees_desc,MSP_circle_of_fifths_arabic_degrees[step]);  
				if (1<(loadedNumSteps-1)) 
				strcat(theHarmonyTypes[4].harmony_degrees_desc,"-");
			}
		}

		if ((stepJ = json_object_get(rootJ, "customPresetStep3")))
		{
			isCustomPreset=true;
			int step=json_integer_value(stepJ);
			theHarmonyTypes[4].harmony_steps[2]=step;
			if (loadedNumSteps>2)
			{
				strcat(theHarmonyTypes[4].harmony_degrees_desc,MSP_circle_of_fifths_arabic_degrees[step]);  
				if (2<(loadedNumSteps-1)) 
				strcat(theHarmonyTypes[4].harmony_degrees_desc,"-");
			}
		}

		if ((stepJ = json_object_get(rootJ, "customPresetStep4")))
		{
			isCustomPreset=true;
			int step=json_integer_value(stepJ);
			theHarmonyTypes[4].harmony_steps[3]=step;
			if (loadedNumSteps>3)
			{
				strcat(theHarmonyTypes[4].harmony_degrees_desc,MSP_circle_of_fifths_arabic_degrees[step]);  
				if (3<(loadedNumSteps-1)) 
				strcat(theHarmonyTypes[4].harmony_degrees_desc,"-");
			}
		}

		if ((stepJ = json_object_get(rootJ, "customPresetStep5")))
		{
			isCustomPreset=true;
			int step=json_integer_value(stepJ);
			theHarmonyTypes[4].harmony_steps[4]=step;
			if (loadedNumSteps>4)
			{
				strcat(theHarmonyTypes[4].harmony_degrees_desc,MSP_circle_of_fifths_arabic_degrees[step]);  
				if (4<(loadedNumSteps-1)) 
				strcat(theHarmonyTypes[4].harmony_degrees_desc,"-");
			}
		}

		if ((stepJ = json_object_get(rootJ, "customPresetStep6")))
		{
			isCustomPreset=true;
			int step=json_integer_value(stepJ);
			theHarmonyTypes[4].harmony_steps[5]=step;
			if (loadedNumSteps>5)
			{
				strcat(theHarmonyTypes[4].harmony_degrees_desc,MSP_circle_of_fifths_arabic_degrees[step]);  
				if (5<(loadedNumSteps-1)) 
				strcat(theHarmonyTypes[4].harmony_degrees_desc,"-");
			}
		}

		if ((stepJ = json_object_get(rootJ, "customPresetStep7")))
		{
			isCustomPreset=true;
			int step=json_integer_value(stepJ);
			theHarmonyTypes[4].harmony_steps[6]=step;
			if (loadedNumSteps>6)
			{
				strcat(theHarmonyTypes[4].harmony_degrees_desc,MSP_circle_of_fifths_arabic_degrees[step]);  
				if (6<(loadedNumSteps-1)) 
				strcat(theHarmonyTypes[4].harmony_degrees_desc,"-");
			}
		}

		if ((stepJ = json_object_get(rootJ, "customPresetStep8")))
		{
			isCustomPreset=true;
			int step=json_integer_value(stepJ);
			theHarmonyTypes[4].harmony_steps[7]=step;
			if (loadedNumSteps>7)
			{
				strcat(theHarmonyTypes[4].harmony_degrees_desc,MSP_circle_of_fifths_arabic_degrees[step]);  
				if (7<(loadedNumSteps-1)) 
				strcat(theHarmonyTypes[4].harmony_degrees_desc,"-");
			}
		}

		if ((stepJ = json_object_get(rootJ, "customPresetStep9")))
		{
			isCustomPreset=true;
			int step=json_integer_value(stepJ);
			theHarmonyTypes[4].harmony_steps[8]=step;
			if (loadedNumSteps>8)
			{
				strcat(theHarmonyTypes[4].harmony_degrees_desc,MSP_circle_of_fifths_arabic_degrees[step]);  
				if (8<(loadedNumSteps-1)) 
				strcat(theHarmonyTypes[4].harmony_degrees_desc,"-");
			}
		}

		if ((stepJ = json_object_get(rootJ, "customPresetStep10")))
		{
			isCustomPreset=true;
			int step=json_integer_value(stepJ);
			theHarmonyTypes[4].harmony_steps[9]=step;
			if (loadedNumSteps>9)
			{
				strcat(theHarmonyTypes[4].harmony_degrees_desc,MSP_circle_of_fifths_arabic_degrees[step]);  
				if (9<(loadedNumSteps-1)) 
				strcat(theHarmonyTypes[4].harmony_degrees_desc,"-");
			}
		}

		if ((stepJ = json_object_get(rootJ, "customPresetStep11")))
		{
			isCustomPreset=true;
			int step=json_integer_value(stepJ);
			theHarmonyTypes[4].harmony_steps[10]=step;
			if (loadedNumSteps>10)
			{
				strcat(theHarmonyTypes[4].harmony_degrees_desc,MSP_circle_of_fifths_arabic_degrees[step]);  
				if (10<(loadedNumSteps-1)) 
				strcat(theHarmonyTypes[4].harmony_degrees_desc,"-");
			}
		}

		if ((stepJ = json_object_get(rootJ, "customPresetStep12")))
		{
			isCustomPreset=true;
			int step=json_integer_value(stepJ);
			theHarmonyTypes[4].harmony_steps[11]=step;
			if (loadedNumSteps>11)
			{
				strcat(theHarmonyTypes[4].harmony_degrees_desc,MSP_circle_of_fifths_arabic_degrees[step]);  
				if (11<(loadedNumSteps-1)) 
				strcat(theHarmonyTypes[4].harmony_degrees_desc,"-");
			}
		}

		if ((stepJ = json_object_get(rootJ, "customPresetStep13")))
		{
			isCustomPreset=true;
			int step=json_integer_value(stepJ);
			theHarmonyTypes[4].harmony_steps[12]=step;
			if (loadedNumSteps>12)
			{
				strcat(theHarmonyTypes[4].harmony_degrees_desc,MSP_circle_of_fifths_arabic_degrees[step]);  
				if (12<(loadedNumSteps-1)) 
				strcat(theHarmonyTypes[4].harmony_degrees_desc,"-");
			}
		}

		if ((stepJ = json_object_get(rootJ, "customPresetStep14")))
		{
			isCustomPreset=true;
			int step=json_integer_value(stepJ);
			theHarmonyTypes[4].harmony_steps[13]=step;
			if (loadedNumSteps>13)
			{
				strcat(theHarmonyTypes[4].harmony_degrees_desc,MSP_circle_of_fifths_arabic_degrees[step]);  
				if (13<(loadedNumSteps-1)) 
				strcat(theHarmonyTypes[4].harmony_degrees_desc,"-");
			}
		}

		if ((stepJ = json_object_get(rootJ, "customPresetStep15")))
		{
			isCustomPreset=true;
			int step=json_integer_value(stepJ);
			if (loadedNumSteps>14)
			{
				strcat(theHarmonyTypes[4].harmony_degrees_desc,MSP_circle_of_fifths_arabic_degrees[step]);  
				if (14<(loadedNumSteps-1)) 
				strcat(theHarmonyTypes[4].harmony_degrees_desc,"-");
			}
		}
	
		if ((stepJ = json_object_get(rootJ, "customPresetStep16")))  
		{
			isCustomPreset=true;
			int step=json_integer_value(stepJ);
			theHarmonyTypes[4].harmony_steps[15]=step;
			if (loadedNumSteps>15)
			{
				strcat(theHarmonyTypes[4].harmony_degrees_desc,MSP_circle_of_fifths_arabic_degrees[step]);  
				if (15<(loadedNumSteps-1)) 
				strcat(theHarmonyTypes[4].harmony_degrees_desc," ");
			}
		} 

		// new in 2.0.29
		json_t *modalmodeJ = json_object_get(rootJ, "modalmode");
        if (modalmodeJ) mode = json_real_value(modalmodeJ);

		json_t *modalrootJ = json_object_get(rootJ, "modalroot");
        if (modalrootJ) root_key = json_real_value(modalrootJ);

	
		// Make sure savedHarmonySteps is reset by always applying it after load 
		savedHarmonySteps=(int)(std::round(params[CONTROL_HARMONY_STEPS_PARAM].getValue()));
		if ((savedHarmonySteps<theHarmonyTypes[harmony_type].min_steps)||(savedHarmonySteps>theHarmonyTypes[harmony_type].max_steps))
		{
			savedHarmonySteps=theHarmonyTypes[harmony_type].max_steps;
			params[CONTROL_HARMONY_STEPS_PARAM].setValue(savedHarmonySteps);
		}
			
		if (isCustomPreset)
		{
			harmony_type=4;
			params[CONTROL_HARMONYPRESETS_PARAM].setValue(4); // added for testing custom 
			params[CONTROL_HARMONY_STEPS_PARAM].setValue(savedHarmonySteps);  // redundant
			copyHarmonyTypeToActiveHarmonyType(harmony_type);
			theActiveHarmonyType.num_harmony_steps=savedHarmonySteps;  
			theHarmonyTypes[harmony_type].num_harmony_steps=savedHarmonySteps;  // add this to fix step edit problem
			theActiveHarmonyType.min_steps=1;
			theActiveHarmonyType.max_steps=16;
			strcpy(theActiveHarmonyType.harmony_type_desc, "custom" );
			setup_harmony();
		}
		else
		{
			circleChanged=true;  // in most cases, trigger off a circle build after load unless custom preset
		}
		
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
	
		if (RunToggle.process(params[BUTTON_RUN_PARAM].getValue() || inputs[IN_RUN_EXT_CV].getVoltage()))  
		{ 
			running=!running;

			if(!running)
			{
				run_start_cpu_time_double= (double)(clock()) / (double)CLOCKS_PER_SEC; 
				i16ts_count = 0;  
				i8ts_count = 0;  
				i4ts_count = 0; 
				i2ts_count = 0; 
				barts_count = 0;    
				
				theModeScaleProgressionsState.theMelodyParms.bar_melody_counted_note=0;
				theModeScaleProgressionsState.theArpParms.note_count=0;
				theModeScaleProgressionsState.theBassParms.bar_bass_counted_note=0;
				outputs[OUT_CLOCK_BAR_OUTPUT].setVoltage(0.0f);	   // bars 	
				outputs[OUT_CLOCK_BEAT_OUTPUT].setVoltage(0.0f);   // 4ts 
				outputs[OUT_CLOCK_BEATX2_OUTPUT].setVoltage(0.0f); // 8ts
				outputs[OUT_CLOCK_BEATX4_OUTPUT].setVoltage(0.0f); // 16ts
				outputs[OUT_CLOCK_BEATX8_OUTPUT].setVoltage(0.0f); // 32ts
			}
			else
			{
				ModeScaleProgressionsLFOclock.setFreq(frequency*(32/time_sig_bottom));	  // for 32ts	
				barts_count_limit = (32*time_sig_top/time_sig_bottom);
			}
			theModeScaleProgressionsState.theHarmonyParms.pending_step_edit=0;
			runPulse.trigger(0.01f); // delay 10ms
		}
		lights[LIGHT_LEDBUTTON_RUN].setBrightness(running ? 1.0f : 0.0f); 
		run_pulse = runPulse.process(1.0 / args.sampleRate);  
		outputs[OUT_RUN_OUT].setVoltage((run_pulse ? 10.0f : 0.0f));

		if (inputs[IN_TEMPO_EXT_CV].isConnected())
		{
			float fvalue=inputs[IN_TEMPO_EXT_CV].getVoltage();
			tempo=std::round(std::pow(2.0, fvalue)*120);
		
			if (tempo<min_bpm)
				tempo=min_bpm;
	
			if (tempo>max_bpm)
				tempo=max_bpm;

			if (true)  // adjust the tempo knob and param
			{
				params[CONTROL_TEMPOBPM_PARAM].setValue(tempo);
			}
		}
		else
		{
			float fvalue = std::round(params[CONTROL_TEMPOBPM_PARAM].getValue());
			if (fvalue!=tempo)
			tempo=fvalue;
		}

		frequency = tempo/60.0f;  // drives 1 tick per 32nd note
						
		// Reset

			
		if (reset_btn_trig.process(params[BUTTON_RESET_PARAM].getValue() || inputs[IN_RESET_EXT_CV].getVoltage() || time_sig_changed || reset_enqueued)) 
		{
			run_start_cpu_time_double= (double)(clock()) / (double)CLOCKS_PER_SEC;
			time_sig_changed=false;
			reset_enqueued=false;
	    	ModeScaleProgressionsLFOclock.setReset(1.0f);
			bar_count = 0;
			bar_note_count=0;
			i16ts_count = 0;  
			i8ts_count = 0;  
			i4ts_count = 0; 
			i2ts_count = 0; 
			barts_count = 0;    
			 

			theModeScaleProgressionsState.theMelodyParms.bar_melody_counted_note=0;
			theModeScaleProgressionsState.theArpParms.note_count=0;
			theModeScaleProgressionsState.theBassParms.bar_bass_counted_note=0;

			theModeScaleProgressionsState.theHarmonyParms.last_circle_step=-1; // for Markov chain
			inportStates[IN_HARMONY_CIRCLE_DEGREE_EXT_CV].lastValue=-999;
			harmonyGatePulse.reset();  // kill the pulse in case it is active
			outputs[OUT_HARMONY_GATE_OUTPUT].setVoltage(0);
				
			resetLight = 1.0;
			resetPulse.trigger(0.01f);  // necessary to pass on reset below vis resetPuls.process()

				
			if (!running)
			{
				harmonyGatePulse.reset();  // kill the pulse in case it is active
				melodyGatePulse.reset();  // kill the pulse in case it is active
				bassGatePulse.reset();  // kill the pulse in case it is active
				outputs[OUT_HARMONY_GATE_OUTPUT].setVoltage(0);
				outputs[OUT_HARMONY_VOLUME_OUTPUT].setVoltage(0);
			}
			
		}

		resetLight -= resetLight / lightLambda / args.sampleRate;
		lights[LIGHT_LEDBUTTON_RESET].setBrightness(resetLight); 
		reset_pulse = resetPulse.process(1.0 / args.sampleRate);
  		outputs[OUT_RESET_OUT].setVoltage((reset_pulse ? 10.0f : 0.0f));
        
	    if (rand_btn_trig.process(params[BUTTON_RAND_PARAM].getValue() || inputs[IN_RAND_EXT_CV].getVoltage() )) 
		{
			randEnqueued=true;
			inportStates[IN_RAND_EXT_CV].lastValue=-999;
			randLight = 1.0;
			randPulse.trigger(0.01f);  
		}

		randLight -= randLight / lightLambda / args.sampleRate;
		lights[LIGHT_LEDBUTTON_RAND].setBrightness(randLight); 
		rand_pulse = randPulse.process(1.0 / args.sampleRate);
	
		if ((step_button_trig.process(params[BUTTON_PROG_STEP_PARAM].getValue() || (  inputs[IN_PROG_STEP_EXT_CV].isConnected()  &&  (inputs[IN_PROG_STEP_EXT_CV].getVoltage() > 0.))))) 
		{
			++bar_count;

			if (theModeScaleProgressionsState.theHarmonyParms.enabled)
			{
				theModeScaleProgressionsState.theHarmonyParms.enabled = false;
				override_step=0;
			}
			else
			{
				++override_step;
				if (override_step>=theActiveHarmonyType.num_harmony_steps)
				  override_step=0;
			    
			}
			theModeScaleProgressionsState.userControllingHarmonyFromCircle=true;
			theModeScaleProgressionsState.last_harmony_step=override_step;
				
			int degreeStep=(theActiveHarmonyType.harmony_steps[override_step])%8;  
			if ((degreeStep<1)||(degreeStep>7))
				  degreeStep=1;  // force to a valid degree to avoid a crash
			current_circle_degree = degreeStep;
			
			//find this in semicircle
			for (int j=0; j<7; ++j)
			{
				if  (theCircleOf5ths.theDegreeSemiCircle.degreeElements[j].Degree==degreeStep)
				{
					current_circle_position = theCircleOf5ths.theDegreeSemiCircle.degreeElements[j].CircleIndex; 
					break;
				}
			}
		
			stepLight = 1.0;
			stepPulse.trigger(0.01f);  // necessary to pass on reset below vis resetPuls.process()
           			
			if (running)
			{
				doHarmony(0, true);
			}
		
		}

		stepLight -= stepLight / lightLambda / args.sampleRate;
		lights[LIGHT_LEDBUTTON_PROG_STEP_PARAM].setBrightness(stepLight);
		step_pulse = stepPulse.process(1.0 / args.sampleRate);

		if(running)  
		{
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
			ModeScaleProgressionsLFOclock.setFreq(frequency*(32/time_sig_bottom));	  // for 32ts	should not hurt top call this each sample
			barts_count_limit = (32*time_sig_top/time_sig_bottom);
			//************************************************************************
						 
			ModeScaleProgressionsLFOclock.step(1.0 / args.sampleRate);

			bool clockTick=false;
			if ( inputs[IN_CLOCK_EXT_CV].isConnected())  // external clock connected to Clock input
			{
				if (!inportStates[IN_CLOCK_EXT_CV].inTransition)
				{
					if (ST_32ts_trig.process(inputs[IN_CLOCK_EXT_CV].getVoltage()))  // triggers from each external clock tick ONLY once when input reaches 1.0V
					{
						clockTick=true;
						outputs[OUT_CLOCK_OUT].setChannels(1);  // set polyphony  
						outputs[OUT_CLOCK_OUT].setVoltage(10.0f);
						inportStates[IN_CLOCK_EXT_CV].inTransition=true;
					}
				}

				if (inportStates[IN_CLOCK_EXT_CV].inTransition)
				{
					if (ST_32ts_trig.process(math::rescale(inputs[IN_CLOCK_EXT_CV].getVoltage(),10.f,0.f,0.f,10.f)))  // triggers from each external clock tick ONLY once when inverted input reaches 0.0V
					{
						outputs[OUT_CLOCK_OUT].setChannels(1);  // set polyphony  
						outputs[OUT_CLOCK_OUT].setVoltage(0.0f);  
						inportStates[IN_CLOCK_EXT_CV].inTransition=false;
					}
				}
			}
			else // no external clock connected to Clock input, use internal clock
			{
				float IntClockLevel=5.0f*(ModeScaleProgressionsLFOclock.sqr()+1.0f);
				if (ST_32ts_trig.process(ModeScaleProgressionsLFOclock.sqr()))                         // triggers from each external clock tick ONLY once when .sqr() reaches 1.0V
				{
					 clockTick=true;
				}
			
				 outputs[OUT_CLOCK_OUT].setChannels(1);  // set polyphony  
				 outputs[OUT_CLOCK_OUT].setVoltage(IntClockLevel);  
			}
				
		    if (clockTick)
			{
			//	bool melodyPlayed=false;   // set to prevent arp note being played on the melody beat
				int barChordNumber=(int)((int)(barts_count*theModeScaleProgressionsState.theHarmonyParms.note_length_divisor)/(int)32);
			
				// bar
				if (barts_count == 0) 
				{
					theModeScaleProgressionsState.theMelodyParms.bar_melody_counted_note=0;
					theModeScaleProgressionsState.theBassParms.bar_bass_counted_note=0;
					bar_note_count=0;
					if ((theModeScaleProgressionsState.theHarmonyParms.note_length_divisor==1)&&(!theModeScaleProgressionsState.userControllingHarmonyFromCircle))
						doHarmony(barChordNumber, theModeScaleProgressionsState.theHarmonyParms.enabled);
				
					clockPulse1ts.trigger(trigger_length);
					// Pulse the output gate 
					barTriggerPulse.trigger(1e-3f);  // 1ms duration  need to use .process to detect this and then send it to output
				}
						
		        // i2ts
				if (i2ts_count == 0)
				{
					if ((theModeScaleProgressionsState.theHarmonyParms.note_length_divisor==2)&&(!theModeScaleProgressionsState.userControllingHarmonyFromCircle))
						doHarmony(barChordNumber, theModeScaleProgressionsState.theHarmonyParms.enabled);
			
					i2ts_count++;

					if (!theModeScaleProgressionsState.theHarmonyParms.STEP_inport_connected_to_ModeScaleProgressions_trigger_port)  // there is no connection to the STEP inport
						clockPulse2ts.trigger(trigger_length);
				}
				else
				if (i2ts_count == (i2ts_count_limit-1))
				{
					i2ts_count = 0;    
					if (theModeScaleProgressionsState.theHarmonyParms.STEP_inport_connected_to_ModeScaleProgressions_trigger_port) // there is a connection to the STEP inport
						clockPulse2ts.trigger(trigger_length);
				}
				else
				{
					i2ts_count++;
				}
								
		
				// i4ts
				if (i4ts_count == 0)
				{
					if ((theModeScaleProgressionsState.theHarmonyParms.note_length_divisor==4)&&(!theModeScaleProgressionsState.userControllingHarmonyFromCircle))
						doHarmony(barChordNumber, theModeScaleProgressionsState.theHarmonyParms.enabled);
				
					i4ts_count++;

					if (!theModeScaleProgressionsState.theHarmonyParms.STEP_inport_connected_to_ModeScaleProgressions_trigger_port) // there is no connection to the STEP inport
					 	clockPulse4ts.trigger(trigger_length);
				}
				else
				if (i4ts_count == (i4ts_count_limit-1))
				{
					i4ts_count = 0;  
					if (theModeScaleProgressionsState.theHarmonyParms.STEP_inport_connected_to_ModeScaleProgressions_trigger_port)  // there is a connection to the STEP inport
					 	clockPulse4ts.trigger(trigger_length);
				}
				else
				{
					i4ts_count++;
				}
				
					  
		 		// i8ts
				if (i8ts_count == 0)
				{
					if ((theModeScaleProgressionsState.theHarmonyParms.note_length_divisor==8)&&(!theModeScaleProgressionsState.userControllingHarmonyFromCircle))
						doHarmony(barChordNumber, theModeScaleProgressionsState.theHarmonyParms.enabled);
				
					i8ts_count++;

					if (!theModeScaleProgressionsState.theHarmonyParms.STEP_inport_connected_to_ModeScaleProgressions_trigger_port)  // there is no connection to the STEP inport
						clockPulse8ts.trigger(trigger_length);
				}
				else
				if (i8ts_count == (i8ts_count_limit-1))
				{
					i8ts_count = 0; 
					if (theModeScaleProgressionsState.theHarmonyParms.STEP_inport_connected_to_ModeScaleProgressions_trigger_port)  // there is a connection to the STEP inport
					  clockPulse8ts.trigger(trigger_length);
				}
				else
				{
					i8ts_count++; 
				} 
				
				
				// i16ts
				if (i16ts_count == 0)
				{
				
					i16ts_count++;

					if (!theModeScaleProgressionsState.theHarmonyParms.STEP_inport_connected_to_ModeScaleProgressions_trigger_port)  // there is no connection to the STEP inport
						clockPulse16ts.trigger(trigger_length);
				}
				else
				if (i16ts_count == (i16ts_count_limit-1))
				{
					i16ts_count = 0;
					if (theModeScaleProgressionsState.theHarmonyParms.STEP_inport_connected_to_ModeScaleProgressions_trigger_port)  // there is a connection to the STEP inport
						clockPulse16ts.trigger(trigger_length);
				}
				else
				{
					i16ts_count++;
				}
				

				//32nds  ***********************************
				
				if (barts_count == (barts_count_limit-1))  // do this after all processing so bar_count does not get incremented too early
				{
					barts_count = 0;  
					bar_note_count=0;
					if (!theModeScaleProgressionsState.userControllingHarmonyFromCircle)  // don't mess up bar count
					++bar_count; 
				}
				else
				{
					barts_count++;  
				}
				
				clockPulse32ts.trigger(trigger_length);  // retrigger the pulse after all done in this loop
			}
			else  // !clockTick
			{
			
			}
			
		}

		pulse1ts = clockPulse1ts.process(1.0 / args.sampleRate);
		pulse2ts = clockPulse2ts.process(1.0 / args.sampleRate);
		pulse4ts = clockPulse4ts.process(1.0 / args.sampleRate);
		pulse8ts = clockPulse8ts.process(1.0 / args.sampleRate);
		pulse16ts = clockPulse16ts.process(1.0 / args.sampleRate);
		pulse32ts = clockPulse32ts.process(1.0 / args.sampleRate);

		// end the gate if pulse timer has expired 

		if (gate_out_mode == STANDARD_GATE) // standard gate voltages 
		{
			if (harmonyGatePulse.process( 1.0 / APP->engine->getSampleRate()))
			{
				outputs[OUT_HARMONY_GATE_OUTPUT].setVoltage(CV_MAX10);
			}
			else
			{
				outputs[OUT_HARMONY_GATE_OUTPUT].setVoltage(0);
				theModeScaleProgressionsState.theHarmonyParms.last_chord_playing=false;
			}
		}

		
		if (gate_out_mode == VOLUME_OVER_GATE) // non-standard volume over gate voltages
		{
			float harmonyGateLevel=theModeScaleProgressionsState.theHarmonyParms.volume; 
			harmonyGateLevel=clamp(harmonyGateLevel, 2.1f, 10.f);  // don't let gate on level drop below 2.0v so it will trigger ADSR etc.
			if (harmonyGatePulse.process( 1.0 / APP->engine->getSampleRate()))
			{
				outputs[OUT_HARMONY_GATE_OUTPUT].setVoltage(harmonyGateLevel);
			}
			else
			{
				outputs[OUT_HARMONY_GATE_OUTPUT].setVoltage(0);
				theModeScaleProgressionsState.theHarmonyParms.last[0].note=0;  // set as invalid since key should no longer be drawn as pressed
				theModeScaleProgressionsState.theHarmonyParms.last[1].note=0;
				theModeScaleProgressionsState.theHarmonyParms.last[2].note=0;
				theModeScaleProgressionsState.theHarmonyParms.last[3].note=0;
			}
        }
				
				
		outputs[OUT_CLOCK_BAR_OUTPUT].setVoltage((pulse1ts ? 10.0f : 0.0f));     // barts  
		outputs[OUT_CLOCK_BEAT_OUTPUT].setVoltage((pulse4ts ? 10.0f : 0.0f));    // 4ts
		outputs[OUT_CLOCK_BEATX2_OUTPUT].setVoltage((pulse8ts ? 10.0f : 0.0f));  // 8ts
		outputs[OUT_CLOCK_BEATX4_OUTPUT].setVoltage((pulse16ts ? 10.0f : 0.0f)); // 16ts
		outputs[OUT_CLOCK_BEATX8_OUTPUT].setVoltage((pulse32ts ? 10.0f : 0.0f)); // 32ts

	        
		if (HarmonyEnableToggle.process(params[BUTTON_ENABLE_HARMONY_PARAM].getValue())) 
		{
			theModeScaleProgressionsState.theHarmonyParms.enabled = !theModeScaleProgressionsState.theHarmonyParms.enabled;
			theModeScaleProgressionsState.userControllingHarmonyFromCircle=false;
		}
		lights[LIGHT_LEDBUTTON_HARMONY_ENABLE].setBrightness(theModeScaleProgressionsState.theHarmonyParms.enabled ? 1.0f : 0.0f); 

		if (HarmonyEnableAll7thsToggle.process(params[BUTTON_ENABLE_HARMONY_ALL7THS_PARAM].getValue())) 
		{
			theModeScaleProgressionsState.theHarmonyParms.enable_all_7ths = !theModeScaleProgressionsState.theHarmonyParms.enable_all_7ths;
			if (theModeScaleProgressionsState.theHarmonyParms.enable_all_7ths)
			{
				theModeScaleProgressionsState.theHarmonyParms.enable_V_7ths=false;
				theModeScaleProgressionsState.theHarmonyParms.enable_4voice_octaves=false;
			}
			setup_harmony();  // calculate harmony notes
			circleChanged=true;
		}
		lights[LIGHT_LEDBUTTON_ENABLE_HARMONY_ALL7THS_PARAM].setBrightness(theModeScaleProgressionsState.theHarmonyParms.enable_all_7ths ? 1.0f : 0.0f); 
		

		if (HarmonyEnableV7thsToggle.process(params[BUTTON_ENABLE_HARMONY_V7THS_PARAM].getValue())) 
		{
			theModeScaleProgressionsState.theHarmonyParms.enable_V_7ths = !theModeScaleProgressionsState.theHarmonyParms.enable_V_7ths;
			if (theModeScaleProgressionsState.theHarmonyParms.enable_V_7ths)
			{
				theModeScaleProgressionsState.theHarmonyParms.enable_all_7ths=false;
				theModeScaleProgressionsState.theHarmonyParms.enable_4voice_octaves=false;
		    }
			setup_harmony();
			circleChanged=true;
		}
		lights[LIGHT_LEDBUTTON_ENABLE_HARMONY_V7THS_PARAM].setBrightness(theModeScaleProgressionsState.theHarmonyParms.enable_V_7ths ? 1.0f : 0.0f);

		if (HarmonyEnable4VoiceToggle.process(params[BUTTON_ENABLE_HARMONY_4VOICE_OCTAVES_PARAM].getValue())) 
		{  
			theModeScaleProgressionsState.theHarmonyParms.enable_4voice_octaves = !theModeScaleProgressionsState.theHarmonyParms.enable_4voice_octaves;
			if (theModeScaleProgressionsState.theHarmonyParms.enable_4voice_octaves)
			{
				theModeScaleProgressionsState.theHarmonyParms.enable_all_7ths=false;
				theModeScaleProgressionsState.theHarmonyParms.enable_V_7ths=false;
			}
			setup_harmony();
			circleChanged=true;
		}
		lights[LIGHT_LEDBUTTON_ENABLE_HARMONY_4VOICE_OCTAVES_PARAM].setBrightness(theModeScaleProgressionsState.theHarmonyParms.enable_4voice_octaves ? 1.0f : 0.0f); 

		if (HarmonyEnableTonicOnCh1Toggle.process(params[BUTTON_ENABLE_HARMONY_TONIC_ON_CH1_PARAM ].getValue())) 
		{
			if (theModeScaleProgressionsState.theHarmonyParms.send_tonic_on_first_channel)
			{
				theModeScaleProgressionsState.theHarmonyParms.send_tonic_on_first_channel=false;
				theModeScaleProgressionsState.theHarmonyParms.send_bass_on_first_channel=true;
			}
			else
			{
				theModeScaleProgressionsState.theHarmonyParms.send_tonic_on_first_channel=true;
				theModeScaleProgressionsState.theHarmonyParms.send_bass_on_first_channel=false;
			}

		}
		lights[LIGHT_LEDBUTTON_ENABLE_HARMONY_TONIC_ON_CH1_PARAM].setBrightness(theModeScaleProgressionsState.theHarmonyParms.send_tonic_on_first_channel ? 1.0f : 0.0f); 

		if (HarmonyEnableBassOnCh1Toggle.process(params[BUTTON_ENABLE_HARMONY_BASS_ON_CH1_PARAM ].getValue())) 
		{
			if (theModeScaleProgressionsState.theHarmonyParms.send_bass_on_first_channel)
			{
				theModeScaleProgressionsState.theHarmonyParms.send_bass_on_first_channel=false;
				theModeScaleProgressionsState.theHarmonyParms.send_tonic_on_first_channel=true;
			}
			else
			{
				theModeScaleProgressionsState.theHarmonyParms.send_bass_on_first_channel=true;
				theModeScaleProgressionsState.theHarmonyParms.send_tonic_on_first_channel=false;
			}
		}
		lights[LIGHT_LEDBUTTON_ENABLE_HARMONY_BASS_ON_CH1_PARAM].setBrightness(theModeScaleProgressionsState.theHarmonyParms.send_bass_on_first_channel ? 1.0f : 0.0f); 


		if (HarmonyEnableStaccatoToggle.process(params[BUTTON_ENABLE_HARMONY_STACCATO_PARAM].getValue())) 
		{		
			theModeScaleProgressionsState.theHarmonyParms.enable_staccato = !theModeScaleProgressionsState.theHarmonyParms.enable_staccato;
			
		}
		lights[LIGHT_LEDBUTTON_ENABLE_HARMONY_STACCATO_PARAM].setBrightness(theModeScaleProgressionsState.theHarmonyParms.enable_staccato); 
	
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
			
				userPlaysCirclePositionHarmony(current_circle_position, theModeScaleProgressionsState.theHarmonyParms.target_octave); 
										
				theModeScaleProgressionsState.userControllingHarmonyFromCircle=true;
				theModeScaleProgressionsState.theHarmonyParms.enabled=false;
				lights[LIGHT_LEDBUTTON_HARMONY_ENABLE].setBrightness(theModeScaleProgressionsState.theHarmonyParms.enabled ? 1.0f : 0.0f); 
			
			
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
							theModeScaleProgressionsState.circleDegree=theDegree; 
							if (harmonic_degree_out_mode == RANGE_1to7)
								outputs[OUT_EXT_HARMONIC_DEGREE_OUTPUT].setVoltage(theModeScaleProgressionsState.circleDegree);  // for degree= 1-7
							else
								outputs[OUT_EXT_HARMONIC_DEGREE_OUTPUT].setVoltage(theModeScaleProgressionsState.circleDegree-1.0);  // for degree= 0-6

							int circle_chord_type= theCircleOf5ths.Circle5ths[current_circle_position].chordType;

							outputs[OUT_EXT_HARMONIC_DEGREE_CHORD_TYPE_OUTPUT].setVoltage(circle_chord_type);  // chord type for circle scale degree

							if (theModeScaleProgressionsState.theHarmonyParms.pending_step_edit)
							{
								theHarmonyTypes[harmony_type].harmony_steps[theModeScaleProgressionsState.theHarmonyParms.pending_step_edit-BUTTON_HARMONY_SETSTEP_1_PARAM]=theDegree;
								
								strcpy(theHarmonyTypes[harmony_type].harmony_degrees_desc,"");
								for (int k=0;k<theHarmonyTypes[harmony_type].num_harmony_steps;++k)
								{
									strcat(theHarmonyTypes[harmony_type].harmony_degrees_desc,MSP_circle_of_fifths_arabic_degrees[theHarmonyTypes[harmony_type].harmony_steps[k]]);  
									if (k<(theHarmonyTypes[harmony_type].num_harmony_steps-1)) 
										strcat(theHarmonyTypes[harmony_type].harmony_degrees_desc,"-");
								}
								
								copyHarmonyTypeToActiveHarmonyType(harmony_type);  
							}
						}
						break;
					}
				} 
			}
		}
			
		if (!running)
		{
			for (int i=0; i<theActiveHarmonyType.num_harmony_steps; ++i) 
			{
				if (CircleStepSetToggles[i].process(params[BUTTON_HARMONY_SETSTEP_1_PARAM+i].getValue())) 
				{
					int selectedStep=i;
					theModeScaleProgressionsState.theHarmonyParms.pending_step_edit=BUTTON_HARMONY_SETSTEP_1_PARAM+selectedStep;

					int current_circle_position=0;
					if (true)
					{
						int degreeStep=(theActiveHarmonyType.harmony_steps[selectedStep])%8;  
						if ((degreeStep<1)||(degreeStep>7))
					    	degreeStep=1;  // force to a valid degree to avoid a crash
						current_circle_degree = degreeStep;
						theModeScaleProgressionsState.circleDegree=degreeStep;
						
						//find this in semicircle
						for (int j=0; j<7; ++j)
						{
							if  (theCircleOf5ths.theDegreeSemiCircle.degreeElements[j].Degree==degreeStep)
							{
								current_circle_position = theCircleOf5ths.theDegreeSemiCircle.degreeElements[j].CircleIndex; 
								break;
							}
						}
					}

					
					for (int i=0; i<12; ++i)  
					{
						lights[LIGHT_LEDBUTTON_CIRCLESTEP_1+i].setBrightness(0.0f);	
					}
					lights[LIGHT_LEDBUTTON_CIRCLESTEP_1+current_circle_position].setBrightness(1.0f);
										
					userPlaysCirclePositionHarmony(current_circle_position, theModeScaleProgressionsState.theHarmonyParms.target_octave);  
				
					if (!CircleStepSetStates[i])
						CircleStepSetStates[i] = true;
					lights[LIGHT_LEDBUTTON_CIRCLESETSTEP_1+i].setBrightness(CircleStepSetStates[i] ? 1.0f : 0.25f);
					
					for (int j=0; j<theActiveHarmonyType.num_harmony_steps; ++j) {
						if (j!=i) {
							CircleStepSetStates[j] = false;
							lights[LIGHT_LEDBUTTON_CIRCLESETSTEP_1+j].setBrightness(0.25f);
						}
					}

					
				}
			} 
		}

		float fvalue=0;
        float circleDegree=0;  // for harmony
		float circleDegreeValue=0;
		float gateValue=0;

	
		if (  (inputs[IN_HARMONY_CIRCLE_DEGREE_GATE_EXT_CV].isConnected()) && (inputs[IN_HARMONY_CIRCLE_DEGREE_EXT_CV].isConnected()) )
		{
			circleDegreeValue=inputs[IN_HARMONY_CIRCLE_DEGREE_EXT_CV].getVoltage();
			circleDegree=circleDegreeValue;
			gateValue=inputs[IN_HARMONY_CIRCLE_DEGREE_GATE_EXT_CV].getVoltage(); 

			theModeScaleProgressionsState.theHarmonyParms.lastCircleDegreeIn=circleDegree;
			extHarmonyIn=circleDegree;
		
			float octave=(float)((int)(circleDegree));  // from the keyboard
			if (octave>3)
				octave=3;
			if (octave<-3)
				octave=-3;

			bool degreeChanged=false; // assume false unless determined true below
			bool skipStep=false;
			bool repeatStep=false;

            if (gateValue==circleDegree)
			{ 
				if ((circleDegreeValue>0)&&(circleDegreeValue<=7.7))  // MarkovSeq or other 1-7V degree  degree.octave 0.0-7.7V
				{   
					if (inportStates[IN_HARMONY_CIRCLE_DEGREE_EXT_CV].inTransition)
					{
						if (circleDegree==inportStates[IN_HARMONY_CIRCLE_DEGREE_EXT_CV].lastValue)
						{
							// was in transition but now is not
							inportStates[IN_HARMONY_CIRCLE_DEGREE_EXT_CV].inTransition=false;
							inportStates[IN_HARMONY_CIRCLE_DEGREE_EXT_CV].lastValue=circleDegreeValue;
							octave = (int)(10.0*std::fmod(circleDegree, 1.0f));
							if (octave>7)
								octave=7;
							if (circleDegree>=1.0)
								circleDegree=(float)((int)circleDegree);
						   	theModeScaleProgressionsState.circleDegree=(int)circleDegree;
						    degreeChanged=true;
							harmonyGatePulse.reset();  // kill the pulse in case it is active
							outputs[OUT_HARMONY_GATE_OUTPUT].setVoltage(0);
							
						}
						else
						if (circleDegree!=inportStates[IN_HARMONY_CIRCLE_DEGREE_EXT_CV].lastValue)
						{
							harmonyGatePulse.reset();  // kill the pulse in case it is active
							outputs[OUT_HARMONY_GATE_OUTPUT].setVoltage(0);
							inportStates[IN_HARMONY_CIRCLE_DEGREE_EXT_CV].lastValue=circleDegreeValue;
						}
					}
					else
					{
						if (circleDegree!=inportStates[IN_HARMONY_CIRCLE_DEGREE_EXT_CV].lastValue)
						{   
							harmonyGatePulse.reset();  // kill the pulse in case it is active
							outputs[OUT_HARMONY_GATE_OUTPUT].setVoltage(0);
							inportStates[IN_HARMONY_CIRCLE_DEGREE_EXT_CV].inTransition=true;
							inportStates[IN_HARMONY_CIRCLE_DEGREE_EXT_CV].lastValue=circleDegreeValue;
						}
					}

					if (circleDegree==0)
					{
						degreeChanged=false;
					}
				
					if ((circleDegreeValue>0)&&(circleDegreeValue<1.0))
					{
					//	degreeChanged=false;
					}
				}

				if ((degreeChanged) && (!inportStates[IN_HARMONY_CIRCLE_DEGREE_EXT_CV].inTransition))
				{
					if ((circleDegreeValue>0)&&(circleDegreeValue<1.0))  // MarkovSeq or other 1-7V degree  degree.octave 0.0-7.7V   0.5 means repeat last note played
					{
						repeatStep=true;
						inportStates[IN_HARMONY_CIRCLE_DEGREE_EXT_CV].lastValue=circleDegreeValue;
						circleDegree=lastPlayedCircleDegree ;
						octave=lastPlayedCircleOctave;
						harmonyGatePulse.reset();  // kill the pulse in case it is active
						outputs[OUT_HARMONY_GATE_OUTPUT].setVoltage(0);
					}
					else
					if ((circleDegree==0)||(circleDegree>=8.0))  // MarkovSeq or other 1-7V degree  degree.octave 1.0-7.7V  <1 or >=8V means skip step  
					{
						inportStates[IN_HARMONY_CIRCLE_DEGREE_EXT_CV].lastValue=circleDegreeValue;
						skipStep=true;
					}
				}
			}
			else  // keyboard  C-B
			{
					float fgvalue=inputs[IN_HARMONY_CIRCLE_DEGREE_GATE_EXT_CV].getVoltage();
					if (inportStates[IN_HARMONY_CIRCLE_DEGREE_GATE_EXT_CV].inTransition)
					{
						if (fgvalue==inportStates[IN_HARMONY_CIRCLE_DEGREE_GATE_EXT_CV].lastValue)
						{
							harmonyGatePulse.reset();  // kill the pulse in case it is active
		                	outputs[OUT_HARMONY_GATE_OUTPUT].setVoltage(0);
							// was in transition but now is not
							inportStates[IN_HARMONY_CIRCLE_DEGREE_GATE_EXT_CV].inTransition=false;
							inportStates[IN_HARMONY_CIRCLE_DEGREE_GATE_EXT_CV].lastValue=fgvalue;
							if (fgvalue)  // gate has gone high
							{
								if ( circleDegree==inportStates[IN_HARMONY_CIRCLE_DEGREE_EXT_CV].lastValue) // the gate has changed but the degree has not
									degreeChanged=true;   // not really, but play like it has so it will be replayed below
							}
						}
						else
						if (fgvalue!=inportStates[IN_HARMONY_CIRCLE_DEGREE_GATE_EXT_CV].lastValue)
						{
							harmonyGatePulse.reset();  // kill the pulse in case it is active
			                outputs[OUT_HARMONY_GATE_OUTPUT].setVoltage(0);
							inportStates[IN_HARMONY_CIRCLE_DEGREE_GATE_EXT_CV].lastValue=fgvalue;
						}
					}
					else
					{
						if (fgvalue!=inportStates[IN_HARMONY_CIRCLE_DEGREE_GATE_EXT_CV].lastValue)
						{
							harmonyGatePulse.reset();  // kill the pulse in case it is active
			                outputs[OUT_HARMONY_GATE_OUTPUT].setVoltage(0);
							inportStates[IN_HARMONY_CIRCLE_DEGREE_GATE_EXT_CV].inTransition=true;
							inportStates[IN_HARMONY_CIRCLE_DEGREE_GATE_EXT_CV].lastValue=fgvalue;
						}
					}

					if ( (degreeChanged) || (circleDegree!=inportStates[IN_HARMONY_CIRCLE_DEGREE_EXT_CV].lastValue)) 
					{
						harmonyGatePulse.reset();  // kill the pulse in case it is active
			            outputs[OUT_HARMONY_GATE_OUTPUT].setVoltage(0);
						inportStates[IN_HARMONY_CIRCLE_DEGREE_EXT_CV].lastValue=circleDegree;
						if (circleDegree>=0)
							circleDegree=(float)std::fmod(std::fabs(circleDegree), 1.0f);
						else
							circleDegree=-(float)std::fmod(std::fabs(circleDegree), 1.0f);
						degreeChanged=true; 
						if (circleDegree>=0)
						{
							if ((std::abs(circleDegree)<.005f))  	   theModeScaleProgressionsState.circleDegree=1;
							else
							if ((std::abs(circleDegree-.167f)<.005f))  theModeScaleProgressionsState.circleDegree=2;
							else
							if ((std::abs(circleDegree-.333f)<.005f))  theModeScaleProgressionsState.circleDegree=3;
							else
							if ((std::abs(circleDegree-.417f)<.005f))  theModeScaleProgressionsState.circleDegree=4;
							else
							if ((std::abs(circleDegree-.583f)<.005f))  theModeScaleProgressionsState.circleDegree=5;
							else
							if ((std::abs(circleDegree-.750f)<.005f))  theModeScaleProgressionsState.circleDegree=6;
							else
							if ((std::abs(circleDegree-.917f)<.005f))  theModeScaleProgressionsState.circleDegree=7;
							else
								degreeChanged=false;
						}
						else
						{
							octave-=1;
							if ((std::abs(circleDegree)<.005f))  			 theModeScaleProgressionsState.circleDegree=1;
							else
							if (std::abs(std::abs(circleDegree)-.083)<.005f)  theModeScaleProgressionsState.circleDegree=7;
							else
							if (std::abs(std::abs(circleDegree)-.250)<.005f)  theModeScaleProgressionsState.circleDegree=6;
							else
							if (std::abs(std::abs(circleDegree)-.417)<.005f)  theModeScaleProgressionsState.circleDegree=5;
							else
							if (std::abs(std::abs(circleDegree)-.583)<.005f)  theModeScaleProgressionsState.circleDegree=4;
							else
							if (std::abs(std::abs(circleDegree)-.667)<.005f)  theModeScaleProgressionsState.circleDegree=3;
							else
							if (std::abs(std::abs(circleDegree)-.833)<.005f)  theModeScaleProgressionsState.circleDegree=2;
							else
								degreeChanged=false;
						}
						
					}	
				
			}
			
        	if ((degreeChanged)&&(!skipStep))
			{
				int theCirclePosition=0;

				if (repeatStep)
				{
					theCirclePosition=lastPlayedCirclePosition; 
				}
				else
				{
					if (theModeScaleProgressionsState.circleDegree<1)
						theModeScaleProgressionsState.circleDegree=1;
					if (theModeScaleProgressionsState.circleDegree>7)
						theModeScaleProgressionsState.circleDegree=7;
													
					int step=1;  // default if not found below
					for (int i=0; i<MAX_STEPS; ++i)
					{
						if (theActiveHarmonyType.harmony_steps[i]==theModeScaleProgressionsState.circleDegree)
						{
							step=i;
							break;
						}
					}

					theModeScaleProgressionsState.last_harmony_step=step;
				
					for (int i=0; i<7; ++i)
					{
						if (theCircleOf5ths.theDegreeSemiCircle.degreeElements[i].Degree==theModeScaleProgressionsState.circleDegree)
						{
							theCirclePosition=theCircleOf5ths.theDegreeSemiCircle.degreeElements[i].CircleIndex;
							break;
						}
					}
				}

				last_circle_position=theCirclePosition;
				lastPlayedCirclePosition = theCirclePosition;  
				lastPlayedCircleOctave = octave;
				lastPlayedCircleDegree=theModeScaleProgressionsState.circleDegree;  

				
				userPlaysCirclePositionHarmony(theCirclePosition, octave+theModeScaleProgressionsState.theHarmonyParms.target_octave);  // play immediate
					
				if (running)
				{
					theModeScaleProgressionsState.userControllingHarmonyFromCircle=true;
					theModeScaleProgressionsState.theHarmonyParms.enabled=false;
				}

				for (int i=0; i<12; ++i) 
				{
					CircleStepStates[i] = false;
					lights[LIGHT_LEDBUTTON_CIRCLESTEP_1+i].setBrightness(CircleStepStates[i] ? 1.0f : 0.0f);	
				}
			
				lights[LIGHT_LEDBUTTON_CIRCLESTEP_1+theCirclePosition].setBrightness(1.0f);
			}
			
		} 
		

		//**************************
		if (lightDivider.process())
		{
		}
			
		if (lowFreqClock.process())
		{
			// check controls for changes
		
			if ((fvalue=std::round(params[CONTROL_TEMPOBPM_PARAM].getValue()))!=tempo)
			{
				tempo = fvalue;
			}
			
       		int ivalue=std::round(params[CONTROL_TIMESIGNATURETOP_PARAM].getValue());
			if (ivalue!=time_sig_top)
			{
				time_sig_top = ivalue;
				time_sig_changed=true;
			}	
			ivalue=std::round(params[CONTROL_TIMESIGNATUREBOTTOM_PARAM].getValue());
			if (std::pow(2,ivalue+1)!=time_sig_bottom)
			{
				time_sig_bottom = std::pow(2,ivalue+1);
				
				int melody_note_length_divisor=0;
				if (time_sig_bottom==2)
				  melody_note_length_divisor=1;
				else
				if (time_sig_bottom==4)
				  melody_note_length_divisor=2;
				else
				if (time_sig_bottom==8)
				  melody_note_length_divisor=3;
				else
				if (time_sig_bottom==16)
				  melody_note_length_divisor=4; 
				theModeScaleProgressionsState.theMelodyParms.note_length_divisor=(int)std::pow(2,melody_note_length_divisor);
							
				time_sig_changed=true;
			}
			  
			
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

			for (int i=0; i<ModeScaleProgressions::NUM_INPUTS; ++i)
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
										
							case IN_TIMESIGNATURETOP_EXT_CV:
								if (fvalue>=0.01)
									{
										float ratio=(fvalue/10.0);
										float range=(13);
										int newValue=2 + (int)(ratio*range);
										newValue=clamp(newValue, 2, 15);
										if (newValue!=time_sig_top)
										{
											time_sig_top=newValue;  
											params[CONTROL_TIMESIGNATURETOP_PARAM].setValue((float)newValue);
											time_sig_changed=true;
										}
									}
									break;
							
							case IN_TIMESIGNATUREBOTTOM_EXT_CV:
								if (fvalue>=0.01)
									{
										float ratio=(fvalue/10.0);
										int exp=(int)(ratio*3);
										exp=clamp(exp, 0, 3);
										int newValue=pow(2,exp+1);

										if (newValue!=time_sig_bottom)
										{
											time_sig_bottom=newValue;  
											params[CONTROL_TIMESIGNATUREBOTTOM_PARAM].setValue((float)exp);
											int melody_note_length_divisor=0;
											if (time_sig_bottom==2)
											melody_note_length_divisor=1;
											else
											if (time_sig_bottom==4)
											melody_note_length_divisor=2;
											else
											if (time_sig_bottom==8)
											melody_note_length_divisor=3;
											else
											if (time_sig_bottom==16)
											melody_note_length_divisor=4; 
											theModeScaleProgressionsState.theMelodyParms.note_length_divisor=(int)std::pow(2,melody_note_length_divisor);
											theModeScaleProgressionsState.theArpParms.note_length_divisor=(int)std::pow(2,melody_note_length_divisor+1);
											time_sig_changed=true;
										}
									}
									break;


							// process harmony input ports

							case IN_HARMONY_ENABLE_EXT_CV:
								if (fvalue>0)
									theModeScaleProgressionsState.theHarmonyParms.enabled = true;
								else
								if (fvalue==0)
									theModeScaleProgressionsState.theHarmonyParms.enabled = false;
								else
								if (fvalue<0) 
								{
									// Do nothing.  Allow local parameter control
								}
								break;

							case IN_HARMONY_VOLUME_EXT_CV:
								if (fvalue>=0.01)
								if (fvalue!=theModeScaleProgressionsState.theHarmonyParms.volume)
								{
									fvalue=clamp(fvalue, 0., 10.);
									theModeScaleProgressionsState.theHarmonyParms.volume=fvalue;  
									params[CONTROL_HARMONY_VOLUME_PARAM].setValue(fvalue);
									outputs[OUT_HARMONY_VOLUME_OUTPUT].setVoltage(theModeScaleProgressionsState.theHarmonyParms.volume);
								}
								break;

							case IN_HARMONY_STEPS_EXT_CV:
								if (fvalue>0)
								{
									float ratio=(fvalue/10.0);
									float range=(theActiveHarmonyType.max_steps-theActiveHarmonyType.min_steps);
									int newValue=theActiveHarmonyType.min_steps + (int)(ratio*range);
									newValue=clamp(newValue, theActiveHarmonyType.min_steps, theActiveHarmonyType.max_steps);
									if (newValue!=params[CONTROL_HARMONY_STEPS_PARAM].getValue())
									{
										if ((newValue>=theActiveHarmonyType.min_steps)&&(newValue<=theActiveHarmonyType.max_steps))
										{
											if (((int)newValue>=theHarmonyTypes[harmony_type].min_steps)&&((int)newValue<=theHarmonyTypes[harmony_type].max_steps))
											{
												params[CONTROL_HARMONY_STEPS_PARAM].setValue(newValue);
												theHarmonyTypes[harmony_type].num_harmony_steps=(int)newValue; 
												theActiveHarmonyType.num_harmony_steps=(int)newValue;  
											}
														
											strcpy(theHarmonyTypes[harmony_type].harmony_degrees_desc,"");
											for (int k=0;k<theHarmonyTypes[harmony_type].num_harmony_steps;++k)
											{
												strcat(theHarmonyTypes[harmony_type].harmony_degrees_desc,MSP_circle_of_fifths_arabic_degrees[theHarmonyTypes[harmony_type].harmony_steps[k]]); 
												if (k<(theHarmonyTypes[harmony_type].num_harmony_steps-1)) 
													strcat(theHarmonyTypes[harmony_type].harmony_degrees_desc,"-");
											}
									
											strcpy(theActiveHarmonyType.harmony_degrees_desc,"");
											for (int k=0;k<theActiveHarmonyType.num_harmony_steps;++k)
											{
												strcat(theActiveHarmonyType.harmony_degrees_desc,MSP_circle_of_fifths_arabic_degrees[theActiveHarmonyType.harmony_steps[k]]); 
												if (k<(theActiveHarmonyType.num_harmony_steps-1)) 
													strcat(theActiveHarmonyType.harmony_degrees_desc,"-");
											}

											setup_harmony();  
											savedHarmonySteps = 0;  // no longer want to apply this elsewhere
											//
										}
									}
								}
								else
								{
									// Do nothing.  Allow local control
								}
								break;
							
							case IN_HARMONY_TARGETOCTAVE_EXT_CV:
								if (fvalue>=0.01)
								{
									float ratio=(fvalue/10.0);
									int newValue=1+ (int)(ratio*6);
									newValue=clamp(newValue, 1, 7);
									if (newValue!=theModeScaleProgressionsState.theHarmonyParms.target_octave)
									{
										theModeScaleProgressionsState.theHarmonyParms.target_octave=(int)newValue;  
										theModeScaleProgressionsState.theHarmonyParms.note_avg_target=theModeScaleProgressionsState.theHarmonyParms.target_octave/10.0;
										theModeScaleProgressionsState.theHarmonyParms.range_top=    theModeScaleProgressionsState.theHarmonyParms.note_avg_target + (theModeScaleProgressionsState.theHarmonyParms.note_octave_range/10.0);
										theModeScaleProgressionsState.theHarmonyParms.range_bottom= theModeScaleProgressionsState.theHarmonyParms.note_avg_target - (theModeScaleProgressionsState.theHarmonyParms.note_octave_range/10.0);
										theModeScaleProgressionsState.theHarmonyParms.r1=(theModeScaleProgressionsState.theHarmonyParms.range_top-theModeScaleProgressionsState.theHarmonyParms.range_bottom); 
										params[CONTROL_HARMONY_TARGETOCTAVE_PARAM].setValue(newValue);
									}
								}
								break;

							case IN_HARMONY_ALPHA_EXT_CV:
								if (fvalue>=0.01)
								{
									float newValue=(fvalue/10.0);
									newValue=clamp(newValue, 0., 1.);
									if (newValue!=theModeScaleProgressionsState.theHarmonyParms.alpha)
									{
										theModeScaleProgressionsState.theHarmonyParms.alpha=newValue;  
										params[CONTROL_HARMONY_ALPHA_PARAM].setValue(newValue);
									}
								}
								break;
							
							case IN_HARMONY_RANGE_EXT_CV:
								if (fvalue>=0.01)
								{
									float ratio=(fvalue/10.0);
									float newValue=1+ratio*6;
									newValue=clamp(newValue, 1., 7.);
									if (newValue!=theModeScaleProgressionsState.theHarmonyParms.note_octave_range)
									{
										theModeScaleProgressionsState.theHarmonyParms.note_octave_range=newValue;  

										theModeScaleProgressionsState.theHarmonyParms.note_avg_target=theModeScaleProgressionsState.theHarmonyParms.target_octave/10.0;
										theModeScaleProgressionsState.theHarmonyParms.range_top=    theModeScaleProgressionsState.theHarmonyParms.note_avg_target + (theModeScaleProgressionsState.theHarmonyParms.note_octave_range/10.0);
										theModeScaleProgressionsState.theHarmonyParms.range_bottom= theModeScaleProgressionsState.theHarmonyParms.note_avg_target - (theModeScaleProgressionsState.theHarmonyParms.note_octave_range/10.0);
										theModeScaleProgressionsState.theHarmonyParms.r1=(theModeScaleProgressionsState.theHarmonyParms.range_top-theModeScaleProgressionsState.theHarmonyParms.range_bottom); 

										params[CONTROL_HARMONY_RANGE_PARAM].setValue(newValue);
									}
								}
								break;

							case IN_HARMONY_DIVISOR_EXT_CV:
								if (fvalue>=0.01)
								{
									float ratio=(fvalue/9.0); // allow for CV that doesn't quite get to 10.0
									int exp=(int)(ratio*3);
									exp=clamp(exp, 0, 3);
									int newValue=pow(2,exp);

									if (newValue!=theModeScaleProgressionsState.theHarmonyParms.note_length_divisor)
									{
										theModeScaleProgressionsState.theHarmonyParms.note_length_divisor=newValue;  
										params[CONTROL_HARMONY_DIVISOR_PARAM].setValue((float)exp);
									}
								}
								break;
							
							case IN_ENABLE_HARMONY_4VOICE_OCTAVES_EXT_CV:
								if (fvalue>0)
								{
									theModeScaleProgressionsState.theHarmonyParms.enable_4voice_octaves = true;
									theModeScaleProgressionsState.theHarmonyParms.enable_all_7ths = false;
									theModeScaleProgressionsState.theHarmonyParms.enable_V_7ths=false;
									setup_harmony();  // calculate harmony notes
								}
								else
								if (fvalue==0)
								{
									theModeScaleProgressionsState.theHarmonyParms.enable_4voice_octaves = false;
									setup_harmony();  // calculate harmony notes
								}
								else
								if (fvalue<0) 
								{
									// Do nothing.  Allow local parameter control
								}
								break;

							case IN_ENABLE_HARMONY_ALL7THS_EXT_CV:
								if (fvalue>0)
								{
									theModeScaleProgressionsState.theHarmonyParms.enable_all_7ths = true;
									theModeScaleProgressionsState.theHarmonyParms.enable_V_7ths=false;
									theModeScaleProgressionsState.theHarmonyParms.enable_4voice_octaves =false;
									setup_harmony();  // calculate harmony notes
								}
								else
								if (fvalue==0)
								{
									theModeScaleProgressionsState.theHarmonyParms.enable_all_7ths = false;
									setup_harmony();  // calculate harmony notes
								}
								else
								if (fvalue<0) 
								{
									// Do nothing.  Allow local parameter control
								}
								break;

							case IN_ENABLE_HARMONY_V7THS_EXT_CV:
								if (fvalue>0)
								{
									theModeScaleProgressionsState.theHarmonyParms.enable_V_7ths = true;
									theModeScaleProgressionsState.theHarmonyParms.enable_all_7ths=false;
									theModeScaleProgressionsState.theHarmonyParms.enable_4voice_octaves =false;
									setup_harmony();  // calculate harmony notes
								}
								else
								if (fvalue==0)
								{
									theModeScaleProgressionsState.theHarmonyParms.enable_V_7ths = false;
									setup_harmony();  // calculate harmony notes
								}
								else
								if (fvalue<0) 
								{
									// Do nothing.  Allow local parameter control
								}
								break;

							case IN_ENABLE_HARMONY_TONIC_ON_CH1_EXT_CV:
								if (fvalue>0)
								{
									theModeScaleProgressionsState.theHarmonyParms.send_tonic_on_first_channel = true;
									theModeScaleProgressionsState.theHarmonyParms.send_bass_on_first_channel=false;
								}
								else
								if (fvalue==0)
								{
									theModeScaleProgressionsState.theHarmonyParms.send_tonic_on_first_channel = false;
									theModeScaleProgressionsState.theHarmonyParms.send_bass_on_first_channel = true;
								}
								else
								if (fvalue<0) 
								{
									// Do nothing.  Allow local parameter control
								}
								break;

								case IN_ENABLE_HARMONY_BASS_ON_CH1_EXT_CV:
								if (fvalue>0)
								{
									theModeScaleProgressionsState.theHarmonyParms.send_bass_on_first_channel = true;
									theModeScaleProgressionsState.theHarmonyParms.send_tonic_on_first_channel=false;
								}
								else
								if (fvalue==0)
								{
									theModeScaleProgressionsState.theHarmonyParms.send_bass_on_first_channel = false;
									theModeScaleProgressionsState.theHarmonyParms.send_tonic_on_first_channel = true;
								}
								else
								if (fvalue<0) 
								{
									// Do nothing.  Allow local parameter control
								}
								break;

							case IN_ENABLE_HARMONY_STACCATO_EXT_CV:
								if (fvalue>0)
								{
									theModeScaleProgressionsState.theHarmonyParms.enable_staccato = true;
								}
								else
								if (fvalue==0)
								{
									theModeScaleProgressionsState.theHarmonyParms.enable_staccato = false;
								}
								else
								if (fvalue<0) 
								{
									// Do nothing.  Allow local parameter control
								}
								break;

							case IN_HARMONYPRESETS_EXT_CV: 
								if (fvalue>=0.01)
								{
									float ratio=(fvalue/10.0);
									float newValue=1.+ (ratio*(MAX_AVAILABLE_HARMONY_PRESETS-1));
									newValue=clamp(newValue, 1., (float)MAX_AVAILABLE_HARMONY_PRESETS);
									if ((int)newValue!=harmony_type)
									{
										if (newValue==4)
										{
											harmony_type=(int)newValue;
											init_custom_harmony();
											copyHarmonyTypeToActiveHarmonyType(harmony_type);
											harmonyPresetChanged=0;
									    	circleChanged=false;  // don't trigger off reconstruction and setup
										}
										else
										harmonyPresetChanged=(int)newValue;  // don't changed until between sequences.  The new harmony_type is in harmonyPresetChanged
									}
									else
									{
										// Ao nothing.  Allow local control
									}
								}
								break;

						
						    // handle mode and root input changes
						  
						    case IN_ROOT_KEY_EXT_CV: 
							if (!theModeScaleProgressionsState.RootInputSuppliedByRootOutput)  // ModeScaleProgressions root key input is NOT fed by ModeScaleProgressions root key output
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
								if (!theModeScaleProgressionsState.ModeInputSuppliedByModeOutput)  // ModeScaleProgressions mode scale input is NOT fed by ModeScaleProgressions mode scale  key output
								{
									float ratio=(fvalue/10.0);
									int newValue=(int)(ratio*6);
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
									}  // end if (newvalue!=)
								}
								else  // ModeScaleProgressions mode scale input IS fed by ModeScaleProgressions mode scale  key output
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

			fvalue=(params[CONTROL_HARMONY_VOLUME_PARAM].getValue());
			if (fvalue!=theModeScaleProgressionsState.theHarmonyParms.volume)
			{
				theModeScaleProgressionsState.theHarmonyParms.volume=fvalue;  
				outputs[OUT_HARMONY_VOLUME_OUTPUT].setVoltage(theModeScaleProgressionsState.theHarmonyParms.volume);
			}

			fvalue=std::round(params[CONTROL_HARMONY_TARGETOCTAVE_PARAM].getValue());
			if (fvalue!=theModeScaleProgressionsState.theHarmonyParms.target_octave)
			{
				theModeScaleProgressionsState.theHarmonyParms.target_octave=fvalue;  
				theModeScaleProgressionsState.theHarmonyParms.note_avg_target=theModeScaleProgressionsState.theHarmonyParms.target_octave/10.0;
				theModeScaleProgressionsState.theHarmonyParms.range_top=    theModeScaleProgressionsState.theHarmonyParms.note_avg_target + (theModeScaleProgressionsState.theHarmonyParms.note_octave_range/10.0);
				theModeScaleProgressionsState.theHarmonyParms.range_bottom= theModeScaleProgressionsState.theHarmonyParms.note_avg_target - (theModeScaleProgressionsState.theHarmonyParms.note_octave_range/10.0);
				theModeScaleProgressionsState.theHarmonyParms.r1=(theModeScaleProgressionsState.theHarmonyParms.range_top-theModeScaleProgressionsState.theHarmonyParms.range_bottom); 
			}

			
			fvalue=params[CONTROL_HARMONY_DIVISOR_PARAM].getValue();
			ivalue=(int)fvalue;
			ivalue=pow(2,ivalue);
			if ((ivalue)!=theModeScaleProgressionsState.theHarmonyParms.note_length_divisor)
			{
				theModeScaleProgressionsState.theHarmonyParms.note_length_divisor=ivalue;  
			}

			fvalue=(params[CONTROL_HARMONY_RANGE_PARAM].getValue());
			if (fvalue!=theModeScaleProgressionsState.theMelodyParms.note_octave_range)
			{
				theModeScaleProgressionsState.theHarmonyParms.note_octave_range=fvalue;  
				theModeScaleProgressionsState.theHarmonyParms.note_avg_target=theModeScaleProgressionsState.theHarmonyParms.target_octave/10.0;
				theModeScaleProgressionsState.theHarmonyParms.range_top=    theModeScaleProgressionsState.theHarmonyParms.note_avg_target + (theModeScaleProgressionsState.theHarmonyParms.note_octave_range/10.0);
				theModeScaleProgressionsState.theHarmonyParms.range_bottom= theModeScaleProgressionsState.theHarmonyParms.note_avg_target - (theModeScaleProgressionsState.theHarmonyParms.note_octave_range/10.0);
				theModeScaleProgressionsState.theHarmonyParms.r1=(theModeScaleProgressionsState.theHarmonyParms.range_top-theModeScaleProgressionsState.theHarmonyParms.range_bottom); 
			}

			fvalue=(params[CONTROL_HARMONY_ALPHA_PARAM].getValue());
			if ((fvalue)!=theModeScaleProgressionsState.theHarmonyParms.alpha)
			{
				theModeScaleProgressionsState.theHarmonyParms.alpha=fvalue;  
			}

		
			//********* 

			if ((fvalue=std::round(params[CONTROL_HARMONYPRESETS_PARAM].getValue()))!=harmony_type)
			{
				if (((int)fvalue)==4)
				{
					harmony_type=(int)fvalue;
					init_custom_harmony();
				    copyHarmonyTypeToActiveHarmonyType(harmony_type);
					params[CONTROL_HARMONY_STEPS_PARAM].setValue(16);
					harmonyPresetChanged=0;
					circleChanged=false;  // don't trigger off reconstruction and setup
				}
				else
					harmonyPresetChanged=(int)fvalue;  // don't changed until between sequences.  The new harmony_type is in harmonyPresetChanged
			}

			fvalue=std::round(params[CONTROL_HARMONY_STEPS_PARAM].getValue());
			if (fvalue!=theActiveHarmonyType.num_harmony_steps)
			{
				if ((fvalue>=theActiveHarmonyType.min_steps)&&(fvalue<=theActiveHarmonyType.max_steps))
				{
					if ((fvalue>=theActiveHarmonyType.min_steps)&&(fvalue<=theActiveHarmonyType.max_steps))
					{
						theActiveHarmonyType.num_harmony_steps=(int)fvalue; 
						theHarmonyTypes[harmony_type].num_harmony_steps=(int)fvalue; 
					}
								
					strcpy(theActiveHarmonyType.harmony_degrees_desc,"");
					for (int k=0;k<theActiveHarmonyType.num_harmony_steps;++k)
					{
						strcat(theActiveHarmonyType.harmony_degrees_desc,MSP_circle_of_fifths_arabic_degrees[theActiveHarmonyType.harmony_steps[k]]); 
						if (k<(theActiveHarmonyType.num_harmony_steps-1)) 
							strcat(theActiveHarmonyType.harmony_degrees_desc,"-");
					}
														
					strcpy(theHarmonyTypes[harmony_type].harmony_degrees_desc,"");
					for (int k=0;k<theHarmonyTypes[harmony_type].num_harmony_steps;++k)
					{
						strcat(theHarmonyTypes[harmony_type].harmony_degrees_desc,MSP_circle_of_fifths_arabic_degrees[theHarmonyTypes[harmony_type].harmony_steps[k]]); 
						if (k<(theHarmonyTypes[harmony_type].num_harmony_steps-1)) 
							strcat(theHarmonyTypes[harmony_type].harmony_degrees_desc,"-");
					}
					
					savedHarmonySteps = 0;  // no longer want to apply this elsewhere 

					setup_harmony();  // seems to work
				}
				//
			}

			// **************************

			if (harmonyPresetChanged) 
			{
				harmony_type=harmonyPresetChanged;
				harmonyPresetChanged=0;
				circleChanged=true;  // trigger off reconstruction and setup
				if (harmony_type!=4)  // not custom
				{
					copyHarmonyTypeToActiveHarmonyType(harmony_type);
					init_harmony(); // reinitialize in case user has changed harmony parms  May be causing max_steps set to 1
					setup_harmony();  // calculate harmony notes
					params[CONTROL_HARMONY_STEPS_PARAM].setValue(theHarmonyTypes[harmony_type].num_harmony_steps);
				}  
				params[CONTROL_HARMONYPRESETS_PARAM].setValue(harmony_type);
				time_sig_changed=true;  // forces a reset so things start over
			}
			
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
						strcpy(note_desig[i],MSP_note_desig_flats[i]);
				}
				else
				{
					for (int i=0; i<12; ++i)
						strcpy(note_desig[i],MSP_note_desig_sharps[i]);
				} 
				
				ConstructCircle5ths(circle_root_key, mode);
				ConstructDegreesSemicircle(circle_root_key, mode); //int circleroot_key, int mode)
			
				init_notes();  // depends on mode and root_key		
				setup_harmony();  // calculate harmony notes 
						
				params[CONTROL_HARMONY_STEPS_PARAM].setValue(theActiveHarmonyType.num_harmony_steps); 
			
				circleChanged=false;
			
				onResetScale();
				onResetQuantizer();
			
				if (savedHarmonySteps)
				{
					params[CONTROL_HARMONY_STEPS_PARAM].setValue(savedHarmonySteps);
					savedHarmonySteps = 0;  // just do it once
				}
				else
					params[CONTROL_HARMONY_STEPS_PARAM].setValue(theActiveHarmonyType.num_harmony_steps);
			
			}
		
		}	

		if (sec1Clock.process())
		{
		}
		
		 	     
	}  // end module process()

	~ModeScaleProgressions() 
	{
	}
  
 
	ModeScaleProgressions() 
	{
		time_t rawtime; 
  		time( &rawtime );
   		
		
		lowFreqClock.setDivision(512);  // every 86 samples, 2ms
		sec1Clock.setDivision(44000);
		lightDivider.setDivision(512);  // every 86 samples, 2ms
				   		
		
		initPerlin();
		ConfigureModuleVars();
		ModeScaleProgressionsMusicStructuresInitialize();  // sets module moduleVarsInitialized=true

			
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

		for (int i=0; i<12; ++i)
			lights[LIGHT_CIRCLE_ROOT_KEY_POSITION_1_LIGHT+i].setBrightness(0.0f);
		lights[LIGHT_CIRCLE_ROOT_KEY_POSITION_1_LIGHT+root_key].setBrightness(1.0f);  // loaded root_key might not be 0/C
		
		CircleStepStates[0]=1.0f;
		lights[LIGHT_LEDBUTTON_CIRCLESTEP_1].setBrightness(1.0f);
		
		CircleStepSetStates[0]=1.0f;
		lights[LIGHT_LEDBUTTON_CIRCLESETSTEP_1].setBrightness(1.0f);

//****************** 59 in ports
		configInput(IN_RUN_EXT_CV, "Mono CV Ext. Run Toggle: >0v :");
		configInput(IN_RESET_EXT_CV, "Mono CV Ext. Reset: >0v :");
		configInput(IN_RAND_EXT_CV, "Mono CV Ext. Rand: >0v :");
		configInput(IN_TEMPO_EXT_CV, "Mono CV Ext. Tempo Set: +-v/oct  0v=120 BPM :");
		configInput(IN_TIMESIGNATURETOP_EXT_CV, "Mono CV Ext. Time Signature Top Set: 0.1v-10v=2-15 :");
		configInput(IN_TIMESIGNATUREBOTTOM_EXT_CV, "Mono CV Ext. Time Signature Bottom Set: 0.1v-10v=2-16 :");
		configInput(IN_ROOT_KEY_EXT_CV, "Mono CV Ext. Root Set: 0.1v-10v=C,G,D,A,E,B,F#,Db,Ab,Eb,Bb,F :");
		configInput(IN_SCALE_EXT_CV, "Mono CV Ext. Mode Set: 0.1v-10v=Lydian,Ionian,Mixolydian,Dorian,Aeolian,Phrygian,Locrian :");
		configInput(IN_CLOCK_EXT_CV, "Mono Ext. 8x Clock In: overrides ModeScaleProgressions internal clock :");
		configInput(IN_HARMONY_CIRCLE_DEGREE_EXT_CV, "Mono CV Ext. Circle Degree Set: Octal radix (degree.octave) 1.1v-7.7v :");
		configInput(IN_HARMONY_CIRCLE_DEGREE_GATE_EXT_CV, "Mono Ext. Circle Degree Gate In: 10v or duplicate of Circle Degree input cable :");
	
		configInput(IN_HARMONY_ENABLE_EXT_CV, "Mono CV Ext. Harmony Enable Toggle: >0v :");
		configInput(IN_HARMONY_DESTUTTER_EXT_CV, "Mono CV Ext. Harmony Hold Tied Notes Toggle: >0v :");
		configInput(IN_HARMONY_VOLUME_EXT_CV, "Mono CV Ext. Harmony Volume Set: 0-10v :");
		configInput(IN_HARMONY_STEPS_EXT_CV, "Mono CV Ext. Harmony Progression Steps Set: 0.1v-10v=n=1-N :");
		configInput(IN_HARMONY_TARGETOCTAVE_EXT_CV, "Mono CV Ext. Harmony Target Octave Set: 0.1v-10v=n=1-7 :");
		configInput(IN_HARMONY_ALPHA_EXT_CV, "Mono CV Ext. Harmony Variability Set: 0.1v-10v=0-1.0 :");
		configInput(IN_HARMONY_RANGE_EXT_CV, "Mono CV Ext. Harmony Octave Range Set: 0.1v-10v=+/- 0-3.0 :");
		configInput(IN_HARMONY_DIVISOR_EXT_CV, "Mono CV Ext. Harmony Note On 1/n Set: 0.1v-10v=n=1,2,4,8 :");
		configInput(IN_HARMONYPRESETS_EXT_CV, "Mono CV Ext. Harmony Progression Preset Set: 0.1v-10v= 1-~60 progression number :");
	
		configInput(IN_ENABLE_HARMONY_ALL7THS_EXT_CV, "Mono CV Ext. Harmony Nice 7ths Enable Toggle: >0v :");
		configInput(IN_ENABLE_HARMONY_V7THS_EXT_CV, "Mono CV Ext. Harmony V7ths Enable Toggle: >0v :");
		configInput(IN_ENABLE_HARMONY_TONIC_ON_CH1_EXT_CV, "Mono CV Ext. Harmony Tonic On Ch1 Enable Toggle: >0v :");
		configInput(IN_ENABLE_HARMONY_BASS_ON_CH1_EXT_CV, "Mono CV Ext. Harmony Bass On Ch1 Enable Toggle: >0v :");
		configInput(IN_ENABLE_HARMONY_4VOICE_OCTAVES_EXT_CV, "Mono CV Ext. Harmony 4-voice octave chords Enable Toggle: >0v :");
		configInput(IN_ENABLE_HARMONY_STACCATO_EXT_CV, "Mono CV Ext. Harmony Staccato Enable Toggle: >0v :");

		configInput(IN_PROG_STEP_EXT_CV, "Mono CV Ext. Progression Step Advance: >0v :");
		configInput(IN_POLY_QUANT_EXT_CV, "Poly External Note(s) To Quantize In : v/oct");

//**************** 27 out ports, 24 used

        configOutput(OUT_RUN_OUT, "Mono Run Enable Ext. Momentary: 10v :");
		configOutput(OUT_RESET_OUT, "Mono Reset Enable Ext. Momentary: 10v :");
		configOutput(OUT_TEMPO_OUT, "Mono CV Ext. Tempo : +-v/oct  0v=120 BPM :");
		configOutput(OUT_CLOCK_OUT, "Mono 8x Clock Out:  :"); 
		configOutput(OUT_HARMONY_GATE_OUTPUT, "Mono Harmony Gate Out: 10v :");
		configOutput(OUT_HARMONY_CV_OUTPUT, "Poly Harmony Chord Notes Out: v/oct :");
		configOutput(OUT_CLOCK_BEATX2_OUTPUT, "Mono BeatX2 1ms Trigger/Clock Out: 10v :");
		configOutput(OUT_CLOCK_BAR_OUTPUT, "Mono Bsr 1ms Trigger/Clock Out: 10v :");
		configOutput(OUT_CLOCK_BEATX4_OUTPUT, "Mono BeatX4 1ms Trigger/Clock Out: 10v :");
		configOutput(OUT_CLOCK_BEATX8_OUTPUT, "Mono BeatX8 1ms Trigger/Clock Out: 10v :");
		configOutput(OUT_CLOCK_BEAT_OUTPUT, "Mono Beat 1ms Trigger/Clock Out: 10v :");
		configOutput(OUT_HARMONY_TRIGGER_OUTPUT, "Mono ");  // not used
		configOutput(OUT_HARMONY_VOLUME_OUTPUT, "Mono CV Harmony Volume Out: 0-10v :");
		configOutput(OUT_EXT_POLY_SCALE_OUTPUT, "Poly Scale Out: v/oct Out: 7, 5 or 12 notes");
		configOutput(OUT_EXT_POLY_QUANT_OUTPUT, "Poly  External Note(s) Quantized Out : v/oct :");
		configOutput(OUT_EXT_ROOT_OUTPUT, "Mono Scale Root Note Out: v/oct :");
	    configOutput(OUT_EXT_HARMONIC_DEGREE_OUTPUT, "Mono Circle Degree Out: CV 1-7 V");
		configOutput(OUT_EXT_POLY_QUANT_TRIGGER_OUTPUT, "Poly quantized out :");
		configOutput(OUT_EXT_HARMONIC_DEGREE_CHORD_TYPE_OUTPUT, "Chord Type:");


//**************** 
								
		configButton(BUTTON_RUN_PARAM,  "Run");
		configButton(BUTTON_RESET_PARAM,  "Reset");
		configButton(BUTTON_RAND_PARAM,  "Rand");

		configParam(CONTROL_TEMPOBPM_PARAM, min_bpm, max_bpm, 120.0f, "Tempo", " BPM");
		getParamQuantity(CONTROL_TEMPOBPM_PARAM)->randomizeEnabled=false;
	    configParam(CONTROL_TIMESIGNATURETOP_PARAM,2.0f, 15.0f, 4.0f, "Time Signature Top");
		getParamQuantity(CONTROL_TIMESIGNATURETOP_PARAM)->randomizeEnabled=false;
		configParam(CONTROL_TIMESIGNATUREBOTTOM_PARAM,0.0f, 3.0f, 1.0f, "Time Signature Bottom");
		getParamQuantity(CONTROL_TIMESIGNATUREBOTTOM_PARAM)->randomizeEnabled=false;
		configParam(CONTROL_ROOT_KEY_PARAM, 0, 11, 0.f, "Root/Key");
		getParamQuantity(CONTROL_ROOT_KEY_PARAM)->randomizeEnabled=false;
		configParam(CONTROL_SCALE_PARAM, 0.f,MSP_num_modes-1, 1.f, "Mode");
		getParamQuantity(CONTROL_SCALE_PARAM)->randomizeEnabled=false;
	
		configButton(BUTTON_ENABLE_HARMONY_PARAM,  "Harmony (chords) Enable/Disable");
		configParam(CONTROL_HARMONY_VOLUME_PARAM, 0.f, 10.f, 8.0f, "Volume (0-10)");
		getParamQuantity(CONTROL_HARMONY_VOLUME_PARAM)->randomizeEnabled=false;
		configParam(CONTROL_HARMONY_STEPS_PARAM, 1.f, 16.f, 16.f, "Steps");
		getParamQuantity(CONTROL_HARMONY_STEPS_PARAM)->randomizeEnabled=false;
		configParam(CONTROL_HARMONY_TARGETOCTAVE_PARAM, 1.f, 7.f, 3.f, "Target Octave");
		getParamQuantity(CONTROL_HARMONY_TARGETOCTAVE_PARAM)->randomizeEnabled=false;
		configParam(CONTROL_HARMONY_ALPHA_PARAM, 0.f, 1.f, .9f, "Variability"); 
		configParam(CONTROL_HARMONY_RANGE_PARAM, 0.f, 3.f, 1.f, "Octave Range");
		getParamQuantity(CONTROL_HARMONY_RANGE_PARAM)->randomizeEnabled=false;
		configParam(CONTROL_HARMONY_DIVISOR_PARAM, 0.f, 3.f, 0.f, "Notes Length");
		configButton(BUTTON_ENABLE_HARMONY_ALL7THS_PARAM,  "7ths Enable/Disable");
		configButton(BUTTON_ENABLE_HARMONY_V7THS_PARAM,  "V 7ths Enable/Disable");
		configButton(BUTTON_ENABLE_HARMONY_STACCATO_PARAM,  "Staccato Enable/Disable");
		configButton(BUTTON_ENABLE_HARMONY_4VOICE_OCTAVES_PARAM,  "4-Voice Enable/Disable");
		configButton(BUTTON_ENABLE_HARMONY_TONIC_ON_CH1_PARAM,  "Tonic on Ch1 Enable/Disable");
		configButton(BUTTON_ENABLE_HARMONY_BASS_ON_CH1_PARAM,  "Bass on Ch1 Enable/Disable");
		configParam(CONTROL_HARMONYPRESETS_PARAM, 1.0f, (float)MAX_AVAILABLE_HARMONY_PRESETS, 1.0f, "Progression Preset");
		getParamQuantity(CONTROL_HARMONYPRESETS_PARAM)->randomizeEnabled=false;
   
		configButton(BUTTON_HARMONY_SETSTEP_1_PARAM,  "Step 1");
		configButton(BUTTON_HARMONY_SETSTEP_2_PARAM,  "Step 2");
		configButton(BUTTON_HARMONY_SETSTEP_3_PARAM,  "Step 3");
		configButton(BUTTON_HARMONY_SETSTEP_4_PARAM,  "Step 4");
		configButton(BUTTON_HARMONY_SETSTEP_5_PARAM,  "Step 5");
		configButton(BUTTON_HARMONY_SETSTEP_6_PARAM,  "Step 6");
		configButton(BUTTON_HARMONY_SETSTEP_7_PARAM,  "Step 7");
		configButton(BUTTON_HARMONY_SETSTEP_8_PARAM,  "Step 8");
		configButton(BUTTON_HARMONY_SETSTEP_9_PARAM,  "Step 9");
		configButton(BUTTON_HARMONY_SETSTEP_10_PARAM,  "Step 10");
		configButton(BUTTON_HARMONY_SETSTEP_11_PARAM,  "Step 11");
		configButton(BUTTON_HARMONY_SETSTEP_12_PARAM,  "Step 12");
		configButton(BUTTON_HARMONY_SETSTEP_13_PARAM,  "Step 13");
		configButton(BUTTON_HARMONY_SETSTEP_14_PARAM,  "Step 14");
		configButton(BUTTON_HARMONY_SETSTEP_15_PARAM,  "Step 15");
		configButton(BUTTON_HARMONY_SETSTEP_16_PARAM,  "Step 16");

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

		configButton(BUTTON_PROG_STEP_PARAM,  "Step Harmony Progression");

		onResetScale();
		onResetQuantizer(); 

	}  // end ModeScaleProgressions()
	
};  // end of struct ModeScaleProgressions

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
	float getDefaultValue() override {return MSP_panelContrastDefault;}
	std::string getLabel() override { return label; }
	std::string getUnit() override { return " "; }
};    


struct ModeScaleProgressionsPanelThemeItem : MenuItem {    
    
		ModeScaleProgressions  *module;
        int theme;
 
        void onAction(const event::Action &e) override {
         	MSP_panelTheme = theme;  
		
        };

        void step() override {
        	rightText = (MSP_panelTheme == theme)? "✔" : "";
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

struct ModeScaleProgressionsScaleOutModeItem : MenuItem {   
    
        ModeScaleProgressions *module;
        ModeScaleProgressions::ScaleOutMode mode;

 
        void onAction(const event::Action &e) override {
                module->scale_out_mode = mode;
				module->onResetScale();
        };

        void step() override {
                rightText = (module->scale_out_mode == mode)? "✔" : "";
        };
    
	};

	
struct ModeScaleProgressionsGateOutModeItem : MenuItem {   
    
        ModeScaleProgressions *module;
        ModeScaleProgressions::GateOutMode mode;

 
        void onAction(const event::Action &e) override {
                module->gate_out_mode = mode;
        };

        void step() override {
                rightText = (module->gate_out_mode == mode)? "✔" : "";
        };
    
	};

struct ModeScaleProgressionsDegreeOutRangeItem : MenuItem {   
    
        ModeScaleProgressions *module;
        ModeScaleProgressions::HarmonicDegreeOutCvRangeMode rangemode;

 
        void onAction(const event::Action &e) override {
                module->harmonic_degree_out_mode = rangemode;
        };

        void step() override {
                rightText = (module->harmonic_degree_out_mode == rangemode)? "✔" : "";
        };
    
	};

	
 
struct ModeScaleProgressionsRootKeySelectLineDisplay : LightWidget {

	ModeScaleProgressions *module=NULL;
	int *val = NULL;
	
	ModeScaleProgressionsRootKeySelectLineDisplay() {
	
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
		nvgFillColor(args.vg,MSP_paramTextColor);
		nvgStrokeWidth(args.vg, 3.0);

		char text[128];
		snprintf(text, sizeof(text), "%s",MSP_root_key_names[*val]);
		nvgText(args.vg, textpos.x, textpos.y, text, NULL);
			
		nvgClosePath(args.vg);
	}

};

struct ModeScaleProgressionsScaleSelectLineDisplay : LightWidget {

   	ModeScaleProgressions *module=NULL;
	int *val = NULL;
	
	ModeScaleProgressionsScaleSelectLineDisplay() {
    
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
		nvgFillColor(args.vg,MSP_paramTextColor);  
	
		if (module)  
		{
			char text[128];
			
			snprintf(text, sizeof(text), "%s",MSP_mode_names[*val]);
			nvgText(args.vg, textpos.x, textpos.y, text, NULL);

			// add on the scale notes display out of this box
			nvgFontSize(args.vg, 16);
			nvgFillColor(args.vg,MSP_panelTextColor);
			strcpy(text,"");
			for (int i=0;i<MSP_mode_step_intervals[*val][0];++i)
			{
				strcat(text,module->note_desig[module->notes[i]%MAX_NOTES]);  
				strcat(text," ");
			}
			
			nvgText(args.vg, textpos.x, textpos.y+20, text, NULL);

			strcpy(module->MSPScaleText, text);  // save for module instance use
	   	}
		
		nvgClosePath(args.vg);	
	} 

};

////////////////////////////////////
struct ModeScaleProgressionsBpmDisplayWidget : LightWidget {

  ModeScaleProgressions *module=NULL;	
  float *val = NULL;

 
  ModeScaleProgressionsBpmDisplayWidget() {
 
  };

  void draw(const DrawArgs &args) override {

	if (!module)
		return;

	std::shared_ptr<Font> font = APP->window->loadFont(asset::system("res/fonts/DSEG7ClassicMini-Bold.ttf"));  // load a Rack font, 7-segment display mini bold
		
	// Background
	NVGcolor backgroundColor = nvgRGB(0x20, 0x10, 0x10);
	NVGcolor borderColor = nvgRGB(0x10, 0x10, 0x10);
	nvgBeginPath(args.vg);
	nvgRoundedRect(args.vg, 0.0, 0.0, box.size.x, box.size.y, 4.0);
	nvgFillColor(args.vg, backgroundColor);
	nvgFill(args.vg);
	nvgStrokeWidth(args.vg, 1.0);
	nvgStrokeColor(args.vg, borderColor);
	nvgStroke(args.vg);
	
	if (!val) { 
      return; 
    }

	nvgFontSize(args.vg, 27);
	if (font)
	nvgFontFaceId(args.vg, font->handle);
	nvgTextLetterSpacing(args.vg, 1.6);

	Vec textPos = Vec(0.0f, 32.0f);

    NVGcolor textColor = nvgRGB(0xdf, 0xd2, 0x2c);
 	nvgFillColor(args.vg, nvgTransRGBA(textColor, 24));
	nvgText(args.vg, textPos.x, textPos.y, "888", NULL);

	std::stringstream to_display;  
   	to_display << std::setw(3) << *val;
	
	textPos = Vec(40.0f, 32.0f);   // this is a relative offset within the box.    1  digit BPM, disallowed
	if (*val > 99)   // 3  digit BPM
		textPos = Vec(0.0f, 32.0f);   // this is a relative offset within the box
	else
	if (*val > 0)    // 2  digit BPM
		textPos = Vec(16.75f, 32.0f);   // this is a relative offset within the box
	  
	textColor =MSP_paramTextColor;
    nvgFillColor(args.vg, textColor);
    nvgText(args.vg, textPos.x, textPos.y, to_display.str().c_str(), NULL);
	
  }
};
//////////////////////////////////// 
struct ModeScaleProgressionsSigDisplayWidget : LightWidget {

  int *value = NULL;
  
  ModeScaleProgressionsSigDisplayWidget() {
        
  };

  void draw(const DrawArgs &args) override {

	std::shared_ptr<Font> font = APP->window->loadFont(asset::system("res/fonts/DSEG7ClassicMini-Bold.ttf"));  // load a Rack font: , 7-segment display mini bold

	// Display Background is now drawn on the svg panel, even if Module is null (value=null)
    NVGcolor backgroundColor = nvgRGB(0x20, 0x10, 0x10);
    NVGcolor borderColor = nvgRGB(0x10, 0x10, 0x10);
    nvgBeginPath(args.vg);
    nvgRoundedRect(args.vg, 0.0, 0.0, box.size.x, box.size.y, 4.0);
    nvgFillColor(args.vg, backgroundColor);
    nvgFill(args.vg);
    nvgStrokeWidth(args.vg, 1.0);
    nvgStrokeColor(args.vg, borderColor);
    nvgStroke(args.vg); 

	 if (!value) {
      return;
    }
       
	nvgFontSize(args.vg, 15);  
	if (font)
    nvgFontFaceId(args.vg, font->handle);
    nvgTextLetterSpacing(args.vg, 2.2);

	Vec textPos = Vec(-1.0f, 18.0f);   // this is a relative offset within the box
	 
    NVGcolor textColor = nvgRGB(0xdf, 0xd2, 0x2c);

	nvgFillColor(args.vg, nvgTransRGBA(textColor, 24));
    nvgText(args.vg, textPos.x, textPos.y, "88", NULL);

    std::stringstream to_display;  
   	to_display << std::setw(2) << *value;

	textPos = Vec(8.0f, 18.0f);   // this is a relative offset within the box
	if (*value > 9) 
		textPos = Vec(-1.5f, 18.0f);   // this is a relative offset within the box
	  
	textColor =MSP_paramTextColor;
    nvgFillColor(args.vg, textColor);
    nvgText(args.vg, textPos.x, textPos.y, to_display.str().c_str(), NULL);
  }
};
 
 
struct ModeScaleProgressionsWidget : ModuleWidget  
{
	SvgPanel* svgPanel;
	SvgPanel* darkPanel;
	
	rack::math::Rect  ParameterRect[MAX_PARAMS];  // warning, don't exceed the dimension
    rack::math::Rect  InportRect[MAX_INPORTS];  // warning, don't exceed the dimension
    rack::math::Rect  OutportRect[MAX_OUTPORTS];  // warning, don't exceed the dimension
 
	ParamWidget* paramWidgets[ModeScaleProgressions::NUM_PARAMS]={0};  // keep track of all ParamWidgets as they are created so they can be moved around later  by the enum parmam ID
	LightWidget* lightWidgets[ModeScaleProgressions::NUM_LIGHTS]={0};  // keep track of all LightWidgets as they are created so they can be moved around later  by the enum parmam ID

	PortWidget* outPortWidgets[ModeScaleProgressions::NUM_OUTPUTS]={0};  // keep track of all output TPortWidgets as they are created so they can be moved around later  by the enum parmam ID
	PortWidget* inPortWidgets[ModeScaleProgressions::NUM_INPUTS]={0};  // keep track of all output TPortWidgets as they are created so they can be moved around later  by the enum parmam ID
	
	struct CircleOf5thsDisplay : TransparentWidget 
	{
		ModeScaleProgressions* module;
		rack::math::Rect*  ParameterRectLocal;   // warning, don't exceed the dimension
		rack::math::Rect*  InportRectLocal; 	 // warning, don't exceed the dimension
		rack::math::Rect*  OutportRectLocal;     // warning, don't exceed the dimension
			
		CircleOf5thsDisplay(ModeScaleProgressions* module)  
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
					nvgFillColor(args.vg,MSP_panelTextColor);
					char text[32];
					snprintf(text, sizeof(text), "%s",MSP_CircleNoteNames[i]);
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
						snprintf(text, sizeof(text), "%s",MSP_circle_of_fifths_degrees_UC[(i - module->theCircleOf5ths.theDegreeSemiCircle.RootKeyCircle5thsPosition+7)%7]);
					else
					if ((chord_type==1)||(chord_type==6)) // minor or diminished
						snprintf(text, sizeof(text), "%s",MSP_circle_of_fifths_degrees_LC[(i - module->theCircleOf5ths.theDegreeSemiCircle.RootKeyCircle5thsPosition+7)%7]);
					
					
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

		void drawHarmonyControlParamLine(const DrawArgs &args, Vec paramControlPos, const char* label, float value, int valueDecimalPoints, NVGcolor textColor=nvgRGBA(0,0,0, 0xff), float fontSize=14)
		{
			Vec displayRectPos= paramControlPos.plus(Vec(128, 0)); 
			nvgBeginPath(args.vg);
		
			if (valueDecimalPoints>=0)
				nvgRoundedRect(args.vg, displayRectPos.x,displayRectPos.y, 45.f, 20.f, 4.f);
			
			char text[128];

			snprintf(text, sizeof(text), "%s", label);
			nvgFillColor(args.vg, textColor);
				
			nvgFontSize(args.vg, fontSize);
			nvgTextAlign(args.vg,NVG_ALIGN_LEFT|NVG_ALIGN_MIDDLE);
			nvgText(args.vg, paramControlPos.x+20, paramControlPos.y+10, text, NULL);

			nvgFillColor(args.vg, nvgRGBA( 0x2f,  0x27, 0x0a, 0xff));
			nvgFill(args.vg);
			nvgFillColor(args.vg,MSP_paramTextColor);
			nvgFontSize(args.vg, 17);  // larger for inside value box
			if (valueDecimalPoints>=0)
			{
				nvgTextAlign(args.vg,NVG_ALIGN_CENTER|NVG_ALIGN_MIDDLE);
				if (valueDecimalPoints==0)
					snprintf(text, sizeof(text), "%d", (int)value);
				else
				if (valueDecimalPoints==1)
					snprintf(text, sizeof(text), "%.1lf", value);
				else
				if (valueDecimalPoints==2)
					snprintf(text, sizeof(text), "%.2lf", value);
				nvgText(args.vg, displayRectPos.x+25, displayRectPos.y+10, text, NULL);
			}
			
		}
   
		void drawLabelAbove(const DrawArgs &args, Rect rect, const char* label, float fontSize)  
		{
			std::shared_ptr<Font> textfont = APP->window->loadFont(asset::plugin(pluginInstance, "res/Ubuntu Condensed 400.ttf"));
								    	
			nvgBeginPath(args.vg);
			nvgFillColor(args.vg,MSP_panelTextColor);
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
			nvgFillColor(args.vg,MSP_panelTextColor);
			nvgFontSize(args.vg, 14);
			if (textfont)
			nvgFontFaceId(args.vg, textfont->handle);
			nvgTextLetterSpacing(args.vg, -1);
			nvgTextAlign(args.vg,NVG_ALIGN_LEFT|NVG_ALIGN_MIDDLE);
			nvgText(args.vg, rect.pos.x+rect.size.x+2, rect.pos.y+rect.size.y/2., label, NULL);
		}

		void drawLabelLeft(const DrawArgs &args, Rect rect, const char* label, float xoffset)  
		{
			std::shared_ptr<Font> textfont = APP->window->loadFont(asset::plugin(pluginInstance, "res/Ubuntu Condensed 400.ttf"));
								    	
			nvgBeginPath(args.vg);
			nvgFillColor(args.vg,MSP_panelTextColor);
			nvgFontSize(args.vg, 14);
			if (textfont)
			nvgFontFaceId(args.vg, textfont->handle);
			nvgTextLetterSpacing(args.vg, -1);
			nvgTextAlign(args.vg,NVG_ALIGN_LEFT|NVG_ALIGN_MIDDLE);
			nvgText(args.vg, rect.pos.x-rect.size.x-2+xoffset, rect.pos.y+rect.size.y/2., label, NULL);
		}

		void drawLabelOffset(const DrawArgs &args, Rect rect, const char* label, float xoffset, float yoffset, int fontsize=14)  
		{
			std::shared_ptr<Font> textfont = APP->window->loadFont(asset::plugin(pluginInstance, "res/Ubuntu Condensed 400.ttf"));
								    	
			nvgBeginPath(args.vg);
			nvgFillColor(args.vg,MSP_panelTextColor);
			nvgFontSize(args.vg, fontsize);
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
			nvgFillColor(args.vg,MSP_panelTextColor);
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
   
		// New code for scale keyobard
        void drawScaleKey(const DrawArgs &args, int k, bool state, int source) // state true means key pressed. Source 0=generic, 1=harmony chord, 2=melody, 3=arp, 4= bass, 5=drone
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
				whiteNoteOnColor=MSP_panelHarmonyKeyboardColor;
				blackNoteOnColor=MSP_panelHarmonyKeyboardColor;
			}
			if (source==1)
			{
				whiteNoteOnColor=MSP_panelMelodyKeyboardColor;
				blackNoteOnColor=MSP_panelMelodyKeyboardColor;
			}
			if (source==2)
			{
				whiteNoteOnColor=MSP_panelArpKeyboardColor;
				blackNoteOnColor=MSP_panelArpKeyboardColor;
			}
			if (source==3)
			{
				whiteNoteOnColor=MSP_panelBassKeyboardColor;
				blackNoteOnColor=MSP_panelBassKeyboardColor;
			}
		
			int numWhiteKeys=52;
			float beginLeftEdge = 220.0;
			float begintopEdge = 345.0;
			float keyboardLength=230.0;
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
		
		void drawScaleKeyboard(const DrawArgs &args) // draw a piano keyboard
		{
			if (!module)
				return;

			for (int k=21; k<109; ++k) 
				drawScaleKey(args, k, false, 0);
        }		
		//

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

						    	
			if (true)  // Harmony  position a paramwidget  can't access paramWidgets here  
			{
				nvgBeginPath(args.vg);
				nvgRoundedRect(args.vg, 484.f,25.f, 196.f, 353.f, 4.f);
				nvgFillColor(args.vg, nvgRGBA(MSP_panelcolor.r, MSP_panelcolor.g,MSP_panelcolor.b,MSP_panelcolor.a));
				nvgStrokeColor(args.vg,nvgRGBA( 0x80,  0x80 , 0x80, 0x80));
				nvgStrokeWidth(args.vg, 2.5);
				nvgStroke(args.vg); 
				
				Vec pos;
				Vec displayRectPos;
				Vec paramControlPos;
				char text[128];
				char labeltext[128];
				nvgFontSize(args.vg, 17);
				if (textfont)
				nvgFontFaceId(args.vg, textfont->handle);
				nvgTextLetterSpacing(args.vg, -1);
				nvgFillColor(args.vg,MSP_panelTextColor);
				nvgStrokeColor(args.vg,nvgRGBA( 0x80,  0x80 , 0x80, 0x80));
				nvgStrokeWidth(args.vg, 2.0); 

				// draw set steps text 
				for(int i = 0; i<MAX_STEPS;++i) 
				{
					if (i==0)
					{
						snprintf(labeltext, sizeof(labeltext), "Set Step");
						drawLabelAbove(args, ParameterRectLocal[ModeScaleProgressions::BUTTON_HARMONY_SETSTEP_1_PARAM+i], labeltext, 15.);  
					}
					snprintf(labeltext, sizeof(labeltext), "%d", i+1);
					drawLabelLeft(args,ParameterRectLocal[ModeScaleProgressions::BUTTON_HARMONY_SETSTEP_1_PARAM+i], labeltext, 0.);  
				}

              

				//***************
				// update harmony panel begin

				nvgStrokeColor(args.vg,nvgRGBA( 0x80,  0x00 , 0x00, 0x80));
		
				snprintf(labeltext, sizeof(labeltext), "%s", "Harmony Chords Enable");
				drawHarmonyControlParamLine(args,ParameterRectLocal[ModeScaleProgressions::BUTTON_ENABLE_HARMONY_PARAM].pos, labeltext, 0, -1,MSP_panelHarmonyPartColor);
				

				nvgStrokeColor(args.vg,nvgRGBA( 0x80,  0x80 , 0x80, 0x80));

				snprintf(labeltext, sizeof(labeltext), "%s", "Volume (0-10.0)");
				drawHarmonyControlParamLine(args,ParameterRectLocal[ModeScaleProgressions::CONTROL_HARMONY_VOLUME_PARAM].pos, labeltext, module->theModeScaleProgressionsState.theHarmonyParms.volume, 1,MSP_panelTextColor);
						    
				snprintf(labeltext, sizeof(labeltext), "Steps (%d-%d)", module->theActiveHarmonyType.min_steps, module->theActiveHarmonyType.max_steps);
				drawHarmonyControlParamLine(args,ParameterRectLocal[ModeScaleProgressions::CONTROL_HARMONY_STEPS_PARAM].pos, labeltext, (float)module->theActiveHarmonyType.num_harmony_steps, 0,MSP_panelTextColor);
				
				snprintf(labeltext, sizeof(labeltext), "%s", "Target Oct.(1-7)");
				drawHarmonyControlParamLine(args,ParameterRectLocal[ModeScaleProgressions::CONTROL_HARMONY_TARGETOCTAVE_PARAM].pos, labeltext, module->theModeScaleProgressionsState.theHarmonyParms.target_octave, 0,MSP_panelTextColor);

				snprintf(labeltext, sizeof(labeltext), "%s", "Variability (0-1)");
				drawHarmonyControlParamLine(args,ParameterRectLocal[ModeScaleProgressions::CONTROL_HARMONY_ALPHA_PARAM].pos, labeltext, module->theModeScaleProgressionsState.theHarmonyParms.alpha, 2,MSP_panelTextColor);

				snprintf(labeltext, sizeof(labeltext), "%s", "+-Octave Range (0-3)");
				drawHarmonyControlParamLine(args,ParameterRectLocal[ModeScaleProgressions::CONTROL_HARMONY_RANGE_PARAM].pos, labeltext, module->theModeScaleProgressionsState.theHarmonyParms.note_octave_range, 2,MSP_panelTextColor);

				snprintf(labeltext, sizeof(labeltext), "%s", "Notes on                    1/");
				drawHarmonyControlParamLine(args,ParameterRectLocal[ModeScaleProgressions::CONTROL_HARMONY_DIVISOR_PARAM].pos, labeltext, module->theModeScaleProgressionsState.theHarmonyParms.note_length_divisor, 0,MSP_panelTextColor);

				snprintf(labeltext, sizeof(labeltext), "%s", "~Nice 7ths");
				drawHarmonyControlParamLine(args,ParameterRectLocal[ModeScaleProgressions::BUTTON_ENABLE_HARMONY_ALL7THS_PARAM].pos, labeltext, 0, -1,MSP_panelTextColor);

				snprintf(labeltext, sizeof(labeltext), "%s", "V 7ths");
				drawHarmonyControlParamLine(args,ParameterRectLocal[ModeScaleProgressions::BUTTON_ENABLE_HARMONY_V7THS_PARAM].pos, labeltext, 0, -1,MSP_panelTextColor);

				snprintf(labeltext, sizeof(labeltext), "%s", "4-Voice Octaves");
				drawHarmonyControlParamLine(args,ParameterRectLocal[ModeScaleProgressions::BUTTON_ENABLE_HARMONY_4VOICE_OCTAVES_PARAM].pos, labeltext, 0, -1,MSP_panelTextColor);

				snprintf(labeltext, sizeof(labeltext), "%s", "Staccato");
				drawHarmonyControlParamLine(args,ParameterRectLocal[ModeScaleProgressions::BUTTON_ENABLE_HARMONY_STACCATO_PARAM].pos, labeltext, 0, -1,MSP_panelTextColor);

				snprintf(labeltext, sizeof(labeltext), "%s", "Tonic ch1");
				drawHarmonyControlParamLine(args,ParameterRectLocal[ModeScaleProgressions::BUTTON_ENABLE_HARMONY_TONIC_ON_CH1_PARAM].pos, labeltext, 0, -1,MSP_panelTextColor);

				snprintf(labeltext, sizeof(labeltext), "%s", "Bass  ch1");
				drawHarmonyControlParamLine(args,ParameterRectLocal[ModeScaleProgressions::BUTTON_ENABLE_HARMONY_BASS_ON_CH1_PARAM].pos, labeltext, 0, -1,MSP_panelTextColor);

				snprintf(labeltext, sizeof(labeltext), "%s", "Progression Presets");
				drawHarmonyControlParamLine(args,ParameterRectLocal[ModeScaleProgressions::CONTROL_HARMONYPRESETS_PARAM].pos, labeltext, 0, -1,MSP_panelTextColor);
			
				//  do the progression displays
				pos =ParameterRectLocal[ModeScaleProgressions::CONTROL_HARMONYPRESETS_PARAM].pos.plus(Vec(-20,45));
							
				nvgBeginPath(args.vg);
				nvgFillColor(args.vg, nvgRGBA( 0x2f,  0x27, 0x0a, 0xff));
				nvgRoundedRect(args.vg, pos.x,pos.y, 195.f, 20.f, 4.f);
				nvgStrokeColor(args.vg,nvgRGBA( 0x80,  0x80 , 0x80, 0x80));
				nvgStrokeWidth(args.vg, 2.0);
				nvgFill(args.vg);
				nvgStroke(args.vg);
		
				nvgBeginPath(args.vg);
				nvgFontSize(args.vg, 14);
				if (textfont)
				nvgFontFaceId(args.vg, textfont->handle);
				nvgTextLetterSpacing(args.vg, -1);
				nvgFillColor(args.vg,MSP_paramTextColor);
				snprintf(text, sizeof(text), "#%d:  %s", module->harmony_type, module->theActiveHarmonyType.harmony_type_desc);
				nvgText(args.vg, pos.x+5, pos.y+10, text, NULL);
				
				pos = pos.plus(Vec(0,20));

							
				nvgBeginPath(args.vg);
				nvgFillColor(args.vg, nvgRGBA( 0x2f,  0x27, 0x0a, 0xff));
				nvgStrokeColor(args.vg,nvgRGBA( 0x80,  0x80 , 0x80, 0x80));
				nvgStrokeWidth(args.vg, 2.0);
				nvgRoundedRect(args.vg, pos.x,pos.y, 195.f, 20.f, 4.f);
				nvgFill(args.vg);
				nvgStroke(args.vg);

		
				nvgBeginPath(args.vg);
				nvgFontSize(args.vg, 12);
				nvgFillColor(args.vg,MSP_paramTextColor);
				snprintf(text, sizeof(text), "%s           ",  module->theActiveHarmonyType.harmony_degrees_desc);
				nvgText(args.vg, pos.x+5, pos.y+10, text, NULL);
										
			}
        
			if (true)  // draw rounded corner rects  for input jacks border 
			{
				char labeltext[128];
					
				snprintf(labeltext, sizeof(labeltext), "%s", "RUN");
				drawLabelAbove(args,ParameterRectLocal[ModeScaleProgressions::BUTTON_RUN_PARAM], labeltext, 12.);

				snprintf(labeltext, sizeof(labeltext), "%s", "Out");
				drawOutport(args, OutportRectLocal[ModeScaleProgressions::OUT_RUN_OUT].pos, labeltext, 0, 1);
				
			
				snprintf(labeltext, sizeof(labeltext), "%s", "RESET");
				drawLabelAbove(args,ParameterRectLocal[ModeScaleProgressions::BUTTON_RESET_PARAM], labeltext, 12.);

				snprintf(labeltext, sizeof(labeltext), "%s", "RAND");
				drawLabelAbove(args,ParameterRectLocal[ModeScaleProgressions::BUTTON_RAND_PARAM], labeltext, 12.);
				
				snprintf(labeltext, sizeof(labeltext), "%s", "Out");
				drawOutport(args, OutportRectLocal[ModeScaleProgressions::OUT_RESET_OUT].pos, labeltext, 0, 1);
			
				snprintf(labeltext, sizeof(labeltext), "%s", "BPM");
				drawLabelRight(args,ParameterRectLocal[ModeScaleProgressions::CONTROL_TEMPOBPM_PARAM], labeltext);

				snprintf(labeltext, sizeof(labeltext), "%s", "Time Sig Top");
				drawLabelRight(args,ParameterRectLocal[ModeScaleProgressions::CONTROL_TIMESIGNATURETOP_PARAM], labeltext);

				snprintf(labeltext, sizeof(labeltext), "%s", "Time Sig Bottom");
				drawLabelRight(args,ParameterRectLocal[ModeScaleProgressions::CONTROL_TIMESIGNATUREBOTTOM_PARAM], labeltext);

				snprintf(labeltext, sizeof(labeltext), "%s", "Root");
				drawLabelRight(args,ParameterRectLocal[ModeScaleProgressions::CONTROL_ROOT_KEY_PARAM], labeltext);
				
				snprintf(labeltext, sizeof(labeltext), "%s", "Mode: bright->to darkest");
				drawLabelRight(args,ParameterRectLocal[ModeScaleProgressions::CONTROL_SCALE_PARAM], labeltext);

			    snprintf(labeltext, sizeof(labeltext), "%s", "|-----Poly Quantizer-----|");
				drawLabelOffset(args, InportRectLocal[ModeScaleProgressions::IN_POLY_QUANT_EXT_CV], labeltext, -5., -29.);  

				snprintf(labeltext, sizeof(labeltext), "%s", "In");
				drawLabelOffset(args, InportRectLocal[ModeScaleProgressions::IN_POLY_QUANT_EXT_CV], labeltext, +2., -12.); 

				snprintf(labeltext, sizeof(labeltext), "%s", "8x BPM Clock");
				drawLabelOffset(args, InportRectLocal[ModeScaleProgressions::IN_CLOCK_EXT_CV], labeltext, -4., -26.); 

				snprintf(labeltext, sizeof(labeltext), "%s", "In");
				drawLabelOffset(args, InportRectLocal[ModeScaleProgressions::IN_CLOCK_EXT_CV], labeltext, +2., -12.); 
													
			}

			if (true)  // draw rounded corner rects  for output jacks border 
			{
				char labeltext[128];
				snprintf(labeltext, sizeof(labeltext), "%s", "1V/Oct");
				drawOutport(args, OutportRectLocal[ModeScaleProgressions::OUT_HARMONY_CV_OUTPUT].pos, labeltext, 0, 1);

				snprintf(labeltext, sizeof(labeltext), "%s", "Gate");
				drawOutport(args, OutportRectLocal[ModeScaleProgressions::OUT_HARMONY_GATE_OUTPUT].pos, labeltext, 0, 1);

				snprintf(labeltext, sizeof(labeltext), "%s", "Volume");
				drawOutport(args, OutportRectLocal[ModeScaleProgressions::OUT_HARMONY_VOLUME_OUTPUT].pos, labeltext, 0, 1);
           
				snprintf(labeltext, sizeof(labeltext), "%s", "Bar");
				drawOutport(args, OutportRectLocal[ModeScaleProgressions::OUT_CLOCK_BAR_OUTPUT].pos, labeltext, 0, 1);

				snprintf(labeltext, sizeof(labeltext), "%s", "Beat");
				drawOutport(args, OutportRectLocal[ModeScaleProgressions::OUT_CLOCK_BEAT_OUTPUT].pos, labeltext, 0, 1);

				snprintf(labeltext, sizeof(labeltext), "%s", "Beatx2");
				drawOutport(args, OutportRectLocal[ModeScaleProgressions::OUT_CLOCK_BEATX2_OUTPUT].pos, labeltext, 0, 1);

				snprintf(labeltext, sizeof(labeltext), "%s", "1ms Clocked Trigger Pulses");
				rack::math::Rect rect=OutportRectLocal[ModeScaleProgressions::OUT_CLOCK_BEATX2_OUTPUT];
				rect.pos=rect.pos.plus(Vec(0,-16));
				drawLabelAbove(args, rect, labeltext, 18.);
				
				snprintf(labeltext, sizeof(labeltext), "%s", "Beatx4");
				drawOutport(args, OutportRectLocal[ModeScaleProgressions::OUT_CLOCK_BEATX4_OUTPUT].pos, labeltext, 0, 1);

				snprintf(labeltext, sizeof(labeltext), "%s", "Beatx8");
				drawOutport(args, OutportRectLocal[ModeScaleProgressions::OUT_CLOCK_BEATX8_OUTPUT].pos, labeltext, 0, 1);
			
				snprintf(labeltext, sizeof(labeltext), "%s", "Out");
				drawOutport(args, OutportRectLocal[ModeScaleProgressions::OUT_EXT_ROOT_OUTPUT].pos, labeltext, 0, 1, 0.8);  // scale height by 0.8x

                snprintf(labeltext, sizeof(labeltext), "%s", "Poly");
				drawLabelOffset(args, OutportRectLocal[ModeScaleProgressions::OUT_EXT_POLY_SCALE_OUTPUT], labeltext, -26., 7.0); 
				snprintf(labeltext, sizeof(labeltext), "%s", "Ext.->");
				drawLabelOffset(args, OutportRectLocal[ModeScaleProgressions::OUT_EXT_POLY_SCALE_OUTPUT], labeltext, -26., +19.0); 
				snprintf(labeltext, sizeof(labeltext), "%s", "Scale");
				drawLabelOffset(args, OutportRectLocal[ModeScaleProgressions::OUT_EXT_POLY_SCALE_OUTPUT], labeltext, -26., +31.0); 
				
				snprintf(labeltext, sizeof(labeltext), "%s", "Out");
				drawOutport(args, OutportRectLocal[ModeScaleProgressions::OUT_EXT_POLY_SCALE_OUTPUT].pos, labeltext, 0, 1);

				snprintf(labeltext, sizeof(labeltext), "%s", "Quants");
				drawOutport(args, OutportRectLocal[ModeScaleProgressions::OUT_EXT_POLY_QUANT_OUTPUT].pos, labeltext, 0, 1);

				snprintf(labeltext, sizeof(labeltext), "%s", "Trigs");
				drawOutport(args, OutportRectLocal[ModeScaleProgressions::OUT_EXT_POLY_QUANT_TRIGGER_OUTPUT].pos, labeltext, 0, 1);

				snprintf(labeltext, sizeof(labeltext), "%s", "Out");
				drawOutport(args, OutportRectLocal[ModeScaleProgressions::OUT_CLOCK_OUT].pos, labeltext, 0, 1);
						
				snprintf(labeltext, sizeof(labeltext), "%s", "Out");
				drawOutport(args, OutportRectLocal[ModeScaleProgressions::OUT_EXT_SCALE_OUTPUT].pos, labeltext, 0, 1);
           
			}

			
			Vec pos;
			char text[128];
			nvgFontSize(args.vg, 17);
			if (textfont)
			nvgFontFaceId(args.vg, textfont->handle);
			nvgTextLetterSpacing(args.vg, -1);
			nvgFillColor(args.vg,MSP_panelTextColor);
			
			//************************
			// circle area 

			float beginEdge = 295; 
			float beginTop =115;
			float lineWidth=0.75; 
			float stafflineLength=100;
			float barLineVoffset=36.;
			float barLineVlength=60.;
			int yLineSpacing=6;
			float yHalfLineSpacing=3.0f;
			int num_sharps1=0;
			int vertical_offset1=0;
            int num_flats1=0;

			if (true) // draw the circle area treble clef scale
			{
			  
	
		      // draw bar left vertical edge
			  if (beginEdge > 0) {
			    	nvgBeginPath(args.vg);
			    	nvgMoveTo(args.vg, beginEdge, beginTop+barLineVoffset);
			    	nvgLineTo(args.vg, beginEdge, beginTop+barLineVlength);
			    	nvgStrokeColor(args.vg,MSP_panelLineColor);
			    	nvgStrokeWidth(args.vg, lineWidth);
			    	nvgStroke(args.vg);
		    	}
		    	// draw staff lines
		    	nvgBeginPath(args.vg);
			  for (int staff = 36, y = staff; y <= staff + 24; y += yLineSpacing) { 	
				  nvgMoveTo(args.vg, beginEdge, beginTop+y);
				  nvgLineTo(args.vg, beginEdge+stafflineLength, beginTop+y);
			  }
			  nvgStrokeColor(args.vg,MSP_panelLineColor);
			  nvgStrokeWidth(args.vg, lineWidth);
			  nvgStroke(args.vg); 

			  nvgFontSize(args.vg, 90);
			  if (musicfont)
			  nvgFontFaceId(args.vg, musicfont->handle);
			  nvgTextLetterSpacing(args.vg, -1);
			  nvgFillColor(args.vg,MSP_panelLineColor);
			  pos=Vec(beginEdge+12, beginTop+54); 
			  snprintf(text, sizeof(text), "%s",MSP_gClef.c_str());  
			  nvgText(args.vg, pos.x, pos.y, text, NULL);
			
			  nvgFontSize(args.vg, 90);
			  nvgTextAlign(args.vg,NVG_ALIGN_CENTER|NVG_ALIGN_MIDDLE);
			  snprintf(text, sizeof(text), "%s",MSP_sharp.c_str());  
					      
			  for (int i=0; i<7; ++i)
			  {
			    	nvgBeginPath(args.vg);
			    	if (MSP_root_key_signatures_chromaticForder[module->notate_mode_as_signature_root_key][i]==1)
				    {
				      vertical_offset1=MSP_root_key_sharps_vertical_display_offset[num_sharps1];
					  pos=Vec(beginEdge+24+(num_sharps1*5), beginTop+33+(vertical_offset1*yHalfLineSpacing));
					  nvgText(args.vg, pos.x, pos.y, text, NULL);
					  ++num_sharps1;
				    }
			  	  nvgClosePath(args.vg);
			  }	
		
			  snprintf(text, sizeof(text), "%s",MSP_flat.c_str());  
			  vertical_offset1=0;
			  for (int i=6; i>=0; --i)
			  {
			   	 nvgBeginPath(args.vg);
				  if (MSP_root_key_signatures_chromaticForder[module->notate_mode_as_signature_root_key][i]==-1)
				  {
				     vertical_offset1=MSP_root_key_flats_vertical_display_offset[num_flats1];
				   	 pos=Vec(beginEdge+24+(num_flats1*5), beginTop+33+(vertical_offset1*yHalfLineSpacing));
					 nvgText(args.vg, pos.x, pos.y, text, NULL);
					 ++num_flats1;
				  }
				  nvgClosePath(args.vg);
			  }	
			}

		
			nvgFontSize(args.vg, 8);
			
			if (textfont)
			nvgFontFaceId(args.vg, textfont->handle);
			nvgTextLetterSpacing(args.vg, -1);
			nvgFillColor(args.vg,MSP_panelTextColor);

			pos=Vec(beginEdge+0, beginTop+95);  
			snprintf(text, sizeof(text), "%s", "In--");
			nvgText(args.vg, pos.x, pos.y, text, NULL);

			pos=Vec(beginEdge+0, beginTop+115);  
			snprintf(text, sizeof(text), "%s", "In--");
			nvgText(args.vg, pos.x, pos.y, text, NULL);

			pos=Vec(beginEdge+46, beginTop+95);   

			nvgFontSize(args.vg, 8);  // make it a bit smaller to fit and scale best
		
		    if (module->harmonic_degree_out_mode == module->RANGE_1to7)
				snprintf(text, sizeof(text), "%s", "(1-7)  1V/Deg  (1-7)");
			else
				snprintf(text, sizeof(text), "%s", "(1-7)  1V/Deg  (0-6)");
			nvgText(args.vg, pos.x, pos.y, text, NULL);
			
			pos=Vec(beginEdge+30, beginTop+115);  
			snprintf(text, sizeof(text), "%s", "--Gate");
			nvgText(args.vg, pos.x, pos.y, text, NULL);

			snprintf(text, sizeof(text), "%s", "Out");
			drawOutport(args, OutportRectLocal[ModeScaleProgressions::OUT_EXT_HARMONIC_DEGREE_OUTPUT].pos, text, 0, 1, 0.8);  // scale height by 0.8x);

			snprintf(text, sizeof(text), "%s", "Chord Type--");
			drawLabelOffset(args, OutportRectLocal[ModeScaleProgressions::OUT_EXT_HARMONIC_DEGREE_CHORD_TYPE_OUTPUT], text, -31., +12.0, 8); 
			snprintf(text, sizeof(text), "%s", "");
			drawOutport(args, OutportRectLocal[ModeScaleProgressions::OUT_EXT_HARMONIC_DEGREE_CHORD_TYPE_OUTPUT].pos, text, 0, 1, 0.8);  // scale height by 0.8x);

			snprintf(text, sizeof(text), "%s", "--Step");
			drawHarmonyControlParamLine(args,ParameterRectLocal[ModeScaleProgressions::BUTTON_PROG_STEP_PARAM].pos.plus(Vec(0,-2)), text, 0, -1,MSP_panelTextColor, 8);

			nvgFontSize(args.vg, 12);
			

			//************************
		
			float notesPlayingDisplayStartX=525.0;
			float notesPlayingDisplayStartY=5.0;
			float notesPlayingDisplayWidth=150.0;
			float notesPlayingDisplayHeight=19.0;
			float notesPlayingDisplayEndX=notesPlayingDisplayStartX+notesPlayingDisplayWidth;
			float notesPlayingDisplayEndY=notesPlayingDisplayStartY+notesPlayingDisplayHeight;
			float notesPlayingDisplayNoteWidth=37.5;
			float notesPlayingDisplayNoteCenterY= notesPlayingDisplayStartY+(notesPlayingDisplayHeight/2.0);

			
			nvgBeginPath(args.vg);

			nvgFontSize(args.vg, 14);
			if (textfont)
			nvgFontFaceId(args.vg, textfont->handle);
			nvgTextLetterSpacing(args.vg, -1);
			nvgTextAlign(args.vg,NVG_ALIGN_LEFT|NVG_ALIGN_MIDDLE);
			nvgFillColor(args.vg,MSP_panelTextColor);
			snprintf(text, sizeof(text), "%s", "Playing Notes---->");  
			pos=Vec(notesPlayingDisplayStartX-85, notesPlayingDisplayNoteCenterY);  
			nvgText(args.vg, pos.x, pos.y, text, NULL);
		
			nvgStrokeWidth(args.vg, 2.0);
			nvgStrokeColor(args.vg, nvgRGBA( 0x00,  0x00, 0x00, 0xff));
								
			nvgMoveTo(args.vg, notesPlayingDisplayStartX,notesPlayingDisplayStartY);
			nvgLineTo(args.vg, notesPlayingDisplayEndX, notesPlayingDisplayStartY);
			nvgLineTo(args.vg, notesPlayingDisplayEndX, notesPlayingDisplayEndY);
			nvgLineTo(args.vg, notesPlayingDisplayStartX, notesPlayingDisplayEndY);
			nvgLineTo(args.vg, notesPlayingDisplayStartX, notesPlayingDisplayStartY);

			nvgStroke(args.vg);
			nvgClosePath(args.vg);

			// draw vertical parts separator lines dark
			nvgBeginPath(args.vg);
			nvgStrokeWidth(args.vg, 2.0);
	
			nvgMoveTo(args.vg, notesPlayingDisplayStartX+((4*notesPlayingDisplayNoteWidth)),notesPlayingDisplayStartY);
			nvgLineTo(args.vg, notesPlayingDisplayStartX+((4*notesPlayingDisplayNoteWidth)), notesPlayingDisplayEndY);

			nvgStroke(args.vg);
			nvgClosePath(args.vg);

			// draw vertical part notes separator lines light

			nvgBeginPath(args.vg);
			nvgStrokeColor(args.vg, nvgRGBA( 0xc0,  0xc0, 0xc0, 0xff));
		
			nvgMoveTo(args.vg, notesPlayingDisplayStartX+((1*notesPlayingDisplayNoteWidth)),notesPlayingDisplayStartY);
			nvgLineTo(args.vg,notesPlayingDisplayStartX+((1*notesPlayingDisplayNoteWidth)), notesPlayingDisplayEndY);

			nvgMoveTo(args.vg, notesPlayingDisplayStartX+((2*notesPlayingDisplayNoteWidth)),notesPlayingDisplayStartY);
			nvgLineTo(args.vg, notesPlayingDisplayStartX+((2*notesPlayingDisplayNoteWidth)), notesPlayingDisplayEndY);

			nvgMoveTo(args.vg, notesPlayingDisplayStartX+((3*notesPlayingDisplayNoteWidth)),notesPlayingDisplayStartY);
			nvgLineTo(args.vg, notesPlayingDisplayStartX+((3*notesPlayingDisplayNoteWidth)), notesPlayingDisplayEndY);
			
			nvgStroke(args.vg);
			nvgClosePath(args.vg);
			
			if (module->moduleVarsInitialized)  // globals fully initialized if Module!=NULL
			{
				nvgFontSize(args.vg, 14);
				if (textfont)
				nvgFontFaceId(args.vg, textfont->handle);
				nvgTextLetterSpacing(args.vg, -1);
				nvgFillColor(args.vg,MSP_panelTextColor);
				nvgTextAlign(args.vg,NVG_ALIGN_CENTER|NVG_ALIGN_MIDDLE);

				// write last harmony note played 1
				if (module->theModeScaleProgressionsState.theHarmonyParms.last[0].note!=0)
				{
					if ((module->theModeScaleProgressionsState.theHarmonyParms.last[0].note>=0)&&(module->theModeScaleProgressionsState.theHarmonyParms.last[0].note<128))
					{
						pos=Vec(notesPlayingDisplayStartX+20, notesPlayingDisplayNoteCenterY);  
						snprintf(text, sizeof(text), "%s%d", module->note_desig[(module->theModeScaleProgressionsState.theHarmonyParms.last[0].note%12)] , module->theModeScaleProgressionsState.theHarmonyParms.last[0].note/12);
						nvgFillColor(args.vg,MSP_panelHarmonyPartColor); 
						nvgText(args.vg, pos.x, pos.y, text, NULL);
						nvgFillColor(args.vg,MSP_panelTextColor);
					}
				}

				// write last harmony note played 2
				if (module->theModeScaleProgressionsState.theHarmonyParms.last[1].note!=0)
				{
					if ((module->theModeScaleProgressionsState.theHarmonyParms.last[1].note>=0)&&(module->theModeScaleProgressionsState.theHarmonyParms.last[1].note<128))
					{
						pos=Vec(notesPlayingDisplayStartX+20+(1*37.5), notesPlayingDisplayNoteCenterY);  
						snprintf(text, sizeof(text), "%s%d",module->note_desig[(module->theModeScaleProgressionsState.theHarmonyParms.last[1].note%12)], module->theModeScaleProgressionsState.theHarmonyParms.last[1].note/12);
						nvgFillColor(args.vg,MSP_panelHarmonyPartColor); 
						nvgText(args.vg, pos.x, pos.y, text, NULL);
						nvgFillColor(args.vg,MSP_panelTextColor);
					}
				}

				// write last harmony note played 3
				if (module->theModeScaleProgressionsState.theHarmonyParms.last[2].note!=0)
				{
					if ((module->theModeScaleProgressionsState.theHarmonyParms.last[2].note>=0)&&(module->theModeScaleProgressionsState.theHarmonyParms.last[2].note<128))
					{
						pos=Vec(notesPlayingDisplayStartX+20+(2*37.5), notesPlayingDisplayNoteCenterY); 
						snprintf(text, sizeof(text), "%s%d", module->note_desig[(module->theModeScaleProgressionsState.theHarmonyParms.last[2].note%12)], module->theModeScaleProgressionsState.theHarmonyParms.last[2].note/12);
						nvgFillColor(args.vg,MSP_panelHarmonyPartColor); 
						nvgText(args.vg, pos.x, pos.y, text, NULL);
						nvgFillColor(args.vg,MSP_panelTextColor);
					}
				}
				// write last harmony note played 4
				if (module->theModeScaleProgressionsState.theHarmonyParms.last[3].note!=0)
				{
					if ((module->theModeScaleProgressionsState.theHarmonyParms.last[3].note>=0)&&(module->theModeScaleProgressionsState.theHarmonyParms.last[3].note<128))
					{
						pos=Vec(notesPlayingDisplayStartX+20+(3*37.5), notesPlayingDisplayNoteCenterY); 
						if ((module->theModeScaleProgressionsState.theHarmonyParms.last_chord_type==2)||(module->theModeScaleProgressionsState.theHarmonyParms.last_chord_type==3)||(module->theModeScaleProgressionsState.theHarmonyParms.last_chord_type==4)||(module->theModeScaleProgressionsState.theHarmonyParms.last_chord_type==5))
							snprintf(text, sizeof(text), "%s%d", module->note_desig[module->theModeScaleProgressionsState.theHarmonyParms.last[3].note%12], module->theModeScaleProgressionsState.theHarmonyParms.last[3].note/12);
						else
							snprintf(text, sizeof(text), "%s", "   ");
						nvgFillColor(args.vg,MSP_panelHarmonyPartColor); 
						nvgText(args.vg, pos.x, pos.y, text, NULL);
						nvgFillColor(args.vg,MSP_panelTextColor);
					}
				}
			}

			int last_chord_root=module->theModeScaleProgressionsState.last_harmony_chord_root_note%12;
			int last_chord_bass_note=module->theModeScaleProgressionsState.theHarmonyParms.last[0].note%12;
			pos=convertSVGtoNVG(107, 61.25, 12.1, 6.5);  // X,Y,W,H in Inkscape mm units
			nvgFontSize(args.vg, 20);
			nvgTextAlign(args.vg,NVG_ALIGN_CENTER|NVG_ALIGN_MIDDLE);

			if (module->valid_current_circle_degree)
		    	nvgFillColor(args.vg,MSP_panelHarmonyPartColor); 
			else
			    nvgFillColor(args.vg,MSP_panelTextColor); 

			char chord_type_desc[16];
			if (module->theModeScaleProgressionsState.theHarmonyParms.last_chord_type==0)
				strcpy(chord_type_desc, "");
			else
			if (module->theModeScaleProgressionsState.theHarmonyParms.last_chord_type==2)  // dom
				strcpy(chord_type_desc, "dom7");
			else
			if (module->theModeScaleProgressionsState.theHarmonyParms.last_chord_type==1)
				strcpy(chord_type_desc, "m");
			else
			if (module->theModeScaleProgressionsState.theHarmonyParms.last_chord_type==3)
				strcpy(chord_type_desc, "7");
			else
			if (module->theModeScaleProgressionsState.theHarmonyParms.last_chord_type==4)
				strcpy(chord_type_desc, "m7");
			else
			if (module->theModeScaleProgressionsState.theHarmonyParms.last_chord_type==5)
				strcpy(chord_type_desc, "dim7");
			else
			if (module->theModeScaleProgressionsState.theHarmonyParms.last_chord_type==6)
				strcpy(chord_type_desc, "dim");

			if (last_chord_bass_note!=last_chord_root) // display inversions
			{
			    if ((module->theModeScaleProgressionsState.theHarmonyParms.last_chord_type==0)||(module->theModeScaleProgressionsState.theHarmonyParms.last_chord_type==2)||(module->theModeScaleProgressionsState.theHarmonyParms.last_chord_type==3))  // major
				{
					if (module->valid_current_circle_degree)
				      snprintf(text, sizeof(text), "%s-%s%s/%s",MSP_circle_of_fifths_arabic_degrees[module->current_circle_degree], module->note_desig[last_chord_root], chord_type_desc, module->note_desig[last_chord_bass_note]);
					else
					  snprintf(text, sizeof(text), "%s%s", module->note_desig[last_chord_root], chord_type_desc);
				}
				else
				if ((module->theModeScaleProgressionsState.theHarmonyParms.last_chord_type==5)||(module->theModeScaleProgressionsState.theHarmonyParms.last_chord_type==6)) // diminished
				{
					if (module->valid_current_circle_degree)
            	       snprintf(text, sizeof(text), "%s'-%s%s/%s",MSP_circle_of_fifths_arabic_degrees_LC[module->current_circle_degree], module->note_desig[last_chord_root], chord_type_desc, module->note_desig[last_chord_bass_note]);
					else
					   snprintf(text, sizeof(text), "%s%s", module->note_desig[last_chord_root], chord_type_desc);
				}
				else  // minor
				{
					if (module->valid_current_circle_degree)
				       snprintf(text, sizeof(text), "%s-%s%s/%s",MSP_circle_of_fifths_arabic_degrees_LC[module->current_circle_degree], module->note_desig[last_chord_root], chord_type_desc, module->note_desig[last_chord_bass_note]);
					else
					    snprintf(text, sizeof(text), "%s%s", module->note_desig[last_chord_root], chord_type_desc);
				}
			}
			else  // do not display inversions as there is none
			{
			    if ((module->theModeScaleProgressionsState.theHarmonyParms.last_chord_type==0)||(module->theModeScaleProgressionsState.theHarmonyParms.last_chord_type==2)||(module->theModeScaleProgressionsState.theHarmonyParms.last_chord_type==3))  // major
				{ 
				   if (module->valid_current_circle_degree)
			         snprintf(text, sizeof(text), "%s-%s%s",MSP_circle_of_fifths_arabic_degrees[module->current_circle_degree], module->note_desig[last_chord_root], chord_type_desc);
				   else
				     snprintf(text, sizeof(text), "%s%s",  module->note_desig[last_chord_root], chord_type_desc);
				}
				else
				if ((module->theModeScaleProgressionsState.theHarmonyParms.last_chord_type==0)||(module->theModeScaleProgressionsState.theHarmonyParms.last_chord_type==5)||(module->theModeScaleProgressionsState.theHarmonyParms.last_chord_type==6))  // diminished
				{
					if (module->valid_current_circle_degree)
			          snprintf(text, sizeof(text), "%s'-%s%s",MSP_circle_of_fifths_arabic_degrees_LC[module->current_circle_degree], module->note_desig[last_chord_root], chord_type_desc);
					else
					  snprintf(text, sizeof(text), "%s%s",  module->note_desig[last_chord_root], chord_type_desc);
				}
				else  // minor
				{
					if (module->valid_current_circle_degree)
				      snprintf(text, sizeof(text), "%s-%s%s",MSP_circle_of_fifths_arabic_degrees_LC[module->current_circle_degree], module->note_desig[last_chord_root], chord_type_desc); 
					else
					  snprintf(text, sizeof(text), "%s%s",  module->note_desig[last_chord_root], chord_type_desc); 
				}
			}
			nvgText(args.vg, pos.x, pos.y, text, NULL);
		

			
			// draw text
			nvgFontSize(args.vg, 15);
			if (textfont)
			nvgFontFaceId(args.vg, textfont->handle);	 
			nvgTextLetterSpacing(args.vg, -1); // as close as possible
			nvgFillColor(args.vg,MSP_panelHarmonyPartColor); 
				
			if (true)  // update scale keys
			{ 
				drawScaleKeyboard(args); // redraw full keyboard per frame. clears any key down states
				int octave=4;
				int semitoneOffset=0;
			    int keyboard_offset=12.0; // adjust note range to middle C on piano keyboard
				int colorIndex=1;  // gray
			
			    
				if ((strstr(module->MSPScaleText,"C# "))||(strstr(module->MSPScaleText,"Db ")))
				{
					semitoneOffset=1;
					if (module->root_key>semitoneOffset)  
					    drawScaleKey(args, ((octave+1)*12)+semitoneOffset+keyboard_offset, true, colorIndex); 
					else
				    	drawScaleKey(args, (octave*12)+semitoneOffset+keyboard_offset, true, colorIndex);  
				}
		
				if (strstr(module->MSPScaleText,"C "))
				{
					semitoneOffset=0;
					if (module->root_key>semitoneOffset)  
					   drawScaleKey(args, ((octave+1)*12)+semitoneOffset+keyboard_offset, true, colorIndex); 
					else
				       drawScaleKey(args, (octave*12)+semitoneOffset+keyboard_offset, true, colorIndex); 
				}
			
				if ((strstr(module->MSPScaleText,"D# "))||(strstr(module->MSPScaleText,"Eb ")))
				{
					semitoneOffset=3;
					if (module->root_key>semitoneOffset)  
					   drawScaleKey(args, ((octave+1)*12)+semitoneOffset+keyboard_offset, true, colorIndex); 
					else
					   drawScaleKey(args, (octave*12)+semitoneOffset+keyboard_offset, true, colorIndex);  
				}
			
				if (strstr(module->MSPScaleText,"D "))
				{
					semitoneOffset=2;
					if (module->root_key>semitoneOffset)  
					   drawScaleKey(args, ((octave+1)*12)+semitoneOffset+keyboard_offset, true, colorIndex); 
					else
					   drawScaleKey(args, (octave*12)+semitoneOffset+keyboard_offset, true, colorIndex);  
				}
			
				if (strstr(module->MSPScaleText,"E "))
				{
					semitoneOffset=4;
					if (module->root_key>semitoneOffset)  
					   drawScaleKey(args, ((octave+1)*12)+semitoneOffset+keyboard_offset, true, colorIndex); 
					else
					  drawScaleKey(args, (octave*12)+semitoneOffset+keyboard_offset, true, colorIndex);  
				}
			
				if ((strstr(module->MSPScaleText,"F# "))||(strstr(module->MSPScaleText,"Gb ")))
				{
					semitoneOffset=6;
					if (module->root_key>semitoneOffset)  
					   drawScaleKey(args, ((octave+1)*12)+semitoneOffset+keyboard_offset, true, colorIndex); 
					else
				  	drawScaleKey(args, (octave*12)+semitoneOffset+keyboard_offset, true, colorIndex);  
				}
			
				if (strstr(module->MSPScaleText,"F "))
				{
					semitoneOffset=5;
					if (module->root_key>semitoneOffset)  
					   drawScaleKey(args, ((octave+1)*12)+semitoneOffset+keyboard_offset, true, colorIndex); 
					else
					  drawScaleKey(args, (octave*12)+semitoneOffset+keyboard_offset, true, colorIndex);  
				}
			
				if ((strstr(module->MSPScaleText,"G# "))||(strstr(module->MSPScaleText,"Ab ")))
				{
					semitoneOffset=8;
					if (module->root_key>semitoneOffset)  
					   drawScaleKey(args, ((octave+1)*12)+semitoneOffset+keyboard_offset, true, colorIndex); 
					else
					   drawScaleKey(args, (octave*12)+semitoneOffset+keyboard_offset, true, colorIndex);  
				}
			
				if (strstr(module->MSPScaleText,"G "))
				{
					semitoneOffset=7;
					if (module->root_key>semitoneOffset)  
					   drawScaleKey(args, ((octave+1)*12)+semitoneOffset+keyboard_offset, true, colorIndex); 
					else
					   drawScaleKey(args, (octave*12)+semitoneOffset+keyboard_offset, true, colorIndex);  
				}
			 
				if ((strstr(module->MSPScaleText,"A# "))||(strstr(module->MSPScaleText,"Bb ")))
				{
					semitoneOffset=10;
					if (module->root_key>semitoneOffset)  
					   drawScaleKey(args, ((octave+1)*12)+semitoneOffset+keyboard_offset, true, colorIndex); 
					else
					   drawScaleKey(args, (octave*12)+semitoneOffset+keyboard_offset, true, colorIndex);  
				}
			
				if (strstr(module->MSPScaleText,"A "))
				{
					semitoneOffset=9;
					if (module->root_key>semitoneOffset)  
					   drawScaleKey(args, ((octave+1)*12)+semitoneOffset+keyboard_offset, true, colorIndex); 
					else
					   drawScaleKey(args, (octave*12)+semitoneOffset+keyboard_offset, true, colorIndex);  
				}
			
				if (strstr(module->MSPScaleText,"B "))
				{
					semitoneOffset=11;
					if (module->root_key>semitoneOffset)  
					   drawScaleKey(args, ((octave+1)*12)+semitoneOffset+keyboard_offset, true, colorIndex); 
					else
					   drawScaleKey(args, (octave*12)+semitoneOffset+keyboard_offset, true, colorIndex);  
				}
			}
					
		}  // end UpdatePanel()

	   
		double smoothedDt=.016;  // start out at 1/60
		int numZeroes=0;
		
		void draw(const DrawArgs &args) override   
		{   
		 	if (!module)  // if there is no module, draw the static panel image, i.e., in the browser
			{
				nvgBeginPath(args.vg);
				nvgRect(args.vg, 0.0, 0.0, box.size.x, box.size.y);
				
				if (MSP_panelTheme==0)  // light theme
				{
					std::shared_ptr<Image> lightPanelImage = APP->window->loadImage(asset::plugin(pluginInstance,"res/ModeScaleProgressions-light.png"));
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
					std::shared_ptr<Image> darkPanelImage = APP->window->loadImage(asset::plugin(pluginInstance,"res/ModeScaleProgressions-dark.png"));
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
				         							
				// draw ModeScaleProgressions logo and chord legend
				
				std::shared_ptr<Font> textfont = APP->window->loadFont(asset::plugin(pluginInstance, "res/Ubuntu Condensed 400.ttf"));
							
				if (textfont)
				{
					nvgBeginPath(args.vg);
			   	//	nvgFontSize(args.vg, 27);
					nvgFontSize(args.vg, 20);
					nvgFontFaceId(args.vg, textfont->handle);
					nvgTextLetterSpacing(args.vg, -1);
					nvgFillColor(args.vg,MSP_panelTextColor);
					nvgTextAlign(args.vg,NVG_ALIGN_CENTER|NVG_ALIGN_MIDDLE);
					
					char text[128];
					snprintf(text, sizeof(text), "%s", "PS-PurrSoftware  ModeScaleProgressions");  
					Vec pos=Vec(245, 15); 
					nvgStrokeWidth(args.vg, 3.0);
					nvgText(args.vg, pos.x, pos.y, text, NULL);

					snprintf(text, sizeof(text), "%s", "Mode Scale Notes");
					nvgFontSize(args.vg, 11);
					pos=Vec(275, 340); 
					nvgStrokeWidth(args.vg, 3.0);
					nvgText(args.vg, pos.x, pos.y, text, NULL);
				

					snprintf(text, sizeof(text), "%s", "Harmonic Progression Diatonic Circle of 5ths");  
					nvgFontSize(args.vg, 15);
					pos=Vec(345, 35);  
					nvgStrokeWidth(args.vg, 2.0);
					nvgText(args.vg, pos.x, pos.y, text, NULL);

					nvgClosePath(args.vg);

					nvgStrokeWidth(args.vg, 1.0);
					nvgStrokeColor(args.vg,MSP_panelLineColor);

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
					nvgFillColor(args.vg,MSP_panelTextColor);
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
					nvgFillColor(args.vg,MSP_panelTextColor);
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
					nvgFillColor(args.vg,MSP_panelTextColor);
					nvgFontSize(args.vg, 10);
					pos=Vec(404, 50);  
					nvgText(args.vg, pos.x, pos.y, text, NULL);
					nvgClosePath(args.vg);

					// Time display on panel
					if (module)  
					{
						if (module->running)
						{
							pos=Vec(485, 315); 
							NVGcolor backgroundColor = nvgRGB(0x20, 0x10, 0x10);
														
							nvgBeginPath(args.vg);
							nvgRoundedRect(args.vg, pos.x,pos.y,195,20, 4.0);
							nvgFillColor(args.vg, backgroundColor);
							nvgFill(args.vg);
							nvgStrokeWidth(args.vg, 2.5);
							nvgStrokeColor(args.vg,nvgRGBA( 0x80,  0x80 , 0x80, 0x80));
							nvgStroke(args.vg);

							nvgFontSize(args.vg, 17);
							pos=Vec(486, 325); 
							nvgFillColor(args.vg,MSP_paramTextColor);
						
							double current_cpu_time_double= (double)(clock()) / (double)CLOCKS_PER_SEC;
							double elapsed_cpu_time= current_cpu_time_double- module->run_start_cpu_time_double;

							if (module->time_sig_bottom==2)
								snprintf(text, sizeof(text), "Bars:: %05d:%02d  Minutes:: %04d:%02d", module->bar_count+1, (module->barts_count/module->i2ts_count_limit)+1, (int)((float)elapsed_cpu_time/60.0), (int)(fmod((float)elapsed_cpu_time,60.0)));
							else 
							if (module->time_sig_bottom==4)
								snprintf(text, sizeof(text), "Bars:: %05d:%02d  Minutes:: %04d:%02d", module->bar_count+1, (module->barts_count/module->i4ts_count_limit)+1, (int)((float)elapsed_cpu_time/60.0), (int)(fmod((float)elapsed_cpu_time,60.0))); 
							else 
							if (module->time_sig_bottom==8)
								snprintf(text, sizeof(text), "Bars:: %05d:%02d  Minutes:: %04d:%02d", module->bar_count+1, (module->barts_count/module->i8ts_count_limit)+1, (int)((float)elapsed_cpu_time/60.0), (int)(fmod((float)elapsed_cpu_time,60.0)));
							else  
							if (module->time_sig_bottom==16)
								snprintf(text, sizeof(text), "Bars:: %05d:%02d  Minutes:: %04d:%02d", module->bar_count+1, (module->barts_count/module->i16ts_count_limit)+1, (int)((float)elapsed_cpu_time/60.0), (int)(fmod((float)elapsed_cpu_time,60.0))); 
						
							nvgText(args.vg, pos.x, pos.y, text, NULL);
						}
					}
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

	ModeScaleProgressionsWidget(ModeScaleProgressions* module)   // all plugins I've looked at use this constructor with module*, even though docs show it deprecated.         
	{ 
		setModule(module);  // most plugins do this
		this->module = module;  //  most plugins do not do this.  It was introduced in singleton implementation
			
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ModeScaleProgressions-light.svg"))); 
		svgPanel=(SvgPanel*)getPanel();
		svgPanel->setVisible((MSP_panelTheme) == 0);  
				
		MSP_panelcolor=nvgRGBA((unsigned char)230,(unsigned char)230,(unsigned char)230,(unsigned char)255);
		
		darkPanel = new SvgPanel();
		darkPanel->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ModeScaleProgressions-dark.svg")));
		darkPanel->setVisible((MSP_panelTheme) == 1);  
		addChild(darkPanel);
							
		rack::random::init();  // must be called per thread

		 if (true)   // must be executed even if module* is null. module* is checked for null below where accessed as it is null in browser preview
		 {
			// create param widgets  Needs to be done even if module is null.
					
			float Quantizer_row=305.0;
			
			ModeScaleProgressionsRootKeySelectLineDisplay *ModeScaleProgressionsRootKeySelectDisplay = new ModeScaleProgressionsRootKeySelectLineDisplay();
			ModeScaleProgressionsRootKeySelectDisplay->box.pos = Vec(115.,175.);  
			ModeScaleProgressionsRootKeySelectDisplay->box.size = Vec(40, 22); 
			ModeScaleProgressionsRootKeySelectDisplay->module=module;
			if (module) 
				ModeScaleProgressionsRootKeySelectDisplay->val = &module->root_key;
			else
			{ 
				ModeScaleProgressionsRootKeySelectDisplay->val = &dummyindex;  // strictly for browser visibility
			}
			addChild(ModeScaleProgressionsRootKeySelectDisplay);

			ModeScaleProgressionsScaleSelectLineDisplay *ModeScaleProgressionsScaleSelectDisplay = new ModeScaleProgressionsScaleSelectLineDisplay();
			ModeScaleProgressionsScaleSelectDisplay->box.pos = Vec(1.,218.); 
			ModeScaleProgressionsScaleSelectDisplay->box.size = Vec(130, 22); 
			ModeScaleProgressionsScaleSelectDisplay->module=module;
			if (module) 
				ModeScaleProgressionsScaleSelectDisplay->val = &module->mode;
			else
			{ 
				ModeScaleProgressionsScaleSelectDisplay->val = &dummyindex;  // strictly for browser visibility
			}
			
			addChild(ModeScaleProgressionsScaleSelectDisplay);

			CircleOf5thsDisplay *display = new CircleOf5thsDisplay(module);
			display->ParameterRectLocal=ParameterRect;
			display->InportRectLocal=InportRect;  
			display->OutportRectLocal=OutportRect;  
			display->box.pos = Vec(0, 0);
			display->box.size = Vec(box.size.x, box.size.y);
			addChild(display);
				
			//BPM DISPLAY  
			ModeScaleProgressionsBpmDisplayWidget *BPMdisplay = new ModeScaleProgressionsBpmDisplayWidget();
			BPMdisplay->box.pos = Vec(80,90);
			BPMdisplay->box.size = Vec(70, 35);
			BPMdisplay->module=module;
			
			if (module) 
				BPMdisplay->val = &module->tempo;    
			else
			{ 
				BPMdisplay->val = &dummytempo;  // strictly for browser visibility
			}
			addChild(BPMdisplay); 
			
			//SIG TOP DISPLAY 
			ModeScaleProgressionsSigDisplayWidget *SigTopDisplay = new ModeScaleProgressionsSigDisplayWidget();
			SigTopDisplay->box.pos = Vec(130,130);
			SigTopDisplay->box.size = Vec(25, 20);
			if (module) 
				SigTopDisplay->value = &module->time_sig_top;
			else
			{
				SigTopDisplay->value = &dummysig;  // strictly for browser visibility
			}
			addChild(SigTopDisplay);
					
			//SIG BOTTOM DISPLAY    
			ModeScaleProgressionsSigDisplayWidget *SigBottomDisplay = new ModeScaleProgressionsSigDisplayWidget();
			SigBottomDisplay->box.pos = Vec(130,150);
			SigBottomDisplay->box.size = Vec(25, 20);
			if (module) 
				SigBottomDisplay->value = &module->time_sig_bottom;
			else
			{
				SigBottomDisplay->value = &dummysig;  // strictly for browser visibility
			}
			addChild(SigBottomDisplay);
		
			// create params, controls, lights and ports.  Needs to be done even if module* is null.
			
			//*************   Note: Each LEDButton needs its light and that light needs a unique ID, needs to be added to an array and then needs to be repositioned along with the button.  Also needs to be enumed with other lights so lights[] picks it up.
			paramWidgets[ModeScaleProgressions::BUTTON_CIRCLESTEP_C_PARAM]=createParamCentered<LEDButton>(mm2px(Vec(116.227, 37.257)), module, ModeScaleProgressions::BUTTON_CIRCLESTEP_C_PARAM);
			addParam(paramWidgets[ModeScaleProgressions::BUTTON_CIRCLESTEP_C_PARAM]);
			lightWidgets[ModeScaleProgressions::LIGHT_LEDBUTTON_CIRCLESTEP_1]=createLightCentered<MediumSimpleLight<GreenLight>>(mm2px(Vec(116.227, 37.257)), module, ModeScaleProgressions::LIGHT_LEDBUTTON_CIRCLESTEP_1);
			addChild(lightWidgets[ModeScaleProgressions::LIGHT_LEDBUTTON_CIRCLESTEP_1]);
		
			paramWidgets[ModeScaleProgressions::BUTTON_CIRCLESTEP_G_PARAM]=createParamCentered<LEDButton>(mm2px(Vec(132.479, 41.32)), module, ModeScaleProgressions::BUTTON_CIRCLESTEP_G_PARAM);
			addParam(paramWidgets[ModeScaleProgressions::BUTTON_CIRCLESTEP_G_PARAM]);
			lightWidgets[ModeScaleProgressions::LIGHT_LEDBUTTON_CIRCLESTEP_2]=createLightCentered<MediumSimpleLight<GreenLight>>(mm2px(Vec(132.479, 41.32)), module, ModeScaleProgressions::LIGHT_LEDBUTTON_CIRCLESTEP_2);
			addChild(lightWidgets[ModeScaleProgressions::LIGHT_LEDBUTTON_CIRCLESTEP_2]);
		
			paramWidgets[ModeScaleProgressions::BUTTON_CIRCLESTEP_D_PARAM]=createParamCentered<LEDButton>(mm2px(Vec(143.163, 52.155)), module, ModeScaleProgressions::BUTTON_CIRCLESTEP_D_PARAM);
			addParam(paramWidgets[ModeScaleProgressions::BUTTON_CIRCLESTEP_D_PARAM]);
			lightWidgets[ModeScaleProgressions::LIGHT_LEDBUTTON_CIRCLESTEP_3]=createLightCentered<MediumSimpleLight<GreenLight>>(mm2px(Vec(143.163, 52.155)), module, ModeScaleProgressions::LIGHT_LEDBUTTON_CIRCLESTEP_3);
			addChild(lightWidgets[ModeScaleProgressions::LIGHT_LEDBUTTON_CIRCLESTEP_3]);
		
			paramWidgets[ModeScaleProgressions::BUTTON_CIRCLESTEP_A_PARAM]=createParamCentered<LEDButton>(mm2px(Vec(147.527, 67.353)), module, ModeScaleProgressions::BUTTON_CIRCLESTEP_A_PARAM);
			addParam(paramWidgets[ModeScaleProgressions::BUTTON_CIRCLESTEP_A_PARAM]);
			lightWidgets[ModeScaleProgressions::LIGHT_LEDBUTTON_CIRCLESTEP_4]=createLightCentered<MediumSimpleLight<GreenLight>>(mm2px(Vec(147.527, 67.353)), module, ModeScaleProgressions::LIGHT_LEDBUTTON_CIRCLESTEP_4);
			addChild(lightWidgets[ModeScaleProgressions::LIGHT_LEDBUTTON_CIRCLESTEP_4]);
		
			paramWidgets[ModeScaleProgressions::BUTTON_CIRCLESTEP_E_PARAM]=createParamCentered<LEDButton>(mm2px(Vec(141.96, 83.906)), module, ModeScaleProgressions::BUTTON_CIRCLESTEP_E_PARAM);
			addParam(paramWidgets[ModeScaleProgressions::BUTTON_CIRCLESTEP_E_PARAM]);
			lightWidgets[ModeScaleProgressions::LIGHT_LEDBUTTON_CIRCLESTEP_5]=createLightCentered<MediumSimpleLight<GreenLight>>(mm2px(Vec(141.96, 83.906)), module, ModeScaleProgressions::LIGHT_LEDBUTTON_CIRCLESTEP_5);
			addChild(lightWidgets[ModeScaleProgressions::LIGHT_LEDBUTTON_CIRCLESTEP_5]);
		
			paramWidgets[ModeScaleProgressions::BUTTON_CIRCLESTEP_B_PARAM]=createParamCentered<LEDButton>(mm2px(Vec(132.931, 94.44)), module, ModeScaleProgressions::BUTTON_CIRCLESTEP_B_PARAM);
			addParam(paramWidgets[ModeScaleProgressions::BUTTON_CIRCLESTEP_B_PARAM]);
			lightWidgets[ModeScaleProgressions::LIGHT_LEDBUTTON_CIRCLESTEP_6]=createLightCentered<MediumSimpleLight<GreenLight>>(mm2px(Vec(132.931, 94.44)), module, ModeScaleProgressions::LIGHT_LEDBUTTON_CIRCLESTEP_6);
			addChild(lightWidgets[ModeScaleProgressions::LIGHT_LEDBUTTON_CIRCLESTEP_6]);
		
			paramWidgets[ModeScaleProgressions::BUTTON_CIRCLESTEP_GBFS_PARAM]=createParamCentered<LEDButton>(mm2px(Vec(116.378, 98.804)), module, ModeScaleProgressions::BUTTON_CIRCLESTEP_GBFS_PARAM);
			addParam(paramWidgets[ModeScaleProgressions::BUTTON_CIRCLESTEP_GBFS_PARAM]);
			lightWidgets[ModeScaleProgressions::LIGHT_LEDBUTTON_CIRCLESTEP_7]=createLightCentered<MediumSimpleLight<GreenLight>>(mm2px(Vec(116.378, 98.804)), module, ModeScaleProgressions::LIGHT_LEDBUTTON_CIRCLESTEP_7);
			addChild(lightWidgets[ModeScaleProgressions::LIGHT_LEDBUTTON_CIRCLESTEP_7]);
		
			paramWidgets[ModeScaleProgressions::BUTTON_CIRCLESTEP_DB_PARAM]=createParamCentered<LEDButton>(mm2px(Vec(101.029, 93.988)), module, ModeScaleProgressions::BUTTON_CIRCLESTEP_DB_PARAM);
			addParam(paramWidgets[ModeScaleProgressions::BUTTON_CIRCLESTEP_DB_PARAM]);
			lightWidgets[ModeScaleProgressions::LIGHT_LEDBUTTON_CIRCLESTEP_8]=createLightCentered<MediumSimpleLight<GreenLight>>(mm2px(Vec(101.029, 93.988)), module, ModeScaleProgressions::LIGHT_LEDBUTTON_CIRCLESTEP_8);
			addChild(lightWidgets[ModeScaleProgressions::LIGHT_LEDBUTTON_CIRCLESTEP_8]);
		
			paramWidgets[ModeScaleProgressions::BUTTON_CIRCLESTEP_AB_PARAM]=createParamCentered<LEDButton>(mm2px(Vec(91.097, 83.906)), module, ModeScaleProgressions::BUTTON_CIRCLESTEP_AB_PARAM);
			addParam(paramWidgets[ModeScaleProgressions::BUTTON_CIRCLESTEP_AB_PARAM]);
			lightWidgets[ModeScaleProgressions::LIGHT_LEDBUTTON_CIRCLESTEP_9]=createLightCentered<MediumSimpleLight<GreenLight>>(mm2px(Vec(91.097, 83.906)), module, ModeScaleProgressions::LIGHT_LEDBUTTON_CIRCLESTEP_9);
			addChild(lightWidgets[ModeScaleProgressions::LIGHT_LEDBUTTON_CIRCLESTEP_9]);
		
			paramWidgets[ModeScaleProgressions::BUTTON_CIRCLESTEP_EB_PARAM]=createParamCentered<LEDButton>(mm2px(Vec(86.282, 68.106)), module, ModeScaleProgressions::BUTTON_CIRCLESTEP_EB_PARAM);
			addParam(paramWidgets[ModeScaleProgressions::BUTTON_CIRCLESTEP_EB_PARAM]);
			lightWidgets[ModeScaleProgressions::LIGHT_LEDBUTTON_CIRCLESTEP_10]=createLightCentered<MediumSimpleLight<GreenLight>>(mm2px(Vec(86.282, 68.106)), module, ModeScaleProgressions::LIGHT_LEDBUTTON_CIRCLESTEP_10);
			addChild(lightWidgets[ModeScaleProgressions::LIGHT_LEDBUTTON_CIRCLESTEP_10]);
		
			paramWidgets[ModeScaleProgressions::BUTTON_CIRCLESTEP_BB_PARAM]=createParamCentered<LEDButton>(mm2px(Vec(89.743, 52.004)), module, ModeScaleProgressions::BUTTON_CIRCLESTEP_BB_PARAM);
			addParam(paramWidgets[ModeScaleProgressions::BUTTON_CIRCLESTEP_BB_PARAM]);
			lightWidgets[ModeScaleProgressions::LIGHT_LEDBUTTON_CIRCLESTEP_11]=createLightCentered<MediumSimpleLight<GreenLight>>(mm2px(Vec(189.743, 52.004)), module, ModeScaleProgressions::LIGHT_LEDBUTTON_CIRCLESTEP_11);
			addChild(lightWidgets[ModeScaleProgressions::LIGHT_LEDBUTTON_CIRCLESTEP_11]);
		
			paramWidgets[ModeScaleProgressions::BUTTON_CIRCLESTEP_F_PARAM]=createParamCentered<LEDButton>(mm2px(Vec(101.781, 40.568)), module, ModeScaleProgressions::BUTTON_CIRCLESTEP_F_PARAM);
			addParam(paramWidgets[ModeScaleProgressions::BUTTON_CIRCLESTEP_F_PARAM]);
			lightWidgets[ModeScaleProgressions::LIGHT_LEDBUTTON_CIRCLESTEP_12]=createLightCentered<MediumSimpleLight<GreenLight>>(mm2px(Vec(101.781, 40.568)), module, ModeScaleProgressions::LIGHT_LEDBUTTON_CIRCLESTEP_12);
			addChild(lightWidgets[ModeScaleProgressions::LIGHT_LEDBUTTON_CIRCLESTEP_12]);
		
	//*************
			lightWidgets[ModeScaleProgressions::LIGHT_CIRCLE_ROOT_KEY_POSITION_1_LIGHT]=createLightCentered<MediumLight<RedLight>>(mm2px(Vec(116.227, 43.878)), module, ModeScaleProgressions::LIGHT_CIRCLE_ROOT_KEY_POSITION_1_LIGHT);
			addChild(lightWidgets[ModeScaleProgressions::LIGHT_CIRCLE_ROOT_KEY_POSITION_1_LIGHT]);
		
			lightWidgets[ModeScaleProgressions::LIGHT_CIRCLE_ROOT_KEY_POSITION_2_LIGHT]=createLightCentered<MediumLight<RedLight>>(mm2px(Vec(129.018, 47.189)), module, ModeScaleProgressions::LIGHT_CIRCLE_ROOT_KEY_POSITION_2_LIGHT);
			addChild(lightWidgets[ModeScaleProgressions::LIGHT_CIRCLE_ROOT_KEY_POSITION_2_LIGHT]);
		
			lightWidgets[ModeScaleProgressions::LIGHT_CIRCLE_ROOT_KEY_POSITION_3_LIGHT]=createLightCentered<MediumLight<RedLight>>(mm2px(Vec(137.295, 56.067)), module, ModeScaleProgressions::LIGHT_CIRCLE_ROOT_KEY_POSITION_3_LIGHT);
			addChild(lightWidgets[ModeScaleProgressions::LIGHT_CIRCLE_ROOT_KEY_POSITION_3_LIGHT]);
		
			lightWidgets[ModeScaleProgressions::LIGHT_CIRCLE_ROOT_KEY_POSITION_4_LIGHT]=createLightCentered<MediumLight<RedLight>>(mm2px(Vec(140.455, 67.654)), module, ModeScaleProgressions::LIGHT_CIRCLE_ROOT_KEY_POSITION_4_LIGHT);
			addChild(lightWidgets[ModeScaleProgressions::LIGHT_CIRCLE_ROOT_KEY_POSITION_4_LIGHT]);
		
			lightWidgets[ModeScaleProgressions::LIGHT_CIRCLE_ROOT_KEY_POSITION_5_LIGHT]=createLightCentered<MediumLight<RedLight>>(mm2px(Vec(137.144, 80.295)), module, ModeScaleProgressions::LIGHT_CIRCLE_ROOT_KEY_POSITION_5_LIGHT);
			addChild(lightWidgets[ModeScaleProgressions::LIGHT_CIRCLE_ROOT_KEY_POSITION_5_LIGHT]);
		
			lightWidgets[ModeScaleProgressions::LIGHT_CIRCLE_ROOT_KEY_POSITION_6_LIGHT]=createLightCentered<MediumLight<RedLight>>(mm2px(Vec(128.868, 88.571)), module, ModeScaleProgressions::LIGHT_CIRCLE_ROOT_KEY_POSITION_6_LIGHT);
			addChild(lightWidgets[ModeScaleProgressions::LIGHT_CIRCLE_ROOT_KEY_POSITION_6_LIGHT]);
		
			lightWidgets[ModeScaleProgressions::LIGHT_CIRCLE_ROOT_KEY_POSITION_7_LIGHT]=createLightCentered<MediumLight<RedLight>>(mm2px(Vec(116.077, 92.183)), module, ModeScaleProgressions::LIGHT_CIRCLE_ROOT_KEY_POSITION_7_LIGHT);
			addChild(lightWidgets[ModeScaleProgressions::LIGHT_CIRCLE_ROOT_KEY_POSITION_7_LIGHT]);
		
			lightWidgets[ModeScaleProgressions::LIGHT_CIRCLE_ROOT_KEY_POSITION_8_LIGHT]=createLightCentered<MediumLight<RedLight>>(mm2px(Vec(104.791, 88.872)), module, ModeScaleProgressions::LIGHT_CIRCLE_ROOT_KEY_POSITION_8_LIGHT);
			addChild(lightWidgets[ModeScaleProgressions::LIGHT_CIRCLE_ROOT_KEY_POSITION_8_LIGHT]);
		
			lightWidgets[ModeScaleProgressions::LIGHT_CIRCLE_ROOT_KEY_POSITION_9_LIGHT]=createLightCentered<MediumLight<RedLight>>(mm2px(Vec(96.213, 80.596)), module, ModeScaleProgressions::LIGHT_CIRCLE_ROOT_KEY_POSITION_9_LIGHT);
			addChild(lightWidgets[ModeScaleProgressions::LIGHT_CIRCLE_ROOT_KEY_POSITION_9_LIGHT]);
		
			lightWidgets[ModeScaleProgressions::LIGHT_CIRCLE_ROOT_KEY_POSITION_10_LIGHT]=createLightCentered<MediumLight<RedLight>>(mm2px(Vec(92.602, 67.654)), module, ModeScaleProgressions::LIGHT_CIRCLE_ROOT_KEY_POSITION_10_LIGHT);
			addChild(lightWidgets[ModeScaleProgressions::LIGHT_CIRCLE_ROOT_KEY_POSITION_10_LIGHT]);
		
			lightWidgets[ModeScaleProgressions::LIGHT_CIRCLE_ROOT_KEY_POSITION_11_LIGHT]=createLightCentered<MediumLight<RedLight>>(mm2px(Vec(95.912, 55.465)), module, ModeScaleProgressions::LIGHT_CIRCLE_ROOT_KEY_POSITION_11_LIGHT);
			addChild(lightWidgets[ModeScaleProgressions::LIGHT_CIRCLE_ROOT_KEY_POSITION_11_LIGHT]);
		
			lightWidgets[ModeScaleProgressions::LIGHT_CIRCLE_ROOT_KEY_POSITION_12_LIGHT]=createLightCentered<MediumLight<RedLight>>(mm2px(Vec(105.393, 46.587)), module, ModeScaleProgressions::LIGHT_CIRCLE_ROOT_KEY_POSITION_12_LIGHT);
			addChild(lightWidgets[ModeScaleProgressions::LIGHT_CIRCLE_ROOT_KEY_POSITION_12_LIGHT]);
				 		

	//*********** Harmony step set selection    

					
			paramWidgets[ModeScaleProgressions::BUTTON_HARMONY_SETSTEP_1_PARAM]=createParamCentered<LEDButton>(mm2px(Vec(65.197, 106.483)), module, ModeScaleProgressions::BUTTON_HARMONY_SETSTEP_1_PARAM);
			addParam(paramWidgets[ModeScaleProgressions::BUTTON_HARMONY_SETSTEP_1_PARAM]);
			lightWidgets[ModeScaleProgressions::LIGHT_LEDBUTTON_CIRCLESETSTEP_1]=createLightCentered<MediumSimpleLight<GreenLight>>(mm2px(Vec(65.197, 106.483)), module, ModeScaleProgressions::LIGHT_LEDBUTTON_CIRCLESETSTEP_1);
			addChild(lightWidgets[ModeScaleProgressions::LIGHT_LEDBUTTON_CIRCLESETSTEP_1]);

			
			paramWidgets[ModeScaleProgressions::BUTTON_HARMONY_SETSTEP_2_PARAM]=createParamCentered<LEDButton>(mm2px(Vec(64.918, 98.02)), module, ModeScaleProgressions::BUTTON_HARMONY_SETSTEP_2_PARAM);
			addParam(paramWidgets[ModeScaleProgressions::BUTTON_HARMONY_SETSTEP_2_PARAM]);
			lightWidgets[ModeScaleProgressions::LIGHT_LEDBUTTON_CIRCLESETSTEP_2]=createLightCentered<MediumSimpleLight<GreenLight>>(mm2px(Vec(64.918, 98.02)), module, ModeScaleProgressions::LIGHT_LEDBUTTON_CIRCLESETSTEP_2);
			addChild(lightWidgets[ModeScaleProgressions::LIGHT_LEDBUTTON_CIRCLESETSTEP_2]);

		
			paramWidgets[ModeScaleProgressions::BUTTON_HARMONY_SETSTEP_3_PARAM]=createParamCentered<LEDButton>(mm2px(Vec(65.193, 89.271)), module, ModeScaleProgressions::BUTTON_HARMONY_SETSTEP_3_PARAM);
			addParam(paramWidgets[ModeScaleProgressions::BUTTON_HARMONY_SETSTEP_3_PARAM]);
			lightWidgets[ModeScaleProgressions::LIGHT_LEDBUTTON_CIRCLESETSTEP_3]=createLightCentered<MediumSimpleLight<GreenLight>>(mm2px(Vec(65.193, 89.271)), module, ModeScaleProgressions::LIGHT_LEDBUTTON_CIRCLESETSTEP_3);
			addChild(lightWidgets[ModeScaleProgressions::LIGHT_LEDBUTTON_CIRCLESETSTEP_3]);

			
			paramWidgets[ModeScaleProgressions::BUTTON_HARMONY_SETSTEP_4_PARAM]=createParamCentered<LEDButton>(mm2px(Vec(64.918, 81.9233)), module, ModeScaleProgressions::BUTTON_HARMONY_SETSTEP_4_PARAM);
			addParam(paramWidgets[ModeScaleProgressions::BUTTON_HARMONY_SETSTEP_4_PARAM]);
			lightWidgets[ModeScaleProgressions::LIGHT_LEDBUTTON_CIRCLESETSTEP_4]=createLightCentered<MediumSimpleLight<GreenLight>>(mm2px(Vec(64.918, 81.923)), module, ModeScaleProgressions::LIGHT_LEDBUTTON_CIRCLESETSTEP_4);
			addChild(lightWidgets[ModeScaleProgressions::LIGHT_LEDBUTTON_CIRCLESETSTEP_4]);

		
			paramWidgets[ModeScaleProgressions::BUTTON_HARMONY_SETSTEP_5_PARAM]=createParamCentered<LEDButton>(mm2px(Vec(65.193, 73.184)), module, ModeScaleProgressions::BUTTON_HARMONY_SETSTEP_5_PARAM);
			addParam(paramWidgets[ModeScaleProgressions::BUTTON_HARMONY_SETSTEP_5_PARAM]);
			lightWidgets[ModeScaleProgressions::LIGHT_LEDBUTTON_CIRCLESETSTEP_5]=createLightCentered<MediumSimpleLight<GreenLight>>(mm2px(Vec(65.193, 73.184)), module, ModeScaleProgressions::LIGHT_LEDBUTTON_CIRCLESETSTEP_5);
			addChild(lightWidgets[ModeScaleProgressions::LIGHT_LEDBUTTON_CIRCLESETSTEP_5]);

			
			paramWidgets[ModeScaleProgressions::BUTTON_HARMONY_SETSTEP_6_PARAM]=createParamCentered<LEDButton>(mm2px(Vec(64.918, 66.129)), module, ModeScaleProgressions::BUTTON_HARMONY_SETSTEP_6_PARAM);
			addParam(paramWidgets[ModeScaleProgressions::BUTTON_HARMONY_SETSTEP_6_PARAM]);
			lightWidgets[ModeScaleProgressions::LIGHT_LEDBUTTON_CIRCLESETSTEP_6]=createLightCentered<MediumSimpleLight<GreenLight>>(mm2px(Vec(64.918, 66.129)), module, ModeScaleProgressions::LIGHT_LEDBUTTON_CIRCLESETSTEP_6);
			addChild(lightWidgets[ModeScaleProgressions::LIGHT_LEDBUTTON_CIRCLESETSTEP_6]);

			
			paramWidgets[ModeScaleProgressions::BUTTON_HARMONY_SETSTEP_7_PARAM]=createParamCentered<LEDButton>(mm2px(Vec(65.193, 57.944)), module, ModeScaleProgressions::BUTTON_HARMONY_SETSTEP_7_PARAM);
			addParam(paramWidgets[ModeScaleProgressions::BUTTON_HARMONY_SETSTEP_7_PARAM]);
			lightWidgets[ModeScaleProgressions::LIGHT_LEDBUTTON_CIRCLESETSTEP_7]=createLightCentered<MediumSimpleLight<GreenLight>>(mm2px(Vec(65.193, 57.944)), module, ModeScaleProgressions::LIGHT_LEDBUTTON_CIRCLESETSTEP_7);
			addChild(lightWidgets[ModeScaleProgressions::LIGHT_LEDBUTTON_CIRCLESETSTEP_7]);

			
			paramWidgets[ModeScaleProgressions::BUTTON_HARMONY_SETSTEP_8_PARAM]=createParamCentered<LEDButton>(mm2px(Vec(64.911, 49.474)), module, ModeScaleProgressions::BUTTON_HARMONY_SETSTEP_8_PARAM);
			addParam(paramWidgets[ModeScaleProgressions::BUTTON_HARMONY_SETSTEP_8_PARAM]);
			lightWidgets[ModeScaleProgressions::LIGHT_LEDBUTTON_CIRCLESETSTEP_8]=createLightCentered<MediumSimpleLight<GreenLight>>(mm2px(Vec(64.911, 49.474)), module, ModeScaleProgressions::LIGHT_LEDBUTTON_CIRCLESETSTEP_8);
			addChild(lightWidgets[ModeScaleProgressions::LIGHT_LEDBUTTON_CIRCLESETSTEP_8]);

			
			paramWidgets[ModeScaleProgressions::BUTTON_HARMONY_SETSTEP_9_PARAM]=createParamCentered<LEDButton>(mm2px(Vec(4.629, 41.011)), module, ModeScaleProgressions::BUTTON_HARMONY_SETSTEP_9_PARAM);
			addParam(paramWidgets[ModeScaleProgressions::BUTTON_HARMONY_SETSTEP_9_PARAM]);
			lightWidgets[ModeScaleProgressions::LIGHT_LEDBUTTON_CIRCLESETSTEP_9]=createLightCentered<MediumSimpleLight<GreenLight>>(mm2px(Vec(4.629, 41.011)), module, ModeScaleProgressions::LIGHT_LEDBUTTON_CIRCLESETSTEP_9);
			addChild(lightWidgets[ModeScaleProgressions::LIGHT_LEDBUTTON_CIRCLESETSTEP_9]);

			
			paramWidgets[ModeScaleProgressions::BUTTON_HARMONY_SETSTEP_10_PARAM]=createParamCentered<LEDButton>(mm2px(Vec(64.629, 32.827)), module, ModeScaleProgressions::BUTTON_HARMONY_SETSTEP_10_PARAM);
			addParam(paramWidgets[ModeScaleProgressions::BUTTON_HARMONY_SETSTEP_10_PARAM]);
			lightWidgets[ModeScaleProgressions::LIGHT_LEDBUTTON_CIRCLESETSTEP_10]=createLightCentered<MediumSimpleLight<GreenLight>>(mm2px(Vec(64.629, 32.827)), module, ModeScaleProgressions::LIGHT_LEDBUTTON_CIRCLESETSTEP_10);
			addChild(lightWidgets[ModeScaleProgressions::LIGHT_LEDBUTTON_CIRCLESETSTEP_10]);

			
			paramWidgets[ModeScaleProgressions::BUTTON_HARMONY_SETSTEP_11_PARAM]=createParamCentered<LEDButton>(mm2px(Vec(64.629, 24.649)), module, ModeScaleProgressions::BUTTON_HARMONY_SETSTEP_11_PARAM);
			addParam(paramWidgets[ModeScaleProgressions::BUTTON_HARMONY_SETSTEP_11_PARAM]);
			lightWidgets[ModeScaleProgressions::LIGHT_LEDBUTTON_CIRCLESETSTEP_11]=createLightCentered<MediumSimpleLight<GreenLight>>(mm2px(Vec(64.629, 24.649)), module, ModeScaleProgressions::LIGHT_LEDBUTTON_CIRCLESETSTEP_11);
			addChild(lightWidgets[ModeScaleProgressions::LIGHT_LEDBUTTON_CIRCLESETSTEP_11]);

			
			paramWidgets[ModeScaleProgressions::BUTTON_HARMONY_SETSTEP_12_PARAM]=createParamCentered<LEDButton>(mm2px(Vec(64.632, 16.176)), module, ModeScaleProgressions::BUTTON_HARMONY_SETSTEP_12_PARAM);
			addParam(paramWidgets[ModeScaleProgressions::BUTTON_HARMONY_SETSTEP_12_PARAM]);
			lightWidgets[ModeScaleProgressions::LIGHT_LEDBUTTON_CIRCLESETSTEP_12]=createLightCentered<MediumSimpleLight<GreenLight>>(mm2px(Vec(64.632, 16.176)), module, ModeScaleProgressions::LIGHT_LEDBUTTON_CIRCLESETSTEP_12);
			addChild(lightWidgets[ModeScaleProgressions::LIGHT_LEDBUTTON_CIRCLESETSTEP_12]);

		
			paramWidgets[ModeScaleProgressions::BUTTON_HARMONY_SETSTEP_13_PARAM]=createParamCentered<LEDButton>(mm2px(Vec(64.632, 16.176)), module, ModeScaleProgressions::BUTTON_HARMONY_SETSTEP_13_PARAM);
			addParam(paramWidgets[ModeScaleProgressions::BUTTON_HARMONY_SETSTEP_13_PARAM]);
			lightWidgets[ModeScaleProgressions::LIGHT_LEDBUTTON_CIRCLESETSTEP_13]=createLightCentered<MediumSimpleLight<GreenLight>>(mm2px(Vec(64.632, 16.176)), module, ModeScaleProgressions::LIGHT_LEDBUTTON_CIRCLESETSTEP_13);
			addChild(lightWidgets[ModeScaleProgressions::LIGHT_LEDBUTTON_CIRCLESETSTEP_13]);

		
			paramWidgets[ModeScaleProgressions::BUTTON_HARMONY_SETSTEP_14_PARAM]=createParamCentered<LEDButton>(mm2px(Vec(64.632, 16.176)), module, ModeScaleProgressions::BUTTON_HARMONY_SETSTEP_14_PARAM);
			addParam(paramWidgets[ModeScaleProgressions::BUTTON_HARMONY_SETSTEP_14_PARAM]);
			lightWidgets[ModeScaleProgressions::LIGHT_LEDBUTTON_CIRCLESETSTEP_14]=createLightCentered<MediumSimpleLight<GreenLight>>(mm2px(Vec(64.632, 16.176)), module, ModeScaleProgressions::LIGHT_LEDBUTTON_CIRCLESETSTEP_14);
			addChild(lightWidgets[ModeScaleProgressions::LIGHT_LEDBUTTON_CIRCLESETSTEP_14]);

			
			paramWidgets[ModeScaleProgressions::BUTTON_HARMONY_SETSTEP_15_PARAM]=createParamCentered<LEDButton>(mm2px(Vec(64.632, 16.176)), module, ModeScaleProgressions::BUTTON_HARMONY_SETSTEP_15_PARAM);
			addParam(paramWidgets[ModeScaleProgressions::BUTTON_HARMONY_SETSTEP_15_PARAM]);
			lightWidgets[ModeScaleProgressions::LIGHT_LEDBUTTON_CIRCLESETSTEP_15]=createLightCentered<MediumSimpleLight<GreenLight>>(mm2px(Vec(64.632, 16.176)), module, ModeScaleProgressions::LIGHT_LEDBUTTON_CIRCLESETSTEP_15);
			addChild(lightWidgets[ModeScaleProgressions::LIGHT_LEDBUTTON_CIRCLESETSTEP_15]);

			
			paramWidgets[ModeScaleProgressions::BUTTON_HARMONY_SETSTEP_16_PARAM]=createParamCentered<LEDButton>(mm2px(Vec(64.632, 16.176)), module, ModeScaleProgressions::BUTTON_HARMONY_SETSTEP_16_PARAM);
			addParam(paramWidgets[ModeScaleProgressions::BUTTON_HARMONY_SETSTEP_16_PARAM]);
			lightWidgets[ModeScaleProgressions::LIGHT_LEDBUTTON_CIRCLESETSTEP_16]=createLightCentered<MediumSimpleLight<GreenLight>>(mm2px(Vec(64.632, 16.176)), module, ModeScaleProgressions::LIGHT_LEDBUTTON_CIRCLESETSTEP_16);
			addChild(lightWidgets[ModeScaleProgressions::LIGHT_LEDBUTTON_CIRCLESETSTEP_16]);

			//**********General************************
			
			paramWidgets[ModeScaleProgressions::BUTTON_RUN_PARAM]=createParamCentered<LEDButton>(mm2px(Vec(19.7, 10.45)), module, ModeScaleProgressions::BUTTON_RUN_PARAM);
			addParam(paramWidgets[ModeScaleProgressions::BUTTON_RUN_PARAM]);
			lightWidgets[ModeScaleProgressions::LIGHT_LEDBUTTON_RUN]=createLightCentered<MediumSimpleLight<RedLight>>(mm2px(Vec(19.7, 10.45)), module, ModeScaleProgressions::LIGHT_LEDBUTTON_RUN);
			addChild(lightWidgets[ModeScaleProgressions::LIGHT_LEDBUTTON_RUN]);
        
			paramWidgets[ModeScaleProgressions::BUTTON_RESET_PARAM]=createParamCentered<LEDButton>(mm2px(Vec(19.7, 22.55)), module, ModeScaleProgressions::BUTTON_RESET_PARAM);
			addParam(paramWidgets[ModeScaleProgressions::BUTTON_RESET_PARAM]);
			lightWidgets[ModeScaleProgressions::LIGHT_LEDBUTTON_RESET]=createLightCentered<MediumSimpleLight<RedLight>>(mm2px(Vec(19.7, 22.55)), module, ModeScaleProgressions::LIGHT_LEDBUTTON_RESET);
			addChild(lightWidgets[ModeScaleProgressions::LIGHT_LEDBUTTON_RESET]);

			paramWidgets[ModeScaleProgressions::BUTTON_RAND_PARAM]=createParamCentered<LEDButton>(mm2px(Vec(29.7, 22.55)), module, ModeScaleProgressions::BUTTON_RAND_PARAM);
			addParam(paramWidgets[ModeScaleProgressions::BUTTON_RAND_PARAM]);
			lightWidgets[ModeScaleProgressions::LIGHT_LEDBUTTON_RAND]=createLightCentered<MediumSimpleLight<RedLight>>(mm2px(Vec(29.7, 22.55)), module, ModeScaleProgressions::LIGHT_LEDBUTTON_RAND);
			addChild(lightWidgets[ModeScaleProgressions::LIGHT_LEDBUTTON_RAND]);
         
			paramWidgets[ModeScaleProgressions::CONTROL_TEMPOBPM_PARAM]=createParamCentered<Trimpot>(mm2px(Vec(8.12, 35.4)), module, ModeScaleProgressions::CONTROL_TEMPOBPM_PARAM);
			addParam(paramWidgets[ModeScaleProgressions::CONTROL_TEMPOBPM_PARAM]);
			
			paramWidgets[ModeScaleProgressions::CONTROL_TIMESIGNATURETOP_PARAM]=createParamCentered<Trimpot>(mm2px(Vec(8.12, 47.322+3.0)), module, ModeScaleProgressions::CONTROL_TIMESIGNATURETOP_PARAM);
			addParam(paramWidgets[ModeScaleProgressions::CONTROL_TIMESIGNATURETOP_PARAM]);
		
			paramWidgets[ModeScaleProgressions::CONTROL_TIMESIGNATUREBOTTOM_PARAM]=createParamCentered<Trimpot>(mm2px(Vec(8.12, 54.84+3.0)), module, ModeScaleProgressions::CONTROL_TIMESIGNATUREBOTTOM_PARAM);
			addParam(paramWidgets[ModeScaleProgressions::CONTROL_TIMESIGNATUREBOTTOM_PARAM]);
			
			paramWidgets[ModeScaleProgressions::CONTROL_ROOT_KEY_PARAM]=createParamCentered<Trimpot>(mm2px(Vec(8.12, 68.179)), module, ModeScaleProgressions::CONTROL_ROOT_KEY_PARAM);
			dynamic_cast<Knob*>(paramWidgets[ModeScaleProgressions::CONTROL_ROOT_KEY_PARAM])->snap=true;
			addParam(paramWidgets[ModeScaleProgressions::CONTROL_ROOT_KEY_PARAM]);
			
			paramWidgets[ModeScaleProgressions::CONTROL_SCALE_PARAM]=createParamCentered<Trimpot>(mm2px(Vec(8.12, 78.675)), module, ModeScaleProgressions::CONTROL_SCALE_PARAM);
			dynamic_cast<Knob*>(paramWidgets[ModeScaleProgressions::CONTROL_SCALE_PARAM])->snap=true;
	 		addParam(paramWidgets[ModeScaleProgressions::CONTROL_SCALE_PARAM]);

			//***************Harmony******************
						
			paramWidgets[ModeScaleProgressions::BUTTON_ENABLE_HARMONY_PARAM]=createParamCentered<LEDButton>(mm2px(Vec(173.849, 12.622)), module, ModeScaleProgressions::BUTTON_ENABLE_HARMONY_PARAM);
			addParam(paramWidgets[ModeScaleProgressions::BUTTON_ENABLE_HARMONY_PARAM]);
			lightWidgets[ModeScaleProgressions::LIGHT_LEDBUTTON_HARMONY_ENABLE]=createLightCentered<MediumSimpleLight<RedLight>>(mm2px(Vec(173.849, 12.622)), module, ModeScaleProgressions::LIGHT_LEDBUTTON_HARMONY_ENABLE);
			addChild(lightWidgets[ModeScaleProgressions::LIGHT_LEDBUTTON_HARMONY_ENABLE]);
		
			paramWidgets[ModeScaleProgressions::CONTROL_HARMONY_VOLUME_PARAM]=createParamCentered<Trimpot>(mm2px(Vec(173.849, 20.384)), module, ModeScaleProgressions::CONTROL_HARMONY_VOLUME_PARAM);
			addParam(paramWidgets[ModeScaleProgressions::CONTROL_HARMONY_VOLUME_PARAM]);
				
			paramWidgets[ModeScaleProgressions::CONTROL_HARMONY_STEPS_PARAM]=createParamCentered<Trimpot>(mm2px(Vec(174.028, 28)), module, ModeScaleProgressions::CONTROL_HARMONY_STEPS_PARAM);
			dynamic_cast<Knob*>(paramWidgets[ModeScaleProgressions::CONTROL_HARMONY_STEPS_PARAM])->snap=true;
			addParam(paramWidgets[ModeScaleProgressions::CONTROL_HARMONY_STEPS_PARAM]);
		
			paramWidgets[ModeScaleProgressions::CONTROL_HARMONY_TARGETOCTAVE_PARAM]=createParamCentered<Trimpot>(mm2px(Vec(174.01, 37.396)), module, ModeScaleProgressions::CONTROL_HARMONY_TARGETOCTAVE_PARAM);
			dynamic_cast<Knob*>(paramWidgets[ModeScaleProgressions::CONTROL_HARMONY_TARGETOCTAVE_PARAM])->snap=true;
			addParam(paramWidgets[ModeScaleProgressions::CONTROL_HARMONY_TARGETOCTAVE_PARAM]);
			
			paramWidgets[ModeScaleProgressions::CONTROL_HARMONY_ALPHA_PARAM]=createParamCentered<Trimpot>(mm2px(Vec(174.27, 45.982)), module, ModeScaleProgressions::CONTROL_HARMONY_ALPHA_PARAM);
			addParam(paramWidgets[ModeScaleProgressions::CONTROL_HARMONY_ALPHA_PARAM]);

			paramWidgets[ModeScaleProgressions::CONTROL_HARMONY_RANGE_PARAM]=createParamCentered<Trimpot>(mm2px(Vec(173.991, 53.788)), module, ModeScaleProgressions::CONTROL_HARMONY_RANGE_PARAM);
			addParam(paramWidgets[ModeScaleProgressions::CONTROL_HARMONY_RANGE_PARAM]);
			  
			paramWidgets[ModeScaleProgressions::CONTROL_HARMONY_DIVISOR_PARAM]=createParamCentered<Trimpot>(mm2px(Vec(173.953, 62.114)), module, ModeScaleProgressions::CONTROL_HARMONY_DIVISOR_PARAM);
			dynamic_cast<Knob*>(paramWidgets[ModeScaleProgressions::CONTROL_HARMONY_DIVISOR_PARAM])->snap=true;
			addParam(paramWidgets[ModeScaleProgressions::CONTROL_HARMONY_DIVISOR_PARAM]);
			
			paramWidgets[ModeScaleProgressions::BUTTON_ENABLE_HARMONY_ALL7THS_PARAM]=createParamCentered<LEDButton>(mm2px(Vec(173.849, 69)), module, ModeScaleProgressions::BUTTON_ENABLE_HARMONY_ALL7THS_PARAM);
			addParam(paramWidgets[ModeScaleProgressions::BUTTON_ENABLE_HARMONY_ALL7THS_PARAM]);
			lightWidgets[ModeScaleProgressions::LIGHT_LEDBUTTON_ENABLE_HARMONY_ALL7THS_PARAM]=createLightCentered<MediumSimpleLight<RedLight>>(mm2px(Vec(173.849, 69)), module, ModeScaleProgressions::LIGHT_LEDBUTTON_ENABLE_HARMONY_ALL7THS_PARAM);
			addChild(lightWidgets[ModeScaleProgressions::LIGHT_LEDBUTTON_ENABLE_HARMONY_ALL7THS_PARAM]);
			
			paramWidgets[ModeScaleProgressions::BUTTON_ENABLE_HARMONY_V7THS_PARAM]=createParamCentered<LEDButton>(mm2px(Vec(203.849, 69)), module, ModeScaleProgressions::BUTTON_ENABLE_HARMONY_V7THS_PARAM);
			addParam(paramWidgets[ModeScaleProgressions::BUTTON_ENABLE_HARMONY_V7THS_PARAM]);
			lightWidgets[ModeScaleProgressions::LIGHT_LEDBUTTON_ENABLE_HARMONY_V7THS_PARAM]=createLightCentered<MediumSimpleLight<RedLight>>(mm2px(Vec(203.849, 69)), module, ModeScaleProgressions::LIGHT_LEDBUTTON_ENABLE_HARMONY_V7THS_PARAM);
			addChild(lightWidgets[ModeScaleProgressions::LIGHT_LEDBUTTON_ENABLE_HARMONY_V7THS_PARAM]);
			
			paramWidgets[ModeScaleProgressions::BUTTON_ENABLE_HARMONY_STACCATO_PARAM]=createParamCentered<LEDButton>(mm2px(Vec(173.849, 75)), module, ModeScaleProgressions::BUTTON_ENABLE_HARMONY_STACCATO_PARAM);
			addParam(paramWidgets[ModeScaleProgressions::BUTTON_ENABLE_HARMONY_STACCATO_PARAM]);
			lightWidgets[ModeScaleProgressions::LIGHT_LEDBUTTON_ENABLE_HARMONY_STACCATO_PARAM]=createLightCentered<MediumSimpleLight<RedLight>>(mm2px(Vec(173.849, 75)), module, ModeScaleProgressions::LIGHT_LEDBUTTON_ENABLE_HARMONY_STACCATO_PARAM);
			addChild(lightWidgets[ModeScaleProgressions::LIGHT_LEDBUTTON_ENABLE_HARMONY_STACCATO_PARAM]);
			
			paramWidgets[ModeScaleProgressions::CONTROL_HARMONYPRESETS_PARAM]=createParamCentered<Trimpot>(mm2px(Vec(174.027, 81.524)), module, ModeScaleProgressions::CONTROL_HARMONYPRESETS_PARAM);
	 		dynamic_cast<Knob*>(paramWidgets[ModeScaleProgressions::CONTROL_HARMONYPRESETS_PARAM])->snap=true;
			addParam(paramWidgets[ModeScaleProgressions::CONTROL_HARMONYPRESETS_PARAM]);

		
			// Progression control  

			paramWidgets[ModeScaleProgressions::BUTTON_PROG_STEP_PARAM]=createParamCentered<LEDButton>(mm2px(Vec(350, 250)), module, ModeScaleProgressions::BUTTON_PROG_STEP_PARAM);
			addParam(paramWidgets[ModeScaleProgressions::BUTTON_PROG_STEP_PARAM]);
			lightWidgets[ModeScaleProgressions::LIGHT_LEDBUTTON_PROG_STEP_PARAM]=createLightCentered<MediumSimpleLight<RedLight>>(mm2px(Vec(350, 250)), module, ModeScaleProgressions::LIGHT_LEDBUTTON_PROG_STEP_PARAM);
			addChild(lightWidgets[ModeScaleProgressions::LIGHT_LEDBUTTON_PROG_STEP_PARAM]);

			// Appended params
			paramWidgets[ModeScaleProgressions::BUTTON_ENABLE_HARMONY_4VOICE_OCTAVES_PARAM]=createParamCentered<LEDButton>(mm2px(Vec(203.849, 75)), module, ModeScaleProgressions::BUTTON_ENABLE_HARMONY_4VOICE_OCTAVES_PARAM);
			addParam(paramWidgets[ModeScaleProgressions::BUTTON_ENABLE_HARMONY_4VOICE_OCTAVES_PARAM]);
			lightWidgets[ModeScaleProgressions::LIGHT_LEDBUTTON_ENABLE_HARMONY_4VOICE_OCTAVES_PARAM]=createLightCentered<MediumSimpleLight<RedLight>>(mm2px(Vec(203.849, 75)), module, ModeScaleProgressions::LIGHT_LEDBUTTON_ENABLE_HARMONY_4VOICE_OCTAVES_PARAM);
			addChild(lightWidgets[ModeScaleProgressions::LIGHT_LEDBUTTON_ENABLE_HARMONY_4VOICE_OCTAVES_PARAM]);

			paramWidgets[ModeScaleProgressions::BUTTON_ENABLE_HARMONY_TONIC_ON_CH1_PARAM]=createParamCentered<LEDButton>(mm2px(Vec(173.849, 75)), module, ModeScaleProgressions::BUTTON_ENABLE_HARMONY_TONIC_ON_CH1_PARAM);
			addParam(paramWidgets[ModeScaleProgressions::BUTTON_ENABLE_HARMONY_TONIC_ON_CH1_PARAM]);
			lightWidgets[ModeScaleProgressions::LIGHT_LEDBUTTON_ENABLE_HARMONY_TONIC_ON_CH1_PARAM]=createLightCentered<MediumSimpleLight<RedLight>>(mm2px(Vec(173.849, 75)), module, ModeScaleProgressions::LIGHT_LEDBUTTON_ENABLE_HARMONY_TONIC_ON_CH1_PARAM);
			addChild(lightWidgets[ModeScaleProgressions::LIGHT_LEDBUTTON_ENABLE_HARMONY_TONIC_ON_CH1_PARAM]);

			paramWidgets[ModeScaleProgressions::BUTTON_ENABLE_HARMONY_BASS_ON_CH1_PARAM]=createParamCentered<LEDButton>(mm2px(Vec(173.849, 75)), module, ModeScaleProgressions::BUTTON_ENABLE_HARMONY_BASS_ON_CH1_PARAM);
			addParam(paramWidgets[ModeScaleProgressions::BUTTON_ENABLE_HARMONY_BASS_ON_CH1_PARAM]);
			lightWidgets[ModeScaleProgressions::LIGHT_LEDBUTTON_ENABLE_HARMONY_BASS_ON_CH1_PARAM]=createLightCentered<MediumSimpleLight<RedLight>>(mm2px(Vec(173.849, 75)), module, ModeScaleProgressions::LIGHT_LEDBUTTON_ENABLE_HARMONY_BASS_ON_CH1_PARAM);
			addChild(lightWidgets[ModeScaleProgressions::LIGHT_LEDBUTTON_ENABLE_HARMONY_BASS_ON_CH1_PARAM]);
			
					 
	//**************  

	// add input ports
		
			for (int i=0; i<ModeScaleProgressions::NUM_INPUTS; ++i)
			{
				if (i==ModeScaleProgressions::IN_HARMONY_DESTUTTER_EXT_CV)  // this inport is not used, so set to null
					inPortWidgets[i]=NULL;
				else
				{
					inPortWidgets[i]=createInputCentered<TinyPJ301MPort>(mm2px(Vec(10*i,5)), module, i);  // temporarily place them along the top before they are repositioned above
					addInput(inPortWidgets[i]);
				}
			}

	// add output ports		
			
			outPortWidgets[ModeScaleProgressions::OUT_RUN_OUT]=createOutputCentered<PJ301MPort>(mm2px(Vec(30.2, 10.95)), module, ModeScaleProgressions::OUT_RUN_OUT);
			addOutput(outPortWidgets[ModeScaleProgressions::OUT_RUN_OUT]);

			outPortWidgets[ModeScaleProgressions::OUT_RESET_OUT]=createOutputCentered<PJ301MPort>(mm2px(Vec(30.2, 23.35)), module, ModeScaleProgressions::OUT_RESET_OUT);
			addOutput(outPortWidgets[ModeScaleProgressions::OUT_RESET_OUT]);
				
			outPortWidgets[ModeScaleProgressions::OUT_HARMONY_VOLUME_OUTPUT]=createOutputCentered<PJ301MPort>(mm2px(Vec(201.789, 107.616)), module, ModeScaleProgressions::OUT_HARMONY_VOLUME_OUTPUT);
			addOutput(outPortWidgets[ModeScaleProgressions::OUT_HARMONY_VOLUME_OUTPUT]);

			outPortWidgets[ModeScaleProgressions::OUT_HARMONY_CV_OUTPUT]=createOutputCentered<PJ301MPort>(mm2px(Vec(202.176, 115.909)), module, ModeScaleProgressions::OUT_HARMONY_CV_OUTPUT);
			addOutput(outPortWidgets[ModeScaleProgressions::OUT_HARMONY_CV_OUTPUT]);

			outPortWidgets[ModeScaleProgressions::OUT_CLOCK_BEATX2_OUTPUT]=createOutputCentered<PJ301MPort>(mm2px(Vec(78.74, 122.291)), module, ModeScaleProgressions::OUT_CLOCK_BEATX2_OUTPUT);
			addOutput(outPortWidgets[ModeScaleProgressions::OUT_CLOCK_BEATX2_OUTPUT]);

			outPortWidgets[ModeScaleProgressions::OUT_CLOCK_BAR_OUTPUT]=createOutputCentered<PJ301MPort>(mm2px(Vec(37.143, 122.537)), module, ModeScaleProgressions::OUT_CLOCK_BAR_OUTPUT);
			addOutput(outPortWidgets[ModeScaleProgressions::OUT_CLOCK_BAR_OUTPUT]);

			outPortWidgets[ModeScaleProgressions::OUT_CLOCK_BEATX4_OUTPUT]=createOutputCentered<PJ301MPort>(mm2px(Vec(99.342, 122.573)), module, ModeScaleProgressions::OUT_CLOCK_BEATX4_OUTPUT);
			addOutput(outPortWidgets[ModeScaleProgressions::OUT_CLOCK_BEATX4_OUTPUT]);

			outPortWidgets[ModeScaleProgressions::OUT_CLOCK_BEATX8_OUTPUT]=createOutputCentered<PJ301MPort>(mm2px(Vec(121.073, 122.573)), module, ModeScaleProgressions::OUT_CLOCK_BEATX8_OUTPUT);
			addOutput(outPortWidgets[ModeScaleProgressions::OUT_CLOCK_BEATX8_OUTPUT]);

			outPortWidgets[ModeScaleProgressions::OUT_CLOCK_BEAT_OUTPUT]=createOutputCentered<PJ301MPort>(mm2px(Vec(57.856, 122.856)), module, ModeScaleProgressions::OUT_CLOCK_BEAT_OUTPUT);
			addOutput(outPortWidgets[ModeScaleProgressions::OUT_CLOCK_BEAT_OUTPUT]);

			outPortWidgets[ModeScaleProgressions::OUT_HARMONY_GATE_OUTPUT]=createOutputCentered<PJ301MPort>(mm2px(Vec(202.071, 124.267)), module, ModeScaleProgressions::OUT_HARMONY_GATE_OUTPUT);
			addOutput(outPortWidgets[ModeScaleProgressions::OUT_HARMONY_GATE_OUTPUT]);

			outPortWidgets[ModeScaleProgressions::OUT_EXT_POLY_SCALE_OUTPUT]=createOutputCentered<PJ301MPort>(mm2px(Vec(380.0, 124.831)), module, ModeScaleProgressions::OUT_EXT_POLY_SCALE_OUTPUT);
			addOutput(outPortWidgets[ModeScaleProgressions::OUT_EXT_POLY_SCALE_OUTPUT]);

			outPortWidgets[ModeScaleProgressions::OUT_CLOCK_OUT]=createOutputCentered<PJ301MPort>(mm2px(Vec(45.0, 350.0)), module, ModeScaleProgressions::OUT_CLOCK_OUT);
			addOutput(outPortWidgets[ModeScaleProgressions::OUT_CLOCK_OUT]);

			outPortWidgets[ModeScaleProgressions::OUT_EXT_POLY_QUANT_OUTPUT]=createOutputCentered<PJ301MPort>(mm2px(Vec(70.0, 345.0)), module, ModeScaleProgressions::OUT_EXT_POLY_QUANT_OUTPUT);
			addOutput(outPortWidgets[ModeScaleProgressions::OUT_EXT_POLY_QUANT_OUTPUT]);

			outPortWidgets[ModeScaleProgressions::OUT_EXT_ROOT_OUTPUT]=createOutputCentered<PJ301MPort>(mm2px(Vec(37.0, 325.0)), module, ModeScaleProgressions::OUT_EXT_ROOT_OUTPUT);
			addOutput(outPortWidgets[ModeScaleProgressions::OUT_EXT_ROOT_OUTPUT]);

			outPortWidgets[ModeScaleProgressions::OUT_EXT_SCALE_OUTPUT]=createOutputCentered<PJ301MPort>(mm2px(Vec(57.0, 350.0)), module, ModeScaleProgressions::OUT_EXT_SCALE_OUTPUT);
			addOutput(outPortWidgets[ModeScaleProgressions::OUT_EXT_SCALE_OUTPUT]);
			
			outPortWidgets[ModeScaleProgressions::OUT_EXT_HARMONIC_DEGREE_OUTPUT]=createOutputCentered<PJ301MPort>(mm2px(Vec(380.0, 124.831)), module, ModeScaleProgressions::OUT_EXT_HARMONIC_DEGREE_OUTPUT);
			addOutput(outPortWidgets[ModeScaleProgressions::OUT_EXT_HARMONIC_DEGREE_OUTPUT]);

			outPortWidgets[ModeScaleProgressions::OUT_EXT_POLY_QUANT_TRIGGER_OUTPUT]=createOutputCentered<PJ301MPort>(Vec(250.98, 1018.70), module, ModeScaleProgressions::OUT_EXT_POLY_QUANT_TRIGGER_OUTPUT);
			addOutput(outPortWidgets[ModeScaleProgressions::OUT_EXT_POLY_QUANT_TRIGGER_OUTPUT]);

			outPortWidgets[ModeScaleProgressions::OUT_EXT_HARMONIC_DEGREE_CHORD_TYPE_OUTPUT]=createOutputCentered<PJ301MPort>(mm2px(Vec(380.0, 140.)), module, ModeScaleProgressions::OUT_EXT_HARMONIC_DEGREE_CHORD_TYPE_OUTPUT);
			addOutput(outPortWidgets[ModeScaleProgressions::OUT_EXT_HARMONIC_DEGREE_CHORD_TYPE_OUTPUT]);

									
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
				controlPosition=controlPosition.minus((paramWidgets[ModeScaleProgressions::BUTTON_CIRCLESTEP_C_PARAM+i]->box.size).div(2.));  // adjust for box size
				paramWidgets[ModeScaleProgressions::BUTTON_CIRCLESTEP_C_PARAM+i]->box.pos=controlPosition;
				
				controlPosition=CircleCenter.plus(radialDirection.mult(OuterCircleRadius*.78f));
				controlPosition=controlPosition.minus((lightWidgets[ModeScaleProgressions::LIGHT_LEDBUTTON_CIRCLESTEP_1+i]->box.size).div(2.));  // adjust for box size
				lightWidgets[ModeScaleProgressions::LIGHT_LEDBUTTON_CIRCLESTEP_1+i]->box.pos=controlPosition;
				

				controlPosition=CircleCenter.plus(radialDirection.mult(OuterCircleRadius*.61f));
				controlPosition=controlPosition.minus((lightWidgets[ModeScaleProgressions::LIGHT_CIRCLE_ROOT_KEY_POSITION_1_LIGHT+i]->box.size).div(2.));
				lightWidgets[ModeScaleProgressions::LIGHT_CIRCLE_ROOT_KEY_POSITION_1_LIGHT+i]->box.pos=controlPosition;
			}
 
		
			// re-layout the circle step set buttons and lights programatically
			Vec start_position=Vec(0,0);
			float verticalIncrement=mm2px(5.92f);
			for(int i = 0; i<MAX_STEPS;++i) 
			{
				start_position=mm2px(Vec(62, 128.9- 109.27));  // for Y subtract SVG Y from panel height 128.9 and then convert to px
				Vec buttonPosition=start_position.plus(Vec(0, (i*verticalIncrement)));
				buttonPosition.y -= paramWidgets[ModeScaleProgressions::BUTTON_HARMONY_SETSTEP_1_PARAM+i]->box.size.y;  // adjust for box height
				paramWidgets[ModeScaleProgressions::BUTTON_HARMONY_SETSTEP_1_PARAM+i]->box.pos=buttonPosition;
				start_position=mm2px(Vec(63.5, 128.9- 110.8));  // for Y subtract SVG Y from panel height 128.9 and then convert to px
				buttonPosition=start_position.plus(Vec(0, (i*verticalIncrement)));
				buttonPosition.y -= lightWidgets[ModeScaleProgressions::LIGHT_LEDBUTTON_CIRCLESETSTEP_1+i]->box.size.y; // adjust for box height
				lightWidgets[ModeScaleProgressions::LIGHT_LEDBUTTON_CIRCLESETSTEP_1+i]->box.pos=buttonPosition;
			}


			// relayout all param controls and lights
		
			Vec drawCenter=Vec(5., 30.);
			
			// do upper left controls and ports
			drawCenter=drawCenter.plus(Vec(37,0));
			paramWidgets[ModeScaleProgressions::BUTTON_RUN_PARAM]->box.pos=drawCenter.minus(paramWidgets[ModeScaleProgressions::BUTTON_RUN_PARAM]->box.size.div(2.));
			lightWidgets[ModeScaleProgressions::LIGHT_LEDBUTTON_RUN]->box.pos=drawCenter.minus(lightWidgets[ModeScaleProgressions::LIGHT_LEDBUTTON_RUN]->box.size.div(2.));
			drawCenter=drawCenter.plus(Vec(37,0));
			outPortWidgets[ModeScaleProgressions::OUT_RUN_OUT]->box.pos=drawCenter.minus(outPortWidgets[ModeScaleProgressions::OUT_RUN_OUT]->box.size.div(2.));
			
			drawCenter=drawCenter.minus(Vec(37,0)); 
			drawCenter=drawCenter.plus(Vec(0,40));
	
			paramWidgets[ModeScaleProgressions::BUTTON_RESET_PARAM]->box.pos=drawCenter.minus(paramWidgets[ModeScaleProgressions::BUTTON_RESET_PARAM]->box.size.div(2.));
			lightWidgets[ModeScaleProgressions::LIGHT_LEDBUTTON_RESET]->box.pos=drawCenter.minus(lightWidgets[ModeScaleProgressions::LIGHT_LEDBUTTON_RESET]->box.size.div(2.));
			drawCenter=drawCenter.plus(Vec(37,0));
			outPortWidgets[ModeScaleProgressions::OUT_RESET_OUT]->box.pos=drawCenter.minus(outPortWidgets[ModeScaleProgressions::OUT_RESET_OUT]->box.size.div(2.));

			drawCenter=Vec(137, 70.);
			paramWidgets[ModeScaleProgressions::BUTTON_RAND_PARAM]->box.pos=drawCenter.minus(paramWidgets[ModeScaleProgressions::BUTTON_RAND_PARAM]->box.size.div(2.));
			lightWidgets[ModeScaleProgressions::LIGHT_LEDBUTTON_RAND]->box.pos=drawCenter.minus(lightWidgets[ModeScaleProgressions::LIGHT_LEDBUTTON_RAND]->box.size.div(2.));
			
			drawCenter=Vec(42., 110.);
			
			paramWidgets[ModeScaleProgressions::CONTROL_TEMPOBPM_PARAM]->box.pos=drawCenter.minus(paramWidgets[ModeScaleProgressions::CONTROL_TEMPOBPM_PARAM]->box.size.div(2.));
				
			drawCenter=drawCenter.plus(Vec(0,25));
			
			paramWidgets[ModeScaleProgressions::CONTROL_TIMESIGNATURETOP_PARAM]->box.pos=drawCenter.minus(paramWidgets[ModeScaleProgressions::CONTROL_TIMESIGNATURETOP_PARAM]->box.size.div(2.));
			drawCenter=drawCenter.plus(Vec(0,25));
			paramWidgets[ModeScaleProgressions::CONTROL_TIMESIGNATUREBOTTOM_PARAM]->box.pos=drawCenter.minus(paramWidgets[ModeScaleProgressions::CONTROL_TIMESIGNATUREBOTTOM_PARAM]->box.size.div(2.));

			drawCenter=drawCenter.plus(Vec(0,25));
			paramWidgets[ModeScaleProgressions::CONTROL_ROOT_KEY_PARAM]->box.pos=drawCenter.minus(paramWidgets[ModeScaleProgressions::CONTROL_ROOT_KEY_PARAM]->box.size.div(2.));


			drawCenter=drawCenter.plus(Vec(0,25));
			paramWidgets[ModeScaleProgressions::CONTROL_SCALE_PARAM]->box.pos=drawCenter.minus(paramWidgets[ModeScaleProgressions::CONTROL_SCALE_PARAM]->box.size.div(2.));
												
			// redo all harmony controls positions
			drawCenter=Vec(512., 40.);
	
			paramWidgets[ModeScaleProgressions::BUTTON_ENABLE_HARMONY_PARAM]->box.pos=drawCenter.minus(paramWidgets[ModeScaleProgressions::BUTTON_ENABLE_HARMONY_PARAM]->box.size.div(2.));
			lightWidgets[ModeScaleProgressions::LIGHT_LEDBUTTON_HARMONY_ENABLE]->box.pos=drawCenter.minus(lightWidgets[ModeScaleProgressions::LIGHT_LEDBUTTON_HARMONY_ENABLE]->box.size.div(2.));
			drawCenter=drawCenter.plus(Vec(0,22));	
			paramWidgets[ModeScaleProgressions::CONTROL_HARMONY_VOLUME_PARAM]->box.pos=drawCenter.minus(paramWidgets[ModeScaleProgressions::CONTROL_HARMONY_VOLUME_PARAM]->box.size.div(2.));
			drawCenter=drawCenter.plus(Vec(-20,22));
			inPortWidgets[ModeScaleProgressions::IN_HARMONY_STEPS_EXT_CV]->box.pos=drawCenter.minus(inPortWidgets[ModeScaleProgressions::IN_HARMONY_STEPS_EXT_CV]->box.size.div(2.));	
			drawCenter=drawCenter.plus(Vec(20,0));
			paramWidgets[ModeScaleProgressions::CONTROL_HARMONY_STEPS_PARAM]->box.pos=drawCenter.minus(paramWidgets[ModeScaleProgressions::CONTROL_HARMONY_STEPS_PARAM]->box.size.div(2.));
			drawCenter=drawCenter.plus(Vec(0,22));		
			paramWidgets[ModeScaleProgressions::CONTROL_HARMONY_TARGETOCTAVE_PARAM]->box.pos=drawCenter.minus(paramWidgets[ModeScaleProgressions::CONTROL_HARMONY_TARGETOCTAVE_PARAM]->box.size.div(2.));
			drawCenter=drawCenter.plus(Vec(0,22));			
			paramWidgets[ModeScaleProgressions::CONTROL_HARMONY_ALPHA_PARAM]->box.pos=drawCenter.minus(paramWidgets[ModeScaleProgressions::CONTROL_HARMONY_ALPHA_PARAM]->box.size.div(2.));
			drawCenter=drawCenter.plus(Vec(0,22));
			paramWidgets[ModeScaleProgressions::CONTROL_HARMONY_RANGE_PARAM]->box.pos=drawCenter.minus(paramWidgets[ModeScaleProgressions::CONTROL_HARMONY_RANGE_PARAM]->box.size.div(2.));
			drawCenter=drawCenter.plus(Vec(0,22));			  
			paramWidgets[ModeScaleProgressions::CONTROL_HARMONY_DIVISOR_PARAM]->box.pos=drawCenter.minus(paramWidgets[ModeScaleProgressions::CONTROL_HARMONY_DIVISOR_PARAM]->box.size.div(2.));
			drawCenter=drawCenter.plus(Vec(0,22));			
			paramWidgets[ModeScaleProgressions::BUTTON_ENABLE_HARMONY_ALL7THS_PARAM]->box.pos=drawCenter.minus(paramWidgets[ModeScaleProgressions::BUTTON_ENABLE_HARMONY_ALL7THS_PARAM]->box.size.div(2.));
			lightWidgets[ModeScaleProgressions::LIGHT_LEDBUTTON_ENABLE_HARMONY_ALL7THS_PARAM]->box.pos=drawCenter.minus(lightWidgets[ModeScaleProgressions::LIGHT_LEDBUTTON_ENABLE_HARMONY_ALL7THS_PARAM]->box.size.div(2.));
			drawCenter=drawCenter.plus(Vec(85,0));
			paramWidgets[ModeScaleProgressions::BUTTON_ENABLE_HARMONY_V7THS_PARAM]->box.pos=drawCenter.minus(paramWidgets[ModeScaleProgressions::BUTTON_ENABLE_HARMONY_V7THS_PARAM]->box.size.div(2.));
			lightWidgets[ModeScaleProgressions::LIGHT_LEDBUTTON_ENABLE_HARMONY_V7THS_PARAM]->box.pos=drawCenter.minus(lightWidgets[ModeScaleProgressions::LIGHT_LEDBUTTON_ENABLE_HARMONY_V7THS_PARAM]->box.size.div(2.));

			drawCenter=drawCenter.plus(Vec(-85,22));			
			paramWidgets[ModeScaleProgressions::BUTTON_ENABLE_HARMONY_STACCATO_PARAM]->box.pos=drawCenter.minus(paramWidgets[ModeScaleProgressions::BUTTON_ENABLE_HARMONY_STACCATO_PARAM]->box.size.div(2.));
			lightWidgets[ModeScaleProgressions::LIGHT_LEDBUTTON_ENABLE_HARMONY_STACCATO_PARAM]->box.pos=drawCenter.minus(lightWidgets[ModeScaleProgressions::LIGHT_LEDBUTTON_ENABLE_HARMONY_STACCATO_PARAM]->box.size.div(2.));

			drawCenter=drawCenter.plus(Vec(85,0));	
			paramWidgets[ModeScaleProgressions::BUTTON_ENABLE_HARMONY_4VOICE_OCTAVES_PARAM]->box.pos=drawCenter.minus(paramWidgets[ModeScaleProgressions::BUTTON_ENABLE_HARMONY_4VOICE_OCTAVES_PARAM]->box.size.div(2.));
			lightWidgets[ModeScaleProgressions::LIGHT_LEDBUTTON_ENABLE_HARMONY_4VOICE_OCTAVES_PARAM]->box.pos=drawCenter.minus(lightWidgets[ModeScaleProgressions::LIGHT_LEDBUTTON_ENABLE_HARMONY_4VOICE_OCTAVES_PARAM]->box.size.div(2.));
    			
			drawCenter=drawCenter.plus(Vec(-85,22));	
			paramWidgets[ModeScaleProgressions::CONTROL_HARMONYPRESETS_PARAM]->box.pos=drawCenter.minus(paramWidgets[ModeScaleProgressions::CONTROL_HARMONYPRESETS_PARAM]->box.size.div(2.));

			drawCenter=drawCenter.plus(Vec(0,22));	
			paramWidgets[ModeScaleProgressions::BUTTON_ENABLE_HARMONY_TONIC_ON_CH1_PARAM]->box.pos=drawCenter.minus(paramWidgets[ModeScaleProgressions::BUTTON_ENABLE_HARMONY_TONIC_ON_CH1_PARAM]->box.size.div(2.));
			lightWidgets[ModeScaleProgressions::LIGHT_LEDBUTTON_ENABLE_HARMONY_TONIC_ON_CH1_PARAM]->box.pos=drawCenter.minus(lightWidgets[ModeScaleProgressions::LIGHT_LEDBUTTON_ENABLE_HARMONY_TONIC_ON_CH1_PARAM]->box.size.div(2.));

			drawCenter=drawCenter.plus(Vec(85,0));	
			paramWidgets[ModeScaleProgressions::BUTTON_ENABLE_HARMONY_BASS_ON_CH1_PARAM]->box.pos=drawCenter.minus(paramWidgets[ModeScaleProgressions::BUTTON_ENABLE_HARMONY_BASS_ON_CH1_PARAM]->box.size.div(2.));
			lightWidgets[ModeScaleProgressions::LIGHT_LEDBUTTON_ENABLE_HARMONY_BASS_ON_CH1_PARAM]->box.pos=drawCenter.minus(lightWidgets[ModeScaleProgressions::LIGHT_LEDBUTTON_ENABLE_HARMONY_BASS_ON_CH1_PARAM]->box.size.div(2.));
								
			drawCenter=Vec(345., 250.);
			paramWidgets[ModeScaleProgressions::BUTTON_PROG_STEP_PARAM]->box.pos=drawCenter.minus(paramWidgets[ModeScaleProgressions::BUTTON_PROG_STEP_PARAM]->box.size.div(2.));
			lightWidgets[ModeScaleProgressions::LIGHT_LEDBUTTON_PROG_STEP_PARAM]->box.pos=drawCenter.minus(lightWidgets[ModeScaleProgressions::LIGHT_LEDBUTTON_PROG_STEP_PARAM]->box.size.div(2.));
		
			// re-layout all input ports.  Work around parm and input enum value mismatch due to history
			for (int i=0; i<ModeScaleProgressions::NUM_INPUTS; ++i)
			{
				if (i==ModeScaleProgressions::IN_RAND_EXT_CV)
				{
					Vec drawCenter=Vec(112., 70.);  
					inPortWidgets[ModeScaleProgressions::IN_RAND_EXT_CV]->box.pos=drawCenter.minus(inPortWidgets[ModeScaleProgressions::IN_RAND_EXT_CV]->box.size.div(2.));
				}
				else
				if (i<=ModeScaleProgressions::IN_SCALE_EXT_CV)
				{
					if ((inPortWidgets[i]!=NULL)&&(paramWidgets[i]!=NULL))
					{
						inPortWidgets[i]->box.pos= paramWidgets[i]->box.pos.minus(Vec(20,-1));
					}
				}
				else
				if (i==ModeScaleProgressions::IN_CLOCK_EXT_CV)
				{
					Vec drawCenter=Vec(20., 365.); 
					inPortWidgets[ModeScaleProgressions::IN_CLOCK_EXT_CV]->box.pos=drawCenter.minus(inPortWidgets[ModeScaleProgressions::IN_CLOCK_EXT_CV]->box.size.div(2.));
				}
				else
				if (i==ModeScaleProgressions::IN_HARMONY_CIRCLE_DEGREE_EXT_CV)
				{
					Vec drawCenter=Vec(310., 210.);
					inPortWidgets[ModeScaleProgressions::IN_HARMONY_CIRCLE_DEGREE_EXT_CV]->box.pos=drawCenter.minus(inPortWidgets[ModeScaleProgressions::IN_HARMONY_CIRCLE_DEGREE_EXT_CV]->box.size.div(2.));
				}
				else
				if (i==ModeScaleProgressions::IN_HARMONY_CIRCLE_DEGREE_GATE_EXT_CV)
				{
					Vec drawCenter=Vec(310., 230.);
					inPortWidgets[ModeScaleProgressions::IN_HARMONY_CIRCLE_DEGREE_GATE_EXT_CV]->box.pos=drawCenter.minus(inPortWidgets[ModeScaleProgressions::IN_HARMONY_CIRCLE_DEGREE_GATE_EXT_CV]->box.size.div(2.));
				}
				else
				if (i==ModeScaleProgressions::IN_POLY_QUANT_EXT_CV)
				{
					Vec drawCenter=Vec(20., Quantizer_row); 
					inPortWidgets[ModeScaleProgressions::IN_POLY_QUANT_EXT_CV]->box.pos=drawCenter.minus(inPortWidgets[ModeScaleProgressions::IN_POLY_QUANT_EXT_CV]->box.size.div(2.));
				}
				else
				if (i==ModeScaleProgressions::IN_ENABLE_HARMONY_4VOICE_OCTAVES_EXT_CV)
				{
					if ((inPortWidgets[i]!=NULL)&&(paramWidgets[ModeScaleProgressions::BUTTON_ENABLE_HARMONY_4VOICE_OCTAVES_PARAM]!=NULL))
						inPortWidgets[i]->box.pos= paramWidgets[ModeScaleProgressions::BUTTON_ENABLE_HARMONY_4VOICE_OCTAVES_PARAM]->box.pos.minus(Vec(20,-1));
				}
				else
				if (i==ModeScaleProgressions::IN_ENABLE_HARMONY_TONIC_ON_CH1_EXT_CV)
				{
					if ((inPortWidgets[i]!=NULL)&&(paramWidgets[ModeScaleProgressions::BUTTON_ENABLE_HARMONY_TONIC_ON_CH1_PARAM]!=NULL))
						inPortWidgets[i]->box.pos= paramWidgets[ModeScaleProgressions::BUTTON_ENABLE_HARMONY_TONIC_ON_CH1_PARAM]->box.pos.minus(Vec(20,-1));
				}
				else
				if (i==ModeScaleProgressions::IN_ENABLE_HARMONY_BASS_ON_CH1_EXT_CV)
				{
					if ((inPortWidgets[i]!=NULL)&&(paramWidgets[ModeScaleProgressions::BUTTON_ENABLE_HARMONY_BASS_ON_CH1_PARAM]!=NULL))
						inPortWidgets[i]->box.pos= paramWidgets[ModeScaleProgressions::BUTTON_ENABLE_HARMONY_BASS_ON_CH1_PARAM]->box.pos.minus(Vec(20,-1));
				}
		          	else
				{
					int parmIndex=ModeScaleProgressions::BUTTON_ENABLE_HARMONY_PARAM+i-ModeScaleProgressions::IN_HARMONY_CIRCLE_DEGREE_GATE_EXT_CV-1;
					if ((inPortWidgets[i]!=NULL)&&(paramWidgets[parmIndex]!=NULL))
						inPortWidgets[i]->box.pos= paramWidgets[parmIndex]->box.pos.minus(Vec(20,-1));
				}
							
				
			}
			

			// re-layout all output port

			drawCenter=Vec(520., 365.);

			outPortWidgets[ModeScaleProgressions::OUT_HARMONY_CV_OUTPUT]->box.pos=drawCenter.minus(outPortWidgets[ModeScaleProgressions::OUT_HARMONY_CV_OUTPUT]->box.size.div(2.));
			drawCenter=drawCenter.plus(Vec(60,0));
			outPortWidgets[ModeScaleProgressions::OUT_HARMONY_GATE_OUTPUT]->box.pos=drawCenter.minus(outPortWidgets[ModeScaleProgressions::OUT_HARMONY_GATE_OUTPUT]->box.size.div(2.));
			drawCenter=drawCenter.plus(Vec(60,0));
			outPortWidgets[ModeScaleProgressions::OUT_HARMONY_VOLUME_OUTPUT]->box.pos=drawCenter.minus(outPortWidgets[ModeScaleProgressions::OUT_HARMONY_VOLUME_OUTPUT]->box.size.div(2.));
       
	   		drawCenter=Vec(85., 365.);
			outPortWidgets[ModeScaleProgressions::OUT_CLOCK_BAR_OUTPUT]->box.pos=drawCenter.minus(outPortWidgets[ModeScaleProgressions::OUT_CLOCK_BAR_OUTPUT]->box.size.div(2.));
			drawCenter=drawCenter.plus(Vec(33,0));
			outPortWidgets[ModeScaleProgressions::OUT_CLOCK_BEAT_OUTPUT]->box.pos=drawCenter.minus(outPortWidgets[ModeScaleProgressions::OUT_CLOCK_BEAT_OUTPUT]->box.size.div(2.));
			drawCenter=drawCenter.plus(Vec(33,0));
			outPortWidgets[ModeScaleProgressions::OUT_CLOCK_BEATX2_OUTPUT]->box.pos=drawCenter.minus(outPortWidgets[ModeScaleProgressions::OUT_CLOCK_BEATX2_OUTPUT]->box.size.div(2.));
			drawCenter=drawCenter.plus(Vec(33,0));
			outPortWidgets[ModeScaleProgressions::OUT_CLOCK_BEATX4_OUTPUT]->box.pos=drawCenter.minus(outPortWidgets[ModeScaleProgressions::OUT_CLOCK_BEATX4_OUTPUT]->box.size.div(2.));
			drawCenter=drawCenter.plus(Vec(33,0));
			outPortWidgets[ModeScaleProgressions::OUT_CLOCK_BEATX8_OUTPUT]->box.pos=drawCenter.minus(outPortWidgets[ModeScaleProgressions::OUT_CLOCK_BEATX8_OUTPUT]->box.size.div(2.));
			drawCenter=drawCenter.plus(Vec(33,0));

			drawCenter=Vec(145., 289.);
			outPortWidgets[ModeScaleProgressions::OUT_EXT_POLY_SCALE_OUTPUT]->box.pos=drawCenter.minus(outPortWidgets[ModeScaleProgressions::OUT_EXT_POLY_SCALE_OUTPUT]->box.size.div(2.));
			drawCenter=drawCenter.plus(Vec(40,0));
				
			drawCenter=Vec(50., 365.); 
			outPortWidgets[ModeScaleProgressions::OUT_CLOCK_OUT]->box.pos=drawCenter.minus(outPortWidgets[ModeScaleProgressions::OUT_CLOCK_OUT]->box.size.div(2.));
			drawCenter=drawCenter.plus(Vec(40,0));
		
			drawCenter=Vec(50., Quantizer_row);  
			outPortWidgets[ModeScaleProgressions::OUT_EXT_POLY_QUANT_OUTPUT]->box.pos=drawCenter.minus(outPortWidgets[ModeScaleProgressions::OUT_EXT_POLY_QUANT_OUTPUT]->box.size.div(2.));
			drawCenter=drawCenter.plus(Vec(40,0));

			drawCenter=Vec(50.0 + 40.0, Quantizer_row);  
			outPortWidgets[ModeScaleProgressions::OUT_EXT_POLY_QUANT_TRIGGER_OUTPUT]->box.pos=drawCenter.minus(outPortWidgets[ModeScaleProgressions::OUT_EXT_POLY_QUANT_TRIGGER_OUTPUT]->box.size.div(2.));
			drawCenter=drawCenter.plus(Vec(40,0));
			
			drawCenter=Vec(95., 193.);  
			outPortWidgets[ModeScaleProgressions::OUT_EXT_ROOT_OUTPUT]->box.pos=drawCenter.minus(outPortWidgets[ModeScaleProgressions::OUT_EXT_ROOT_OUTPUT]->box.size.div(2.));
			drawCenter=drawCenter.plus(Vec(40,0));

			drawCenter=Vec(145., 245.);  
			outPortWidgets[ModeScaleProgressions::OUT_EXT_SCALE_OUTPUT]->box.pos=drawCenter.minus(outPortWidgets[ModeScaleProgressions::OUT_EXT_SCALE_OUTPUT]->box.size.div(2.));
			drawCenter=drawCenter.plus(Vec(40,0));

			drawCenter=Vec( 383., 211.);
			outPortWidgets[ModeScaleProgressions::OUT_EXT_HARMONIC_DEGREE_OUTPUT]->box.pos=drawCenter.minus(outPortWidgets[ModeScaleProgressions::OUT_EXT_HARMONIC_DEGREE_OUTPUT]->box.size.div(2.));
			drawCenter=drawCenter.plus(Vec(40,0));
			
			drawCenter=Vec( 383., 230.);
			outPortWidgets[ModeScaleProgressions::OUT_EXT_HARMONIC_DEGREE_CHORD_TYPE_OUTPUT]->box.pos=drawCenter.minus(outPortWidgets[ModeScaleProgressions::OUT_EXT_HARMONIC_DEGREE_CHORD_TYPE_OUTPUT]->box.size.div(2.));
			drawCenter=drawCenter.plus(Vec(40,0));

			//********************
			
			for (int i=0; ((i<ModeScaleProgressions::NUM_PARAMS)&&(i<MAX_PARAMS)); ++i)  // get the paramWidget box into a ModeScaleProgressionsWidget array so it can be accessed as needed
			{
				if (paramWidgets[i]!=NULL) 
					ParameterRect[i]=paramWidgets[i]->box;
			}

			for (int i=0; ((i<ModeScaleProgressions::NUM_OUTPUTS)&&(i<MAX_OUTPORTS)); ++i)  // get the paramWidget box into a ModeScaleProgressionsWidget array so it can be accessed as needed
			{
				if (outPortWidgets[i]!=NULL) 
					OutportRect[i]=outPortWidgets[i]->box;
			}

			for (int i=0; ((i<ModeScaleProgressions::NUM_INPUTS)&&(i<MAX_INPORTS)); ++i)  // get the paramWidget box into a ModeScaleProgressionsWidget array so it can be accessed as needed
			{
				if (inPortWidgets[i]!=NULL) 
					InportRect[i]=inPortWidgets[i]->box;
			}
				
		}  // end if (true)  

	}    // end ModeScaleProgressionsWidget(ModeScaleProgressions* module)  

 
	// create panel theme and contrast control
	
	void appendContextMenu(Menu *menu) override 
	{  
        ModeScaleProgressions *module = dynamic_cast<ModeScaleProgressions*>(this->module);
		if (module==NULL)
			return;
   
		MenuLabel *panelthemeLabel = new MenuLabel();
        panelthemeLabel->text = "Panel Theme                               ";
        menu->addChild(panelthemeLabel);

		ModeScaleProgressionsPanelThemeItem *lightpaneltheme_Item = new ModeScaleProgressionsPanelThemeItem();  // this accomodates json loaded value
        lightpaneltheme_Item->text = "  light";
		lightpaneltheme_Item->module = module;
   	    lightpaneltheme_Item->theme = 0;
	    menu->addChild(lightpaneltheme_Item); 

		ModeScaleProgressionsPanelThemeItem *darkpaneltheme_Item = new ModeScaleProgressionsPanelThemeItem();  // this accomodates json loaded value
        darkpaneltheme_Item->text = "  dark";
		darkpaneltheme_Item->module = module;
   	    darkpaneltheme_Item->theme = 1;
        menu->addChild(darkpaneltheme_Item);

		// create contrast control
	
		MinMaxSliderItem *minSliderItem = new MinMaxSliderItem(&MSP_panelContrast, "Contrast");
		minSliderItem->box.size.x = 200.f;
		menu->addChild(minSliderItem);
	
		//

		MenuLabel *modeLabel = new MenuLabel();
        modeLabel->text = "Scale Out Mode                               ";
        menu->addChild(modeLabel);
		
		
		ModeScaleProgressionsScaleOutModeItem *chromatic_Item = new ModeScaleProgressionsScaleOutModeItem();
        chromatic_Item->text = "  Heptatonic Chromatic Scale-12ch";
        chromatic_Item->module = module;
        chromatic_Item->mode = ModeScaleProgressions::HEPTATONIC_CHROMATIC_12CH;
        menu->addChild(chromatic_Item);

		ModeScaleProgressionsScaleOutModeItem *heptatonic_Item = new ModeScaleProgressionsScaleOutModeItem();
        heptatonic_Item->text = "  Heptatonic Diatonic STD-7ch";
        heptatonic_Item->module = module;
        heptatonic_Item->mode = ModeScaleProgressions::HEPTATONIC_DIATONIC_STD_7CH;
        menu->addChild(heptatonic_Item);


		ModeScaleProgressionsScaleOutModeItem *pentatonic_Item = new ModeScaleProgressionsScaleOutModeItem();
        pentatonic_Item->text = "  Pentatonic-5ch";
        pentatonic_Item->module =module;
        pentatonic_Item->mode = ModeScaleProgressions::PENTATONIC_5CH;
        menu->addChild(pentatonic_Item); 
		

		ModeScaleProgressionsScaleOutModeItem *chromatic_pentatonic_Item = new ModeScaleProgressionsScaleOutModeItem();
        chromatic_pentatonic_Item->text = "  Pentatonic Chromatic-12ch";
        chromatic_pentatonic_Item->module = module;
        chromatic_pentatonic_Item->mode = ModeScaleProgressions::PENTATONIC_CHROMATIC_12CH;
        menu->addChild(chromatic_pentatonic_Item);

		
   		MenuLabel *modeLabel3 = new MenuLabel();
        modeLabel3->text = "Gate Out Mode                 ";
        menu->addChild(modeLabel3);

				
		ModeScaleProgressionsGateOutModeItem *gate_standard_Item = new ModeScaleProgressionsGateOutModeItem();
        gate_standard_Item->text = "  Standard 10V";
        gate_standard_Item->module = module;
        gate_standard_Item->mode = ModeScaleProgressions::STANDARD_GATE;
        menu->addChild(gate_standard_Item);

		ModeScaleProgressionsGateOutModeItem *gate_volume_Item = new ModeScaleProgressionsGateOutModeItem();
        gate_volume_Item->text = "  Volume over gate 2.1-10V";
        gate_volume_Item->module = module;
        gate_volume_Item->mode = ModeScaleProgressions::VOLUME_OVER_GATE;
        menu->addChild(gate_volume_Item);

		//

		MenuLabel *modeLabel4 = new MenuLabel();
        modeLabel4->text = "Harmonic Degree Output Range                ";
        menu->addChild(modeLabel4);

				
		ModeScaleProgressionsDegreeOutRangeItem *degree_out_range_standard_Item = new ModeScaleProgressionsDegreeOutRangeItem();
        degree_out_range_standard_Item->text = "  ModeScaleProgressions Standard 1-7V";
        degree_out_range_standard_Item->module = module;
        degree_out_range_standard_Item->rangemode = ModeScaleProgressions::RANGE_1to7;
        menu->addChild(degree_out_range_standard_Item);

		ModeScaleProgressionsDegreeOutRangeItem *degree_out_range_alternative_Item = new ModeScaleProgressionsDegreeOutRangeItem();
        degree_out_range_alternative_Item->text = "  External 0-6V";
        degree_out_range_alternative_Item->module = module;
        degree_out_range_alternative_Item->rangemode = ModeScaleProgressions::RANGE_0to6;
        menu->addChild(degree_out_range_alternative_Item);

		
	}  // end ModeScaleProgressionsWidget() 

	void step() override   // note, this is a widget step() which is not deprecated and is a GUI call.  This advances UI by one "frame"    
	{  
		ModeScaleProgressions *module = dynamic_cast<ModeScaleProgressions*>(this->module);  

		if (true) // needs to happen even if module==null
		{
			if (svgPanel)
			    svgPanel->setVisible((MSP_panelTheme) == 0);    
			if (darkPanel)                             
				darkPanel->setVisible((MSP_panelTheme) == 1);    
		
			float contrast=MSP_panelContrast;

			// update the global panel theme vars
			if (MSP_panelTheme==0)  // light theme
			{
				MSP_panelcolor=nvgRGBA((unsigned char)230,(unsigned char)230,(unsigned char)230,(unsigned char)255);
				float color =255*(1-contrast);
				MSP_panelTextColor=nvgRGBA((unsigned char)color,(unsigned char)color,(unsigned char)color,(unsigned char)255);  // black text
				MSP_panelLineColor=nvgRGBA((unsigned char)color,(unsigned char)color,(unsigned char)color,(unsigned char)255);  // black lines
				color =255*contrast;
				MSP_paramTextColor=nvgRGBA((unsigned char)color,(unsigned char)color,(unsigned char)0,(unsigned char)255);  // yellow text
			
				{
					float r=MSP_panelHarmonyPartBaseColor.r; 
					float g=(1-contrast);
					float b=(1-contrast);
					MSP_panelHarmonyPartColor=nvgRGBA(r*156, g*255, b*255, 255);
				}
				{
					float r=(1-contrast);
					float g=(1-contrast);
					float b=MSP_panelArpPartBaseColor.b;
					MSP_panelArpPartColor=nvgRGBA(r*255, g*255, b*255, 255);
				}
				{
					float r=(1-contrast);
					float g=MSP_panelBassPartBaseColor.g;
					float b=(1-contrast);
					MSP_panelBassPartColor=nvgRGBA(r*255, g*128, b*255, 255);
				}
				{
					float r=(1-contrast);
					float g=(1-contrast);
					float b=(1-contrast);
					MSP_panelMelodyPartColor=nvgRGBA(r*255, g*255, b*255, 255);
				}
						 
			}
			else  // dark theme   
			{
				MSP_panelcolor=nvgRGBA((unsigned char)40,(unsigned char)40,(unsigned char)40,(unsigned char)255);
				float color = 255*contrast;
				MSP_panelTextColor=nvgRGBA((unsigned char)color,(unsigned char)color,(unsigned char)color,(unsigned char)255);  // white text
				MSP_panelLineColor=nvgRGBA((unsigned char)color,(unsigned char)color,(unsigned char)color,(unsigned char)255);  // white lines
				color =255*contrast;
				MSP_paramTextColor=nvgRGBA((unsigned char)color,(unsigned char)color,(unsigned char)0,(unsigned char)255);  // yellow text

				{
					float r=MSP_panelHarmonyPartBaseColor.r*contrast;
					float g=0.45;
					float b=0.45;
					MSP_panelHarmonyPartColor=nvgRGBA(r*255, g*255, b*255, 255);
				}
				{
					float b=MSP_panelArpPartBaseColor.b*contrast;
					float r=0.45;
					float g=0.45;
					MSP_panelArpPartColor=nvgRGBA(r*255, g*255, b*255, 255);
				}
				{
					float g=MSP_panelBassPartBaseColor.g*contrast;
					float r=0.45;
					float b=0.45;
					MSP_panelBassPartColor=nvgRGBA(r*255, g*255, b*255, 255);
				}
				{
					float r=(contrast);
					float g=(contrast);
					float b=(contrast);
					MSP_panelMelodyPartColor=nvgRGBA(r*228, g*228, b*228, 255);
				}
			}
		}

		// determine STEP cable connections to ModeScaleProgressions trigger outs, if any
	  	if (module != NULL)  // not in the browser
		{  
			module->onResetScale();  // make sure channels and scale notes outports are initialized for each frame, in case they have not been iniitialized
			
			module->theModeScaleProgressionsState.theHarmonyParms.STEP_inport_connected_to_ModeScaleProgressions_trigger_port=0;  // 0 will be interpreted elsewhere as "no connection", which may be overwritten below
	
			for (CableWidget* cwIn : APP->scene->rack->getCablesOnPort(inPortWidgets[ModeScaleProgressions::IN_PROG_STEP_EXT_CV]))
			{	
				if (!cwIn->isComplete())        // new for testing, from    vcvrack-packone strip.cpp                                              
                   continue;

				module->theModeScaleProgressionsState.theHarmonyParms.STEP_inport_connected_to_ModeScaleProgressions_trigger_port=1;  // 1 will be interpreted elsewhere as an unknown complete connection, which may be overwritten below
				
				for (CableWidget* cwOut : APP->scene->rack->getCablesOnPort(outPortWidgets[ModeScaleProgressions::OUT_CLOCK_BAR_OUTPUT]))
				{
					if (!cwOut->isComplete())        // new for testing, from    vcvrack-packone strip.cpp                                              
                   		continue;
					if (cwOut->cable->id == cwIn->cable->id)
					{
						module->theModeScaleProgressionsState.theHarmonyParms.STEP_inport_connected_to_ModeScaleProgressions_trigger_port=ModeScaleProgressions::OUT_CLOCK_BAR_OUTPUT;  
					}
										
				}
				for (CableWidget* cwOut : APP->scene->rack->getCablesOnPort(outPortWidgets[ModeScaleProgressions::OUT_CLOCK_BEAT_OUTPUT]))
				{
					if (!cwOut->isComplete())        // new for testing, from    vcvrack-packone strip.cpp                                              
                   		continue;
					if (cwOut->cable->id == cwIn->cable->id)
					{
						module->theModeScaleProgressionsState.theHarmonyParms.STEP_inport_connected_to_ModeScaleProgressions_trigger_port=ModeScaleProgressions::OUT_CLOCK_BEAT_OUTPUT;
					}
					
				}
				for (CableWidget* cwOut : APP->scene->rack->getCablesOnPort(outPortWidgets[ModeScaleProgressions::OUT_CLOCK_BEATX2_OUTPUT]))
				{
					if (!cwOut->isComplete())        // new for testing, from    vcvrack-packone strip.cpp                                              
                   		continue;
					if (cwOut->cable->id == cwIn->cable->id)
					{
						module->theModeScaleProgressionsState.theHarmonyParms.STEP_inport_connected_to_ModeScaleProgressions_trigger_port=ModeScaleProgressions::OUT_CLOCK_BEATX2_OUTPUT;
					}
					
				}
				for (CableWidget* cwOut : APP->scene->rack->getCablesOnPort(outPortWidgets[ModeScaleProgressions::OUT_CLOCK_BEATX4_OUTPUT]))
				{
					if (!cwOut->isComplete())        // new for testing, from    vcvrack-packone strip.cpp                                              
                   		continue;
					if (cwOut->cable->id == cwIn->cable->id)
					{
						module->theModeScaleProgressionsState.theHarmonyParms.STEP_inport_connected_to_ModeScaleProgressions_trigger_port=ModeScaleProgressions::OUT_CLOCK_BEATX4_OUTPUT;
					}
					
				}
				for (CableWidget* cwOut : APP->scene->rack->getCablesOnPort(outPortWidgets[ModeScaleProgressions::OUT_CLOCK_BEATX8_OUTPUT]))
				{
					if (!cwOut->isComplete())        // new for testing, from    vcvrack-packone strip.cpp                                              
                   		continue;
					if (cwOut->cable->id == cwIn->cable->id)
					{
						module->theModeScaleProgressionsState.theHarmonyParms.STEP_inport_connected_to_ModeScaleProgressions_trigger_port=ModeScaleProgressions::OUT_CLOCK_BEATX8_OUTPUT;  
					}
				}
			}
		}

		// root inport cable handling 
		if (module != NULL)  // not in the browser
		{
			for (CableWidget* cwIn : APP->scene->rack->getCablesOnPort(inPortWidgets[ModeScaleProgressions::IN_ROOT_KEY_EXT_CV]))  // look at each cable on the root key input port. There should be 0 or 1  cables on an input port.
			{
				if (!cwIn->isComplete())    // the cable connection has NOT been completed
				{        
				   module->theModeScaleProgressionsState.RootInputSuppliedByRootOutput=false;	
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
							if ((outputmodel->slug.substr(0, 21) == std::string("ModeScaleProgressions")) || 
							    (outputmodel->slug.substr(0, 14) == std::string("ModeScaleQuant")) ||
							    (outputmodel->slug.substr(0, 7) == std::string("Meander")))  
							{
							    if ((outputId==4)||(outputId==26)||(outputId==15))  // "cable outputID is OUT_EXT_ROOT_OUTPUT" kludge out of scope ModeScaleQuant variable access
								{
									module->theModeScaleProgressionsState.RootInputSuppliedByRootOutput=true;  // but may be made false based on cable input
								}
							}
							else // connected to moudule other than ModeScaleProgressions
							{
								module->theModeScaleProgressionsState.RootInputSuppliedByRootOutput=false;
							}
						}
						else  // no outputModel
						{
							module->theModeScaleProgressionsState.RootInputSuppliedByRootOutput=false;
						}
					}
					else  // no outputModule
					{
						module->theModeScaleProgressionsState.RootInputSuppliedByRootOutput=false;
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
										if (inputId==ModeScaleProgressions::IN_ROOT_KEY_EXT_CV)  // "cable inputID is  IN_ROOT_KEY_EXT_CV"
										{
										}
										else  // "cable inputID is not IN_ROOT_KEY_EXT_CV"
										{
											module->theModeScaleProgressionsState.RootInputSuppliedByRootOutput=false;
										}
									}
									else  // connected to moudule other than Maender
									{
										module->theModeScaleProgressionsState.RootInputSuppliedByRootOutput=false;
									}
							}
							else  // no input model
							{
								module->theModeScaleProgressionsState.RootInputSuppliedByRootOutput=false;
							}
						}
						else  // "cable inputModule is equal to cable outputModule"
						{
							module->theModeScaleProgressionsState.RootInputSuppliedByRootOutput=false;
						}
					}
					else  // no input module
					{
						module->theModeScaleProgressionsState.RootInputSuppliedByRootOutput=false;
					}
				}
				else  // there is no cable attached
				{
					module->theModeScaleProgressionsState.RootInputSuppliedByRootOutput=false;
				}
			} 
		}  

		// add mode inport cable handling 
		if (module != NULL)  // not in the browser
		{
			//"Examine module MODE cables--------------------"
			for (CableWidget* cwIn : APP->scene->rack->getCablesOnPort(inPortWidgets[ModeScaleProgressions::IN_SCALE_EXT_CV]))  // look at each cable on the mode scale input port. There should be 0 or 1  cables on an input port.
			{
				if (!cwIn->isComplete())   // the cable connection has NOT been completed    
				{        
				   module->theModeScaleProgressionsState.ModeInputSuppliedByModeOutput=false;	
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
									module->theModeScaleProgressionsState.ModeInputSuppliedByModeOutput=true;  // but may be made false based on cable input
								}
							}
							else // connected to moudule other than ModeScaleProgressions
							{
								module->theModeScaleProgressionsState.ModeInputSuppliedByModeOutput=false;
							}
						}
						else  // no outputModel
						{
							module->theModeScaleProgressionsState.ModeInputSuppliedByModeOutput=false;
						}
					}
					else  // no outputModule
					{
						module->theModeScaleProgressionsState.ModeInputSuppliedByModeOutput=false;
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
									if (inputId==ModeScaleProgressions::IN_SCALE_EXT_CV)  // "cable inputID is  IN_SCALE_EXT_CV"
									{
									}
									else // "cable inputID is not IN_SCALE_EXT_CV"
									{
										module->theModeScaleProgressionsState.ModeInputSuppliedByModeOutput=false;
									}
								}
								else  // connected to moudule other than Maender
								{
									module->theModeScaleProgressionsState.ModeInputSuppliedByModeOutput=false;
								}
							}
							else  // no input model
							{
								module->theModeScaleProgressionsState.ModeInputSuppliedByModeOutput=false;
							}
						}
						else  // "cable inputModule is equal to cable outputModule"
						{
							module->theModeScaleProgressionsState.ModeInputSuppliedByModeOutput=false;
						}
					}
					else  // no input module
					{
						module->theModeScaleProgressionsState.ModeInputSuppliedByModeOutput=false;
					}
				}
				else  // there is no cable attached
				{
					module->theModeScaleProgressionsState.ModeInputSuppliedByModeOutput=false;
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

};  // end struct ModeScaleProgressionsWidget



Model* modelModeScaleProgressions = createModel<ModeScaleProgressions, ModeScaleProgressionsWidget>("ModeScaleProgressions");     


