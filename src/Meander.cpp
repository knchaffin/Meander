/*  Copyright (C) 2019-2022 Ken Chaffin
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

#include "Meander.hpp"  


// the following is for performance profiling under Windows by the developer and should not be defined in the release distribution
// #define _USE_WINDOWS_PERFTIME

#if defined(_USE_WINDOWS_PERFTIME) 
#include <windows.h>
#endif

struct Meander : Module  
{
	// poly quant vars and functions
	
	bool polyQuant_scaleNotes[12];
	int polyQuant_searchSpaces[24];
	bool polyQuant_outputNotes[24];

	int MeanderScale[7]; // heptatonic scale

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
				for (int i=0;i<mode_step_intervals[mode][0];++i)
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
				for (int i=0;i<mode_step_intervals[mode][0];++i)
				{
					int note=(int)(notes[i]%(MAX_NOTES));  
					outputs[OUT_EXT_POLY_SCALE_OUTPUT].setVoltage((float)note/12.0,i);  // (scale note, channel) 
				}
			}
			
			if ((scale_out_mode == PENTATONIC_5CH)&&((mode==0)||(mode==1)||(mode==2)))   // major modes
			{
				outputs[OUT_EXT_POLY_SCALE_OUTPUT].setChannels(5);  // set polyphony to 5 channels
				int scale_note_index=0;
				for (int i=0;i<mode_step_intervals[mode][0];++i)
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
				for (int i=0;i<mode_step_intervals[mode][0];++i)
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
				for (int i=0;i<mode_step_intervals[mode][0];++i)
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
				for (int i=0;i<mode_step_intervals[mode][0];++i)
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
		// get Meander scale
		for (int i=0;((i<7)&&(i<mode_step_intervals[mode][0]));++i)
		{
			int note=(int)(notes[i]%12);  
			MeanderScale[i]=note;
		}

		for (int i = 0; i < 12; i++) // initialize clear scale notes
		{
			polyQuant_scaleNotes[i] = false;
		} 

		// get Meander heptatonic scale
		
		for (int i=0;i<7;++i)
		{
			int scale_note=MeanderScale[i];
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

		BUTTON_ENABLE_MELODY_PARAM,
		CONTROL_MELODY_VOLUME_PARAM,
		BUTTON_MELODY_DESTUTTER_PARAM,
		CONTROL_MELODY_NOTE_LENGTH_DIVISOR_PARAM,
		CONTROL_MELODY_TARGETOCTAVE_PARAM,
		CONTROL_MELODY_ALPHA_PARAM,
		CONTROL_MELODY_RANGE_PARAM,


		BUTTON_ENABLE_ARP_PARAM,
		CONTROL_ARP_COUNT_PARAM,
		CONTROL_ARP_INCREMENT_PARAM,
		CONTROL_ARP_DECAY_PARAM,
		CONTROL_ARP_PATTERN_PARAM,
		BUTTON_ENABLE_ARP_CHORDAL_PARAM,
		BUTTON_ENABLE_ARP_SCALER_PARAM,


		BUTTON_ENABLE_HARMONY_PARAM,
		BUTTON_HARMONY_DESTUTTER_PARAM,
		CONTROL_HARMONY_VOLUME_PARAM,
		CONTROL_HARMONY_STEPS_PARAM,
		CONTROL_HARMONY_TARGETOCTAVE_PARAM,
		CONTROL_HARMONY_ALPHA_PARAM,
		CONTROL_HARMONY_RANGE_PARAM,
		CONTROL_HARMONY_DIVISOR_PARAM,
		CONTROL_HARMONYPRESETS_PARAM,

		BUTTON_ENABLE_BASS_PARAM,
		CONTROL_BASS_VOLUME_PARAM,
		CONTROL_BASS_TARGETOCTAVE_PARAM,
		BUTTON_BASS_ACCENT_PARAM,
		BUTTON_BASS_SYNCOPATE_PARAM,
		BUTTON_BASS_SHUFFLE_PARAM,
		CONTROL_BASS_DIVISOR_PARAM,

				
		CONTROL_HARMONY_FBM_OCTAVES_PARAM,
		CONTROL_MELODY_FBM_OCTAVES_PARAM,
		CONTROL_ARP_FBM_OCTAVES_PARAM,

		CONTROL_HARMONY_FBM_PERIOD_PARAM,
		CONTROL_MELODY_FBM_PERIOD_PARAM,
		CONTROL_ARP_FBM_PERIOD_PARAM,

		BUTTON_ENABLE_MELODY_CHORDAL_PARAM,
		BUTTON_ENABLE_MELODY_SCALER_PARAM,

		BUTTON_ENABLE_HARMONY_ALL7THS_PARAM,
		BUTTON_ENABLE_HARMONY_V7THS_PARAM,

		BUTTON_ENABLE_HARMONY_STACCATO_PARAM,
		BUTTON_ENABLE_MELODY_STACCATO_PARAM,
		BUTTON_ENABLE_BASS_STACCATO_PARAM,

		BUTTON_BASS_OCTAVES_PARAM,

		BUTTON_PROG_STEP_PARAM,

		BUTTON_ENABLE_KEYBOARD_RENDER_PARAM,
		BUTTON_ENABLE_SCORE_RENDER_PARAM,
				
		
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

		
		IN_MELODY_ENABLE_EXT_CV,			
		IN_MELODY_VOLUME_EXT_CV,
		IN_MELODY_DESTUTTER_EXT_CV,
		IN_MELODY_NOTE_LENGTH_DIVISOR_EXT_CV,
		IN_MELODY_TARGETOCTAVE_EXT_CV,
		IN_MELODY_ALPHA_EXT_CV,
		IN_MELODY_RANGE_EXT_CV,

		IN_ARP_ENABLE_EXT_CV,
		IN_ARP_COUNT_EXT_CV,
		IN_ARP_INCREMENT_EXT_CV,
		IN_ARP_DECAY_EXT_CV,
		IN_ARP_PATTERN_EXT_CV,
		IN_ENABLE_ARP_CHORDAL_EXT_CV,
		IN_ENABLE_ARP_SCALER_EXT_CV,


		IN_HARMONY_ENABLE_EXT_CV,
		IN_HARMONY_DESTUTTER_EXT_CV,
		IN_HARMONY_VOLUME_EXT_CV,
		IN_HARMONY_STEPS_EXT_CV,
		IN_HARMONY_TARGETOCTAVE_EXT_CV,
		IN_HARMONY_ALPHA_EXT_CV,
		IN_HARMONY_RANGE_EXT_CV,
		IN_HARMONY_DIVISOR_EXT_CV,
		IN_HARMONYPRESETS_EXT_CV,

		IN_BASS_ENABLE_EXT_CV,
		IN_BASS_VOLUME_EXT_CV,
		IN_BASS_TARGETOCTAVE_EXT_CV,
		IN_BASS_ACCENT_EXT_CV,
		IN_BASS_SYNCOPATE_EXT_CV,
		IN_BASS_SHUFFLE_EXT_CV,
		IN_BASS_DIVISOR_EXT_CV,

				
		IN_HARMONY_FBM_OCTAVES_EXT_CV,
		IN_MELODY_FBM_OCTAVES_EXT_CV,
		IN_ARP_FBM_OCTAVES_EXT_CV,

		IN_HARMONY_FBM_PERIOD_EXT_CV,
		IN_MELODY_FBM_PERIOD_EXT_CV,
		IN_ARP_FBM_PERIOD_EXT_CV,

		IN_ENABLE_MELODY_CHORDAL_EXT_CV,
		IN_MELODY_SCALER_EXT_CV,

		IN_ENABLE_HARMONY_ALL7THS_EXT_CV,
		IN_ENABLE_HARMONY_V7THS_EXT_CV,

		IN_ENABLE_HARMONY_STACCATO_EXT_CV,
		IN_ENABLE_MELODY_STACCATO_EXT_CV,
		IN_ENABLE_BASS_STACCATO_EXT_CV,

		IN_BASS_OCTAVES_EXT_CV,

		IN_PROG_STEP_EXT_CV,

		IN_MELODY_SCALE_DEGREE_EXT_CV,
		IN_MELODY_SCALE_GATE_EXT_CV,

		IN_POLY_QUANT_EXT_CV,
		
		NUM_INPUTS
		
	};
	
	

	enum OutputIds 
	{
		OUT_RUN_OUT,
		OUT_RESET_OUT,
		OUT_TEMPO_OUT,
		OUT_CLOCK_OUT, 
		OUT_MELODY_GATE_OUTPUT,
		OUT_HARMONY_GATE_OUTPUT,
		OUT_BASS_GATE_OUTPUT,
		OUT_FBM_HARMONY_OUTPUT,
		OUT_MELODY_CV_OUTPUT,
		OUT_FBM_MELODY_OUTPUT,
		OUT_BASS_CV_OUTPUT,
		OUT_HARMONY_CV_OUTPUT,
		OUT_CLOCK_BEATX2_OUTPUT,
		OUT_CLOCK_BAR_OUTPUT,
		OUT_CLOCK_BEATX4_OUTPUT,
		OUT_CLOCK_BEATX8_OUTPUT,
		OUT_CLOCK_BEAT_OUTPUT,
		OUT_BASS_TRIGGER_OUTPUT,
		OUT_HARMONY_TRIGGER_OUTPUT,
		OUT_MELODY_TRIGGER_OUTPUT,
		OUT_FBM_ARP_OUTPUT,
		OUT_MELODY_VOLUME_OUTPUT,
		OUT_HARMONY_VOLUME_OUTPUT,
		OUT_BASS_VOLUME_OUTPUT,
		OUT_EXT_POLY_SCALE_OUTPUT,
		OUT_EXT_POLY_QUANT_OUTPUT,

		OUT_EXT_ROOT_OUTPUT,
	
		NUM_OUTPUTS
	};

	enum LightIds 
	{
		LIGHT_LEDBUTTON_HARMONY_ENABLE,
		LIGHT_LEDBUTTON_MELODY_ENABLE,
		LIGHT_LEDBUTTON_ARP_ENABLE,
		LIGHT_LEDBUTTON_ARP_ENABLE_CHORDAL,
		LIGHT_LEDBUTTON_ARP_ENABLE_SCALER,
		LIGHT_LEDBUTTON_BASS_ENABLE,
		LIGHT_LEDBUTTON_RUN,
		LIGHT_LEDBUTTON_RESET,
		LIGHT_LEDBUTTON_MELODY_DESTUTTER,
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
		LIGHT_LEDBUTTON_MELODY_ENABLE_CHORDAL,
		LIGHT_LEDBUTTON_MELODY_ENABLE_SCALER,

		LIGHT_LEDBUTTON_ENABLE_HARMONY_ALL7THS_PARAM,
		LIGHT_LEDBUTTON_ENABLE_HARMONY_V7THS_PARAM,
		LIGHT_LEDBUTTON_BASS_SYNCOPATE_PARAM,
		LIGHT_LEDBUTTON_BASS_ACCENT_PARAM,
		LIGHT_LEDBUTTON_BASS_SHUFFLE_PARAM,

		LIGHT_LEDBUTTON_ENABLE_HARMONY_STACCATO_PARAM,
		LIGHT_LEDBUTTON_ENABLE_MELODY_STACCATO_PARAM,
		LIGHT_LEDBUTTON_ENABLE_BASS_STACCATO_PARAM,

		LIGHT_LEDBUTTON_BASS_OCTAVES_PARAM,

		LIGHT_LEDBUTTON_PROG_STEP_PARAM,

		LIGHT_LEDBUTTON_ENABLE_RENDER_KEYBOARD_PARAM,
		LIGHT_LEDBUTTON_ENABLE_RENDER_SCORE_PARAM,
		
		NUM_LIGHTS
	};

	#include "data_and_functions.hpp" // for module vars 

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

	// Clock code adapted from Strum and AS

	struct LFOGenerator 
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
	};  // struct LFOGenerator 

	void userPlaysCirclePositionHarmony(int circle_position, float octaveOffset)  // C=0   play immediate
	{
		theMeanderState.last_harmony_chord_root_note=circle_of_fifths[circle_position];

		if (octaveOffset>9)
			octaveOffset=9;

		for (int i=0; i<7; ++i) // melody and bass will use this to accompany 
		{
			if  (theCircleOf5ths.theDegreeSemiCircle.degreeElements[i].CircleIndex==circle_position)
			{
				int theDegree=theCircleOf5ths.theDegreeSemiCircle.degreeElements[i].Degree;
				if ((theDegree<1)||(theDegree>7))
			     	  theDegree=1;  // force to a valid degree to avoid a crash
				current_circle_degree = theDegree;
				for (int j=0;j<MAX_STEPS;++j)
				{
					if (theHarmonyTypes[harmony_type].harmony_steps[j]==theDegree)
					{
						theMeanderState.last_harmony_step=j;
						break;
					}
				}
				break;
			} 
		} 

		theMeanderState.theMelodyParms.last_step=theMeanderState.last_harmony_step;
		int note_index=	(int)(theMeanderState.theMelodyParms.note_avg*num_step_chord_notes[theMeanderState.last_harmony_step]);		// not sure this is necessary
		note_index=clamp(note_index, 0, num_step_chord_notes[theMeanderState.last_harmony_step]-1);
		theMeanderState.theMelodyParms.last_chord_note_index= note_index;
		 
		int current_chord_note=0;
		int root_key_note=circle_of_fifths[circle_position]; 
		int circle_chord_type= theCircleOf5ths.Circle5ths[circle_position].chordType;
		theMeanderState.theHarmonyParms.last_chord_type=circle_chord_type;
		int num_chord_members=chord_type_num_notes[circle_chord_type];
		
		if (((theMeanderState.theHarmonyParms.enable_all_7ths)||(theMeanderState.theHarmonyParms.enable_V_7ths))			
			&& ((circle_chord_type==2)
			||  (circle_chord_type==3)
			||  (circle_chord_type==4)
			||  (circle_chord_type==5)))
				outputs[OUT_HARMONY_CV_OUTPUT].setChannels(4);  // set polyphony
			else
				outputs[OUT_HARMONY_CV_OUTPUT].setChannels(3);  // set polyphony

		for (int j=0;j<num_chord_members;++j) 
		{
			current_chord_note=(int)((int)root_key_note+(int)chord_type_intervals[circle_chord_type][j]);
			int note_to_play=current_chord_note+(octaveOffset*12);
			note_to_play=clamp(note_to_play, root_key, 108+root_key); // clamp to MIDI range root0 to (C8+root)
			outputs[OUT_HARMONY_CV_OUTPUT].setVoltage((note_to_play/12.0)-4.0,j);  // (note, channel) 
					
			if (j<4)
			{
				theMeanderState.theHarmonyParms.last[j].note=note_to_play;
				theMeanderState.theHarmonyParms.last[j].noteType=NOTE_TYPE_CHORD;
				theMeanderState.theHarmonyParms.last[j].length=theMeanderState.theHarmonyParms.note_length_divisor;  
				theMeanderState.theHarmonyParms.last[j].time32s=barts_count;
				theMeanderState.theHarmonyParms.last[j].countInBar=bar_note_count;
				theMeanderState.theHarmonyParms.last[j].isPlaying=true;
				if (bar_note_count<256)
				played_notes_circular_buffer[bar_note_count++]=theMeanderState.theHarmonyParms.last[j];
				
			}
            
		}
		float durationFactor=1.0;
		if (theMeanderState.theHarmonyParms.enable_staccato)
			durationFactor=0.5;
		else
			durationFactor=1.0;
		
		float note_duration=durationFactor*time_sig_top/(frequency*theMeanderState.theHarmonyParms.note_length_divisor);
		harmonyGatePulse.reset();  // kill the pulse in case it is active
		harmonyGatePulse.trigger(note_duration);  
	}

	void doHarmony(int barChordNumber=1, bool playFlag=false)
	{
		outputs[OUT_HARMONY_VOLUME_OUTPUT].setVoltage(theMeanderState.theHarmonyParms.volume);
		
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
				step=theMeanderState.theHarmonyParms.last_circle_step;
			}
		}
		else
		if (harmony_type==22) // random coming home
		{
			if (barChordNumber!=0)
			{
				step=theMeanderState.theHarmonyParms.last_circle_step;
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
				step=theMeanderState.theHarmonyParms.last_circle_step;
			}
		}
		else
		if ((harmony_type==31)||(harmony_type==42)||(harmony_type==43)||(harmony_type==44)||(harmony_type==45)||(harmony_type==46)||(harmony_type==47)||(harmony_type==48))  // Markov chains
		{   
			if (barChordNumber==0)
			{
				float rnd = rack::random::uniform();
				
				if (theMeanderState.theHarmonyParms.last_circle_step==-1)
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
							probabilityTargetTop[i]=bottom+MarkovProgressionTransitionMatrixBach1[theMeanderState.theHarmonyParms.last_circle_step+1][i];
						else
						if (harmony_type==42)
							probabilityTargetTop[i]=bottom+MarkovProgressionTransitionMatrixBach2[theMeanderState.theHarmonyParms.last_circle_step+1][i];
						else
						if (harmony_type==43)
							probabilityTargetTop[i]=bottom+MarkovProgressionTransitionMatrixMozart1[theMeanderState.theHarmonyParms.last_circle_step+1][i];
						else
						if (harmony_type==44)
							probabilityTargetTop[i]=bottom+MarkovProgressionTransitionMatrixMozart2[theMeanderState.theHarmonyParms.last_circle_step+1][i];
						else
						if (harmony_type==45)
							probabilityTargetTop[i]=bottom+MarkovProgressionTransitionMatrixPalestrina1[theMeanderState.theHarmonyParms.last_circle_step+1][i];
						else
						if (harmony_type==46)
							probabilityTargetTop[i]=bottom+MarkovProgressionTransitionMatrixBeethoven1[theMeanderState.theHarmonyParms.last_circle_step+1][i];
						else
						if (harmony_type==47)
							probabilityTargetTop[i]=bottom+MarkovProgressionTransitionMatrixTraditional1[theMeanderState.theHarmonyParms.last_circle_step+1][i];
						else
						if (harmony_type==48)
							probabilityTargetTop[i]=bottom+MarkovProgressionTransitionMatrix_I_IV_V[theMeanderState.theHarmonyParms.last_circle_step+1][i];
											
						
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
				step=theMeanderState.theHarmonyParms.last_circle_step;
			}
			
		}
		
    			

		int degreeStep=(theActiveHarmonyType.harmony_steps[step])%8; 
		if ((degreeStep<1)||(degreeStep>7))
		  degreeStep=1;  // force to a valid degree to avoid a crash
		current_circle_degree = degreeStep;
			
		theMeanderState.theHarmonyParms.last_circle_step=step;  // used for Markov chain

		//find this in semicircle
		for (int i=0; i<7; ++i)
		{
			if  (theCircleOf5ths.theDegreeSemiCircle.degreeElements[i].Degree==degreeStep)
			{
				current_circle_position = theCircleOf5ths.theDegreeSemiCircle.degreeElements[i].CircleIndex; 
				break;
			}
		}
				
		lights[LIGHT_LEDBUTTON_CIRCLESETSTEP_1+step].setBrightness(1.0f);
		lights[LIGHT_LEDBUTTON_CIRCLESTEP_1+ (current_circle_position)%12].setBrightness(1.0f);
		
		double period=1.0/theMeanderState.theHarmonyParms.period; // 1/seconds
		double fBmarg=theMeanderState.theHarmonyParms.seed + (double)(period*current_cpu_time_double); 
	    double fBmrand=(FastfBm1DNoise(fBmarg,theMeanderState.theHarmonyParms.noctaves) +1.)/2; 
			
		theMeanderState.theHarmonyParms.note_avg = 
			(1.0-theMeanderState.theHarmonyParms.alpha)*theMeanderState.theHarmonyParms.note_avg + 
			theMeanderState.theHarmonyParms.alpha*(theMeanderState.theHarmonyParms.range_bottom + (fBmrand*theMeanderState.theHarmonyParms.r1));
					
		if (theMeanderState.theHarmonyParms.note_avg>theMeanderState.theHarmonyParms.range_top)
		theMeanderState.theHarmonyParms.note_avg=theMeanderState.theHarmonyParms.range_top;
		if (theMeanderState.theHarmonyParms.note_avg<theMeanderState.theHarmonyParms.range_bottom)
		theMeanderState.theHarmonyParms.note_avg=theMeanderState.theHarmonyParms.range_bottom;
					
		int step_chord_type= theCircleOf5ths.Circle5ths[current_circle_position].chordType;
		
		if (((theMeanderState.theHarmonyParms.enable_all_7ths)||(theMeanderState.theHarmonyParms.enable_V_7ths))			
		&& ((theCircleOf5ths.Circle5ths[current_circle_position].chordType==2)
		||  (theCircleOf5ths.Circle5ths[current_circle_position].chordType==3)
		||  (theCircleOf5ths.Circle5ths[current_circle_position].chordType==4)
		||  (theCircleOf5ths.Circle5ths[current_circle_position].chordType==5)))
			outputs[OUT_HARMONY_CV_OUTPUT].setChannels(4);  // set polyphony
		else
			outputs[OUT_HARMONY_CV_OUTPUT].setChannels(3);  // set polyphony
		
		int num_chord_members=chord_type_num_notes[step_chord_type]; 
		
		theMeanderState.theHarmonyParms.last_chord_type=step_chord_type;
		theMeanderState.last_harmony_chord_root_note=circle_of_fifths[current_circle_position];
		theMeanderState.last_harmony_step=step;
					
		bool tonicFound=false;
		for (int j=0;j<num_chord_members;++j) 
		{
				current_chord_notes[j]= step_chord_notes[step][(int)(theMeanderState.theHarmonyParms.note_avg*num_step_chord_notes[step])+j]; // may create inversion
				int note_to_play=current_chord_notes[j]-12;  // drop it an octave to get target octave right
				note_to_play=clamp(note_to_play, root_key, 108+root_key); // clamp to MIDI range root0 to (C8+root)
											
				if (playFlag)  
				{
					if (j<4)
					{
						theMeanderState.theHarmonyParms.last[j].note=note_to_play;
						theMeanderState.theHarmonyParms.last[j].noteType=NOTE_TYPE_CHORD;
					//	theMeanderState.theHarmonyParms.last[j].length=1;  // need chords per measure
						theMeanderState.theHarmonyParms.last[j].length=theMeanderState.theHarmonyParms.note_length_divisor;
						theMeanderState.theHarmonyParms.last[j].time32s=barts_count;
						theMeanderState.theHarmonyParms.last[j].countInBar=bar_note_count;
						theMeanderState.theHarmonyParms.last[j].isPlaying=true;
						if (bar_note_count<256)
						played_notes_circular_buffer[bar_note_count++]=theMeanderState.theHarmonyParms.last[j];
					}
				
					//  get tonic in channel 0
											
					if ( (note_to_play%MAX_NOTES)==(theMeanderState.last_harmony_chord_root_note%MAX_NOTES))
					{
						outputs[OUT_HARMONY_CV_OUTPUT].setVoltage((note_to_play/12.0)-4.0,0);  // (note, channel)
						tonicFound=true;
					}
					else
					{
						if (!tonicFound)
							outputs[OUT_HARMONY_CV_OUTPUT].setVoltage((note_to_play/12.0)-4.0,j+1);  // (note, channel)
						else
							outputs[OUT_HARMONY_CV_OUTPUT].setVoltage((note_to_play/12.0)-4.0,j);  // (note, channel)
					}
					
				
				}
		}
		
		if (playFlag)
		{ 
			outputs[OUT_HARMONY_VOLUME_OUTPUT].setVoltage(theMeanderState.theHarmonyParms.volume);

			// output some fBm noise
			outputs[OUT_FBM_HARMONY_OUTPUT].setChannels(1);  // set polyphony  
		
			float durationFactor=1.0;
			if (theMeanderState.theHarmonyParms.enable_staccato)
				durationFactor=0.5;
			else
				durationFactor=0.95;

					
			if (theMeanderState.theHarmonyParms.STEP_inport_connected_to_Meander_trigger_port==OUT_CLOCK_BAR_OUTPUT)
				durationFactor*=1.0;
			else	
			if (theMeanderState.theHarmonyParms.STEP_inport_connected_to_Meander_trigger_port==OUT_CLOCK_BEAT_OUTPUT)
				durationFactor*=.25;
			else	
			if (theMeanderState.theHarmonyParms.STEP_inport_connected_to_Meander_trigger_port==OUT_CLOCK_BEATX2_OUTPUT)
				durationFactor*=.125;
			else	
			if (theMeanderState.theHarmonyParms.STEP_inport_connected_to_Meander_trigger_port==OUT_CLOCK_BEATX4_OUTPUT)
				durationFactor*=.0625;	
			else
			if (theMeanderState.theHarmonyParms.STEP_inport_connected_to_Meander_trigger_port==OUT_CLOCK_BEATX8_OUTPUT)
				durationFactor*=.03125;	
			else
			if ( inputs[IN_PROG_STEP_EXT_CV].isConnected()) // something is connected to the circle STEP input but we do not know what. Assume it is an 16X BPM frequency
		  		durationFactor *= .01562;  
			
						
			float note_duration=durationFactor*4/(frequency*theMeanderState.theHarmonyParms.note_length_divisor);

			theMeanderState.theHarmonyParms.last_chord_playing=true;

			harmonyGatePulse.reset();  // kill the pulse in case it is active
			harmonyGatePulse.trigger(note_duration);  
		}

		outputs[OUT_FBM_HARMONY_OUTPUT].setVoltage((float)clamp((10.f*fBmrand), 0.f, 10.f) ,0); // rescale fBm output to 0-10V so it can be used better for CV.  Output even if harmony disabled

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

	void userPlaysScaleDegreeMelody(int degree, float octaveOffset)  // C=0   play immediate
	{
		if (degree<1)
			degree=1;
		if (degree>7)
			degree=7;

		

		int	note_to_play=root_key_notes[root_key][degree-1]+(octaveOffset*12); 
		note_to_play=clamp(note_to_play, root_key, 108+root_key); // clamp to MIDI range root0 to (C8+root)
		outputs[OUT_MELODY_CV_OUTPUT].setVoltage((note_to_play/12.0)-4.0,0);  // (note, channel)  shift down 1 ocatve/v
					
		theMeanderState.theMelodyParms.last[0].note=note_to_play;
		theMeanderState.theMelodyParms.last[0].noteType=NOTE_TYPE_MELODY;
		theMeanderState.theMelodyParms.last[0].length=1;  // whole  note for now
		theMeanderState.theMelodyParms.last[0].time32s=barts_count;
		theMeanderState.theMelodyParms.last[0].countInBar=bar_note_count;
		theMeanderState.theMelodyParms.last[0].isPlaying=true;

		if (bar_note_count<256)
		played_notes_circular_buffer[bar_note_count++]=theMeanderState.theMelodyParms.last[0];
			
		float durationFactor=1.0;
		if (theMeanderState.theMelodyParms.enable_staccato)
			durationFactor=0.5;
		else
			durationFactor=1.0;
		
		float note_duration=durationFactor*time_sig_top/(frequency*theMeanderState.theMelodyParms.note_length_divisor);
		melodyGatePulse.reset();  // kill the pulse in case it is active
		melodyGatePulse.trigger(note_duration);  
	}

	void doMelody()
	{
		
		outputs[OUT_MELODY_VOLUME_OUTPUT].setVoltage(theMeanderState.theMelodyParms.volume);
		clock_t current_cpu_t= clock();  // cpu clock ticks since program began
		double current_cpu_time_double= (double)(current_cpu_t) / (double)CLOCKS_PER_SEC;
	
		++theMeanderState.theMelodyParms.bar_melody_counted_note;

		theMeanderState.theArpParms.note_count=0;  // where does this really go, at the begining of a melody note
	
		double period=1.0/theMeanderState.theMelodyParms.period; // 1/seconds
		double fBmarg=theMeanderState.theMelodyParms.seed + (double)(period*current_cpu_time_double); 
		double fBmrand=(FastfBm1DNoise(fBmarg,theMeanderState.theMelodyParms.noctaves) +1.)/2; 
			
		theMeanderState.theMelodyParms.note_avg = 
			(1.0-theMeanderState.theMelodyParms.alpha)*theMeanderState.theMelodyParms.note_avg + 
			theMeanderState.theMelodyParms.alpha*(theMeanderState.theMelodyParms.range_bottom + (fBmrand*theMeanderState.theMelodyParms.r1));
					
		if (theMeanderState.theMelodyParms.note_avg>theMeanderState.theMelodyParms.range_top)
		theMeanderState.theMelodyParms.note_avg=theMeanderState.theMelodyParms.range_top;
		if (theMeanderState.theMelodyParms.note_avg<theMeanderState.theMelodyParms.range_bottom)
		theMeanderState.theMelodyParms.note_avg=theMeanderState.theMelodyParms.range_bottom;
		
		int step=theMeanderState.last_harmony_step;	
		theMeanderState.theMelodyParms.last_step= step;
		int note_index=	(int)(theMeanderState.theMelodyParms.note_avg*num_step_chord_notes[step]);	
		note_index=clamp(note_index, 0, num_step_chord_notes[step]-1);
		theMeanderState.theMelodyParms.last_chord_note_index= note_index;
		int note_to_play=step_chord_notes[step][note_index]; 
		note_to_play=clamp(note_to_play, root_key, 108+root_key); // clamp to MIDI range root0 to (C8+root)

		if (theMeanderState.theMelodyParms.chordal)
		{
			note_index=	(int)(theMeanderState.theMelodyParms.note_avg*num_step_chord_notes[step]);	
			note_index=clamp(note_index, 0, num_step_chord_notes[step]-1);
			note_to_play=step_chord_notes[step][note_index]; 
			note_to_play=clamp(note_to_play, root_key, 108+root_key); // clamp to MIDI range root0 to (C8+root)
	
		}
		else
		if (theMeanderState.theMelodyParms.scaler)
		{
			note_index=	(int)(theMeanderState.theMelodyParms.note_avg*num_root_key_notes[root_key]);
			note_index=clamp(note_index, 0, num_root_key_notes[root_key]-1);
			note_to_play=root_key_notes[root_key][note_index]; 
			note_to_play=clamp(note_to_play, root_key, 108+root_key); // clamp to MIDI range root0 to (C8+root)
		}

		
		if (true)	// do it even if melody notes will not be played, so arp will have roots
		{   
			if ((theMeanderState.theMelodyParms.destutter) && (note_to_play==theMeanderState.theMelodyParms.last[0].note) && (theMeanderState.theMelodyParms.last_stutter_step==step))   // seems like gate always fires
			{
				theMeanderState.theMelodyParms.stutter_detected=true;
				theMeanderState.theMelodyParms.last_stutter_step=step;
			}
			else
			{
				theMeanderState.theMelodyParms.last_stutter_step=step;
				theMeanderState.theMelodyParms.stutter_detected=false;
										
				theMeanderState.theMelodyParms.last[0].note=note_to_play;
				theMeanderState.theMelodyParms.last[0].noteType=NOTE_TYPE_MELODY;
				theMeanderState.theMelodyParms.last[0].length=theMeanderState.theMelodyParms.note_length_divisor;
				theMeanderState.theMelodyParms.last[0].time32s=barts_count;
				theMeanderState.theMelodyParms.last[0].countInBar=bar_note_count;
				theMeanderState.theMelodyParms.last[0].isPlaying=true;

				if ((theMeanderState.theMelodyParms.enabled)&&(bar_note_count<256))
					played_notes_circular_buffer[bar_note_count++]=theMeanderState.theMelodyParms.last[0];
			

				if (theMeanderState.theMelodyParms.enabled)
				{						
					outputs[OUT_MELODY_CV_OUTPUT].setChannels(1);  // set polyphony  may need to deal with unset channel voltages
					outputs[OUT_MELODY_CV_OUTPUT].setVoltage(((note_to_play/12.0) -4.0) ,0);  // (note, channel) -4 since midC=c4=0voutputs[OUT_MELODY_CV_OUTPUT].setVoltage((note_to_play -4 + theMeanderState.theMelodyParms.target_octave,0);  // (note, channel) -4 since midC=c4=0v
		    	}

				// output some fBm noise
				outputs[OUT_FBM_MELODY_OUTPUT].setChannels(1);  // set polyphony  
				outputs[OUT_FBM_MELODY_OUTPUT].setVoltage((float)clamp((10.f*fBmrand), 0.f, 10.f) ,0); // rescale fBm output to 0-10V so it can be used better for CV

				float durationFactor=1.0;
				if (theMeanderState.theMelodyParms.enable_staccato)
					durationFactor=0.5;
				else
					durationFactor=0.95;
				float note_duration=durationFactor*4/(frequency*theMeanderState.theMelodyParms.note_length_divisor);

				// adjust melody note duration if arp enabled
				
				if (theMeanderState.theArpParms.enabled)
					note_duration=durationFactor*4/(frequency*theMeanderState.theArpParms.note_length_divisor);
				

				if (theMeanderState.theMelodyParms.enabled)
				melodyGatePulse.trigger(note_duration);  // Test 1s duration  need to use .process to detect this and then send it to output
			}
		}
	}

	void doArp() 
	{
		if (theMeanderState.theArpParms.note_count>=theMeanderState.theArpParms.count)
	  		return;

		int arp_note=0;

		
		if ((theMeanderState.theArpParms.pattern>=-1)&&(theMeanderState.theArpParms.pattern<=1))
			arp_note=theMeanderState.theArpParms.note_count*theMeanderState.theArpParms.pattern;
		else
		if (theMeanderState.theArpParms.pattern==2)
		{
			if (theMeanderState.theArpParms.note_count<=((theMeanderState.theArpParms.count)/2))
			arp_note=theMeanderState.theArpParms.note_count;
			else
			arp_note=theMeanderState.theArpParms.count-theMeanderState.theArpParms.note_count-1;
		}
		else
		if (theMeanderState.theArpParms.pattern==-2)
		{
			if (theMeanderState.theArpParms.note_count<=((theMeanderState.theArpParms.count)/2))
			arp_note-=theMeanderState.theArpParms.note_count;
			else
			arp_note=-theMeanderState.theArpParms.count+theMeanderState.theArpParms.note_count-1;
		}
		else
			arp_note=theMeanderState.theArpParms.note_count*theMeanderState.theArpParms.pattern;

		if (theMeanderState.theArpParms.pattern!=0)
		++arp_note; // above the melody note

		++theMeanderState.theArpParms.note_count;

		float volume=theMeanderState.theMelodyParms.volume;
		float volume_factor=pow((1.0-theMeanderState.theArpParms.decay), theMeanderState.theArpParms.note_count);
		volume *= volume_factor;
		
		int note_to_play=100; // bogus

		if (theMeanderState.theArpParms.chordal) // use step_chord_notes
		{
           note_to_play=step_chord_notes[theMeanderState.theMelodyParms.last_step][(theMeanderState.theMelodyParms.last_chord_note_index + arp_note)% num_step_chord_notes[theMeanderState.theMelodyParms.last_step]];
		   note_to_play=clamp(note_to_play, root_key, 108+root_key); // clamp to MIDI range root0 to (C8+root)
		}
		else 
		if (theMeanderState.theArpParms.scaler) // use root_key_notes rather than step_chord_notes.  This is slower since scale note index has to be looked up
		{   
					
			if (true)  // new // BSP search  .  
			{
				int note_to_search_for=theMeanderState.theMelodyParms.last[0].note;
				int num_to_search=num_root_key_notes[root_key];
				int start_search_index=0;
				int end_search_index=num_root_key_notes[root_key]-1;
				int pass=0;
				int partition_index=0;
				while (pass<8)
				{
					partition_index=(end_search_index+start_search_index)/2;
					if ( note_to_search_for>root_key_notes[root_key][partition_index])
					{
						start_search_index=partition_index;
					}
					else
					if ( note_to_search_for<root_key_notes[root_key][partition_index])
					{
						end_search_index=partition_index;
					}
					else
					{
						/* we found it */
						pass=8;
						break;
					}
					++pass;
				}
				if ((partition_index>=0) && (partition_index<num_to_search))
				{
					note_to_play=root_key_notes[root_key][partition_index+arp_note];
					note_to_play=clamp(note_to_play, root_key, 108+root_key); // clamp to MIDI range root0 to (C8+root)
				}
							
			}
			
		}
		
		 
	
		if (((theMeanderState.theMelodyParms.enabled)||(theMeanderState.theArpParms.enabled))&&theMeanderState.theArpParms.note_count<32)
		{
			theMeanderState.theArpParms.last[theMeanderState.theArpParms.note_count].note=note_to_play;
			theMeanderState.theArpParms.last[theMeanderState.theArpParms.note_count].noteType=NOTE_TYPE_ARP;
			theMeanderState.theArpParms.last[theMeanderState.theArpParms.note_count].length=theMeanderState.theArpParms.note_length_divisor;
			theMeanderState.theArpParms.last[theMeanderState.theArpParms.note_count].time32s=barts_count;
			theMeanderState.theArpParms.last[theMeanderState.theArpParms.note_count].countInBar=bar_note_count;
			theMeanderState.theArpParms.last[theMeanderState.theArpParms.note_count].isPlaying=true;
			if (bar_note_count<256)
			played_notes_circular_buffer[bar_note_count++]=theMeanderState.theArpParms.last[theMeanderState.theArpParms.note_count];
		}
	
		outputs[OUT_MELODY_CV_OUTPUT].setChannels(1);  // set polyphony  may need to deal with unset channel voltages
		outputs[OUT_MELODY_CV_OUTPUT].setVoltage(((note_to_play/12.0) -4.0) ,0);  // (note, channel) -4 since midC=c4=0voutputs[OUT_MELODY_CV_OUTPUT].setVoltage((note_to_play -4 + theMeanderState.theMelodyParms.target_octave,0);  // (note, channel) -4 since midC=c4=0v
		outputs[OUT_MELODY_VOLUME_OUTPUT].setVoltage(volume);  // this is strictly for arp decay
		melodyGatePulse.reset();  // kill the pulse in case it is active
		float durationFactor=1.0;
		if (theMeanderState.theMelodyParms.enable_staccato)
			durationFactor=0.5;
		else
			durationFactor=0.95;
		float note_duration=durationFactor*4/(frequency*theMeanderState.theArpParms.note_length_divisor);
		melodyGatePulse.trigger(note_duration);  
	}
 

	void doBass()
	{
	
	    outputs[OUT_BASS_VOLUME_OUTPUT].setVoltage(theMeanderState.theBassParms.volume);
				
		if (theMeanderState.theBassParms.enabled) 
		{
			++theMeanderState.theBassParms.bar_bass_counted_note;
			if ((theMeanderState.theBassParms.syncopate)&&(theMeanderState.theBassParms.bar_bass_counted_note==2))  // experimenting with bass patterns
			  return;
			if ((theMeanderState.theBassParms.shuffle)&&((theMeanderState.theBassParms.bar_bass_counted_note%3)==2))  // experimenting with bass patterns
			return;

			if (theMeanderState.theBassParms.octave_enabled)
				outputs[OUT_BASS_CV_OUTPUT].setChannels(2);  // set polyphony  may need to deal with unset channel voltages
			else
				outputs[OUT_BASS_CV_OUTPUT].setChannels(1);  // set polyphony  may need to deal with unset channel voltages
			
			theMeanderState.theBassParms.last[0].note=theMeanderState.last_harmony_chord_root_note+ (theMeanderState.theBassParms.target_octave*12);  
			theMeanderState.theBassParms.last[0].noteType=NOTE_TYPE_BASS;
		//	theMeanderState.theBassParms.last[0].length=1;  // need bass notes per measure
			theMeanderState.theBassParms.last[0].length=theMeanderState.theBassParms.note_length_divisor;
			theMeanderState.theBassParms.last[0].time32s=barts_count;
			theMeanderState.theBassParms.last[0].countInBar=bar_note_count;
			theMeanderState.theBassParms.last[0].isPlaying=true;
			if (bar_note_count<256)
			played_notes_circular_buffer[bar_note_count++]=theMeanderState.theBassParms.last[0];

			outputs[OUT_BASS_CV_OUTPUT].setVoltage((theMeanderState.last_harmony_chord_root_note/12.0)-4.0 +theMeanderState.theBassParms.target_octave ,0);  //(note, channel)	
				
			if (theMeanderState.theBassParms.octave_enabled)
			{
		
				theMeanderState.theBassParms.last[1].note=theMeanderState.theBassParms.last[0].note+12; 
				theMeanderState.theBassParms.last[1].noteType=NOTE_TYPE_BASS;
			//	theMeanderState.theBassParms.last[1].length=1;  // need bass notes per measure
				theMeanderState.theBassParms.last[1].length=theMeanderState.theBassParms.note_length_divisor;
				theMeanderState.theBassParms.last[1].time32s=barts_count;
				theMeanderState.theBassParms.last[1].countInBar=bar_note_count;
				theMeanderState.theBassParms.last[1].isPlaying=true;
				if (bar_note_count<256)
				played_notes_circular_buffer[bar_note_count++]=theMeanderState.theBassParms.last[1];

			 	outputs[OUT_BASS_CV_OUTPUT].setVoltage((theMeanderState.last_harmony_chord_root_note/12.0)-3.0 +theMeanderState.theBassParms.target_octave ,1);
			}

			if (!theMeanderState.theBassParms.accent)  // need to redo with new gate procedure
			{
				theMeanderState.theBassParms.note_accented=false; 
			}
			else
			{
				if (theMeanderState.theBassParms.bar_bass_counted_note==1)  // experimenting with bass patterns
					theMeanderState.theBassParms.note_accented=true; 
				else
					theMeanderState.theBassParms.note_accented=false; 
			}	
			
			float durationFactor=1.0;
			if (theMeanderState.theBassParms.enable_staccato)
				durationFactor=0.5;
			else
				durationFactor=0.95;
			
			if (theMeanderState.theHarmonyParms.STEP_inport_connected_to_Meander_trigger_port==OUT_CLOCK_BAR_OUTPUT)
				durationFactor*=1.0;
			else	
			if (theMeanderState.theHarmonyParms.STEP_inport_connected_to_Meander_trigger_port==OUT_CLOCK_BEAT_OUTPUT)
				durationFactor*=.25;
			else	
			if (theMeanderState.theHarmonyParms.STEP_inport_connected_to_Meander_trigger_port==OUT_CLOCK_BEATX2_OUTPUT)
				durationFactor*=.125;
			else	
			if (theMeanderState.theHarmonyParms.STEP_inport_connected_to_Meander_trigger_port==OUT_CLOCK_BEATX4_OUTPUT)
				durationFactor*=.0625;	
			else
			if (theMeanderState.theHarmonyParms.STEP_inport_connected_to_Meander_trigger_port==OUT_CLOCK_BEATX8_OUTPUT)
				durationFactor*=.03125;	
			else
			if ( inputs[IN_PROG_STEP_EXT_CV].isConnected()) // something is connected to the circle STEP input but we do not know what. Assume it is an 16X BPM frequency
		  		durationFactor *= .01562;  
			
			float note_duration=durationFactor*time_sig_top/(frequency*theMeanderState.theBassParms.note_length_divisor);

		    bassGatePulse.trigger(note_duration);  
		}
	}
   
	LFOGenerator LFOclock;
	
	dsp::SchmittTrigger ST_32ts_trig;  // 32nd note timer tick

	dsp::SchmittTrigger run_button_trig;
	dsp::SchmittTrigger ext_run_trig;
	dsp::SchmittTrigger reset_btn_trig;
	dsp::SchmittTrigger reset_ext_trig;
	dsp::SchmittTrigger bpm_mode_trig;
	dsp::SchmittTrigger step_button_trig;

	dsp::PulseGenerator resetPulse;
	bool reset_pulse = false;

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
//	float max_bpm = 300.0f;
//	float max_bpm = 480.0f; // corresponds to CV=2.0
	float max_bpm = 960.0f; // corresponds to CV=3.0

	float extHarmonyIn=-99;
  

	// end of clock **************************

	dsp::ClockDivider lowFreqClock;
	dsp::ClockDivider sec1Clock;
	dsp::ClockDivider lightDivider;
	
	float phase = 0.f;
  		
	float last_melody_note=0;
	float current_melody_note=0;

	int circle_step_index=0;

	dsp::SchmittTrigger HarmonyEnableToggle;
	dsp::SchmittTrigger MelodyEnableToggle;
	dsp::SchmittTrigger BassEnableToggle;
	dsp::SchmittTrigger ArpEnableToggle;
	dsp::SchmittTrigger ArpEnableChordalToggle;
	dsp::SchmittTrigger ArpEnableScalerToggle;

	dsp::SchmittTrigger HarmonyEnableAll7thsToggle;
	dsp::SchmittTrigger HarmonyEnableV7thsToggle;

	dsp::SchmittTrigger HarmonyEnableStaccatoToggle;
	 
	
	dsp::SchmittTrigger MelodyDestutterToggle;
	dsp::SchmittTrigger MelodyEnableChordalToggle;
	dsp::SchmittTrigger MelodyEnableScalerToggle;

	dsp::SchmittTrigger MelodyEnableStaccatoToggle;
	
	dsp::SchmittTrigger BassSyncopateToggle;
	dsp::SchmittTrigger BassAccentToggle;
	dsp::SchmittTrigger BassShuffleToggle;
	dsp::SchmittTrigger BassOctavesToggle;

	dsp::SchmittTrigger BassEnableStaccatoToggle;
		
	dsp::SchmittTrigger RunToggle;

	dsp::SchmittTrigger RenderKeyboardToggle;
	dsp::SchmittTrigger RenderScoreToggle;
	
	
	dsp::SchmittTrigger CircleStepToggles[MAX_STEPS];
	dsp::SchmittTrigger CircleStepSetToggles[MAX_STEPS];

	bool CircleStepStates[MAX_STEPS]={};
	bool CircleStepSetStates[MAX_STEPS]={};

	rack::dsp::PulseGenerator barTriggerPulse; 
	rack::dsp::PulseGenerator harmonyGatePulse; 
	rack::dsp::PulseGenerator melodyGatePulse; 
	rack::dsp::PulseGenerator bassGatePulse; 
	rack::dsp::PulseGenerator barGaterPulse; 

	bool time_sig_changed=false;
	bool reset_enqueued=false;

	int override_step=1;

    // save button states
	json_t *dataToJson() override {
		json_t *rootJ = json_object();

		// running
		// running
		json_object_set_new(rootJ, "running", json_boolean(running)); 

		json_object_set_new(rootJ, "theHarmonyParmsenabled", json_boolean(theMeanderState.theHarmonyParms.enabled));
		json_object_set_new(rootJ, "harmony_staccato_enable", json_boolean(theMeanderState.theHarmonyParms.enable_staccato));
		json_object_set_new(rootJ, "theHarmonyParmsenable_all_7ths", json_boolean(theMeanderState.theHarmonyParms.enable_all_7ths));
		json_object_set_new(rootJ, "theHarmonyParmsenable_V_7ths", json_boolean(theMeanderState.theHarmonyParms.enable_V_7ths));
		json_object_set_new(rootJ, "theMelodyParmsenabled", json_boolean(theMeanderState.theMelodyParms.enabled));
		json_object_set_new(rootJ, "theMelodyParmsdestutter", json_boolean(theMeanderState.theMelodyParms.destutter));
		json_object_set_new(rootJ, "theMelodyParmsenable_staccato", json_boolean(theMeanderState.theMelodyParms.enable_staccato));
		json_object_set_new(rootJ, "theMelodyParmschordal", json_boolean(theMeanderState.theMelodyParms.chordal));
		json_object_set_new(rootJ, "theMelodyParmsscaler", json_boolean(theMeanderState.theMelodyParms.scaler));
		json_object_set_new(rootJ, "theArpParmsenabled", json_boolean(theMeanderState.theArpParms.enabled));
		json_object_set_new(rootJ, "theArpParmschordal", json_boolean(theMeanderState.theArpParms.chordal));
		json_object_set_new(rootJ, "theArpParmsscaler", json_boolean(theMeanderState.theArpParms.scaler));
		json_object_set_new(rootJ, "theBassParmsenabled", json_boolean(theMeanderState.theBassParms.enabled));
		json_object_set_new(rootJ, "theBassParmsenable_staccato", json_boolean(theMeanderState.theBassParms.enable_staccato));
		json_object_set_new(rootJ, "theBassParmssyncopate", json_boolean(theMeanderState.theBassParms.syncopate));
		json_object_set_new(rootJ, "theBassParmsaccent", json_boolean(theMeanderState.theBassParms.accent));
		json_object_set_new(rootJ, "theBassParmsshuffle", json_boolean(theMeanderState.theBassParms.shuffle));
		json_object_set_new(rootJ, "theBassParmsoctave_enabled", json_boolean(theMeanderState.theBassParms.octave_enabled));
		json_object_set_new(rootJ, "scale_out_mode", json_integer(scale_out_mode));
		json_object_set_new(rootJ, "gate_out_mode", json_integer(gate_out_mode));

		json_object_set_new(rootJ, "keyboard_render", json_boolean(theMeanderState.renderKeyboardEnabled));
		json_object_set_new(rootJ, "score_render", json_boolean(theMeanderState.renderScoreEnabled));

		json_object_set_new(rootJ, "paneltheme", json_integer(panelTheme));
		json_object_set_new(rootJ, "panelcontrast", json_real(panelContrast));

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
		
		return rootJ;
	}

	void dataFromJson(json_t *rootJ) override {

		// running
		
		json_t *runningJ = json_object_get(rootJ, "running");
		if (runningJ)
			running = json_is_true(runningJ);
				
		json_t *HarmonyParmsenabledJ = json_object_get(rootJ, "theHarmonyParmsenabled");
		if (HarmonyParmsenabledJ)
			theMeanderState.theHarmonyParms.enabled = json_is_true(HarmonyParmsenabledJ);

		json_t *HarmonyParmsstaccato_enableJ = json_object_get(rootJ, "harmony_staccato_enable");
		if (HarmonyParmsstaccato_enableJ)
			theMeanderState.theHarmonyParms.enable_staccato = json_is_true(HarmonyParmsstaccato_enableJ);

		json_t *HarmonyParmsenable_all_7thsJ = json_object_get(rootJ, "theHarmonyParmsenable_all_7ths");
		if (HarmonyParmsenable_all_7thsJ)
			theMeanderState.theHarmonyParms.enable_all_7ths = json_is_true(HarmonyParmsenable_all_7thsJ);

		json_t *HarmonyParmsenable_V_7thsJ = json_object_get(rootJ, "theHarmonyParmsenable_V_7ths");
		if (HarmonyParmsenable_V_7thsJ)
			theMeanderState.theHarmonyParms.enable_V_7ths = json_is_true(HarmonyParmsenable_V_7thsJ);

		json_t *MelodyParmsenabledJ = json_object_get(rootJ, "theMelodyParmsenabled");
		if (MelodyParmsenabledJ)
			theMeanderState.theMelodyParms.enabled = json_is_true(MelodyParmsenabledJ);

		json_t *MelodyParmsdestutterJ = json_object_get(rootJ, "theMelodyParmsdestutter");
		if (MelodyParmsdestutterJ)
			theMeanderState.theMelodyParms.destutter = json_is_true(MelodyParmsdestutterJ);
			
		json_t *MelodyParmsenable_staccatoJ = json_object_get(rootJ, "theMelodyParmsenable_staccato");
		if (MelodyParmsenable_staccatoJ)
			theMeanderState.theMelodyParms.enable_staccato = json_is_true(MelodyParmsenable_staccatoJ);

		json_t *MelodyParmschordalJ = json_object_get(rootJ, "theMelodyParmschordal");
		if (MelodyParmschordalJ)
			theMeanderState.theMelodyParms.chordal = json_is_true(MelodyParmschordalJ);

		json_t *MelodyParmsscalerJ = json_object_get(rootJ, "theMelodyParmsscaler");
		if (MelodyParmsscalerJ)
			theMeanderState.theMelodyParms.scaler = json_is_true(MelodyParmsscalerJ);

		json_t *ArpParmsenabledJ = json_object_get(rootJ, "theArpParmsenabled");
		if (ArpParmsenabledJ)
			theMeanderState.theArpParms.enabled = json_is_true(ArpParmsenabledJ);

		json_t *ArpParmschordalJ = json_object_get(rootJ, "theArpParmschordal");
		if (ArpParmschordalJ)
			theMeanderState.theArpParms.chordal = json_is_true(ArpParmschordalJ);

		json_t *ArpParmsscalerJ = json_object_get(rootJ, "theArpParmsscaler");
		if (ArpParmsscalerJ)
			theMeanderState.theArpParms.scaler = json_is_true(ArpParmsscalerJ);

		json_t *BassParmsenabledJ = json_object_get(rootJ, "theBassParmsenabled");
		if (BassParmsenabledJ)
			theMeanderState.theBassParms.enabled = json_is_true(BassParmsenabledJ);

		json_t *BassParmsenable_staccatoJ = json_object_get(rootJ, "theBassParmsenable_staccato");
		if (BassParmsenable_staccatoJ)
			theMeanderState.theBassParms.enable_staccato = json_is_true(BassParmsenable_staccatoJ);

		json_t *BassParmssyncopateJ = json_object_get(rootJ, "theBassParmssyncopate");
		if (BassParmssyncopateJ)
			theMeanderState.theBassParms.syncopate = json_is_true(BassParmssyncopateJ);

		json_t *BassParmsaccentJ = json_object_get(rootJ, "theBassParmsaccent");
		if (BassParmsaccentJ)
			theMeanderState.theBassParms.accent = json_is_true(BassParmsaccentJ);

		json_t *BassParmsshuffleJ = json_object_get(rootJ, "theBassParmsshuffle");
		if (BassParmsshuffleJ)
			theMeanderState.theBassParms.shuffle = json_is_true(BassParmsshuffleJ); 

		json_t *BassParmsoctave_enabledJ = json_object_get(rootJ, "theBassParmsoctave_enabled"); 
		if (BassParmsoctave_enabledJ)
			theMeanderState.theBassParms.octave_enabled = json_is_true(BassParmsoctave_enabledJ);
		
		json_t *modeJ = json_object_get(rootJ, "scale_out_mode");
        if(modeJ) scale_out_mode = (ScaleOutMode) json_integer_value(modeJ);
				
		modeJ = json_object_get(rootJ, "gate_out_mode");
        if(modeJ) gate_out_mode = (GateOutMode) json_integer_value(modeJ);

		json_t *render_keyboard_enabledJ = json_object_get(rootJ, "keyboard_render");
        if (render_keyboard_enabledJ)
			theMeanderState.renderKeyboardEnabled = json_is_true(render_keyboard_enabledJ);

		json_t *render_score_enabledJ = json_object_get(rootJ, "score_render");
        if (render_score_enabledJ)
			theMeanderState.renderScoreEnabled = json_is_true(render_score_enabledJ);
	
		json_t *panelthemeJ = json_object_get(rootJ, "paneltheme");
        if (panelthemeJ) panelTheme = json_integer_value(panelthemeJ);
	
		json_t *panelcontrastJ = json_object_get(rootJ, "panelcontrast");
        if (panelcontrastJ) panelContrast = json_real_value(panelcontrastJ);

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
				strcat(theHarmonyTypes[4].harmony_degrees_desc,circle_of_fifths_arabic_degrees[step]);  
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
				strcat(theHarmonyTypes[4].harmony_degrees_desc,circle_of_fifths_arabic_degrees[step]);  
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
				strcat(theHarmonyTypes[4].harmony_degrees_desc,circle_of_fifths_arabic_degrees[step]);  
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
				strcat(theHarmonyTypes[4].harmony_degrees_desc,circle_of_fifths_arabic_degrees[step]);  
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
				strcat(theHarmonyTypes[4].harmony_degrees_desc,circle_of_fifths_arabic_degrees[step]);  
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
				strcat(theHarmonyTypes[4].harmony_degrees_desc,circle_of_fifths_arabic_degrees[step]);  
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
				strcat(theHarmonyTypes[4].harmony_degrees_desc,circle_of_fifths_arabic_degrees[step]);  
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
				strcat(theHarmonyTypes[4].harmony_degrees_desc,circle_of_fifths_arabic_degrees[step]);  
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
				strcat(theHarmonyTypes[4].harmony_degrees_desc,circle_of_fifths_arabic_degrees[step]);  
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
				strcat(theHarmonyTypes[4].harmony_degrees_desc,circle_of_fifths_arabic_degrees[step]);  
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
				strcat(theHarmonyTypes[4].harmony_degrees_desc,circle_of_fifths_arabic_degrees[step]);  
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
				strcat(theHarmonyTypes[4].harmony_degrees_desc,circle_of_fifths_arabic_degrees[step]);  
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
				strcat(theHarmonyTypes[4].harmony_degrees_desc,circle_of_fifths_arabic_degrees[step]);  
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
				strcat(theHarmonyTypes[4].harmony_degrees_desc,circle_of_fifths_arabic_degrees[step]);  
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
				strcat(theHarmonyTypes[4].harmony_degrees_desc,circle_of_fifths_arabic_degrees[step]);  
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
				strcat(theHarmonyTypes[4].harmony_degrees_desc,circle_of_fifths_arabic_degrees[step]);  
				if (15<(loadedNumSteps-1)) 
				strcat(theHarmonyTypes[4].harmony_degrees_desc," ");
			}
		} 

	
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
				
				theMeanderState.theMelodyParms.bar_melody_counted_note=0;
				theMeanderState.theArpParms.note_count=0;
				theMeanderState.theBassParms.bar_bass_counted_note=0;
				outputs[OUT_CLOCK_BAR_OUTPUT].setVoltage(0.0f);	   // bars 	
				outputs[OUT_CLOCK_BEAT_OUTPUT].setVoltage(0.0f);   // 4ts 
				outputs[OUT_CLOCK_BEATX2_OUTPUT].setVoltage(0.0f); // 8ts
				outputs[OUT_CLOCK_BEATX4_OUTPUT].setVoltage(0.0f); // 16ts
				outputs[OUT_CLOCK_BEATX8_OUTPUT].setVoltage(0.0f); // 32ts
			}
			else
			{
				LFOclock.setFreq(frequency*(32/time_sig_bottom));	  // for 32ts	
				barts_count_limit = (32*time_sig_top/time_sig_bottom);
			}
			theMeanderState.theHarmonyParms.pending_step_edit=0;
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
	    	LFOclock.setReset(1.0f);
			bar_count = 0;
			bar_note_count=0;
			i16ts_count = 0;  
			i8ts_count = 0;  
			i4ts_count = 0; 
			i2ts_count = 0; 
			barts_count = 0;    
			 

			theMeanderState.theMelodyParms.bar_melody_counted_note=0;
			theMeanderState.theArpParms.note_count=0;
			theMeanderState.theBassParms.bar_bass_counted_note=0;

			theMeanderState.theHarmonyParms.last_circle_step=-1; // for Markov chain
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
				outputs[OUT_MELODY_GATE_OUTPUT].setVoltage(0);
				outputs[OUT_BASS_GATE_OUTPUT].setVoltage(0);
				outputs[OUT_HARMONY_VOLUME_OUTPUT].setVoltage(0);
				outputs[OUT_MELODY_VOLUME_OUTPUT].setVoltage(0);
				outputs[OUT_BASS_VOLUME_OUTPUT].setVoltage(0);
			}
			
		}

		resetLight -= resetLight / lightLambda / args.sampleRate;
		lights[LIGHT_LEDBUTTON_RESET].setBrightness(resetLight); 
		reset_pulse = resetPulse.process(1.0 / args.sampleRate);
  		outputs[OUT_RESET_OUT].setVoltage((reset_pulse ? 10.0f : 0.0f));
        
	
		if ((step_button_trig.process(params[BUTTON_PROG_STEP_PARAM].getValue() || (  inputs[IN_PROG_STEP_EXT_CV].isConnected()  &&  (inputs[IN_PROG_STEP_EXT_CV].getVoltage() > 0.))))) 
		{
			++bar_count;

			if (theMeanderState.theHarmonyParms.enabled)
			{
				theMeanderState.theHarmonyParms.enabled = false;
				override_step=0;
			}
			else
			{
				++override_step;
				if (override_step>=theActiveHarmonyType.num_harmony_steps)
				  override_step=0;
			    
			}
			theMeanderState.userControllingHarmonyFromCircle=true;
			theMeanderState.last_harmony_step=override_step;
				
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
				if (theMeanderState.theBassParms.enabled)
					doBass();
			}
		
		}

		stepLight -= stepLight / lightLambda / args.sampleRate;
		lights[LIGHT_LEDBUTTON_PROG_STEP_PARAM].setBrightness(stepLight);
		step_pulse = stepPulse.process(1.0 / args.sampleRate);

		if(running)  
		{
			// these should be done in initialization rather than every process() call
			LFOclock.setFreq(frequency*(32/time_sig_bottom));	  // for 32ts	should not hurt top call this each sample
			barts_count_limit = (32*time_sig_top/time_sig_bottom);
			//************************************************************************
						 
			LFOclock.step(1.0 / args.sampleRate);

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
				float IntClockLevel=5.0f*(LFOclock.sqr()+1.0f);
				if (ST_32ts_trig.process(LFOclock.sqr()))                         // triggers from each external clock tick ONLY once when .sqr() reaches 1.0V
				{
					 clockTick=true;
				}
			
				 outputs[OUT_CLOCK_OUT].setChannels(1);  // set polyphony  
				 outputs[OUT_CLOCK_OUT].setVoltage(IntClockLevel);  
			}
				
		    if (clockTick)
			{
				bool melodyPlayed=false;   // set to prevent arp note being played on the melody beat
				int barChordNumber=(int)((int)(barts_count*theMeanderState.theHarmonyParms.note_length_divisor)/(int)32);
			
				// bar
				if (barts_count == 0) 
				{
					theMeanderState.theMelodyParms.bar_melody_counted_note=0;
					theMeanderState.theBassParms.bar_bass_counted_note=0;
					bar_note_count=0;
					if ((theMeanderState.theHarmonyParms.note_length_divisor==1)&&(!theMeanderState.userControllingHarmonyFromCircle))
						doHarmony(barChordNumber, theMeanderState.theHarmonyParms.enabled);
					if ((theMeanderState.theBassParms.note_length_divisor==1)&&(!theMeanderState.userControllingHarmonyFromCircle))
						doBass();
					if ((theMeanderState.theMelodyParms.note_length_divisor==1)&&(!theMeanderState.userControllingMelody))
					{
						doMelody();
						melodyPlayed=true;
					}
					clockPulse1ts.trigger(trigger_length);
					// Pulse the output gate 
					barTriggerPulse.trigger(1e-3f);  // 1ms duration  need to use .process to detect this and then send it to output
				}
						
		        // i2ts
				if (i2ts_count == 0)
				{
					if ((theMeanderState.theHarmonyParms.note_length_divisor==2)&&(!theMeanderState.userControllingHarmonyFromCircle))
						doHarmony(barChordNumber, theMeanderState.theHarmonyParms.enabled);
					if ((theMeanderState.theBassParms.note_length_divisor==2)&&(!theMeanderState.userControllingHarmonyFromCircle))
						doBass();
					if ((theMeanderState.theMelodyParms.note_length_divisor==2)&&(!theMeanderState.userControllingMelody))
					{
						doMelody();
						melodyPlayed=true;
					}
					i2ts_count++;

					if (!theMeanderState.theHarmonyParms.STEP_inport_connected_to_Meander_trigger_port)  // there is no connection to the STEP inport
						clockPulse2ts.trigger(trigger_length);
				}
				else
				if (i2ts_count == (i2ts_count_limit-1))
				{
					i2ts_count = 0;    
					if (theMeanderState.theHarmonyParms.STEP_inport_connected_to_Meander_trigger_port) // there is a connection to the STEP inport
						clockPulse2ts.trigger(trigger_length);
				}
				else
				{
					i2ts_count++;
				}
								
		
				// i4ts
				if (i4ts_count == 0)
				{
					if ((theMeanderState.theHarmonyParms.note_length_divisor==4)&&(!theMeanderState.userControllingHarmonyFromCircle))
						doHarmony(barChordNumber, theMeanderState.theHarmonyParms.enabled);
					if ((theMeanderState.theBassParms.note_length_divisor==4)&&(!theMeanderState.userControllingHarmonyFromCircle))
						doBass();
					if ((theMeanderState.theMelodyParms.note_length_divisor==4)&&(!theMeanderState.userControllingMelody))
					{
						doMelody();
						melodyPlayed=true;
					}
					if ((theMeanderState.theArpParms.enabled)&&(theMeanderState.theArpParms.note_length_divisor==4)&&(!melodyPlayed))
						doArp();

					i4ts_count++;

					if (!theMeanderState.theHarmonyParms.STEP_inport_connected_to_Meander_trigger_port) // there is no connection to the STEP inport
					 	clockPulse4ts.trigger(trigger_length);
				}
				else
				if (i4ts_count == (i4ts_count_limit-1))
				{
					i4ts_count = 0;  
					if (theMeanderState.theHarmonyParms.STEP_inport_connected_to_Meander_trigger_port)  // there is a connection to the STEP inport
					 	clockPulse4ts.trigger(trigger_length);
				}
				else
				{
					i4ts_count++;
				}
				
					  
		 		// i8ts
				if (i8ts_count == 0)
				{
					if ((theMeanderState.theHarmonyParms.note_length_divisor==8)&&(!theMeanderState.userControllingHarmonyFromCircle))
						doHarmony(barChordNumber, theMeanderState.theHarmonyParms.enabled);
					if ((theMeanderState.theBassParms.note_length_divisor==8)&&(!theMeanderState.userControllingHarmonyFromCircle))
						doBass();
					if ((theMeanderState.theMelodyParms.note_length_divisor==8)&&(!theMeanderState.userControllingMelody))
					{
						doMelody();
						melodyPlayed=true;
					}
					if ((theMeanderState.theArpParms.enabled)&&(theMeanderState.theArpParms.note_length_divisor==8)&&(!melodyPlayed))
						doArp();

					i8ts_count++;

					if (!theMeanderState.theHarmonyParms.STEP_inport_connected_to_Meander_trigger_port)  // there is no connection to the STEP inport
						clockPulse8ts.trigger(trigger_length);
				}
				else
				if (i8ts_count == (i8ts_count_limit-1))
				{
					i8ts_count = 0; 
					if (theMeanderState.theHarmonyParms.STEP_inport_connected_to_Meander_trigger_port)  // there is a connection to the STEP inport
					  clockPulse8ts.trigger(trigger_length);
				}
				else
				{
					i8ts_count++; 
				}
				
				
				// i16ts
				if (i16ts_count == 0)
				{
					if ((theMeanderState.theMelodyParms.note_length_divisor==16)&&(!theMeanderState.userControllingMelody))
					{
						doMelody();  
						melodyPlayed=true;  
					}
					if ((theMeanderState.theArpParms.enabled)&&(theMeanderState.theArpParms.note_length_divisor==16)&&(!melodyPlayed))
						doArp();

					i16ts_count++;

					if (!theMeanderState.theHarmonyParms.STEP_inport_connected_to_Meander_trigger_port)  // there is no connection to the STEP inport
						clockPulse16ts.trigger(trigger_length);
				}
				else
				if (i16ts_count == (i16ts_count_limit-1))
				{
					i16ts_count = 0;
					if (theMeanderState.theHarmonyParms.STEP_inport_connected_to_Meander_trigger_port)  // there is a connection to the STEP inport
						clockPulse16ts.trigger(trigger_length);
				}
				else
				{
					i16ts_count++;
				}
				

				//32nds  ***********************************

				clock_t current_cpu_t= clock();  // cpu clock ticks since program began
				double current_cpu_time_double= (double)(current_cpu_t) / (double)CLOCKS_PER_SEC;
			
				 // do on each 1/32nd clock tick
				
				if ((theMeanderState.theMelodyParms.note_length_divisor==32)&&(!theMeanderState.userControllingMelody))
				{
					doMelody();   
					melodyPlayed=true; 
				}
				if ((theMeanderState.theArpParms.enabled)&&(theMeanderState.theArpParms.note_length_divisor==32)&&(!melodyPlayed))
					doArp(); 
							
				// output some fBm noise
				double period=1.0/theMeanderState.theArpParms.period; // 1/seconds
				double fBmarg=theMeanderState.theArpParms.seed + (double)(period*current_cpu_time_double); 
				double fBmrand=(FastfBm1DNoise(fBmarg,theMeanderState.theArpParms.noctaves) +1.)/2; 
				outputs[OUT_FBM_ARP_OUTPUT].setChannels(1);  // set polyphony  
				outputs[OUT_FBM_ARP_OUTPUT].setVoltage((float)clamp((10.f*fBmrand), 0.f, 10.f) ,0);  // rescale fBm output to 0-10V so it can be used better for CV
				
				if (barts_count == (barts_count_limit-1))  // do this after all processing so bar_count does not get incremented too early
				{
					barts_count = 0;  
					theMeanderState.theMelodyParms.bar_melody_counted_note=0;
					theMeanderState.theBassParms.bar_bass_counted_note=0;
					bar_note_count=0;
					if (!theMeanderState.userControllingHarmonyFromCircle)  // don't mess up bar count
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
				theMeanderState.theHarmonyParms.last_chord_playing=false;
			}
		
			if (melodyGatePulse.process( 1.0 / APP->engine->getSampleRate()))
			{
				outputs[OUT_MELODY_GATE_OUTPUT].setVoltage(CV_MAX10);
			}
			else
			{
				outputs[OUT_MELODY_GATE_OUTPUT].setVoltage(0);
				for (int i=0; ((i<256)&&(i<bar_note_count)); ++i)
				{
					if (played_notes_circular_buffer[i].noteType==NOTE_TYPE_MELODY) 
						played_notes_circular_buffer[i].isPlaying=false;
					if (played_notes_circular_buffer[i].noteType==NOTE_TYPE_ARP) 
						played_notes_circular_buffer[i].isPlaying=false;
				}
			}

		
		    if (bassGatePulse.process( 1.0 / APP->engine->getSampleRate()))
			{
				outputs[OUT_BASS_GATE_OUTPUT].setVoltage(CV_MAX10);
			}
			else
			{
				outputs[OUT_BASS_GATE_OUTPUT].setVoltage(0);
				for (int i=0; ((i<256)&&(i<bar_note_count)); ++i)
				{
					if (played_notes_circular_buffer[i].noteType==NOTE_TYPE_BASS) 
						played_notes_circular_buffer[i].isPlaying=false;
				}
			}


			float bassVolumeLevel=theMeanderState.theBassParms.volume;
			if (theMeanderState.theBassParms.accent)
			{
				if (theMeanderState.theBassParms.note_accented)
					bassVolumeLevel=theMeanderState.theBassParms.volume;
				else
					bassVolumeLevel=0.5*theMeanderState.theBassParms.volume;
				bassVolumeLevel=clamp(bassVolumeLevel, 0.0f, 10.f); 

			}
			outputs[OUT_BASS_VOLUME_OUTPUT].setVoltage(bassVolumeLevel);
		}

		
		if (gate_out_mode == VOLUME_OVER_GATE) // non-standard volume over gate voltages
		{
			float harmonyGateLevel=theMeanderState.theHarmonyParms.volume; 
			harmonyGateLevel=clamp(harmonyGateLevel, 2.1f, 10.f);  // don't let gate on level drop below 2.0v so it will trigger ADSR etc.
			if (harmonyGatePulse.process( 1.0 / APP->engine->getSampleRate()))
			{
				outputs[OUT_HARMONY_GATE_OUTPUT].setVoltage(harmonyGateLevel);
			}
			else
			{
				outputs[OUT_HARMONY_GATE_OUTPUT].setVoltage(0);
				theMeanderState.theHarmonyParms.last[0].note=0;  // set as invalid since key should no longer be drawn as pressed
				theMeanderState.theHarmonyParms.last[1].note=0;
				theMeanderState.theHarmonyParms.last[2].note=0;
				theMeanderState.theHarmonyParms.last[3].note=0;
			}

			float melodyGateLevel=theMeanderState.theMelodyParms.volume; 
			melodyGateLevel=clamp(melodyGateLevel, 2.1f, 10.f);   // don't let gate on level drop below 2.0v so it will trigger ADSR etc.
		
			if (melodyGatePulse.process( 1.0 / APP->engine->getSampleRate()))
			{
				outputs[OUT_MELODY_GATE_OUTPUT].setVoltage( melodyGateLevel);
			}
			else
			{
				outputs[OUT_MELODY_GATE_OUTPUT].setVoltage(0);
				for (int i=0; ((i<256)&&(i<bar_note_count)); ++i)
				{
					if (played_notes_circular_buffer[i].noteType==NOTE_TYPE_MELODY) 
						played_notes_circular_buffer[i].isPlaying=false;
					if (played_notes_circular_buffer[i].noteType==NOTE_TYPE_ARP) 
						played_notes_circular_buffer[i].isPlaying=false;
				}
			}

			float bassGateLevel=theMeanderState.theBassParms.volume;

			if (theMeanderState.theBassParms.accent)
			{
				if (!theMeanderState.theBassParms.note_accented)
					bassGateLevel*=.8;
			}

			bassGateLevel=clamp(bassGateLevel, 2.1f, 10.f); // don't let gate on level drop below 2.0v so it will trigger ADSR etc.
		
		    if (bassGatePulse.process( 1.0 / APP->engine->getSampleRate()))
			{
				outputs[OUT_BASS_GATE_OUTPUT].setVoltage(bassGateLevel);
			}
			else
			{
				outputs[OUT_BASS_GATE_OUTPUT].setVoltage(0);
				theMeanderState.theBassParms.last[0].note=0;  // set as invalid since key should no longer be drawn as pressed
				theMeanderState.theBassParms.last[1].note=0;
			}


			float bassVolumeLevel=theMeanderState.theBassParms.volume;
			if (theMeanderState.theBassParms.accent)
			{
				if (theMeanderState.theBassParms.note_accented)
					bassVolumeLevel=theMeanderState.theBassParms.volume;
				else
					bassVolumeLevel=0.5*theMeanderState.theBassParms.volume;
				bassVolumeLevel=clamp(bassVolumeLevel, 0.0f, 10.f); 

			}
			outputs[OUT_BASS_VOLUME_OUTPUT].setVoltage(bassVolumeLevel);
		}
				
				
		outputs[OUT_CLOCK_BAR_OUTPUT].setVoltage((pulse1ts ? 10.0f : 0.0f));     // barts  
		outputs[OUT_CLOCK_BEAT_OUTPUT].setVoltage((pulse4ts ? 10.0f : 0.0f));    // 4ts
		outputs[OUT_CLOCK_BEATX2_OUTPUT].setVoltage((pulse8ts ? 10.0f : 0.0f));  // 8ts
		outputs[OUT_CLOCK_BEATX4_OUTPUT].setVoltage((pulse16ts ? 10.0f : 0.0f)); // 16ts
		outputs[OUT_CLOCK_BEATX8_OUTPUT].setVoltage((pulse32ts ? 10.0f : 0.0f)); // 32ts

	        
		if (HarmonyEnableToggle.process(params[BUTTON_ENABLE_HARMONY_PARAM].getValue())) 
		{
			theMeanderState.theHarmonyParms.enabled = !theMeanderState.theHarmonyParms.enabled;
			theMeanderState.userControllingHarmonyFromCircle=false;
		}
		lights[LIGHT_LEDBUTTON_HARMONY_ENABLE].setBrightness(theMeanderState.theHarmonyParms.enabled ? 1.0f : 0.0f); 

		if (HarmonyEnableAll7thsToggle.process(params[BUTTON_ENABLE_HARMONY_ALL7THS_PARAM].getValue())) 
		{
			theMeanderState.theHarmonyParms.enable_all_7ths = !theMeanderState.theHarmonyParms.enable_all_7ths;
			if (theMeanderState.theHarmonyParms.enable_all_7ths)
				theMeanderState.theHarmonyParms.enable_V_7ths=false;
			setup_harmony();  // calculate harmony notes
			circleChanged=true;
		}
		lights[LIGHT_LEDBUTTON_ENABLE_HARMONY_ALL7THS_PARAM].setBrightness(theMeanderState.theHarmonyParms.enable_all_7ths ? 1.0f : 0.0f); 
		

		if (HarmonyEnableV7thsToggle.process(params[BUTTON_ENABLE_HARMONY_V7THS_PARAM].getValue())) 
		{
			theMeanderState.theHarmonyParms.enable_V_7ths = !theMeanderState.theHarmonyParms.enable_V_7ths;
			if (theMeanderState.theHarmonyParms.enable_V_7ths)
				theMeanderState.theHarmonyParms.enable_all_7ths=false;
			setup_harmony();
			circleChanged=true;
		}
		lights[LIGHT_LEDBUTTON_ENABLE_HARMONY_V7THS_PARAM].setBrightness(theMeanderState.theHarmonyParms.enable_V_7ths ? 1.0f : 0.0f); 


		if (HarmonyEnableStaccatoToggle.process(params[BUTTON_ENABLE_HARMONY_STACCATO_PARAM].getValue())) 
		{		
			theMeanderState.theHarmonyParms.enable_staccato = !theMeanderState.theHarmonyParms.enable_staccato;
			
		}
		lights[LIGHT_LEDBUTTON_ENABLE_HARMONY_STACCATO_PARAM].setBrightness(theMeanderState.theHarmonyParms.enable_staccato); 
	

		if (MelodyEnableStaccatoToggle.process(params[BUTTON_ENABLE_MELODY_STACCATO_PARAM].getValue())) 
		{
			theMeanderState.theMelodyParms.enable_staccato = !theMeanderState.theMelodyParms.enable_staccato;
	
		}
		lights[LIGHT_LEDBUTTON_ENABLE_MELODY_STACCATO_PARAM].setBrightness(theMeanderState.theMelodyParms.enable_staccato ? 1.0f : 0.0f); 

		if (BassEnableStaccatoToggle.process(params[BUTTON_ENABLE_BASS_STACCATO_PARAM].getValue())) 
		{
			theMeanderState.theBassParms.enable_staccato = !theMeanderState.theBassParms.enable_staccato;
	
		}
		lights[LIGHT_LEDBUTTON_ENABLE_BASS_STACCATO_PARAM].setBrightness(theMeanderState.theBassParms.enable_staccato ? 1.0f : 0.0f); 
		
		if (BassEnableToggle.process(params[BUTTON_ENABLE_BASS_PARAM].getValue())) 
		{
			theMeanderState.theBassParms.enabled = !theMeanderState.theBassParms.enabled;
		}
		lights[LIGHT_LEDBUTTON_BASS_ENABLE].setBrightness(theMeanderState.theBassParms.enabled ? 1.0f : 0.0f); 

		
		
		if (MelodyEnableToggle.process(params[BUTTON_ENABLE_MELODY_PARAM].getValue())) 
		{
			theMeanderState.theMelodyParms.enabled = !theMeanderState.theMelodyParms.enabled;
			if (theMeanderState.theMelodyParms.enabled)
			   theMeanderState.userControllingMelody=false;
		}
		lights[LIGHT_LEDBUTTON_MELODY_ENABLE].setBrightness(theMeanderState.theMelodyParms.enabled ? 1.0f : 0.0f); 

		if (MelodyDestutterToggle.process(params[BUTTON_MELODY_DESTUTTER_PARAM].getValue())) 
		{
			theMeanderState.theMelodyParms.destutter = !theMeanderState.theMelodyParms.destutter;
		}
		lights[LIGHT_LEDBUTTON_MELODY_DESTUTTER].setBrightness(theMeanderState.theMelodyParms.destutter ? 1.0f : 0.0f); 

		if (MelodyEnableChordalToggle.process(params[BUTTON_ENABLE_MELODY_CHORDAL_PARAM].getValue())) 
		{
			theMeanderState.theMelodyParms.chordal = !theMeanderState.theMelodyParms.chordal;
			theMeanderState.theMelodyParms.scaler = !theMeanderState.theMelodyParms.scaler;
		}
		lights[LIGHT_LEDBUTTON_MELODY_ENABLE_CHORDAL].setBrightness(theMeanderState.theMelodyParms.chordal ? 1.0f : 0.0f); 

		if (MelodyEnableScalerToggle.process(params[BUTTON_ENABLE_MELODY_SCALER_PARAM].getValue())) 
		{
			theMeanderState.theMelodyParms.scaler = !theMeanderState.theMelodyParms.scaler;
			theMeanderState.theMelodyParms.chordal = !theMeanderState.theMelodyParms.chordal;
		}
		lights[LIGHT_LEDBUTTON_MELODY_ENABLE_SCALER].setBrightness(theMeanderState.theMelodyParms.scaler ? 1.0f : 0.0f); 

				

		if (ArpEnableToggle.process(params[BUTTON_ENABLE_ARP_PARAM].getValue())) 
		{
			theMeanderState.theArpParms.enabled = !theMeanderState.theArpParms.enabled;
		}
		lights[LIGHT_LEDBUTTON_ARP_ENABLE].setBrightness(theMeanderState.theArpParms.enabled ? 1.0f : 0.0f); 

		if (ArpEnableChordalToggle.process(params[BUTTON_ENABLE_ARP_CHORDAL_PARAM].getValue())) 
		{
			theMeanderState.theArpParms.chordal = !theMeanderState.theArpParms.chordal;
			theMeanderState.theArpParms.scaler = !theMeanderState.theArpParms.scaler;
		}
		lights[LIGHT_LEDBUTTON_ARP_ENABLE_CHORDAL].setBrightness(theMeanderState.theArpParms.chordal ? 1.0f : 0.0f); 

		if (ArpEnableScalerToggle.process(params[BUTTON_ENABLE_ARP_SCALER_PARAM].getValue())) 
		{
			theMeanderState.theArpParms.scaler = !theMeanderState.theArpParms.scaler;
			theMeanderState.theArpParms.chordal = !theMeanderState.theArpParms.chordal;
		}
		lights[LIGHT_LEDBUTTON_ARP_ENABLE_SCALER].setBrightness(theMeanderState.theArpParms.scaler ? 1.0f : 0.0f); 

		//****Bass

		if (BassSyncopateToggle.process(params[BUTTON_BASS_SYNCOPATE_PARAM].getValue())) 
		{
			theMeanderState.theBassParms.syncopate = !theMeanderState.theBassParms.syncopate;
		}
		lights[LIGHT_LEDBUTTON_BASS_SYNCOPATE_PARAM].setBrightness(theMeanderState.theBassParms.syncopate ? 1.0f : 0.0f); 	

		if (BassAccentToggle.process(params[BUTTON_BASS_ACCENT_PARAM].getValue())) 
		{
			theMeanderState.theBassParms.accent = !theMeanderState.theBassParms.accent;
		}
		lights[LIGHT_LEDBUTTON_BASS_ACCENT_PARAM].setBrightness(theMeanderState.theBassParms.accent ? 1.0f : 0.0f); 	

		if (BassShuffleToggle.process(params[BUTTON_BASS_SHUFFLE_PARAM].getValue())) 
		{
			theMeanderState.theBassParms.shuffle = !theMeanderState.theBassParms.shuffle;
		}
		lights[LIGHT_LEDBUTTON_BASS_SHUFFLE_PARAM].setBrightness(theMeanderState.theBassParms.shuffle ? 1.0f : 0.0f); 	

		if (BassOctavesToggle.process(params[BUTTON_BASS_OCTAVES_PARAM].getValue())) 
		{
			theMeanderState.theBassParms.octave_enabled = !theMeanderState.theBassParms.octave_enabled;
		}
		lights[LIGHT_LEDBUTTON_BASS_OCTAVES_PARAM].setBrightness(theMeanderState.theBassParms.octave_enabled ? 1.0f : 0.0f); 	


		if (RenderKeyboardToggle.process(params[BUTTON_ENABLE_KEYBOARD_RENDER_PARAM].getValue())) 
		{
			theMeanderState.renderKeyboardEnabled = !theMeanderState.renderKeyboardEnabled;
		}
		lights[LIGHT_LEDBUTTON_ENABLE_RENDER_KEYBOARD_PARAM].setBrightness(theMeanderState.renderKeyboardEnabled ? 1.0f : 0.0f); 

		if (RenderScoreToggle.process(params[BUTTON_ENABLE_SCORE_RENDER_PARAM].getValue())) 
		{
			theMeanderState.renderScoreEnabled = !theMeanderState.renderScoreEnabled;
		}
		lights[LIGHT_LEDBUTTON_ENABLE_RENDER_SCORE_PARAM].setBrightness(theMeanderState.renderScoreEnabled ? 1.0f : 0.0f);	

			
		
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
			
				userPlaysCirclePositionHarmony(current_circle_position, theMeanderState.theHarmonyParms.target_octave); 
										
				theMeanderState.userControllingHarmonyFromCircle=true;
				theMeanderState.theHarmonyParms.enabled=false;
				lights[LIGHT_LEDBUTTON_HARMONY_ENABLE].setBrightness(theMeanderState.theHarmonyParms.enabled ? 1.0f : 0.0f); 
			
			
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
							if (theMeanderState.theHarmonyParms.pending_step_edit)
							{
								theHarmonyTypes[harmony_type].harmony_steps[theMeanderState.theHarmonyParms.pending_step_edit-BUTTON_HARMONY_SETSTEP_1_PARAM]=theDegree;
								
								strcpy(theHarmonyTypes[harmony_type].harmony_degrees_desc,"");
								for (int k=0;k<theHarmonyTypes[harmony_type].num_harmony_steps;++k)
								{
									strcat(theHarmonyTypes[harmony_type].harmony_degrees_desc,circle_of_fifths_arabic_degrees[theHarmonyTypes[harmony_type].harmony_steps[k]]);  
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
					theMeanderState.theHarmonyParms.pending_step_edit=BUTTON_HARMONY_SETSTEP_1_PARAM+selectedStep;

					int current_circle_position=0;
					if (true)
					{
						int degreeStep=(theActiveHarmonyType.harmony_steps[selectedStep])%8;  
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
					}

					
					for (int i=0; i<12; ++i)  
					{
						lights[LIGHT_LEDBUTTON_CIRCLESTEP_1+i].setBrightness(0.0f);	
					}
					lights[LIGHT_LEDBUTTON_CIRCLESTEP_1+current_circle_position].setBrightness(1.0f);
										
					userPlaysCirclePositionHarmony(current_circle_position, theMeanderState.theHarmonyParms.target_octave);  
				
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
		float scaleDegree=0;   // for melody
		float gateValue=0;

		if (  (inputs[IN_HARMONY_CIRCLE_DEGREE_GATE_EXT_CV].isConnected()) && (inputs[IN_HARMONY_CIRCLE_DEGREE_EXT_CV].isConnected()) )
		{
			circleDegree=inputs[IN_HARMONY_CIRCLE_DEGREE_EXT_CV].getVoltage();
			gateValue=inputs[IN_HARMONY_CIRCLE_DEGREE_GATE_EXT_CV].getVoltage(); 

			theMeanderState.theHarmonyParms.lastCircleDegreeIn=circleDegree;
			extHarmonyIn=circleDegree;
		
			float octave=(float)((int)(circleDegree));  // from the keyboard
			if (octave>3)
				octave=3;
			if (octave<-3)
				octave=-3;
			bool degreeChanged=false; // assume false unless determined true below
			bool skipStep=false;

			if ((gateValue==circleDegree)&&(circleDegree>=1)&&(circleDegree<=7.7))  // MarkovSeq or other 1-7V degree  degree.octave 0.0-7.7V
			{
				if (inportStates[IN_HARMONY_CIRCLE_DEGREE_EXT_CV].inTransition)
				{
					if (circleDegree==inportStates[IN_HARMONY_CIRCLE_DEGREE_EXT_CV].lastValue)
					{
						// was in transition but now is not
						inportStates[IN_HARMONY_CIRCLE_DEGREE_EXT_CV].inTransition=false;
						inportStates[IN_HARMONY_CIRCLE_DEGREE_EXT_CV].lastValue=circleDegree;
						octave = (int)(10.0*std::fmod(circleDegree, 1.0f));
						if (octave>7)
							octave=7;
						circleDegree=(float)((int)circleDegree);
						theMeanderState.circleDegree=(int)circleDegree;
						degreeChanged=true;
						harmonyGatePulse.reset();  // kill the pulse in case it is active
		            	outputs[OUT_HARMONY_GATE_OUTPUT].setVoltage(0);
					}
					else
					if (circleDegree!=inportStates[IN_HARMONY_CIRCLE_DEGREE_EXT_CV].lastValue)
					{
						harmonyGatePulse.reset();  // kill the pulse in case it is active
		            	outputs[OUT_HARMONY_GATE_OUTPUT].setVoltage(0);
						inportStates[IN_HARMONY_CIRCLE_DEGREE_EXT_CV].lastValue=circleDegree;
					}
				}
				else
				{
					if (circleDegree!=inportStates[IN_HARMONY_CIRCLE_DEGREE_EXT_CV].lastValue)
					{
						harmonyGatePulse.reset();  // kill the pulse in case it is active
			            outputs[OUT_HARMONY_GATE_OUTPUT].setVoltage(0);
						inportStates[IN_HARMONY_CIRCLE_DEGREE_EXT_CV].inTransition=true;
						inportStates[IN_HARMONY_CIRCLE_DEGREE_EXT_CV].lastValue=circleDegree;
					}
				}

				if (circleDegree==0)
				{
					degreeChanged=false;
				}
			}
			else
			if ((gateValue==circleDegree)&&((circleDegree<1.0)||(circleDegree>=8.0)))  // MarkovSeq or other 1-7V degree  degree.octave 1.0-7.7V  <1 or >=8V means skip step
			{
				inportStates[IN_HARMONY_CIRCLE_DEGREE_EXT_CV].lastValue=fvalue;
				degreeChanged=true;
				skipStep=true;
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
							if ((std::abs(circleDegree)<.005f))  	   theMeanderState.circleDegree=1;
							else
							if ((std::abs(circleDegree-.167f)<.005f))  theMeanderState.circleDegree=2;
							else
							if ((std::abs(circleDegree-.333f)<.005f))  theMeanderState.circleDegree=3;
							else
							if ((std::abs(circleDegree-.417f)<.005f))  theMeanderState.circleDegree=4;
							else
							if ((std::abs(circleDegree-.583f)<.005f))  theMeanderState.circleDegree=5;
							else
							if ((std::abs(circleDegree-.750f)<.005f))  theMeanderState.circleDegree=6;
							else
							if ((std::abs(circleDegree-.917f)<.005f))  theMeanderState.circleDegree=7;
							else
								degreeChanged=false;
						}
						else
						{
							octave-=1;
							if ((std::abs(circleDegree)<.005f))  			 theMeanderState.circleDegree=1;
							else
							if (std::abs(std::abs(circleDegree)-.083)<.005f)  theMeanderState.circleDegree=7;
							else
							if (std::abs(std::abs(circleDegree)-.250)<.005f)  theMeanderState.circleDegree=6;
							else
							if (std::abs(std::abs(circleDegree)-.417)<.005f)  theMeanderState.circleDegree=5;
							else
							if (std::abs(std::abs(circleDegree)-.583)<.005f)  theMeanderState.circleDegree=4;
							else
							if (std::abs(std::abs(circleDegree)-.667)<.005f)  theMeanderState.circleDegree=3;
							else
							if (std::abs(std::abs(circleDegree)-.833)<.005f)  theMeanderState.circleDegree=2;
							else
								degreeChanged=false;
						}
						
					}	
				
			}
			
        	if ((degreeChanged)&&(!skipStep))
			{
				if (theMeanderState.circleDegree<1)
					theMeanderState.circleDegree=1;
				if (theMeanderState.circleDegree>7)
					theMeanderState.circleDegree=7;
								
				int step=1;  // default if not found below
				for (int i=0; i<MAX_STEPS; ++i)
				{
					if (theActiveHarmonyType.harmony_steps[i]==theMeanderState.circleDegree)
					{
						step=i;
						break;
					}
				}

				theMeanderState.last_harmony_step=step;

				int theCirclePosition=0;
				for (int i=0; i<7; ++i)
				{
					if (theCircleOf5ths.theDegreeSemiCircle.degreeElements[i].Degree==theMeanderState.circleDegree)
					{
						theCirclePosition=theCircleOf5ths.theDegreeSemiCircle.degreeElements[i].CircleIndex;
						break;
					}
				}

				last_circle_position=theCirclePosition;
			
				userPlaysCirclePositionHarmony(theCirclePosition, octave+theMeanderState.theHarmonyParms.target_octave);  // play immediate
				if (theMeanderState.theBassParms.enabled)
			    	doBass();
			
				if (running)
				{
					theMeanderState.userControllingHarmonyFromCircle=true;
					theMeanderState.theHarmonyParms.enabled=false;
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
				theMeanderState.theMelodyParms.note_length_divisor=(int)std::pow(2,melody_note_length_divisor);
				params[CONTROL_MELODY_NOTE_LENGTH_DIVISOR_PARAM].setValue(melody_note_length_divisor);

				theMeanderState.theArpParms.note_length_divisor=(int)std::pow(2,melody_note_length_divisor+1);
				params[CONTROL_ARP_INCREMENT_PARAM].setValue(melody_note_length_divisor+1);
				time_sig_changed=true;
			}
			  
			
			frequency = tempo/60.0f;  // BPS
			
		
			if ((fvalue=std::round(params[CONTROL_ROOT_KEY_PARAM].getValue()))!=circle_root_key)
			{
				circle_root_key=(int)fvalue;
				root_key=circle_of_fifths[circle_root_key];
				outputs[OUT_EXT_ROOT_OUTPUT].setVoltage(root_key/12.0);
				for (int i=0; i<12; ++i)
					lights[LIGHT_CIRCLE_ROOT_KEY_POSITION_1_LIGHT+i].setBrightness(0.0f);
				lights[LIGHT_CIRCLE_ROOT_KEY_POSITION_1_LIGHT+circle_root_key].setBrightness(1.0f);
				circleChanged=true;
			}

			
			if ((fvalue=std::round(params[CONTROL_SCALE_PARAM].getValue()))!=mode)
			{
				mode = fvalue;
				circleChanged=true;
			}
			

			// check input ports for change

			for (int i=0; i<Meander::NUM_INPUTS; ++i)
			{
				if (inputs[i].isConnected())
				{
					float fvalue=inputs[i].getVoltage();
								
					if ((i==IN_MELODY_SCALE_DEGREE_EXT_CV)||(fvalue!=inportStates[i].lastValue))  // don't do anything unless input changed
					{
						if (i!=IN_MELODY_SCALE_DEGREE_EXT_CV)
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
											theMeanderState.theMelodyParms.note_length_divisor=(int)std::pow(2,melody_note_length_divisor);
											params[CONTROL_MELODY_NOTE_LENGTH_DIVISOR_PARAM].setValue(melody_note_length_divisor);
											theMeanderState.theArpParms.note_length_divisor=(int)std::pow(2,melody_note_length_divisor+1);
											params[CONTROL_ARP_INCREMENT_PARAM].setValue(melody_note_length_divisor+1);
											time_sig_changed=true;
										}
									}
									break;


							// process harmony input ports

							case IN_HARMONY_ENABLE_EXT_CV:
								if (fvalue>0)
									theMeanderState.theHarmonyParms.enabled = true;
								else
								if (fvalue==0)
									theMeanderState.theHarmonyParms.enabled = false;
								else
								if (fvalue<0) 
								{
									// Do nothing.  Allow local parameter control
								}
								break;

							case IN_HARMONY_VOLUME_EXT_CV:
								if (fvalue>=0.01)
								if (fvalue!=theMeanderState.theHarmonyParms.volume)
								{
									fvalue=clamp(fvalue, 0., 10.);
									theMeanderState.theHarmonyParms.volume=fvalue;  
									params[CONTROL_HARMONY_VOLUME_PARAM].setValue(fvalue);
									outputs[OUT_HARMONY_VOLUME_OUTPUT].setVoltage(theMeanderState.theHarmonyParms.volume);
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
												strcat(theHarmonyTypes[harmony_type].harmony_degrees_desc,circle_of_fifths_arabic_degrees[theHarmonyTypes[harmony_type].harmony_steps[k]]); 
												if (k<(theHarmonyTypes[harmony_type].num_harmony_steps-1)) 
													strcat(theHarmonyTypes[harmony_type].harmony_degrees_desc,"-");
											}

										//	if (((int)newValue>=theActiveHarmonyType.min_steps)&&((int)newValue<=theActiveHarmonyType.max_steps))
										//		theActiveHarmonyType.num_harmony_steps=(int)newValue;  
										
											strcpy(theActiveHarmonyType.harmony_degrees_desc,"");
											for (int k=0;k<theActiveHarmonyType.num_harmony_steps;++k)
											{
												strcat(theActiveHarmonyType.harmony_degrees_desc,circle_of_fifths_arabic_degrees[theActiveHarmonyType.harmony_steps[k]]); 
												if (k<(theActiveHarmonyType.num_harmony_steps-1)) 
													strcat(theActiveHarmonyType.harmony_degrees_desc,"-");
											}

											setup_harmony();  // seems to work
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
									if (newValue!=theMeanderState.theHarmonyParms.target_octave)
									{
										theMeanderState.theHarmonyParms.target_octave=(int)newValue;  
										theMeanderState.theHarmonyParms.note_avg_target=theMeanderState.theHarmonyParms.target_octave/10.0;
										theMeanderState.theHarmonyParms.range_top=    theMeanderState.theHarmonyParms.note_avg_target + (theMeanderState.theHarmonyParms.note_octave_range/10.0);
										theMeanderState.theHarmonyParms.range_bottom= theMeanderState.theHarmonyParms.note_avg_target - (theMeanderState.theHarmonyParms.note_octave_range/10.0);
										theMeanderState.theHarmonyParms.r1=(theMeanderState.theHarmonyParms.range_top-theMeanderState.theHarmonyParms.range_bottom); 
										params[CONTROL_HARMONY_TARGETOCTAVE_PARAM].setValue(newValue);
									}
								}
								break;

							case IN_HARMONY_ALPHA_EXT_CV:
								if (fvalue>=0.01)
								{
									float newValue=(fvalue/10.0);
									newValue=clamp(newValue, 0., 1.);
									if (newValue!=theMeanderState.theHarmonyParms.alpha)
									{
										theMeanderState.theHarmonyParms.alpha=newValue;  
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
									if (newValue!=theMeanderState.theHarmonyParms.note_octave_range)
									{
										theMeanderState.theHarmonyParms.note_octave_range=newValue;  

										theMeanderState.theHarmonyParms.note_avg_target=theMeanderState.theHarmonyParms.target_octave/10.0;
										theMeanderState.theHarmonyParms.range_top=    theMeanderState.theHarmonyParms.note_avg_target + (theMeanderState.theHarmonyParms.note_octave_range/10.0);
										theMeanderState.theHarmonyParms.range_bottom= theMeanderState.theHarmonyParms.note_avg_target - (theMeanderState.theHarmonyParms.note_octave_range/10.0);
										theMeanderState.theHarmonyParms.r1=(theMeanderState.theHarmonyParms.range_top-theMeanderState.theHarmonyParms.range_bottom); 

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

									if (newValue!=theMeanderState.theHarmonyParms.note_length_divisor)
									{
										theMeanderState.theHarmonyParms.note_length_divisor=newValue;  
										params[CONTROL_HARMONY_DIVISOR_PARAM].setValue((float)exp);
									}
								}
								break;

							case IN_ENABLE_HARMONY_ALL7THS_EXT_CV:
								if (fvalue>0)
								{
									theMeanderState.theHarmonyParms.enable_all_7ths = true;
									theMeanderState.theHarmonyParms.enable_V_7ths=false;
									setup_harmony();  // calculate harmony notes
								}
								else
								if (fvalue==0)
								{
									theMeanderState.theHarmonyParms.enable_all_7ths = false;
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
									theMeanderState.theHarmonyParms.enable_V_7ths = true;
									theMeanderState.theHarmonyParms.enable_all_7ths=false;
									setup_harmony();  // calculate harmony notes
								}
								else
								if (fvalue==0)
								{
									theMeanderState.theHarmonyParms.enable_V_7ths = false;
									setup_harmony();  // calculate harmony notes
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
									theMeanderState.theHarmonyParms.enable_staccato = true;
								}
								else
								if (fvalue==0)
								{
									theMeanderState.theHarmonyParms.enable_staccato = false;
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

							// process melody input ports

							case IN_MELODY_ENABLE_EXT_CV:
								if (fvalue>0)
								{
									theMeanderState.theMelodyParms.enabled = true;
									theMeanderState.userControllingMelody=false;
								}
								else
								if (fvalue==0)
									theMeanderState.theMelodyParms.enabled = false;
								else
								if (fvalue<0) 
								{
									// Do nothing.  Allow local parameter control
								}
								break;

							case IN_MELODY_SCALE_DEGREE_EXT_CV:
													
								if (inputs[IN_MELODY_SCALE_DEGREE_EXT_CV].isConnected() && inputs[IN_MELODY_SCALE_GATE_EXT_CV].isConnected())
								{
									scaleDegree=inputs[IN_MELODY_SCALE_DEGREE_EXT_CV].getVoltage();
									gateValue=inputs[IN_MELODY_SCALE_GATE_EXT_CV].getVoltage();

									float octave=(float)((int)(scaleDegree));  // from the keyboard
									if (octave>3)
										octave=3;
									if (octave<-3)
										octave=-3;
									bool degreeChanged=false; // assume false unless determined true below
									bool skipStep=false;

								
									if ((gateValue==scaleDegree)&&(scaleDegree>=1)&&(scaleDegree<=7.7))  // MarkovSeq or other 1-7V degree  degree.octave 1.0-7.7V
									{
										if (inportStates[IN_MELODY_SCALE_DEGREE_EXT_CV].inTransition)
										{
											if (fvalue==inportStates[IN_MELODY_SCALE_DEGREE_EXT_CV].lastValue)
											{
												// was in transition but now is not
												inportStates[IN_MELODY_SCALE_DEGREE_EXT_CV].inTransition=false;
												inportStates[IN_MELODY_SCALE_DEGREE_EXT_CV].lastValue=fvalue;
												theMeanderState.theMelodyParms.lastMelodyDegreeIn=fvalue;
												octave = (int)(10.0*std::fmod(scaleDegree, 1.0f));
												if (octave>7)
													octave=7;
												scaleDegree=(float)((int)scaleDegree);
												degreeChanged=true;  // not really, but replay the note below
											}
											else
											if (fvalue!=inportStates[IN_MELODY_SCALE_DEGREE_EXT_CV].lastValue)
											{
												inportStates[IN_MELODY_SCALE_DEGREE_EXT_CV].lastValue=fvalue;
											}
										}
										else
										{
											if (fvalue!=inportStates[IN_MELODY_SCALE_DEGREE_EXT_CV].lastValue)
											{
												inportStates[IN_MELODY_SCALE_DEGREE_EXT_CV].inTransition=true;
												inportStates[IN_MELODY_SCALE_DEGREE_EXT_CV].lastValue=fvalue;
											}
										}
									}
									else
									if ((gateValue==scaleDegree)&&((scaleDegree<1.0)||(scaleDegree>=8.0)))  // MarkovSeq or other 1-7V degree  degree.octave 1.0-7.7V  <1 or >=8V means skip step
									{
										inportStates[IN_MELODY_SCALE_DEGREE_EXT_CV].lastValue=fvalue;
										degreeChanged=true;
										skipStep=true;
									}
									else  // keyboard  C-B
									if (!(gateValue==scaleDegree))
									{
										float fgvalue=inputs[IN_MELODY_SCALE_GATE_EXT_CV].getVoltage();
										if (inportStates[IN_MELODY_SCALE_GATE_EXT_CV].inTransition)
										{
											if (fgvalue==inportStates[IN_MELODY_SCALE_GATE_EXT_CV].lastValue)
											{
												// was in transition but now is not
												inportStates[IN_MELODY_SCALE_GATE_EXT_CV].inTransition=false;
												inportStates[IN_MELODY_SCALE_GATE_EXT_CV].lastValue=fgvalue;
												if (fgvalue)  // gate has gone high
												{
													if ( scaleDegree==inportStates[IN_MELODY_SCALE_DEGREE_EXT_CV].lastValue) // the gate has changed but the degree has not
														degreeChanged=true;   // not really, but play like it has so it will be replayed below
												}
											}
											else
											if (fgvalue!=inportStates[IN_MELODY_SCALE_GATE_EXT_CV].lastValue)
											{
												inportStates[IN_MELODY_SCALE_GATE_EXT_CV].lastValue=fgvalue;
											}
										}
										else
										{
											if (fgvalue!=inportStates[IN_MELODY_SCALE_GATE_EXT_CV].lastValue)
											{
												inportStates[IN_MELODY_SCALE_GATE_EXT_CV].inTransition=true;
												inportStates[IN_MELODY_SCALE_GATE_EXT_CV].lastValue=fgvalue;
											}
										}

										if ( (degreeChanged) || (scaleDegree!=inportStates[IN_MELODY_SCALE_DEGREE_EXT_CV].lastValue))
										{
											inportStates[IN_MELODY_SCALE_DEGREE_EXT_CV].lastValue= scaleDegree;
											octave=(float)((int)(scaleDegree));  // from the keyboard
											if (octave>3)
												octave=3;
											if (octave<-3)
												octave=-3;
											if (scaleDegree>=0)
												scaleDegree=(float)std::fmod(std::fabs(scaleDegree), 1.0f);
											else
												scaleDegree=-(float)std::fmod(std::fabs(scaleDegree), 1.0f);
											degreeChanged=true; 
											if (scaleDegree>=0)
											{
												if ((std::abs(scaleDegree)<.005f))   scaleDegree=1;
												else
												if ((std::abs(scaleDegree-.167f)<.005f))  scaleDegree=2;
												else
												if ((std::abs(scaleDegree-.333f)<.005f))  scaleDegree=3;
												else
												if ((std::abs(scaleDegree-.417f)<.005f))  scaleDegree=4;
												else
												if ((std::abs(scaleDegree-.583f)<.005f))  scaleDegree=5;
												else
												if ((std::abs(scaleDegree-.750f)<.005f))  scaleDegree=6;
												else
												if ((std::abs(scaleDegree-.917f)<.005f))  scaleDegree=7;
												else
													degreeChanged=false;
											}
											else
											{
												octave-=1;
												if ((std::abs(scaleDegree)<.005f))   scaleDegree=1;
												else
											    if (std::abs(std::abs(scaleDegree)-.083)<.005f)  scaleDegree=7;
												else
												if (std::abs(std::abs(scaleDegree)-.250)<.005f)  scaleDegree=6;
												else
											    if (std::abs(std::abs(scaleDegree)-.417)<.005f)  scaleDegree=5;
												else
											    if (std::abs(std::abs(scaleDegree)-.583)<.005f)  scaleDegree=4;
												else
											    if (std::abs(std::abs(scaleDegree)-.667)<.005f)  scaleDegree=3;
												else
											    if (std::abs(std::abs(scaleDegree)-.833)<.005f)  scaleDegree=2;
												else
													degreeChanged=false;
											}
											
										}	
									}

																
									if ((degreeChanged)&&(!skipStep))  
									{
										if (scaleDegree<1)
											scaleDegree=1;
										if (scaleDegree>7)
											scaleDegree=7;
																											
										if (scaleDegree>0)
										{
											userPlaysScaleDegreeMelody(scaleDegree, octave+theMeanderState.theMelodyParms.target_octave); 
											theMeanderState.theArpParms.note_count=0; 
										}
										
										if (running)
										{
											if (theMeanderState.theMelodyParms.enabled)
												theMeanderState.theMelodyParms.enabled = false;
											theMeanderState.userControllingMelody=true;
										}
										
									}
								}

								

								break;

							case IN_MELODY_VOLUME_EXT_CV:
								if (fvalue>=.01)
								if (fvalue!=theMeanderState.theMelodyParms.volume)
								{
									fvalue=clamp(fvalue, 0., 10.);
									theMeanderState.theMelodyParms.volume=fvalue;  
									params[CONTROL_MELODY_VOLUME_PARAM].setValue(fvalue);
									outputs[OUT_MELODY_VOLUME_OUTPUT].setVoltage(theMeanderState.theMelodyParms.volume);
								}
								break;

							case IN_MELODY_NOTE_LENGTH_DIVISOR_EXT_CV:
								if (fvalue>=.01)
								{
									float ratio=(fvalue/9.0);  // allow for CV that doesn't quite get to 10V
									int exp=(int)(ratio*5);
									exp=clamp(exp, 0, 5);
									int newValue=pow(2,exp);

									if (newValue!=theMeanderState.theMelodyParms.note_length_divisor)
									{
										theMeanderState.theMelodyParms.note_length_divisor=newValue;  
										params[CONTROL_MELODY_NOTE_LENGTH_DIVISOR_PARAM].setValue((float)exp);
									}
								}
								break;

							case IN_MELODY_TARGETOCTAVE_EXT_CV:
								if (fvalue>=.01)
								{
									float ratio=(fvalue/10.0);
									int newValue=1+ (int)(ratio*6);
									newValue=clamp(newValue, 1, 7);
									if (newValue!=theMeanderState.theMelodyParms.target_octave)
									{
										theMeanderState.theMelodyParms.target_octave=(int)newValue;  
										theMeanderState.theMelodyParms.note_avg_target=theMeanderState.theMelodyParms.target_octave/10.0;
										theMeanderState.theMelodyParms.range_top=    theMeanderState.theMelodyParms.note_avg_target + (theMeanderState.theMelodyParms.note_octave_range/10.0);
										theMeanderState.theMelodyParms.range_bottom= theMeanderState.theMelodyParms.note_avg_target - (theMeanderState.theMelodyParms.note_octave_range/10.0);
										theMeanderState.theMelodyParms.r1=(theMeanderState.theMelodyParms.range_top-theMeanderState.theMelodyParms.range_bottom); 
										params[CONTROL_MELODY_TARGETOCTAVE_PARAM].setValue(newValue);
									}
								}
								break;

							case IN_MELODY_ALPHA_EXT_CV:
								if (fvalue>=.01)
								{
									float newValue=(fvalue/10.0);
									newValue=clamp(newValue, 0., 1.);
									if (newValue!=theMeanderState.theMelodyParms.alpha)
									{
										theMeanderState.theMelodyParms.alpha=newValue;  
										params[CONTROL_MELODY_ALPHA_PARAM].setValue(newValue);
									}
								}
								break;
							
							case IN_MELODY_RANGE_EXT_CV:
								if (fvalue>=.01)
								{
									float ratio=(fvalue/10.0);
									float newValue=1+ratio*6;
									newValue=clamp(newValue, 1., 7.);
									if (newValue!=theMeanderState.theMelodyParms.note_octave_range)
									{
										theMeanderState.theMelodyParms.note_octave_range=newValue;  

										theMeanderState.theMelodyParms.note_avg_target=theMeanderState.theMelodyParms.target_octave/10.0;
										theMeanderState.theMelodyParms.range_top=    theMeanderState.theMelodyParms.note_avg_target + (theMeanderState.theMelodyParms.note_octave_range/10.0);
										theMeanderState.theMelodyParms.range_bottom= theMeanderState.theMelodyParms.note_avg_target - (theMeanderState.theMelodyParms.note_octave_range/10.0);
										theMeanderState.theMelodyParms.r1=(theMeanderState.theMelodyParms.range_top-theMeanderState.theMelodyParms.range_bottom); 

										params[CONTROL_MELODY_RANGE_PARAM].setValue(newValue);
									}
								}
								break;

							
							case IN_MELODY_DESTUTTER_EXT_CV:
								if (fvalue>0)
								{
									theMeanderState.theMelodyParms.destutter = true;
								}
								else
								if (fvalue==0)
								{
									theMeanderState.theMelodyParms.destutter = false;
								}
								else
								if (fvalue<0) 
								{
									// Do nothing.  Allow local parameter control
								}
								break;
							

							case IN_ENABLE_MELODY_STACCATO_EXT_CV:
								if (fvalue>0)
								{
									theMeanderState.theMelodyParms.enable_staccato = true;
								}
								else
								if (fvalue==0)
								{
									theMeanderState.theMelodyParms.enable_staccato = false;
								}
								else
								if (fvalue<0) 
								{
									// Do nothing.  Allow local parameter control
								}
								break;
							
							case IN_ENABLE_MELODY_CHORDAL_EXT_CV:
								if (fvalue>0)
								{
									theMeanderState.theMelodyParms.chordal = true;
									theMeanderState.theMelodyParms.scaler = false;
								}
								else
								if (fvalue==0)
								{
									theMeanderState.theMelodyParms.chordal = false;
									theMeanderState.theMelodyParms.scaler = true;
								}
								else
								if (fvalue<0) 
								{
									// Do nothing.  Allow local parameter control
								}
								break;

							case IN_MELODY_SCALER_EXT_CV:
								if (fvalue>0)
								{
									theMeanderState.theMelodyParms.scaler = true;
									theMeanderState.theMelodyParms.chordal = false;
								}
								else
								if (fvalue==0)
								{
									theMeanderState.theMelodyParms.scaler = false;
									theMeanderState.theMelodyParms.chordal = true;
								}
								else
								if (fvalue<0) 
								{
									// Do nothing.  Allow local parameter control
								}
								break;
							

							// process arp input ports

							case IN_ARP_ENABLE_EXT_CV:
								if (fvalue>0)
									theMeanderState.theArpParms.enabled = true;
								else
								if (fvalue==0)
									theMeanderState.theArpParms.enabled = false;
								else
								if (fvalue<0) 
								{
									// Do nothing.  Allow local parameter control
								}
								break;

							case IN_ARP_COUNT_EXT_CV:
								if (fvalue>=.01)
								{
									float ratio=(fvalue/10.0);
									int newValue=(int)(ratio*31);
									newValue=clamp(newValue, 0, 31);
									
									if (newValue!=theMeanderState.theArpParms.count)
									{
										theMeanderState.theArpParms.count=newValue;  
										params[CONTROL_ARP_COUNT_PARAM].setValue((float)newValue);
									}
								}
								break;


							case IN_ARP_INCREMENT_EXT_CV:
								if (fvalue>=.01)
								{
									float ratio=(fvalue/9.0);  // allow for CV that doesn't quite get to 10.0
									int exp=(int)(ratio*3);
									exp=clamp(exp, 0, 3)+2;
									int newValue=pow(2,exp);

									if (newValue!=theMeanderState.theArpParms.note_length_divisor)
									{
										theMeanderState.theArpParms.note_length_divisor=newValue;  
										params[CONTROL_ARP_INCREMENT_PARAM].setValue((float)exp);
									}
								}
								break;

							case IN_ARP_DECAY_EXT_CV:
								if (fvalue>=.01)
								{
									float newValue=(fvalue/10.0);
									newValue=clamp(newValue, 0., 1.);
									if (newValue!=theMeanderState.theArpParms.decay)
									{
										theMeanderState.theArpParms.decay=newValue;  
										params[CONTROL_ARP_DECAY_PARAM].setValue(newValue);
									}
								}
								break;

							case IN_ENABLE_ARP_CHORDAL_EXT_CV:
								if (fvalue>0)
								{
									theMeanderState.theArpParms.chordal = true;
									theMeanderState.theArpParms.scaler = false;
								}
								else
								if (fvalue==0)
								{
									theMeanderState.theArpParms.chordal = false;
									theMeanderState.theArpParms.scaler = true;
								}
								else
								if (fvalue<0) 
								{
									// Do nothing.  Allow local parameter control
								}
								break;

							case IN_ENABLE_ARP_SCALER_EXT_CV:
								if (fvalue>0)
								{
									theMeanderState.theArpParms.scaler = true;
									theMeanderState.theArpParms.chordal = false;
								}
								else
								if (fvalue==0)
								{
									theMeanderState.theArpParms.scaler = false;
									theMeanderState.theArpParms.chordal = true;
								}
								else
								if (fvalue<0) 
								{
									// Do nothing.  Allow local parameter control
								}
								break;

							case IN_ARP_PATTERN_EXT_CV:  // issue with range of -3 to +3
							    if (true)  // just for local variable scope
								{
									float ratio=(fvalue/9.0);  // handle CV's that do not quite make it to 10.0V
									int newValue=(int)(ratio*4);
									newValue -= 2;
									newValue=clamp(newValue, -2, 2);
									if (newValue!=theMeanderState.theArpParms.pattern)
									{
										theMeanderState.theArpParms.pattern=newValue;  
										params[CONTROL_ARP_PATTERN_PARAM].setValue(newValue);
									}
								}

							    break;

							
							// process bass input ports

							case IN_BASS_ENABLE_EXT_CV:
								if (fvalue>0)
									theMeanderState.theBassParms.enabled = true;
								else
								if (fvalue==0)
									theMeanderState.theBassParms.enabled = false;
								else
								if (fvalue<0) 
								{
									// Do nothing.  Allow local parameter control
								}
								break;

							case IN_BASS_VOLUME_EXT_CV:
								if (fvalue>=.01)
								if (fvalue!=theMeanderState.theBassParms.volume)
								{
									fvalue=clamp(fvalue, 0., 10.);
									theMeanderState.theBassParms.volume=fvalue;  
									params[CONTROL_BASS_VOLUME_PARAM].setValue(fvalue);
									outputs[OUT_BASS_VOLUME_OUTPUT].setVoltage(theMeanderState.theBassParms.volume);
								}
								break;

							case IN_BASS_TARGETOCTAVE_EXT_CV:
								if (fvalue>=.01)
								{
									float ratio=(fvalue/10.0);
									int newValue=1+ (int)(ratio*6);
									newValue=clamp(newValue, 1, 7);
									if (newValue!=theMeanderState.theBassParms.target_octave)
									{
										theMeanderState.theBassParms.target_octave=(int)newValue;  
										params[CONTROL_BASS_TARGETOCTAVE_PARAM].setValue(newValue);
									}
								}
								break;

							case IN_BASS_DIVISOR_EXT_CV:
								if (fvalue>=.01)
								{
									float ratio=(fvalue/9.0); // allow for CV that doesn't quite get to 10.0
									int exp=(int)(ratio*3);
									exp=clamp(exp, 0, 3);
									int newValue=pow(2,exp);

									if (newValue!=theMeanderState.theBassParms.note_length_divisor)
									{
										theMeanderState.theBassParms.note_length_divisor=newValue;  
										params[CONTROL_BASS_DIVISOR_PARAM].setValue((float)exp);
									}
								}
								break;
							
							case IN_ENABLE_BASS_STACCATO_EXT_CV:
								if (fvalue>0)
								{
									theMeanderState.theBassParms.enable_staccato = true;
								}
								else
								if (fvalue==0)
								{
									theMeanderState.theBassParms.enable_staccato = false;
								}
								else
								if (fvalue<0) 
								{
									// Do nothing.  Allow local parameter control
								}
								break;

							case IN_BASS_ACCENT_EXT_CV:
								if (fvalue>0)
								{
									theMeanderState.theBassParms.accent = true;
								}
								else
								if (fvalue==0)
								{
									theMeanderState.theBassParms.accent = false;
								}
								else
								if (fvalue<0) 
								{
									// Do nothing.  Allow local parameter control
								}
								break;


							case IN_BASS_SYNCOPATE_EXT_CV:
								if (fvalue>0)
								{
									theMeanderState.theBassParms.syncopate = true;
								}
								else
								if (fvalue==0)
								{
									theMeanderState.theBassParms.syncopate = false;
								}
								else
								if (fvalue<0) 
								{
									// Do nothing.  Allow local parameter control
								}
								break;

							case IN_BASS_SHUFFLE_EXT_CV:
								if (fvalue>0)
								{
									theMeanderState.theBassParms.shuffle = true;
								}
								else
								if (fvalue==0)
								{
									theMeanderState.theBassParms.shuffle = false;
								}
								else
								if (fvalue<0) 
								{
									// Do nothing.  Allow local parameter control
								}
								break;

							case IN_BASS_OCTAVES_EXT_CV:
								if (fvalue>0)
								{
									theMeanderState.theBassParms.octave_enabled = true;
								}
								else
								if (fvalue==0)
								{
									theMeanderState.theBassParms.octave_enabled = false;
								}
								else
								if (fvalue<0) 
								{
									// Do nothing.  Allow local parameter control
								}
								break;

							// process fBn input ports	

							case IN_HARMONY_FBM_OCTAVES_EXT_CV:
							if (fvalue>=.01)
								{
									float ratio=(fvalue/10.0);
									int newValue=1+(int)(ratio*5);
									newValue=clamp(newValue, 1, 6);
									
									if (newValue!=theMeanderState.theHarmonyParms.noctaves)
									{
										theMeanderState.theHarmonyParms.noctaves=newValue;  
										params[CONTROL_HARMONY_FBM_OCTAVES_PARAM].setValue((float)newValue);
									}
								}
								break;

							case IN_HARMONY_FBM_PERIOD_EXT_CV:
							if (fvalue>=.01)
								{
									float ratio=(fvalue/10.0);
									int newValue=1+(int)(ratio*100);
									newValue=clamp(newValue, 1, 100);
									
									if (newValue!=theMeanderState.theHarmonyParms.period)
									{
										theMeanderState.theHarmonyParms.period=newValue;  
										params[CONTROL_HARMONY_FBM_PERIOD_PARAM].setValue((float)newValue);
									}
								}
								break;

							case IN_MELODY_FBM_OCTAVES_EXT_CV:
							if (fvalue>=.01)
								{
									float ratio=(fvalue/10.0);
									int newValue=1+(int)(ratio*5);
									newValue=clamp(newValue, 1, 6);
									
									if (newValue!=theMeanderState.theMelodyParms.noctaves)
									{
										theMeanderState.theMelodyParms.noctaves=newValue;  
										params[CONTROL_MELODY_FBM_OCTAVES_PARAM].setValue((float)newValue);
									}
								}
								break;

							case IN_MELODY_FBM_PERIOD_EXT_CV:
							if (fvalue>=.01)
								{
									float ratio=(fvalue/10.0);
									int newValue=1+(int)(ratio*100);
									newValue=clamp(newValue, 1, 100);
									
									if (newValue!=theMeanderState.theMelodyParms.period)
									{
										theMeanderState.theMelodyParms.period=newValue;  
										params[CONTROL_MELODY_FBM_PERIOD_PARAM].setValue((float)newValue);
									}
								}
								break;

							case IN_ARP_FBM_OCTAVES_EXT_CV:
							if (fvalue>=.01)
								{
									float ratio=(fvalue/10.0);
									int newValue=1+(int)(ratio*5);
									newValue=clamp(newValue, 1, 6);
									
									if (newValue!=theMeanderState.theArpParms.noctaves)
									{
										theMeanderState.theArpParms.noctaves=newValue;  
										params[CONTROL_ARP_FBM_OCTAVES_PARAM].setValue((float)newValue);
									}
								}
								break;

							case IN_ARP_FBM_PERIOD_EXT_CV:
							if (fvalue>=.01)
								{
									float ratio=(fvalue/10.0);
									int newValue=1+(int)(ratio*100);
									newValue=clamp(newValue, 1, 100);
									
									if (newValue!=theMeanderState.theArpParms.period)
									{
										theMeanderState.theArpParms.period=newValue;  
										params[CONTROL_ARP_FBM_PERIOD_PARAM].setValue((float)newValue);
									}
								}
								break;

						    // handle mode and root input changes
						  
						    case IN_ROOT_KEY_EXT_CV:
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
					        	break;

							case IN_SCALE_EXT_CV:
						    if (fvalue>=.01)
								{
									float ratio=(fvalue/10.0);
									int newValue=(int)(ratio*6);
									newValue=clamp(newValue, 0, 6);
									if (newValue!=mode)
									{
										mode=(int)newValue;
										params[CONTROL_SCALE_PARAM].setValue(mode);
										circleChanged=true;
									}
								}
					        	break;

						};  // end switch
					}
				}
			}
		
			// harmony params

			fvalue=(params[CONTROL_HARMONY_VOLUME_PARAM].getValue());
			if (fvalue!=theMeanderState.theHarmonyParms.volume)
			{
				theMeanderState.theHarmonyParms.volume=fvalue;  
				outputs[OUT_HARMONY_VOLUME_OUTPUT].setVoltage(theMeanderState.theHarmonyParms.volume);
			}

			fvalue=std::round(params[CONTROL_HARMONY_TARGETOCTAVE_PARAM].getValue());
			if (fvalue!=theMeanderState.theHarmonyParms.target_octave)
			{
				theMeanderState.theHarmonyParms.target_octave=fvalue;  
				theMeanderState.theHarmonyParms.note_avg_target=theMeanderState.theHarmonyParms.target_octave/10.0;
				theMeanderState.theHarmonyParms.range_top=    theMeanderState.theHarmonyParms.note_avg_target + (theMeanderState.theHarmonyParms.note_octave_range/10.0);
				theMeanderState.theHarmonyParms.range_bottom= theMeanderState.theHarmonyParms.note_avg_target - (theMeanderState.theHarmonyParms.note_octave_range/10.0);
				theMeanderState.theHarmonyParms.r1=(theMeanderState.theHarmonyParms.range_top-theMeanderState.theHarmonyParms.range_bottom); 
			}

			
			fvalue=params[CONTROL_HARMONY_DIVISOR_PARAM].getValue();
			ivalue=(int)fvalue;
			ivalue=pow(2,ivalue);
			if ((ivalue)!=theMeanderState.theHarmonyParms.note_length_divisor)
			{
				theMeanderState.theHarmonyParms.note_length_divisor=ivalue;  
			}

			fvalue=(params[CONTROL_HARMONY_RANGE_PARAM].getValue());
			if (fvalue!=theMeanderState.theMelodyParms.note_octave_range)
			{
				theMeanderState.theHarmonyParms.note_octave_range=fvalue;  
				theMeanderState.theHarmonyParms.note_avg_target=theMeanderState.theHarmonyParms.target_octave/10.0;
				theMeanderState.theHarmonyParms.range_top=    theMeanderState.theHarmonyParms.note_avg_target + (theMeanderState.theHarmonyParms.note_octave_range/10.0);
				theMeanderState.theHarmonyParms.range_bottom= theMeanderState.theHarmonyParms.note_avg_target - (theMeanderState.theHarmonyParms.note_octave_range/10.0);
				theMeanderState.theHarmonyParms.r1=(theMeanderState.theHarmonyParms.range_top-theMeanderState.theHarmonyParms.range_bottom); 
			}

			fvalue=(params[CONTROL_HARMONY_ALPHA_PARAM].getValue());
			if ((fvalue)!=theMeanderState.theHarmonyParms.alpha)
			{
				theMeanderState.theHarmonyParms.alpha=fvalue;  
			}

			fvalue=params[CONTROL_HARMONY_FBM_OCTAVES_PARAM].getValue();
			if (fvalue!=theMeanderState.theHarmonyParms.noctaves)
			{
				theMeanderState.theHarmonyParms.noctaves=fvalue;  
			}

			fvalue=params[CONTROL_HARMONY_FBM_PERIOD_PARAM].getValue();
			if (fvalue!=theMeanderState.theHarmonyParms.period)
			{
				theMeanderState.theHarmonyParms.period=fvalue;  
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
						strcat(theActiveHarmonyType.harmony_degrees_desc,circle_of_fifths_arabic_degrees[theActiveHarmonyType.harmony_steps[k]]); 
						if (k<(theActiveHarmonyType.num_harmony_steps-1)) 
							strcat(theActiveHarmonyType.harmony_degrees_desc,"-");
					}
														
					strcpy(theHarmonyTypes[harmony_type].harmony_degrees_desc,"");
					for (int k=0;k<theHarmonyTypes[harmony_type].num_harmony_steps;++k)
					{
						strcat(theHarmonyTypes[harmony_type].harmony_degrees_desc,circle_of_fifths_arabic_degrees[theHarmonyTypes[harmony_type].harmony_steps[k]]); 
						if (k<(theHarmonyTypes[harmony_type].num_harmony_steps-1)) 
							strcat(theHarmonyTypes[harmony_type].harmony_degrees_desc,"-");
					}
					
					savedHarmonySteps = 0;  // no longer want to apply this elsewhere 

					setup_harmony();  // seems to work
				}
				//
			}

			// Melody Params

			fvalue=(params[CONTROL_MELODY_VOLUME_PARAM].getValue());
			if (fvalue!=theMeanderState.theMelodyParms.volume)
			{
				theMeanderState.theMelodyParms.volume=fvalue;  
				outputs[OUT_MELODY_VOLUME_OUTPUT].setVoltage(theMeanderState.theMelodyParms.volume);
			}
			
			fvalue=std::round(params[CONTROL_MELODY_TARGETOCTAVE_PARAM].getValue());
			if (fvalue!=theMeanderState.theMelodyParms.target_octave)
			{
				theMeanderState.theMelodyParms.target_octave=fvalue;  
				theMeanderState.theMelodyParms.note_avg_target=theMeanderState.theMelodyParms.target_octave/10.0;
				theMeanderState.theMelodyParms.range_top=    theMeanderState.theMelodyParms.note_avg_target + (theMeanderState.theMelodyParms.note_octave_range/10.0);
				theMeanderState.theMelodyParms.range_bottom= theMeanderState.theMelodyParms.note_avg_target - (theMeanderState.theMelodyParms.note_octave_range/10.0);
				theMeanderState.theMelodyParms.r1=(theMeanderState.theMelodyParms.range_top-theMeanderState.theMelodyParms.range_bottom); 
			}

			fvalue=(params[CONTROL_MELODY_RANGE_PARAM].getValue());
			if (fvalue!=theMeanderState.theMelodyParms.note_octave_range)
			{
				theMeanderState.theMelodyParms.note_octave_range=fvalue;  
				theMeanderState.theMelodyParms.note_avg_target=theMeanderState.theMelodyParms.target_octave/10.0;
				theMeanderState.theMelodyParms.range_top=    theMeanderState.theMelodyParms.note_avg_target + (theMeanderState.theMelodyParms.note_octave_range/10.0);
				theMeanderState.theMelodyParms.range_bottom= theMeanderState.theMelodyParms.note_avg_target - (theMeanderState.theMelodyParms.note_octave_range/10.0);
				theMeanderState.theMelodyParms.r1=(theMeanderState.theMelodyParms.range_top-theMeanderState.theMelodyParms.range_bottom); 
			}

			fvalue=(params[CONTROL_MELODY_ALPHA_PARAM].getValue());
			if ((fvalue)!=theMeanderState.theMelodyParms.alpha)
			{
				theMeanderState.theMelodyParms.alpha=fvalue;  
			}
			
			fvalue=params[CONTROL_MELODY_NOTE_LENGTH_DIVISOR_PARAM].getValue();
			ivalue=(int)fvalue;
			ivalue=pow(2,ivalue);
			if ((ivalue)!=theMeanderState.theMelodyParms.note_length_divisor)
			{
				theMeanderState.theMelodyParms.note_length_divisor=ivalue;  
			}

			fvalue=params[CONTROL_MELODY_FBM_OCTAVES_PARAM].getValue();
			if (fvalue!=theMeanderState.theMelodyParms.noctaves)
			{
				theMeanderState.theMelodyParms.noctaves=fvalue;  
			}

			fvalue=params[CONTROL_MELODY_FBM_PERIOD_PARAM].getValue();
			if (fvalue!=theMeanderState.theMelodyParms.period)
			{
				theMeanderState.theMelodyParms.period=fvalue;  
			}

           														
			// bass params***********

			fvalue=(params[CONTROL_BASS_VOLUME_PARAM].getValue());
			if (fvalue!=theMeanderState.theBassParms.volume)
			{
				theMeanderState.theBassParms.volume=fvalue;  
				outputs[OUT_BASS_VOLUME_OUTPUT].setVoltage(theMeanderState.theBassParms.volume);
			}

			fvalue=params[CONTROL_BASS_DIVISOR_PARAM].getValue();
			ivalue=(int)fvalue;
			ivalue=pow(2,ivalue);
			if ((ivalue)!=theMeanderState.theBassParms.note_length_divisor)
			{
				theMeanderState.theBassParms.note_length_divisor=ivalue;  
			}


			fvalue=std::round(params[CONTROL_BASS_TARGETOCTAVE_PARAM].getValue());
			if (fvalue!=theMeanderState.theBassParms.target_octave)
			{
				theMeanderState.theBassParms.target_octave=fvalue;  
			}

			
			fvalue=std::round(params[CONTROL_ARP_COUNT_PARAM].getValue());
			if (fvalue!=theMeanderState.theArpParms.count)
			{
				theMeanderState.theArpParms.count=fvalue;  
			}

			fvalue=std::round(params[CONTROL_ARP_INCREMENT_PARAM].getValue());
			ivalue=(int)fvalue;
			ivalue=pow(2,ivalue);
			if (ivalue!=theMeanderState.theArpParms.note_length_divisor)
			{
				theMeanderState.theArpParms.note_length_divisor=ivalue;  
			}

			fvalue=(params[	CONTROL_ARP_DECAY_PARAM].getValue());
			if (fvalue!=theMeanderState.theArpParms.decay)
			{
				theMeanderState.theArpParms.decay=fvalue;  
			}

			fvalue=std::round(params[CONTROL_ARP_PATTERN_PARAM].getValue());
			if (fvalue!=theMeanderState.theArpParms.pattern)
			{
				theMeanderState.theArpParms.pattern=fvalue;  
			}

			fvalue=params[CONTROL_ARP_FBM_OCTAVES_PARAM].getValue();
			if (fvalue!=theMeanderState.theArpParms.noctaves)
			{
				theMeanderState.theArpParms.noctaves=fvalue;  
			}

			fvalue=params[CONTROL_ARP_FBM_PERIOD_PARAM].getValue();
			if (fvalue!=theMeanderState.theArpParms.period)
			{
				theMeanderState.theArpParms.period=fvalue;  
			}

			// **************************

			if (harmonyPresetChanged) 
			{
				harmony_type=harmonyPresetChanged;
			 // copyHarmonyTypeToActiveHarmonyType(harmony_type);
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
			//	params[CONTROL_HARMONY_STEPS_PARAM].setValue(theHarmonyTypes[harmony_type].num_harmony_steps);
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
						strcpy(note_desig[i], note_desig_flats[i]);
				}
				else
				{
					for (int i=0; i<12; ++i)
						strcpy(note_desig[i], note_desig_sharps[i]);
				} 
				
				ConstructCircle5ths(circle_root_key, mode);
				ConstructDegreesSemicircle(circle_root_key, mode); //int circleroot_key, int mode)
			
				init_notes();  // depends on mode and root_key		
			// 	init_harmony();  // sets up original progressions
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

	~Meander() 
	{
	}
  
 
	Meander() 
	{
		time_t rawtime; 
  		time( &rawtime );
   		
		
		lowFreqClock.setDivision(512);  // every 86 samples, 2ms
		sec1Clock.setDivision(44000);
		lightDivider.setDivision(512);  // every 86 samples, 2ms
				   		
		
		initPerlin();
		ConfigureModuleVars();
		MeanderMusicStructuresInitialize();  // sets module moduleVarsInitialized=true

			
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

		for (int i=0; i<12; ++i)
			lights[LIGHT_CIRCLE_ROOT_KEY_POSITION_1_LIGHT+i].setBrightness(0.0f);
		lights[LIGHT_CIRCLE_ROOT_KEY_POSITION_1_LIGHT+root_key].setBrightness(1.0f);  // loaded root_key might not be 0/C
		
		CircleStepStates[0]=1.0f;
		lights[LIGHT_LEDBUTTON_CIRCLESTEP_1].setBrightness(1.0f);
		
		CircleStepSetStates[0]=1.0f;
		lights[LIGHT_LEDBUTTON_CIRCLESETSTEP_1].setBrightness(1.0f);

//****************** 58 in ports
		configInput(IN_RUN_EXT_CV, "Mono CV Ext. Run Toggle: >0v :");
		configInput(IN_RESET_EXT_CV, "Mono CV Ext. Reset: >0v :");
		configInput(IN_TEMPO_EXT_CV, "Mono CV Ext. Tempo Set: +-v/oct  0v=120 BPM :");
		configInput(IN_TIMESIGNATURETOP_EXT_CV, "Mono CV Ext. Time Signature Top Set: 0.1v-10v=2-15 :");
		configInput(IN_TIMESIGNATUREBOTTOM_EXT_CV, "Mono CV Ext. Time Signature Bottom Set: 0.1v-10v=2-16 :");
		configInput(IN_ROOT_KEY_EXT_CV, "Mono CV Ext. Root Set: 0.1v-10v=C,G,D,A,E,B,F#,Db,Ab,Eb,Bb,F :");
		configInput(IN_SCALE_EXT_CV, "Mono CV Ext. Mode Set: 0.1v-10v=Lydian,Ionian,Mixolydian,Dorian,Aeolian,Phrygian,Locrian :");
		configInput(IN_CLOCK_EXT_CV, "Mono Ext. 8x Clock In: overrides Meander internal clock :");
		configInput(IN_HARMONY_CIRCLE_DEGREE_EXT_CV, "Mono CV Ext. Circle Degree Set: Octal radix (degree.octave) 1.1v-7.7v :");
		configInput(IN_HARMONY_CIRCLE_DEGREE_GATE_EXT_CV, "Mono Ext. Circle Degree Gate In: 10v or duplicate of Circle Degree input cable :");
		configInput(IN_MELODY_ENABLE_EXT_CV, "Mono CV Ext. Melody Enable Toggle: >0v :");			
		configInput(IN_MELODY_VOLUME_EXT_CV, "Mono CV Ext. Melody Volume Set: 0-10v :");
		configInput(IN_MELODY_DESTUTTER_EXT_CV, "Mono CV Ext. Melody Hold Tied Notes Toggle: >0v :");
		configInput(IN_MELODY_NOTE_LENGTH_DIVISOR_EXT_CV, "Mono CV Ext. Melody Note On 1/n Set: 0.1v-10v=n=1,2,4,8,16,32 :");
		configInput(IN_MELODY_TARGETOCTAVE_EXT_CV, "Mono CV Ext. Melody Target Octave Set: 0.1v-10v=n=1-7 :");
		configInput(IN_MELODY_ALPHA_EXT_CV, "Mono CV Ext. Melody Variability Set: 0.1v-10v=0-1.0 :");
		configInput(IN_MELODY_RANGE_EXT_CV, "Mono CV Ext. Melody Octave Range Set: 0.1v-10v=+/- 0-3.0 :");
		configInput(IN_ARP_ENABLE_EXT_CV, "Mono CV Ext. Melody Arp Enable Toggle: >0v :");
		configInput(IN_ARP_COUNT_EXT_CV, "Mono CV Ext. Melody Arp steps Set: 0.1v-10v=n=0-31 :");
		configInput(IN_ARP_INCREMENT_EXT_CV, "Mono CV Ext. Melody Arp Note On 1/n Set: 0.1v-10v=n=4,8,16,32 :");
		configInput(IN_ARP_DECAY_EXT_CV, "Mono CV Ext. Melody Arp Note Volume Decay Set: 0.1v-10v=n=0-1.0 :");
		configInput(IN_ARP_PATTERN_EXT_CV, "Mono CV Ext. Arp Step Pattern Set: 0.1v-10v= (-1,+1),(-1),(0),(+1,-1) :");
		configInput(IN_ENABLE_ARP_CHORDAL_EXT_CV, "Mono CV Ext. Melody Arp Chordal Notes Enable Toggle: >0v :");
		configInput(IN_ENABLE_ARP_SCALER_EXT_CV, "Mono CV Ext. Melody Arp Scaler Notes Enable Toggle: >0v :");
		configInput(IN_HARMONY_ENABLE_EXT_CV, "Mono CV Ext. Harmony Enable Toggle: >0v :");
		configInput(IN_HARMONY_DESTUTTER_EXT_CV, "Mono CV Ext. Harmony Hold Tied Notes Toggle: >0v :");
		configInput(IN_HARMONY_VOLUME_EXT_CV, "Mono CV Ext. Harmony Volume Set: 0-10v :");
		configInput(IN_HARMONY_STEPS_EXT_CV, "Mono CV Ext. Harmony Progression Steps Set: 0.1v-10v=n=1-N :");
		configInput(IN_HARMONY_TARGETOCTAVE_EXT_CV, "Mono CV Ext. Harmony Target Octave Set: 0.1v-10v=n=1-7 :");
		configInput(IN_HARMONY_ALPHA_EXT_CV, "Mono CV Ext. Harmony Variability Set: 0.1v-10v=0-1.0 :");
		configInput(IN_HARMONY_RANGE_EXT_CV, "Mono CV Ext. Harmony Octave Range Set: 0.1v-10v=+/- 0-3.0 :");
		configInput(IN_HARMONY_DIVISOR_EXT_CV, "Mono CV Ext. Harmony Note On 1/n Set: 0.1v-10v=n=1,2,4,8 :");
		configInput(IN_HARMONYPRESETS_EXT_CV, "Mono CV Ext. Harmony Progression Preset Set: 0.1v-10v= 1-~60 progression number :");
		configInput(IN_BASS_ENABLE_EXT_CV, "Mono CV Ext. Bass Enable Toggle: >0v :");
		configInput(IN_BASS_VOLUME_EXT_CV, "Mono CV Ext. Bass Volume Set: 0-10v :");
		configInput(IN_BASS_TARGETOCTAVE_EXT_CV, "Mono CV Ext. Bass Target Octave Set: 0.1v-10v=n=1-7 :");
		configInput(IN_BASS_ACCENT_EXT_CV, "Mono CV Ext. Bass Accent Enable Toggle: >0v :");
		configInput(IN_BASS_SYNCOPATE_EXT_CV, "Mono CV Ext. Bass Syncopate Enable Toggle: >0v :");
		configInput(IN_BASS_SHUFFLE_EXT_CV, "Mono CV Ext. Bass Shuffle Enable Toggle: >0v :");
		configInput(IN_BASS_DIVISOR_EXT_CV, "Mono CV Ext. Bass Note On 1/n Set: 0.1v-10v=n=1,2,4,8 :");
		configInput(IN_HARMONY_FBM_OCTAVES_EXT_CV, "Mono CV Ext. Harmony fBm Generator Octaves Set: 0.1v-10v=1-6 :");
		configInput(IN_MELODY_FBM_OCTAVES_EXT_CV, "Mono CV Ext. Melody fBm Generator Octaves Set: 0.1v-10v=1-6 :");
		configInput(IN_ARP_FBM_OCTAVES_EXT_CV, "Mono CV Ext. Arp/32nds fBm Generator Octaves Set: 0.1v-10v=1-6 :");
		configInput(IN_HARMONY_FBM_PERIOD_EXT_CV, "Mono CV Ext. Harmony fBm Generator Period Seconds Set: 0.1v-10v=1-100 :");
		configInput(IN_MELODY_FBM_PERIOD_EXT_CV, "Mono CV Ext. Melody fBm Generator Period Seconds Set: 0.1v-10v=1-100 :");
		configInput(IN_ARP_FBM_PERIOD_EXT_CV, "Mono CV Ext. Arp/32nds fBm Generator Period Seconds Set: 0.1v-10v=1-100 :");
		configInput(IN_ENABLE_MELODY_CHORDAL_EXT_CV, "Mono CV Ext. Melody Chordal Notes Enable Toggle: >0v :");
		configInput(IN_MELODY_SCALER_EXT_CV, "Mono CV Ext. Melody Scaler Notes Enable Toggle: >0v :");
		configInput(IN_ENABLE_HARMONY_ALL7THS_EXT_CV, "Mono CV Ext. Harmony Nice 7ths Enable Toggle: >0v :");
		configInput(IN_ENABLE_HARMONY_V7THS_EXT_CV, "Mono CV Ext. Harmony V7ths Enable Toggle: >0v :");
		configInput(IN_ENABLE_HARMONY_STACCATO_EXT_CV, "Mono CV Ext. Harmony Staccato Enable Toggle: >0v :");
		configInput(IN_ENABLE_MELODY_STACCATO_EXT_CV, "Mono CV Ext. Melody Staccato Enable Toggle: >0v :");
		configInput(IN_ENABLE_BASS_STACCATO_EXT_CV, "Mono CV Ext. Bass Staccato Enable Toggle: >0v :");
		configInput(IN_BASS_OCTAVES_EXT_CV, "Mono CV Ext. Bass Play Octaves (x2) Enable Toggle: >0v :");
		configInput(IN_PROG_STEP_EXT_CV, "Mono CV Ext. Progression Step Advance: >0v :");
		configInput(IN_MELODY_SCALE_DEGREE_EXT_CV, "Mono CV Ext. Melody Scale Degree Set: Octal radix (degree.octave) 1.1v-7.7v :");
		configInput(IN_MELODY_SCALE_GATE_EXT_CV, "Mono Ext. Melody Scale Degree Gate In: 10v or duplicate of Circle Degree input cable :");
		configInput(IN_POLY_QUANT_EXT_CV, "Poly External Note(s) To Quantize In : v/oct");

//**************** 27 out ports, 24 used

        configOutput(OUT_RUN_OUT, "Mono Run Enable Ext. Momentary: 10v :");
		configOutput(OUT_RESET_OUT, "Mono Reset Enable Ext. Momentary: 10v :");
		configOutput(OUT_TEMPO_OUT, "Mono CV Ext. Tempo : +-v/oct  0v=120 BPM :");
		configOutput(OUT_CLOCK_OUT, "Mono 8x Clock Out:  :"); 
		configOutput(OUT_MELODY_GATE_OUTPUT, "Mono Melody Gate Out: 10v :");
		configOutput(OUT_HARMONY_GATE_OUTPUT, "Mono Harmony Gate Out: 10v :");
		configOutput(OUT_BASS_GATE_OUTPUT, "Mono Bass Gate Out: 10v :");
		configOutput(OUT_FBM_HARMONY_OUTPUT, "Mono Harmony fBm Noise Out: 0-10.0v :");
		configOutput(OUT_MELODY_CV_OUTPUT, "Mono Melody Notes Out: v/oct :");
		configOutput(OUT_FBM_MELODY_OUTPUT, "Mono Melody fBm Noise Out: 0-10.0v :");
		configOutput(OUT_BASS_CV_OUTPUT, "Poly Bass Notes Out: v/oct :");
		configOutput(OUT_HARMONY_CV_OUTPUT, "Poly Harmony Chord Notes Out: v/oct :");
		configOutput(OUT_CLOCK_BEATX2_OUTPUT, "Mono BeatX2 1ms Trigger/Clock Out: 10v :");
		configOutput(OUT_CLOCK_BAR_OUTPUT, "Mono Bsr 1ms Trigger/Clock Out: 10v :");
		configOutput(OUT_CLOCK_BEATX4_OUTPUT, "Mono BeatX4 1ms Trigger/Clock Out: 10v :");
		configOutput(OUT_CLOCK_BEATX8_OUTPUT, "Mono BeatX8 1ms Trigger/Clock Out: 10v :");
		configOutput(OUT_CLOCK_BEAT_OUTPUT, "Mono Beat 1ms Trigger/Clock Out: 10v :");
		configOutput(OUT_BASS_TRIGGER_OUTPUT, "Mono ");  // not used
		configOutput(OUT_HARMONY_TRIGGER_OUTPUT, "Mono ");  // not used
		configOutput(OUT_MELODY_TRIGGER_OUTPUT, "Mono ");  // not used
		configOutput(OUT_FBM_ARP_OUTPUT, "Mono  Melody Arp/32nds fBm Noise Out: 0-10.0v :");
		configOutput(OUT_MELODY_VOLUME_OUTPUT, "Mono CV Melody Volume Out: 0-10v :");
		configOutput(OUT_HARMONY_VOLUME_OUTPUT, "Mono CV Harmony Volume Out: 0-10v :");
		configOutput(OUT_BASS_VOLUME_OUTPUT, "Mono CV Bass Volume Out: 0-10v :");
		configOutput(OUT_EXT_POLY_SCALE_OUTPUT, "Poly Scale Out: v/oct Out: 7, 5 or 12 notes");
		configOutput(OUT_EXT_POLY_QUANT_OUTPUT, "Poly  External Note(s) Quantized Out : v/oct :");
		configOutput(OUT_EXT_ROOT_OUTPUT, "Mono Scale Root Note Out: v/oct :");

//****************
								
		configButton(BUTTON_RUN_PARAM,  "Run");
		configButton(BUTTON_RESET_PARAM,  "Reset");

		configParam(CONTROL_TEMPOBPM_PARAM, min_bpm, max_bpm, 120.0f, "Tempo", " BPM");
	    configParam(CONTROL_TIMESIGNATURETOP_PARAM,2.0f, 15.0f, 4.0f, "Time Signature Top");
		configParam(CONTROL_TIMESIGNATUREBOTTOM_PARAM,0.0f, 3.0f, 1.0f, "Time Signature Bottom");
		configParam(CONTROL_ROOT_KEY_PARAM, 0, 11, 0.f, "Root/Key");
		configParam(CONTROL_SCALE_PARAM, 0.f, num_modes-1, 1.f, "Mode");
	
		configButton(BUTTON_ENABLE_MELODY_PARAM, "Melody Enable/Disable");
		configParam(CONTROL_MELODY_VOLUME_PARAM, 0.f, 10.f, 8.0f, "Volume");
		configButton(BUTTON_MELODY_DESTUTTER_PARAM,  "Hold Tied Notes Enable/Disable");
		configParam(CONTROL_MELODY_NOTE_LENGTH_DIVISOR_PARAM, 0.f, 5.f, 2.f, "Notes on 1/N");
		configParam(CONTROL_MELODY_TARGETOCTAVE_PARAM, 1.f, 7.f, 3.f, "Target Octave");
		configParam(CONTROL_MELODY_ALPHA_PARAM, 0.f, 1.f, .9f, "Variablility");
		configParam(CONTROL_MELODY_RANGE_PARAM, 0.f, 3.f, 1.f, "Octave Range");
		configButton(BUTTON_ENABLE_MELODY_STACCATO_PARAM,  "Staccato Enable/Disable");

		configButton(BUTTON_ENABLE_HARMONY_PARAM,  "Harmony (chords) Enable/Disable");
		configButton(BUTTON_ENABLE_MELODY_CHORDAL_PARAM,  "Chordal Notes Enable/Disable");
		configButton(BUTTON_ENABLE_MELODY_SCALER_PARAM,  "Scaler Notes Enable/Disable");
		configParam(CONTROL_HARMONY_VOLUME_PARAM, 0.f, 10.f, 8.0f, "Volume (0-10)");
		configParam(CONTROL_HARMONY_STEPS_PARAM, 1.f, 16.f, 16.f, "Steps");
		configParam(CONTROL_HARMONY_TARGETOCTAVE_PARAM, 1.f, 7.f, 3.f, "Target Octave");
		configParam(CONTROL_HARMONY_ALPHA_PARAM, 0.f, 1.f, .9f, "Variability"); 
		configParam(CONTROL_HARMONY_RANGE_PARAM, 0.f, 3.f, 1.f, "Octave Range");
		configParam(CONTROL_HARMONY_DIVISOR_PARAM, 0.f, 3.f, 0.f, "Notes Length");
		configButton(BUTTON_ENABLE_HARMONY_ALL7THS_PARAM,  "7ths Enable/Disable");
		configButton(BUTTON_ENABLE_HARMONY_V7THS_PARAM,  "V 7ths Enable/Disable");
		configButton(BUTTON_ENABLE_HARMONY_STACCATO_PARAM,  "Staccato Enable/Disable");
		configParam(CONTROL_HARMONYPRESETS_PARAM, 1.0f, (float)MAX_AVAILABLE_HARMONY_PRESETS, 1.0f, "Progression Preset");

		configButton(BUTTON_ENABLE_ARP_PARAM,  "Arp Enable/Disable");
		configButton(BUTTON_ENABLE_ARP_CHORDAL_PARAM,  "Chordal Notes Enable/Disable");
		configButton(BUTTON_ENABLE_ARP_SCALER_PARAM,  "Scaler Notes Enable/Disable");
		configParam(CONTROL_ARP_COUNT_PARAM, 0.f, 31.f, 0.f, "Arp Notes Played");
		configParam(CONTROL_ARP_INCREMENT_PARAM, 2.f, 5.f, 4.f, "Arp Notes Length");
		configParam(CONTROL_ARP_DECAY_PARAM, 0.f, 1.f, 0.f, "Volume Decay");
		configParam(CONTROL_ARP_PATTERN_PARAM, -3.f, 3.f, 1.f, "Pattern Preset");

		configButton(BUTTON_ENABLE_BASS_PARAM,  "Bass Enable/Disable");
		configParam(CONTROL_BASS_VOLUME_PARAM, 0.f, 10.f, 8.0f, "Volume");
		configParam(CONTROL_BASS_DIVISOR_PARAM, 0.f, 3.f, 0.f, "Notes Length");
		configParam(CONTROL_BASS_TARGETOCTAVE_PARAM, 1.f, 7.f, 2.f, "Target Octave"); 
		configButton(BUTTON_BASS_ACCENT_PARAM,  "Accent Enable/Disable");
		configButton(BUTTON_BASS_SYNCOPATE_PARAM,  "Syncopate Enable/Disable");
		configButton(BUTTON_BASS_SHUFFLE_PARAM,  "Shuffle Enable/Disable");
		configButton(BUTTON_ENABLE_BASS_STACCATO_PARAM,  "Staccato Enable/Disable"); 
		configButton(BUTTON_BASS_OCTAVES_PARAM,  "Play as 2 Octaves Enable/Disable");

		configButton(BUTTON_ENABLE_KEYBOARD_RENDER_PARAM,  "Render Piano Keyboard Enable/Disable");
		configButton(BUTTON_ENABLE_SCORE_RENDER_PARAM,  "Render Score Enable/Disable");
							
		configParam(CONTROL_HARMONY_FBM_OCTAVES_PARAM, 1.f, 6.f, 3.f, "Harmony fBm 1/f meandering noise. Number of noise octaves.");
		configParam(CONTROL_MELODY_FBM_OCTAVES_PARAM, 1.f, 6.f, 3.f, "Melody fBm 1/f meandering noise. Number of noise octaves.");
		configParam(CONTROL_ARP_FBM_OCTAVES_PARAM, 1.f, 6.f, 3.f, "Arp/32nds fBm 1/f meandering noise. Number of noise octaves.");

		configParam(CONTROL_HARMONY_FBM_PERIOD_PARAM, 1.f, 100.f, 60.f, "Harmony fBm 1/f meandering noise. Noise 'period' seconds.");
		configParam(CONTROL_MELODY_FBM_PERIOD_PARAM, 1.f, 100.f, 10.f, "Melody fBm 1/f meandering noise. Noise 'period' seconds.");
		configParam(CONTROL_ARP_FBM_PERIOD_PARAM, 1.f, 100.f, 1.f, "Arp/32nds fBm 1/f meandering noise. Noise 'period' seconds.");
       	

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

	}  // end Meander()
	
};  // end of struct Meander

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
	float getDefaultValue() override {return panelContrastDefault;}
	std::string getLabel() override { return label; }
	std::string getUnit() override { return " "; }
};    


struct MeanderPanelThemeItem : MenuItem {    
    
		Meander  *module;
        int theme;
 
        void onAction(const event::Action &e) override {
         	panelTheme = theme;  
		
        };

        void step() override {
        	rightText = (panelTheme == theme)? "✔" : "";
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

struct MeanderScaleOutModeItem : MenuItem {   
    
        Meander *module;
        Meander::ScaleOutMode mode;

 
        void onAction(const event::Action &e) override {
                module->scale_out_mode = mode;
				module->onResetScale();
        };

        void step() override {
                rightText = (module->scale_out_mode == mode)? "✔" : "";
        };
    
	};

	
struct MeanderGateOutModeItem : MenuItem {   
    
        Meander *module;
        Meander::GateOutMode mode;

 
        void onAction(const event::Action &e) override {
                module->gate_out_mode = mode;
        };

        void step() override {
                rightText = (module->gate_out_mode == mode)? "✔" : "";
        };
    
	};

	
 
struct RootKeySelectLineDisplay : LightWidget {

	Meander *module=NULL;
	int *val = NULL;
	
	RootKeySelectLineDisplay() {
	
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
		nvgFillColor(args.vg, paramTextColor);
		nvgStrokeWidth(args.vg, 3.0);

		char text[128];
		
		snprintf(text, sizeof(text), "%s", root_key_names[*val]);
		nvgText(args.vg, textpos.x, textpos.y, text, NULL);
		
		nvgClosePath(args.vg);
	}

};

struct ScaleSelectLineDisplay : LightWidget {

	Meander *module=NULL;
	int *val = NULL;
	
	ScaleSelectLineDisplay() {

	}

	void draw(const DrawArgs &args) override { 

		if (!module)
			return;

		std::shared_ptr<Font> font = APP->window->loadFont(asset::system("res/fonts/Nunito-Bold.ttf"));  // load a Rack font: a sans-serif bold
		
		Vec textpos = Vec(65,12); 
		
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
		nvgFillColor(args.vg, paramTextColor);  
	
		if (module)  
		{
			char text[128];
			
			snprintf(text, sizeof(text), "%s", mode_names[*val]);
			nvgText(args.vg, textpos.x, textpos.y, text, NULL);

			// add on the scale notes display out of this box
			nvgFontSize(args.vg, 18);
			nvgFillColor(args.vg, panelTextColor);
			strcpy(text,"");
			for (int i=0;i<mode_step_intervals[*val][0];++i)
			{
				strcat(text,module->note_desig[module->notes[i]%MAX_NOTES]);  
				strcat(text," ");
			}
			
			nvgText(args.vg, textpos.x, textpos.y+20, text, NULL);
		}
		
		nvgClosePath(args.vg);	
	} 

};

////////////////////////////////////
struct BpmDisplayWidget : LightWidget {

  Meander *module=NULL;	
  float *val = NULL;

 
  BpmDisplayWidget() {
 
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

	// if val is null (Module null)  // should never get here
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
//	if (*val > 9)    // 2  digit BPM
	if (*val > 0)    // 2  digit BPM
		textPos = Vec(16.75f, 32.0f);   // this is a relative offset within the box
	  
	textColor = paramTextColor;
    nvgFillColor(args.vg, textColor);
    nvgText(args.vg, textPos.x, textPos.y, to_display.str().c_str(), NULL);
	
  }
};
//////////////////////////////////// 
struct SigDisplayWidget : LightWidget {

  int *value = NULL;
  
  SigDisplayWidget() {
        
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
	  
	textColor = paramTextColor;
    nvgFillColor(args.vg, textColor);
    nvgText(args.vg, textPos.x, textPos.y, to_display.str().c_str(), NULL);
  }
};
 
 
struct MeanderWidget : ModuleWidget  
{
	SvgPanel* svgPanel;
	SvgPanel* darkPanel;
	
	rack::math::Rect  ParameterRect[MAX_PARAMS];  // warning, don't exceed the dimension
    rack::math::Rect  InportRect[MAX_INPORTS];  // warning, don't exceed the dimension
    rack::math::Rect  OutportRect[MAX_OUTPORTS];  // warning, don't exceed the dimension
 
	ParamWidget* paramWidgets[Meander::NUM_PARAMS]={0};  // keep track of all ParamWidgets as they are created so they can be moved around later  by the enum parmam ID
	LightWidget* lightWidgets[Meander::NUM_LIGHTS]={0};  // keep track of all LightWidgets as they are created so they can be moved around later  by the enum parmam ID

	PortWidget* outPortWidgets[Meander::NUM_OUTPUTS]={0};  // keep track of all output TPortWidgets as they are created so they can be moved around later  by the enum parmam ID
	PortWidget* inPortWidgets[Meander::NUM_INPUTS]={0};  // keep track of all output TPortWidgets as they are created so they can be moved around later  by the enum parmam ID
	
	struct CircleOf5thsDisplay : TransparentWidget 
	{
		Meander* module;
		rack::math::Rect*  ParameterRectLocal;   // warning, don't exceed the dimension
		rack::math::Rect*  InportRectLocal; 	 // warning, don't exceed the dimension
		rack::math::Rect*  OutportRectLocal;     // warning, don't exceed the dimension
			
		CircleOf5thsDisplay(Meander* module)  
		{
		//	setModule(module);  // most plugins do this
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
					nvgFillColor(args.vg, panelTextColor);
					char text[32];
					snprintf(text, sizeof(text), "%s", CircleNoteNames[i]);
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
						snprintf(text, sizeof(text), "%s", circle_of_fifths_degrees_UC[(i - module->theCircleOf5ths.theDegreeSemiCircle.RootKeyCircle5thsPosition+7)%7]);
					else
					if ((chord_type==1)||(chord_type==6)) // minor or diminished
						snprintf(text, sizeof(text), "%s", circle_of_fifths_degrees_LC[(i - module->theCircleOf5ths.theDegreeSemiCircle.RootKeyCircle5thsPosition+7)%7]);
					
					
					Vec TextPosition=module->theCircleOf5ths.CircleCenter.plus(module->theCircleOf5ths.theDegreeSemiCircle.degreeElements[i].radialDirection.mult(module->theCircleOf5ths.OuterCircleRadius*.92f));
					nvgTextAlign(args.vg,NVG_ALIGN_CENTER|NVG_ALIGN_MIDDLE);
					nvgText(args.vg, TextPosition.x, TextPosition.y, text, NULL);
					if (i==6) // draw diminished
					{
						Vec TextPositionBdim=Vec(TextPosition.x+9, TextPosition.y-4);
						sprintf(text, "o");
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

		void drawHarmonyControlParamLine(const DrawArgs &args, Vec paramControlPos, const char* label, float value, int valueDecimalPoints, NVGcolor textColor=nvgRGBA(0,0,0, 0xff))
		{
			Vec displayRectPos= paramControlPos.plus(Vec(128, 0)); 
			nvgBeginPath(args.vg);
		
			if (valueDecimalPoints>=0)
				nvgRoundedRect(args.vg, displayRectPos.x,displayRectPos.y, 45.f, 20.f, 4.f);
			
			char text[128];

			snprintf(text, sizeof(text), "%s", label);
			nvgFillColor(args.vg, textColor);
				
			nvgFontSize(args.vg, 14);
			nvgTextAlign(args.vg,NVG_ALIGN_LEFT|NVG_ALIGN_MIDDLE);
			nvgText(args.vg, paramControlPos.x+20, paramControlPos.y+10, text, NULL);

			nvgFillColor(args.vg, nvgRGBA( 0x2f,  0x27, 0x0a, 0xff));
			nvgFill(args.vg);
			nvgFillColor(args.vg, paramTextColor);
			nvgFontSize(args.vg, 17);
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

		void drawMelodyControlParamLine(const DrawArgs &args, Vec paramControlPos, const char* label, float value, int valueDecimalPoints, NVGcolor textColor=nvgRGBA(0,0,0, 0xff))
		{
			Vec displayRectPos= paramControlPos.plus(Vec(115, 0));
			nvgBeginPath(args.vg);

			if (valueDecimalPoints>=0)
				nvgRoundedRect(args.vg, displayRectPos.x,displayRectPos.y, 45.f, 20.f, 4.f);
			
			char text[128];

			snprintf(text, sizeof(text), "%s", label);
			nvgFillColor(args.vg, textColor);
				
			nvgFontSize(args.vg, 14);
			nvgTextAlign(args.vg,NVG_ALIGN_LEFT|NVG_ALIGN_MIDDLE);
			nvgText(args.vg, paramControlPos.x+20, paramControlPos.y+10, text, NULL);

			nvgFillColor(args.vg, nvgRGBA( 0x2f,  0x27, 0x0a, 0xff));
			nvgFill(args.vg);
			nvgFillColor(args.vg, paramTextColor);
			nvgFontSize(args.vg, 17);
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

		void drawBassControlParamLine(const DrawArgs &args, Vec paramControlPos, const char* label, float value, int valueDecimalPoints, NVGcolor textColor=nvgRGBA(0,0,0, 0xff))
		{
			Vec displayRectPos= paramControlPos.plus(Vec(85, 0));
			nvgBeginPath(args.vg);

			if (valueDecimalPoints>=0)
				nvgRoundedRect(args.vg, displayRectPos.x,displayRectPos.y, 45.f, 20.f, 4.f);
			
			char text[128];

			snprintf(text, sizeof(text), "%s", label);
			nvgFillColor(args.vg, textColor);
			nvgFontSize(args.vg, 14);
			nvgTextAlign(args.vg,NVG_ALIGN_LEFT|NVG_ALIGN_MIDDLE);
			nvgText(args.vg, paramControlPos.x+20, paramControlPos.y+10, text, NULL);

			nvgFillColor(args.vg, nvgRGBA( 0x2f,  0x27, 0x0a, 0xff));
			nvgFill(args.vg);
			nvgFillColor(args.vg, paramTextColor);
			nvgFontSize(args.vg, 17);
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

		void drawfBmControlParamLine(const DrawArgs &args, Vec paramControlPos, const char* label, float value, int valueDecimalPoints, NVGcolor textColor=nvgRGBA(0,0,0, 0xff))
		{
			Vec displayRectPos= paramControlPos.plus(Vec(105, 0));
			nvgBeginPath(args.vg);

			if (valueDecimalPoints>=0)
				nvgRoundedRect(args.vg, displayRectPos.x,displayRectPos.y, 45.f, 20.f, 4.f);
			
			char text[128];

			snprintf(text, sizeof(text), "%s", label);
			nvgFillColor(args.vg, textColor);
			nvgFontSize(args.vg, 14);
			nvgTextAlign(args.vg,NVG_ALIGN_LEFT|NVG_ALIGN_MIDDLE);
			nvgText(args.vg, paramControlPos.x+20, paramControlPos.y+10, text, NULL);

			nvgFillColor(args.vg, nvgRGBA( 0x2f,  0x27, 0x0a, 0xff));
			nvgFill(args.vg);
			nvgFillColor(args.vg, paramTextColor);
			nvgFontSize(args.vg, 17);
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
			nvgFillColor(args.vg, panelTextColor);
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
			nvgFillColor(args.vg, panelTextColor);
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
			nvgFillColor(args.vg, panelTextColor);
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
			nvgFillColor(args.vg, panelTextColor);
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
			nvgFillColor(args.vg, panelTextColor);
			if (textfont)
			nvgFontFaceId(args.vg, textfont->handle);
			nvgTextLetterSpacing(args.vg, -1);
			nvgFillColor(args.vg, nvgRGBA(0xff, 0xff, 0xff, 0xFF));
			nvgTextAlign(args.vg,NVG_ALIGN_CENTER|NVG_ALIGN_MIDDLE);
			nvgText(args.vg, OutportPos.x+12, OutportPos.y-8, label, NULL);
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

			if (source==3)  // arp
			{
				whiteNoteOffColor=nvgRGBA( 0xff,  0xff, 0xff, 0xff);
				whiteNoteOnColor=panelArpKeyboardColor;
				blackNoteOffColor=nvgRGBA( 0x0,  0x0, 0x0, 0xff);
				blackNoteOnColor=panelArpKeyboardColor;
			}
			else
			if (source==2)  // melody
			{
				whiteNoteOffColor=nvgRGBA( 0xff,  0xff, 0xff, 0xff);
				whiteNoteOnColor=panelMelodyKeyboardColor;
				blackNoteOffColor=nvgRGBA( 0x0,  0x0, 0x0, 0xff);
				blackNoteOnColor=panelMelodyKeyboardColor;
			}
			else
			if (source==0)  // generic
			{
				whiteNoteOffColor=nvgRGBA( 0xff,  0xff, 0xff, 0xff);
				whiteNoteOnColor=nvgRGBA( 0x80,  0x80, 0x80, 0xff);
				blackNoteOffColor=nvgRGBA( 0x0,  0x0, 0x0, 0xff);
				blackNoteOnColor=nvgRGBA( 0x80,  0x80, 0x80, 0xff);
			}
			else
			if (source==1)  // harmony  panelHarmonyPartColor
			{
				whiteNoteOffColor=nvgRGBA( 0xff,  0xff, 0xff, 0xff);
				whiteNoteOnColor=panelHarmonyKeyboardColor;
				blackNoteOffColor=nvgRGBA( 0x0,  0x0, 0x0, 0xff);
				blackNoteOnColor=panelHarmonyKeyboardColor;
			}
			else
			if (source==4)  // bass
			{
				whiteNoteOffColor=nvgRGBA( 0xff,  0xff, 0xff, 0xff);
				whiteNoteOnColor=panelBassKeyboardColor;
				blackNoteOffColor=nvgRGBA( 0x0,  0x0, 0x0, 0xff);
				blackNoteOnColor=panelBassKeyboardColor;
			}

		
			int numWhiteKeys=52;
			float beginLeftEdge = 930.0;
			float begintopEdge = 5.0;
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

			std::shared_ptr<Font> textfont = APP->window->loadFont(asset::plugin(pluginInstance, "res/Ubuntu Condensed 400.ttf"));
			std::shared_ptr<Font> musicfont = APP->window->loadFont(asset::plugin(pluginInstance, "res/Bravura.otf"));

						    	
			if (true)  // Harmony  position a paramwidget  can't access paramWidgets here  
			{
				nvgBeginPath(args.vg);
				nvgRoundedRect(args.vg, 484.f,25.f, 196.f, 353.f, 4.f);
				nvgFillColor(args.vg, nvgRGBA( panelcolor.r,  panelcolor.g, panelcolor.b, panelcolor.a));
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
				nvgFillColor(args.vg, panelTextColor);
				nvgStrokeColor(args.vg,nvgRGBA( 0x80,  0x80 , 0x80, 0x80));
				nvgStrokeWidth(args.vg, 2.0); 

				// draw set steps text 
				for(int i = 0; i<MAX_STEPS;++i) 
				{
					if (i==0)
					{
						sprintf(labeltext, "Set Step");
						drawLabelAbove(args, ParameterRectLocal[Meander::BUTTON_HARMONY_SETSTEP_1_PARAM+i], labeltext, 15.);  
					}
					sprintf(labeltext, "%d", i+1);
					drawLabelLeft(args,ParameterRectLocal[Meander::BUTTON_HARMONY_SETSTEP_1_PARAM+i], labeltext, 0.);  
				}

              

				//***************
				// update harmony panel begin

				nvgStrokeColor(args.vg,nvgRGBA( 0x80,  0x00 , 0x00, 0x80));
		
				snprintf(labeltext, sizeof(labeltext), "%s", "Harmony Chords Enable");
				drawHarmonyControlParamLine(args,ParameterRectLocal[Meander::BUTTON_ENABLE_HARMONY_PARAM].pos, labeltext, 0, -1, panelHarmonyPartColor);
				

				nvgStrokeColor(args.vg,nvgRGBA( 0x80,  0x80 , 0x80, 0x80));

				snprintf(labeltext, sizeof(labeltext), "%s", "Volume (0-10.0)");
				drawHarmonyControlParamLine(args,ParameterRectLocal[Meander::CONTROL_HARMONY_VOLUME_PARAM].pos, labeltext, module->theMeanderState.theHarmonyParms.volume, 1, panelTextColor);
						    
				snprintf(labeltext, sizeof(labeltext), "Steps (%d-%d)", module->theActiveHarmonyType.min_steps, module->theActiveHarmonyType.max_steps);
				drawHarmonyControlParamLine(args,ParameterRectLocal[Meander::CONTROL_HARMONY_STEPS_PARAM].pos, labeltext, (float)module->theActiveHarmonyType.num_harmony_steps, 0, panelTextColor);
				
				snprintf(labeltext, sizeof(labeltext), "%s", "Target Oct.(1-7)");
				drawHarmonyControlParamLine(args,ParameterRectLocal[Meander::CONTROL_HARMONY_TARGETOCTAVE_PARAM].pos, labeltext, module->theMeanderState.theHarmonyParms.target_octave, 0, panelTextColor);

				snprintf(labeltext, sizeof(labeltext), "%s", "Variability (0-1)");
				drawHarmonyControlParamLine(args,ParameterRectLocal[Meander::CONTROL_HARMONY_ALPHA_PARAM].pos, labeltext, module->theMeanderState.theHarmonyParms.alpha, 2, panelTextColor);

				snprintf(labeltext, sizeof(labeltext), "%s", "+-Octave Range (0-3)");
				drawHarmonyControlParamLine(args,ParameterRectLocal[Meander::CONTROL_HARMONY_RANGE_PARAM].pos, labeltext, module->theMeanderState.theHarmonyParms.note_octave_range, 2, panelTextColor);

				snprintf(labeltext, sizeof(labeltext), "%s", "Notes on                    1/");
				drawHarmonyControlParamLine(args,ParameterRectLocal[Meander::CONTROL_HARMONY_DIVISOR_PARAM].pos, labeltext, module->theMeanderState.theHarmonyParms.note_length_divisor, 0, panelTextColor);

				snprintf(labeltext, sizeof(labeltext), "%s", "~Nice 7ths");
				drawHarmonyControlParamLine(args,ParameterRectLocal[Meander::BUTTON_ENABLE_HARMONY_ALL7THS_PARAM].pos, labeltext, 0, -1, panelTextColor);

				snprintf(labeltext, sizeof(labeltext), "%s", "V 7ths");
				drawHarmonyControlParamLine(args,ParameterRectLocal[Meander::BUTTON_ENABLE_HARMONY_V7THS_PARAM].pos, labeltext, 0, -1, panelTextColor);

				snprintf(labeltext, sizeof(labeltext), "%s", "Staccato");
				drawHarmonyControlParamLine(args,ParameterRectLocal[Meander::BUTTON_ENABLE_HARMONY_STACCATO_PARAM].pos, labeltext, 0, -1, panelTextColor);

				snprintf(labeltext, sizeof(labeltext), "%s", "Progression Presets");
				drawHarmonyControlParamLine(args,ParameterRectLocal[Meander::CONTROL_HARMONYPRESETS_PARAM].pos, labeltext, 0, -1, panelTextColor);
				
				snprintf(labeltext, sizeof(labeltext), "%s", " STEP");
				drawHarmonyControlParamLine(args,ParameterRectLocal[Meander::BUTTON_PROG_STEP_PARAM].pos, labeltext, 0, -1, panelTextColor);
 
				//  do the progression displays
				pos =ParameterRectLocal[Meander::CONTROL_HARMONYPRESETS_PARAM].pos.plus(Vec(-20,40));
							
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
				nvgFillColor(args.vg, paramTextColor);
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
				nvgFillColor(args.vg, paramTextColor);
				snprintf(text, sizeof(text), "%s           ",  module->theActiveHarmonyType.harmony_degrees_desc);
				nvgText(args.vg, pos.x+5, pos.y+10, text, NULL);
										
			}

			if  (true)  // update melody control display
			{

				nvgBeginPath(args.vg);
				nvgRoundedRect(args.vg, 680.f,25.f, 187.f, 353.f, 4.f);
				nvgFillColor(args.vg, nvgRGBA( panelcolor.r,  panelcolor.g, panelcolor.b, panelcolor.a));
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
				nvgFillColor(args.vg, panelTextColor);
				nvgStrokeColor(args.vg,nvgRGBA( 0x80,  0x80 , 0x80, 0x80));
				nvgStrokeWidth(args.vg, 2.0);
				//***************

				// update melody panel begin

				nvgStrokeColor(args.vg,nvgRGBA( 0x80,  0x80 , 0x80, 0x80));
		
				snprintf(labeltext, sizeof(labeltext), "%s", "Melody Enable");
				drawMelodyControlParamLine(args,ParameterRectLocal[Meander::BUTTON_ENABLE_MELODY_PARAM].pos, labeltext, 0, -1, panelMelodyPartColor);

				nvgStrokeColor(args.vg,nvgRGBA( 0x80,  0x80 , 0x80, 0x80));

				snprintf(labeltext, sizeof(labeltext), "%s", "Degree(1-7)");
				drawMelodyControlParamLine(args,ParameterRectLocal[Meander::BUTTON_ENABLE_MELODY_PARAM].pos.plus(Vec(92,-8)), labeltext, 0, -1, panelTextColor);

				snprintf(labeltext, sizeof(labeltext), "%s", "Gate");
				drawMelodyControlParamLine(args,ParameterRectLocal[Meander::BUTTON_ENABLE_MELODY_PARAM].pos.plus(Vec(92,6)), labeltext, 0, -1, panelTextColor);

				snprintf(labeltext, sizeof(labeltext), "%s", "Chordal");
				drawMelodyControlParamLine(args,ParameterRectLocal[Meander::BUTTON_ENABLE_MELODY_CHORDAL_PARAM].pos, labeltext, 0, -1, panelTextColor);

				snprintf(labeltext, sizeof(labeltext), "%s", "Scaler");
				drawMelodyControlParamLine(args,ParameterRectLocal[Meander::BUTTON_ENABLE_MELODY_SCALER_PARAM].pos, labeltext, 0, -1, panelTextColor);

				snprintf(labeltext, sizeof(labeltext), "%s", "Volume (0-10.0)");
				drawMelodyControlParamLine(args,ParameterRectLocal[Meander::CONTROL_MELODY_VOLUME_PARAM].pos, labeltext, module->theMeanderState.theMelodyParms.volume, 1, panelTextColor);

				snprintf(labeltext, sizeof(labeltext), "%s", "Hold tied");
				drawMelodyControlParamLine(args,ParameterRectLocal[Meander::BUTTON_MELODY_DESTUTTER_PARAM].pos, labeltext, 0, -1, panelTextColor);

				snprintf(labeltext, sizeof(labeltext), "%s", "Notes on               1/");
				drawMelodyControlParamLine(args,ParameterRectLocal[Meander::CONTROL_MELODY_NOTE_LENGTH_DIVISOR_PARAM].pos, labeltext, module->theMeanderState.theMelodyParms.note_length_divisor, 0, panelTextColor);

				snprintf(labeltext, sizeof(labeltext), "%s", "Target Oct.(1-7)");
				drawMelodyControlParamLine(args,ParameterRectLocal[Meander::CONTROL_MELODY_TARGETOCTAVE_PARAM].pos, labeltext, module->theMeanderState.theMelodyParms.target_octave, 0, panelTextColor);

				snprintf(labeltext, sizeof(labeltext), "%s", "Variability (0-1)");
				drawMelodyControlParamLine(args,ParameterRectLocal[Meander::CONTROL_MELODY_ALPHA_PARAM].pos, labeltext, module->theMeanderState.theMelodyParms.alpha, 2, panelTextColor);

				snprintf(labeltext, sizeof(labeltext), "%s", "+-Octave Range (0-3)");
				drawMelodyControlParamLine(args,ParameterRectLocal[Meander::CONTROL_MELODY_RANGE_PARAM].pos, labeltext, module->theMeanderState.theMelodyParms.note_octave_range, 2, panelTextColor);

				snprintf(labeltext, sizeof(labeltext), "%s", "Staccato");
				drawMelodyControlParamLine(args,ParameterRectLocal[Meander::BUTTON_ENABLE_MELODY_STACCATO_PARAM].pos, labeltext, 0, -1, panelTextColor);

				// draw division line
				pos = ParameterRectLocal[Meander::BUTTON_ENABLE_ARP_PARAM].pos.plus(Vec(-20,-2));
				nvgMoveTo(args.vg, 	pos.x, pos.y);
				pos=pos.plus(Vec(190,0));
				nvgLineTo(args.vg, pos.x, pos.y);  
								  
			    nvgFillColor(args.vg, nvgRGBA( panelcolor.r,  panelcolor.g, panelcolor.b, panelcolor.a));
				nvgStrokeWidth(args.vg, 2.5);
				nvgStroke(args.vg);
				//
				nvgStrokeColor(args.vg,nvgRGBA( 0x00,  0x00 , 0x80, 0x80));

				snprintf(labeltext, sizeof(labeltext), "%s", "Arp Enable");
				drawMelodyControlParamLine(args,ParameterRectLocal[Meander::BUTTON_ENABLE_ARP_PARAM].pos, labeltext, 0, -1, panelArpPartColor);

				nvgStrokeColor(args.vg,nvgRGBA( 0x80,  0x80 , 0x80, 0x80));

				snprintf(labeltext, sizeof(labeltext), "%s", "Count (0-31)");
				drawMelodyControlParamLine(args,ParameterRectLocal[Meander::CONTROL_ARP_COUNT_PARAM].pos, labeltext, module->theMeanderState.theArpParms.count, 0, panelTextColor);

				snprintf(labeltext, sizeof(labeltext), "%s", "Notes on               1/");
				drawMelodyControlParamLine(args,ParameterRectLocal[Meander::CONTROL_ARP_INCREMENT_PARAM].pos, labeltext, module->theMeanderState.theArpParms.note_length_divisor, 0, panelTextColor);

				snprintf(labeltext, sizeof(labeltext), "%s", "Decay (0-1.0)");
				drawMelodyControlParamLine(args,ParameterRectLocal[Meander::CONTROL_ARP_DECAY_PARAM].pos, labeltext, module->theMeanderState.theArpParms.decay, 2, panelTextColor);

				snprintf(labeltext, sizeof(labeltext), "%s", "Chordal");
				drawMelodyControlParamLine(args,ParameterRectLocal[Meander::BUTTON_ENABLE_ARP_CHORDAL_PARAM].pos, labeltext, 0, -1, panelTextColor);

				snprintf(labeltext, sizeof(labeltext), "%s", "Scaler");
				drawMelodyControlParamLine(args,ParameterRectLocal[Meander::BUTTON_ENABLE_ARP_SCALER_PARAM].pos, labeltext, 0, -1, panelTextColor);

				snprintf(labeltext, sizeof(labeltext), "%s", "Pattern(-3-+3)");
				drawMelodyControlParamLine(args,ParameterRectLocal[Meander::CONTROL_ARP_PATTERN_PARAM].pos, labeltext, module->theMeanderState.theArpParms.pattern, -1, panelTextColor);

				pos =ParameterRectLocal[Meander::CONTROL_ARP_PATTERN_PARAM].pos.plus(Vec(102,0));
							
				nvgBeginPath(args.vg);

				nvgStrokeColor(args.vg,nvgRGBA( 0x80,  0x80 , 0x80, 0x80));
				nvgFillColor(args.vg, nvgRGBA( 0x2f,  0x27, 0x0a, 0xff));
				nvgRoundedRect(args.vg, pos.x-18.0,pos.y, 76.f, 20.f, 4.f);
				nvgStrokeWidth(args.vg, 2.0);
				nvgFill(args.vg);
			
				nvgFontSize(args.vg, 15);
				nvgFillColor(args.vg, paramTextColor);
				if (module->theMeanderState.theArpParms.pattern==0)
					snprintf(text, sizeof(text), "%d: 0-echo", module->theMeanderState.theArpParms.pattern);
				else
				if (module->theMeanderState.theArpParms.pattern==1)
					snprintf(text, sizeof(text), "%d: UPx1", module->theMeanderState.theArpParms.pattern);
				else
				if (module->theMeanderState.theArpParms.pattern==2)
					snprintf(text, sizeof(text), "%d: UPx1,DNx1", module->theMeanderState.theArpParms.pattern);
				else
				if (module->theMeanderState.theArpParms.pattern==3)
					snprintf(text, sizeof(text), "%d: UPx2", module->theMeanderState.theArpParms.pattern);
				else
				if (module->theMeanderState.theArpParms.pattern==-1)
					snprintf(text, sizeof(text), "%d: DNx1", module->theMeanderState.theArpParms.pattern);
				else
				if (module->theMeanderState.theArpParms.pattern==-2)
					snprintf(text, sizeof(text), "%d: DNx1,UPx1", module->theMeanderState.theArpParms.pattern);
				else
				if (module->theMeanderState.theArpParms.pattern==-3)
					snprintf(text, sizeof(text), "%d: DNx2", module->theMeanderState.theArpParms.pattern);
				else
					snprintf(text, sizeof(text), "%s", "      ");  // since text is used above, needs to be cleared in fallthrough case
				
				nvgText(args.vg, pos.x-10, pos.y+10, text, NULL);

				

			}

			if (true)  // update bass panel
			{
				nvgBeginPath(args.vg);
				nvgRoundedRect(args.vg, 869.f, 137.859f, 155.f, 239.1f, 4.f);
			   	nvgFillColor(args.vg, nvgRGBA( panelcolor.r,  panelcolor.g, panelcolor.b, panelcolor.a));
				nvgStrokeColor(args.vg,nvgRGBA( 0x80,  0x80 , 0x80, 0x80));
				nvgStrokeWidth(args.vg, 2.5);
				nvgStroke(args.vg); 

				
				Vec pos;
				Vec displayRectPos;
				Vec paramControlPos;
				char labeltext[128];
				nvgFontSize(args.vg, 17);
				if (textfont)
				nvgFontFaceId(args.vg, textfont->handle);
				nvgTextLetterSpacing(args.vg, -1);
				nvgFillColor(args.vg, panelTextColor);
				nvgStrokeColor(args.vg,nvgRGBA( 0x80,  0x80 , 0x80, 0x80));
				nvgStrokeWidth(args.vg, 2.0);
				
				nvgStrokeColor(args.vg,nvgRGBA( 0x00,  0x80 , 0x00, 0x80));
		
				snprintf(labeltext, sizeof(labeltext), "%s", "Bass Enable");
				drawBassControlParamLine(args,ParameterRectLocal[Meander::BUTTON_ENABLE_BASS_PARAM].pos, labeltext, 0, -1, panelBassPartColor);

				nvgStrokeColor(args.vg,nvgRGBA( 0x80,  0x80 , 0x80, 0x80));

				snprintf(labeltext, sizeof(labeltext), "%s", "Volume (0-10)");
				drawBassControlParamLine(args,ParameterRectLocal[Meander::CONTROL_BASS_VOLUME_PARAM].pos, labeltext, module->theMeanderState.theBassParms.volume, 1, panelTextColor);

				snprintf(labeltext, sizeof(labeltext), "%s", "Target Oct.(1-7)");
				drawBassControlParamLine(args,ParameterRectLocal[Meander::CONTROL_BASS_TARGETOCTAVE_PARAM].pos, labeltext, module->theMeanderState.theBassParms.target_octave, 0, panelTextColor);

				snprintf(labeltext, sizeof(labeltext), "%s", "Notes on    1/");
				drawBassControlParamLine(args,ParameterRectLocal[Meander::CONTROL_BASS_DIVISOR_PARAM].pos, labeltext, module->theMeanderState.theBassParms.note_length_divisor, 0, panelTextColor);

				snprintf(labeltext, sizeof(labeltext), "%s", "Staccato");
				drawBassControlParamLine(args,ParameterRectLocal[Meander::BUTTON_ENABLE_BASS_STACCATO_PARAM].pos, labeltext, module->theMeanderState.theBassParms.enable_staccato, -1, panelTextColor);

				snprintf(labeltext, sizeof(labeltext), "%s", "Accent");
				drawBassControlParamLine(args,ParameterRectLocal[Meander::BUTTON_BASS_ACCENT_PARAM].pos, labeltext, module->theMeanderState.theBassParms.accent, -1, panelTextColor);

				snprintf(labeltext, sizeof(labeltext), "%s", "Syncopate");
				drawBassControlParamLine(args,ParameterRectLocal[Meander::BUTTON_BASS_SYNCOPATE_PARAM].pos, labeltext, module->theMeanderState.theBassParms.syncopate, -1, panelTextColor);

				snprintf(labeltext, sizeof(labeltext), "%s", "Shuffle");
				drawBassControlParamLine(args,ParameterRectLocal[Meander::BUTTON_BASS_SHUFFLE_PARAM].pos, labeltext, module->theMeanderState.theBassParms.shuffle, -1, panelTextColor);

				snprintf(labeltext, sizeof(labeltext), "%s", "Octaves");
				drawBassControlParamLine(args,ParameterRectLocal[Meander::BUTTON_BASS_OCTAVES_PARAM].pos, labeltext, module->theMeanderState.theBassParms.octave_enabled, -1, panelTextColor);
			

			}

			if (true)  // update fBm panel
			{
				nvgBeginPath(args.vg);
				nvgRoundedRect(args.vg, 1025.f, 137.859f, 171.f, 239.1f, 4.f);
				nvgFillColor(args.vg, nvgRGBA( panelcolor.r,  panelcolor.g, panelcolor.b, panelcolor.a));
				nvgStrokeColor(args.vg,nvgRGBA( 0x80,  0x80 , 0x80, 0x80));
				nvgStrokeWidth(args.vg, 2.5);
				nvgStroke(args.vg); 

				
				Vec pos;
				Vec displayRectPos;
				Vec paramControlPos;
				char labeltext[128];
				nvgFontSize(args.vg, 17);
				if (textfont)
				nvgFontFaceId(args.vg, textfont->handle);
				nvgTextLetterSpacing(args.vg, -1);
				nvgFillColor(args.vg, nvgRGBA(0x0, 0x0, 0x0, 0xff));
				nvgStrokeColor(args.vg,nvgRGBA( 0x80,  0x80 , 0x80, 0x80));
				nvgStrokeWidth(args.vg, 2.0);

				snprintf(labeltext, sizeof(labeltext), "%s", "fBm 1/f Noise");
				drawfBmControlParamLine(args,ParameterRectLocal[Meander::CONTROL_HARMONY_FBM_OCTAVES_PARAM].pos.plus(Vec(30,-25)), labeltext, 0, -1, panelTextColor);
				
				snprintf(labeltext, sizeof(labeltext), "%s", "Harmony");
				drawfBmControlParamLine(args,ParameterRectLocal[Meander::CONTROL_HARMONY_FBM_OCTAVES_PARAM].pos.plus(Vec(37,-13)), labeltext, 0, -1, panelTextColor);
		
				snprintf(labeltext, sizeof(labeltext), "%s", "Octaves (1-6)");
				drawfBmControlParamLine(args,ParameterRectLocal[Meander::CONTROL_HARMONY_FBM_OCTAVES_PARAM].pos, labeltext, module->theMeanderState.theHarmonyParms.noctaves, 0, panelTextColor);

				snprintf(labeltext, sizeof(labeltext), "%s", "Period Sec. (1-100)");
				drawfBmControlParamLine(args,ParameterRectLocal[Meander::CONTROL_HARMONY_FBM_PERIOD_PARAM].pos, labeltext, module->theMeanderState.theHarmonyParms.period, 1, panelTextColor);

				snprintf(labeltext, sizeof(labeltext), "%s", "Melody");
				drawBassControlParamLine(args,ParameterRectLocal[Meander::CONTROL_MELODY_FBM_OCTAVES_PARAM].pos.plus(Vec(41,-13)), labeltext, 0, -1, panelTextColor);

				snprintf(labeltext, sizeof(labeltext), "%s", "Octaves (1-6)");
				drawfBmControlParamLine(args,ParameterRectLocal[Meander::CONTROL_MELODY_FBM_OCTAVES_PARAM].pos, labeltext, module->theMeanderState.theMelodyParms.noctaves, 0, panelTextColor);

				snprintf(labeltext, sizeof(labeltext), "%s", "Period Sec. (1-100)");
				drawfBmControlParamLine(args,ParameterRectLocal[Meander::CONTROL_MELODY_FBM_PERIOD_PARAM].pos, labeltext, module->theMeanderState.theMelodyParms.period, 1, panelTextColor);

				snprintf(labeltext, sizeof(labeltext), "%s", "32nds");
				drawfBmControlParamLine(args,ParameterRectLocal[Meander::CONTROL_ARP_FBM_OCTAVES_PARAM].pos.plus(Vec(47,-13)), labeltext, 0, -1, panelTextColor);

				snprintf(labeltext, sizeof(labeltext), "%s", "Octaves (1-6)");
				drawfBmControlParamLine(args,ParameterRectLocal[Meander::CONTROL_ARP_FBM_OCTAVES_PARAM].pos, labeltext, module->theMeanderState.theArpParms.noctaves, 0, panelTextColor);

				snprintf(labeltext, sizeof(labeltext), "%s", "Period Sec. (1-100)");
				drawfBmControlParamLine(args,ParameterRectLocal[Meander::CONTROL_ARP_FBM_PERIOD_PARAM].pos, labeltext, module->theMeanderState.theArpParms.period, 1, panelTextColor);

				
			} 

			if (true)  // draw rounded corner rects  for input jacks border 
			{
				char labeltext[128];
							
		
				snprintf(labeltext, sizeof(labeltext), "%s", "RUN");
				drawLabelAbove(args,ParameterRectLocal[Meander::BUTTON_RUN_PARAM], labeltext, 12.);

				snprintf(labeltext, sizeof(labeltext), "%s", "Out");
				drawOutport(args, OutportRectLocal[Meander::OUT_RUN_OUT].pos, labeltext, 0, 1);
				
			
				snprintf(labeltext, sizeof(labeltext), "%s", "RESET");
				drawLabelAbove(args,ParameterRectLocal[Meander::BUTTON_RESET_PARAM], labeltext, 12.);
				
				snprintf(labeltext, sizeof(labeltext), "%s", "Out");
				drawOutport(args, OutportRectLocal[Meander::OUT_RESET_OUT].pos, labeltext, 0, 1);
			
				snprintf(labeltext, sizeof(labeltext), "%s", "BPM");
				drawLabelRight(args,ParameterRectLocal[Meander::CONTROL_TEMPOBPM_PARAM], labeltext);

				snprintf(labeltext, sizeof(labeltext), "%s", "Time Sig Top");
				drawLabelRight(args,ParameterRectLocal[Meander::CONTROL_TIMESIGNATURETOP_PARAM], labeltext);

				snprintf(labeltext, sizeof(labeltext), "%s", "Time Sig Bottom");
				drawLabelRight(args,ParameterRectLocal[Meander::CONTROL_TIMESIGNATUREBOTTOM_PARAM], labeltext);

				snprintf(labeltext, sizeof(labeltext), "%s", "Root");
				drawLabelRight(args,ParameterRectLocal[Meander::CONTROL_ROOT_KEY_PARAM], labeltext);
				
				snprintf(labeltext, sizeof(labeltext), "%s", "Mode: bright->to darkest");
				drawLabelRight(args,ParameterRectLocal[Meander::CONTROL_SCALE_PARAM], labeltext);

				snprintf(labeltext, sizeof(labeltext), "%s", "Poly");
				drawLabelOffset(args, InportRectLocal[Meander::IN_POLY_QUANT_EXT_CV], labeltext, -4., -12.); 

				snprintf(labeltext, sizeof(labeltext), "%s", "8x BPM Clock");
				drawLabelOffset(args, InportRectLocal[Meander::IN_CLOCK_EXT_CV], labeltext, -4., -26.); 
			
				snprintf(labeltext, sizeof(labeltext), "%s", "Poly Ext Scale");
				drawLabelLeft(args, OutportRectLocal[Meander::OUT_EXT_POLY_SCALE_OUTPUT], labeltext, -40.);

				snprintf(labeltext, sizeof(labeltext), "%s", "Out");
				drawOutport(args, OutportRectLocal[Meander::OUT_EXT_POLY_SCALE_OUTPUT].pos, labeltext, 0, 1);
											
			}

			if (true)  // draw rounded corner rects  for output jacks border 
			{
				char labeltext[128];
				snprintf(labeltext, sizeof(labeltext), "%s", "1V/Oct");
				drawOutport(args, OutportRectLocal[Meander::OUT_HARMONY_CV_OUTPUT].pos, labeltext, 0, 1);

				snprintf(labeltext, sizeof(labeltext), "%s", "Gate");
				drawOutport(args, OutportRectLocal[Meander::OUT_HARMONY_GATE_OUTPUT].pos, labeltext, 0, 1);

				snprintf(labeltext, sizeof(labeltext), "%s", "Volume");
				drawOutport(args, OutportRectLocal[Meander::OUT_HARMONY_VOLUME_OUTPUT].pos, labeltext, 0, 1);

				snprintf(labeltext, sizeof(labeltext), "%s", "1V/Oct");
				drawOutport(args, OutportRectLocal[Meander::OUT_MELODY_CV_OUTPUT].pos, labeltext, 0, 1);

				snprintf(labeltext, sizeof(labeltext), "%s", "Gate");
				drawOutport(args, OutportRectLocal[Meander::OUT_MELODY_GATE_OUTPUT].pos, labeltext, 0, 1);

				snprintf(labeltext, sizeof(labeltext), "%s", "Volume");
				drawOutport(args, OutportRectLocal[Meander::OUT_MELODY_VOLUME_OUTPUT].pos, labeltext, 0, 1);

				snprintf(labeltext, sizeof(labeltext), "%s", "1V/Oct");
				drawOutport(args, OutportRectLocal[Meander::OUT_BASS_CV_OUTPUT].pos, labeltext, 0, 1);

				snprintf(labeltext, sizeof(labeltext), "%s", "Gate");
				drawOutport(args, OutportRectLocal[Meander::OUT_BASS_GATE_OUTPUT].pos, labeltext, 0, 1);

				snprintf(labeltext, sizeof(labeltext), "%s", "Volume");
				drawOutport(args, OutportRectLocal[Meander::OUT_BASS_VOLUME_OUTPUT].pos, labeltext, 0, 1);

				snprintf(labeltext, sizeof(labeltext), "%s", "Melody");
				drawOutport(args, OutportRectLocal[Meander::OUT_FBM_MELODY_OUTPUT].pos, labeltext, 0, 1);

				sprintf(labeltext, "%s", "Outputs are 0-10V fBm noise");
				nvgFillColor(args.vg, panelTextColor);
				nvgFontSize(args.vg, 17);
				if (textfont)
				nvgFontFaceId(args.vg, textfont->handle);
				nvgTextLetterSpacing(args.vg, -1);
				nvgBeginPath(args.vg);
				if (textfont)
				nvgFontFaceId(args.vg, textfont->handle);
				nvgTextAlign(args.vg,NVG_ALIGN_CENTER|NVG_ALIGN_MIDDLE);
			    nvgText(args.vg, OutportRectLocal[Meander::OUT_FBM_MELODY_OUTPUT].pos.x+13,  OutportRectLocal[Meander::OUT_FBM_MELODY_OUTPUT].pos.y-30, labeltext, NULL);
				

				snprintf(labeltext, sizeof(labeltext), "%s", "Harmony");
				drawOutport(args, OutportRectLocal[Meander::OUT_FBM_HARMONY_OUTPUT].pos, labeltext, 0, 1);

				snprintf(labeltext, sizeof(labeltext), "%s", "32nds");
				drawOutport(args, OutportRectLocal[Meander::OUT_FBM_ARP_OUTPUT].pos, labeltext, 0, 1);

				snprintf(labeltext, sizeof(labeltext), "%s", "Bar");
				drawOutport(args, OutportRectLocal[Meander::OUT_CLOCK_BAR_OUTPUT].pos, labeltext, 0, 1);

				snprintf(labeltext, sizeof(labeltext), "%s", "Beat");
				drawOutport(args, OutportRectLocal[Meander::OUT_CLOCK_BEAT_OUTPUT].pos, labeltext, 0, 1);

				snprintf(labeltext, sizeof(labeltext), "%s", "Beatx2");
				drawOutport(args, OutportRectLocal[Meander::OUT_CLOCK_BEATX2_OUTPUT].pos, labeltext, 0, 1);

				snprintf(labeltext, sizeof(labeltext), "%s", "1ms Clocked Trigger Pulses");
				rack::math::Rect rect=OutportRectLocal[Meander::OUT_CLOCK_BEATX2_OUTPUT];
				rect.pos=rect.pos.plus(Vec(0,-16));
				drawLabelAbove(args, rect, labeltext, 18.);
				
				snprintf(labeltext, sizeof(labeltext), "%s", "Beatx4");
				drawOutport(args, OutportRectLocal[Meander::OUT_CLOCK_BEATX4_OUTPUT].pos, labeltext, 0, 1);

				snprintf(labeltext, sizeof(labeltext), "%s", "Beatx8");
				drawOutport(args, OutportRectLocal[Meander::OUT_CLOCK_BEATX8_OUTPUT].pos, labeltext, 0, 1);
			
				snprintf(labeltext, sizeof(labeltext), "%s", "These can trigger STEP above or external");
				rect=OutportRectLocal[Meander::OUT_CLOCK_BEATX8_OUTPUT];
				rect.pos=rect.pos.plus(Vec(5,5));
				drawLabelRight(args, rect, labeltext);

				snprintf(labeltext, sizeof(labeltext), "%s", "Out");
				drawOutport(args, OutportRectLocal[Meander::OUT_EXT_ROOT_OUTPUT].pos, labeltext, 0, 1, 0.8);  // scale height by 0.8x
				
				snprintf(labeltext, sizeof(labeltext), "%s", "Out");
				drawOutport(args, OutportRectLocal[Meander::OUT_EXT_POLY_SCALE_OUTPUT].pos, labeltext, 0, 1);

				snprintf(labeltext, sizeof(labeltext), "%s", "Quant");
				drawOutport(args, OutportRectLocal[Meander::OUT_EXT_POLY_QUANT_OUTPUT].pos, labeltext, 0, 1);

				snprintf(labeltext, sizeof(labeltext), "%s", "Out");
				drawOutport(args, OutportRectLocal[Meander::OUT_CLOCK_OUT].pos, labeltext, 0, 1);
				
			}

			
			Vec pos;
			char text[128];
			nvgFontSize(args.vg, 17);
			if (textfont)
			nvgFontFaceId(args.vg, textfont->handle);
			nvgTextLetterSpacing(args.vg, -1);
			nvgFillColor(args.vg, panelTextColor);
			
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
	
		    // draw bar left vertical edge
			if (beginEdge > 0) {
				nvgBeginPath(args.vg);
				nvgMoveTo(args.vg, beginEdge, beginTop+barLineVoffset);
				nvgLineTo(args.vg, beginEdge, beginTop+barLineVlength);
				nvgStrokeColor(args.vg, panelLineColor);
				nvgStrokeWidth(args.vg, lineWidth);
				nvgStroke(args.vg);
			}
			// draw staff lines
			nvgBeginPath(args.vg);
			for (int staff = 36, y = staff; y <= staff + 24; y += yLineSpacing) { 	
				nvgMoveTo(args.vg, beginEdge, beginTop+y);
				nvgLineTo(args.vg, beginEdge+stafflineLength, beginTop+y);
			}
			nvgStrokeColor(args.vg, panelLineColor);
			nvgStrokeWidth(args.vg, lineWidth);
			nvgStroke(args.vg); 

			nvgFontSize(args.vg, 90);
			if (musicfont)
			nvgFontFaceId(args.vg, musicfont->handle);
			nvgTextLetterSpacing(args.vg, -1);
			nvgFillColor(args.vg, panelLineColor);
			pos=Vec(beginEdge+12, beginTop+54); 
			snprintf(text, sizeof(text), "%s", gClef.c_str());  
			nvgText(args.vg, pos.x, pos.y, text, NULL);
			
			nvgFontSize(args.vg, 90);
			nvgTextAlign(args.vg,NVG_ALIGN_CENTER|NVG_ALIGN_MIDDLE);
			snprintf(text, sizeof(text), "%s", sharp.c_str());  
			
			int num_sharps1=0;
			int vertical_offset1=0;
			for (int i=0; i<7; ++i)
			{
				nvgBeginPath(args.vg);
				if (root_key_signatures_chromaticForder[module->notate_mode_as_signature_root_key][i]==1)
				{
					vertical_offset1=root_key_sharps_vertical_display_offset[num_sharps1];
					pos=Vec(beginEdge+24+(num_sharps1*5), beginTop+33+(vertical_offset1*yHalfLineSpacing));
					nvgText(args.vg, pos.x, pos.y, text, NULL);
					++num_sharps1;
				}
				nvgClosePath(args.vg);
			}	
		
			snprintf(text, sizeof(text), "%s", flat.c_str());  
			int num_flats1=0;
			vertical_offset1=0;
			for (int i=6; i>=0; --i)
			{
				nvgBeginPath(args.vg);
				if (root_key_signatures_chromaticForder[module->notate_mode_as_signature_root_key][i]==-1)
				{
					vertical_offset1=root_key_flats_vertical_display_offset[num_flats1];
					pos=Vec(beginEdge+24+(num_flats1*5), beginTop+33+(vertical_offset1*yHalfLineSpacing));
					nvgText(args.vg, pos.x, pos.y, text, NULL);
					++num_flats1;
				}
				nvgClosePath(args.vg);
			}	

			nvgFontSize(args.vg, 12);
			if (textfont)
			nvgFontFaceId(args.vg, textfont->handle);
			nvgTextLetterSpacing(args.vg, -1);
			nvgFillColor(args.vg, panelTextColor);

			pos=Vec(beginEdge+30, beginTop+95);  
			snprintf(text, sizeof(text), "%s", "In");
			nvgText(args.vg, pos.x, pos.y, text, NULL);

			pos=Vec(beginEdge+30, beginTop+115);  
			snprintf(text, sizeof(text), "%s", "In");
			nvgText(args.vg, pos.x, pos.y, text, NULL);

			pos=Vec(beginEdge+90, beginTop+95);   
			snprintf(text, sizeof(text), "%s", "Degree (1-7)");
			nvgText(args.vg, pos.x, pos.y, text, NULL);

			pos=Vec(beginEdge+74, beginTop+115);  
			snprintf(text, sizeof(text), "%s", "Gate");
			nvgText(args.vg, pos.x, pos.y, text, NULL);

			//************************

			// display area 
		
			
			snprintf(text, sizeof(text), "%s", "Display Keys");
			drawLabelRight(args,ParameterRectLocal[Meander::BUTTON_ENABLE_KEYBOARD_RENDER_PARAM], text);

			snprintf(text, sizeof(text), "%s", "Display Score");
			drawLabelRight(args,ParameterRectLocal[Meander::BUTTON_ENABLE_SCORE_RENDER_PARAM], text);

			// draw staff lines
			if (module->theMeanderState.renderScoreEnabled)
			{
				beginEdge = 890;
				beginTop =8;
				lineWidth=0.75; 
				
				stafflineLength=300;
				barLineVoffset=36.;
				barLineVlength=60.;
				yLineSpacing=6;
				yHalfLineSpacing=3.0f;
				
				// draw bar left vertical edge

				if (beginEdge > 0) {
					nvgBeginPath(args.vg);
					nvgMoveTo(args.vg, beginEdge, beginTop+barLineVoffset);
					nvgLineTo(args.vg, beginEdge, beginTop+(1.60*barLineVlength));
					nvgStrokeColor(args.vg, panelLineColor);
					nvgStrokeWidth(args.vg, lineWidth);
					nvgStroke(args.vg);
				}

				// draw bar right vertical edge
				if (beginEdge > 0) {
					nvgBeginPath(args.vg);
					nvgMoveTo(args.vg, beginEdge+stafflineLength, beginTop+barLineVoffset);
					nvgLineTo(args.vg, beginEdge+stafflineLength, beginTop+(1.60*barLineVlength));
					nvgStrokeColor(args.vg, panelLineColor);
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

				nvgStrokeColor(args.vg, panelLineColor);
				nvgStrokeWidth(args.vg, lineWidth);
				nvgStroke(args.vg);

				nvgFontSize(args.vg, 90);
				if (musicfont)
				nvgFontFaceId(args.vg, musicfont->handle);
				nvgTextLetterSpacing(args.vg, -1);
				nvgFillColor(args.vg, panelLineColor);
				pos=Vec(beginEdge+3, beginTop+54);  
				snprintf(text, sizeof(text), "%s", gClef.c_str());  
				nvgText(args.vg, pos.x, pos.y, text, NULL);

				nvgFontSize(args.vg, 90);
				pos=Vec(beginEdge+3, beginTop+78.5); 
				snprintf(text, sizeof(text), "%s", fClef.c_str());  
				nvgText(args.vg, pos.x, pos.y, text, NULL);
				
				nvgFontSize(args.vg, 90);
			
				// draw time signature top number
				int time_sig_top_tens=module->time_sig_top/10;
				int time_sig_top_units=module->time_sig_top%10;

				// draw time sig top number units digit     
				switch(time_sig_top_units)
				{
					case 0:
						snprintf(text, sizeof(text), "%s", timeSig0.c_str());  
						break;
					case 1:
						snprintf(text, sizeof(text), "%s", timeSig1.c_str());  
						break;
					case 2:
						snprintf(text, sizeof(text), "%s", timeSig2.c_str());  
						break;
					case 3:
						snprintf(text, sizeof(text), "%s", timeSig3.c_str());  
						break;
					case 4:
						snprintf(text, sizeof(text), "%s", timeSig4.c_str());  
						break;
					case 5:
						snprintf(text, sizeof(text), "%s", timeSig5.c_str());  
						break;
					case 6:
						snprintf(text, sizeof(text), "%s", timeSig6.c_str());  
						break;
					case 7:
						snprintf(text, sizeof(text), "%s", timeSig7.c_str());  
						break;
					case 8:
						snprintf(text, sizeof(text), "%s", timeSig8.c_str());  
						break;
					case 9:
						snprintf(text, sizeof(text), "%s", timeSig9.c_str());  
						break;
					default:
						snprintf(text, sizeof(text), "%s", "");  
						break;
				}

				// draw on treble clef staves    
			    if (time_sig_top_tens>0)
					pos=Vec(beginEdge+65, beginTop+43);
				else
					pos=Vec(beginEdge+60, beginTop+43);

				nvgText(args.vg, pos.x, pos.y, text, NULL);
				// draw on bass clef staves 
				if (time_sig_top_tens>0)
					pos=Vec(beginEdge+65, beginTop+79);
				else
					pos=Vec(beginEdge+60, beginTop+79);
				nvgText(args.vg, pos.x, pos.y, text, NULL); 

				// draw time sig top number tens digit     
				if (time_sig_top_tens>0)
				{
					switch(time_sig_top_tens)
					{
						case 0:
							snprintf(text, sizeof(text), "%s", timeSig0.c_str());  
							break;
						case 1:
							snprintf(text, sizeof(text), "%s", timeSig1.c_str());  
							break;
						case 2:
							snprintf(text, sizeof(text), "%s", timeSig2.c_str());  
							break;
						case 3:
							snprintf(text, sizeof(text), "%s", timeSig3.c_str());  
							break;
						case 4:
							snprintf(text, sizeof(text), "%s", timeSig4.c_str());  
							break;
						case 5:
							snprintf(text, sizeof(text), "%s", timeSig5.c_str());  
							break;
						case 6:
							snprintf(text, sizeof(text), "%s", timeSig6.c_str());  
							break;
						case 7:
							snprintf(text, sizeof(text), "%s", timeSig7.c_str());  
							break;
						case 8:
							snprintf(text, sizeof(text), "%s", timeSig8.c_str());  
							break;
						case 9:
							snprintf(text, sizeof(text), "%s", timeSig9.c_str());  
							break;
						default:
							snprintf(text, sizeof(text), "%s", "");  
							break;
					}	
					// draw on treble clef staves  
					pos=Vec(beginEdge+55, beginTop+43);
					nvgText(args.vg, pos.x, pos.y, text, NULL);
					// draw on bass clef staves  
					pos=Vec(beginEdge+55, beginTop+79);
				    nvgText(args.vg, pos.x, pos.y, text, NULL);
				}

				// draw time signature bottom number
				int time_sig_bottom_tens=module->time_sig_bottom/10;
				int time_sig_bottom_units=module->time_sig_bottom%10;

				// draw time sig bottom number units digit     		     
				switch(time_sig_bottom_units)
				{
					case 0:
						snprintf(text, sizeof(text), "%s", timeSig0.c_str());  
						break;
					case 1:
						snprintf(text, sizeof(text), "%s", timeSig1.c_str());  
						break;
					case 2:
						snprintf(text, sizeof(text), "%s", timeSig2.c_str());  
						break;
					case 3:
						snprintf(text, sizeof(text), "%s", timeSig3.c_str());  
						break;
					case 4:
						snprintf(text, sizeof(text), "%s", timeSig4.c_str());  
						break;
					case 5:
						snprintf(text, sizeof(text), "%s", timeSig5.c_str());  
						break;
					case 6:
						snprintf(text, sizeof(text), "%s", timeSig6.c_str());  
						break;
					case 7:
						snprintf(text, sizeof(text), "%s", timeSig7.c_str());  
						break;
					case 8:
						snprintf(text, sizeof(text), "%s", timeSig8.c_str());  
						break;
					case 9:
						snprintf(text, sizeof(text), "%s", timeSig9.c_str());  
						break;
					default:
						snprintf(text, sizeof(text), "%s", "");  
						break;
				}
			
				// draw on treble clef staves  
				if (time_sig_bottom_tens>0)
					pos=Vec(beginEdge+65, beginTop+53);
				else
					pos=Vec(beginEdge+60, beginTop+53);
				nvgText(args.vg, pos.x, pos.y, text, NULL);
				// draw on bass clef staves  
				if (time_sig_bottom_tens>0)
					pos=Vec(beginEdge+65, beginTop+89);
				else
					pos=Vec(beginEdge+60, beginTop+89);
				nvgText(args.vg, pos.x, pos.y, text, NULL);

				// draw time sig bottom number tens digit     
				if (time_sig_bottom_tens>0)
				{
					switch(time_sig_bottom_tens)
					{
						case 0:
							snprintf(text, sizeof(text), "%s", timeSig0.c_str());  
							break;
						case 1:
							snprintf(text, sizeof(text), "%s", timeSig1.c_str());  
							break;
						case 2:
							snprintf(text, sizeof(text), "%s", timeSig2.c_str());  
							break;
						case 3:
							snprintf(text, sizeof(text), "%s", timeSig3.c_str());  
							break;
						case 4:
							snprintf(text, sizeof(text), "%s", timeSig4.c_str());  
							break;
						case 5:
							snprintf(text, sizeof(text), "%s", timeSig5.c_str());  
							break;
						case 6:
							snprintf(text, sizeof(text), "%s", timeSig6.c_str());  
							break;
						case 7:
							snprintf(text, sizeof(text), "%s", timeSig7.c_str());  
							break;
						case 8:
							snprintf(text, sizeof(text), "%s", timeSig8.c_str());  
							break;
						case 9:
							snprintf(text, sizeof(text), "%s", timeSig9.c_str());  
							break;
						default:
							snprintf(text, sizeof(text), "%s", "");  
							break;
					}	

					// draw on treble clef staves  
					pos=Vec(beginEdge+55, beginTop+53);
				    nvgText(args.vg, pos.x, pos.y, text, NULL);
					// draw on treble clef staves  
					pos=Vec(beginEdge+55, beginTop+89);
				    nvgText(args.vg, pos.x, pos.y, text, NULL);
				}
			
				// do root_key signature
				
				nvgTextAlign(args.vg,NVG_ALIGN_CENTER|NVG_ALIGN_MIDDLE);
				snprintf(text, sizeof(text), "%s", sharp.c_str());  
				
				num_sharps1=0;
				vertical_offset1=0;
				for (int i=0; i<7; ++i)
				{
					nvgBeginPath(args.vg);
					if (root_key_signatures_chromaticForder[module->notate_mode_as_signature_root_key][i]==1)
					{
						vertical_offset1=root_key_sharps_vertical_display_offset[num_sharps1];
						pos=Vec(beginEdge+24+(num_sharps1*5), beginTop+33+(vertical_offset1*yHalfLineSpacing));
						nvgText(args.vg, pos.x, pos.y, text, NULL);
						++num_sharps1;
					}
					nvgClosePath(args.vg);
				}	
			
				snprintf(text, sizeof(text), "%s", flat.c_str());  
				num_flats1=0;
				vertical_offset1=0;
				for (int i=6; i>=0; --i)  
				{
					nvgBeginPath(args.vg);
					if (root_key_signatures_chromaticForder[module->notate_mode_as_signature_root_key][i]==-1)
					{
						vertical_offset1=root_key_flats_vertical_display_offset[num_flats1];
						pos=Vec(beginEdge+24+(num_flats1*5), beginTop+33+(vertical_offset1*yHalfLineSpacing));
						nvgText(args.vg, pos.x, pos.y, text, NULL);
						++num_flats1;
					}
					nvgClosePath(args.vg);
				}	

				// now do for bass clef

				nvgFontSize(args.vg, 90);
				nvgTextAlign(args.vg,NVG_ALIGN_CENTER|NVG_ALIGN_MIDDLE);
				snprintf(text, sizeof(text), "%s", sharp.c_str());  
				
				num_sharps1=0;
				vertical_offset1=0;
				for (int i=0; i<7; ++i)
				{
					nvgBeginPath(args.vg);
					if (root_key_signatures_chromaticForder[module->notate_mode_as_signature_root_key][i]==1)
					{
						vertical_offset1=root_key_sharps_vertical_display_offset[num_sharps1];
						pos=Vec(beginEdge+24+(num_sharps1*5), beginTop+75+(vertical_offset1*yHalfLineSpacing));
						nvgText(args.vg, pos.x, pos.y, text, NULL);
						++num_sharps1;
					}
					nvgClosePath(args.vg);
				}	
			
				snprintf(text, sizeof(text), "%s", flat.c_str());  
				num_flats1=0;
				vertical_offset1=0;
				for (int i=6; i>=0; --i)
				{
					nvgBeginPath(args.vg);
					if (root_key_signatures_chromaticForder[module->notate_mode_as_signature_root_key][i]==-1)
					{
						vertical_offset1=root_key_flats_vertical_display_offset[num_flats1];
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
				nvgFillColor(args.vg, panelTextColor);
				float mid_C_position = 74.0;  // middle C
				pos=Vec(beginEdge-12, mid_C_position);  
				snprintf(text, sizeof(text), "%s", "C4");  
				nvgText(args.vg, pos.x, pos.y, text, NULL);
						
		
				// draw notes on staves
				float display_note_position=0; 
				char noteText[128];
		
				if (module->moduleVarsInitialized)  // only initialized if Module!=NULL
				{
					nvgFontSize(args.vg, 90);
					if (musicfont)
					nvgFontFaceId(args.vg, musicfont->handle);
					nvgTextLetterSpacing(args.vg, -1);
					nvgFillColor(args.vg, panelTextColor);

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
								nvgFillColor(args.vg, panelHarmonyPartColor); 
							else
							if (module->played_notes_circular_buffer[i].noteType==NOTE_TYPE_MELODY)
								nvgFillColor(args.vg, panelMelodyPartColor); 
							else
							if (module->played_notes_circular_buffer[i].noteType==NOTE_TYPE_ARP)
								nvgFillColor(args.vg, panelArpPartColor); 
							else
							if (module->played_notes_circular_buffer[i].noteType==NOTE_TYPE_BASS)
								nvgFillColor(args.vg, panelBassPartColor); 
							
						}

						
						nvgFontSize(args.vg, 90);
						if (module->played_notes_circular_buffer[i].length==1)
						{
							snprintf(noteText, sizeof(noteText), "%s", noteWhole.c_str());  
						}
						else
						if (module->played_notes_circular_buffer[i].length==2)
						{
							if ((display_note<38)||((display_note>47)&&(display_note<59)))
							{
								snprintf(noteText, sizeof(noteText), "%s", noteHalfUp.c_str()); 
								pos.x+=1.0;  // move up stem notes a bit to right to avoid collisions with down stem notes
							}
							else
							{
								snprintf(noteText, sizeof(noteText), "%s", noteHalfDown.c_str()); 
								pos.x-=1.0;  // move down stem notes a bit to left to avoid collisions with up stem notes
							}
						}
						else
						if (module->played_notes_circular_buffer[i].length==4)
						{
							if ((display_note<38)||((display_note>47)&&(display_note<59)))
							{
								snprintf(noteText, sizeof(noteText), "%s", noteQuarterUp.c_str()); 
								pos.x+=1.0;  // move up stem notes a bit to right to avoid collisions with down stem notes
							}
							else
							{
								snprintf(noteText, sizeof(noteText), "%s", noteQuarterDown.c_str()); 
								pos.x-=1.0;  // move down stem notes a bit to left to avoid collisions with up stem notes
							}
						}
						else
						if (module->played_notes_circular_buffer[i].length==8)
						{
							if ((display_note<38)||((display_note>47)&&(display_note<59)))
							{
								snprintf(noteText, sizeof(noteText), "%s", noteEighthUp.c_str()); 
								pos.x+=1.0;  // move up stem notes a bit to right to avoid collisions with down stem notes
							}
							else
							{
								snprintf(noteText, sizeof(noteText), "%s", noteEighthDown.c_str()); 
								pos.x-=1.0;  // move down stem notes a bit to left to avoid collisions with up stem notes
							}
						}
						else
						if (module->played_notes_circular_buffer[i].length==16)
						{
							if ((display_note<38)||((display_note>47)&&(display_note<59)))
							{
								snprintf(noteText, sizeof(noteText), "%s", noteSixteenthUp.c_str()); 
								pos.x+=1.0;  // move up stem notes a bit to right to avoid collisions with down stem notes
							}
							else
							{
								snprintf(noteText, sizeof(noteText), "%s", noteSixteenthDown.c_str()); 
								pos.x-=1.0;  // move down stem notes a bit to left to avoid collisions with up stem notes
							}
						}
						else
						if (module->played_notes_circular_buffer[i].length==32)
						{
							if ((display_note<38)||((display_note>47)&&(display_note<59)))
							{
								snprintf(noteText, sizeof(noteText), "%s", noteThirtysecondthUp.c_str()); 
								pos.x+=1.0;  // move up stem notes a bit to right to avoid collisions with down stem notes
							}
							else
							{
								snprintf(text, sizeof(noteText), "%s", noteThirtysecondthDown.c_str()); 
								pos.x-=1.0;  // move down stem notes a bit to left to avoid collisions with up stem notes
							}
						}   
					//	nvgText(args.vg, pos.x, pos.y, noteText, NULL);  // defer note draw to after ledger lines draw

						// do ledger lines
						int onLineNumberAboveStaves=0;  // value= 1,2,3
						int onLineNumberBetweenStaves=0;// valid=1
						int onLineNumberBelowStaves=0;  // value= 1,2,3
						int onSpaceNumberAboveStaves=0;  // value= 1,2,3
						int onSpaceNumberBetweenStaves=0;// valid=1
						int onSpaceNumberBelowStaves=0;  // value= 1,2,3

						// ((scale_note==0)&&(octave==2))  //C4, middle C as drawn

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
							nvgFillColor(args.vg, panelTextColor);
							nvgStrokeWidth(args.vg, lineWidth);
													
							if ((module->played_notes_circular_buffer[i].length==8)||(module->played_notes_circular_buffer[i].length==16)||(module->played_notes_circular_buffer[i].length==32))
								ledgerPos.x -= 2.75;
							else
								ledgerPos.x += 0.25;

							ledgerPos.y += 11.5;
							snprintf(text, sizeof(text), "%s", staff1Line.c_str()); 
						//	snprintf(text, sizeof(text), "%s", ledgerLine.c_str()); 
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
						//	else
						//	if ((scale_note==0)&&(octave==2))  //C4
						//		onSpaceNumberBetweenStaves=1;
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
								nvgFillColor(args.vg, panelTextColor);
								nvgStrokeWidth(args.vg, lineWidth);
								
								if ((module->played_notes_circular_buffer[i].length==8)||(module->played_notes_circular_buffer[i].length==16)||(module->played_notes_circular_buffer[i].length==32))
									ledgerPos.x -= 2.75;
								else
									ledgerPos.x += 0.25;

								snprintf(text, sizeof(text), "%s", staff1Line.c_str()); 
							//	snprintf(text, sizeof(text), "%s", ledgerLine.c_str()); 
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
								nvgFillColor(args.vg, panelHarmonyPartColor); 
							else
							if (module->played_notes_circular_buffer[i].noteType==NOTE_TYPE_MELODY)
								nvgFillColor(args.vg, panelMelodyPartColor); 
							else
							if (module->played_notes_circular_buffer[i].noteType==NOTE_TYPE_ARP)
								nvgFillColor(args.vg, panelArpPartColor); 
							else
							if (module->played_notes_circular_buffer[i].noteType==NOTE_TYPE_BASS)
								nvgFillColor(args.vg, panelBassPartColor); 
							
						}
						nvgText(args.vg, pos.x, pos.y, noteText, NULL);  // now draw notes after ledger lines
					}
				}
			}
			//*********************

	
			float notesPlayingDisplayStartX=565.0;
			float notesPlayingDisplayStartY=5.0;
			float notesPlayingDisplayWidth=300.0;
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
			nvgFillColor(args.vg, panelTextColor);
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

			nvgMoveTo(args.vg, notesPlayingDisplayStartX+((5*notesPlayingDisplayNoteWidth)),notesPlayingDisplayStartY);
			nvgLineTo(args.vg, notesPlayingDisplayStartX+((5*notesPlayingDisplayNoteWidth)), notesPlayingDisplayEndY);

			nvgMoveTo(args.vg, notesPlayingDisplayStartX+((6*notesPlayingDisplayNoteWidth)),notesPlayingDisplayStartY);
			nvgLineTo(args.vg, notesPlayingDisplayStartX+((6*notesPlayingDisplayNoteWidth)), notesPlayingDisplayEndY);

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

			nvgMoveTo(args.vg, notesPlayingDisplayStartX+((7*notesPlayingDisplayNoteWidth)),notesPlayingDisplayStartY);
			nvgLineTo(args.vg, notesPlayingDisplayStartX+((7*notesPlayingDisplayNoteWidth)), notesPlayingDisplayEndY);

	
			nvgStroke(args.vg);
			nvgClosePath(args.vg);
			
			if (module->moduleVarsInitialized)  // globals fully initialized if Module!=NULL
			{
				nvgFontSize(args.vg, 14);
				if (textfont)
				nvgFontFaceId(args.vg, textfont->handle);
				nvgTextLetterSpacing(args.vg, -1);
				nvgFillColor(args.vg, panelTextColor);
				nvgTextAlign(args.vg,NVG_ALIGN_CENTER|NVG_ALIGN_MIDDLE);

				// write last harmony note played 1
				if (module->theMeanderState.theHarmonyParms.last[0].note!=0)
				{
					if ((module->theMeanderState.theHarmonyParms.last[0].note>=0)&&(module->theMeanderState.theHarmonyParms.last[0].note<128))
					{
						pos=Vec(notesPlayingDisplayStartX+20, notesPlayingDisplayNoteCenterY);  
						snprintf(text, sizeof(text), "%s%d", module->note_desig[(module->theMeanderState.theHarmonyParms.last[0].note%12)] , module->theMeanderState.theHarmonyParms.last[0].note/12);
						nvgFillColor(args.vg, panelHarmonyPartColor); 
						nvgText(args.vg, pos.x, pos.y, text, NULL);
						nvgFillColor(args.vg, panelTextColor);
					}
				}

				// write last harmony note played 2
				if (module->theMeanderState.theHarmonyParms.last[1].note!=0)
				{
					if ((module->theMeanderState.theHarmonyParms.last[1].note>=0)&&(module->theMeanderState.theHarmonyParms.last[1].note<128))
					{
						pos=Vec(notesPlayingDisplayStartX+20+(1*37.5), notesPlayingDisplayNoteCenterY);  
						snprintf(text, sizeof(text), "%s%d",module->note_desig[(module->theMeanderState.theHarmonyParms.last[1].note%12)], module->theMeanderState.theHarmonyParms.last[1].note/12);
						nvgFillColor(args.vg, panelHarmonyPartColor); 
						nvgText(args.vg, pos.x, pos.y, text, NULL);
						nvgFillColor(args.vg, panelTextColor);
					}
				}

				// write last harmony note played 3
				if (module->theMeanderState.theHarmonyParms.last[2].note!=0)
				{
					if ((module->theMeanderState.theHarmonyParms.last[2].note>=0)&&(module->theMeanderState.theHarmonyParms.last[2].note<128))
					{
						pos=Vec(notesPlayingDisplayStartX+20+(2*37.5), notesPlayingDisplayNoteCenterY); 
						snprintf(text, sizeof(text), "%s%d", module->note_desig[(module->theMeanderState.theHarmonyParms.last[2].note%12)], module->theMeanderState.theHarmonyParms.last[2].note/12);
						nvgFillColor(args.vg, panelHarmonyPartColor); 
						nvgText(args.vg, pos.x, pos.y, text, NULL);
						nvgFillColor(args.vg, panelTextColor);
					}
				}
				// write last harmony note played 4
				if (module->theMeanderState.theHarmonyParms.last[3].note!=0)
				{
					if ((module->theMeanderState.theHarmonyParms.last[3].note>=0)&&(module->theMeanderState.theHarmonyParms.last[3].note<128))
					{
						pos=Vec(notesPlayingDisplayStartX+20+(3*37.5), notesPlayingDisplayNoteCenterY); 
						if ((module->theMeanderState.theHarmonyParms.last_chord_type==2)||(module->theMeanderState.theHarmonyParms.last_chord_type==3)||(module->theMeanderState.theHarmonyParms.last_chord_type==4)||(module->theMeanderState.theHarmonyParms.last_chord_type==5))
							snprintf(text, sizeof(text), "%s%d", module->note_desig[module->theMeanderState.theHarmonyParms.last[3].note%12], module->theMeanderState.theHarmonyParms.last[3].note/12);
						else
							snprintf(text, sizeof(text), "%s", "   ");
						nvgFillColor(args.vg, panelHarmonyPartColor); 
						nvgText(args.vg, pos.x, pos.y, text, NULL);
						nvgFillColor(args.vg, panelTextColor);
					}
				}
						

				// write last melody note played 
				for (int i=0; ((i<256)&&(i<module->bar_note_count)); ++i)
				{
					if (module->played_notes_circular_buffer[i].noteType==NOTE_TYPE_MELODY) // display even if ended to avoid strobing display
					{
						if ((module->theMeanderState.theMelodyParms.last[0].note>=0)&&(module->theMeanderState.theMelodyParms.last[0].note<128))
						{
							pos=Vec(notesPlayingDisplayStartX+20+(4*37.5), notesPlayingDisplayNoteCenterY); 
							snprintf(text, sizeof(text), "%s%d", module->note_desig[(module->theMeanderState.theMelodyParms.last[0].note%12)], (int)(module->theMeanderState.theMelodyParms.last[0].note/12 ));
							nvgFillColor(args.vg, panelMelodyPartColor); 
							nvgText(args.vg, pos.x, pos.y, text, NULL);
							nvgFillColor(args.vg, panelTextColor);
						}
					}
				}
			
				// write last arp note played 
				for (int i=0; ((i<256)&&(i<module->bar_note_count)); ++i)
				{
					if (module->played_notes_circular_buffer[i].noteType==NOTE_TYPE_ARP)  // display even if ended to avoid strobing display
					{
					//	if ((module->theMeanderState.theArpParms.last[module->theMeanderState.theArpParms.note_count].note>=0)&&(module->theMeanderState.theArpParms.last[module->theMeanderState.theArpParms.note_count].note<128))
						if ((module->theMeanderState.theArpParms.last[module->theMeanderState.theArpParms.note_count].note>0)&&(module->theMeanderState.theArpParms.last[module->theMeanderState.theArpParms.note_count].note<128))
						{
							pos=Vec(notesPlayingDisplayStartX+20+(5*37.5), notesPlayingDisplayNoteCenterY); 
							snprintf(text, sizeof(text), "%s%d", module->note_desig[(module->theMeanderState.theArpParms.last[module->theMeanderState.theArpParms.note_count].note%12)], (int)(module->theMeanderState.theArpParms.last[module->theMeanderState.theArpParms.note_count].note/12 ));
							nvgFillColor(args.vg, panelArpPartColor); 
							nvgText(args.vg, pos.x, pos.y, text, NULL);
							nvgFillColor(args.vg, panelTextColor);
						}
					}
				}
							
				// write last bass note played 
				if (module->theMeanderState.theBassParms.last[0].note!=0)
				{
					if ((module->theMeanderState.theBassParms.last[0].note>=0)&&(module->theMeanderState.theBassParms.last[0].note<128))
					{
						pos=Vec(notesPlayingDisplayStartX+20+(6*37.5), notesPlayingDisplayNoteCenterY); 
						snprintf(text, sizeof(text), "%s%d", module->note_desig[(module->theMeanderState.theBassParms.last[0].note%12)], (module->theMeanderState.theBassParms.last[0].note/12));
						nvgFillColor(args.vg, panelBassPartColor); 
						nvgText(args.vg, pos.x, pos.y, text, NULL);
						nvgFillColor(args.vg, panelTextColor);
					}
				}

				// write last octave bass note played 
				if (module->theMeanderState.theBassParms.octave_enabled)
				{
					if (module->theMeanderState.theBassParms.last[1].note!=0)
					{
						if ((module->theMeanderState.theBassParms.last[1].note>=0)&&(module->theMeanderState.theBassParms.last[1].note<128))
						{
							pos=Vec(notesPlayingDisplayStartX+20+(7*37.5), notesPlayingDisplayNoteCenterY); 
							snprintf(text, sizeof(text), "%s%d", module->note_desig[(module->theMeanderState.theBassParms.last[1].note%12)], (module->theMeanderState.theBassParms.last[1].note/12));
							nvgFillColor(args.vg,panelBassPartColor); 
							nvgText(args.vg, pos.x, pos.y, text, NULL);
							nvgFillColor(args.vg, panelTextColor);
						}
					}
				}
			}

			int last_chord_root=module->theMeanderState.last_harmony_chord_root_note%12;
			int last_chord_bass_note=module->theMeanderState.theHarmonyParms.last[0].note%12;
			pos=convertSVGtoNVG(110, 62, 12.1, 6.5);  // X,Y,W,H in Inkscape mm units
			nvgFontSize(args.vg, 30);
			nvgTextAlign(args.vg,NVG_ALIGN_CENTER|NVG_ALIGN_MIDDLE);

			nvgFillColor(args.vg, panelHarmonyPartColor); 

			char chord_type_desc[16];
			if (module->theMeanderState.theHarmonyParms.last_chord_type==0)
				strcpy(chord_type_desc, "");
			else
			if (module->theMeanderState.theHarmonyParms.last_chord_type==2)  // dom
				strcpy(chord_type_desc, "dom7");
			else
			if (module->theMeanderState.theHarmonyParms.last_chord_type==1)
				strcpy(chord_type_desc, "m");
			else
			if (module->theMeanderState.theHarmonyParms.last_chord_type==3)
				strcpy(chord_type_desc, "7");
			else
			if (module->theMeanderState.theHarmonyParms.last_chord_type==4)
				strcpy(chord_type_desc, "m7");
			else
			if (module->theMeanderState.theHarmonyParms.last_chord_type==5)
				strcpy(chord_type_desc, "dim7");
			else
			if (module->theMeanderState.theHarmonyParms.last_chord_type==6)
				strcpy(chord_type_desc, "dim");

			if (last_chord_bass_note!=last_chord_root) 
				snprintf(text, sizeof(text), "%s%s/%s", module->note_desig[last_chord_root], chord_type_desc, module->note_desig[last_chord_bass_note]);
			else
				snprintf(text, sizeof(text), "%s%s", module->note_desig[last_chord_root], chord_type_desc);

			nvgText(args.vg, pos.x, pos.y, text, NULL);


			// display chord circle degree in circle above staves

			// draw text
			nvgFontSize(args.vg, 15);
			if (textfont)
			nvgFontFaceId(args.vg, textfont->handle);	
			nvgTextLetterSpacing(args.vg, -1); // as close as possible
			nvgFillColor(args.vg, panelHarmonyPartColor); 
			
			snprintf(text, sizeof(text), "%s", circle_of_fifths_arabic_degrees[module->current_circle_degree]);
			Vec TextPosition=module->theCircleOf5ths.CircleCenter.plus(Vec(0,-60));
			nvgTextAlign(args.vg,NVG_ALIGN_CENTER|NVG_ALIGN_MIDDLE);
			nvgText(args.vg, TextPosition.x, TextPosition.y, text, NULL);
			

			//
			
		//	drawGrid(args);  // here after all updates are completed so grid is on top
	
			if (module->theMeanderState.renderKeyboardEnabled)
			{
				drawKeyboard(args); // redraw full keyboard per frame. clears any key down states
				int keyboard_offset=12.0; // adjust note range to middle C on piano keyboard

				if (module->theMeanderState.theHarmonyParms.last_chord_playing)
				{
					drawKey(args, module->theMeanderState.theHarmonyParms.last[0].note+keyboard_offset, true, 1);  // note: the gate end timer sets last[n].note to 0, causing drawKey to ignore drawing this key since it is no longer valid
					drawKey(args, module->theMeanderState.theHarmonyParms.last[1].note+keyboard_offset, true, 1);
					drawKey(args, module->theMeanderState.theHarmonyParms.last[2].note+keyboard_offset, true, 1);
					if ((module->theMeanderState.theHarmonyParms.last_chord_type==2)||(module->theMeanderState.theHarmonyParms.last_chord_type==3)||(module->theMeanderState.theHarmonyParms.last_chord_type==4)||(module->theMeanderState.theHarmonyParms.last_chord_type==5))
						drawKey(args, module->theMeanderState.theHarmonyParms.last[3].note+keyboard_offset, true, 1);  // 7th chords
				}

				for (int i=0; ((i<256)&&(i<module->bar_note_count)); ++i)
				{
					if ((module->played_notes_circular_buffer[i].noteType==NOTE_TYPE_MELODY) && (module->played_notes_circular_buffer[i].isPlaying))
						drawKey(args, module->played_notes_circular_buffer[i].note+keyboard_offset, true, 2);

					if ((module->played_notes_circular_buffer[i].noteType==NOTE_TYPE_ARP) && (module->played_notes_circular_buffer[i].isPlaying))
						drawKey(args, module->played_notes_circular_buffer[i].note+keyboard_offset, true, 3);

					if ((module->played_notes_circular_buffer[i].noteType==NOTE_TYPE_BASS) && (module->played_notes_circular_buffer[i].isPlaying))
						drawKey(args, module->played_notes_circular_buffer[i].note+keyboard_offset, true, 4);
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
				
				if (panelTheme==0)  // light theme
				{
					std::shared_ptr<Image> lightPanelImage = APP->window->loadImage(asset::plugin(pluginInstance,"res/Meander-light.png"));
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
					std::shared_ptr<Image> darkPanelImage = APP->window->loadImage(asset::plugin(pluginInstance,"res/Meander-dark.png"));
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
				#if defined(_USE_WINDOWS_PERFTIME)
				LARGE_INTEGER t;
				QueryPerformanceFrequency(&t);
				double frequency=double(t.QuadPart);
				QueryPerformanceCounter(&t);
				int64_t n= t.QuadPart;
				double count= (double)(n)/frequency;
				double time1=count;
				#endif
            							
				// draw Meander logo and chord legend
				
				std::shared_ptr<Font> textfont = APP->window->loadFont(asset::plugin(pluginInstance, "res/Ubuntu Condensed 400.ttf"));
							
				if (textfont)
				{
					nvgBeginPath(args.vg);
			    	nvgFontSize(args.vg, 30);
					nvgFontFaceId(args.vg, textfont->handle);
					nvgTextLetterSpacing(args.vg, -1);
					nvgFillColor(args.vg, panelTextColor);
					nvgTextAlign(args.vg,NVG_ALIGN_CENTER|NVG_ALIGN_MIDDLE);
					
					char text[128];
					snprintf(text, sizeof(text), "%s", "PS   Meander");  
					Vec pos=Vec(345, 17);  
					nvgStrokeWidth(args.vg, 3.0);
					nvgText(args.vg, pos.x, pos.y, text, NULL);
				

					snprintf(text, sizeof(text), "%s", "Harmonic Progression Diatonic Circle of 5ths");  
					nvgFontSize(args.vg, 15);
					pos=Vec(345, 35);  
					nvgStrokeWidth(args.vg, 2.0);
					nvgText(args.vg, pos.x, pos.y, text, NULL);

					nvgClosePath(args.vg);

					nvgStrokeWidth(args.vg, 1.0);
					nvgStrokeColor(args.vg, panelLineColor);

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
					nvgFillColor(args.vg, panelTextColor);
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
					nvgFillColor(args.vg, panelTextColor);
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
					nvgFillColor(args.vg, panelTextColor);
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
							nvgFillColor(args.vg, paramTextColor);
						
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
				 
				
				#if defined(_USE_WINDOWS_PERFTIME)
				QueryPerformanceCounter(&t);
				n= t.QuadPart;
				count= (double)(n)/frequency;
				double time2=count;
				double deltaTime=time2-time1;
				smoothedDt=(.9*smoothedDt) + (.1*(deltaTime));
				
				nvgFontSize(args.vg, 12);
				if (textfont)
				nvgFontFaceId(args.vg, textfont->handle);
				nvgTextAlign(args.vg,NVG_ALIGN_LEFT|NVG_ALIGN_MIDDLE); 
				nvgTextLetterSpacing(args.vg, -1);
				nvgFillColor(args.vg, nvgRGBA(0x0, 0x0, 0x0, 0xFF));
				Vec pos=Vec(10, 338);
				char text[128];
				snprintf(text, sizeof(text), "UI   %.2lf ms", smoothedDt*1000.0);
				nvgText(args.vg, pos.x, pos.y, text, NULL);
				#endif
				
			} 
		}  

		
	
	};  // end struct CircleOf5thsDisplay  

	float dummytempo=120;  // for module==null browser visibility purposes
	int dummysig=4;        // for module==null browser visibility purposes
	int dummyindex=0;      // for module==null browser visibility purposes

	MeanderWidget(Meander* module)   // all plugins I've looked at use this constructor with module*, even though docs show it deprecated.     
	{ 
		setModule(module);  // most plugins do this
		this->module = module;  //  most plugins do not do this.  It was introduced in singleton implementation
			
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Meander-light.svg")));
		svgPanel=(SvgPanel*)getPanel();
		svgPanel->setVisible((panelTheme) == 0);  
				
		panelcolor=nvgRGBA((unsigned char)230,(unsigned char)230,(unsigned char)230,(unsigned char)255);
		
		darkPanel = new SvgPanel();
		darkPanel->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Meander-dark.svg")));
		darkPanel->setVisible((panelTheme) == 1);  
		addChild(darkPanel);
							
		rack::random::init();  // must be called per thread

		 if (true)   // must be executed even if module* is null. module* is checked for null below where accessed as it is null in browser preview
		 {
			// create param widgets  Needs to be done even if module is null.
			
			RootKeySelectLineDisplay *MeanderRootKeySelectDisplay = new RootKeySelectLineDisplay();
			MeanderRootKeySelectDisplay->box.pos = Vec(115.,175.);  
			MeanderRootKeySelectDisplay->box.size = Vec(40, 22); 
			MeanderRootKeySelectDisplay->module=module;
			if (module) 
				MeanderRootKeySelectDisplay->val = &module->root_key;
			else
			{ 
				MeanderRootKeySelectDisplay->val = &dummyindex;  // strictly for browser visibility
			}
			addChild(MeanderRootKeySelectDisplay);

			ScaleSelectLineDisplay *MeanderScaleSelectDisplay = new ScaleSelectLineDisplay();
			MeanderScaleSelectDisplay->box.pos = Vec(30.,228.); 
			MeanderScaleSelectDisplay->box.size = Vec(130, 22); 
			MeanderScaleSelectDisplay->module=module;
			if (module) 
				MeanderScaleSelectDisplay->val = &module->mode;
			else
			{ 
				MeanderScaleSelectDisplay->val = &dummyindex;  // strictly for browser visibility
			}
			
			addChild(MeanderScaleSelectDisplay);

			CircleOf5thsDisplay *display = new CircleOf5thsDisplay(module);
			display->ParameterRectLocal=ParameterRect;
			display->InportRectLocal=InportRect;  
			display->OutportRectLocal=OutportRect;  
			display->box.pos = Vec(0, 0);
			display->box.size = Vec(box.size.x, box.size.y);
			addChild(display);
				
			//BPM DISPLAY  
			BpmDisplayWidget *BPMdisplay = new BpmDisplayWidget();
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
			SigDisplayWidget *SigTopDisplay = new SigDisplayWidget();
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
			SigDisplayWidget *SigBottomDisplay = new SigDisplayWidget();
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
			paramWidgets[Meander::BUTTON_CIRCLESTEP_C_PARAM]=createParamCentered<LEDButton>(mm2px(Vec(116.227, 37.257)), module, Meander::BUTTON_CIRCLESTEP_C_PARAM);
			addParam(paramWidgets[Meander::BUTTON_CIRCLESTEP_C_PARAM]);
			lightWidgets[Meander::LIGHT_LEDBUTTON_CIRCLESTEP_1]=createLightCentered<MediumSimpleLight<GreenLight>>(mm2px(Vec(116.227, 37.257)), module, Meander::LIGHT_LEDBUTTON_CIRCLESTEP_1);
			addChild(lightWidgets[Meander::LIGHT_LEDBUTTON_CIRCLESTEP_1]);
		
			paramWidgets[Meander::BUTTON_CIRCLESTEP_G_PARAM]=createParamCentered<LEDButton>(mm2px(Vec(132.479, 41.32)), module, Meander::BUTTON_CIRCLESTEP_G_PARAM);
			addParam(paramWidgets[Meander::BUTTON_CIRCLESTEP_G_PARAM]);
			lightWidgets[Meander::LIGHT_LEDBUTTON_CIRCLESTEP_2]=createLightCentered<MediumSimpleLight<GreenLight>>(mm2px(Vec(132.479, 41.32)), module, Meander::LIGHT_LEDBUTTON_CIRCLESTEP_2);
			addChild(lightWidgets[Meander::LIGHT_LEDBUTTON_CIRCLESTEP_2]);
		
			paramWidgets[Meander::BUTTON_CIRCLESTEP_D_PARAM]=createParamCentered<LEDButton>(mm2px(Vec(143.163, 52.155)), module, Meander::BUTTON_CIRCLESTEP_D_PARAM);
			addParam(paramWidgets[Meander::BUTTON_CIRCLESTEP_D_PARAM]);
			lightWidgets[Meander::LIGHT_LEDBUTTON_CIRCLESTEP_3]=createLightCentered<MediumSimpleLight<GreenLight>>(mm2px(Vec(143.163, 52.155)), module, Meander::LIGHT_LEDBUTTON_CIRCLESTEP_3);
			addChild(lightWidgets[Meander::LIGHT_LEDBUTTON_CIRCLESTEP_3]);
		
			paramWidgets[Meander::BUTTON_CIRCLESTEP_A_PARAM]=createParamCentered<LEDButton>(mm2px(Vec(147.527, 67.353)), module, Meander::BUTTON_CIRCLESTEP_A_PARAM);
			addParam(paramWidgets[Meander::BUTTON_CIRCLESTEP_A_PARAM]);
			lightWidgets[Meander::LIGHT_LEDBUTTON_CIRCLESTEP_4]=createLightCentered<MediumSimpleLight<GreenLight>>(mm2px(Vec(147.527, 67.353)), module, Meander::LIGHT_LEDBUTTON_CIRCLESTEP_4);
			addChild(lightWidgets[Meander::LIGHT_LEDBUTTON_CIRCLESTEP_4]);
		
			paramWidgets[Meander::BUTTON_CIRCLESTEP_E_PARAM]=createParamCentered<LEDButton>(mm2px(Vec(141.96, 83.906)), module, Meander::BUTTON_CIRCLESTEP_E_PARAM);
			addParam(paramWidgets[Meander::BUTTON_CIRCLESTEP_E_PARAM]);
			lightWidgets[Meander::LIGHT_LEDBUTTON_CIRCLESTEP_5]=createLightCentered<MediumSimpleLight<GreenLight>>(mm2px(Vec(141.96, 83.906)), module, Meander::LIGHT_LEDBUTTON_CIRCLESTEP_5);
			addChild(lightWidgets[Meander::LIGHT_LEDBUTTON_CIRCLESTEP_5]);
		
			paramWidgets[Meander::BUTTON_CIRCLESTEP_B_PARAM]=createParamCentered<LEDButton>(mm2px(Vec(132.931, 94.44)), module, Meander::BUTTON_CIRCLESTEP_B_PARAM);
			addParam(paramWidgets[Meander::BUTTON_CIRCLESTEP_B_PARAM]);
			lightWidgets[Meander::LIGHT_LEDBUTTON_CIRCLESTEP_6]=createLightCentered<MediumSimpleLight<GreenLight>>(mm2px(Vec(132.931, 94.44)), module, Meander::LIGHT_LEDBUTTON_CIRCLESTEP_6);
			addChild(lightWidgets[Meander::LIGHT_LEDBUTTON_CIRCLESTEP_6]);
		
			paramWidgets[Meander::BUTTON_CIRCLESTEP_GBFS_PARAM]=createParamCentered<LEDButton>(mm2px(Vec(116.378, 98.804)), module, Meander::BUTTON_CIRCLESTEP_GBFS_PARAM);
			addParam(paramWidgets[Meander::BUTTON_CIRCLESTEP_GBFS_PARAM]);
			lightWidgets[Meander::LIGHT_LEDBUTTON_CIRCLESTEP_7]=createLightCentered<MediumSimpleLight<GreenLight>>(mm2px(Vec(116.378, 98.804)), module, Meander::LIGHT_LEDBUTTON_CIRCLESTEP_7);
			addChild(lightWidgets[Meander::LIGHT_LEDBUTTON_CIRCLESTEP_7]);
		
			paramWidgets[Meander::BUTTON_CIRCLESTEP_DB_PARAM]=createParamCentered<LEDButton>(mm2px(Vec(101.029, 93.988)), module, Meander::BUTTON_CIRCLESTEP_DB_PARAM);
			addParam(paramWidgets[Meander::BUTTON_CIRCLESTEP_DB_PARAM]);
			lightWidgets[Meander::LIGHT_LEDBUTTON_CIRCLESTEP_8]=createLightCentered<MediumSimpleLight<GreenLight>>(mm2px(Vec(101.029, 93.988)), module, Meander::LIGHT_LEDBUTTON_CIRCLESTEP_8);
			addChild(lightWidgets[Meander::LIGHT_LEDBUTTON_CIRCLESTEP_8]);
		
			paramWidgets[Meander::BUTTON_CIRCLESTEP_AB_PARAM]=createParamCentered<LEDButton>(mm2px(Vec(91.097, 83.906)), module, Meander::BUTTON_CIRCLESTEP_AB_PARAM);
			addParam(paramWidgets[Meander::BUTTON_CIRCLESTEP_AB_PARAM]);
			lightWidgets[Meander::LIGHT_LEDBUTTON_CIRCLESTEP_9]=createLightCentered<MediumSimpleLight<GreenLight>>(mm2px(Vec(91.097, 83.906)), module, Meander::LIGHT_LEDBUTTON_CIRCLESTEP_9);
			addChild(lightWidgets[Meander::LIGHT_LEDBUTTON_CIRCLESTEP_9]);
		
			paramWidgets[Meander::BUTTON_CIRCLESTEP_EB_PARAM]=createParamCentered<LEDButton>(mm2px(Vec(86.282, 68.106)), module, Meander::BUTTON_CIRCLESTEP_EB_PARAM);
			addParam(paramWidgets[Meander::BUTTON_CIRCLESTEP_EB_PARAM]);
			lightWidgets[Meander::LIGHT_LEDBUTTON_CIRCLESTEP_10]=createLightCentered<MediumSimpleLight<GreenLight>>(mm2px(Vec(86.282, 68.106)), module, Meander::LIGHT_LEDBUTTON_CIRCLESTEP_10);
			addChild(lightWidgets[Meander::LIGHT_LEDBUTTON_CIRCLESTEP_10]);
		
			paramWidgets[Meander::BUTTON_CIRCLESTEP_BB_PARAM]=createParamCentered<LEDButton>(mm2px(Vec(89.743, 52.004)), module, Meander::BUTTON_CIRCLESTEP_BB_PARAM);
			addParam(paramWidgets[Meander::BUTTON_CIRCLESTEP_BB_PARAM]);
			lightWidgets[Meander::LIGHT_LEDBUTTON_CIRCLESTEP_11]=createLightCentered<MediumSimpleLight<GreenLight>>(mm2px(Vec(189.743, 52.004)), module, Meander::LIGHT_LEDBUTTON_CIRCLESTEP_11);
			addChild(lightWidgets[Meander::LIGHT_LEDBUTTON_CIRCLESTEP_11]);
		
			paramWidgets[Meander::BUTTON_CIRCLESTEP_F_PARAM]=createParamCentered<LEDButton>(mm2px(Vec(101.781, 40.568)), module, Meander::BUTTON_CIRCLESTEP_F_PARAM);
			addParam(paramWidgets[Meander::BUTTON_CIRCLESTEP_F_PARAM]);
			lightWidgets[Meander::LIGHT_LEDBUTTON_CIRCLESTEP_12]=createLightCentered<MediumSimpleLight<GreenLight>>(mm2px(Vec(101.781, 40.568)), module, Meander::LIGHT_LEDBUTTON_CIRCLESTEP_12);
			addChild(lightWidgets[Meander::LIGHT_LEDBUTTON_CIRCLESTEP_12]);
		
	//*************
			lightWidgets[Meander::LIGHT_CIRCLE_ROOT_KEY_POSITION_1_LIGHT]=createLightCentered<MediumLight<RedLight>>(mm2px(Vec(116.227, 43.878)), module, Meander::LIGHT_CIRCLE_ROOT_KEY_POSITION_1_LIGHT);
			addChild(lightWidgets[Meander::LIGHT_CIRCLE_ROOT_KEY_POSITION_1_LIGHT]);
		
			lightWidgets[Meander::LIGHT_CIRCLE_ROOT_KEY_POSITION_2_LIGHT]=createLightCentered<MediumLight<RedLight>>(mm2px(Vec(129.018, 47.189)), module, Meander::LIGHT_CIRCLE_ROOT_KEY_POSITION_2_LIGHT);
			addChild(lightWidgets[Meander::LIGHT_CIRCLE_ROOT_KEY_POSITION_2_LIGHT]);
		
			lightWidgets[Meander::LIGHT_CIRCLE_ROOT_KEY_POSITION_3_LIGHT]=createLightCentered<MediumLight<RedLight>>(mm2px(Vec(137.295, 56.067)), module, Meander::LIGHT_CIRCLE_ROOT_KEY_POSITION_3_LIGHT);
			addChild(lightWidgets[Meander::LIGHT_CIRCLE_ROOT_KEY_POSITION_3_LIGHT]);
		
			lightWidgets[Meander::LIGHT_CIRCLE_ROOT_KEY_POSITION_4_LIGHT]=createLightCentered<MediumLight<RedLight>>(mm2px(Vec(140.455, 67.654)), module, Meander::LIGHT_CIRCLE_ROOT_KEY_POSITION_4_LIGHT);
			addChild(lightWidgets[Meander::LIGHT_CIRCLE_ROOT_KEY_POSITION_4_LIGHT]);
		
			lightWidgets[Meander::LIGHT_CIRCLE_ROOT_KEY_POSITION_5_LIGHT]=createLightCentered<MediumLight<RedLight>>(mm2px(Vec(137.144, 80.295)), module, Meander::LIGHT_CIRCLE_ROOT_KEY_POSITION_5_LIGHT);
			addChild(lightWidgets[Meander::LIGHT_CIRCLE_ROOT_KEY_POSITION_5_LIGHT]);
		
			lightWidgets[Meander::LIGHT_CIRCLE_ROOT_KEY_POSITION_6_LIGHT]=createLightCentered<MediumLight<RedLight>>(mm2px(Vec(128.868, 88.571)), module, Meander::LIGHT_CIRCLE_ROOT_KEY_POSITION_6_LIGHT);
			addChild(lightWidgets[Meander::LIGHT_CIRCLE_ROOT_KEY_POSITION_6_LIGHT]);
		
			lightWidgets[Meander::LIGHT_CIRCLE_ROOT_KEY_POSITION_7_LIGHT]=createLightCentered<MediumLight<RedLight>>(mm2px(Vec(116.077, 92.183)), module, Meander::LIGHT_CIRCLE_ROOT_KEY_POSITION_7_LIGHT);
			addChild(lightWidgets[Meander::LIGHT_CIRCLE_ROOT_KEY_POSITION_7_LIGHT]);
		
			lightWidgets[Meander::LIGHT_CIRCLE_ROOT_KEY_POSITION_8_LIGHT]=createLightCentered<MediumLight<RedLight>>(mm2px(Vec(104.791, 88.872)), module, Meander::LIGHT_CIRCLE_ROOT_KEY_POSITION_8_LIGHT);
			addChild(lightWidgets[Meander::LIGHT_CIRCLE_ROOT_KEY_POSITION_8_LIGHT]);
		
			lightWidgets[Meander::LIGHT_CIRCLE_ROOT_KEY_POSITION_9_LIGHT]=createLightCentered<MediumLight<RedLight>>(mm2px(Vec(96.213, 80.596)), module, Meander::LIGHT_CIRCLE_ROOT_KEY_POSITION_9_LIGHT);
			addChild(lightWidgets[Meander::LIGHT_CIRCLE_ROOT_KEY_POSITION_9_LIGHT]);
		
			lightWidgets[Meander::LIGHT_CIRCLE_ROOT_KEY_POSITION_10_LIGHT]=createLightCentered<MediumLight<RedLight>>(mm2px(Vec(92.602, 67.654)), module, Meander::LIGHT_CIRCLE_ROOT_KEY_POSITION_10_LIGHT);
			addChild(lightWidgets[Meander::LIGHT_CIRCLE_ROOT_KEY_POSITION_10_LIGHT]);
		
			lightWidgets[Meander::LIGHT_CIRCLE_ROOT_KEY_POSITION_11_LIGHT]=createLightCentered<MediumLight<RedLight>>(mm2px(Vec(95.912, 55.465)), module, Meander::LIGHT_CIRCLE_ROOT_KEY_POSITION_11_LIGHT);
			addChild(lightWidgets[Meander::LIGHT_CIRCLE_ROOT_KEY_POSITION_11_LIGHT]);
		
			lightWidgets[Meander::LIGHT_CIRCLE_ROOT_KEY_POSITION_12_LIGHT]=createLightCentered<MediumLight<RedLight>>(mm2px(Vec(105.393, 46.587)), module, Meander::LIGHT_CIRCLE_ROOT_KEY_POSITION_12_LIGHT);
			addChild(lightWidgets[Meander::LIGHT_CIRCLE_ROOT_KEY_POSITION_12_LIGHT]);
				 		

	//*********** Harmony step set selection    

					
			paramWidgets[Meander::BUTTON_HARMONY_SETSTEP_1_PARAM]=createParamCentered<LEDButton>(mm2px(Vec(65.197, 106.483)), module, Meander::BUTTON_HARMONY_SETSTEP_1_PARAM);
			addParam(paramWidgets[Meander::BUTTON_HARMONY_SETSTEP_1_PARAM]);
			lightWidgets[Meander::LIGHT_LEDBUTTON_CIRCLESETSTEP_1]=createLightCentered<MediumSimpleLight<GreenLight>>(mm2px(Vec(65.197, 106.483)), module, Meander::LIGHT_LEDBUTTON_CIRCLESETSTEP_1);
			addChild(lightWidgets[Meander::LIGHT_LEDBUTTON_CIRCLESETSTEP_1]);

			
			paramWidgets[Meander::BUTTON_HARMONY_SETSTEP_2_PARAM]=createParamCentered<LEDButton>(mm2px(Vec(64.918, 98.02)), module, Meander::BUTTON_HARMONY_SETSTEP_2_PARAM);
			addParam(paramWidgets[Meander::BUTTON_HARMONY_SETSTEP_2_PARAM]);
			lightWidgets[Meander::LIGHT_LEDBUTTON_CIRCLESETSTEP_2]=createLightCentered<MediumSimpleLight<GreenLight>>(mm2px(Vec(64.918, 98.02)), module, Meander::LIGHT_LEDBUTTON_CIRCLESETSTEP_2);
			addChild(lightWidgets[Meander::LIGHT_LEDBUTTON_CIRCLESETSTEP_2]);

		
			paramWidgets[Meander::BUTTON_HARMONY_SETSTEP_3_PARAM]=createParamCentered<LEDButton>(mm2px(Vec(65.193, 89.271)), module, Meander::BUTTON_HARMONY_SETSTEP_3_PARAM);
			addParam(paramWidgets[Meander::BUTTON_HARMONY_SETSTEP_3_PARAM]);
			lightWidgets[Meander::LIGHT_LEDBUTTON_CIRCLESETSTEP_3]=createLightCentered<MediumSimpleLight<GreenLight>>(mm2px(Vec(65.193, 89.271)), module, Meander::LIGHT_LEDBUTTON_CIRCLESETSTEP_3);
			addChild(lightWidgets[Meander::LIGHT_LEDBUTTON_CIRCLESETSTEP_3]);

			
			paramWidgets[Meander::BUTTON_HARMONY_SETSTEP_4_PARAM]=createParamCentered<LEDButton>(mm2px(Vec(64.918, 81.9233)), module, Meander::BUTTON_HARMONY_SETSTEP_4_PARAM);
			addParam(paramWidgets[Meander::BUTTON_HARMONY_SETSTEP_4_PARAM]);
			lightWidgets[Meander::LIGHT_LEDBUTTON_CIRCLESETSTEP_4]=createLightCentered<MediumSimpleLight<GreenLight>>(mm2px(Vec(64.918, 81.923)), module, Meander::LIGHT_LEDBUTTON_CIRCLESETSTEP_4);
			addChild(lightWidgets[Meander::LIGHT_LEDBUTTON_CIRCLESETSTEP_4]);

		
			paramWidgets[Meander::BUTTON_HARMONY_SETSTEP_5_PARAM]=createParamCentered<LEDButton>(mm2px(Vec(65.193, 73.184)), module, Meander::BUTTON_HARMONY_SETSTEP_5_PARAM);
			addParam(paramWidgets[Meander::BUTTON_HARMONY_SETSTEP_5_PARAM]);
			lightWidgets[Meander::LIGHT_LEDBUTTON_CIRCLESETSTEP_5]=createLightCentered<MediumSimpleLight<GreenLight>>(mm2px(Vec(65.193, 73.184)), module, Meander::LIGHT_LEDBUTTON_CIRCLESETSTEP_5);
			addChild(lightWidgets[Meander::LIGHT_LEDBUTTON_CIRCLESETSTEP_5]);

			
			paramWidgets[Meander::BUTTON_HARMONY_SETSTEP_6_PARAM]=createParamCentered<LEDButton>(mm2px(Vec(64.918, 66.129)), module, Meander::BUTTON_HARMONY_SETSTEP_6_PARAM);
			addParam(paramWidgets[Meander::BUTTON_HARMONY_SETSTEP_6_PARAM]);
			lightWidgets[Meander::LIGHT_LEDBUTTON_CIRCLESETSTEP_6]=createLightCentered<MediumSimpleLight<GreenLight>>(mm2px(Vec(64.918, 66.129)), module, Meander::LIGHT_LEDBUTTON_CIRCLESETSTEP_6);
			addChild(lightWidgets[Meander::LIGHT_LEDBUTTON_CIRCLESETSTEP_6]);

			
			paramWidgets[Meander::BUTTON_HARMONY_SETSTEP_7_PARAM]=createParamCentered<LEDButton>(mm2px(Vec(65.193, 57.944)), module, Meander::BUTTON_HARMONY_SETSTEP_7_PARAM);
			addParam(paramWidgets[Meander::BUTTON_HARMONY_SETSTEP_7_PARAM]);
			lightWidgets[Meander::LIGHT_LEDBUTTON_CIRCLESETSTEP_7]=createLightCentered<MediumSimpleLight<GreenLight>>(mm2px(Vec(65.193, 57.944)), module, Meander::LIGHT_LEDBUTTON_CIRCLESETSTEP_7);
			addChild(lightWidgets[Meander::LIGHT_LEDBUTTON_CIRCLESETSTEP_7]);

			
			paramWidgets[Meander::BUTTON_HARMONY_SETSTEP_8_PARAM]=createParamCentered<LEDButton>(mm2px(Vec(64.911, 49.474)), module, Meander::BUTTON_HARMONY_SETSTEP_8_PARAM);
			addParam(paramWidgets[Meander::BUTTON_HARMONY_SETSTEP_8_PARAM]);
			lightWidgets[Meander::LIGHT_LEDBUTTON_CIRCLESETSTEP_8]=createLightCentered<MediumSimpleLight<GreenLight>>(mm2px(Vec(64.911, 49.474)), module, Meander::LIGHT_LEDBUTTON_CIRCLESETSTEP_8);
			addChild(lightWidgets[Meander::LIGHT_LEDBUTTON_CIRCLESETSTEP_8]);

			
			paramWidgets[Meander::BUTTON_HARMONY_SETSTEP_9_PARAM]=createParamCentered<LEDButton>(mm2px(Vec(4.629, 41.011)), module, Meander::BUTTON_HARMONY_SETSTEP_9_PARAM);
			addParam(paramWidgets[Meander::BUTTON_HARMONY_SETSTEP_9_PARAM]);
			lightWidgets[Meander::LIGHT_LEDBUTTON_CIRCLESETSTEP_9]=createLightCentered<MediumSimpleLight<GreenLight>>(mm2px(Vec(4.629, 41.011)), module, Meander::LIGHT_LEDBUTTON_CIRCLESETSTEP_9);
			addChild(lightWidgets[Meander::LIGHT_LEDBUTTON_CIRCLESETSTEP_9]);

			
			paramWidgets[Meander::BUTTON_HARMONY_SETSTEP_10_PARAM]=createParamCentered<LEDButton>(mm2px(Vec(64.629, 32.827)), module, Meander::BUTTON_HARMONY_SETSTEP_10_PARAM);
			addParam(paramWidgets[Meander::BUTTON_HARMONY_SETSTEP_10_PARAM]);
			lightWidgets[Meander::LIGHT_LEDBUTTON_CIRCLESETSTEP_10]=createLightCentered<MediumSimpleLight<GreenLight>>(mm2px(Vec(64.629, 32.827)), module, Meander::LIGHT_LEDBUTTON_CIRCLESETSTEP_10);
			addChild(lightWidgets[Meander::LIGHT_LEDBUTTON_CIRCLESETSTEP_10]);

			
			paramWidgets[Meander::BUTTON_HARMONY_SETSTEP_11_PARAM]=createParamCentered<LEDButton>(mm2px(Vec(64.629, 24.649)), module, Meander::BUTTON_HARMONY_SETSTEP_11_PARAM);
			addParam(paramWidgets[Meander::BUTTON_HARMONY_SETSTEP_11_PARAM]);
			lightWidgets[Meander::LIGHT_LEDBUTTON_CIRCLESETSTEP_11]=createLightCentered<MediumSimpleLight<GreenLight>>(mm2px(Vec(64.629, 24.649)), module, Meander::LIGHT_LEDBUTTON_CIRCLESETSTEP_11);
			addChild(lightWidgets[Meander::LIGHT_LEDBUTTON_CIRCLESETSTEP_11]);

			
			paramWidgets[Meander::BUTTON_HARMONY_SETSTEP_12_PARAM]=createParamCentered<LEDButton>(mm2px(Vec(64.632, 16.176)), module, Meander::BUTTON_HARMONY_SETSTEP_12_PARAM);
			addParam(paramWidgets[Meander::BUTTON_HARMONY_SETSTEP_12_PARAM]);
			lightWidgets[Meander::LIGHT_LEDBUTTON_CIRCLESETSTEP_12]=createLightCentered<MediumSimpleLight<GreenLight>>(mm2px(Vec(64.632, 16.176)), module, Meander::LIGHT_LEDBUTTON_CIRCLESETSTEP_12);
			addChild(lightWidgets[Meander::LIGHT_LEDBUTTON_CIRCLESETSTEP_12]);

		
			paramWidgets[Meander::BUTTON_HARMONY_SETSTEP_13_PARAM]=createParamCentered<LEDButton>(mm2px(Vec(64.632, 16.176)), module, Meander::BUTTON_HARMONY_SETSTEP_13_PARAM);
			addParam(paramWidgets[Meander::BUTTON_HARMONY_SETSTEP_13_PARAM]);
			lightWidgets[Meander::LIGHT_LEDBUTTON_CIRCLESETSTEP_13]=createLightCentered<MediumSimpleLight<GreenLight>>(mm2px(Vec(64.632, 16.176)), module, Meander::LIGHT_LEDBUTTON_CIRCLESETSTEP_13);
			addChild(lightWidgets[Meander::LIGHT_LEDBUTTON_CIRCLESETSTEP_13]);

		
			paramWidgets[Meander::BUTTON_HARMONY_SETSTEP_14_PARAM]=createParamCentered<LEDButton>(mm2px(Vec(64.632, 16.176)), module, Meander::BUTTON_HARMONY_SETSTEP_14_PARAM);
			addParam(paramWidgets[Meander::BUTTON_HARMONY_SETSTEP_14_PARAM]);
			lightWidgets[Meander::LIGHT_LEDBUTTON_CIRCLESETSTEP_14]=createLightCentered<MediumSimpleLight<GreenLight>>(mm2px(Vec(64.632, 16.176)), module, Meander::LIGHT_LEDBUTTON_CIRCLESETSTEP_14);
			addChild(lightWidgets[Meander::LIGHT_LEDBUTTON_CIRCLESETSTEP_14]);

			
			paramWidgets[Meander::BUTTON_HARMONY_SETSTEP_15_PARAM]=createParamCentered<LEDButton>(mm2px(Vec(64.632, 16.176)), module, Meander::BUTTON_HARMONY_SETSTEP_15_PARAM);
			addParam(paramWidgets[Meander::BUTTON_HARMONY_SETSTEP_15_PARAM]);
			lightWidgets[Meander::LIGHT_LEDBUTTON_CIRCLESETSTEP_15]=createLightCentered<MediumSimpleLight<GreenLight>>(mm2px(Vec(64.632, 16.176)), module, Meander::LIGHT_LEDBUTTON_CIRCLESETSTEP_15);
			addChild(lightWidgets[Meander::LIGHT_LEDBUTTON_CIRCLESETSTEP_15]);

			
			paramWidgets[Meander::BUTTON_HARMONY_SETSTEP_16_PARAM]=createParamCentered<LEDButton>(mm2px(Vec(64.632, 16.176)), module, Meander::BUTTON_HARMONY_SETSTEP_16_PARAM);
			addParam(paramWidgets[Meander::BUTTON_HARMONY_SETSTEP_16_PARAM]);
			lightWidgets[Meander::LIGHT_LEDBUTTON_CIRCLESETSTEP_16]=createLightCentered<MediumSimpleLight<GreenLight>>(mm2px(Vec(64.632, 16.176)), module, Meander::LIGHT_LEDBUTTON_CIRCLESETSTEP_16);
			addChild(lightWidgets[Meander::LIGHT_LEDBUTTON_CIRCLESETSTEP_16]);

			//**********General************************
			
			paramWidgets[Meander::BUTTON_RUN_PARAM]=createParamCentered<LEDButton>(mm2px(Vec(19.7, 10.45)), module, Meander::BUTTON_RUN_PARAM);
			addParam(paramWidgets[Meander::BUTTON_RUN_PARAM]);
			lightWidgets[Meander::LIGHT_LEDBUTTON_RUN]=createLightCentered<MediumSimpleLight<RedLight>>(mm2px(Vec(19.7, 10.45)), module, Meander::LIGHT_LEDBUTTON_RUN);
			addChild(lightWidgets[Meander::LIGHT_LEDBUTTON_RUN]);
        
			paramWidgets[Meander::BUTTON_RESET_PARAM]=createParamCentered<LEDButton>(mm2px(Vec(19.7, 22.55)), module, Meander::BUTTON_RESET_PARAM);
			addParam(paramWidgets[Meander::BUTTON_RESET_PARAM]);
			lightWidgets[Meander::LIGHT_LEDBUTTON_RESET]=createLightCentered<MediumSimpleLight<RedLight>>(mm2px(Vec(19.7, 22.55)), module, Meander::LIGHT_LEDBUTTON_RESET);
			addChild(lightWidgets[Meander::LIGHT_LEDBUTTON_RESET]);
         
			paramWidgets[Meander::CONTROL_TEMPOBPM_PARAM]=createParamCentered<Trimpot>(mm2px(Vec(8.12, 35.4)), module, Meander::CONTROL_TEMPOBPM_PARAM);
			addParam(paramWidgets[Meander::CONTROL_TEMPOBPM_PARAM]);
			
			paramWidgets[Meander::CONTROL_TIMESIGNATURETOP_PARAM]=createParamCentered<Trimpot>(mm2px(Vec(8.12, 47.322+3.0)), module, Meander::CONTROL_TIMESIGNATURETOP_PARAM);
			addParam(paramWidgets[Meander::CONTROL_TIMESIGNATURETOP_PARAM]);
		
			paramWidgets[Meander::CONTROL_TIMESIGNATUREBOTTOM_PARAM]=createParamCentered<Trimpot>(mm2px(Vec(8.12, 54.84+3.0)), module, Meander::CONTROL_TIMESIGNATUREBOTTOM_PARAM);
			addParam(paramWidgets[Meander::CONTROL_TIMESIGNATUREBOTTOM_PARAM]);
			
			paramWidgets[Meander::CONTROL_ROOT_KEY_PARAM]=createParamCentered<Trimpot>(mm2px(Vec(8.12, 68.179)), module, Meander::CONTROL_ROOT_KEY_PARAM);
			dynamic_cast<Knob*>(paramWidgets[Meander::CONTROL_ROOT_KEY_PARAM])->snap=true;
			addParam(paramWidgets[Meander::CONTROL_ROOT_KEY_PARAM]);
			
			paramWidgets[Meander::CONTROL_SCALE_PARAM]=createParamCentered<Trimpot>(mm2px(Vec(8.12, 78.675)), module, Meander::CONTROL_SCALE_PARAM);
			dynamic_cast<Knob*>(paramWidgets[Meander::CONTROL_SCALE_PARAM])->snap=true;
	 		addParam(paramWidgets[Meander::CONTROL_SCALE_PARAM]);

			//***************Harmony******************
						
			paramWidgets[Meander::BUTTON_ENABLE_HARMONY_PARAM]=createParamCentered<LEDButton>(mm2px(Vec(173.849, 12.622)), module, Meander::BUTTON_ENABLE_HARMONY_PARAM);
			addParam(paramWidgets[Meander::BUTTON_ENABLE_HARMONY_PARAM]);
			lightWidgets[Meander::LIGHT_LEDBUTTON_HARMONY_ENABLE]=createLightCentered<MediumSimpleLight<RedLight>>(mm2px(Vec(173.849, 12.622)), module, Meander::LIGHT_LEDBUTTON_HARMONY_ENABLE);
			addChild(lightWidgets[Meander::LIGHT_LEDBUTTON_HARMONY_ENABLE]);
		
			paramWidgets[Meander::CONTROL_HARMONY_VOLUME_PARAM]=createParamCentered<Trimpot>(mm2px(Vec(173.849, 20.384)), module, Meander::CONTROL_HARMONY_VOLUME_PARAM);
			addParam(paramWidgets[Meander::CONTROL_HARMONY_VOLUME_PARAM]);
				
			paramWidgets[Meander::CONTROL_HARMONY_STEPS_PARAM]=createParamCentered<Trimpot>(mm2px(Vec(174.028, 28)), module, Meander::CONTROL_HARMONY_STEPS_PARAM);
			dynamic_cast<Knob*>(paramWidgets[Meander::CONTROL_HARMONY_STEPS_PARAM])->snap=true;
			addParam(paramWidgets[Meander::CONTROL_HARMONY_STEPS_PARAM]);
		
			paramWidgets[Meander::CONTROL_HARMONY_TARGETOCTAVE_PARAM]=createParamCentered<Trimpot>(mm2px(Vec(174.01, 37.396)), module, Meander::CONTROL_HARMONY_TARGETOCTAVE_PARAM);
			dynamic_cast<Knob*>(paramWidgets[Meander::CONTROL_HARMONY_TARGETOCTAVE_PARAM])->snap=true;
			addParam(paramWidgets[Meander::CONTROL_HARMONY_TARGETOCTAVE_PARAM]);
			
			paramWidgets[Meander::CONTROL_HARMONY_ALPHA_PARAM]=createParamCentered<Trimpot>(mm2px(Vec(174.27, 45.982)), module, Meander::CONTROL_HARMONY_ALPHA_PARAM);
			addParam(paramWidgets[Meander::CONTROL_HARMONY_ALPHA_PARAM]);

			paramWidgets[Meander::CONTROL_HARMONY_RANGE_PARAM]=createParamCentered<Trimpot>(mm2px(Vec(173.991, 53.788)), module, Meander::CONTROL_HARMONY_RANGE_PARAM);
			addParam(paramWidgets[Meander::CONTROL_HARMONY_RANGE_PARAM]);
			  
			paramWidgets[Meander::CONTROL_HARMONY_DIVISOR_PARAM]=createParamCentered<Trimpot>(mm2px(Vec(173.953, 62.114)), module, Meander::CONTROL_HARMONY_DIVISOR_PARAM);
			dynamic_cast<Knob*>(paramWidgets[Meander::CONTROL_HARMONY_DIVISOR_PARAM])->snap=true;
			addParam(paramWidgets[Meander::CONTROL_HARMONY_DIVISOR_PARAM]);
			
			paramWidgets[Meander::BUTTON_ENABLE_HARMONY_ALL7THS_PARAM]=createParamCentered<LEDButton>(mm2px(Vec(173.849, 69)), module, Meander::BUTTON_ENABLE_HARMONY_ALL7THS_PARAM);
			addParam(paramWidgets[Meander::BUTTON_ENABLE_HARMONY_ALL7THS_PARAM]);
			lightWidgets[Meander::LIGHT_LEDBUTTON_ENABLE_HARMONY_ALL7THS_PARAM]=createLightCentered<MediumSimpleLight<RedLight>>(mm2px(Vec(173.849, 69)), module, Meander::LIGHT_LEDBUTTON_ENABLE_HARMONY_ALL7THS_PARAM);
			addChild(lightWidgets[Meander::LIGHT_LEDBUTTON_ENABLE_HARMONY_ALL7THS_PARAM]);
			
			paramWidgets[Meander::BUTTON_ENABLE_HARMONY_V7THS_PARAM]=createParamCentered<LEDButton>(mm2px(Vec(203.849, 69)), module, Meander::BUTTON_ENABLE_HARMONY_V7THS_PARAM);
			addParam(paramWidgets[Meander::BUTTON_ENABLE_HARMONY_V7THS_PARAM]);
			lightWidgets[Meander::LIGHT_LEDBUTTON_ENABLE_HARMONY_V7THS_PARAM]=createLightCentered<MediumSimpleLight<RedLight>>(mm2px(Vec(203.849, 69)), module, Meander::LIGHT_LEDBUTTON_ENABLE_HARMONY_V7THS_PARAM);
			addChild(lightWidgets[Meander::LIGHT_LEDBUTTON_ENABLE_HARMONY_V7THS_PARAM]);
			
			paramWidgets[Meander::BUTTON_ENABLE_HARMONY_STACCATO_PARAM]=createParamCentered<LEDButton>(mm2px(Vec(173.849, 75)), module, Meander::BUTTON_ENABLE_HARMONY_STACCATO_PARAM);
			addParam(paramWidgets[Meander::BUTTON_ENABLE_HARMONY_STACCATO_PARAM]);
			lightWidgets[Meander::LIGHT_LEDBUTTON_ENABLE_HARMONY_STACCATO_PARAM]=createLightCentered<MediumSimpleLight<RedLight>>(mm2px(Vec(173.849, 75)), module, Meander::LIGHT_LEDBUTTON_ENABLE_HARMONY_STACCATO_PARAM);
			addChild(lightWidgets[Meander::LIGHT_LEDBUTTON_ENABLE_HARMONY_STACCATO_PARAM]);
			
			paramWidgets[Meander::CONTROL_HARMONYPRESETS_PARAM]=createParamCentered<Trimpot>(mm2px(Vec(174.027, 81.524)), module, Meander::CONTROL_HARMONYPRESETS_PARAM);
	 		dynamic_cast<Knob*>(paramWidgets[Meander::CONTROL_HARMONYPRESETS_PARAM])->snap=true;
			addParam(paramWidgets[Meander::CONTROL_HARMONYPRESETS_PARAM]);

			//**************Melody********************
						
			paramWidgets[Meander::BUTTON_ENABLE_MELODY_PARAM]=createParamCentered<LEDButton>(mm2px(Vec(240.353, 10.986)), module, Meander::BUTTON_ENABLE_MELODY_PARAM);
			addParam(paramWidgets[Meander::BUTTON_ENABLE_MELODY_PARAM]);
			lightWidgets[Meander::LIGHT_LEDBUTTON_MELODY_ENABLE]=createLightCentered<MediumSimpleLight<RedLight>>(mm2px(Vec(240.353, 10.986)), module, Meander::LIGHT_LEDBUTTON_MELODY_ENABLE);
			addChild(lightWidgets[Meander::LIGHT_LEDBUTTON_MELODY_ENABLE]);
			
			paramWidgets[Meander::BUTTON_ENABLE_MELODY_CHORDAL_PARAM]=createParamCentered<LEDButton>(mm2px(Vec(270.353, 10.986)), module, Meander::BUTTON_ENABLE_MELODY_CHORDAL_PARAM);
			addParam(paramWidgets[Meander::BUTTON_ENABLE_MELODY_CHORDAL_PARAM]);
			lightWidgets[Meander::LIGHT_LEDBUTTON_MELODY_ENABLE_CHORDAL]=createLightCentered<MediumSimpleLight<RedLight>>(mm2px(Vec(270.353, 10.986)), module, Meander::LIGHT_LEDBUTTON_MELODY_ENABLE_CHORDAL);
			addChild(lightWidgets[Meander::LIGHT_LEDBUTTON_MELODY_ENABLE_CHORDAL]);
			
			paramWidgets[Meander::BUTTON_ENABLE_MELODY_SCALER_PARAM]=createParamCentered<LEDButton>(mm2px(Vec(287.274, 10.986)), module, Meander::BUTTON_ENABLE_MELODY_SCALER_PARAM);
			addParam(paramWidgets[Meander::BUTTON_ENABLE_MELODY_SCALER_PARAM]);
			lightWidgets[Meander::LIGHT_LEDBUTTON_MELODY_ENABLE_SCALER]=createLightCentered<MediumSimpleLight<RedLight>>(mm2px(Vec(287.274, 10.986)), module, Meander::LIGHT_LEDBUTTON_MELODY_ENABLE_SCALER);
			addChild(lightWidgets[Meander::LIGHT_LEDBUTTON_MELODY_ENABLE_SCALER]);
			
			paramWidgets[Meander::BUTTON_MELODY_DESTUTTER_PARAM]=createParamCentered<LEDButton>(mm2px(Vec(240.409, 25.524)), module, Meander::BUTTON_MELODY_DESTUTTER_PARAM);
			addParam(paramWidgets[Meander::BUTTON_MELODY_DESTUTTER_PARAM]);
			lightWidgets[Meander::LIGHT_LEDBUTTON_MELODY_DESTUTTER]=createLightCentered<MediumSimpleLight<RedLight>>(mm2px(Vec(240.409, 25.524)), module, Meander::LIGHT_LEDBUTTON_MELODY_DESTUTTER);
			addChild(lightWidgets[Meander::LIGHT_LEDBUTTON_MELODY_DESTUTTER]);
			
			paramWidgets[Meander::CONTROL_MELODY_VOLUME_PARAM]=createParamCentered<Trimpot>(mm2px(Vec(240.353, 19.217)), module, Meander::CONTROL_MELODY_VOLUME_PARAM);
			addParam(paramWidgets[Meander::CONTROL_MELODY_VOLUME_PARAM]);
 		
			paramWidgets[Meander::CONTROL_MELODY_NOTE_LENGTH_DIVISOR_PARAM]=createParamCentered<Trimpot>(mm2px(Vec(240.353, 32.217)), module, Meander::CONTROL_MELODY_NOTE_LENGTH_DIVISOR_PARAM);
			dynamic_cast<Knob*>(paramWidgets[Meander::CONTROL_MELODY_NOTE_LENGTH_DIVISOR_PARAM])->snap=true;
			addParam(paramWidgets[Meander::CONTROL_MELODY_NOTE_LENGTH_DIVISOR_PARAM]);
			
			paramWidgets[Meander::CONTROL_MELODY_TARGETOCTAVE_PARAM]=createParamCentered<Trimpot>(mm2px(Vec(240.353, 39.217)), module, Meander::CONTROL_MELODY_TARGETOCTAVE_PARAM);
			dynamic_cast<Knob*>(paramWidgets[Meander::CONTROL_MELODY_TARGETOCTAVE_PARAM])->snap=true;
			addParam(paramWidgets[Meander::CONTROL_MELODY_TARGETOCTAVE_PARAM]);
			
			paramWidgets[Meander::CONTROL_MELODY_ALPHA_PARAM]=createParamCentered<Trimpot>(mm2px(Vec(240.334, 47.803)), module, Meander::CONTROL_MELODY_ALPHA_PARAM);
			addParam(paramWidgets[Meander::CONTROL_MELODY_ALPHA_PARAM]);

			paramWidgets[Meander::CONTROL_MELODY_RANGE_PARAM]=createParamCentered<Trimpot>(mm2px(Vec(240.334, 55.33)), module, Meander::CONTROL_MELODY_RANGE_PARAM);
			addParam(paramWidgets[Meander::CONTROL_MELODY_RANGE_PARAM]);

			//*******************Arp********************
			
			paramWidgets[Meander::BUTTON_ENABLE_ARP_PARAM]=createParamCentered<LEDButton>(mm2px(Vec(240.274, 62.01)), module, Meander::BUTTON_ENABLE_ARP_PARAM);
			addParam(paramWidgets[Meander::BUTTON_ENABLE_ARP_PARAM]);
			lightWidgets[Meander::LIGHT_LEDBUTTON_ARP_ENABLE]=createLightCentered<MediumSimpleLight<RedLight>>(mm2px(Vec(240.274, 62.01)), module, Meander::LIGHT_LEDBUTTON_ARP_ENABLE);
			addChild(lightWidgets[Meander::LIGHT_LEDBUTTON_ARP_ENABLE]);

			paramWidgets[Meander::BUTTON_ENABLE_ARP_CHORDAL_PARAM]=createParamCentered<LEDButton>(mm2px(Vec(265.274, 62.01)), module, Meander::BUTTON_ENABLE_ARP_CHORDAL_PARAM);
			addParam(paramWidgets[Meander::BUTTON_ENABLE_ARP_CHORDAL_PARAM]);
			lightWidgets[Meander::LIGHT_LEDBUTTON_ARP_ENABLE_CHORDAL]=createLightCentered<MediumSimpleLight<RedLight>>(mm2px(Vec(265.274, 62.01)), module, Meander::LIGHT_LEDBUTTON_ARP_ENABLE_CHORDAL);
			addChild(lightWidgets[Meander::LIGHT_LEDBUTTON_ARP_ENABLE_CHORDAL]);

			paramWidgets[Meander::BUTTON_ENABLE_ARP_SCALER_PARAM]=createParamCentered<LEDButton>(mm2px(Vec(283.274, 62.01)), module, Meander::BUTTON_ENABLE_ARP_SCALER_PARAM);
			addParam(paramWidgets[Meander::BUTTON_ENABLE_ARP_SCALER_PARAM]);
			lightWidgets[Meander::LIGHT_LEDBUTTON_ARP_ENABLE_SCALER]=createLightCentered<MediumSimpleLight<RedLight>>(mm2px(Vec(283.274, 62.01)), module, Meander::LIGHT_LEDBUTTON_ARP_ENABLE_SCALER);
			addChild(lightWidgets[Meander::LIGHT_LEDBUTTON_ARP_ENABLE_SCALER]);

			paramWidgets[Meander::CONTROL_ARP_COUNT_PARAM]=createParamCentered<Trimpot>(mm2px(Vec(240.274, 68.014)), module, Meander::CONTROL_ARP_COUNT_PARAM);
			dynamic_cast<Knob*>(paramWidgets[Meander::CONTROL_ARP_COUNT_PARAM])->snap=true;
			addParam(paramWidgets[Meander::CONTROL_ARP_COUNT_PARAM]);
			
			paramWidgets[Meander::BUTTON_ENABLE_MELODY_STACCATO_PARAM]=createParamCentered<LEDButton>(mm2px(Vec(240.274, 75)), module, Meander::BUTTON_ENABLE_MELODY_STACCATO_PARAM);
			addParam(paramWidgets[Meander::BUTTON_ENABLE_MELODY_STACCATO_PARAM]);
			lightWidgets[Meander::LIGHT_LEDBUTTON_ENABLE_MELODY_STACCATO_PARAM]=createLightCentered<MediumSimpleLight<RedLight>>(mm2px(Vec(240.274, 75)), module, Meander::LIGHT_LEDBUTTON_ENABLE_MELODY_STACCATO_PARAM);
			addChild(lightWidgets[Meander::LIGHT_LEDBUTTON_ENABLE_MELODY_STACCATO_PARAM]);

			paramWidgets[Meander::CONTROL_ARP_INCREMENT_PARAM]=createParamCentered<Trimpot>(mm2px(Vec(240.256, 82.807)), module, Meander::CONTROL_ARP_INCREMENT_PARAM);
			dynamic_cast<Knob*>(paramWidgets[Meander::CONTROL_ARP_INCREMENT_PARAM])->snap=true;
			addParam(paramWidgets[Meander::CONTROL_ARP_INCREMENT_PARAM]);
		
			paramWidgets[Meander::CONTROL_ARP_DECAY_PARAM]=createParamCentered<Trimpot>(mm2px(Vec(240.237, 91.691)), module, Meander::CONTROL_ARP_DECAY_PARAM);
			addParam(paramWidgets[Meander::CONTROL_ARP_DECAY_PARAM]);
		
			paramWidgets[Meander::CONTROL_ARP_PATTERN_PARAM]=createParamCentered<Trimpot>(mm2px(Vec(240.757, 99.497)), module, Meander::CONTROL_ARP_PATTERN_PARAM);
			dynamic_cast<Knob*>(paramWidgets[Meander::CONTROL_ARP_PATTERN_PARAM])->snap=true;
			addParam(paramWidgets[Meander::CONTROL_ARP_PATTERN_PARAM]);

			//*************Bass********************
		
			paramWidgets[Meander::BUTTON_ENABLE_BASS_PARAM]=createParamCentered<LEDButton>(mm2px(Vec(305, 10.378)), module, Meander::BUTTON_ENABLE_BASS_PARAM);
			addParam(paramWidgets[Meander::BUTTON_ENABLE_BASS_PARAM]);
			lightWidgets[Meander::LIGHT_LEDBUTTON_BASS_ENABLE]=createLightCentered<MediumSimpleLight<RedLight>>(mm2px(Vec(305, 10.378)), module, Meander::LIGHT_LEDBUTTON_BASS_ENABLE);
			addChild(lightWidgets[Meander::LIGHT_LEDBUTTON_BASS_ENABLE]);
			
		    paramWidgets[Meander::CONTROL_BASS_VOLUME_PARAM]=createParamCentered<Trimpot>(mm2px(Vec(305, 21.217)), module, Meander::CONTROL_BASS_VOLUME_PARAM);
			addParam(paramWidgets[Meander::CONTROL_BASS_VOLUME_PARAM]);
		
			paramWidgets[Meander::CONTROL_BASS_TARGETOCTAVE_PARAM]=createParamCentered<Trimpot>(mm2px(Vec(305,  29.217)), module, Meander::CONTROL_BASS_TARGETOCTAVE_PARAM);
			dynamic_cast<Knob*>(paramWidgets[Meander::CONTROL_BASS_TARGETOCTAVE_PARAM])->snap=true;
			addParam(paramWidgets[Meander::CONTROL_BASS_TARGETOCTAVE_PARAM]);
			
			paramWidgets[Meander::BUTTON_BASS_ACCENT_PARAM]=createParamCentered<LEDButton>(mm2px(Vec(305,  37.217)), module, Meander::BUTTON_BASS_ACCENT_PARAM);
			addParam(paramWidgets[Meander::BUTTON_BASS_ACCENT_PARAM]);
			lightWidgets[Meander::LIGHT_LEDBUTTON_BASS_ACCENT_PARAM]=createLightCentered<MediumSimpleLight<RedLight>>(mm2px(Vec(305,  37.217)), module, Meander::LIGHT_LEDBUTTON_BASS_ACCENT_PARAM);
			addChild(lightWidgets[Meander::LIGHT_LEDBUTTON_BASS_ACCENT_PARAM]);
			
			paramWidgets[Meander::BUTTON_BASS_SYNCOPATE_PARAM]=createParamCentered<LEDButton>(mm2px(Vec(305,  45.217)), module, Meander::BUTTON_BASS_SYNCOPATE_PARAM);
			addParam(paramWidgets[Meander::BUTTON_BASS_SYNCOPATE_PARAM]);
			lightWidgets[Meander::LIGHT_LEDBUTTON_BASS_SYNCOPATE_PARAM]=createLightCentered<MediumSimpleLight<RedLight>>(mm2px(Vec(305,  45.217)), module, Meander::LIGHT_LEDBUTTON_BASS_SYNCOPATE_PARAM);
			addChild(lightWidgets[Meander::LIGHT_LEDBUTTON_BASS_SYNCOPATE_PARAM]);
			
			paramWidgets[Meander::BUTTON_BASS_SHUFFLE_PARAM]=createParamCentered<LEDButton>(mm2px(Vec(305,  53.217)), module, Meander::BUTTON_BASS_SHUFFLE_PARAM);
			addParam(paramWidgets[Meander::BUTTON_BASS_SHUFFLE_PARAM]);
			lightWidgets[Meander::LIGHT_LEDBUTTON_BASS_SHUFFLE_PARAM]=createLightCentered<MediumSimpleLight<RedLight>>(mm2px(Vec(305,  53.217)), module, Meander::LIGHT_LEDBUTTON_BASS_SHUFFLE_PARAM);
			addChild(lightWidgets[Meander::LIGHT_LEDBUTTON_BASS_SHUFFLE_PARAM]);
			
			paramWidgets[Meander::BUTTON_BASS_OCTAVES_PARAM]=createParamCentered<LEDButton>(mm2px(Vec(305,  53.217)), module, Meander::BUTTON_BASS_OCTAVES_PARAM);
			addParam(paramWidgets[Meander::BUTTON_BASS_OCTAVES_PARAM]);
			lightWidgets[Meander::LIGHT_LEDBUTTON_BASS_OCTAVES_PARAM]=createLightCentered<MediumSimpleLight<RedLight>>(mm2px(Vec(305,  74.076)), module, Meander::LIGHT_LEDBUTTON_BASS_OCTAVES_PARAM);
			addChild(lightWidgets[Meander::LIGHT_LEDBUTTON_BASS_OCTAVES_PARAM]);
			
			paramWidgets[Meander::CONTROL_BASS_DIVISOR_PARAM]=createParamCentered<Trimpot>(mm2px(Vec(305, 61.217)), module, Meander::CONTROL_BASS_DIVISOR_PARAM);
			dynamic_cast<Knob*>(paramWidgets[Meander::CONTROL_BASS_DIVISOR_PARAM])->snap=true;
			addParam(paramWidgets[Meander::CONTROL_BASS_DIVISOR_PARAM]);
			
			paramWidgets[Meander::BUTTON_ENABLE_BASS_STACCATO_PARAM]=createParamCentered<LEDButton>(mm2px(Vec(305, 88.859)), module, Meander::BUTTON_ENABLE_BASS_STACCATO_PARAM);
			addParam(paramWidgets[Meander::BUTTON_ENABLE_BASS_STACCATO_PARAM]);
			lightWidgets[Meander::LIGHT_LEDBUTTON_ENABLE_BASS_STACCATO_PARAM]=createLightCentered<MediumSimpleLight<RedLight>>(mm2px(Vec(305, 70)), module, Meander::LIGHT_LEDBUTTON_ENABLE_BASS_STACCATO_PARAM);
			addChild(lightWidgets[Meander::LIGHT_LEDBUTTON_ENABLE_BASS_STACCATO_PARAM]);

	//************************		

			paramWidgets[Meander::BUTTON_ENABLE_KEYBOARD_RENDER_PARAM]=createParamCentered<LEDButton>(mm2px(Vec(880, 12)), module, Meander::BUTTON_ENABLE_KEYBOARD_RENDER_PARAM);
			addParam(paramWidgets[Meander::BUTTON_ENABLE_KEYBOARD_RENDER_PARAM]);
			lightWidgets[Meander::LIGHT_LEDBUTTON_ENABLE_RENDER_KEYBOARD_PARAM]=createLightCentered<MediumSimpleLight<RedLight>>(mm2px(Vec(1000, 10)), module, Meander::LIGHT_LEDBUTTON_ENABLE_RENDER_KEYBOARD_PARAM);
			addChild(lightWidgets[Meander::LIGHT_LEDBUTTON_ENABLE_RENDER_KEYBOARD_PARAM]);

			paramWidgets[Meander::BUTTON_ENABLE_SCORE_RENDER_PARAM]=createParamCentered<LEDButton>(mm2px(Vec(880, 30)), module, Meander::BUTTON_ENABLE_SCORE_RENDER_PARAM);
			addParam(paramWidgets[Meander::BUTTON_ENABLE_SCORE_RENDER_PARAM]);
			lightWidgets[Meander::LIGHT_LEDBUTTON_ENABLE_RENDER_SCORE_PARAM]=createLightCentered<MediumSimpleLight<RedLight>>(mm2px(Vec(1000, 200)), module, Meander::LIGHT_LEDBUTTON_ENABLE_RENDER_SCORE_PARAM);
			addChild(lightWidgets[Meander::LIGHT_LEDBUTTON_ENABLE_RENDER_SCORE_PARAM]);

			//**************fBm************************
						
			paramWidgets[Meander::CONTROL_HARMONY_FBM_OCTAVES_PARAM]=createParamCentered<Trimpot>(mm2px(Vec(358, 19)), module, Meander::CONTROL_HARMONY_FBM_OCTAVES_PARAM);
			dynamic_cast<Knob*>(paramWidgets[Meander::CONTROL_HARMONY_FBM_OCTAVES_PARAM])->snap=true;
			addParam(paramWidgets[Meander::CONTROL_HARMONY_FBM_OCTAVES_PARAM]);
			
			paramWidgets[Meander::CONTROL_HARMONY_FBM_PERIOD_PARAM]=createParamCentered<Trimpot>(mm2px(Vec(358, 25)), module, Meander::CONTROL_HARMONY_FBM_PERIOD_PARAM);
			addParam(paramWidgets[Meander::CONTROL_HARMONY_FBM_PERIOD_PARAM]);
		
			paramWidgets[Meander::CONTROL_MELODY_FBM_OCTAVES_PARAM]=createParamCentered<Trimpot>(mm2px(Vec(358, 34)), module, Meander::CONTROL_MELODY_FBM_OCTAVES_PARAM);
			dynamic_cast<Knob*>(paramWidgets[Meander::CONTROL_MELODY_FBM_OCTAVES_PARAM])->snap=true;
			addParam(paramWidgets[Meander::CONTROL_MELODY_FBM_OCTAVES_PARAM]);
			
			paramWidgets[Meander::CONTROL_MELODY_FBM_PERIOD_PARAM]=createParamCentered<Trimpot>(mm2px(Vec(358, 43)), module, Meander::CONTROL_MELODY_FBM_PERIOD_PARAM);
			addParam(paramWidgets[Meander::CONTROL_MELODY_FBM_PERIOD_PARAM]);
		
			paramWidgets[Meander::CONTROL_ARP_FBM_OCTAVES_PARAM]=createParamCentered<Trimpot>(mm2px(Vec(358, 51)), module, Meander::CONTROL_ARP_FBM_OCTAVES_PARAM);
			dynamic_cast<Knob*>(paramWidgets[Meander::CONTROL_ARP_FBM_OCTAVES_PARAM])->snap=true;
			addParam(paramWidgets[Meander::CONTROL_ARP_FBM_OCTAVES_PARAM]);
			
			paramWidgets[Meander::CONTROL_ARP_FBM_PERIOD_PARAM]=createParamCentered<Trimpot>(mm2px(Vec(358, 59)), module, Meander::CONTROL_ARP_FBM_PERIOD_PARAM);
			addParam(paramWidgets[Meander::CONTROL_ARP_FBM_PERIOD_PARAM]);

			// Progression control

			paramWidgets[Meander::BUTTON_PROG_STEP_PARAM]=createParamCentered<LEDButton>(mm2px(Vec(350, 250)), module, Meander::BUTTON_PROG_STEP_PARAM);
			addParam(paramWidgets[Meander::BUTTON_PROG_STEP_PARAM]);
			lightWidgets[Meander::LIGHT_LEDBUTTON_PROG_STEP_PARAM]=createLightCentered<MediumSimpleLight<RedLight>>(mm2px(Vec(350, 250)), module, Meander::LIGHT_LEDBUTTON_PROG_STEP_PARAM);
			addChild(lightWidgets[Meander::LIGHT_LEDBUTTON_PROG_STEP_PARAM]);
			
					 
	//**************  

	// add input ports
		
			for (int i=0; i<Meander::NUM_INPUTS; ++i)
			{
				if (i==Meander::IN_HARMONY_DESTUTTER_EXT_CV)  // this inport is not used, so set to null
					inPortWidgets[i]=NULL;
				else
				{
					inPortWidgets[i]=createInputCentered<TinyPJ301MPort>(mm2px(Vec(10*i,5)), module, i);  // temporarily place them along the top before they are repositioned above
					addInput(inPortWidgets[i]);
				}
			}

	// add output ports		
			
			outPortWidgets[Meander::OUT_RUN_OUT]=createOutputCentered<PJ301MPort>(mm2px(Vec(30.2, 10.95)), module, Meander::OUT_RUN_OUT);
			addOutput(outPortWidgets[Meander::OUT_RUN_OUT]);

			outPortWidgets[Meander::OUT_RESET_OUT]=createOutputCentered<PJ301MPort>(mm2px(Vec(30.2, 23.35)), module, Meander::OUT_RESET_OUT);
			addOutput(outPortWidgets[Meander::OUT_RESET_OUT]);
					
			outPortWidgets[Meander::OUT_MELODY_VOLUME_OUTPUT]=createOutputCentered<PJ301MPort>(mm2px(Vec(266.982, 107.333)), module, Meander::OUT_MELODY_VOLUME_OUTPUT);
			addOutput(outPortWidgets[Meander::OUT_MELODY_VOLUME_OUTPUT]);

			outPortWidgets[Meander::OUT_HARMONY_VOLUME_OUTPUT]=createOutputCentered<PJ301MPort>(mm2px(Vec(201.789, 107.616)), module, Meander::OUT_HARMONY_VOLUME_OUTPUT);
			addOutput(outPortWidgets[Meander::OUT_HARMONY_VOLUME_OUTPUT]);

			outPortWidgets[Meander::OUT_BASS_VOLUME_OUTPUT]=createOutputCentered<PJ301MPort>(mm2px(Vec(325.12, 107.616)), module, Meander::OUT_BASS_VOLUME_OUTPUT);
			addOutput(outPortWidgets[Meander::OUT_BASS_VOLUME_OUTPUT]);
			
			outPortWidgets[Meander::OUT_MELODY_CV_OUTPUT]=createOutputCentered<PJ301MPort>(mm2px(Vec(266.899, 115.813)), module, Meander::OUT_MELODY_CV_OUTPUT);
			addOutput(outPortWidgets[Meander::OUT_MELODY_CV_OUTPUT]);
			
			outPortWidgets[Meander::OUT_BASS_CV_OUTPUT]=createOutputCentered<PJ301MPort>(mm2px(Vec(325.266, 115.817)), module, Meander::OUT_BASS_CV_OUTPUT);
			addOutput(outPortWidgets[Meander::OUT_BASS_CV_OUTPUT]);

			outPortWidgets[Meander::OUT_HARMONY_CV_OUTPUT]=createOutputCentered<PJ301MPort>(mm2px(Vec(202.176, 115.909)), module, Meander::OUT_HARMONY_CV_OUTPUT);
			addOutput(outPortWidgets[Meander::OUT_HARMONY_CV_OUTPUT]);

			outPortWidgets[Meander::OUT_CLOCK_BEATX2_OUTPUT]=createOutputCentered<PJ301MPort>(mm2px(Vec(78.74, 122.291)), module, Meander::OUT_CLOCK_BEATX2_OUTPUT);
			addOutput(outPortWidgets[Meander::OUT_CLOCK_BEATX2_OUTPUT]);

			outPortWidgets[Meander::OUT_CLOCK_BAR_OUTPUT]=createOutputCentered<PJ301MPort>(mm2px(Vec(37.143, 122.537)), module, Meander::OUT_CLOCK_BAR_OUTPUT);
			addOutput(outPortWidgets[Meander::OUT_CLOCK_BAR_OUTPUT]);

			outPortWidgets[Meander::OUT_CLOCK_BEATX4_OUTPUT]=createOutputCentered<PJ301MPort>(mm2px(Vec(99.342, 122.573)), module, Meander::OUT_CLOCK_BEATX4_OUTPUT);
			addOutput(outPortWidgets[Meander::OUT_CLOCK_BEATX4_OUTPUT]);

			outPortWidgets[Meander::OUT_CLOCK_BEATX8_OUTPUT]=createOutputCentered<PJ301MPort>(mm2px(Vec(121.073, 122.573)), module, Meander::OUT_CLOCK_BEATX8_OUTPUT);
			addOutput(outPortWidgets[Meander::OUT_CLOCK_BEATX8_OUTPUT]);

			outPortWidgets[Meander::OUT_CLOCK_BEAT_OUTPUT]=createOutputCentered<PJ301MPort>(mm2px(Vec(57.856, 122.856)), module, Meander::OUT_CLOCK_BEAT_OUTPUT);
			addOutput(outPortWidgets[Meander::OUT_CLOCK_BEAT_OUTPUT]);

			outPortWidgets[Meander::OUT_BASS_GATE_OUTPUT]=createOutputCentered<PJ301MPort>(mm2px(Vec(325.402, 123.984)), module, Meander::OUT_BASS_GATE_OUTPUT);
			addOutput(outPortWidgets[Meander::OUT_BASS_GATE_OUTPUT]);

			outPortWidgets[Meander::OUT_HARMONY_GATE_OUTPUT]=createOutputCentered<PJ301MPort>(mm2px(Vec(202.071, 124.267)), module, Meander::OUT_HARMONY_GATE_OUTPUT);
			addOutput(outPortWidgets[Meander::OUT_HARMONY_GATE_OUTPUT]);

			outPortWidgets[Meander::OUT_MELODY_GATE_OUTPUT]=createOutputCentered<PJ301MPort>(mm2px(Vec(266.982, 124.549)), module, Meander::OUT_MELODY_GATE_OUTPUT);
			addOutput(outPortWidgets[Meander::OUT_MELODY_GATE_OUTPUT]);

			outPortWidgets[Meander::OUT_FBM_HARMONY_OUTPUT]=createOutputCentered<PJ301MPort>(mm2px(Vec(380.0, 107.616)), module, Meander::OUT_FBM_HARMONY_OUTPUT);
			addOutput(outPortWidgets[Meander::OUT_FBM_HARMONY_OUTPUT]);

			outPortWidgets[Meander::OUT_FBM_MELODY_OUTPUT]=createOutputCentered<PJ301MPort>(mm2px(Vec(380.0, 115.815)), module, Meander::OUT_FBM_MELODY_OUTPUT);
			addOutput(outPortWidgets[Meander::OUT_FBM_MELODY_OUTPUT]);

			outPortWidgets[Meander::OUT_FBM_ARP_OUTPUT]=createOutputCentered<PJ301MPort>(mm2px(Vec(380.0, 124.831)), module, Meander::OUT_FBM_ARP_OUTPUT);
			addOutput(outPortWidgets[Meander::OUT_FBM_ARP_OUTPUT]);

			outPortWidgets[Meander::OUT_EXT_POLY_SCALE_OUTPUT]=createOutputCentered<PJ301MPort>(mm2px(Vec(380.0, 124.831)), module, Meander::OUT_EXT_POLY_SCALE_OUTPUT);
			addOutput(outPortWidgets[Meander::OUT_EXT_POLY_SCALE_OUTPUT]);

			outPortWidgets[Meander::OUT_CLOCK_OUT]=createOutputCentered<PJ301MPort>(mm2px(Vec(45.0, 350.0)), module, Meander::OUT_CLOCK_OUT);
			addOutput(outPortWidgets[Meander::OUT_CLOCK_OUT]);

			outPortWidgets[Meander::OUT_EXT_POLY_QUANT_OUTPUT]=createOutputCentered<PJ301MPort>(mm2px(Vec(70.0, 345.0)), module, Meander::OUT_EXT_POLY_QUANT_OUTPUT);
			addOutput(outPortWidgets[Meander::OUT_EXT_POLY_QUANT_OUTPUT]);

			outPortWidgets[Meander::OUT_EXT_ROOT_OUTPUT]=createOutputCentered<PJ301MPort>(mm2px(Vec(37.0, 325.0)), module, Meander::OUT_EXT_ROOT_OUTPUT);
			addOutput(outPortWidgets[Meander::OUT_EXT_ROOT_OUTPUT]);

									
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
				controlPosition=controlPosition.minus((paramWidgets[Meander::BUTTON_CIRCLESTEP_C_PARAM+i]->box.size).div(2.));  // adjust for box size
				paramWidgets[Meander::BUTTON_CIRCLESTEP_C_PARAM+i]->box.pos=controlPosition;
				
				controlPosition=CircleCenter.plus(radialDirection.mult(OuterCircleRadius*.78f));
				controlPosition=controlPosition.minus((lightWidgets[Meander::LIGHT_LEDBUTTON_CIRCLESTEP_1+i]->box.size).div(2.));  // adjust for box size
				lightWidgets[Meander::LIGHT_LEDBUTTON_CIRCLESTEP_1+i]->box.pos=controlPosition;
				

				controlPosition=CircleCenter.plus(radialDirection.mult(OuterCircleRadius*.61f));
				controlPosition=controlPosition.minus((lightWidgets[Meander::LIGHT_CIRCLE_ROOT_KEY_POSITION_1_LIGHT+i]->box.size).div(2.));
				lightWidgets[Meander::LIGHT_CIRCLE_ROOT_KEY_POSITION_1_LIGHT+i]->box.pos=controlPosition;
			}
 
		
			// re-layout the circle step set buttons and lights programatically
			Vec start_position=Vec(0,0);
			float verticalIncrement=mm2px(5.92f);
			for(int i = 0; i<MAX_STEPS;++i) 
			{
				start_position=mm2px(Vec(62, 128.9- 109.27));  // for Y subtract SVG Y from panel height 128.9 and then convert to px
				Vec buttonPosition=start_position.plus(Vec(0, (i*verticalIncrement)));
				buttonPosition.y -= paramWidgets[Meander::BUTTON_HARMONY_SETSTEP_1_PARAM+i]->box.size.y;  // adjust for box height
				paramWidgets[Meander::BUTTON_HARMONY_SETSTEP_1_PARAM+i]->box.pos=buttonPosition;
				start_position=mm2px(Vec(63.5, 128.9- 110.8));  // for Y subtract SVG Y from panel height 128.9 and then convert to px
				buttonPosition=start_position.plus(Vec(0, (i*verticalIncrement)));
				buttonPosition.y -= lightWidgets[Meander::LIGHT_LEDBUTTON_CIRCLESETSTEP_1+i]->box.size.y; // adjust for box height
				lightWidgets[Meander::LIGHT_LEDBUTTON_CIRCLESETSTEP_1+i]->box.pos=buttonPosition;
			}


			// relayout all param controls and lights
		
			Vec drawCenter=Vec(5., 30.);
			
			// do upper left controls and ports
			drawCenter=drawCenter.plus(Vec(37,0));
			paramWidgets[Meander::BUTTON_RUN_PARAM]->box.pos=drawCenter.minus(paramWidgets[Meander::BUTTON_RUN_PARAM]->box.size.div(2.));
			lightWidgets[Meander::LIGHT_LEDBUTTON_RUN]->box.pos=drawCenter.minus(lightWidgets[Meander::LIGHT_LEDBUTTON_RUN]->box.size.div(2.));
			drawCenter=drawCenter.plus(Vec(37,0));
			outPortWidgets[Meander::OUT_RUN_OUT]->box.pos=drawCenter.minus(outPortWidgets[Meander::OUT_RUN_OUT]->box.size.div(2.));
			
			drawCenter=drawCenter.minus(Vec(37,0)); 
			drawCenter=drawCenter.plus(Vec(0,40));
	
			paramWidgets[Meander::BUTTON_RESET_PARAM]->box.pos=drawCenter.minus(paramWidgets[Meander::BUTTON_RESET_PARAM]->box.size.div(2.));
			lightWidgets[Meander::LIGHT_LEDBUTTON_RESET]->box.pos=drawCenter.minus(lightWidgets[Meander::LIGHT_LEDBUTTON_RESET]->box.size.div(2.));
			drawCenter=drawCenter.plus(Vec(37,0));
			outPortWidgets[Meander::OUT_RESET_OUT]->box.pos=drawCenter.minus(outPortWidgets[Meander::OUT_RESET_OUT]->box.size.div(2.));

			drawCenter=Vec(42., 110.);
			
			paramWidgets[Meander::CONTROL_TEMPOBPM_PARAM]->box.pos=drawCenter.minus(paramWidgets[Meander::CONTROL_TEMPOBPM_PARAM]->box.size.div(2.));
				
			drawCenter=drawCenter.plus(Vec(0,25));
			
			paramWidgets[Meander::CONTROL_TIMESIGNATURETOP_PARAM]->box.pos=drawCenter.minus(paramWidgets[Meander::CONTROL_TIMESIGNATURETOP_PARAM]->box.size.div(2.));
			drawCenter=drawCenter.plus(Vec(0,25));
			paramWidgets[Meander::CONTROL_TIMESIGNATUREBOTTOM_PARAM]->box.pos=drawCenter.minus(paramWidgets[Meander::CONTROL_TIMESIGNATUREBOTTOM_PARAM]->box.size.div(2.));

			drawCenter=drawCenter.plus(Vec(0,25));
			paramWidgets[Meander::CONTROL_ROOT_KEY_PARAM]->box.pos=drawCenter.minus(paramWidgets[Meander::CONTROL_ROOT_KEY_PARAM]->box.size.div(2.));


			drawCenter=drawCenter.plus(Vec(0,25));
			paramWidgets[Meander::CONTROL_SCALE_PARAM]->box.pos=drawCenter.minus(paramWidgets[Meander::CONTROL_SCALE_PARAM]->box.size.div(2.));
		
			drawCenter=Vec(880,12);
			paramWidgets[Meander::BUTTON_ENABLE_KEYBOARD_RENDER_PARAM]->box.pos=drawCenter.minus(paramWidgets[Meander::BUTTON_ENABLE_KEYBOARD_RENDER_PARAM]->box.size.div(2.));
			lightWidgets[Meander::LIGHT_LEDBUTTON_ENABLE_RENDER_KEYBOARD_PARAM]->box.pos=drawCenter.minus(lightWidgets[Meander::LIGHT_LEDBUTTON_ENABLE_RENDER_KEYBOARD_PARAM]->box.size.div(2.));

			drawCenter=Vec(880,30);
			paramWidgets[Meander::BUTTON_ENABLE_SCORE_RENDER_PARAM]->box.pos=drawCenter.minus(paramWidgets[Meander::BUTTON_ENABLE_SCORE_RENDER_PARAM]->box.size.div(2.));
			lightWidgets[Meander::LIGHT_LEDBUTTON_ENABLE_RENDER_SCORE_PARAM]->box.pos=drawCenter.minus(lightWidgets[Meander::LIGHT_LEDBUTTON_ENABLE_RENDER_SCORE_PARAM]->box.size.div(2.));

											
			// redo all harmony controls positions
			drawCenter=Vec(512., 40.);
	
			paramWidgets[Meander::BUTTON_ENABLE_HARMONY_PARAM]->box.pos=drawCenter.minus(paramWidgets[Meander::BUTTON_ENABLE_HARMONY_PARAM]->box.size.div(2.));
			lightWidgets[Meander::LIGHT_LEDBUTTON_HARMONY_ENABLE]->box.pos=drawCenter.minus(lightWidgets[Meander::LIGHT_LEDBUTTON_HARMONY_ENABLE]->box.size.div(2.));
			drawCenter=drawCenter.plus(Vec(0,22));	
			paramWidgets[Meander::CONTROL_HARMONY_VOLUME_PARAM]->box.pos=drawCenter.minus(paramWidgets[Meander::CONTROL_HARMONY_VOLUME_PARAM]->box.size.div(2.));
			drawCenter=drawCenter.plus(Vec(-20,22));
			inPortWidgets[Meander::IN_HARMONY_STEPS_EXT_CV]->box.pos=drawCenter.minus(inPortWidgets[Meander::IN_HARMONY_STEPS_EXT_CV]->box.size.div(2.));	
			drawCenter=drawCenter.plus(Vec(20,0));
			paramWidgets[Meander::CONTROL_HARMONY_STEPS_PARAM]->box.pos=drawCenter.minus(paramWidgets[Meander::CONTROL_HARMONY_STEPS_PARAM]->box.size.div(2.));
			drawCenter=drawCenter.plus(Vec(0,22));		
			paramWidgets[Meander::CONTROL_HARMONY_TARGETOCTAVE_PARAM]->box.pos=drawCenter.minus(paramWidgets[Meander::CONTROL_HARMONY_TARGETOCTAVE_PARAM]->box.size.div(2.));
			drawCenter=drawCenter.plus(Vec(0,22));			
			paramWidgets[Meander::CONTROL_HARMONY_ALPHA_PARAM]->box.pos=drawCenter.minus(paramWidgets[Meander::CONTROL_HARMONY_ALPHA_PARAM]->box.size.div(2.));
			drawCenter=drawCenter.plus(Vec(0,22));
			paramWidgets[Meander::CONTROL_HARMONY_RANGE_PARAM]->box.pos=drawCenter.minus(paramWidgets[Meander::CONTROL_HARMONY_RANGE_PARAM]->box.size.div(2.));
			drawCenter=drawCenter.plus(Vec(0,22));			  
			paramWidgets[Meander::CONTROL_HARMONY_DIVISOR_PARAM]->box.pos=drawCenter.minus(paramWidgets[Meander::CONTROL_HARMONY_DIVISOR_PARAM]->box.size.div(2.));
			drawCenter=drawCenter.plus(Vec(0,22));			
			paramWidgets[Meander::BUTTON_ENABLE_HARMONY_ALL7THS_PARAM]->box.pos=drawCenter.minus(paramWidgets[Meander::BUTTON_ENABLE_HARMONY_ALL7THS_PARAM]->box.size.div(2.));
			lightWidgets[Meander::LIGHT_LEDBUTTON_ENABLE_HARMONY_ALL7THS_PARAM]->box.pos=drawCenter.minus(lightWidgets[Meander::LIGHT_LEDBUTTON_ENABLE_HARMONY_ALL7THS_PARAM]->box.size.div(2.));
			drawCenter=drawCenter.plus(Vec(85,0));
			paramWidgets[Meander::BUTTON_ENABLE_HARMONY_V7THS_PARAM]->box.pos=drawCenter.minus(paramWidgets[Meander::BUTTON_ENABLE_HARMONY_V7THS_PARAM]->box.size.div(2.));
			lightWidgets[Meander::LIGHT_LEDBUTTON_ENABLE_HARMONY_V7THS_PARAM]->box.pos=drawCenter.minus(lightWidgets[Meander::LIGHT_LEDBUTTON_ENABLE_HARMONY_V7THS_PARAM]->box.size.div(2.));
			drawCenter=drawCenter.plus(Vec(-85,22));			
			paramWidgets[Meander::BUTTON_ENABLE_HARMONY_STACCATO_PARAM]->box.pos=drawCenter.minus(paramWidgets[Meander::BUTTON_ENABLE_HARMONY_STACCATO_PARAM]->box.size.div(2.));
			lightWidgets[Meander::LIGHT_LEDBUTTON_ENABLE_HARMONY_STACCATO_PARAM]->box.pos=drawCenter.minus(lightWidgets[Meander::LIGHT_LEDBUTTON_ENABLE_HARMONY_STACCATO_PARAM]->box.size.div(2.));
			drawCenter=drawCenter.plus(Vec(0,22));	
			paramWidgets[Meander::CONTROL_HARMONYPRESETS_PARAM]->box.pos=drawCenter.minus(paramWidgets[Meander::CONTROL_HARMONYPRESETS_PARAM]->box.size.div(2.));

			// redo all melody controls positions

			drawCenter=Vec(710., 40.);

			paramWidgets[Meander::BUTTON_ENABLE_MELODY_PARAM]->box.pos=drawCenter.minus(paramWidgets[Meander::BUTTON_ENABLE_MELODY_PARAM]->box.size.div(2.));
			lightWidgets[Meander::LIGHT_LEDBUTTON_MELODY_ENABLE]->box.pos=drawCenter.minus(lightWidgets[Meander::LIGHT_LEDBUTTON_MELODY_ENABLE]->box.size.div(2.));
			drawCenter=drawCenter.plus(Vec(0,22));
			paramWidgets[Meander::CONTROL_MELODY_VOLUME_PARAM]->box.pos=drawCenter.minus(paramWidgets[Meander::CONTROL_MELODY_VOLUME_PARAM]->box.size.div(2.));
			drawCenter=drawCenter.plus(Vec(0,22));
			paramWidgets[Meander::CONTROL_MELODY_NOTE_LENGTH_DIVISOR_PARAM]->box.pos=drawCenter.minus(paramWidgets[Meander::CONTROL_MELODY_NOTE_LENGTH_DIVISOR_PARAM]->box.size.div(2.));
			drawCenter=drawCenter.plus(Vec(0,22));
			paramWidgets[Meander::CONTROL_MELODY_TARGETOCTAVE_PARAM]->box.pos=drawCenter.minus(paramWidgets[Meander::CONTROL_MELODY_TARGETOCTAVE_PARAM]->box.size.div(2.));
			drawCenter=drawCenter.plus(Vec(0,22));
			paramWidgets[Meander::CONTROL_MELODY_ALPHA_PARAM]->box.pos=drawCenter.minus(paramWidgets[Meander::CONTROL_MELODY_ALPHA_PARAM]->box.size.div(2.));
			drawCenter=drawCenter.plus(Vec(0,22));
			paramWidgets[Meander::CONTROL_MELODY_RANGE_PARAM]->box.pos=drawCenter.minus(paramWidgets[Meander::CONTROL_MELODY_RANGE_PARAM]->box.size.div(2.));
			drawCenter=drawCenter.plus(Vec(0,22));
			paramWidgets[Meander::BUTTON_MELODY_DESTUTTER_PARAM]->box.pos=drawCenter.minus(paramWidgets[Meander::BUTTON_MELODY_DESTUTTER_PARAM]->box.size.div(2.));
			lightWidgets[Meander::LIGHT_LEDBUTTON_MELODY_DESTUTTER]->box.pos=drawCenter.minus(lightWidgets[Meander::LIGHT_LEDBUTTON_MELODY_DESTUTTER]->box.size.div(2.));
			drawCenter=drawCenter.plus(Vec(90,0));
			paramWidgets[Meander::BUTTON_ENABLE_MELODY_STACCATO_PARAM]->box.pos=drawCenter.minus(paramWidgets[Meander::BUTTON_ENABLE_MELODY_STACCATO_PARAM]->box.size.div(2.));
			lightWidgets[Meander::LIGHT_LEDBUTTON_ENABLE_MELODY_STACCATO_PARAM]->box.pos=drawCenter.minus(lightWidgets[Meander::LIGHT_LEDBUTTON_ENABLE_MELODY_STACCATO_PARAM]->box.size.div(2.));
			drawCenter=drawCenter.plus(Vec(-90,22));
			paramWidgets[Meander::BUTTON_ENABLE_MELODY_CHORDAL_PARAM]->box.pos=drawCenter.minus(paramWidgets[Meander::BUTTON_ENABLE_MELODY_CHORDAL_PARAM]->box.size.div(2.));
			lightWidgets[Meander::LIGHT_LEDBUTTON_MELODY_ENABLE_CHORDAL]->box.pos=drawCenter.minus(lightWidgets[Meander::LIGHT_LEDBUTTON_MELODY_ENABLE_CHORDAL]->box.size.div(2.));
			drawCenter=drawCenter.plus(Vec(90,0));
			paramWidgets[Meander::BUTTON_ENABLE_MELODY_SCALER_PARAM]->box.pos=drawCenter.minus(paramWidgets[Meander::BUTTON_ENABLE_MELODY_SCALER_PARAM]->box.size.div(2.));
			lightWidgets[Meander::LIGHT_LEDBUTTON_MELODY_ENABLE_SCALER]->box.pos=drawCenter.minus(lightWidgets[Meander::LIGHT_LEDBUTTON_MELODY_ENABLE_SCALER]->box.size.div(2.));
			drawCenter=drawCenter.plus(Vec(-90,0));

			// arp

			drawCenter=drawCenter.plus(Vec(0,22));
			paramWidgets[Meander::BUTTON_ENABLE_ARP_PARAM]->box.pos=drawCenter.minus(paramWidgets[Meander::BUTTON_ENABLE_ARP_PARAM]->box.size.div(2.));
			lightWidgets[Meander::LIGHT_LEDBUTTON_ARP_ENABLE]->box.pos=drawCenter.minus(lightWidgets[Meander::LIGHT_LEDBUTTON_ARP_ENABLE]->box.size.div(2.));
			drawCenter=drawCenter.plus(Vec(0,22));
			paramWidgets[Meander::CONTROL_ARP_COUNT_PARAM]->box.pos=drawCenter.minus(paramWidgets[Meander::CONTROL_ARP_COUNT_PARAM]->box.size.div(2.));
			drawCenter=drawCenter.plus(Vec(0,22));
			paramWidgets[Meander::CONTROL_ARP_INCREMENT_PARAM]->box.pos=drawCenter.minus(paramWidgets[Meander::CONTROL_ARP_INCREMENT_PARAM]->box.size.div(2.));
			drawCenter=drawCenter.plus(Vec(0,22));
			paramWidgets[Meander::CONTROL_ARP_DECAY_PARAM]->box.pos=drawCenter.minus(paramWidgets[Meander::CONTROL_ARP_DECAY_PARAM]->box.size.div(2.));
			drawCenter=drawCenter.plus(Vec(0,22));
			paramWidgets[Meander::CONTROL_ARP_PATTERN_PARAM]->box.pos=drawCenter.minus(paramWidgets[Meander::CONTROL_ARP_PATTERN_PARAM]->box.size.div(2.));
			drawCenter=drawCenter.plus(Vec(0,22));
			paramWidgets[Meander::BUTTON_ENABLE_ARP_CHORDAL_PARAM]->box.pos=drawCenter.minus(paramWidgets[Meander::BUTTON_ENABLE_ARP_CHORDAL_PARAM]->box.size.div(2.));
			lightWidgets[Meander::LIGHT_LEDBUTTON_ARP_ENABLE_CHORDAL]->box.pos=drawCenter.minus(lightWidgets[Meander::LIGHT_LEDBUTTON_ARP_ENABLE_CHORDAL]->box.size.div(2.));
			drawCenter=drawCenter.plus(Vec(90,0));
			paramWidgets[Meander::BUTTON_ENABLE_ARP_SCALER_PARAM]->box.pos=drawCenter.minus(paramWidgets[Meander::BUTTON_ENABLE_ARP_SCALER_PARAM]->box.size.div(2.));
			lightWidgets[Meander::LIGHT_LEDBUTTON_ARP_ENABLE_SCALER]->box.pos=drawCenter.minus(lightWidgets[Meander::LIGHT_LEDBUTTON_ARP_ENABLE_SCALER]->box.size.div(2.));
							
			drawCenter=Vec(900., 148.859);
		
			paramWidgets[Meander::BUTTON_ENABLE_BASS_PARAM]->box.pos=drawCenter.minus(paramWidgets[Meander::BUTTON_ENABLE_BASS_PARAM]->box.size.div(2.));
			lightWidgets[Meander::LIGHT_LEDBUTTON_BASS_ENABLE]->box.pos=drawCenter.minus(lightWidgets[Meander::LIGHT_LEDBUTTON_BASS_ENABLE]->box.size.div(2.));
			drawCenter=drawCenter.plus(Vec(0,22));
			paramWidgets[Meander::CONTROL_BASS_VOLUME_PARAM]->box.pos=drawCenter.minus(paramWidgets[Meander::CONTROL_BASS_VOLUME_PARAM]->box.size.div(2.));
			drawCenter=drawCenter.plus(Vec(0,22));
			paramWidgets[Meander::CONTROL_BASS_TARGETOCTAVE_PARAM]->box.pos=drawCenter.minus(paramWidgets[Meander::CONTROL_BASS_TARGETOCTAVE_PARAM]->box.size.div(2.));
			drawCenter=drawCenter.plus(Vec(0,22));
			paramWidgets[Meander::CONTROL_BASS_DIVISOR_PARAM]->box.pos=drawCenter.minus(paramWidgets[Meander::CONTROL_BASS_DIVISOR_PARAM]->box.size.div(2.));
			drawCenter=drawCenter.plus(Vec(0,22));
			paramWidgets[Meander::BUTTON_ENABLE_BASS_STACCATO_PARAM]->box.pos=drawCenter.minus(paramWidgets[Meander::BUTTON_ENABLE_BASS_STACCATO_PARAM]->box.size.div(2.));
			lightWidgets[Meander::LIGHT_LEDBUTTON_ENABLE_BASS_STACCATO_PARAM]->box.pos=drawCenter.minus(lightWidgets[Meander::LIGHT_LEDBUTTON_ENABLE_BASS_STACCATO_PARAM]->box.size.div(2.));
			drawCenter=drawCenter.plus(Vec(0,22));
			paramWidgets[Meander::BUTTON_BASS_ACCENT_PARAM]->box.pos=drawCenter.minus(paramWidgets[Meander::BUTTON_BASS_ACCENT_PARAM]->box.size.div(2.));
			lightWidgets[Meander::LIGHT_LEDBUTTON_BASS_ACCENT_PARAM]->box.pos=drawCenter.minus(lightWidgets[Meander::LIGHT_LEDBUTTON_BASS_ACCENT_PARAM]->box.size.div(2.));
			drawCenter=drawCenter.plus(Vec(0,22));
			paramWidgets[Meander::BUTTON_BASS_SYNCOPATE_PARAM]->box.pos=drawCenter.minus(paramWidgets[Meander::BUTTON_BASS_SYNCOPATE_PARAM]->box.size.div(2.));
			lightWidgets[Meander::LIGHT_LEDBUTTON_BASS_SYNCOPATE_PARAM]->box.pos=drawCenter.minus(lightWidgets[Meander::LIGHT_LEDBUTTON_BASS_SYNCOPATE_PARAM]->box.size.div(2.));
			drawCenter=drawCenter.plus(Vec(0,22));
			paramWidgets[Meander::BUTTON_BASS_SHUFFLE_PARAM]->box.pos=drawCenter.minus(paramWidgets[Meander::BUTTON_BASS_SHUFFLE_PARAM]->box.size.div(2.));
			lightWidgets[Meander::LIGHT_LEDBUTTON_BASS_SHUFFLE_PARAM]->box.pos=drawCenter.minus(lightWidgets[Meander::LIGHT_LEDBUTTON_BASS_SHUFFLE_PARAM]->box.size.div(2.));
			drawCenter=drawCenter.plus(Vec(0,22));
			paramWidgets[Meander::BUTTON_BASS_OCTAVES_PARAM]->box.pos=drawCenter.minus(paramWidgets[Meander::BUTTON_BASS_OCTAVES_PARAM]->box.size.div(2.));
			lightWidgets[Meander::LIGHT_LEDBUTTON_BASS_OCTAVES_PARAM]->box.pos=drawCenter.minus(lightWidgets[Meander::LIGHT_LEDBUTTON_BASS_OCTAVES_PARAM]->box.size.div(2.));
			
			// fBm controls

			drawCenter=Vec(1055., 168.859);


			paramWidgets[Meander::CONTROL_HARMONY_FBM_OCTAVES_PARAM]->box.pos=drawCenter.minus(paramWidgets[Meander::CONTROL_HARMONY_FBM_OCTAVES_PARAM]->box.size.div(2.));
			drawCenter=drawCenter.plus(Vec(0,22));
			paramWidgets[Meander::CONTROL_HARMONY_FBM_PERIOD_PARAM]->box.pos=drawCenter.minus(paramWidgets[Meander::CONTROL_HARMONY_FBM_PERIOD_PARAM]->box.size.div(2.));
			drawCenter=drawCenter.plus(Vec(0,33));
			paramWidgets[Meander::CONTROL_MELODY_FBM_OCTAVES_PARAM]->box.pos=drawCenter.minus(paramWidgets[Meander::CONTROL_MELODY_FBM_OCTAVES_PARAM]->box.size.div(2.));
			drawCenter=drawCenter.plus(Vec(0,22));
			paramWidgets[Meander::CONTROL_MELODY_FBM_PERIOD_PARAM]->box.pos=drawCenter.minus(paramWidgets[Meander::CONTROL_MELODY_FBM_PERIOD_PARAM]->box.size.div(2.));
			drawCenter=drawCenter.plus(Vec(0,33));
			paramWidgets[Meander::CONTROL_ARP_FBM_OCTAVES_PARAM]->box.pos=drawCenter.minus(paramWidgets[Meander::CONTROL_ARP_FBM_OCTAVES_PARAM]->box.size.div(2.));
			drawCenter=drawCenter.plus(Vec(0,22));
			paramWidgets[Meander::CONTROL_ARP_FBM_PERIOD_PARAM]->box.pos=drawCenter.minus(paramWidgets[Meander::CONTROL_ARP_FBM_PERIOD_PARAM]->box.size.div(2.));
			drawCenter=drawCenter.plus(Vec(0,22));

		
			drawCenter=Vec(345., 250.);
			paramWidgets[Meander::BUTTON_PROG_STEP_PARAM]->box.pos=drawCenter.minus(paramWidgets[Meander::BUTTON_PROG_STEP_PARAM]->box.size.div(2.));
			lightWidgets[Meander::LIGHT_LEDBUTTON_PROG_STEP_PARAM]->box.pos=drawCenter.minus(lightWidgets[Meander::LIGHT_LEDBUTTON_PROG_STEP_PARAM]->box.size.div(2.));
		
			// re-layout all input ports.  Work around parm and input enum value mismatch due to history
			for (int i=0; i<Meander::NUM_INPUTS; ++i)
			{
				if (i<=Meander::IN_SCALE_EXT_CV)
				{
					if ((inPortWidgets[i]!=NULL)&&(paramWidgets[i]!=NULL))
						inPortWidgets[i]->box.pos= paramWidgets[i]->box.pos.minus(Vec(20,-1));
				}
				else
				if (i==Meander::IN_CLOCK_EXT_CV)
				{
					Vec drawCenter=Vec(20., 355.);  
					inPortWidgets[Meander::IN_CLOCK_EXT_CV]->box.pos=drawCenter.minus(inPortWidgets[Meander::IN_CLOCK_EXT_CV]->box.size.div(2.));
				}
				else
				if (i==Meander::IN_HARMONY_CIRCLE_DEGREE_EXT_CV)
				{
					Vec drawCenter=Vec(345., 210.);
					inPortWidgets[Meander::IN_HARMONY_CIRCLE_DEGREE_EXT_CV]->box.pos=drawCenter.minus(inPortWidgets[Meander::IN_HARMONY_CIRCLE_DEGREE_EXT_CV]->box.size.div(2.));
				}
				else
				if (i==Meander::IN_HARMONY_CIRCLE_DEGREE_GATE_EXT_CV)
				{
					Vec drawCenter=Vec(345., 230.);
					inPortWidgets[Meander::IN_HARMONY_CIRCLE_DEGREE_GATE_EXT_CV]->box.pos=drawCenter.minus(inPortWidgets[Meander::IN_HARMONY_CIRCLE_DEGREE_GATE_EXT_CV]->box.size.div(2.));
				}
				else
				if (i==Meander::IN_MELODY_SCALE_DEGREE_EXT_CV)
				{
					Vec drawCenter=Vec(512.+290, 31.);
					inPortWidgets[Meander::IN_MELODY_SCALE_DEGREE_EXT_CV]->box.pos=drawCenter.minus(inPortWidgets[Meander::IN_MELODY_SCALE_DEGREE_EXT_CV]->box.size.div(2.));
				}
				else
				if (i==Meander::IN_MELODY_SCALE_GATE_EXT_CV)
				{
					Vec drawCenter=Vec(512.+290, 47.);
					inPortWidgets[Meander::IN_MELODY_SCALE_GATE_EXT_CV]->box.pos=drawCenter.minus(inPortWidgets[Meander::IN_MELODY_SCALE_GATE_EXT_CV]->box.size.div(2.));
				}
				else
				if (i==Meander::IN_POLY_QUANT_EXT_CV)
				{
					Vec drawCenter=Vec(20., 295.);
					inPortWidgets[Meander::IN_POLY_QUANT_EXT_CV]->box.pos=drawCenter.minus(inPortWidgets[Meander::IN_POLY_QUANT_EXT_CV]->box.size.div(2.));
				}
				else
				{
					int parmIndex=Meander::BUTTON_ENABLE_MELODY_PARAM+i-Meander::IN_HARMONY_CIRCLE_DEGREE_GATE_EXT_CV-1;
					if ((inPortWidgets[i]!=NULL)&&(paramWidgets[parmIndex]!=NULL))
						inPortWidgets[i]->box.pos= paramWidgets[parmIndex]->box.pos.minus(Vec(20,-1));
				}
				
			}
			

			// re-layout all output port

			drawCenter=Vec(520., 365.);

			outPortWidgets[Meander::OUT_HARMONY_CV_OUTPUT]->box.pos=drawCenter.minus(outPortWidgets[Meander::OUT_HARMONY_CV_OUTPUT]->box.size.div(2.));
			drawCenter=drawCenter.plus(Vec(60,0));
			outPortWidgets[Meander::OUT_HARMONY_GATE_OUTPUT]->box.pos=drawCenter.minus(outPortWidgets[Meander::OUT_HARMONY_GATE_OUTPUT]->box.size.div(2.));
			drawCenter=drawCenter.plus(Vec(60,0));
			outPortWidgets[Meander::OUT_HARMONY_VOLUME_OUTPUT]->box.pos=drawCenter.minus(outPortWidgets[Meander::OUT_HARMONY_VOLUME_OUTPUT]->box.size.div(2.));

			drawCenter=Vec(717., 365.);
			outPortWidgets[Meander::OUT_MELODY_CV_OUTPUT]->box.pos=drawCenter.minus(outPortWidgets[Meander::OUT_MELODY_CV_OUTPUT]->box.size.div(2.));
			drawCenter=drawCenter.plus(Vec(60,0));
			outPortWidgets[Meander::OUT_MELODY_GATE_OUTPUT]->box.pos=drawCenter.minus(outPortWidgets[Meander::OUT_MELODY_GATE_OUTPUT]->box.size.div(2.));
			drawCenter=drawCenter.plus(Vec(60,0));
			outPortWidgets[Meander::OUT_MELODY_VOLUME_OUTPUT]->box.pos=drawCenter.minus(outPortWidgets[Meander::OUT_MELODY_VOLUME_OUTPUT]->box.size.div(2.));

			drawCenter=Vec(895., 365.);
			outPortWidgets[Meander::OUT_BASS_CV_OUTPUT]->box.pos=drawCenter.minus(outPortWidgets[Meander::OUT_BASS_CV_OUTPUT]->box.size.div(2.));
			drawCenter=drawCenter.plus(Vec(50,0));
			outPortWidgets[Meander::OUT_BASS_GATE_OUTPUT]->box.pos=drawCenter.minus(outPortWidgets[Meander::OUT_BASS_GATE_OUTPUT]->box.size.div(2.));
			drawCenter=drawCenter.plus(Vec(50,0));
			outPortWidgets[Meander::OUT_BASS_VOLUME_OUTPUT]->box.pos=drawCenter.minus(outPortWidgets[Meander::OUT_BASS_VOLUME_OUTPUT]->box.size.div(2.));

			drawCenter=Vec(1060., 365.);
			outPortWidgets[Meander::OUT_FBM_HARMONY_OUTPUT]->box.pos=drawCenter.minus(outPortWidgets[Meander::OUT_FBM_HARMONY_OUTPUT]->box.size.div(2.));
			drawCenter=drawCenter.plus(Vec(50,0));
			outPortWidgets[Meander::OUT_FBM_MELODY_OUTPUT]->box.pos=drawCenter.minus(outPortWidgets[Meander::OUT_FBM_MELODY_OUTPUT]->box.size.div(2.));
			drawCenter=drawCenter.plus(Vec(50,0));
			outPortWidgets[Meander::OUT_FBM_ARP_OUTPUT]->box.pos=drawCenter.minus(outPortWidgets[Meander::OUT_FBM_ARP_OUTPUT]->box.size.div(2.));
			
			//

			drawCenter=Vec(100., 365.);
			outPortWidgets[Meander::OUT_CLOCK_BAR_OUTPUT]->box.pos=drawCenter.minus(outPortWidgets[Meander::OUT_CLOCK_BAR_OUTPUT]->box.size.div(2.));
			drawCenter=drawCenter.plus(Vec(40,0));
			outPortWidgets[Meander::OUT_CLOCK_BEAT_OUTPUT]->box.pos=drawCenter.minus(outPortWidgets[Meander::OUT_CLOCK_BEAT_OUTPUT]->box.size.div(2.));
			drawCenter=drawCenter.plus(Vec(40,0));
			outPortWidgets[Meander::OUT_CLOCK_BEATX2_OUTPUT]->box.pos=drawCenter.minus(outPortWidgets[Meander::OUT_CLOCK_BEATX2_OUTPUT]->box.size.div(2.));
			drawCenter=drawCenter.plus(Vec(40,0));
			outPortWidgets[Meander::OUT_CLOCK_BEATX4_OUTPUT]->box.pos=drawCenter.minus(outPortWidgets[Meander::OUT_CLOCK_BEATX4_OUTPUT]->box.size.div(2.));
			drawCenter=drawCenter.plus(Vec(40,0));
			outPortWidgets[Meander::OUT_CLOCK_BEATX8_OUTPUT]->box.pos=drawCenter.minus(outPortWidgets[Meander::OUT_CLOCK_BEATX8_OUTPUT]->box.size.div(2.));
			drawCenter=drawCenter.plus(Vec(40,0));

			drawCenter=Vec(145., 295.);
			outPortWidgets[Meander::OUT_EXT_POLY_SCALE_OUTPUT]->box.pos=drawCenter.minus(outPortWidgets[Meander::OUT_EXT_POLY_SCALE_OUTPUT]->box.size.div(2.));
			drawCenter=drawCenter.plus(Vec(40,0));
				
			drawCenter=Vec(50., 355.);   
			outPortWidgets[Meander::OUT_CLOCK_OUT]->box.pos=drawCenter.minus(outPortWidgets[Meander::OUT_CLOCK_OUT]->box.size.div(2.));
			drawCenter=drawCenter.plus(Vec(40,0));

			drawCenter=Vec(50., 295.);  
			outPortWidgets[Meander::OUT_EXT_POLY_QUANT_OUTPUT]->box.pos=drawCenter.minus(outPortWidgets[Meander::OUT_EXT_POLY_QUANT_OUTPUT]->box.size.div(2.));
			drawCenter=drawCenter.plus(Vec(40,0));


			drawCenter=Vec(95., 193.);  
			outPortWidgets[Meander::OUT_EXT_ROOT_OUTPUT]->box.pos=drawCenter.minus(outPortWidgets[Meander::OUT_EXT_ROOT_OUTPUT]->box.size.div(2.));
			drawCenter=drawCenter.plus(Vec(40,0));

			//********************
			
			for (int i=0; ((i<Meander::NUM_PARAMS)&&(i<MAX_PARAMS)); ++i)  // get the paramWidget box into a MeanderWidget array so it can be accessed as needed
			{
				if (paramWidgets[i]!=NULL) 
					ParameterRect[i]=paramWidgets[i]->box;
			}

			for (int i=0; ((i<Meander::NUM_OUTPUTS)&&(i<MAX_OUTPORTS)); ++i)  // get the paramWidget box into a MeanderWidget array so it can be accessed as needed
			{
				if (outPortWidgets[i]!=NULL) 
					OutportRect[i]=outPortWidgets[i]->box;
			}

			for (int i=0; ((i<Meander::NUM_INPUTS)&&(i<MAX_INPORTS)); ++i)  // get the paramWidget box into a MeanderWidget array so it can be accessed as needed
			{
				if (inPortWidgets[i]!=NULL) 
					InportRect[i]=inPortWidgets[i]->box;
			}
				
		}  // end if (true)  

	}    // end MeanderWidget(Meander* module)  

 
	// create panel theme and contrast control
	
	void appendContextMenu(Menu *menu) override 
	{  
        Meander *module = dynamic_cast<Meander*>(this->module);
		if (module==NULL)
			return;
   
		MenuLabel *panelthemeLabel = new MenuLabel();
        panelthemeLabel->text = "Panel Theme                               ";
        menu->addChild(panelthemeLabel);

		MeanderPanelThemeItem *lightpaneltheme_Item = new MeanderPanelThemeItem();  // this accomodates json loaded value
        lightpaneltheme_Item->text = "  light";
		lightpaneltheme_Item->module = module;
   	    lightpaneltheme_Item->theme = 0;
	    menu->addChild(lightpaneltheme_Item);

		MeanderPanelThemeItem *darkpaneltheme_Item = new MeanderPanelThemeItem();  // this accomodates json loaded value
        darkpaneltheme_Item->text = "  dark";
		darkpaneltheme_Item->module = module;
   	    darkpaneltheme_Item->theme = 1;
        menu->addChild(darkpaneltheme_Item);

		// create contrast control
	
		MinMaxSliderItem *minSliderItem = new MinMaxSliderItem(&panelContrast, "Contrast");
		minSliderItem->box.size.x = 200.f;
		menu->addChild(minSliderItem);
	
		//

		MenuLabel *modeLabel = new MenuLabel();
        modeLabel->text = "Scale Out Mode                               ";
        menu->addChild(modeLabel);
		
		
		MeanderScaleOutModeItem *chromatic_Item = new MeanderScaleOutModeItem();
        chromatic_Item->text = "  Heptatonic Chromatic Scale-12ch";
        chromatic_Item->module = module;
        chromatic_Item->mode = Meander::HEPTATONIC_CHROMATIC_12CH;
        menu->addChild(chromatic_Item);

		MeanderScaleOutModeItem *heptatonic_Item = new MeanderScaleOutModeItem();
        heptatonic_Item->text = "  Heptatonic Diatonic STD-7ch";
        heptatonic_Item->module = module;
        heptatonic_Item->mode = Meander::HEPTATONIC_DIATONIC_STD_7CH;
        menu->addChild(heptatonic_Item);


		MeanderScaleOutModeItem *pentatonic_Item = new MeanderScaleOutModeItem();
        pentatonic_Item->text = "  Pentatonic-5ch";
        pentatonic_Item->module =module;
        pentatonic_Item->mode = Meander::PENTATONIC_5CH;
        menu->addChild(pentatonic_Item); 
		

		MeanderScaleOutModeItem *chromatic_pentatonic_Item = new MeanderScaleOutModeItem();
        chromatic_pentatonic_Item->text = "  Pentatonic Chromatic-12ch";
        chromatic_pentatonic_Item->module = module;
        chromatic_pentatonic_Item->mode = Meander::PENTATONIC_CHROMATIC_12CH;
        menu->addChild(chromatic_pentatonic_Item);

		
   		MenuLabel *modeLabel3 = new MenuLabel();
        modeLabel3->text = "Gate Out Mode                 ";
        menu->addChild(modeLabel3);

				
		MeanderGateOutModeItem *gate_standard_Item = new MeanderGateOutModeItem();
        gate_standard_Item->text = "  Standard 10V";
        gate_standard_Item->module = module;
        gate_standard_Item->mode = Meander::STANDARD_GATE;
        menu->addChild(gate_standard_Item);

		MeanderGateOutModeItem *gate_volume_Item = new MeanderGateOutModeItem();
        gate_volume_Item->text = "  Volume over gate 2.1-10V";
        gate_volume_Item->module = module;
        gate_volume_Item->mode = Meander::VOLUME_OVER_GATE;
        menu->addChild(gate_volume_Item);

		// experimental code inside appendContextMenu for a text field harmonic progression
		if (false)
		{
			if (module->harmony_type==4)  // custom
			{
				MenuLabel *modeLabel3 = new MenuLabel();
				modeLabel3->text = "Harmonic progression";
				menu->addChild(modeLabel3);

				auto holder = new rack::Widget;
				holder->box.size.x = 201;
				holder->box.size.y = 20;
				auto lab = new rack::Label;
				lab->text = "Foo : ";
				lab->box.size = 49;
				holder->addChild(lab);
				auto textfield = new rack::TextField;
				textfield->box.pos.x = 50;
				textfield->box.size.x = 200;
				holder->addChild(textfield);
				menu->addChild(holder);
			}
		}
	}  // end MeanderWidget() 

	void step() override   // note, this is a widget step() which is not deprecated and is a GUI call.  This advances UI by one "frame"    
	{  
		Meander *module = dynamic_cast<Meander*>(this->module);  

		if (true) // needs to happen even if module==null
		{
			if (svgPanel)
			    svgPanel->setVisible((panelTheme) == 0);    
			if (darkPanel)                             
				darkPanel->setVisible((panelTheme) == 1);    
		
			float contrast=panelContrast;

			// update the global panel theme vars
			if (panelTheme==0)  // light theme
			{
				panelcolor=nvgRGBA((unsigned char)230,(unsigned char)230,(unsigned char)230,(unsigned char)255);
				float color =255*(1-contrast);
				panelTextColor=nvgRGBA((unsigned char)color,(unsigned char)color,(unsigned char)color,(unsigned char)255);  // black text
				panelLineColor=nvgRGBA((unsigned char)color,(unsigned char)color,(unsigned char)color,(unsigned char)255);  // black lines
				color =255*contrast;
				paramTextColor=nvgRGBA((unsigned char)color,(unsigned char)color,(unsigned char)0,(unsigned char)255);  // yellow text
			
				{
					float r=panelHarmonyPartBaseColor.r; 
					float g=(1-contrast);
					float b=(1-contrast);
					panelHarmonyPartColor=nvgRGBA(r*156, g*255, b*255, 255);
				}
				{
					float r=(1-contrast);
					float g=(1-contrast);
					float b=panelArpPartBaseColor.b;
					panelArpPartColor=nvgRGBA(r*255, g*255, b*255, 255);
				}
				{
					float r=(1-contrast);
					float g=panelBassPartBaseColor.g;
					float b=(1-contrast);
					panelBassPartColor=nvgRGBA(r*255, g*128, b*255, 255);
				}
				{
					float r=(1-contrast);
					float g=(1-contrast);
					float b=(1-contrast);
					panelMelodyPartColor=nvgRGBA(r*255, g*255, b*255, 255);
				}
						
			}
			else  // dark theme
			{
				panelcolor=nvgRGBA((unsigned char)40,(unsigned char)40,(unsigned char)40,(unsigned char)255);
				float color = 255*contrast;
				panelTextColor=nvgRGBA((unsigned char)color,(unsigned char)color,(unsigned char)color,(unsigned char)255);  // white text
				panelLineColor=nvgRGBA((unsigned char)color,(unsigned char)color,(unsigned char)color,(unsigned char)255);  // white lines
				color =255*contrast;
				paramTextColor=nvgRGBA((unsigned char)color,(unsigned char)color,(unsigned char)0,(unsigned char)255);  // yellow text

				{
					float r=panelHarmonyPartBaseColor.r*contrast;
					float g=0.45;
					float b=0.45;
					panelHarmonyPartColor=nvgRGBA(r*255, g*255, b*255, 255);
				}
				{
					float b=panelArpPartBaseColor.b*contrast;
					float r=0.45;
					float g=0.45;
					panelArpPartColor=nvgRGBA(r*255, g*255, b*255, 255);
				}
				{
					float g=panelBassPartBaseColor.g*contrast;
					float r=0.45;
					float b=0.45;
					panelBassPartColor=nvgRGBA(r*255, g*255, b*255, 255);
				}
				{
					float r=(contrast);
					float g=(contrast);
					float b=(contrast);
					panelMelodyPartColor=nvgRGBA(r*228, g*228, b*228, 255);
				}
			}
		}

		// determine STEP cable connections to Meander trigger outs, if any
	  	if (module != NULL)  // not in the browser
		{  
			module->onResetScale();  // make sure channels and scale notes outports are initialized for each frame, in case they have not been iniitialized
			
			module->theMeanderState.theHarmonyParms.STEP_inport_connected_to_Meander_trigger_port=0;  // 0 will be interpreted elsewhere as "no connection", which may be overwritten below
	
			for (CableWidget* cwIn : APP->scene->rack->getCablesOnPort(inPortWidgets[Meander::IN_PROG_STEP_EXT_CV]))
			{	
				if (!cwIn->isComplete())        // new for testing, from    vcvrack-packone strip.cpp                                              
                   continue;

				module->theMeanderState.theHarmonyParms.STEP_inport_connected_to_Meander_trigger_port=1;  // 1 will be interpreted elsewhere as an unknown complete connection, which may be overwritten below
				
				for (CableWidget* cwOut : APP->scene->rack->getCablesOnPort(outPortWidgets[Meander::OUT_CLOCK_BAR_OUTPUT]))
				{
					if (!cwOut->isComplete())        // new for testing, from    vcvrack-packone strip.cpp                                              
                   		continue;
					if (cwOut->cable->id == cwIn->cable->id)
					{
						module->theMeanderState.theHarmonyParms.STEP_inport_connected_to_Meander_trigger_port=Meander::OUT_CLOCK_BAR_OUTPUT;  
					}
										
				}
				for (CableWidget* cwOut : APP->scene->rack->getCablesOnPort(outPortWidgets[Meander::OUT_CLOCK_BEAT_OUTPUT]))
				{
					if (!cwOut->isComplete())        // new for testing, from    vcvrack-packone strip.cpp                                              
                   		continue;
					if (cwOut->cable->id == cwIn->cable->id)
					{
						module->theMeanderState.theHarmonyParms.STEP_inport_connected_to_Meander_trigger_port=Meander::OUT_CLOCK_BEAT_OUTPUT;
					}
					
				}
				for (CableWidget* cwOut : APP->scene->rack->getCablesOnPort(outPortWidgets[Meander::OUT_CLOCK_BEATX2_OUTPUT]))
				{
					if (!cwOut->isComplete())        // new for testing, from    vcvrack-packone strip.cpp                                              
                   		continue;
					if (cwOut->cable->id == cwIn->cable->id)
					{
						module->theMeanderState.theHarmonyParms.STEP_inport_connected_to_Meander_trigger_port=Meander::OUT_CLOCK_BEATX2_OUTPUT;
					}
					
				}
				for (CableWidget* cwOut : APP->scene->rack->getCablesOnPort(outPortWidgets[Meander::OUT_CLOCK_BEATX4_OUTPUT]))
				{
					if (!cwOut->isComplete())        // new for testing, from    vcvrack-packone strip.cpp                                              
                   		continue;
					if (cwOut->cable->id == cwIn->cable->id)
					{
						module->theMeanderState.theHarmonyParms.STEP_inport_connected_to_Meander_trigger_port=Meander::OUT_CLOCK_BEATX4_OUTPUT;
					}
					
				}
				for (CableWidget* cwOut : APP->scene->rack->getCablesOnPort(outPortWidgets[Meander::OUT_CLOCK_BEATX8_OUTPUT]))
				{
					if (!cwOut->isComplete())        // new for testing, from    vcvrack-packone strip.cpp                                              
                   		continue;
					if (cwOut->cable->id == cwIn->cable->id)
					{
						module->theMeanderState.theHarmonyParms.STEP_inport_connected_to_Meander_trigger_port=Meander::OUT_CLOCK_BEATX8_OUTPUT;  
					}
				}
			}
		}

		// if in the browser, force a panel redraw per frame with the current panel theme
		if (!module) {
			DirtyEvent eDirty;
			parent->parent->onDirty(eDirty);
		}
		else  // not in the browser
		Widget::step();  // most modules do this rather than ModuleWidget::step()
	
		
	} // end step()

};  // end struct MeanderWidget



Model* modelMeander = createModel<Meander, MeanderWidget>("Meander");


