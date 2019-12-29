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

struct Meander : Module 
{
	bool instanceRunning = false;
	
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
				
		
		NUM_PARAMS
	};

	enum InputIds 
	{

		IN_RUN_EXT,
		IN_RESET_EXT,
		IN_CLOCK_EXT,
		IN_TEMPO_EXT,
		IN_HARMONY_CV_EXT,
		IN_HARMONY_GATE_EXT,
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
		OUT_FBM_GATE_OUTPUT,
		OUT_MELODY_CV_OUTPUT,
		OUT_FBM_CV_OUTPUT,
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
		OUT_FBM_TRIGGER_OUTPUT,
		OUT_MELODY_VOLUME_OUTPUT,
		OUT_HARMONY_VOLUME_OUTPUT,
		OUT_BASS_VOLUME_OUTPUT,
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
		
		NUM_LIGHTS
	};

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
			float sqr = phase < pw ? 1.0f : -1.0f;
			return sqr;
		}
	};  // struct LFOGenerator 

	void userPlaysCirclePosition(int circle_position, float octaveOffset)  // C=0
	{
		DEBUG("userPlaysCirclePosition(%d)", circle_position);
		outputs[OUT_HARMONY_CV_OUTPUT].setChannels(3);  // set polyphony
		
		
		DEBUG("circle_position=%d", circle_position);
	
		theMeanderState.last_harmony_chord_root_note=circle_of_fifths[circle_position];

		for (int i=0; i<7; ++i) // melody and bass will use this to accompany if we could set it properly here
		{
			if  (theCircleOf5ths.theDegreeSemiCircle.degreeElements[i].CircleIndex==circle_position)
			{
				int theDegree=theCircleOf5ths.theDegreeSemiCircle.degreeElements[i].Degree;
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
 
		int current_chord_note=0;
		int root_key_note=circle_of_fifths[circle_position]; 
		DEBUG("root_key_note=%d %s", root_key_note, note_desig[root_key_note%12]); 
		int circle_chord_type= theCircleOf5ths.Circle5ths[circle_position].chordType;
		theMeanderState.theHarmonyParms.last_chord_type=circle_chord_type;
		DEBUG("circle_chord_type=%d", circle_chord_type);
		int num_chord_members=chord_type_num_notes[circle_chord_type];
		DEBUG("num_chord_members=%d", num_chord_members);
		for (int j=0;j<num_chord_members;++j) 
		{
			current_chord_note=(int)((int)root_key_note+(int)chord_type_intervals[circle_chord_type][j]);
			DEBUG("  current_chord_note=%d %s", current_chord_note, note_desig[current_chord_note%12]);
			int note_to_play=current_chord_note+(octaveOffset*12);
			// don't play the notes immediately but rather wait and let doHarmony do it at the appropriate time
			if (!running)
			{
				outputs[OUT_HARMONY_CV_OUTPUT].setVoltage((note_to_play/12.0)-1.0+octaveOffset,j);  // (note, channel)  sift down 1 ocatve/v
			//	outputs[OUT_HARMONY_VOLUME_OUTPUT].setVoltage(theMeanderState.theHarmonyParms.volume);
			//	outputs[OUT_MELODY_VOLUME_OUTPUT].setVoltage(theMeanderState.theMelodyParms.volume);
			//	outputs[OUT_BASS_VOLUME_OUTPUT].setVoltage(theMeanderState.theBassParms.volume);
				float durationFactor=1.0;
				if (theMeanderState.theHarmonyParms.enable_staccato)
					durationFactor=0.5;
				else
					durationFactor=1.0;
				
				float note_duration=durationFactor*time_sig_top/(frequency*theMeanderState.theHarmonyParms.note_length_divisor);
				harmonyGatePulse.reset();  // kill the pulse in case it is active
				harmonyGatePulse.trigger(note_duration);  
			}
		
			if (j<4)
			{
				theMeanderState.theHarmonyParms.last[j].note=note_to_play;
				theMeanderState.theHarmonyParms.last[j].noteType=NOTE_TYPE_CHORD;
				theMeanderState.theHarmonyParms.last[j].length=1;  // whole  note for now
				theMeanderState.theHarmonyParms.last[j].time32s=barts_count;
				theMeanderState.theHarmonyParms.last[j].countInBar=bar_note_count;
				if (bar_note_count<256)
				played_notes_circular_buffer[bar_note_count++]=theMeanderState. theHarmonyParms.last[j];
			}
            
		}
	}

	void doHarmony()
	{
		DEBUG("doHarmony");
		DEBUG("doHarmony() theActiveHarmonyType.min_steps=%d, theActiveHarmonyType.max_steps=%d", theActiveHarmonyType.min_steps, theActiveHarmonyType.max_steps );

		outputs[OUT_HARMONY_VOLUME_OUTPUT].setVoltage(theMeanderState. theHarmonyParms.volume);
		
		clock_t current_cpu_t= clock();  // cpu clock ticks since program began
		double current_cpu_time_double= (double)(current_cpu_t) / (double)CLOCKS_PER_SEC;
		//	DEBUG("current_cpu_time=%ld", (long)current_cpu_t);
		//	DEBUG("current_cpu_time_double=%.3lf", (double)current_cpu_time_double);

		DEBUG("\nHarmony: barCount=%d Time=%.3lf", bar_count, (double)current_cpu_time_double);
													
		current_melody_note += 1.0/12.0;
		current_melody_note=fmod(current_melody_note, 1.0f);	

		
		if (!theMeanderState.userControllingHarmonyFromCircle)
		{
			for (int i=0; i<MAX_CIRCLE_STATIONS; ++i) {
				CircleStepStates[i] = false;
				lights[LIGHT_LEDBUTTON_CIRCLESTEP_1+i].value=0.0f;
			}

			for (int i=0; i<16; ++i) {
				if (i<theActiveHarmonyType.num_harmony_steps)
					lights[LIGHT_LEDBUTTON_CIRCLESETSTEP_1+i].value=0.25f;
				else
					lights[LIGHT_LEDBUTTON_CIRCLESETSTEP_1+i].value=0.0f;
					
			}
		}
		else  // theMeanderState.userControllingHarmonyFromCircle
		{
			DEBUG("doHarmony() theMeanderState.userControllingHarmonyFromCircle %d", theMeanderState. theHarmonyParms.last[0]);
			outputs[OUT_HARMONY_CV_OUTPUT].setChannels(3);  // set polyphony
			for (int j=0;j<3;++j) 
			{
				int note_to_play=1;
		
				note_to_play=theMeanderState. theHarmonyParms.last[j].note;

				outputs[OUT_HARMONY_CV_OUTPUT].setVoltage((note_to_play/12.0)-1.0,j);  // (note, channel)	
					
			}
		
		    outputs[OUT_HARMONY_VOLUME_OUTPUT].setVoltage(theMeanderState.theHarmonyParms.volume);
			outputs[OUT_MELODY_VOLUME_OUTPUT].setVoltage(theMeanderState.theMelodyParms.volume);
			outputs[OUT_BASS_VOLUME_OUTPUT].setVoltage(theMeanderState.theBassParms.volume);
			
			float durationFactor=1.0;
						
			if (theMeanderState.theHarmonyParms.enable_staccato)
				durationFactor=0.5;
			else
				durationFactor=1.0;
			
			float note_duration=durationFactor*time_sig_top/(frequency*theMeanderState.theHarmonyParms.note_length_divisor);
			harmonyGatePulse.reset();  // kill the pulse in case it is active
			harmonyGatePulse.trigger(note_duration);  
		}
     
		DEBUG("theHarmonyTypes[%d].num_harmony_steps=%d", harmony_type, theActiveHarmonyType.num_harmony_steps);
		int step=(bar_count%theActiveHarmonyType.num_harmony_steps);  // 0-(n-1)
 

		if (randomize_harmony) // this could be used to randomize any progression
		{
			float rnd = rack::random::uniform();
			step = (int)((rnd*theActiveHarmonyType.num_harmony_steps));
			step=step%theActiveHarmonyType.num_harmony_steps;
		}
		else
		if (harmony_type==23) // this could be used to randomize any progression
		{
			float rnd = rack::random::uniform();
			step = (int)((rnd*theActiveHarmonyType.num_harmony_steps));
			step=step%theActiveHarmonyType.num_harmony_steps;
		}
		else
		if ((harmony_type==31)||(harmony_type==42)||(harmony_type==43)||(harmony_type==44)||(harmony_type==45)||(harmony_type==46)||(harmony_type==47)||(harmony_type==48))  // Markov chains
		{
			float rnd = rack::random::uniform();
			DEBUG("rnd=%.2f",rnd);
		    DEBUG("Markov theMeanderState.theHarmonyParms.last_circle_step=%d", theMeanderState.theHarmonyParms.last_circle_step);

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
				DEBUG("Markov Probabilities:");
				for (int i=1; i<8; ++i)  // skip first array index since this is 1 based
				{
					if (harmony_type==31)
					{
						DEBUG("i=%d: p=%.2f b=%.2f t=%.2f", i, MarkovProgressionTransitionMatrixBach1[theMeanderState.theHarmonyParms.last_circle_step+1][i], probabilityTargetBottom[i], probabilityTargetTop[i]);
					}
					else
					if (harmony_type==42)
					{
						DEBUG("i=%d: p=%.2f b=%.2f t=%.2f", i, MarkovProgressionTransitionMatrixBach2[theMeanderState.theHarmonyParms.last_circle_step+1][i], probabilityTargetBottom[i], probabilityTargetTop[i]);
					}
					else
					if (harmony_type==43)
					{
						DEBUG("i=%d: p=%.2f b=%.2f t=%.2f", i, MarkovProgressionTransitionMatrixMozart1[theMeanderState.theHarmonyParms.last_circle_step+1][i], probabilityTargetBottom[i], probabilityTargetTop[i]);
					}
					else
					if (harmony_type==44)
					{
						DEBUG("i=%d: p=%.2f b=%.2f t=%.2f", i, MarkovProgressionTransitionMatrixMozart2[theMeanderState.theHarmonyParms.last_circle_step+1][i], probabilityTargetBottom[i], probabilityTargetTop[i]);
					}
					else
					if (harmony_type==45)
					{
						DEBUG("i=%d: p=%.2f b=%.2f t=%.2f", i, MarkovProgressionTransitionMatrixPalestrina1[theMeanderState.theHarmonyParms.last_circle_step+1][i], probabilityTargetBottom[i], probabilityTargetTop[i]);
					}
					else
					if (harmony_type==46)
					{
						DEBUG("i=%d: p=%.2f b=%.2f t=%.2f", i, MarkovProgressionTransitionMatrixBeethoven1[theMeanderState.theHarmonyParms.last_circle_step+1][i], probabilityTargetBottom[i], probabilityTargetTop[i]);
					}
					else
					if (harmony_type==47)
					{
						DEBUG("i=%d: p=%.2f b=%.2f t=%.2f", i, MarkovProgressionTransitionMatrixTraditional1[theMeanderState.theHarmonyParms.last_circle_step+1][i], probabilityTargetBottom[i], probabilityTargetTop[i]);
					}
					else
					if (harmony_type==48)
					{
						DEBUG("i=%d: p=%.2f b=%.2f t=%.2f", i, MarkovProgressionTransitionMatrix_I_IV_V[theMeanderState.theHarmonyParms.last_circle_step+1][i], probabilityTargetBottom[i], probabilityTargetTop[i]);
					}					

					if ((rnd>probabilityTargetBottom[i])&&(rnd<= probabilityTargetTop[i]))
					{
						step=i-1;
						DEBUG("step=%d", step);
					}
				}
			
			}
		
		}
		
    	

		DEBUG("step=%d", step);

		int degreeStep=(theActiveHarmonyType.harmony_steps[step])%8;  
		DEBUG("degreeStep=%d", degreeStep);
	
		theMeanderState.theHarmonyParms.last_circle_step=step;  // used for Markov chain

		//find this in semicircle
		for (int i=0; i<7; ++i)
		{
			if  (theCircleOf5ths.theDegreeSemiCircle.degreeElements[i].Degree==degreeStep)
			{
				current_circle_position = theCircleOf5ths.theDegreeSemiCircle.degreeElements[i].CircleIndex; 
				break;
			}
			if (i==7)
			{
	    	   DEBUG("  warning circleposition could not be found 2");
			}
		}
		
	//	CircleStepStates[step] = true;  // this was causing problems in prog edit

		if (!theMeanderState.userControllingHarmonyFromCircle) 
		{ 
			lights[LIGHT_LEDBUTTON_CIRCLESETSTEP_1+step].value=1.0f;
			lights[LIGHT_LEDBUTTON_CIRCLESTEP_1+ (current_circle_position)%12].value=1.0f;
		}

		DEBUG("current_circle_position=%d root=%d %s", current_circle_position, circle_of_fifths[current_circle_position], note_desig[circle_of_fifths[current_circle_position]]);		
		DEBUG("theCircleOf5ths.Circle5ths[current_circle_position].chordType=%d", theCircleOf5ths.Circle5ths[current_circle_position].chordType);
		
		
		double period=1.0/theMeanderState.theHarmonyParms.period; // 1/seconds
		double fBmarg=theMeanderState.theHarmonyParms.seed + (double)(period*current_cpu_time_double); 
	    double fBmrand=(FastfBm1DNoise(fBmarg,theMeanderState.theHarmonyParms.noctaves) +1.)/2; 
	//	DEBUG("fBmrand=%f",(float)fBmrand);
		
		theMeanderState.theHarmonyParms.note_avg = 
			(1.0-theMeanderState.theHarmonyParms.alpha)*theMeanderState.theHarmonyParms.note_avg + 
			theMeanderState.theHarmonyParms.alpha*(theMeanderState.theHarmonyParms.range_bottom + (fBmrand*theMeanderState.theHarmonyParms.r1));
					
		if (theMeanderState.theHarmonyParms.note_avg>theMeanderState.theHarmonyParms.range_top)
		theMeanderState.theHarmonyParms.note_avg=theMeanderState.theHarmonyParms.range_top;
		if (theMeanderState.theHarmonyParms.note_avg<theMeanderState.theHarmonyParms.range_bottom)
		theMeanderState.theHarmonyParms.note_avg=theMeanderState.theHarmonyParms.range_bottom;
					
		int step_chord_type= theCircleOf5ths.Circle5ths[current_circle_position].chordType;
		
		if (((theMeanderState.theHarmonyParms.enable_all_7ths)||(theMeanderState.theHarmonyParms.enable_V_7ths))			
		&& ((theCircleOf5ths.Circle5ths[current_circle_position].chordType==3)
		||  (theCircleOf5ths.Circle5ths[current_circle_position].chordType==4)
		||  (theCircleOf5ths.Circle5ths[current_circle_position].chordType==5)))
			outputs[OUT_HARMONY_CV_OUTPUT].setChannels(4);  // set polyphony
		else
			outputs[OUT_HARMONY_CV_OUTPUT].setChannels(3);  // set polyphony
		
		DEBUG("step_chord_type=%d", step_chord_type);
		int num_chord_members=chord_type_num_notes[step_chord_type]; 
		DEBUG("num_chord_members=%d", num_chord_members);
		
		if (!theMeanderState.userControllingHarmonyFromCircle) // otherwise let these be set by usercontrolsharmony..
		{
			theMeanderState.theHarmonyParms.last_chord_type=step_chord_type;
			theMeanderState.last_harmony_chord_root_note=circle_of_fifths[current_circle_position];
			theMeanderState.last_harmony_step=step;
		}

		DEBUG("theMeanderState.last_harmony_chord_root_note=%d %s", theMeanderState.last_harmony_chord_root_note, note_desig[theMeanderState.last_harmony_chord_root_note%MAX_NOTES]);

		DEBUG("1st 3 step_chord_notes=%d %s, %d %s, %d %s", step_chord_notes[step][0], note_desig[step_chord_notes[step][0]%MAX_NOTES], step_chord_notes[step][1], note_desig[step_chord_notes[step][1]%MAX_NOTES], step_chord_notes[step][2], note_desig[step_chord_notes[step][2]%MAX_NOTES]);
			
		for (int j=0;j<num_chord_members;++j) 
		{
				DEBUG("num_step_chord_notes[%d]=%d", step, num_step_chord_notes[step]);
				current_chord_notes[j]= step_chord_notes[step][(int)(theMeanderState. theHarmonyParms.note_avg*num_step_chord_notes[step])+j]; // do not create inversion
				DEBUG("current_chord_notes[%d]=%d %s", j, current_chord_notes[j], note_desig[current_chord_notes[j]%MAX_NOTES]);
				
				int note_to_play=current_chord_notes[j];
				DEBUG("    h_note_to_play=%d %s", note_to_play, note_desig[note_to_play%MAX_NOTES]);
				
				if (theMeanderState.theHarmonyParms.enabled) 
				{
					if (j<4)
					{
						theMeanderState.theHarmonyParms.last[j].note=note_to_play;
						theMeanderState.theHarmonyParms.last[j].noteType=NOTE_TYPE_CHORD;
						theMeanderState.theHarmonyParms.last[j].length=1;  // need chords per measure
						theMeanderState.theHarmonyParms.last[j].time32s=barts_count;
						theMeanderState.theHarmonyParms.last[j].countInBar=bar_note_count;
						if (bar_note_count<256)
						played_notes_circular_buffer[bar_note_count++]=theMeanderState.theHarmonyParms.last[j];
					}
				
					outputs[OUT_HARMONY_CV_OUTPUT].setVoltage((note_to_play/12.0)-4.0,j);  // (note, channel)
				}
		}
		if (theMeanderState.theHarmonyParms.enabled)
		{ 
			outputs[OUT_HARMONY_VOLUME_OUTPUT].setVoltage(theMeanderState. theHarmonyParms.volume);

			// output some fBm noise
			outputs[OUT_FBM_GATE_OUTPUT].setChannels(1);  // set polyphony  
			outputs[OUT_FBM_GATE_OUTPUT].setVoltage((float)fBmrand ,0); 

		//	frequency = tempo/60.0f;  // drives 1 tick per 32nd note
		//	LFOclock.setFreq(frequency*(32/time_sig_bottom));	  // for 32ts	
		//	barts_count_limit = (32*time_sig_top/time_sig_bottom);
		//	float freq32s= (frequency*(32/time_sig_bottom));	  // for 32ts	
		//	float time32s=1.0f/freq32s;
		//	float note_duration=.5f*time32s*(time_sig_top*(32/time_sig_bottom)/theMeanderState.theHarmonyParms.note_length_divisor);
		//	float staccato=0.5f;
		//	float legato=0.99f; 
			float durationFactor=1.0;
			if (theMeanderState.theHarmonyParms.enable_staccato)
				durationFactor=0.5;
			else
				durationFactor=1.0;
						
			float note_duration=durationFactor*time_sig_top/(frequency*theMeanderState.theHarmonyParms.note_length_divisor);
		    harmonyGatePulse.trigger(note_duration);  
		}

		++circle_step_index;
		if (circle_step_index>=theActiveHarmonyType.num_harmony_steps)
			circle_step_index=0;

		if ((harmony_type==22)&&(step==0))
		{
			float rnd = rack::random::uniform();
			int temp_num_harmony_steps=1 + (int)((rnd*(theHarmonyTypes[22].num_harmony_steps-1)));
			bar_count += (theHarmonyTypes[22].num_harmony_steps-temp_num_harmony_steps);
		}

	}

	void doMelody()
	{
		DEBUG("doMelody()");

		outputs[OUT_MELODY_VOLUME_OUTPUT].setVoltage(theMeanderState. theMelodyParms.volume);
		clock_t current_cpu_t= clock();  // cpu clock ticks since program began
		double current_cpu_time_double= (double)(current_cpu_t) / (double)CLOCKS_PER_SEC;
		//	DEBUG("current_cpu_time=%ld", (long)current_cpu_t);
	//	DEBUG("current_cpu_time_double=%.3lf", (double)current_cpu_time_double);

		DEBUG("Melody: Time=%.3lf",  (double)current_cpu_time_double);

		++theMeanderState.theMelodyParms.bar_melody_counted_note;

		theMeanderState.theArpParms.note_count=0;  // where does this really go, at the begining of a melody note
	
		double period=1.0/theMeanderState.theMelodyParms.period; // 1/seconds
		double fBmarg=theMeanderState.theMelodyParms.seed + (double)(period*current_cpu_time_double); 
		double fBmrand=(FastfBm1DNoise(fBmarg,theMeanderState.theMelodyParms.noctaves) +1.)/2; 
	//	DEBUG("fBmrand=%f",(float)fBmrand);
		
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
		theMeanderState.theMelodyParms.last_chord_note_index= note_index;
		int note_to_play=step_chord_notes[step][note_index]; 

		if (theMeanderState.theMelodyParms.chordal)
		{
			note_index=	(int)(theMeanderState.theMelodyParms.note_avg*num_step_chord_notes[step]);	
			note_to_play=step_chord_notes[step][note_index]; 
	
		}
		else
		if (theMeanderState.theMelodyParms.scaler)
		{
			note_index=	(int)(theMeanderState.theMelodyParms.note_avg*num_root_key_notes[root_key]);
			note_to_play=root_key_notes[root_key][note_index]; 
		}


        if (false) // accidentals are probably not notated correctly  Flat accidentals may sound better than sharp
		{
			if ((theMeanderState.theMelodyParms.bar_melody_counted_note!=1)&&(theMeanderState.theMelodyParms.bar_melody_counted_note==(theMeanderState.theMelodyParms.note_length_divisor-1))) // allow accidentals, but not on first or last melody note in bar
			{
			
				float rnd = rack::random::uniform();
				if (rnd<.05)
					note_to_play += 1;
				else
				if (rnd>.90)
					note_to_play -= 1;
			}
		}
				
		DEBUG("    melody note_to_play=%d %s", note_to_play, note_desig[note_to_play%MAX_NOTES]);
				
		if (theMeanderState.theMelodyParms.enabled)
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
				if (bar_note_count<256)
				played_notes_circular_buffer[bar_note_count++]=theMeanderState.theMelodyParms.last[0];
			
											
				outputs[OUT_MELODY_CV_OUTPUT].setChannels(1);  // set polyphony  may need to deal with unset channel voltages
				outputs[OUT_MELODY_CV_OUTPUT].setVoltage(((note_to_play/12.0) -4.0) ,0);  // (note, channel) -4 since midC=c4=0voutputs[OUT_MELODY_CV_OUTPUT].setVoltage((note_to_play -4 + theMeanderState.theMelodyParms.target_octave,0);  // (note, channel) -4 since midC=c4=0v
			//	outputs[OUT_MELODY_GATE_OUTPUT].setVoltage(theMeanderState.theMelodyParms.volume);

				// output some fBm noise
				outputs[OUT_FBM_CV_OUTPUT].setChannels(1);  // set polyphony  
				outputs[OUT_FBM_CV_OUTPUT].setVoltage((float)fBmrand ,0); 

			//	melodyGatePulse.trigger(1.0f);  // Test 1s duration  need to use .process to detect this and then send it to output

			//	float freq32s= (frequency*(32/time_sig_bottom));	  // for 32ts	
			//	float time32s=1.0f/freq32s;
			//	float note_duration=.5f*time32s*(time_sig_top*(32/time_sig_bottom)/theMeanderState.theMelodyParms.note_length_divisor);
			//	float staccato=0.5f;
			//	float legato=0.99f;

				float durationFactor=1.0;
				if (theMeanderState.theMelodyParms.enable_staccato)
					durationFactor=0.5;
				else
					durationFactor=1.0;
				float note_duration=durationFactor*time_sig_top/(frequency*theMeanderState.theMelodyParms.note_length_divisor);
				melodyGatePulse.trigger(note_duration);  // Test 1s duration  need to use .process to detect this and then send it to output
			}
		}
	}

	void doArp() 
	{
		DEBUG("doArp()");
	
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

		
		DEBUG("theMeanderState.theMelodyParms.last_chord_note_index=%d", theMeanderState.theMelodyParms.last_chord_note_index);
		DEBUG("num_step_chord_notes[%d]=%d", theMeanderState.theMelodyParms.last_step, num_step_chord_notes[theMeanderState.theMelodyParms.last_step]);
				
		int note_to_play=100; // bogus

		if (theMeanderState.theArpParms.chordal) // use step_chord_notes
		{
           note_to_play=step_chord_notes[theMeanderState.theMelodyParms.last_step][(theMeanderState.theMelodyParms.last_chord_note_index + arp_note)% num_step_chord_notes[theMeanderState.theMelodyParms.last_step]];
		}
		else 
		if (theMeanderState.theArpParms.scaler) // use root_key_notes rather than step_chord_notes.  This is slower since scale note index has to be looked up
		{   
			if (false)  // old brute force search from beginning
			{
				for (int x=0; x<num_root_key_notes[root_key]; ++x)
				{
					if (root_key_notes[root_key][x]==theMeanderState.theMelodyParms.last[0].note)
					{
						note_to_play=root_key_notes[root_key][x+arp_note];
						DEBUG("note fount at index=%d root_key_notes[root_key][x]=%d", x, root_key_notes[root_key][x]);
						break;
					}
				}
			}
		
			if (true)  // new // BSP search
			{
				int note_to_search_for=theMeanderState.theMelodyParms.last[0].note;
				DEBUG("BSP  note_to_search_for=%d",  note_to_search_for);
				int num_to_search=num_root_key_notes[root_key];
				DEBUG("BSP num_to_search=%d", num_to_search);
				int start_search_index=0;
				int end_search_index=num_root_key_notes[root_key]-1;
				int pass=0;
				int partition_index=0;
				while (pass<8)
				{
					DEBUG("start_search_index=%d end_search_index=%d", start_search_index, end_search_index);
					partition_index=(end_search_index+start_search_index)/2;
					DEBUG("BSP start_search_index=%d end_search_index=%d partition_index=%d", start_search_index, end_search_index, partition_index);
					if ( note_to_search_for>root_key_notes[root_key][partition_index])
					{
						start_search_index=partition_index;
						DEBUG(">BSP root_key_notes[root_key][partition_index]=%d", root_key_notes[root_key][partition_index]);
					}
					else
					if ( note_to_search_for<root_key_notes[root_key][partition_index])
					{
						end_search_index=partition_index;
						DEBUG("<BSP root_key_notes[root_key][partition_index]=%d", root_key_notes[root_key][partition_index]);
					}
					else
					{
						/* we found it */
						DEBUG("value %d found at index %d", root_key_notes[root_key][partition_index], partition_index);
						pass=8;
						break;
					}
					++pass;
				}
				if ((partition_index>=0) && (partition_index<num_to_search))
					note_to_play=root_key_notes[root_key][partition_index+arp_note];
							
			}
			
		}
		
		
	
		if ((theMeanderState.theMelodyParms.enabled)&&(theMeanderState.theArpParms.enabled)&&theMeanderState.theArpParms.note_count<32)
		{
			theMeanderState.theArpParms.last[theMeanderState.theArpParms.note_count].note=note_to_play;
			theMeanderState.theArpParms.last[theMeanderState.theArpParms.note_count].noteType=NOTE_TYPE_ARP;
			theMeanderState.theArpParms.last[theMeanderState.theArpParms.note_count].length=theMeanderState.theArpParms.note_length_divisor;
			theMeanderState.theArpParms.last[theMeanderState.theArpParms.note_count].time32s=barts_count;
			theMeanderState.theArpParms.last[theMeanderState.theArpParms.note_count].countInBar=bar_note_count;
			if (bar_note_count<256)
			played_notes_circular_buffer[bar_note_count++]=theMeanderState.theArpParms.last[theMeanderState.theArpParms.note_count];
		}
	
		outputs[OUT_MELODY_CV_OUTPUT].setChannels(1);  // set polyphony  may need to deal with unset channel voltages
		outputs[OUT_MELODY_CV_OUTPUT].setVoltage(((note_to_play/12.0) -4.0) ,0);  // (note, channel) -4 since midC=c4=0voutputs[OUT_MELODY_CV_OUTPUT].setVoltage((note_to_play -4 + theMeanderState.theMelodyParms.target_octave,0);  // (note, channel) -4 since midC=c4=0v
	//	outputs[OUT_MELODY_GATE_OUTPUT].setVoltage(volume);
		melodyGatePulse.reset();  // kill the pulse in case it is active
	 //   melodyGatePulse.trigger(1.0f);  // Test 1s duration  need to use .process to detect this and then send it to output	
		float durationFactor=1.0;
		if (theMeanderState.theMelodyParms.enable_staccato)
			durationFactor=0.5;
		else
			durationFactor=1.0;
		float note_duration=durationFactor*time_sig_top/(frequency*theMeanderState.theArpParms.note_length_divisor);
		melodyGatePulse.trigger(note_duration);  
	}
 

	void doBass()
	{
		DEBUG("doBass()");
	//	clock_t current_cpu_t= clock();  // cpu clock ticks since program began
	//	double current_cpu_time_double= (double)(current_cpu_t) / (double)CLOCKS_PER_SEC;
		//	DEBUG("current_cpu_time=%ld", (long)current_cpu_t);
		//	DEBUG("current_cpu_time_double=%.3lf", (double)current_cpu_time_double);
        outputs[OUT_BASS_VOLUME_OUTPUT].setVoltage(theMeanderState. theBassParms.volume);
				
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
			DEBUG("    bass note to play=%d %s", theMeanderState.last_harmony_chord_root_note, note_desig[theMeanderState.last_harmony_chord_root_note%MAX_NOTES]);
				
			theMeanderState.theBassParms.last[0].note=theMeanderState.last_harmony_chord_root_note+ (theMeanderState.theBassParms.target_octave*12);  
			theMeanderState.theBassParms.last[0].noteType=NOTE_TYPE_BASS;
			theMeanderState.theBassParms.last[0].length=1;  // need bass notes per measure
			theMeanderState.theBassParms.last[0].time32s=barts_count;
			theMeanderState.theBassParms.last[0].countInBar=bar_note_count;
			if (bar_note_count<256)
			played_notes_circular_buffer[bar_note_count++]=theMeanderState.theBassParms.last[0];

			outputs[OUT_BASS_CV_OUTPUT].setVoltage((theMeanderState.last_harmony_chord_root_note/12.0)-4.0 +theMeanderState.theBassParms.target_octave ,0);  //(note, channel)	
				
			if (theMeanderState.theBassParms.octave_enabled)
			{
		
				theMeanderState.theBassParms.last[1].note=theMeanderState.theBassParms.last[0].note+12; 
				theMeanderState.theBassParms.last[1].noteType=NOTE_TYPE_BASS;
				theMeanderState.theBassParms.last[1].length=1;  // need bass notes per measure
				theMeanderState.theBassParms.last[1].time32s=barts_count;
				theMeanderState.theBassParms.last[1].countInBar=bar_note_count;
				if (bar_note_count<256)
				played_notes_circular_buffer[bar_note_count++]=theMeanderState.theBassParms.last[1];

			 	outputs[OUT_BASS_CV_OUTPUT].setVoltage((theMeanderState.last_harmony_chord_root_note/12.0)-3.0 +theMeanderState.theBassParms.target_octave ,1);
			}

			if (!theMeanderState.theBassParms.accent)  // need o redo with new gate procedure
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
				durationFactor=1.0;
			float note_duration=durationFactor*time_sig_top/(frequency*theMeanderState.theBassParms.note_length_divisor);
		    bassGatePulse.trigger(note_duration);  // Test 1s duration  need to use .process to detect this and then send it to output
		}
	}
   
	LFOGenerator LFOclock;
	
	dsp::SchmittTrigger ST_32ts_trig;  // 32nd note timer tick

	dsp::SchmittTrigger run_button_trig;
	dsp::SchmittTrigger ext_run_trig;
	dsp::SchmittTrigger reset_btn_trig;
	dsp::SchmittTrigger reset_ext_trig;
	dsp::SchmittTrigger bpm_mode_trig;

	dsp::PulseGenerator resetPulse;
	bool reset_pulse = false;

	dsp::PulseGenerator runPulse;
	bool run_pulse = false;

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

	bool running = true;
	
	int bar_count = 0;  // number of bars running count
	
	int i32ts_count = 0;  // counted 32s notes per 32s note
	int i16ts_count = 0;  // counted 32s notes per sixteenth note
	int i8ts_count = 0;  // counted 32s notes per eighth note
	int i4ts_count = 0; // counted 32s notes per quarter note
	int i2ts_count = 0; // counted 32s notes per half note
	int barts_count = 0;     // counted 32s notes per bar

	float tempo =120.0f;
	float frequency = 2.0f;

	
	int i32ts_count_limit = 1;// 32s notes per 32s note
	int i16ts_count_limit = 2;// 32s notes per sixteenth note
	int i8ts_count_limit = 4;   // 32s notes per eighth note
	int i4ts_count_limit = 8;  // 32s notes per quarter note
	int i2ts_count_limit =16;  // 32s notes per half note
	int barts_count_limit = 32;     // 32s notes per bar
	
	float min_bpm = 10.0f;
	float max_bpm = 300.0f;

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

	bool time_sig_changed=false;
   
    	
	void process(const ProcessArgs &args) override 
	{
		
		if (!instanceRunning)
			return;
	
		if (!initialized)
			return;

		//Run
		if (RunToggle.process(params[BUTTON_RUN_PARAM].getValue()) || inputs[IN_RUN_EXT].getVoltage()) 
		{ 
			if (!running)
				bar_note_count=0;  // reinitialize if running just starting
			running=!running;
			theMeanderState.theHarmonyParms.pending_step_edit=0;
			runPulse.trigger(0.01f); // delay 10ms
		}
		lights[LIGHT_LEDBUTTON_RUN].value = running ? 1.0f : 0.0f; 
		run_pulse = runPulse.process(1.0 / args.sampleRate);  
		outputs[OUT_RUN_OUT].setVoltage((run_pulse ? 10.0f : 0.0f));

		if (inputs[IN_TEMPO_EXT].isConnected())
		{
			float fvalue=inputs[IN_TEMPO_EXT].getVoltage();
			tempo=std::round(std::pow(2.0, fvalue)*120);
			if (tempo<10)
				tempo=10;
			if (tempo>300)
				tempo=300;
		}
		else
		{
			//	tempo = std::round(params[CONTROL_TEMPOBPM_PARAM].getValue());
			float fvalue = std::round(params[CONTROL_TEMPOBPM_PARAM].getValue());
			if (fvalue!=tempo)
			tempo=fvalue;
		}

		frequency = tempo/60.0f;  // drives 1 tick per 32nd note
						
		// Reset

		if (time_sig_changed) 
		{
			LFOclock.setReset(1.0f);
			bar_count = 0;
			bar_note_count=0;
		
			i32ts_count = i32ts_count_limit;  // so play will start at the first beat, properly initialized
			i16ts_count = i16ts_count_limit;
			i8ts_count = i8ts_count_limit;
			i4ts_count = i4ts_count_limit;
			i2ts_count = i2ts_count_limit;
			barts_count = barts_count_limit;

			theMeanderState.theMelodyParms.bar_melody_counted_note=0;
			theMeanderState.theArpParms.note_count=0;
			theMeanderState.theBassParms.bar_bass_counted_note=0;
			resetPulse.trigger(0.01f);
			time_sig_changed=false;
		
		}

		if (reset_btn_trig.process(params[BUTTON_RESET_PARAM].getValue() || inputs[IN_RESET_EXT].getVoltage())) 
		{
			LFOclock.setReset(1.0f);
			bar_count = 0;
			bar_note_count=0;
		
			i32ts_count = i32ts_count_limit;  // so play will start at the first beat, properly initialized
			i16ts_count = i16ts_count_limit;
			i8ts_count = i8ts_count_limit;
			i4ts_count = i4ts_count_limit;
			i2ts_count = i2ts_count_limit;
			barts_count = barts_count_limit;

			theMeanderState.theMelodyParms.bar_melody_counted_note=0;
			theMeanderState.theArpParms.note_count=0;
			theMeanderState.theBassParms.bar_bass_counted_note=0;

			theMeanderState.theHarmonyParms.last_circle_step=-1; // for Markov chain
			resetLight = 1.0;
			resetPulse.trigger(0.01f);
		
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
		lights[LIGHT_LEDBUTTON_RESET].value = resetLight;
		reset_pulse = resetPulse.process(1.0 / args.sampleRate);
  		outputs[OUT_RESET_OUT].setVoltage((reset_pulse ? 10.0f : 0.0f));

		if(!running)
		{
			i32ts_count = i32ts_count_limit;  // so play will start at the first beat, properly initialized
			i16ts_count = i16ts_count_limit;
			i8ts_count = i8ts_count_limit;
			i4ts_count = i4ts_count_limit;
			i2ts_count = i2ts_count_limit;
			barts_count = barts_count_limit;
		
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
			i2ts_count_limit =16;  // these never change 32nds per note
			i4ts_count_limit = 8;
			i8ts_count_limit = 4;
			i16ts_count_limit = 2;
			i32ts_count_limit = 1;  
			LFOclock.setFreq(frequency*(32/time_sig_bottom));	  // for 32ts	
			barts_count_limit = (32*time_sig_top/time_sig_bottom);
		}

		if(running) 
		{
						 
			LFOclock.step(1.0 / args.sampleRate);

			bool clockTick=false;
			if ( inputs[IN_CLOCK_EXT].isConnected())
			{
				if (ST_32ts_trig.process(inputs[IN_CLOCK_EXT].getVoltage()))  // triggers from each external clock tick 
				 	 clockTick=true;
			}
			else
			{
				if (ST_32ts_trig.process(LFOclock.sqr()))
					 clockTick=true;
			}
			
				   
		    if (clockTick)
			{
				bool melodyPlayed=false;   // set to prevent arp note being played on the melody beat
				// bar
				if (barts_count == barts_count_limit)
				{
					barts_count = 0;  
					theMeanderState.theMelodyParms.bar_melody_counted_note=0;
					theMeanderState.theBassParms.bar_bass_counted_note=0;
					bar_note_count=0;
			
					if (theMeanderState.theHarmonyParms.note_length_divisor==1)
						doHarmony();
					if (theMeanderState.theBassParms.note_length_divisor==1)
						doBass();
					if (theMeanderState.theMelodyParms.note_length_divisor==1)
					{
						doMelody();
						melodyPlayed=true;
					}
					++bar_count;  // moved here so reset will start on first step rather than second
					clockPulse1ts.trigger(trigger_length);
					// Pulse the output gate 
					barTriggerPulse.trigger(1e-3f);  // 1ms duration  need to use .process to detect this and then send it to output
				}
			
		        // i2ts
				if (i2ts_count == i2ts_count_limit)

				{
					i2ts_count = 0;    
					if (theMeanderState.theHarmonyParms.note_length_divisor==2)
						doHarmony();
					if (theMeanderState.theBassParms.note_length_divisor==2)
						doBass();
					if (theMeanderState.theMelodyParms.note_length_divisor==2)
					{
						doMelody();
						melodyPlayed=true;
					}

			
					clockPulse2ts.trigger(trigger_length);
				}
		
				// i4ts
				if (i4ts_count == i4ts_count_limit)
				{
					i4ts_count = 0;  
			
					if (theMeanderState.theHarmonyParms.note_length_divisor==4)
						doHarmony();
					if (theMeanderState.theBassParms.note_length_divisor==4)
						doBass();
					if (theMeanderState.theMelodyParms.note_length_divisor==4)
					{
						doMelody();
						melodyPlayed=true;
					}
					if ((theMeanderState.theArpParms.enabled)&&(theMeanderState.theArpParms.note_length_divisor==4)&&(!melodyPlayed))
						doArp();
					clockPulse4ts.trigger(trigger_length);
					
				//	barTriggerPulse.trigger(1e-3f);  // 1ms duration  need to use .process to detect this and then send it to output
				}
					  
		 		// i8ts
				if (i8ts_count == i8ts_count_limit)
				{
					i8ts_count = 0;  
					if (theMeanderState.theHarmonyParms.note_length_divisor==8)
						doHarmony();
					if (theMeanderState.theBassParms.note_length_divisor==8)
						doBass();
					if (theMeanderState.theMelodyParms.note_length_divisor==8)
					{
						doMelody();
						melodyPlayed=true;
					}
					if ((theMeanderState.theArpParms.enabled)&&(theMeanderState.theArpParms.note_length_divisor==8)&&(!melodyPlayed))
						doArp();
				
					clockPulse8ts.trigger(trigger_length);
				}

				// i16ts
				if (i16ts_count == i16ts_count_limit)
				{
					if (theMeanderState.theMelodyParms.note_length_divisor==16)
					{
						doMelody();  
						melodyPlayed=true;  
					}
					if ((theMeanderState.theArpParms.enabled)&&(theMeanderState.theArpParms.note_length_divisor==16)&&(!melodyPlayed))
						doArp();
					i16ts_count = 0;
					clockPulse16ts.trigger(trigger_length);
				}


				//32nds  ***********************************

				clock_t current_cpu_t= clock();  // cpu clock ticks since program began
				double current_cpu_time_double= (double)(current_cpu_t) / (double)CLOCKS_PER_SEC;
			
				 // do on each 1/32nd clock tick
				
				if (theMeanderState.theMelodyParms.note_length_divisor==32)
				{
					doMelody();   
					melodyPlayed=true; 
				}
				if ((theMeanderState.theArpParms.enabled)&&(theMeanderState.theArpParms.note_length_divisor==32)&&(!melodyPlayed))
					doArp(); 
								
				clockPulse32ts.trigger(trigger_length);

				// output some fBm noise
				double period=1.0/theMeanderState.theArpParms.period; // 1/seconds
				double fBmarg=theMeanderState.theArpParms.seed + (double)(period*current_cpu_time_double); 
				double fBmrand=(FastfBm1DNoise(fBmarg,theMeanderState.theArpParms.noctaves) +1.)/2; 
				outputs[OUT_FBM_TRIGGER_OUTPUT].setChannels(1);  // set polyphony  
				outputs[OUT_FBM_TRIGGER_OUTPUT].setVoltage((float)fBmrand ,0); 
				

				barts_count++;    
				i2ts_count++;
				i4ts_count++;
				i8ts_count++;
				i16ts_count++;
				i32ts_count++;
				
			}
		}

		pulse1ts = clockPulse1ts.process(1.0 / args.sampleRate);
		pulse2ts = clockPulse2ts.process(1.0 / args.sampleRate);
		pulse4ts = clockPulse4ts.process(1.0 / args.sampleRate);
		pulse8ts = clockPulse8ts.process(1.0 / args.sampleRate);
		pulse16ts = clockPulse16ts.process(1.0 / args.sampleRate);
		pulse32ts = clockPulse32ts.process(1.0 / args.sampleRate);

		// end the gate if pulse timer has expired 

		if (true) // standard gate voltages 
		{
			outputs[OUT_HARMONY_GATE_OUTPUT].setVoltage( harmonyGatePulse.process( 1.0 / APP->engine->getSampleRate() ) ? CV_MAX10 : 0.0 ); 
			outputs[OUT_MELODY_GATE_OUTPUT].setVoltage( melodyGatePulse.process( 1.0 / APP->engine->getSampleRate() ) ? CV_MAX10 : 0.0 ); 
			outputs[OUT_BASS_GATE_OUTPUT].setVoltage( bassGatePulse.process( 1.0 / APP->engine->getSampleRate() ) ? CV_MAX10 : 0.0 ); 

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
		else  // non-standard volume over gate voltages
		{
			float harmonyGateLevel=theMeanderState.theHarmonyParms.volume; 
			harmonyGateLevel=clamp(harmonyGateLevel, 1.1f, 10.f);  // don't let gate on level drop below 1.0v so it will trigger ADSR etc.
			outputs[OUT_HARMONY_GATE_OUTPUT].setVoltage( harmonyGatePulse.process( 1.0 / APP->engine->getSampleRate() ) ? harmonyGateLevel : 0.0 ); 

			float melodyGateLevel=theMeanderState.theMelodyParms.volume; 
			melodyGateLevel=clamp(melodyGateLevel, 1.1f, 10.f);   // don't let gate on level drop below 1.0v so it will trigger ADSR etc.
			outputs[OUT_MELODY_GATE_OUTPUT].setVoltage( melodyGatePulse.process( 1.0 / APP->engine->getSampleRate() ) ? melodyGateLevel : 0.0 ); 

			float bassGateLevel=theMeanderState.theBassParms.volume;
			if (theMeanderState.theBassParms.note_accented)
				bassGateLevel=10.0;
			bassGateLevel=clamp(bassGateLevel, 1.1f, 10.f); // don't let gate on level drop below 1.0v so it will trigger ADSR etc.
			outputs[OUT_BASS_GATE_OUTPUT].setVoltage( bassGatePulse.process( 1.0 / APP->engine->getSampleRate() ) ?bassGateLevel : 0.0 ); 
		}
				
				
		outputs[OUT_CLOCK_BAR_OUTPUT].setVoltage((pulse1ts ? 10.0f : 0.0f));     // barts
		outputs[OUT_CLOCK_BEAT_OUTPUT].setVoltage((pulse4ts ? 10.0f : 0.0f));    // 4ts
		outputs[OUT_CLOCK_BEATX2_OUTPUT].setVoltage((pulse8ts ? 10.0f : 0.0f));  // 8ts
		outputs[OUT_CLOCK_BEATX4_OUTPUT].setVoltage((pulse16ts ? 10.0f : 0.0f)); // 16ts
		outputs[OUT_CLOCK_BEATX8_OUTPUT].setVoltage((pulse32ts ? 10.0f : 0.0f)); // 32ts

	        
		if (HarmonyEnableToggle.process(params[BUTTON_ENABLE_HARMONY_PARAM].getValue())) 
		{
			theMeanderState. theHarmonyParms.enabled = !theMeanderState. theHarmonyParms.enabled;
			theMeanderState.userControllingHarmonyFromCircle=false;
		}
		lights[LIGHT_LEDBUTTON_HARMONY_ENABLE].value = theMeanderState. theHarmonyParms.enabled ? 1.0f : 0.0f; 

		if (HarmonyEnableAll7thsToggle.process(params[BUTTON_ENABLE_HARMONY_ALL7THS_PARAM].getValue())) 
		{
			theMeanderState. theHarmonyParms.enable_all_7ths = !theMeanderState. theHarmonyParms.enable_all_7ths;
			setup_harmony();  // calculate harmony notes
			circleChanged=true;
		}
		lights[LIGHT_LEDBUTTON_ENABLE_HARMONY_ALL7THS_PARAM].value = theMeanderState. theHarmonyParms.enable_all_7ths ? 1.0f : 0.0f; 

		if (HarmonyEnableV7thsToggle.process(params[BUTTON_ENABLE_HARMONY_V7THS_PARAM].getValue())) 
		{
			theMeanderState. theHarmonyParms.enable_V_7ths = !theMeanderState. theHarmonyParms.enable_V_7ths;
			setup_harmony();
			circleChanged=true;
		}
		lights[LIGHT_LEDBUTTON_ENABLE_HARMONY_V7THS_PARAM].value = theMeanderState. theHarmonyParms.enable_V_7ths ? 1.0f : 0.0f; 
//
		if (HarmonyEnableStaccatoToggle.process(params[BUTTON_ENABLE_HARMONY_STACCATO_PARAM].getValue())) 
		{
			theMeanderState. theHarmonyParms.enable_staccato = !theMeanderState. theHarmonyParms.enable_staccato;
	
		}
		lights[LIGHT_LEDBUTTON_ENABLE_HARMONY_STACCATO_PARAM].value = theMeanderState. theHarmonyParms.enable_staccato ? 1.0f : 0.0f; 

		if (MelodyEnableStaccatoToggle.process(params[BUTTON_ENABLE_MELODY_STACCATO_PARAM].getValue())) 
		{
			theMeanderState. theMelodyParms.enable_staccato = !theMeanderState. theMelodyParms.enable_staccato;
	
		}
		lights[LIGHT_LEDBUTTON_ENABLE_MELODY_STACCATO_PARAM].value = theMeanderState. theMelodyParms.enable_staccato ? 1.0f : 0.0f; 

		if (BassEnableStaccatoToggle.process(params[BUTTON_ENABLE_BASS_STACCATO_PARAM].getValue())) 
		{
			theMeanderState. theBassParms.enable_staccato = !theMeanderState. theBassParms.enable_staccato;
	
		}
		lights[LIGHT_LEDBUTTON_ENABLE_BASS_STACCATO_PARAM].value = theMeanderState. theBassParms.enable_staccato ? 1.0f : 0.0f; 
//
		
		if (BassEnableToggle.process(params[BUTTON_ENABLE_BASS_PARAM].getValue())) 
		{
			theMeanderState.theBassParms.enabled = !theMeanderState.theBassParms.enabled;
		}
		lights[LIGHT_LEDBUTTON_BASS_ENABLE].value = theMeanderState.theBassParms.enabled ? 1.0f : 0.0f; 

		
		
		if (MelodyEnableToggle.process(params[BUTTON_ENABLE_MELODY_PARAM].getValue())) 
		{
			theMeanderState.theMelodyParms.enabled = !theMeanderState.theMelodyParms.enabled;
		}
		lights[LIGHT_LEDBUTTON_MELODY_ENABLE].value = theMeanderState.theMelodyParms.enabled ? 1.0f : 0.0f; 

		if (MelodyDestutterToggle.process(params[BUTTON_MELODY_DESTUTTER_PARAM].getValue())) 
		{
			theMeanderState.theMelodyParms.destutter = !theMeanderState.theMelodyParms.destutter;
		}
		lights[LIGHT_LEDBUTTON_MELODY_DESTUTTER].value = theMeanderState.theMelodyParms.destutter ? 1.0f : 0.0f; 

		if (MelodyEnableChordalToggle.process(params[BUTTON_ENABLE_MELODY_CHORDAL_PARAM].getValue())) 
		{
			theMeanderState.theMelodyParms.chordal = !theMeanderState.theMelodyParms.chordal;
			theMeanderState.theMelodyParms.scaler = !theMeanderState.theMelodyParms.scaler;
		}
		lights[LIGHT_LEDBUTTON_MELODY_ENABLE_CHORDAL].value = theMeanderState.theMelodyParms.chordal ? 1.0f : 0.0f; 

		if (MelodyEnableScalerToggle.process(params[BUTTON_ENABLE_MELODY_SCALER_PARAM].getValue())) 
		{
			theMeanderState.theMelodyParms.scaler = !theMeanderState.theMelodyParms.scaler;
			theMeanderState.theMelodyParms.chordal = !theMeanderState.theMelodyParms.chordal;
		}
		lights[LIGHT_LEDBUTTON_MELODY_ENABLE_SCALER].value = theMeanderState.theMelodyParms.scaler ? 1.0f : 0.0f; 

				

		if (ArpEnableToggle.process(params[BUTTON_ENABLE_ARP_PARAM].getValue())) 
		{
			theMeanderState.theArpParms.enabled = !theMeanderState.theArpParms.enabled;
		}
		lights[LIGHT_LEDBUTTON_ARP_ENABLE].value = theMeanderState.theArpParms.enabled ? 1.0f : 0.0f; 

		if (ArpEnableChordalToggle.process(params[BUTTON_ENABLE_ARP_CHORDAL_PARAM].getValue())) 
		{
			theMeanderState.theArpParms.chordal = !theMeanderState.theArpParms.chordal;
			theMeanderState.theArpParms.scaler = !theMeanderState.theArpParms.scaler;
		}
		lights[LIGHT_LEDBUTTON_ARP_ENABLE_CHORDAL].value = theMeanderState.theArpParms.chordal ? 1.0f : 0.0f; 

		if (ArpEnableScalerToggle.process(params[BUTTON_ENABLE_ARP_SCALER_PARAM].getValue())) 
		{
			theMeanderState.theArpParms.scaler = !theMeanderState.theArpParms.scaler;
			theMeanderState.theArpParms.chordal = !theMeanderState.theArpParms.chordal;
		}
		lights[LIGHT_LEDBUTTON_ARP_ENABLE_SCALER].value = theMeanderState.theArpParms.scaler ? 1.0f : 0.0f; 

		//****Bass

		if (BassSyncopateToggle.process(params[BUTTON_BASS_SYNCOPATE_PARAM].getValue())) 
		{
			theMeanderState.theBassParms.syncopate = !theMeanderState.theBassParms.syncopate;
		}
		lights[LIGHT_LEDBUTTON_BASS_SYNCOPATE_PARAM].value = theMeanderState.theBassParms.syncopate ? 1.0f : 0.0f; 	

		if (BassAccentToggle.process(params[BUTTON_BASS_ACCENT_PARAM].getValue())) 
		{
			theMeanderState.theBassParms.accent = !theMeanderState.theBassParms.accent;
		}
		lights[LIGHT_LEDBUTTON_BASS_ACCENT_PARAM].value = theMeanderState.theBassParms.accent ? 1.0f : 0.0f; 	

		if (BassShuffleToggle.process(params[BUTTON_BASS_SHUFFLE_PARAM].getValue())) 
		{
			theMeanderState.theBassParms.shuffle = !theMeanderState.theBassParms.shuffle;
		}
		lights[LIGHT_LEDBUTTON_BASS_SHUFFLE_PARAM].value = theMeanderState.theBassParms.shuffle ? 1.0f : 0.0f; 	
		
		//***************
			 
		

		for (int i=0; i<12; ++i) 
		{
			if (CircleStepToggles[i].process(params[BUTTON_CIRCLESTEP_C_PARAM+i].getValue()))  // circle button clicked
			{
				int current_circle_position=i;
				DEBUG("harmony step edit-pt3 current_circle_position=%d", current_circle_position);

				for (int j=0; j<12; ++j) 
				{
					if (j!=current_circle_position) 
					{
						CircleStepStates[j] = false;
						lights[LIGHT_LEDBUTTON_CIRCLESTEP_1+j].value=CircleStepStates[j] ? 1.0f : 0.0f;
					}
				}

				CircleStepStates[current_circle_position] = !CircleStepStates[current_circle_position];
				lights[LIGHT_LEDBUTTON_CIRCLESTEP_1+current_circle_position].value=CircleStepStates[current_circle_position] ? 1.0f : 0.0f;	
			
				userPlaysCirclePosition(current_circle_position, 0); 
				if (running)
				{
					theMeanderState.userControllingHarmonyFromCircle=true;
					theMeanderState. theHarmonyParms.enabled=false;
					lights[LIGHT_LEDBUTTON_HARMONY_ENABLE].value = theMeanderState. theHarmonyParms.enabled ? 1.0f : 0.0f; 
				}

			
				//find this in circle
				
				for (int j=0; j<7; ++j) 
				{
					if  (theCircleOf5ths.theDegreeSemiCircle.degreeElements[j].CircleIndex==current_circle_position)
					{
						int theDegree=theCircleOf5ths.theDegreeSemiCircle.degreeElements[j].Degree;
						DEBUG("harmony step edit-pt4 theDegree=%d", theDegree);
						if ((theDegree>=1)&&(theDegree<=7))
						{
							if (theMeanderState.theHarmonyParms.pending_step_edit)
							{
								DEBUG("harmony step edit-pt5 theMeanderState.theHarmonyParms.pending_step_edit=%d", theMeanderState.theHarmonyParms.pending_step_edit);
								DEBUG("harmony step edit-pt6 theDegree=%d found", theDegree);
								theHarmonyTypes[harmony_type].harmony_steps[theMeanderState.theHarmonyParms.pending_step_edit-BUTTON_HARMONY_SETSTEP_1_PARAM]=theDegree;
								//
								strcpy(theHarmonyTypes[harmony_type].harmony_degrees_desc,"");
								for (int k=0;k<theHarmonyTypes[harmony_type].num_harmony_steps;++k)
								{
									strcat(theHarmonyTypes[harmony_type].harmony_degrees_desc,circle_of_fifths_arabic_degrees[theHarmonyTypes[harmony_type].harmony_steps[k]]);  
									strcat(theHarmonyTypes[harmony_type].harmony_degrees_desc," ");
								}
								//
								copyHarmonyTypeToActiveHarmonyType(harmony_type);
								setup_harmony();
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
					DEBUG("harmony step edit-pt1 step=%d clicked", i);
					int selectedStep=i;
					theMeanderState.theHarmonyParms.pending_step_edit=BUTTON_HARMONY_SETSTEP_1_PARAM+selectedStep;

					int current_circle_position=0;
					if (true)
					{
						int degreeStep=(theActiveHarmonyType.harmony_steps[selectedStep])%8;  
						
						//find this in semicircle
						for (int j=0; j<7; ++j)
						{
							if  (theCircleOf5ths.theDegreeSemiCircle.degreeElements[j].Degree==degreeStep)
							{
								current_circle_position = theCircleOf5ths.theDegreeSemiCircle.degreeElements[j].CircleIndex; 
								DEBUG("harmony step edit-pt2 current_circle_position=%d", current_circle_position);
								break;
							}
						}
					}

					
					for (int i=0; i<12; ++i) 
					{
						lights[LIGHT_LEDBUTTON_CIRCLESTEP_1+i].value=0.0f;	
					}
					lights[LIGHT_LEDBUTTON_CIRCLESTEP_1+current_circle_position].value=1.0f;
					
				
					userPlaysCirclePosition(current_circle_position, 0);  // testing play
					CircleStepSetStates[i] = !CircleStepSetStates[i];
					lights[LIGHT_LEDBUTTON_CIRCLESETSTEP_1+i].value=CircleStepSetStates[i] ? 1.0f : 0.25f;
					
					for (int j=0; j<theActiveHarmonyType.num_harmony_steps; ++j) {
						if (j!=i) {
							CircleStepSetStates[j] = false;
							lights[LIGHT_LEDBUTTON_CIRCLESETSTEP_1+j].value=0.25f;
						}
					}

					
				}
			} 
		}

		float fvalue=0;

        float circleDegree=0;
		float gateValue=0;
		if (  (inputs[IN_HARMONY_GATE_EXT].isConnected())
			&&((gateValue=inputs[IN_HARMONY_GATE_EXT].getVoltage()))
			&&((circleDegree=inputs[IN_HARMONY_CV_EXT].getVoltage())>=0) )  
		{
			DEBUG("IN_HARMONY_GATE_EXT is connected and circleDegree=%f", circleDegree);

			extHarmonyIn=circleDegree;
		
			float octave=(float)((int)(circleDegree));

			circleDegree=(float)std::fmod(std::fabs(circleDegree), 1.0);

			DEBUG("IN_HARMONY_CV_EXT circleDegree=%f", circleDegree);
					
			if ((std::abs(circleDegree-0)<.01)) circleDegree=1;
			if ((std::abs(circleDegree-.167)<.01)) circleDegree=2;
			if ((std::abs(circleDegree-.334)<.01)) circleDegree=3;
			if ((std::abs(circleDegree-.417)<.01)) circleDegree=4;
			if ((std::abs(circleDegree-.584)<.01)) circleDegree=5;
			if ((std::abs(circleDegree-.751)<.01)) circleDegree=6;
			if ((std::abs(circleDegree-.917)<.01)) circleDegree=7;
			if (circleDegree<1)
			  	circleDegree=1;
			if (circleDegree>7)
			  	circleDegree=7;
			DEBUG("IN_HARMONY_CV_EXT=%d", (int)circleDegree);

			int step=1;
			for (int i=0; i<MAX_STEPS; ++i)
			{
				if (theActiveHarmonyType.harmony_steps[i]==circleDegree)
				{
					step=i;
					break;
				}
			}

			theMeanderState.last_harmony_step=step;

			int theCirclePosition=0;
			for (int i=0; i<7; ++i)
			{
				if (theCircleOf5ths.theDegreeSemiCircle.degreeElements[i].Degree==circleDegree)
				{
					theCirclePosition=theCircleOf5ths.theDegreeSemiCircle.degreeElements[i].CircleIndex;
					break;
				}
			}

			last_circle_position=theCirclePosition;

			userPlaysCirclePosition(theCirclePosition, octave); 

			if (running)
			{
				theMeanderState.userControllingHarmonyFromCircle=true;
				theMeanderState. theHarmonyParms.enabled=false;
			}

			for (int i=0; i<12; ++i) 
			{
				CircleStepStates[i] = false;
				lights[LIGHT_LEDBUTTON_CIRCLESTEP_1+i].value=CircleStepStates[i] ? 1.0f : 0.0f;	
			
				for (int j=0; j<12; ++j) 
				{
					if (j!=i) {
				//		lights[LIGHT_LEDBUTTON_CIRCLESETSTEP_1+j].value=0.0f;
					}
				}
					
			}
		
			lights[LIGHT_LEDBUTTON_CIRCLESTEP_1+theCirclePosition].value=1.0f;

			
		}
		
	
		


		//**************************
		if (lightDivider.process())
		{
		}
			
		if (lowFreqClock.process())
		{
			if (!instanceRunning)
				return;
			// check controls for changes

			if ((fvalue=std::round(params[CONTROL_TEMPOBPM_PARAM].getValue()))!=tempo)
			{
				tempo = fvalue;
				DEBUG("tempo changed to %d", (int)tempo);
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
				params[CONTROL_MELODY_NOTE_LENGTH_DIVISOR_PARAM].value=melody_note_length_divisor;

				theMeanderState.theArpParms.note_length_divisor=(int)std::pow(2,melody_note_length_divisor+1);
				params[CONTROL_ARP_INCREMENT_PARAM].value=melody_note_length_divisor+1;
				time_sig_changed=true;
			}
			
			
			frequency = tempo/60.0f;  // BPS
			
			if ((fvalue=std::round(params[CONTROL_ROOT_KEY_PARAM].getValue()))!=circle_root_key)
			{
				circle_root_key=(int)fvalue;
				root_key=circle_of_fifths[circle_root_key];
				DEBUG("root_key changed to %d = %s", root_key, note_desig[root_key]);
				for (int i=0; i<12; ++i)
					lights[LIGHT_CIRCLE_ROOT_KEY_POSITION_1_LIGHT+i].value=0.0f;
				lights[LIGHT_CIRCLE_ROOT_KEY_POSITION_1_LIGHT+circle_root_key].value=1.0f;
				circleChanged=true;
			}

			
			if ((fvalue=std::round(params[CONTROL_SCALE_PARAM].getValue()))!=mode)
			{
				mode = fvalue;
				DEBUG("mode changed to %d", mode);
				circleChanged=true;
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
				DEBUG("harmony_type changed to %d %s", (int)fvalue, "test");  // need actual descriptor
				harmonyStepsChanged=(int)fvalue;  // don't changed until between sequences.  The new harmony_type is in harmonyStepsChanged
			}

			fvalue=std::round(params[CONTROL_HARMONY_STEPS_PARAM].getValue());
			if ((fvalue!=theActiveHarmonyType.num_harmony_steps)&&(fvalue>=theActiveHarmonyType.min_steps)&&(fvalue<=theActiveHarmonyType.max_steps)&&(fvalue!=theActiveHarmonyType.num_harmony_steps))
			{
				DEBUG("theActiveHarmonyType.min_steps=%d, theActiveHarmonyType.max_steps=%d", theActiveHarmonyType.min_steps, theActiveHarmonyType.max_steps );
				DEBUG("theActiveHarmonyType.num_harmony_steps changed to %d %s", (int)fvalue, "test");  // need actual descriptor
				if ((fvalue>=theActiveHarmonyType.min_steps)&&(fvalue<=theActiveHarmonyType.max_steps))
					theActiveHarmonyType.num_harmony_steps=(int)fvalue;  
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

			if (harmonyStepsChanged)
			{
				harmony_type=harmonyStepsChanged;
				copyHarmonyTypeToActiveHarmonyType(harmony_type);
				harmonyStepsChanged=0;
				circleChanged=true;  // trigger off reconstruction and setup
				init_harmony(); // reinitialize in case user has changed harmony parms
				setup_harmony();  // calculate harmony notes
				params[CONTROL_HARMONY_STEPS_PARAM].value=theHarmonyTypes[harmony_type].num_harmony_steps;
			//	AuditHarmonyData(2);
			}
			
			// reconstruct initially and when dirty
			if (circleChanged)  
			{	
				DEBUG("circleChanged");	
				
				notate_mode_as_signature_root_key=((root_key-(mode_natural_roots[mode_root_key_signature_offset[mode]]))+12)%12;
				DEBUG("notate_mode_as_signature_root_key=%d", notate_mode_as_signature_root_key);
				
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
				init_harmony();  // sets up original progressions
				AuditHarmonyData(3);
				setup_harmony();  // calculate harmony notes
				params[CONTROL_HARMONY_STEPS_PARAM].value=theHarmonyTypes[harmony_type].num_harmony_steps;
				AuditHarmonyData(3);
				circleChanged=false;
			}
		}	

		if (sec1Clock.process())
		{
		}
		
		 	     
	}  // end module process()

	~Meander() 
	{
		
		if (instanceRunning) {
		//	 Release ownership of singleton
			owned = false;
		}
	}


	Meander() 
	{

		DEBUG("");  // clear debug log file

				
		if (!owned) {
			// Take ownership of singleton
			owned = true;
		    instanceRunning = true;
 		}
			
		time_t rawtime;
  		time( &rawtime );
   		
		
		lowFreqClock.setDivision(512);  // every 86 samples, 2ms
		sec1Clock.setDivision(44000);
		lightDivider.setDivision(512);  // every 86 samples, 2ms
		   		
		
		initPerlin();
		MeanderMusicStructuresInitialize();

			
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

		for (int i=0; i<12; ++i)
				lights[LIGHT_CIRCLE_ROOT_KEY_POSITION_1_LIGHT+i].value=0.0f;
		lights[LIGHT_CIRCLE_ROOT_KEY_POSITION_1_LIGHT+root_key].value=1.0f;  // loaded root_key might not be 0/C
		
		CircleStepStates[0]=1.0f;
		lights[LIGHT_LEDBUTTON_CIRCLESTEP_1].value=1.0f;
		
		CircleStepSetStates[0]=1.0f;
		lights[LIGHT_LEDBUTTON_CIRCLESETSTEP_1].value=1.0f;
		
						
		configParam(BUTTON_RUN_PARAM, 0.f, 1.f, 0.f, "");
		configParam(BUTTON_RESET_PARAM, 0.f, 1.f, 0.f, "");

		configParam(CONTROL_TEMPOBPM_PARAM, min_bpm, max_bpm, 120.0f, "Tempo", " BPM");
	    configParam(CONTROL_TIMESIGNATURETOP_PARAM,2.0f, 15.0f, 4.0f, "Time Signature Top");
		configParam(CONTROL_TIMESIGNATUREBOTTOM_PARAM,0.0f, 3.0f, 1.0f, "Time Signature Bottom");
		configParam(CONTROL_ROOT_KEY_PARAM, 0, 11, 1.f, "");
		configParam(CONTROL_SCALE_PARAM, 0.f, num_modes-1, 1.f, "");

		configParam(BUTTON_ENABLE_MELODY_PARAM, 0.f, 1.f, 0.f, "");
		configParam(CONTROL_MELODY_VOLUME_PARAM, 0.f, 10.f, 8.0f, "");
		configParam(BUTTON_MELODY_DESTUTTER_PARAM, 0.f, 1.f, 0.f, "");
		configParam(CONTROL_MELODY_NOTE_LENGTH_DIVISOR_PARAM, 0.f, 5.f, 2.f, "");
		configParam(CONTROL_MELODY_TARGETOCTAVE_PARAM, 1.f, 6.f, 3.f, "");
		configParam(CONTROL_MELODY_ALPHA_PARAM, 0.f, 1.f, .9f, "");
		configParam(CONTROL_MELODY_RANGE_PARAM, 0.f, 3.f, 1.f, "");
		configParam(BUTTON_ENABLE_MELODY_STACCATO_PARAM, 0.f, 1.f, 1.f, "");

		configParam(BUTTON_ENABLE_HARMONY_PARAM, 0.f, 1.f, 0.f, "");
		configParam(BUTTON_ENABLE_MELODY_CHORDAL_PARAM, 0.f, 1.f, 1.f, "");
		configParam(BUTTON_ENABLE_MELODY_SCALER_PARAM, 0.f, 1.f, 0.f, "");
		configParam(CONTROL_HARMONY_VOLUME_PARAM, 0.f, 10.f, 8.0f, "");
		configParam(CONTROL_HARMONY_STEPS_PARAM, 1.f, 16.f, 16.f, "");
		configParam(CONTROL_HARMONY_TARGETOCTAVE_PARAM, 1.f, 6.f, 3.f, "");
		configParam(CONTROL_HARMONY_ALPHA_PARAM, 0.f, 1.f, .9f, "");
		configParam(CONTROL_HARMONY_RANGE_PARAM, 0.f, 3.f, 1.f, "");
		configParam(CONTROL_HARMONY_DIVISOR_PARAM, 0.f, 3.f, 0.f, "");
		configParam(BUTTON_ENABLE_HARMONY_ALL7THS_PARAM, 0.f, 1.f, 0.f, "");
		configParam(BUTTON_ENABLE_HARMONY_V7THS_PARAM, 0.f, 1.f, 0.f, "");
		configParam(BUTTON_ENABLE_HARMONY_STACCATO_PARAM, 0.f, 1.f, 0.f, "");
		configParam(CONTROL_HARMONYPRESETS_PARAM, 1.0f, (float)MAX_AVAILABLE_HARMONY_PRESETS, 1.0f, "");

		configParam(BUTTON_ENABLE_ARP_PARAM, 0.f, 1.f, 0.f, "");
		configParam(BUTTON_ENABLE_ARP_CHORDAL_PARAM, 0.f, 1.f, 1.f, "");
		configParam(BUTTON_ENABLE_ARP_SCALER_PARAM, 0.f, 1.f, 0.f, "");
		configParam(CONTROL_ARP_COUNT_PARAM, 0.f, 7.f, 0.f, "");
		configParam(CONTROL_ARP_INCREMENT_PARAM, 2.f, 5.f, 4.f, "");
		configParam(CONTROL_ARP_DECAY_PARAM, 0.f, 1.f, 0.f, "");
		configParam(CONTROL_ARP_PATTERN_PARAM, -3.f, 3.f, 1.f, "");

		configParam(BUTTON_ENABLE_BASS_PARAM, 0.f, 1.f, 0.f, "");
		configParam(CONTROL_BASS_VOLUME_PARAM, 0.f, 10.f, 8.0f, "");
		configParam(CONTROL_BASS_DIVISOR_PARAM, 0.f, 3.f, 0.f, "");
		configParam(CONTROL_BASS_TARGETOCTAVE_PARAM, 0.f, 3.f, 2.f, ""); 
		configParam(BUTTON_BASS_ACCENT_PARAM, 0.f, 1.f, 0.f, "");
		configParam(BUTTON_BASS_SYNCOPATE_PARAM, 0.f, 1.f, 0.f, "");
		configParam(BUTTON_BASS_SHUFFLE_PARAM, 0.f, 1.f, 0.f, "");
		configParam(BUTTON_ENABLE_BASS_STACCATO_PARAM, 0.f, 1.f, 1.f, ""); 
							
		configParam(CONTROL_HARMONY_FBM_OCTAVES_PARAM, 1.f, 6.f, 3.f, "");
		configParam(CONTROL_MELODY_FBM_OCTAVES_PARAM, 1.f, 6.f, 3.f, "");
		configParam(CONTROL_ARP_FBM_OCTAVES_PARAM, 1.f, 6.f, 3.f, "");

		configParam(CONTROL_HARMONY_FBM_PERIOD_PARAM, 1.f, 100.f, 60.f, "");
		configParam(CONTROL_MELODY_FBM_PERIOD_PARAM, 1.f, 100.f, 10.f, "");
		configParam(CONTROL_ARP_FBM_PERIOD_PARAM, 1.f, 100.f, 1.f, "");
       	

		configParam(BUTTON_HARMONY_SETSTEP_1_PARAM, 0.f, 1.f, 0.f, "");
		configParam(BUTTON_HARMONY_SETSTEP_2_PARAM, 0.f, 1.f, 0.f, "");
		configParam(BUTTON_HARMONY_SETSTEP_3_PARAM, 0.f, 1.f, 0.f, "");
		configParam(BUTTON_HARMONY_SETSTEP_4_PARAM, 0.f, 1.f, 0.f, "");
		configParam(BUTTON_HARMONY_SETSTEP_5_PARAM, 0.f, 1.f, 0.f, "");
		configParam(BUTTON_HARMONY_SETSTEP_6_PARAM, 0.f, 1.f, 0.f, "");
		configParam(BUTTON_HARMONY_SETSTEP_7_PARAM, 0.f, 1.f, 0.f, "");
		configParam(BUTTON_HARMONY_SETSTEP_8_PARAM, 0.f, 1.f, 0.f, "");
		configParam(BUTTON_HARMONY_SETSTEP_9_PARAM, 0.f, 1.f, 0.f, "");
		configParam(BUTTON_HARMONY_SETSTEP_10_PARAM, 0.f, 1.f, 0.f, "");
		configParam(BUTTON_HARMONY_SETSTEP_11_PARAM, 0.f, 1.f, 0.f, "");
		configParam(BUTTON_HARMONY_SETSTEP_12_PARAM, 0.f, 1.f, 0.f, "");
		configParam(BUTTON_HARMONY_SETSTEP_13_PARAM, 0.f, 1.f, 0.f, "");
		configParam(BUTTON_HARMONY_SETSTEP_14_PARAM, 0.f, 1.f, 0.f, "");
		configParam(BUTTON_HARMONY_SETSTEP_15_PARAM, 0.f, 1.f, 0.f, "");
		configParam(BUTTON_HARMONY_SETSTEP_16_PARAM, 0.f, 1.f, 0.f, "");

		configParam(BUTTON_CIRCLESTEP_C_PARAM, 0.f, 1.f, 1.f, "");
		configParam(BUTTON_CIRCLESTEP_G_PARAM, 0.f, 1.f, 0.f, "");
		configParam(BUTTON_CIRCLESTEP_D_PARAM, 0.f, 1.f, 0.f, "");
		configParam(BUTTON_CIRCLESTEP_A_PARAM, 0.f, 1.f, 0.f, "");
		configParam(BUTTON_CIRCLESTEP_E_PARAM, 0.f, 1.f, 0.f, "");
		configParam(BUTTON_CIRCLESTEP_B_PARAM, 0.f, 1.f, 0.f, "");
		configParam(BUTTON_CIRCLESTEP_GBFS_PARAM, 0.f, 1.f, 0.f, "");
		configParam(BUTTON_CIRCLESTEP_DB_PARAM, 0.f, 1.f, 0.f, "");
		configParam(BUTTON_CIRCLESTEP_AB_PARAM, 0.f, 1.f, 0.f, "");
		configParam(BUTTON_CIRCLESTEP_EB_PARAM, 0.f, 1.f, 0.f, "");
		configParam(BUTTON_CIRCLESTEP_BB_PARAM, 0.f, 1.f, 0.f, "");
		configParam(BUTTON_CIRCLESTEP_F_PARAM, 0.f, 1.f, 0.f, "");

	}  // end Meander()
	
};

 struct MeanderProgressionPresetLineDisplay : TransparentWidget {
	
	int frame = 0;
	std::shared_ptr<Font> font; 
	
	MeanderProgressionPresetLineDisplay() {
		font = APP->window->loadFont(asset::plugin(pluginInstance, "res/EurostileBold.ttf"));
	}

	void draw(const DrawArgs &ctx) override {

	
		Vec pos = Vec(-40, -3); // this is the offset if any in the passed box position, particularly x indention -7.3=box height
		nvgFontSize(ctx.vg, 14);
		nvgFontFaceId(ctx.vg, font->handle);
		nvgTextLetterSpacing(ctx.vg, -1);
		nvgFillColor(ctx.vg, nvgRGBA(0xFF, 0xFF, 0x2C, 0xFF));
		char text[128]; 
		snprintf(text, sizeof(text), "#%d:  %s", harmony_type, theActiveHarmonyType.harmony_type_desc);
		nvgText(ctx.vg, pos.x, pos.y, text, NULL);

		nvgFillColor(ctx.vg, nvgRGBA(0xFF, 0xFF, 0x2C, 0xFF));
		pos = Vec(120, -180); // this is the offset if any in the passed box position, particularly x indention -7.3=box height
		snprintf(text, sizeof(text), "%d",  theActiveHarmonyType.num_harmony_steps);
		nvgText(ctx.vg, pos.x, pos.y, text, NULL);

		nvgFillColor(ctx.vg, nvgRGBA(0x0, 0x0, 0x0, 0xFF));
		pos = Vec(40, -180); // this is the offset if any in the passed box position, particularly x indention -7.3=box height
		snprintf(text, sizeof(text), "[%d-%d]           ",  theActiveHarmonyType.min_steps, theActiveHarmonyType.max_steps);
		nvgText(ctx.vg, pos.x, pos.y, text, NULL);

		nvgFontSize(ctx.vg, 12);
		nvgFillColor(ctx.vg, nvgRGBA(0xFF, 0xFF, 0x2C, 0xFF));
		pos = Vec(-35, 26); // this is the offset if any in the passed box position, particularly x indention -7.3=box height
		snprintf(text, sizeof(text), "%s           ",  theActiveHarmonyType.harmony_degrees_desc);
		nvgText(ctx.vg, pos.x, pos.y, text, NULL);
	
	} 

};

struct RootKeySelectLineDisplay : TransparentWidget {
	
	int frame = 0;
	std::shared_ptr<Font> font;

	RootKeySelectLineDisplay() {
		font = APP->window->loadFont(asset::plugin(pluginInstance, "res/EurostileBold.ttf"));
	}

	void draw(const DrawArgs &ctx) override {

	//	Vec pos = Vec(18,-11); // this is the offset if any in the passed box position, particularly x indention -7.3=box height
		Vec pos = Vec(12,-11); // this is the offset if any in the passed box position, particularly x indention -7.3=box height
	
	
		nvgFontSize(ctx.vg,18 );
		nvgFontFaceId(ctx.vg, font->handle);
		nvgTextLetterSpacing(ctx.vg, -1);
		nvgTextAlign(ctx.vg,NVG_ALIGN_CENTER|NVG_ALIGN_MIDDLE);
		nvgFillColor(ctx.vg, nvgRGBA(0xff, 0xff, 0x2c, 0xFF));

		char text[128];
		
		snprintf(text, sizeof(text), "%s", root_key_names[root_key]);
		nvgText(ctx.vg, pos.x, pos.y, text, NULL);
	}

};

struct ScaleSelectLineDisplay : TransparentWidget {
	
	int frame = 0;
	std::shared_ptr<Font> font;

	ScaleSelectLineDisplay() {
		font = APP->window->loadFont(asset::plugin(pluginInstance, "res/EurostileBold.ttf"));
	}

	void draw(const DrawArgs &ctx) override {

		Vec pos = Vec(36,-20); // this is the offset if any in the passed box position, particularly x indention -7.3=box height
	 
		nvgFontSize(ctx.vg, 12);
		nvgFontFaceId(ctx.vg, font->handle);
		nvgTextLetterSpacing(ctx.vg, -1);
		nvgTextAlign(ctx.vg,NVG_ALIGN_CENTER|NVG_ALIGN_MIDDLE);
		nvgFillColor(ctx.vg, nvgRGBA(0xff, 0xff, 0x2c, 0xFF));
	
		char text[128];
		
		snprintf(text, sizeof(text), "%s", mode_names[mode]);
		nvgText(ctx.vg, pos.x, pos.y, text, NULL);

		// add on the scale notes display out of this box
		nvgFillColor(ctx.vg, nvgRGBA(0xff, 0xff, 0xff, 0xFF));
		nvgText(ctx.vg, pos.x-35, pos.y+39,"                        ", NULL);  // erase current content
		nvgFillColor(ctx.vg, nvgRGBA(0x00, 0x0, 0x0, 0xFF));
		strcpy(text,"");
		for (int i=0;i<mode_step_intervals[mode][0];++i)
		{
			strcat(text,note_desig[notes[i]%MAX_NOTES]);  
			strcat(text," ");
		}
		
		nvgText(ctx.vg, pos.x-35, pos.y+39, text, NULL);
		
	} 

};

////////////////////////////////////
struct BpmDisplayWidget : TransparentWidget {


  float *value = NULL;
  std::shared_ptr<Font> font;

  BpmDisplayWidget() {
    font = APP->window->loadFont(asset::plugin(pluginInstance, "res/Segment7Standard.ttf"));
  };

  void draw(const DrawArgs &args) override {

	  if (!value) {
      return;
    }
    
    // text 
    nvgFontSize(args.vg, 36);
    nvgFontFaceId(args.vg, font->handle);
    nvgTextLetterSpacing(args.vg, 2.3);

	std::stringstream to_display;   
    to_display << std::setw(3) << *value;

   	Vec textPos = Vec(6.0f, -6.0f);   // this is a relative offset within the widget box
  
	NVGcolor textColor = nvgRGB(0xff, 0xff, 0x2c);
	nvgFillColor(args.vg, textColor);
    nvgText(args.vg, textPos.x, textPos.y, to_display.str().c_str(), NULL);
  }
};
////////////////////////////////////
struct SigDisplayWidget : TransparentWidget {

  int *value = NULL;
  std::shared_ptr<Font> font;

  SigDisplayWidget() {
    font = APP->window->loadFont(asset::plugin(pluginInstance, "res/Segment7Standard.ttf"));
    
    
  };

  void draw(const DrawArgs &args) override {
	
    if (!value) {
      return;
    }
    
    // text 
  	nvgFontSize(args.vg, 18);
    nvgFontFaceId(args.vg, font->handle);
    nvgTextLetterSpacing(args.vg, 2.2);

    std::stringstream to_display;   
    to_display << std::setw(2) << *value;

 	Vec textPos = Vec(14.0f, 14.0f);   // this is a relative offset within the box
   
 	NVGcolor textColor = nvgRGBA(0xff, 0xff, 0x2c, 0xff);
    nvgFillColor(args.vg, textColor);
    nvgText(args.vg, textPos.x, textPos.y, to_display.str().c_str(), NULL);
  }
};
//////////////////////////////////

struct RSLabelCentered : LedDisplay {
	std::shared_ptr<Font> font;
	int fontSize;
	std::string text;
	NVGcolor color;

//	RSLabelCentered(int x, int y, const char* str = "", int fontSize = 10, const NVGcolor& colour = COLOR_RS_GREY) {
	RSLabelCentered(int x, int y, const char* str = "", int fontSize = 10, const NVGcolor& colour = nvgRGB(0x00, 0x00, 0x00)) {
	//	font = APP->window->loadFont(asset::plugin(pluginInstance, "res/fonts/Ubuntu Condensed 400.ttf"));
		font = APP->window->loadFont(asset::plugin(pluginInstance, "res/DejaVuSansMono.ttf"));
		this->fontSize = fontSize;
		box.pos = Vec(x, y);
		text = str;
		color = colour;
	}

	void draw(const DrawArgs &args) override {
		if(font->handle >= 0) {
			bndSetFont(font->handle);

			nvgFontSize(args.vg, fontSize);
			nvgFontFaceId(args.vg, font->handle);
			nvgTextLetterSpacing(args.vg, 0);
			nvgTextAlign(args.vg, NVG_ALIGN_CENTER);

			nvgBeginPath(args.vg);
			nvgFillColor(args.vg, color);
			nvgText(args.vg, 0, 0, text.c_str(), NULL);
			nvgStroke(args.vg);

			bndSetFont(APP->window->uiFont->handle);
		}
	}
};

///////////////////////////////
 

struct MeanderWidget : ModuleWidget 
{
//	Meander* module;  // KNC debugging
	struct CircleOf5thsDisplay : TransparentWidget 
	{
					
		int frame = 0;
		std::shared_ptr<Font> textfont;
		std::shared_ptr<Font> musicfont; 

		CircleOf5thsDisplay() 
		{
			textfont = APP->window->loadFont(asset::plugin(pluginInstance, "res/DejaVuSansMono.ttf"));
	    	musicfont = APP->window->loadFont(asset::plugin(pluginInstance, "res/Musisync-KVLZ.ttf"));
		}
 
	 	  

		void DrawCircle5ths(const DrawArgs &args, int root_key) 
		{
			DEBUG("DrawCircle5ths()");
			// clear panel area to opaque white to overwrite anything in the SVg
			NVGcolor whiteOpaque = nvgRGBA(250, 250, 250, 255);
				
			nvgBeginPath(args.vg);
			nvgCircle(args.vg, theCircleOf5ths.CircleCenter.x, theCircleOf5ths.CircleCenter.y, (theCircleOf5ths.OuterCircleRadius+2.0));  // clear background to +2mm radius of largest circle
			nvgFillColor(args.vg, whiteOpaque);
			nvgFill(args.vg);
			nvgStrokeColor(args.vg, whiteOpaque);
			nvgStrokeWidth(args.vg, 2.0);
			nvgStroke(args.vg);
			nvgClosePath(args.vg);

			for (int i=0; i<MAX_CIRCLE_STATIONS; ++i)
			{
					// draw root_key annulus sector

					int relativeCirclePosition = ((i - circle_root_key + mode)+12) % MAX_CIRCLE_STATIONS;
					DEBUG("\nrelativeCirclePosition-1=%d", relativeCirclePosition);

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
				
					nvgArc(args.vg,theCircleOf5ths.CircleCenter.x,theCircleOf5ths.CircleCenter.y,theCircleOf5ths.MiddleCircleRadius,theCircleOf5ths.Circle5ths[i].startDegree,theCircleOf5ths.Circle5ths[i].endDegree,NVG_CW);
					nvgLineTo(args.vg,theCircleOf5ths.Circle5ths[i].pt3.x,theCircleOf5ths.Circle5ths[i].pt3.y);
					nvgArc(args.vg,theCircleOf5ths.CircleCenter.x,theCircleOf5ths.CircleCenter.y,theCircleOf5ths.InnerCircleRadius,theCircleOf5ths.Circle5ths[i].endDegree,theCircleOf5ths.Circle5ths[i].startDegree,NVG_CCW);
					nvgLineTo(args.vg,theCircleOf5ths.Circle5ths[i].pt2.x,theCircleOf5ths.Circle5ths[i].pt2.y);
								
					nvgFill(args.vg);
					nvgStroke(args.vg);
					
					nvgClosePath(args.vg);	
								
					// draw text
					nvgFontSize(args.vg, 12);
					nvgFontFaceId(args.vg, textfont->handle);	
					nvgTextLetterSpacing(args.vg, -1);
					nvgFillColor(args.vg, nvgRGBA(0x00, 0x00, 0x00, 0xff));
					char text[32];
					snprintf(text, sizeof(text), "%s", CircleNoteNames[i]);
					DEBUG("radialDirection= %.3f %.3f", theCircleOf5ths.Circle5ths[i].radialDirection.x, theCircleOf5ths.Circle5ths[i].radialDirection.y);
					Vec TextPosition=theCircleOf5ths.CircleCenter.plus(theCircleOf5ths.Circle5ths[i].radialDirection.mult(theCircleOf5ths.MiddleCircleRadius*.93f));
					nvgTextAlign(args.vg,NVG_ALIGN_CENTER|NVG_ALIGN_MIDDLE);
					nvgText(args.vg, TextPosition.x, TextPosition.y, text, NULL);

			}		
		};

		void DrawDegreesSemicircle(const DrawArgs &args, int root_key) 
		{
			DEBUG("DrawDegreesSemicircle()");
			int chord_type=0;

			for (int i=0; i<MAX_HARMONIC_DEGREES; ++i)
			{
				// draw degree annulus sector

					nvgBeginPath(args.vg);
					float opacity = 128;

					nvgStrokeColor(args.vg, nvgRGBA(0x0, 0x0, 0x0, 0xff));
					nvgStrokeWidth(args.vg, 2);

					chord_type=theCircleOf5ths.theDegreeSemiCircle.degreeElements[i].chordType;

					if (false)
					{
						if (chord_type==0)  // major
							nvgFillColor(args.vg, nvgRGBA(0xff, 0x20, 0x20, (int)opacity));  // reddish
						else
						if (chord_type==1)  //minor
							nvgFillColor(args.vg, nvgRGBA(0x20, 0x20, 0xff, (int)opacity));  //bluish
						else
						if (chord_type==6)  // diminished
							nvgFillColor(args.vg, nvgRGBA(0x20, 0xff, 0x20, (int)opacity));  // greenish
					}
					else
						nvgFillColor(args.vg, nvgRGBA(0xf9, 0xf9, 0x20, (int)opacity));  // yellowish
						
					nvgArc(args.vg,theCircleOf5ths.CircleCenter.x,theCircleOf5ths.CircleCenter.y,theCircleOf5ths.OuterCircleRadius,theCircleOf5ths.theDegreeSemiCircle.degreeElements[i].startDegree,theCircleOf5ths.theDegreeSemiCircle.degreeElements[i].endDegree,NVG_CW);
					nvgLineTo(args.vg,theCircleOf5ths.theDegreeSemiCircle.degreeElements[i].pt3.x,theCircleOf5ths.theDegreeSemiCircle.degreeElements[i].pt3.y);
					nvgArc(args.vg,theCircleOf5ths.CircleCenter.x,theCircleOf5ths.CircleCenter.y,theCircleOf5ths.MiddleCircleRadius,theCircleOf5ths.theDegreeSemiCircle.degreeElements[i].endDegree,theCircleOf5ths.theDegreeSemiCircle.degreeElements[i].startDegree,NVG_CCW);
					nvgLineTo(args.vg,theCircleOf5ths.theDegreeSemiCircle.degreeElements[i].pt2.x,theCircleOf5ths.theDegreeSemiCircle.degreeElements[i].pt2.y);
					
					nvgFill(args.vg);
					nvgStroke(args.vg);
					
					nvgClosePath(args.vg);	
								
					// draw text
					nvgFontSize(args.vg, 10);
					nvgFontFaceId(args.vg, textfont->handle);	
					nvgTextLetterSpacing(args.vg, -1); // as close as possible
					nvgFillColor(args.vg, nvgRGBA(0x00, 0x00, 0x00, 0xff));
					char text[32];
				
					
					if (chord_type==0) // major
						snprintf(text, sizeof(text), "%s", circle_of_fifths_degrees_UC[(i - theCircleOf5ths.theDegreeSemiCircle.RootKeyCircle5thsPosition+7)%7]);
						else
					if ((chord_type==1)||(chord_type==6)) // minor or diminished
						snprintf(text, sizeof(text), "%s", circle_of_fifths_degrees_LC[(i - theCircleOf5ths.theDegreeSemiCircle.RootKeyCircle5thsPosition+7)%7]);
					
					DEBUG("radialDirection= %.3f %.3f", theCircleOf5ths.theDegreeSemiCircle.degreeElements[i].radialDirection.x, theCircleOf5ths.theDegreeSemiCircle.degreeElements[i].radialDirection.y);
					Vec TextPosition=theCircleOf5ths.CircleCenter.plus(theCircleOf5ths.theDegreeSemiCircle.degreeElements[i].radialDirection.mult(theCircleOf5ths.OuterCircleRadius*.92f));
					nvgTextAlign(args.vg,NVG_ALIGN_CENTER|NVG_ALIGN_MIDDLE);
					nvgText(args.vg, TextPosition.x, TextPosition.y, text, NULL);
					if (i==6) // draw diminished
					{
						Vec TextPositionBdim=Vec(TextPosition.x+9, TextPosition.y-4);
						sprintf(text, "o");
						DEBUG(text);
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

		void updatePanelText(const DrawArgs &args)
		{
			Vec pos;
			char text[128];
			nvgFontSize(args.vg, 17);
			nvgFontFaceId(args.vg, textfont->handle);
			nvgTextLetterSpacing(args.vg, -1);
			nvgFillColor(args.vg, nvgRGBA(0x0, 0x0, 0x0, 0xFF));

			// harmony params******

			pos=Vec(672, 61);  
			nvgFontSize(args.vg, 17);
			nvgFillColor(args.vg, nvgRGBA(0xFF, 0xFF, 0x2C, 0xFF));
			snprintf(text, sizeof(text), "%.1lf", theMeanderState.theHarmonyParms.volume);
			nvgText(args.vg, pos.x, pos.y, text, NULL);

			pos=Vec(672, 114);  
			nvgFontSize(args.vg, 17);
			nvgFillColor(args.vg, nvgRGBA(0xFF, 0xFF, 0x2C, 0xFF));
			snprintf(text, sizeof(text), "%d", (int)theMeanderState.theHarmonyParms.target_octave);
			nvgText(args.vg, pos.x, pos.y, text, NULL);

						
			pos=Vec(672, 139);  
			nvgFontSize(args.vg, 17);
			nvgFillColor(args.vg, nvgRGBA(0xFF, 0xFF, 0x2C, 0xFF));
			snprintf(text, sizeof(text), "%.1lf", theMeanderState.theHarmonyParms.alpha);
			nvgText(args.vg, pos.x, pos.y, text, NULL);

			pos=Vec(672, 163);  
			nvgFontSize(args.vg, 17);
			nvgFillColor(args.vg, nvgRGBA(0xFF, 0xFF, 0x2C, 0xFF));
			snprintf(text, sizeof(text), "%.1lf", theMeanderState.theHarmonyParms.note_octave_range);
			nvgText(args.vg, pos.x, pos.y, text, NULL);

			pos=Vec(672, 189);  
			nvgFontSize(args.vg, 17);
			nvgFillColor(args.vg, nvgRGBA(0xFF, 0xFF, 0x2C, 0xFF));
			snprintf(text, sizeof(text), "%d", (int)theMeanderState.theHarmonyParms.note_length_divisor);
			nvgText(args.vg, pos.x, pos.y, text, NULL);

			//**************

			pos=Vec(1177, 58);    
			nvgFontSize(args.vg, 17);
			nvgFillColor(args.vg, nvgRGBA(0xFF, 0xFF, 0x2C, 0xFF));
			snprintf(text, sizeof(text), "%d", (int)theMeanderState.theHarmonyParms.noctaves);
			nvgText(args.vg, pos.x, pos.y, text, NULL);

			pos=Vec(1177, 80);    
			nvgFontSize(args.vg, 17);
			nvgFillColor(args.vg, nvgRGBA(0xFF, 0xFF, 0x2C, 0xFF));
			snprintf(text, sizeof(text), "%d", (int)theMeanderState.theHarmonyParms.period);
			nvgText(args.vg, pos.x, pos.y, text, NULL);

			pos=Vec(1177, 104);    
			nvgFontSize(args.vg, 17);
			nvgFillColor(args.vg, nvgRGBA(0xFF, 0xFF, 0x2C, 0xFF));
			snprintf(text, sizeof(text), "%d", (int)theMeanderState.theMelodyParms.noctaves);
			nvgText(args.vg, pos.x, pos.y, text, NULL);

			pos=Vec(1177, 126);    
			nvgFontSize(args.vg, 17);
			nvgFillColor(args.vg, nvgRGBA(0xFF, 0xFF, 0x2C, 0xFF));
			snprintf(text, sizeof(text), "%d", (int)theMeanderState.theMelodyParms.period);
			nvgText(args.vg, pos.x, pos.y, text, NULL);

			pos=Vec(1177, 150);    
			nvgFontSize(args.vg, 17);
			nvgFillColor(args.vg, nvgRGBA(0xFF, 0xFF, 0x2C, 0xFF));
			snprintf(text, sizeof(text), "%d", (int)theMeanderState.theArpParms.noctaves);
			nvgText(args.vg, pos.x, pos.y, text, NULL);

			pos=Vec(1177, 172);    
			nvgFontSize(args.vg, 17);
			nvgFillColor(args.vg, nvgRGBA(0xFF, 0xFF, 0x2C, 0xFF));
			snprintf(text, sizeof(text), "%d", (int)theMeanderState.theArpParms.period);
			nvgText(args.vg, pos.x, pos.y, text, NULL);
    
			//*************

				

			pos=Vec(865, 63);  
			nvgFontSize(args.vg, 17);
			nvgFillColor(args.vg, nvgRGBA(0xFF, 0xFF, 0x2C, 0xFF));
			snprintf(text, sizeof(text), "%.1lf", theMeanderState.theMelodyParms.volume);
			nvgText(args.vg, pos.x, pos.y, text, NULL);

			pos=Vec(873, 100);  
			nvgFontSize(args.vg, 17);
			nvgFillColor(args.vg, nvgRGBA(0xFF, 0xFF, 0x2C, 0xFF));
			snprintf(text, sizeof(text), "%d", (int)theMeanderState.theMelodyParms.note_length_divisor);
			nvgText(args.vg, pos.x, pos.y, text, NULL);

			pos=Vec(872, 122);  
			nvgFontSize(args.vg, 17);
			nvgFillColor(args.vg, nvgRGBA(0xFF, 0xFF, 0x2C, 0xFF));
			snprintf(text, sizeof(text), "%d", (int)theMeanderState.theMelodyParms.target_octave);
			nvgText(args.vg, pos.x, pos.y, text, NULL);
			
			pos=Vec(865, 146);  
			nvgFontSize(args.vg, 17);
			nvgFillColor(args.vg, nvgRGBA(0xFF, 0xFF, 0x2C, 0xFF));
			snprintf(text, sizeof(text), "%.1lf", theMeanderState.theMelodyParms.alpha);
			nvgText(args.vg, pos.x, pos.y, text, NULL);

			pos=Vec(865, 166);  
			nvgFontSize(args.vg, 17);
			nvgFillColor(args.vg, nvgRGBA(0xFF, 0xFF, 0x2C, 0xFF));
			snprintf(text, sizeof(text), "%.1lf", theMeanderState.theMelodyParms.note_octave_range);
			nvgText(args.vg, pos.x, pos.y, text, NULL);
   

			// bass params**********

			pos=Vec(1020, 63);  
			nvgFontSize(args.vg, 17);
			snprintf(text, sizeof(text), "%.1lf", theMeanderState.theBassParms.volume);
			nvgText(args.vg, pos.x, pos.y, text, NULL);

			pos=Vec(1035, 85);  
			nvgFontSize(args.vg, 17);
			nvgFillColor(args.vg, nvgRGBA(0xFF, 0xFF, 0x2C, 0xFF));
			snprintf(text, sizeof(text), "%d", (int)theMeanderState.theBassParms.target_octave);
			nvgText(args.vg, pos.x, pos.y, text, NULL);

			pos=Vec(1035, 181);  
			nvgFontSize(args.vg, 17);
			nvgFillColor(args.vg, nvgRGBA(0xFF, 0xFF, 0x2C, 0xFF));
			snprintf(text, sizeof(text), "%d", (int)theMeanderState.theBassParms.note_length_divisor);
			nvgText(args.vg, pos.x, pos.y, text, NULL);

			// arp params************
			pos=Vec(865, 203);  
			nvgFontSize(args.vg, 17);
			nvgFillColor(args.vg, nvgRGBA(0xFF, 0xFF, 0x2C, 0xFF));
			snprintf(text, sizeof(text), "%d", theMeanderState.theArpParms.count);
			nvgText(args.vg, pos.x, pos.y, text, NULL);
 
			pos=Vec(865, 249);  
			nvgFontSize(args.vg, 17);
			nvgFillColor(args.vg, nvgRGBA(0xFF, 0xFF, 0x2C, 0xFF));
			snprintf(text, sizeof(text), "%d", theMeanderState.theArpParms.note_length_divisor);
			nvgText(args.vg, pos.x, pos.y, text, NULL);

			pos=Vec(865, 271);  
			nvgFontSize(args.vg, 17);
			nvgFillColor(args.vg, nvgRGBA(0xFF, 0xFF, 0x2C, 0xFF));
			snprintf(text, sizeof(text), "%.2f", theMeanderState.theArpParms.decay);
			nvgText(args.vg, pos.x, pos.y, text, NULL);

			pos=Vec(845, 295);  
			nvgFontSize(args.vg, 17);
			nvgFillColor(args.vg, nvgRGBA(0xFF, 0xFF, 0x2C, 0xFF));
			if (theMeanderState.theArpParms.pattern==0)
				snprintf(text, sizeof(text), "%d: 0-echo", theMeanderState.theArpParms.pattern);
			else
			if (theMeanderState.theArpParms.pattern==1)
				snprintf(text, sizeof(text), "%d: +1", theMeanderState.theArpParms.pattern);
			else
			if (theMeanderState.theArpParms.pattern==2)
				snprintf(text, sizeof(text), "%d: +1,-1", theMeanderState.theArpParms.pattern);
			else
			if (theMeanderState.theArpParms.pattern==3)
				snprintf(text, sizeof(text), "%d: +2", theMeanderState.theArpParms.pattern);
			else
			if (theMeanderState.theArpParms.pattern==-1)
				snprintf(text, sizeof(text), "%d: -1", theMeanderState.theArpParms.pattern);
			else
			if (theMeanderState.theArpParms.pattern==-2)
				snprintf(text, sizeof(text), "%d: -1,+1", theMeanderState.theArpParms.pattern);
			else
			if (theMeanderState.theArpParms.pattern==-3)
				snprintf(text, sizeof(text), "%d: -2", theMeanderState.theArpParms.pattern);
			
			nvgText(args.vg, pos.x, pos.y, text, NULL);

			//************************
			// circle area 
			
			float beginEdge = 295;
			float beginTop =115;
			float lineWidth=1.0; 
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
				nvgStrokeColor(args.vg, nvgRGB(0, 0, 0));
				nvgStrokeWidth(args.vg, lineWidth);
				nvgStroke(args.vg);
			}
			// draw staff lines
			nvgBeginPath(args.vg);
			for (int staff = 36, y = staff; y <= staff + 24; y += yLineSpacing) { 	
				nvgMoveTo(args.vg, beginEdge, beginTop+y);
				nvgLineTo(args.vg, beginEdge+stafflineLength, beginTop+y);
			}
			nvgStrokeColor(args.vg, nvgRGB(0x7f, 0x7f, 0x7f));
			nvgStrokeWidth(args.vg, lineWidth);
			nvgStroke(args.vg);

			nvgFontSize(args.vg, 45);
			nvgFontFaceId(args.vg, musicfont->handle);
			nvgTextLetterSpacing(args.vg, -1);
			nvgFillColor(args.vg, nvgRGBA(0x0, 0x0, 0x0, 0xFF));
			pos=Vec(beginEdge+10, beginTop+45);  
			snprintf(text, sizeof(text), "G");
			nvgText(args.vg, pos.x, pos.y, text, NULL);
			
			nvgFontSize(args.vg, 35);
			nvgTextAlign(args.vg,NVG_ALIGN_CENTER|NVG_ALIGN_MIDDLE);
			snprintf(text, sizeof(text), "B");
			
			int num_sharps1=0;
			int vertical_offset1=0;
			for (int i=0; i<7; ++i)
			{
				nvgBeginPath(args.vg);
				if (root_key_signatures_chromaticForder[notate_mode_as_signature_root_key][i]==1)
				{
					vertical_offset1=root_key_sharps_vertical_display_offset[num_sharps1];
					pos=Vec(beginEdge+20+(num_sharps1*5), beginTop+24+(vertical_offset1*yHalfLineSpacing));
					nvgText(args.vg, pos.x, pos.y, text, NULL);
					++num_sharps1;
				}
				nvgClosePath(args.vg);
			}	
		
			snprintf(text, sizeof(text), "b");
			int num_flats1=0;
			vertical_offset1=0;
			for (int i=6; i>=0; --i)
			{
				nvgBeginPath(args.vg);
				if (root_key_signatures_chromaticForder[notate_mode_as_signature_root_key][i]==-1)
				{
					vertical_offset1=root_key_flats_vertical_display_offset[num_flats1];
					pos=Vec(beginEdge+20+(num_flats1*5), beginTop+24+(vertical_offset1*yHalfLineSpacing));
					nvgText(args.vg, pos.x, pos.y, text, NULL);
					++num_flats1;
				}
				nvgClosePath(args.vg);
			}	

			nvgFontSize(args.vg, 12);
			nvgFontFaceId(args.vg, textfont->handle);
			nvgTextLetterSpacing(args.vg, -1);
			nvgFillColor(args.vg, nvgRGBA(0x0, 0x0, 0x0, 0xFF));

			pos=Vec(beginEdge+30, beginTop+105);  
			snprintf(text, sizeof(text), "In");
			nvgText(args.vg, pos.x, pos.y, text, NULL);

			pos=Vec(beginEdge+30, beginTop+135);  
			snprintf(text, sizeof(text), "In");
			nvgText(args.vg, pos.x, pos.y, text, NULL);

			pos=Vec(beginEdge+90, beginTop+105);  
			snprintf(text, sizeof(text), "Position");
			nvgText(args.vg, pos.x, pos.y, text, NULL);

			pos=Vec(beginEdge+80, beginTop+135);  
			snprintf(text, sizeof(text), "Gate");
			nvgText(args.vg, pos.x, pos.y, text, NULL);

			//************************

			// display area 

			// draw staff lines

			beginEdge = 890;
			beginTop =190;
			lineWidth=1.0; 
			
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
				nvgStrokeColor(args.vg, nvgRGB(0, 0, 0));
				nvgStrokeWidth(args.vg, lineWidth);
				nvgStroke(args.vg);
			}

			 // draw bar right vertical edge
			if (beginEdge > 0) {
				nvgBeginPath(args.vg);
				nvgMoveTo(args.vg, beginEdge+stafflineLength, beginTop+barLineVoffset);
				nvgLineTo(args.vg, beginEdge+stafflineLength, beginTop+(1.60*barLineVlength));
				nvgStrokeColor(args.vg, nvgRGB(0, 0, 0));
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

			nvgStrokeColor(args.vg, nvgRGB(0x7f, 0x7f, 0x7f));
			nvgStrokeWidth(args.vg, lineWidth);
			nvgStroke(args.vg);

			nvgFontSize(args.vg, 45);
			nvgFontFaceId(args.vg, musicfont->handle);
			nvgTextLetterSpacing(args.vg, -1);
			nvgFillColor(args.vg, nvgRGBA(0x0, 0x0, 0x0, 0xFF));
			pos=Vec(beginEdge+10, beginTop+45);  
			snprintf(text, sizeof(text), "G");  // treble clef
			nvgText(args.vg, pos.x, pos.y, text, NULL);

			nvgFontSize(args.vg, 36);
			pos=Vec(beginEdge+10, beginTop+80); 
			snprintf(text, sizeof(text), "?");   // bass clef
			nvgText(args.vg, pos.x, pos.y, text, NULL);
			
			nvgFontSize(args.vg, 40);
			pos=Vec(beginEdge+53, beginTop+33);
			snprintf(text, sizeof(text), "%d",time_sig_top);
			nvgText(args.vg, pos.x, pos.y, text, NULL);

			nvgFontSize(args.vg, 40);
			pos=Vec(beginEdge+53, beginTop+69);
			snprintf(text, sizeof(text), "%d",time_sig_top);  
			nvgText(args.vg, pos.x, pos.y, text, NULL);

			nvgFontSize(args.vg, 40);
			pos=Vec(beginEdge+53, beginTop+45);
			snprintf(text, sizeof(text), "%d",time_sig_bottom);
			nvgText(args.vg, pos.x, pos.y, text, NULL);

			nvgFontSize(args.vg, 40);
			pos=Vec(beginEdge+53, beginTop+81);
			snprintf(text, sizeof(text), "%d",time_sig_bottom);  
			nvgText(args.vg, pos.x, pos.y, text, NULL);

			// do root_key signature
			
			nvgFontSize(args.vg, 35);
			nvgTextAlign(args.vg,NVG_ALIGN_CENTER|NVG_ALIGN_MIDDLE);
			snprintf(text, sizeof(text), "B");  // # sharp
			
			num_sharps1=0;
			vertical_offset1=0;
			for (int i=0; i<7; ++i)
			{
				nvgBeginPath(args.vg);
				if (root_key_signatures_chromaticForder[notate_mode_as_signature_root_key][i]==1)
				{
					vertical_offset1=root_key_sharps_vertical_display_offset[num_sharps1];
					pos=Vec(beginEdge+20+(num_sharps1*5), beginTop+24+(vertical_offset1*yHalfLineSpacing));
					nvgText(args.vg, pos.x, pos.y, text, NULL);
					++num_sharps1;
				}
				nvgClosePath(args.vg);
			}	
		
			snprintf(text, sizeof(text), "b");  // b flat
			num_flats1=0;
			vertical_offset1=0;
			for (int i=6; i>=0; --i)  
			{
				nvgBeginPath(args.vg);
				if (root_key_signatures_chromaticForder[notate_mode_as_signature_root_key][i]==-1)
				{
					vertical_offset1=root_key_flats_vertical_display_offset[num_flats1];
					pos=Vec(beginEdge+20+(num_flats1*5), beginTop+24+(vertical_offset1*yHalfLineSpacing));
					nvgText(args.vg, pos.x, pos.y, text, NULL);
					++num_flats1;
				}
				nvgClosePath(args.vg);
			}	

			// now do for bass clef

			nvgFontSize(args.vg, 35);
			nvgTextAlign(args.vg,NVG_ALIGN_CENTER|NVG_ALIGN_MIDDLE);
			snprintf(text, sizeof(text), "B");  // # sharp
			
			num_sharps1=0;
			vertical_offset1=0;
			for (int i=0; i<7; ++i)
			{
				nvgBeginPath(args.vg);
				if (root_key_signatures_chromaticForder[notate_mode_as_signature_root_key][i]==1)
				{
					vertical_offset1=root_key_sharps_vertical_display_offset[num_sharps1];
					pos=Vec(beginEdge+20+(num_sharps1*5), beginTop+67+(vertical_offset1*yHalfLineSpacing));
					nvgText(args.vg, pos.x, pos.y, text, NULL);
					++num_sharps1;
				}
				nvgClosePath(args.vg);
			}	
		
			snprintf(text, sizeof(text), "b");  // b flat
			num_flats1=0;
			vertical_offset1=0;
			for (int i=6; i>=0; --i)
			{
				nvgBeginPath(args.vg);
				if (root_key_signatures_chromaticForder[notate_mode_as_signature_root_key][i]==-1)
				{
					vertical_offset1=root_key_flats_vertical_display_offset[num_flats1];
					pos=Vec(beginEdge+20+(num_flats1*5), beginTop+67+(vertical_offset1*yHalfLineSpacing));
					nvgText(args.vg, pos.x, pos.y, text, NULL);
					++num_flats1;
				}
				nvgClosePath(args.vg);
			}	
			

			//****************
		

			nvgFontSize(args.vg, 30);
			nvgFontFaceId(args.vg, musicfont->handle);
			nvgTextLetterSpacing(args.vg, -1);
			nvgFillColor(args.vg, nvgRGBA(0x0, 0x0, 0x0, 0xFF));
		
			float display_note_position=0; 

			for (int i=0; ((i<bar_note_count)&&(i<256)); ++i)
			{
				int display_note=played_notes_circular_buffer[i].note;
				DEBUG("display_note=%d %s", display_note, note_desig[display_note%12]);
			 
				int scale_note=0;
				if (strstr(note_desig[display_note%12],"C"))
					scale_note=0;
				else
				if (strstr(note_desig[display_note%12],"D"))
					scale_note=1;
				else
				if (strstr(note_desig[display_note%12],"E"))
					scale_note=2;
				else
				if (strstr(note_desig[display_note%12],"F"))
					scale_note=3;
				else
				if (strstr(note_desig[display_note%12],"G"))
					scale_note=4;
				else
				if (strstr(note_desig[display_note%12],"A"))
					scale_note=5;
				else
				if (strstr(note_desig[display_note%12],"B"))
					scale_note=6;
				DEBUG("scale_note=%d", scale_note%12);
			  
				int octave=(display_note/12)-2;
				DEBUG("octave=%d", octave);
			
				display_note_position = 108.0-(octave*21.0)-(scale_note*3.0)-7.5;
				DEBUG("display_note_position=%d", (int)display_note_position);
			
				
				float note_x_spacing= 230.0/(32*time_sig_top/time_sig_bottom);  // experimenting with note spacing function of time_signature.  barts_count_limit is not in scope, needs to be global
				pos=Vec(beginEdge+70+(played_notes_circular_buffer[i].time32s*note_x_spacing), beginTop+display_note_position);  
				if (true)  // color code notes in staff rendering
				{ 
					if (played_notes_circular_buffer[i].noteType==NOTE_TYPE_CHORD)
						nvgFillColor(args.vg, nvgRGBA(0xFF, 0x0, 0x0, 0xFF)); 
					else
					if (played_notes_circular_buffer[i].noteType==NOTE_TYPE_MELODY)
						nvgFillColor(args.vg, nvgRGBA(0x0, 0x0, 0x0, 0xFF)); 
					else
					if (played_notes_circular_buffer[i].noteType==NOTE_TYPE_ARP)
						nvgFillColor(args.vg, nvgRGBA(0x0, 0x0, 0xFF, 0xFF)); 
					else
					if (played_notes_circular_buffer[i].noteType==NOTE_TYPE_BASS)
						nvgFillColor(args.vg, nvgRGBA(0x0, 0xFF, 0x0, 0xFF)); 
					
				}

				
				nvgFontSize(args.vg, 30);
				if (played_notes_circular_buffer[i].length==1)
					snprintf(text, sizeof(text), "w");  // mnemonic W=whole, h=half, q-quarter, e=eighth, s=sixteenth notes
				else
				if (played_notes_circular_buffer[i].length==2)
					snprintf(text, sizeof(text), "h");  // mnemonic W=whole, h=half, q-quarter, e=eighth, s=sixteenth notes
				else
				if (played_notes_circular_buffer[i].length==4)
					snprintf(text, sizeof(text), "q");  // mnemonic W=whole, h=half, q-quarter, e=eighth, s=sixteenth notes
				else
				if (played_notes_circular_buffer[i].length==8)
					snprintf(text, sizeof(text), "e");  // mnemonic W=whole, h=half, q-quarter, e=eighth, s=sixteenth notes
				else
				if (played_notes_circular_buffer[i].length==16)
					snprintf(text, sizeof(text), "s");  // mnemonic W=whole, h=half, q-quarter, e=eighth, s=sixteenth notes
				else
				if (played_notes_circular_buffer[i].length==32)
					snprintf(text, sizeof(text), "s");  // mnemonic W=whole, h=half, q-quarter, e=eighth, s=sixteenth notes
				nvgText(args.vg, pos.x, pos.y, text, NULL);

				if (played_notes_circular_buffer[i].length==32)  // do overstrike for 1/32 symbol
				{
					nvgFontSize(args.vg, 15);
					snprintf(text, sizeof(text), "e");  // mnemonic W=whole, h=half, q-quarter, e=eighth, s=sixteenth notes
					nvgText(args.vg, pos.x-.5, pos.y+4.5, text, NULL);
				
				}


				if (((scale_note==5)&&(octave==3))  //A3
				  ||((scale_note==0)&&(octave==4))  //C4
				  ||((scale_note==0)&&(octave==2))  //C2
				  ||((scale_note==2)&&(octave==0))  //E0 
				  ||((scale_note==0)&&(octave==0))) //C0 
				{
					nvgFontSize(args.vg, 30);
					pos.x -= 2.0;
					pos.y -= 4.4;
					snprintf(text, sizeof(text), "_");  
					nvgText(args.vg, pos.x, pos.y, text, NULL);
				} 
			}

			
			//*********************

			nvgFontSize(args.vg, 14);
			nvgFontFaceId(args.vg, textfont->handle);
			nvgTextLetterSpacing(args.vg, -1);
			nvgFillColor(args.vg, nvgRGBA(0x0, 0x0, 0x0, 0xFF));
			 
			// write last melody note played 
			pos=convertSVGtoNVG(261.4, 120.3, 12.1, 6.5);  // X,Y,W,H in Inkscape mm units
			snprintf(text, sizeof(text), "%s%d", note_desig[(theMeanderState.theMelodyParms.last[0].note%12)], (int)(theMeanderState.theMelodyParms.last[0].note/12 ));
			nvgText(args.vg, pos.x, pos.y, text, NULL);

			
			// write last harmony note played 1
			pos=convertSVGtoNVG(187.8, 119.8, 12.1, 6.5);  // X,Y,W,H in Inkscape mm units
			snprintf(text, sizeof(text), "%s%d", note_desig[(theMeanderState. theHarmonyParms.last[0].note%12)] , theMeanderState. theHarmonyParms.last[0].note/12);
			nvgText(args.vg, pos.x, pos.y, text, NULL);

			// write last harmony note played 2
			pos=convertSVGtoNVG(199.1, 119.8, 12.1, 6.5);  // X,Y,W,H in Inkscape mm units
			snprintf(text, sizeof(text), "%s%d", note_desig[(theMeanderState. theHarmonyParms.last[1].note%12)], theMeanderState. theHarmonyParms.last[1].note/12);
			nvgText(args.vg, pos.x, pos.y, text, NULL);

			// write last harmony note played 3
			pos=convertSVGtoNVG(210.4, 119.8, 12.1, 6.5);  // X,Y,W,H in Inkscape mm units
			snprintf(text, sizeof(text), "%s%d", note_desig[(theMeanderState. theHarmonyParms.last[2].note%12)], theMeanderState. theHarmonyParms.last[2].note/12);
			nvgText(args.vg, pos.x, pos.y, text, NULL);

			// write last harmony note played 4
			pos=convertSVGtoNVG(221.7, 119.8, 12.1, 6.5);  // X,Y,W,H in Inkscape mm units
			snprintf(text, sizeof(text), "%s%d", note_desig[(theMeanderState. theHarmonyParms.last[3].note%12)], theMeanderState. theHarmonyParms.last[3].note/12);
					
			// write last bass note played 
			pos=convertSVGtoNVG(319.1, 121.0, 12.1, 6.5);  // X,Y,W,H in Inkscape mm units
			snprintf(text, sizeof(text), "%s%d", note_desig[(theMeanderState.theBassParms.last[0].note%12)], (theMeanderState.theBassParms.last[0].note/12));
			nvgText(args.vg, pos.x, pos.y, text, NULL);

			// write last octave bass note played 
			if (theMeanderState.theBassParms.octave_enabled)
			{
				pos=convertSVGtoNVG(330.1, 121.0, 12.1, 6.5);  // X,Y,W,H in Inkscape mm units
				snprintf(text, sizeof(text), "%s%d", note_desig[(theMeanderState.theBassParms.last[1].note%12)], (theMeanderState.theBassParms.last[1].note/12));
				nvgText(args.vg, pos.x, pos.y, text, NULL);
			}

			int last_chord_root=theMeanderState.last_harmony_chord_root_note%12;
			int last_chord_bass_note=theMeanderState.theHarmonyParms.last[0].note%12;
			pos=convertSVGtoNVG(110, 60, 12.1, 6.5);  // X,Y,W,H in Inkscape mm units
			nvgFontSize(args.vg, 30);

			char chord_type_desc[16];
			if (theMeanderState.theHarmonyParms.last_chord_type==0)
				strcpy(chord_type_desc, "");
			if (theMeanderState.theHarmonyParms.last_chord_type==1)
				strcpy(chord_type_desc, "m");
			if (theMeanderState.theHarmonyParms.last_chord_type==3)
				strcpy(chord_type_desc, "7");
			if (theMeanderState.theHarmonyParms.last_chord_type==4)
				strcpy(chord_type_desc, "m7");
			if (theMeanderState.theHarmonyParms.last_chord_type==5)
				strcpy(chord_type_desc, "dim7");
			if (theMeanderState.theHarmonyParms.last_chord_type==6)
				strcpy(chord_type_desc, "dim");

			if (last_chord_bass_note!=last_chord_root) 
				snprintf(text, sizeof(text), "%s%s/%s", note_desig[last_chord_root], chord_type_desc, note_desig[last_chord_bass_note]);
			else
				snprintf(text, sizeof(text), "%s%s", note_desig[last_chord_root], chord_type_desc);

			nvgText(args.vg, pos.x, pos.y, text, NULL);

					
			DEBUG("UpdatePanel()-end");
		}

	
		void draw(const DrawArgs &args) override 
		{
			DrawCircle5ths(args, root_key);  // has to be done each frame as panel redraws as SVG and needs to be blanked and cirecles redrawn
			DrawDegreesSemicircle(args,  root_key);
			updatePanelText(args);
		}

		
	
	};  // end struct CircleOf5thsDisplay

	MeanderWidget(Meander* module) 
	{ 
		DEBUG("MeanderWidget()");
		setModule(module);
	//	this->module = module;  // KNC debugging
	
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Meander.svg")));
					
		rack::random::init();  // must be called per thread
	
		 if (true)   // must be executed in order to see ModuleWidget panel display in preview, module* is checked for null below as it is null in browser preview
		 {
			if ((module) &&(!module->instanceRunning)) 
			{
				box.size.x = mm2px(5.08 * 32);
				int middle = box.size.x / 2 + 7;
				addChild(new RSLabelCentered(middle+97, (box.size.y / 2)-20, "DISABLED", 28));	
				addChild(new RSLabelCentered(middle+97, (box.size.y / 2)+50, "DISABLED", 28));	
				addChild(new RSLabelCentered(middle+97, (box.size.y / 2) -11, "ONLY ONE INSTANCE OF MEANDER REQUIRED",7));
				addChild(new RSLabelCentered(middle+97, (box.size.y / 2) +20, "ONLY ONE INSTANCE OF MEANDER REQUIRED",7));
				return;
			}

			MeanderProgressionPresetLineDisplay *ProgressionPresetDisplay = createWidget<MeanderProgressionPresetLineDisplay>(mm2px(Vec(186.1, (128.93-37.5))));  // From SVG file  186.1, (panelheight - boxY - boxheight)
			ProgressionPresetDisplay->box.size = mm2px(Vec(46, 7.3));  // from SVG file
			addChild(ProgressionPresetDisplay);

			
			RootKeySelectLineDisplay *MeanderRootKeySelectDisplay = createWidget<RootKeySelectLineDisplay>(mm2px(Vec(39.0, (128.93-56.5))));  // From SVG file  186.1, (panelheight - boxY - boxheight)
			MeanderRootKeySelectDisplay->box.size = mm2px(Vec(13.8, 7.2));  // from SVG file
			addChild(MeanderRootKeySelectDisplay);

			ScaleSelectLineDisplay *MeanderScaleSelectDisplay = createWidget<ScaleSelectLineDisplay>(mm2px(Vec(26.5, (128.93-42.9))));  // From SVG file  186.1, (panelheight - boxY - boxheight)
			MeanderScaleSelectDisplay->box.size = mm2px(Vec(13.8, 7.2));  // from SVG file
			addChild(MeanderScaleSelectDisplay);

			CircleOf5thsDisplay *display = new CircleOf5thsDisplay();
					
			display->box.pos = Vec(0, 0);
			display->box.size = Vec(box.size.x, box.size.y);
			addChild(display);
			
			addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
			addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
			addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
			addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

			//BPM DISPLAY  
			BpmDisplayWidget *BPMdisplay = new BpmDisplayWidget();
			BPMdisplay->box.pos = mm2px(Vec(12.8,42.5));
			BPMdisplay->box.size = mm2px(Vec(38.0, 13.9));
			if (module) 
				BPMdisplay->value = &module->tempo;
			addChild(BPMdisplay); 
			
			//SIG TOP DISPLAY 
			SigDisplayWidget *SigTopDisplay = new SigDisplayWidget();
			SigTopDisplay->box.pos = mm2px(Vec(42.2-4.5,50.9-5.6+3.0));
			SigTopDisplay->box.size = mm2px(Vec(8.3, 5.6));
			SigTopDisplay->value = &time_sig_top;
					
			addChild(SigTopDisplay);
			//SIG TOP KNOB
		
			//SIG BOTTOM DISPLAY    
			SigDisplayWidget *SigBottomDisplay = new SigDisplayWidget();
			SigBottomDisplay->box.pos = mm2px(Vec(42.2-4.5,56-5.6+3.0));
			SigBottomDisplay->box.size = mm2px(Vec(8.3, 5.6));
			SigBottomDisplay->value = &time_sig_bottom;
					
			addChild(SigBottomDisplay);
			
			int CircleOf5thsOuterButtonindex=0;
			int CircleOf5thsOuterButtonLightindex=0;
			int CircleOf5thsInnerLightindex=0;  
			int CircleOf5thsStepSelectButtonindex=0;  
			int CircleOf5thsStepSelectButtonLightindex=0;  

	//*************   Note: Each LEDButton needs its light and that light needs a unique ID, needs to be added to an array and then needs to be repositioned along with the button.  Also needs to be enumed with other lights so lights[] picks it up.
			CircleOf5thsOuterButton[CircleOf5thsOuterButtonindex]=createParamCentered<LEDButton>(mm2px(Vec(116.227, 37.257)), module, Meander::BUTTON_CIRCLESTEP_C_PARAM);
			addParam(CircleOf5thsOuterButton[CircleOf5thsOuterButtonindex++]);
			CircleOf5thsOuterButtonLight[CircleOf5thsOuterButtonLightindex]=createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(116.227, 37.257)), module, Meander::LIGHT_LEDBUTTON_CIRCLESTEP_1);
			addChild(CircleOf5thsOuterButtonLight[CircleOf5thsOuterButtonLightindex++]); 

			CircleOf5thsOuterButton[CircleOf5thsOuterButtonindex]=createParamCentered<LEDButton>(mm2px(Vec(132.479, 41.32)), module, Meander::BUTTON_CIRCLESTEP_G_PARAM);
			addParam(CircleOf5thsOuterButton[CircleOf5thsOuterButtonindex++]);
			CircleOf5thsOuterButtonLight[CircleOf5thsOuterButtonLightindex]=createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(132.479, 41.32)), module, Meander::LIGHT_LEDBUTTON_CIRCLESTEP_2);
			addChild(CircleOf5thsOuterButtonLight[CircleOf5thsOuterButtonLightindex++]);  


			CircleOf5thsOuterButton[CircleOf5thsOuterButtonindex]=createParamCentered<LEDButton>(mm2px(Vec(143.163, 52.155)), module, Meander::BUTTON_CIRCLESTEP_D_PARAM);
			addParam(CircleOf5thsOuterButton[CircleOf5thsOuterButtonindex++]);
			CircleOf5thsOuterButtonLight[CircleOf5thsOuterButtonLightindex]=createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(143.163, 52.155)), module, Meander::LIGHT_LEDBUTTON_CIRCLESTEP_3);
			addChild(CircleOf5thsOuterButtonLight[CircleOf5thsOuterButtonLightindex++]);  

			CircleOf5thsOuterButton[CircleOf5thsOuterButtonindex]=createParamCentered<LEDButton>(mm2px(Vec(147.527, 67.353)), module, Meander::BUTTON_CIRCLESTEP_A_PARAM);
			addParam(CircleOf5thsOuterButton[CircleOf5thsOuterButtonindex++]);
			CircleOf5thsOuterButtonLight[CircleOf5thsOuterButtonLightindex]=createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(147.527, 67.353)), module, Meander::LIGHT_LEDBUTTON_CIRCLESTEP_4);
			addChild(CircleOf5thsOuterButtonLight[CircleOf5thsOuterButtonLightindex++]);  

			CircleOf5thsOuterButton[CircleOf5thsOuterButtonindex]=createParamCentered<LEDButton>(mm2px(Vec(141.96, 83.906)), module, Meander::BUTTON_CIRCLESTEP_E_PARAM);
			addParam(CircleOf5thsOuterButton[CircleOf5thsOuterButtonindex++]);
			CircleOf5thsOuterButtonLight[CircleOf5thsOuterButtonLightindex]=createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(141.96, 83.906)), module, Meander::LIGHT_LEDBUTTON_CIRCLESTEP_5);
			addChild(CircleOf5thsOuterButtonLight[CircleOf5thsOuterButtonLightindex++]);  

			CircleOf5thsOuterButton[CircleOf5thsOuterButtonindex]=createParamCentered<LEDButton>(mm2px(Vec(132.931, 94.44)), module, Meander::BUTTON_CIRCLESTEP_B_PARAM);
			addParam(CircleOf5thsOuterButton[CircleOf5thsOuterButtonindex++]);
			CircleOf5thsOuterButtonLight[CircleOf5thsOuterButtonLightindex]=createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(132.931, 94.44)), module, Meander::LIGHT_LEDBUTTON_CIRCLESTEP_6);
			addChild(CircleOf5thsOuterButtonLight[CircleOf5thsOuterButtonLightindex++]);  

			CircleOf5thsOuterButton[CircleOf5thsOuterButtonindex]=createParamCentered<LEDButton>(mm2px(Vec(116.378, 98.804)), module, Meander::BUTTON_CIRCLESTEP_GBFS_PARAM);
			addParam(CircleOf5thsOuterButton[CircleOf5thsOuterButtonindex++]);
			CircleOf5thsOuterButtonLight[CircleOf5thsOuterButtonLightindex]=createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(116.378, 98.804)), module, Meander::LIGHT_LEDBUTTON_CIRCLESTEP_7);
			addChild(CircleOf5thsOuterButtonLight[CircleOf5thsOuterButtonLightindex++]);  

			CircleOf5thsOuterButton[CircleOf5thsOuterButtonindex]=createParamCentered<LEDButton>(mm2px(Vec(101.029, 93.988)), module, Meander::BUTTON_CIRCLESTEP_DB_PARAM);
			addParam(CircleOf5thsOuterButton[CircleOf5thsOuterButtonindex++]);
			CircleOf5thsOuterButtonLight[CircleOf5thsOuterButtonLightindex]=createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(101.029, 93.988)), module, Meander::LIGHT_LEDBUTTON_CIRCLESTEP_8);
			addChild(CircleOf5thsOuterButtonLight[CircleOf5thsOuterButtonLightindex++]);  

			CircleOf5thsOuterButton[CircleOf5thsOuterButtonindex]=createParamCentered<LEDButton>(mm2px(Vec(91.097, 83.906)), module, Meander::BUTTON_CIRCLESTEP_AB_PARAM);
			addParam(CircleOf5thsOuterButton[CircleOf5thsOuterButtonindex++]);
			CircleOf5thsOuterButtonLight[CircleOf5thsOuterButtonLightindex]=createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(91.097, 83.906)), module, Meander::LIGHT_LEDBUTTON_CIRCLESTEP_9);
			addChild(CircleOf5thsOuterButtonLight[CircleOf5thsOuterButtonLightindex++]);  

			CircleOf5thsOuterButton[CircleOf5thsOuterButtonindex]=createParamCentered<LEDButton>(mm2px(Vec(86.282, 68.106)), module, Meander::BUTTON_CIRCLESTEP_EB_PARAM);
			addParam(CircleOf5thsOuterButton[CircleOf5thsOuterButtonindex++]);
			CircleOf5thsOuterButtonLight[CircleOf5thsOuterButtonLightindex]=createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(86.282, 68.106)), module, Meander::LIGHT_LEDBUTTON_CIRCLESTEP_10);
			addChild(CircleOf5thsOuterButtonLight[CircleOf5thsOuterButtonLightindex++]); 

			CircleOf5thsOuterButton[CircleOf5thsOuterButtonindex]=createParamCentered<LEDButton>(mm2px(Vec(89.743, 52.004)), module, Meander::BUTTON_CIRCLESTEP_BB_PARAM);
			addParam(CircleOf5thsOuterButton[CircleOf5thsOuterButtonindex++]);
			CircleOf5thsOuterButtonLight[CircleOf5thsOuterButtonLightindex]=createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(89.743, 52.004)), module, Meander::LIGHT_LEDBUTTON_CIRCLESTEP_11);
			addChild(CircleOf5thsOuterButtonLight[CircleOf5thsOuterButtonLightindex++]);  

			CircleOf5thsOuterButton[CircleOf5thsOuterButtonindex]=createParamCentered<LEDButton>(mm2px(Vec(101.781, 40.568)), module, Meander::BUTTON_CIRCLESTEP_F_PARAM);
			addParam(CircleOf5thsOuterButton[CircleOf5thsOuterButtonindex++]);
			CircleOf5thsOuterButtonLight[CircleOf5thsOuterButtonLightindex]=createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(101.781, 40.568)), module, Meander::LIGHT_LEDBUTTON_CIRCLESTEP_12);
			addChild(CircleOf5thsOuterButtonLight[CircleOf5thsOuterButtonLightindex++]);  

	
	//*************
			CircleOf5thsInnerLight[CircleOf5thsInnerLightindex]=createLightCentered<MediumLight<RedLight>>(mm2px(Vec(116.227, 43.878)), module, Meander::LIGHT_CIRCLE_ROOT_KEY_POSITION_1_LIGHT);
			addChild(CircleOf5thsInnerLight[CircleOf5thsInnerLightindex++]);
			CircleOf5thsInnerLight[CircleOf5thsInnerLightindex]=createLightCentered<MediumLight<RedLight>>(mm2px(Vec(129.018, 47.189)), module, Meander::LIGHT_CIRCLE_ROOT_KEY_POSITION_2_LIGHT);
			addChild(CircleOf5thsInnerLight[CircleOf5thsInnerLightindex++]);
			CircleOf5thsInnerLight[CircleOf5thsInnerLightindex]=createLightCentered<MediumLight<RedLight>>(mm2px(Vec(137.295, 56.067)), module, Meander::LIGHT_CIRCLE_ROOT_KEY_POSITION_3_LIGHT);
			addChild(CircleOf5thsInnerLight[CircleOf5thsInnerLightindex++]);
			CircleOf5thsInnerLight[CircleOf5thsInnerLightindex]=createLightCentered<MediumLight<RedLight>>(mm2px(Vec(140.455, 67.654)), module, Meander::LIGHT_CIRCLE_ROOT_KEY_POSITION_4_LIGHT);
			addChild(CircleOf5thsInnerLight[CircleOf5thsInnerLightindex++]);
			CircleOf5thsInnerLight[CircleOf5thsInnerLightindex]=createLightCentered<MediumLight<RedLight>>(mm2px(Vec(137.144, 80.295)), module, Meander::LIGHT_CIRCLE_ROOT_KEY_POSITION_5_LIGHT);
			addChild(CircleOf5thsInnerLight[CircleOf5thsInnerLightindex++]);
			CircleOf5thsInnerLight[CircleOf5thsInnerLightindex]=createLightCentered<MediumLight<RedLight>>(mm2px(Vec(128.868, 88.571)), module, Meander::LIGHT_CIRCLE_ROOT_KEY_POSITION_6_LIGHT);
			addChild(CircleOf5thsInnerLight[CircleOf5thsInnerLightindex++]);
			CircleOf5thsInnerLight[CircleOf5thsInnerLightindex]=createLightCentered<MediumLight<RedLight>>(mm2px(Vec(116.077, 92.183)), module, Meander::LIGHT_CIRCLE_ROOT_KEY_POSITION_7_LIGHT);
			addChild(CircleOf5thsInnerLight[CircleOf5thsInnerLightindex++]);
			CircleOf5thsInnerLight[CircleOf5thsInnerLightindex]=createLightCentered<MediumLight<RedLight>>(mm2px(Vec(104.791, 88.872)), module, Meander::LIGHT_CIRCLE_ROOT_KEY_POSITION_8_LIGHT);
			addChild(CircleOf5thsInnerLight[CircleOf5thsInnerLightindex++]);
			CircleOf5thsInnerLight[CircleOf5thsInnerLightindex]=createLightCentered<MediumLight<RedLight>>(mm2px(Vec(96.213, 80.596)), module, Meander::LIGHT_CIRCLE_ROOT_KEY_POSITION_9_LIGHT);
			addChild(CircleOf5thsInnerLight[CircleOf5thsInnerLightindex++]);
			CircleOf5thsInnerLight[CircleOf5thsInnerLightindex]=createLightCentered<MediumLight<RedLight>>(mm2px(Vec(92.602, 67.654)), module, Meander::LIGHT_CIRCLE_ROOT_KEY_POSITION_10_LIGHT);
			addChild(CircleOf5thsInnerLight[CircleOf5thsInnerLightindex++]);
			CircleOf5thsInnerLight[CircleOf5thsInnerLightindex]=createLightCentered<MediumLight<RedLight>>(mm2px(Vec(95.912, 55.465)), module, Meander::LIGHT_CIRCLE_ROOT_KEY_POSITION_11_LIGHT);
			addChild(CircleOf5thsInnerLight[CircleOf5thsInnerLightindex++]);
			CircleOf5thsInnerLight[CircleOf5thsInnerLightindex]=createLightCentered<MediumLight<RedLight>>(mm2px(Vec(105.393, 46.587)), module, Meander::LIGHT_CIRCLE_ROOT_KEY_POSITION_12_LIGHT);
			addChild(CircleOf5thsInnerLight[CircleOf5thsInnerLightindex++]); 
			 		


	//*************


			addParam(createParamCentered<LEDButton>(mm2px(Vec(19.7, 10.45)), module, Meander::BUTTON_RUN_PARAM));
			addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(19.7, 10.45)), module, Meander::LIGHT_LEDBUTTON_RUN));
					
			addParam(createParamCentered<LEDButton>(mm2px(Vec(19.7, 22.55)), module, Meander::BUTTON_RESET_PARAM));
			addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(19.7, 22.55)), module, Meander::LIGHT_LEDBUTTON_RESET));

			addParam(createParamCentered<Trimpot>(mm2px(Vec(8.12, 35.4)), module, Meander::CONTROL_TEMPOBPM_PARAM));
			addParam(createParamCentered<Trimpot>(mm2px(Vec(8.12, 47.322+3.0)), module, Meander::CONTROL_TIMESIGNATURETOP_PARAM));
			addParam(createParamCentered<Trimpot>(mm2px(Vec(8.12, 54.84+3.0)), module, Meander::CONTROL_TIMESIGNATUREBOTTOM_PARAM));
		
			{
				auto w= createParamCentered<Trimpot>(mm2px(Vec(8.12, 68.179)), module, Meander::CONTROL_ROOT_KEY_PARAM);
				w->snap=true;
				addParam(w); 
			}
			
			{
				auto w= createParamCentered<Trimpot>(mm2px(Vec(8.12, 78.675)), module, Meander::CONTROL_SCALE_PARAM);
				w->snap=true;
				addParam(w); 
			}

			addParam(createParamCentered<LEDButton>(mm2px(Vec(173.849, 12.622)), module, Meander::BUTTON_ENABLE_HARMONY_PARAM));
			addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(173.849, 12.622)), module, Meander::LIGHT_LEDBUTTON_HARMONY_ENABLE));
			addParam(createParamCentered<Trimpot>(mm2px(Vec(173.849, 20.384)), module, Meander::CONTROL_HARMONY_VOLUME_PARAM));
		
			{
				auto w= createParamCentered<Trimpot>(mm2px(Vec(174.028, 28)), module, Meander::CONTROL_HARMONY_STEPS_PARAM);
				w->snap=true;
				addParam(w); 
			}
	
			{
				auto w= createParamCentered<Trimpot>(mm2px(Vec(174.01, 37.396)), module, Meander::CONTROL_HARMONY_TARGETOCTAVE_PARAM);
				w->snap=true;
				addParam(w); 
			}
			addParam(createParamCentered<Trimpot>(mm2px(Vec(174.27, 45.982)), module, Meander::CONTROL_HARMONY_ALPHA_PARAM));
			addParam(createParamCentered<Trimpot>(mm2px(Vec(173.991, 53.788)), module, Meander::CONTROL_HARMONY_RANGE_PARAM));
			  
			{
				auto w= createParamCentered<Trimpot>(mm2px(Vec(173.953, 62.114)), module, Meander::CONTROL_HARMONY_DIVISOR_PARAM);
				w->snap=true;
				addParam(w); 
			} 

			addParam(createParamCentered<LEDButton>(mm2px(Vec(173.849, 69)), module, Meander::BUTTON_ENABLE_HARMONY_ALL7THS_PARAM));
			addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(173.849, 69)), module, Meander::LIGHT_LEDBUTTON_ENABLE_HARMONY_ALL7THS_PARAM));
			addParam(createParamCentered<LEDButton>(mm2px(Vec(203.849, 69)), module, Meander::BUTTON_ENABLE_HARMONY_V7THS_PARAM));
			addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(203.849, 69)), module, Meander::LIGHT_LEDBUTTON_ENABLE_HARMONY_V7THS_PARAM));

						
			addParam(createParamCentered<LEDButton>(mm2px(Vec(173.849, 75)), module, Meander::BUTTON_ENABLE_HARMONY_STACCATO_PARAM));
			addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(173.849, 75)), module, Meander::LIGHT_LEDBUTTON_ENABLE_HARMONY_STACCATO_PARAM));

		
			{
				auto w= createParamCentered<Trimpot>(mm2px(Vec(174.027, 81.524)), module, Meander::CONTROL_HARMONYPRESETS_PARAM);
				w->snap=true;
				addParam(w); 
			} 
 
			addParam(createParamCentered<LEDButton>(mm2px(Vec(240.353, 10.986)), module, Meander::BUTTON_ENABLE_MELODY_PARAM));
			addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(240.353, 10.986)), module, Meander::LIGHT_LEDBUTTON_MELODY_ENABLE));

			addParam(createParamCentered<LEDButton>(mm2px(Vec(270.353, 10.986)), module, Meander::BUTTON_ENABLE_MELODY_CHORDAL_PARAM));
			addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(270.274, 10.986)), module, Meander::LIGHT_LEDBUTTON_MELODY_ENABLE_CHORDAL));
			addParam(createParamCentered<LEDButton>(mm2px(Vec(287.274, 10.986)), module, Meander::BUTTON_ENABLE_MELODY_SCALER_PARAM));
			addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(287.274, 10.986)), module, Meander::LIGHT_LEDBUTTON_MELODY_ENABLE_SCALER));

			addParam(createParamCentered<LEDButton>(mm2px(Vec(240.409, 25.524)), module, Meander::BUTTON_MELODY_DESTUTTER_PARAM));
			addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(240.409, 25.524)), module, Meander::LIGHT_LEDBUTTON_MELODY_DESTUTTER));
			addParam(createParamCentered<Trimpot>(mm2px(Vec(240.353, 19.217)), module, Meander::CONTROL_MELODY_VOLUME_PARAM));
 		
			{
				auto w= createParamCentered<Trimpot>(mm2px(Vec(240.353, 32.217)), module, Meander::CONTROL_MELODY_NOTE_LENGTH_DIVISOR_PARAM);
				w->snap=true;
				addParam(w); 
			}
		
			{
				auto w= createParamCentered<Trimpot>(mm2px(Vec(240.353, 39.217)), module, Meander::CONTROL_MELODY_TARGETOCTAVE_PARAM);
				w->snap=true;
				addParam(w); 
			}
			addParam(createParamCentered<Trimpot>(mm2px(Vec(240.334, 47.803)), module, Meander::CONTROL_MELODY_ALPHA_PARAM));
			addParam(createParamCentered<Trimpot>(mm2px(Vec(240.334, 55.33)), module, Meander::CONTROL_MELODY_RANGE_PARAM));

			addParam(createParamCentered<LEDButton>(mm2px(Vec(305, 10.378)), module, Meander::BUTTON_ENABLE_BASS_PARAM));
			addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(305, 10.378)), module, Meander::LIGHT_LEDBUTTON_BASS_ENABLE));
			addParam(createParamCentered<Trimpot>(mm2px(Vec(305, 21.217)), module, Meander::CONTROL_BASS_VOLUME_PARAM));
		
			{
				auto w= createParamCentered<Trimpot>(mm2px(Vec(305,  29.217)), module, Meander::CONTROL_BASS_TARGETOCTAVE_PARAM);
				w->snap=true;
				addParam(w); 
			}
			addParam(createParamCentered<LEDButton>(mm2px(Vec(305,  37.217)), module, Meander::BUTTON_BASS_ACCENT_PARAM));
			addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(305,  37.217)), module, Meander::LIGHT_LEDBUTTON_BASS_ACCENT_PARAM));

			addParam(createParamCentered<LEDButton>(mm2px(Vec(305,  45.217)), module, Meander::BUTTON_BASS_SYNCOPATE_PARAM));
			addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(305,  45.217)), module, Meander::LIGHT_LEDBUTTON_BASS_SYNCOPATE_PARAM));

			addParam(createParamCentered<LEDButton>(mm2px(Vec(305,  53.217)), module, Meander::BUTTON_BASS_SHUFFLE_PARAM));
			addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(305,  53.217)), module, Meander::LIGHT_LEDBUTTON_BASS_SHUFFLE_PARAM));
		
			{
				auto w= createParamCentered<Trimpot>(mm2px(Vec(305, 61.217)), module, Meander::CONTROL_BASS_DIVISOR_PARAM);
				w->snap=true;
				addParam(w); 
			} 
			
			addParam(createParamCentered<LEDButton>(mm2px(Vec(305, 70)), module, Meander::BUTTON_ENABLE_BASS_STACCATO_PARAM));
			addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(305, 70)), module, Meander::LIGHT_LEDBUTTON_ENABLE_BASS_STACCATO_PARAM));

			{
				auto w= createParamCentered<Trimpot>(mm2px(Vec(358, 19)), module, Meander::CONTROL_HARMONY_FBM_OCTAVES_PARAM);
				w->snap=true;
				addParam(w); 
			}
			addParam(createParamCentered<Trimpot>(mm2px(Vec(358, 25)), module, Meander::CONTROL_HARMONY_FBM_PERIOD_PARAM));
		
			{
				auto w= createParamCentered<Trimpot>(mm2px(Vec(358, 34)), module, Meander::CONTROL_MELODY_FBM_OCTAVES_PARAM);
				w->snap=true;
				addParam(w); 
			}
			addParam(createParamCentered<Trimpot>(mm2px(Vec(358, 43)), module, Meander::CONTROL_MELODY_FBM_PERIOD_PARAM));
		
			{
				auto w= createParamCentered<Trimpot>(mm2px(Vec(358, 51)), module, Meander::CONTROL_ARP_FBM_OCTAVES_PARAM);
				w->snap=true;
				addParam(w); 
			}
			addParam(createParamCentered<Trimpot>(mm2px(Vec(358, 59)), module, Meander::CONTROL_ARP_FBM_PERIOD_PARAM));
					 

			addParam(createParamCentered<LEDButton>(mm2px(Vec(240.274, 62.01)), module, Meander::BUTTON_ENABLE_ARP_PARAM));
			addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(240.274, 62.01)), module, Meander::LIGHT_LEDBUTTON_ARP_ENABLE));
			addParam(createParamCentered<LEDButton>(mm2px(Vec(265.274, 62.01)), module, Meander::BUTTON_ENABLE_ARP_CHORDAL_PARAM));
			addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(265.274, 62.01)), module, Meander::LIGHT_LEDBUTTON_ARP_ENABLE_CHORDAL));
			addParam(createParamCentered<LEDButton>(mm2px(Vec(283.274, 62.01)), module, Meander::BUTTON_ENABLE_ARP_SCALER_PARAM));
			addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(283.274, 62.01)), module, Meander::LIGHT_LEDBUTTON_ARP_ENABLE_SCALER));
		
			{
				auto w= createParamCentered<Trimpot>(mm2px(Vec(240.274, 68.014)), module, Meander::CONTROL_ARP_COUNT_PARAM);
				w->snap=true;
				addParam(w); 
			}

			addParam(createParamCentered<LEDButton>(mm2px(Vec(240.274, 75)), module, Meander::BUTTON_ENABLE_MELODY_STACCATO_PARAM));
			addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(240.274, 75)), module, Meander::LIGHT_LEDBUTTON_ENABLE_MELODY_STACCATO_PARAM));
		
			{
				auto w= createParamCentered<Trimpot>(mm2px(Vec(240.256, 82.807)), module, Meander::CONTROL_ARP_INCREMENT_PARAM);
				w->snap=true;
				addParam(w); 
			}
			addParam(createParamCentered<Trimpot>(mm2px(Vec(240.237, 91.691)), module, Meander::CONTROL_ARP_DECAY_PARAM));
		
			{
				auto w= createParamCentered<Trimpot>(mm2px(Vec(240.757, 99.497)), module, Meander::CONTROL_ARP_PATTERN_PARAM);
				w->snap=true;
				addParam(w); 
			}


			

	//*********** Harmony step set selection    

			CircleOf5thsSelectStepButton[CircleOf5thsStepSelectButtonindex]=createParamCentered<LEDButton>(mm2px(Vec(65.197, 106.483)), module, Meander::BUTTON_HARMONY_SETSTEP_1_PARAM);
			addParam(CircleOf5thsSelectStepButton[CircleOf5thsStepSelectButtonindex++]);
			CircleOf5thsSelectStepButtonLight[CircleOf5thsStepSelectButtonLightindex]=createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(65.197, 106.483)), module, Meander::LIGHT_LEDBUTTON_CIRCLESETSTEP_1);
			addChild(CircleOf5thsSelectStepButtonLight[CircleOf5thsStepSelectButtonLightindex++]); 

			CircleOf5thsSelectStepButton[CircleOf5thsStepSelectButtonindex]=createParamCentered<LEDButton>(mm2px(Vec(64.918, 98.02)), module, Meander::BUTTON_HARMONY_SETSTEP_2_PARAM);
			addParam(CircleOf5thsSelectStepButton[CircleOf5thsStepSelectButtonindex++]);
			CircleOf5thsSelectStepButtonLight[CircleOf5thsStepSelectButtonLightindex]=createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(64.918, 98.02)), module, Meander::LIGHT_LEDBUTTON_CIRCLESETSTEP_2);
			addChild(CircleOf5thsSelectStepButtonLight[CircleOf5thsStepSelectButtonLightindex++]); 

			CircleOf5thsSelectStepButton[CircleOf5thsStepSelectButtonindex]=createParamCentered<LEDButton>(mm2px(Vec(65.193, 89.271)), module, Meander::BUTTON_HARMONY_SETSTEP_3_PARAM);
			addParam(CircleOf5thsSelectStepButton[CircleOf5thsStepSelectButtonindex++]);
			CircleOf5thsSelectStepButtonLight[CircleOf5thsStepSelectButtonLightindex]=createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(65.193, 89.271)), module, Meander::LIGHT_LEDBUTTON_CIRCLESETSTEP_3);
			addChild(CircleOf5thsSelectStepButtonLight[CircleOf5thsStepSelectButtonLightindex++]); 

			CircleOf5thsSelectStepButton[CircleOf5thsStepSelectButtonindex]=createParamCentered<LEDButton>(mm2px(Vec(64.918, 81.923)), module, Meander::BUTTON_HARMONY_SETSTEP_4_PARAM);
			addParam(CircleOf5thsSelectStepButton[CircleOf5thsStepSelectButtonindex++]);
			CircleOf5thsSelectStepButtonLight[CircleOf5thsStepSelectButtonLightindex]=createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(64.918, 81.923)), module, Meander::LIGHT_LEDBUTTON_CIRCLESETSTEP_4);
			addChild(CircleOf5thsSelectStepButtonLight[CircleOf5thsStepSelectButtonLightindex++]); 

			CircleOf5thsSelectStepButton[CircleOf5thsStepSelectButtonindex]=createParamCentered<LEDButton>(mm2px(Vec(65.193, 73.184)), module, Meander::BUTTON_HARMONY_SETSTEP_5_PARAM);
			addParam(CircleOf5thsSelectStepButton[CircleOf5thsStepSelectButtonindex++]);
			CircleOf5thsSelectStepButtonLight[CircleOf5thsStepSelectButtonLightindex]=createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(65.193, 73.184)), module, Meander::LIGHT_LEDBUTTON_CIRCLESETSTEP_5);
			addChild(CircleOf5thsSelectStepButtonLight[CircleOf5thsStepSelectButtonLightindex++]); 

			CircleOf5thsSelectStepButton[CircleOf5thsStepSelectButtonindex]=createParamCentered<LEDButton>(mm2px(Vec(64.918, 66.129)), module, Meander::BUTTON_HARMONY_SETSTEP_6_PARAM);
			addParam(CircleOf5thsSelectStepButton[CircleOf5thsStepSelectButtonindex++]);
			CircleOf5thsSelectStepButtonLight[CircleOf5thsStepSelectButtonLightindex]=createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(64.918, 66.129)), module, Meander::LIGHT_LEDBUTTON_CIRCLESETSTEP_6);
			addChild(CircleOf5thsSelectStepButtonLight[CircleOf5thsStepSelectButtonLightindex++]); 

			CircleOf5thsSelectStepButton[CircleOf5thsStepSelectButtonindex]=createParamCentered<LEDButton>(mm2px(Vec(65.193, 57.944)), module, Meander::BUTTON_HARMONY_SETSTEP_7_PARAM);
			addParam(CircleOf5thsSelectStepButton[CircleOf5thsStepSelectButtonindex++]);
			CircleOf5thsSelectStepButtonLight[CircleOf5thsStepSelectButtonLightindex]=createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(65.193, 57.944)), module, Meander::LIGHT_LEDBUTTON_CIRCLESETSTEP_7);
			addChild(CircleOf5thsSelectStepButtonLight[CircleOf5thsStepSelectButtonLightindex++]); 

			CircleOf5thsSelectStepButton[CircleOf5thsStepSelectButtonindex]=createParamCentered<LEDButton>(mm2px(Vec(64.911, 49.474)), module, Meander::BUTTON_HARMONY_SETSTEP_8_PARAM);
			addParam(CircleOf5thsSelectStepButton[CircleOf5thsStepSelectButtonindex++]);
			CircleOf5thsSelectStepButtonLight[CircleOf5thsStepSelectButtonLightindex]=createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(64.911, 49.474)), module, Meander::LIGHT_LEDBUTTON_CIRCLESETSTEP_8);
			addChild(CircleOf5thsSelectStepButtonLight[CircleOf5thsStepSelectButtonLightindex++]); 

			CircleOf5thsSelectStepButton[CircleOf5thsStepSelectButtonindex]=createParamCentered<LEDButton>(mm2px(Vec(64.629, 41.011)), module, Meander::BUTTON_HARMONY_SETSTEP_9_PARAM);
			addParam(CircleOf5thsSelectStepButton[CircleOf5thsStepSelectButtonindex++]);
			CircleOf5thsSelectStepButtonLight[CircleOf5thsStepSelectButtonLightindex]=createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(64.629, 41.011)), module, Meander::LIGHT_LEDBUTTON_CIRCLESETSTEP_9);
			addChild(CircleOf5thsSelectStepButtonLight[CircleOf5thsStepSelectButtonLightindex++]); 

			CircleOf5thsSelectStepButton[CircleOf5thsStepSelectButtonindex]=createParamCentered<LEDButton>(mm2px(Vec(64.629, 32.827)), module, Meander::BUTTON_HARMONY_SETSTEP_10_PARAM);
			addParam(CircleOf5thsSelectStepButton[CircleOf5thsStepSelectButtonindex++]);
			CircleOf5thsSelectStepButtonLight[CircleOf5thsStepSelectButtonLightindex]=createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(64.629, 32.827)), module, Meander::LIGHT_LEDBUTTON_CIRCLESETSTEP_10);
			addChild(CircleOf5thsSelectStepButtonLight[CircleOf5thsStepSelectButtonLightindex++]); 

			CircleOf5thsSelectStepButton[CircleOf5thsStepSelectButtonindex]=createParamCentered<LEDButton>(mm2px(Vec(64.629, 24.649)), module, Meander::BUTTON_HARMONY_SETSTEP_11_PARAM);
			addParam(CircleOf5thsSelectStepButton[CircleOf5thsStepSelectButtonindex++]);
			CircleOf5thsSelectStepButtonLight[CircleOf5thsStepSelectButtonLightindex]=createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(64.629, 24.649)), module, Meander::LIGHT_LEDBUTTON_CIRCLESETSTEP_11);
			addChild(CircleOf5thsSelectStepButtonLight[CircleOf5thsStepSelectButtonLightindex++]); 

			CircleOf5thsSelectStepButton[CircleOf5thsStepSelectButtonindex]=createParamCentered<LEDButton>(mm2px(Vec(64.632, 16.176)), module, Meander::BUTTON_HARMONY_SETSTEP_12_PARAM);
			addParam(CircleOf5thsSelectStepButton[CircleOf5thsStepSelectButtonindex++]);
			CircleOf5thsSelectStepButtonLight[CircleOf5thsStepSelectButtonLightindex]=createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(64.632, 16.176)), module, Meander::LIGHT_LEDBUTTON_CIRCLESETSTEP_12);
			addChild(CircleOf5thsSelectStepButtonLight[CircleOf5thsStepSelectButtonLightindex++]); 


			CircleOf5thsSelectStepButton[CircleOf5thsStepSelectButtonindex]=createParamCentered<LEDButton>(mm2px(Vec(64.632, 16.176)), module, Meander::BUTTON_HARMONY_SETSTEP_13_PARAM);
			addParam(CircleOf5thsSelectStepButton[CircleOf5thsStepSelectButtonindex++]);
			CircleOf5thsSelectStepButtonLight[CircleOf5thsStepSelectButtonLightindex]=createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(64.632, 16.176)), module, Meander::LIGHT_LEDBUTTON_CIRCLESETSTEP_13);
			addChild(CircleOf5thsSelectStepButtonLight[CircleOf5thsStepSelectButtonLightindex++]); 

			CircleOf5thsSelectStepButton[CircleOf5thsStepSelectButtonindex]=createParamCentered<LEDButton>(mm2px(Vec(64.632, 16.176)), module, Meander::BUTTON_HARMONY_SETSTEP_14_PARAM);
			addParam(CircleOf5thsSelectStepButton[CircleOf5thsStepSelectButtonindex++]);
			CircleOf5thsSelectStepButtonLight[CircleOf5thsStepSelectButtonLightindex]=createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(64.632, 16.176)), module, Meander::LIGHT_LEDBUTTON_CIRCLESETSTEP_14);
			addChild(CircleOf5thsSelectStepButtonLight[CircleOf5thsStepSelectButtonLightindex++]); 

			CircleOf5thsSelectStepButton[CircleOf5thsStepSelectButtonindex]=createParamCentered<LEDButton>(mm2px(Vec(64.632, 16.176)), module, Meander::BUTTON_HARMONY_SETSTEP_15_PARAM);
			addParam(CircleOf5thsSelectStepButton[CircleOf5thsStepSelectButtonindex++]);
			CircleOf5thsSelectStepButtonLight[CircleOf5thsStepSelectButtonLightindex]=createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(64.632, 16.176)), module, Meander::LIGHT_LEDBUTTON_CIRCLESETSTEP_15);
			addChild(CircleOf5thsSelectStepButtonLight[CircleOf5thsStepSelectButtonLightindex++]); 

			CircleOf5thsSelectStepButton[CircleOf5thsStepSelectButtonindex]=createParamCentered<LEDButton>(mm2px(Vec(64.632, 16.176)), module, Meander::BUTTON_HARMONY_SETSTEP_16_PARAM);
			addParam(CircleOf5thsSelectStepButton[CircleOf5thsStepSelectButtonindex++]);
			CircleOf5thsSelectStepButtonLight[CircleOf5thsStepSelectButtonLightindex]=createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(64.632, 16.176)), module, Meander::LIGHT_LEDBUTTON_CIRCLESETSTEP_16);
			addChild(CircleOf5thsSelectStepButtonLight[CircleOf5thsStepSelectButtonLightindex++]); 

	//**************  
			
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8.12, 101.563)), module, Meander::IN_CLOCK_EXT));
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8.12, 10.95)), module, Meander::IN_RUN_EXT));
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8.12, 23.35)), module, Meander::IN_RESET_EXT));
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8.12, 43.05)), module, Meander::IN_TEMPO_EXT));
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(117, 75)), module, Meander::IN_HARMONY_CV_EXT));
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(117, 85)), module, Meander::IN_HARMONY_GATE_EXT));
			
			
			addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(30.2, 10.95)), module, Meander::OUT_RUN_OUT));
			addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(30.2, 23.35)), module, Meander::OUT_RESET_OUT));

		//	addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(266.982, 107.333)), module, Meander::OUT_MELODY_GATE_OUTPUT));
		//	addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(201.789, 107.616)), module, Meander::OUT_HARMONY_GATE_OUTPUT));
		//	addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(325.12, 107.616)), module, Meander::OUT_BASS_GATE_OUTPUT));

			addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(266.982, 107.333)), module, Meander::OUT_MELODY_VOLUME_OUTPUT));
			addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(201.789, 107.616)), module, Meander::OUT_HARMONY_VOLUME_OUTPUT));
			addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(325.12, 107.616)), module, Meander::OUT_BASS_VOLUME_OUTPUT));
			
			addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(266.899, 115.813)), module, Meander::OUT_MELODY_CV_OUTPUT));
			
			addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(325.266, 115.817)), module, Meander::OUT_BASS_CV_OUTPUT));
			addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(202.176, 115.909)), module, Meander::OUT_HARMONY_CV_OUTPUT));
			addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(78.74, 122.291)), module, Meander::OUT_CLOCK_BEATX2_OUTPUT));
			addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(37.143, 122.537)), module, Meander::OUT_CLOCK_BAR_OUTPUT));
			addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(99.342, 122.573)), module, Meander::OUT_CLOCK_BEATX4_OUTPUT));
			addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(121.073, 122.573)), module, Meander::OUT_CLOCK_BEATX8_OUTPUT));
			addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(57.856, 122.856)), module, Meander::OUT_CLOCK_BEAT_OUTPUT));

			addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(325.402, 123.984)), module, Meander::OUT_BASS_GATE_OUTPUT));
			addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(202.071, 124.267)), module, Meander::OUT_HARMONY_GATE_OUTPUT));
			addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(266.982, 124.549)), module, Meander::OUT_MELODY_GATE_OUTPUT));

						
			addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(380.0, 107.616)), module, Meander::OUT_FBM_GATE_OUTPUT));
			addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(380.0, 115.815)), module, Meander::OUT_FBM_CV_OUTPUT));
			addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(380.0, 124.831)), module, Meander::OUT_FBM_TRIGGER_OUTPUT));

			addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(6.865, 101.793)), module, Meander::LIGHT_CLOCK_IN_LIGHT));

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
				controlPosition=controlPosition.minus((CircleOf5thsOuterButton[i]->box.size).div(2.));  // adjust for box size
				CircleOf5thsOuterButton[i]->box.pos=controlPosition;
				
				controlPosition=CircleCenter.plus(radialDirection.mult(OuterCircleRadius*.78f));
				controlPosition=controlPosition.minus((CircleOf5thsOuterButtonLight[i]->box.size).div(2.));  // adjust for box size
				CircleOf5thsOuterButtonLight[i]->box.pos=controlPosition;

				controlPosition=CircleCenter.plus(radialDirection.mult(OuterCircleRadius*.61f));
				controlPosition=controlPosition.minus((CircleOf5thsInnerLight[i]->box.size).div(2.));
				CircleOf5thsInnerLight[i]->box.pos=controlPosition;
			}

		
			// re-layout the circle step set buttons and lights programatically
			Vec start_position=Vec(0,0);
			float verticalIncrement=mm2px(5.92f);
			for(int i = 0; i<MAX_STEPS;++i) 
			{
				start_position=mm2px(Vec(62, 128.9- 109.27));  // for Y subtract SVG Y from panel height 128.9 and then convert to px
				Vec buttonPosition=start_position.plus(Vec(0, (i*verticalIncrement)));
				buttonPosition.y -= CircleOf5thsSelectStepButton[i]->box.size.y;  // adjust for box height
				CircleOf5thsSelectStepButton[i]->box.pos=buttonPosition;
				start_position=mm2px(Vec(63.5, 128.9- 110.8));  // for Y subtract SVG Y from panel height 128.9 and then convert to px
				buttonPosition=start_position.plus(Vec(0, (i*verticalIncrement)));
				buttonPosition.y -= CircleOf5thsSelectStepButtonLight[i]->box.size.y; // adjust for box height
				CircleOf5thsSelectStepButtonLight[i]->box.pos=buttonPosition;
			}
		}

	}    // end MeanderWidget(Meander* module)  
 
		
	void step() override   // note, this is a widget step() which is not deprecated and is a GUI call.  This advances UI by one "frame"
	{  
		if(!module) return;
	
		ModuleWidget::step();
	} 
};

Model* modelMeander = createModel<Meander, MeanderWidget>("Meander");


