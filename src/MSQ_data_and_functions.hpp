// these are module scope vars


/*  Copyright (C) 2019-2024 Ken Chaffin
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.
*/

#include "Common-Noise.hpp"   

//*********************************************************Module Vars************************************************

bool moduleVarsInitialized=false;  //  initialized only during Module()
bool patchDataLoaded=false;  // set true in dataFromJson()

bool randEnqueued=false;

struct inPortState 
{
	bool inTransition=false;
	float lastValue=-999.;
};


struct inPortState inportStates[MAX_INPORTS];

int time_sig_top = 4;
int time_sig_bottom = 4;

float last_poly_quant_value[16]={-999};

struct CircleElement
{
	// this data is mostly static, once created
	int chordType=0;  // may be overridden by degree semicircle
	float startDegree=0; // of annular segment
	float endDegree=0;   // of annular segment
	Vec pt1;  //vertices of annular ring segment
	Vec pt2;
	Vec pt3;
	Vec pt4;
	Vec radialDirection;
};


struct DegreeElement
{
	// this data varies with root_key and mode
	int chordType=0;
	float startDegree=0;  // of pie slice
	float endDegree=0;
	Vec pt1; //vertices of annular ring segment
	Vec pt2;
	Vec pt3;
	Vec pt4;
	Vec radialDirection;
	int Degree=1; // 1-7 corresponding to I-VII
	int CircleIndex=0;  // correspondence between degree element and circle element.  All degrees have one.
};


struct DegreeSemiCircle
{
	int RootKeyCircle5thsPosition=0;    // semicircle [index] of "I" degree 
	int OffsetSteps=0;  // how many 30 degree steps the semicircle must be rotated to display correctly
	DegreeElement degreeElements[MAX_HARMONIC_DEGREES];
};


struct CircleOf5ths
{
	float OuterCircleRadius=mm2px(47);        
	float MiddleCircleRadius=mm2px(39);
	float InnerCircleRadius=mm2px(26);
	Vec CircleCenter= Vec(mm2px(116.75),mm2px(67.75));
	
	int root_keyCirclePosition=0;
	int root_key_note=0;
	struct CircleElement Circle5ths[MAX_CIRCLE_STATIONS];
	struct DegreeElement degreeElements[MAX_CIRCLE_STATIONS];
	struct DegreeSemiCircle theDegreeSemiCircle;
		
} theCircleOf5ths;


//bool circleChanged=true;
bool circleChanged=false;

int semiCircleDegrees[7]={1, 5, 2, 6, 3, 7, 4};  // default order if starting at C
int circleDegreeLookup[8]= {0, 0, 2, 4, 6, 1, 3, 5};  // to convert from arabic roman equivalents to circle degrees
int arabicStepDegreeSemicircleIndex[8];  // where is 1, 2... step in degree semicircle  // [8] so 1 based indexing can be used



//*******************************************

int  mode=1;  // Ionian/Major

struct note
{
	int note=0;
	int noteType=0; // NOTE_TYPE_CHORD etc.
	int time32s=0;
	int length=0;  // 1/1,2,4,8
	int countInBar=0;
	bool isPlaying=false; // set when note played. unset upon next gate end for noteType
};

int bar_note_count=0;  // how many notes have been played in bar.  Use it as index into  played_notes_circular_buffer[]

struct note played_notes_circular_buffer[256];  // worst case maximum of 256 harmony, melody and bass notes can be played per bar.  


const char* noteNames[MAX_NOTES] = {"C","C#/Db","D","D#/Eb","E","F","F#/Gb","G","G#/Ab","A","A#/Bb","B"};

int circle_root_key=0; // root_key position on the circle 0,1,2... CW
int root_key=0;  // 0 initially

char note_desig[MAX_NOTES][MAXSHORTSTRLEN]={"C","Db","D","Eb","E","F","F#","G","Ab","A","Bb","B"};
int notate_mode_as_signature_root_key=0; // 0 initially
// any mode and root_key is equivalent to a maj key by transposing down these numbers of major keydown semitones
// Mode        Transpose down by interval  or semitones
// Ionian						Perfrect Unison    0
// Dorian						Major second       2
// Phrygian						Major third		   4 
// Lydian						Perfect fourth     5
// Mixolydian					Perfect fifth	   7
// Aeolian						Major sixth        9
// Locrian						Major seventh     11

// more simply modes can also be transposed to major scale by using the mode natural roots semitone counts for white keys CDEFGAB
int mode_natural_roots[MAX_MODES] = {  // for which scale is all white notes  these can also be used as transpose semitones if modes in IDPLMAL order
	0,  //C  Ionian
	2,  //D  Dorian 
	4,  //E  Phrygian
	5,  //F  Lydian
	7,  //G  Mixolydian
	9,  //A  Aeolian
	11  //B  Locrian
};

int mode_root_key_signature_offset[7]={3,0,4,1,5,2,6};  // index into mode_natural_roots[] using the IDPLyMALo = 1,2,3,4,5,6,7 rule for ModeScaleQuant mode ordering

char root_key_name[MAXSHORTSTRLEN];

#define MAX_NOTES_CANDIDATES 130
int  notes[MAX_NOTES_CANDIDATES];

int  num_notes=0;
int  root_key_notes[MAX_ROOT_KEYS][MAX_NOTES_CANDIDATES];

int  num_root_key_notes[MAX_ROOT_KEYS];

int meter_numerator=4;  // need to unify with sig_top...
int meter_denominator=4;


struct HarmonyParms
{
	bool enabled=true;
	float volume=10.0f;  // 0-10 V
	int note_length_divisor=1;  // 1, 2, 4, 8   doHarmony() on these boundaries
	int target_octave=4;
	double note_octave_range=1.0;
	double note_avg_target=target_octave/10.0;  
	double range_top=    note_avg_target + (note_octave_range/10.0);
	double range_bottom= note_avg_target - (note_octave_range/10.0);
	double r1=(range_top-range_bottom); 
	double note_avg= note_avg_target;  
	double alpha=.1;
	double seed=1234;
	int noctaves=3;
	float period=100.0;
	int last_circle_step=-1;  // used for Markov chains
	int last_chord_type=0;
	bool last_chord_playing=false;
	int bar_harmony_chords_counted_note=0;
	bool enable_all_7ths=false;  // actually all "nice 7ths" based on common practice  
	bool enable_V_7ths=false;
	bool enable_4voice_octaves=false;  // octave chord is a triad with the root raised by an octave and addeed as 4th note
    bool enable_staccato=false;
	int pending_step_edit=0;
	struct note last[4];
	float lastCircleDegreeIn=0;
	int STEP_inport_connected_to_ModeScaleQuant_trigger_port=0;
	bool send_tonic_on_first_channel=true; // else send bass on first channel. Default for histrorical reasons
	bool send_bass_on_first_channel=false; // else send tonic on first channel
};  

struct MelodyParms  
{
	bool enabled=true;
	bool chordal=true;
	bool scaler=false;
	float volume=8.0f;  // 0-10 V
	int note_length_divisor=4;
	double target_octave=3.0;  // 4=middle-C C4 0v  must be an integer value
	double note_octave_range=1.0;
	double note_avg_target=target_octave/10.0;  
	double range_top=    note_avg_target + (note_octave_range/10.0);
	double range_bottom= note_avg_target - (note_octave_range/10.0);
	double r1=(range_top-range_bottom); 

	double note_avg= note_avg_target;  
	double alpha=.9;
	double seed=12345;
	int noctaves=4;
	float period=10.0;
	bool destutter=false;
	bool stutter_detected=false;
	int last_stutter_step=0;
	int last_chord_note_index=0;
	int last_step=1; 
	int bar_melody_counted_note=0;
    bool enable_staccato=true;
	struct note last[1];
	float lastMelodyDegreeIn=0.0f;
}; 

struct ArpParms
{
	bool enabled=false;
	bool chordal=true;
	bool scaler=false;
	int count=3;
	int note_length_divisor=16;  // 8, 16, 32
	float decay=0;
	int pattern=0;
	int note_count=0;  // number of arp notes played per current melody note
	double seed=123344;
	int noctaves=5;
	float period=1.0;
	struct note last[32];  // may not need this if arp is considered melody
}; 

struct BassParms
{
	bool enabled=true; 
	int target_octave=2;
	int note_length_divisor=1;  // 1, 2, 4, 8   doHarmony() on these boundaries
	bool octave_enabled=true;  // play bass as 2 notes an octave apart
	float volume=10.0f;  // 0-10 V
	int bar_bass_counted_note=0;
    bool accent=false;  // enables accents
	bool syncopate=false;
	bool shuffle=false;
	struct note last[4];
    bool enable_staccato= true;
    bool note_accented=false;  // is current played note accented
}; 


struct ModeScaleQuantState
{
	HarmonyParms  theHarmonyParms;
	MelodyParms theMelodyParms;
	BassParms theBassParms;
	ArpParms theArpParms;
	bool userControllingHarmonyFromCircle=false;
	int last_harmony_chord_root_note=0;
	int last_harmony_chord_bass_note=0;
	int last_harmony_step=0;
	int circleDegree=1;
	int scaleDegree=1;
	bool userControllingMelody=false;
	bool RootInputSuppliedByRootOutput=false;
	bool ModeInputSuppliedByModeOutput=false;
}	theModeScaleQuantState;

 
char chord_type_name[30][MAXSHORTSTRLEN]; 
int chord_type_intervals[30][16];
int chord_type_num_notes[30]; 
int  current_chord_notes[16];

#define MAX_HARMONY_TYPES 100

// for up to MAX_HARMONY_TYPES harmony_types, up to MAX_STEPS steps
int    harmony_type=14;  // 1- MAX_AVAILABLE_HARMONY_PRESETS
bool randomize_harmony=false;



struct HarmonyType
{
	int    harmony_type=0;  // used by theActiveHarmonyType
	char   harmony_type_desc[64]; 
	char   harmony_degrees_desc[128]; 
	int    num_harmony_steps=1;
	int    min_steps=1;
	int    max_steps=1;
	int    harmony_step_chord_type[MAX_STEPS];
	int    harmony_steps[MAX_STEPS]={1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};  // initialize to a valid step degree
};
struct HarmonyType theHarmonyTypes[MAX_HARMONY_TYPES];

struct HarmonyType theActiveHarmonyType;

int  circle_of_fifths[MAX_CIRCLE_STATIONS];

int    home_circle_position;
int    current_circle_position;
int    last_circle_position;

int    current_circle_degree=1;
bool   valid_current_circle_degree=false;

int  step_chord_notes[MAX_STEPS][MAX_NOTES_CANDIDATES];
int  num_step_chord_notes[MAX_STEPS]={};

struct chord_type_info 
{
	char name[MAXSHORTSTRLEN]="";
	int num_notes=0;
	int intervals[8]={0,0,0,0,0,0,0,0};
};
struct chord_type_info chordTypeInfo[30];


void init_module_vars()
{
	
	circle_of_fifths[0]=0;
	circle_of_fifths[1]=7;
	circle_of_fifths[2]=2;
	circle_of_fifths[3]=9;
	circle_of_fifths[4]=4;
	circle_of_fifths[5]=11;
	circle_of_fifths[6]=6;
	circle_of_fifths[7]=1;
	circle_of_fifths[8]=8;
	circle_of_fifths[9]=3;
	circle_of_fifths[10]=10;
	circle_of_fifths[11]=5;

	// modulo 12 notation		
	strcpy(chord_type_name[0],"Major");
	chord_type_num_notes[0]=3;
	chord_type_intervals[0][0]=0;
	chord_type_intervals[0][1]=4;
	chord_type_intervals[0][2]=7;
	strcpy(chord_type_name[1],"Minor");
	chord_type_num_notes[1]=3;
	chord_type_intervals[1][0]=0;
	chord_type_intervals[1][1]=3;
	chord_type_intervals[1][2]=7;
	strcpy(chord_type_name[2],"7th");  // usually a dominant 7th chord is a major triad with minor 7th
	chord_type_num_notes[2]=4;
	chord_type_intervals[2][0]=0;
	chord_type_intervals[2][1]=4;
	chord_type_intervals[2][2]=7;
	chord_type_intervals[2][3]=10;
	strcpy(chord_type_name[3],"maj7th");
	chord_type_num_notes[3]=4;
	chord_type_intervals[3][0]=0;
	chord_type_intervals[3][1]=4;
	chord_type_intervals[3][2]=7;
	chord_type_intervals[3][3]=11;
	strcpy(chord_type_name[4],"min7th");
	chord_type_num_notes[4]=4;
	chord_type_intervals[4][0]=0;
	chord_type_intervals[4][1]=3;
	chord_type_intervals[4][2]=7;
	chord_type_intervals[4][3]=10;
	strcpy(chord_type_name[5],"dim7th");
	chord_type_num_notes[5]=4;
	chord_type_intervals[5][0]=0;
	chord_type_intervals[5][1]=3;
	chord_type_intervals[5][2]=6;
	chord_type_intervals[5][3]=10;
	strcpy(chord_type_name[6],"Dim");
   	chord_type_num_notes[6]=3;
	chord_type_intervals[6][0]=0;
	chord_type_intervals[6][1]=3;
	chord_type_intervals[6][2]=6;
	strcpy(chord_type_name[7],"Aug");
	chord_type_num_notes[7]=3;
	chord_type_intervals[7][0]=0;
	chord_type_intervals[7][1]=4;
	chord_type_intervals[7][2]=8;
	strcpy(chord_type_name[8],"6th");
	chord_type_num_notes[8]=4;
	chord_type_intervals[8][0]=0;
	chord_type_intervals[8][1]=4;
	chord_type_intervals[8][2]=7;
	chord_type_intervals[8][3]=9;
	strcpy(chord_type_name[9],"min6th");
	chord_type_num_notes[9]=4;
   	chord_type_intervals[9][0]=0;
	chord_type_intervals[9][1]=3;
	chord_type_intervals[9][2]=7;
	chord_type_intervals[9][3]=9;
	strcpy(chord_type_name[10],"dim6th");
	chord_type_num_notes[10]=4;
	chord_type_intervals[10][0]=0;
	chord_type_intervals[10][1]=4;
	chord_type_intervals[10][2]=6;
	chord_type_intervals[10][3]=9;
	strcpy(chord_type_name[11],"9th");
	chord_type_num_notes[11]=5;
	chord_type_intervals[11][0]=0;
	chord_type_intervals[11][1]=4;
	chord_type_intervals[11][2]=7;
	chord_type_intervals[11][3]=10;
	chord_type_intervals[11][4]=14;
	strcpy(chord_type_name[12],"10th");
	chord_type_num_notes[12]=2;
	chord_type_intervals[12][0]=0;
	chord_type_intervals[12][1]=9;
	strcpy(chord_type_name[13],"11th");
	chord_type_num_notes[13]=6;
	chord_type_intervals[13][0]=0;
	chord_type_intervals[13][1]=4;
	chord_type_intervals[13][2]=7;
	chord_type_intervals[13][3]=10;
	chord_type_intervals[13][4]=14;
	chord_type_intervals[13][5]=17;
	strcpy(chord_type_name[14],"13th");
	chord_type_num_notes[14]=7;
	chord_type_intervals[14][0]=0;
	chord_type_intervals[14][1]=4;
	chord_type_intervals[14][2]=7;
	chord_type_intervals[14][3]=10;
	chord_type_intervals[14][4]=14;
	chord_type_intervals[14][5]=17;
	chord_type_intervals[14][6]=21;
	strcpy(chord_type_name[15],"Quartel");
	chord_type_num_notes[15]=4;
	chord_type_intervals[15][0]=0;
	chord_type_intervals[15][1]=5;
	chord_type_intervals[15][2]=10;
	chord_type_intervals[15][3]=15;
	strcpy(chord_type_name[16],"Perf5th");
	chord_type_num_notes[16]=4;
	chord_type_intervals[16][0]=0;
	chord_type_intervals[16][1]=7;
	chord_type_intervals[16][2]=14;
	chord_type_intervals[16][3]=21;
	strcpy(chord_type_name[17],"Scalar");
	chord_type_num_notes[17]=3;
	chord_type_intervals[17][0]=0;
	chord_type_intervals[17][1]=4;
	chord_type_intervals[17][2]=7;

	notes[0]=root_key;                                                        
}

void init_notes()
{
	notes[0]=root_key;  
	int nmn=MSQ_mode_step_intervals[mode][0];  // number of mode notes
	num_notes=0;                                                                
	for (int i=1;i<127;++i)                                                         
	{     
		notes[i]=notes[i-1]+                                                    
			MSQ_mode_step_intervals[mode][((i-1)%nmn)+1];  
				    
		++num_notes;                                                            
		if (notes[i]>=127) break;                                               
	}     
																

	for (int j=0;j<12;++j)
	{
		root_key_notes[j][0]=j;
		num_root_key_notes[j]=1;
		

		int num_mode_notes=10*MSQ_mode_step_intervals[mode][0]; // the [0] entry is the notes per scale value, times 10 ocatves of midi

		int nmn=MSQ_mode_step_intervals[mode][0];  // number of mode notes
		for (int i=1;i<num_mode_notes ;++i)
		{
			root_key_notes[j][i]=root_key_notes[j][i-1]+
		   		MSQ_mode_step_intervals[mode][((i-1)%nmn)+1];  
			++num_root_key_notes[j];
		}
			
	}

	char  strng[128];
	strcpy(strng,"");
	for (int i=0;i<MSQ_mode_step_intervals[mode][0];++i)
	{
		strcat(strng,note_desig[notes[i]%MAX_NOTES]);
	}
}


void copyHarmonyTypeToActiveHarmonyType(int harmType)
{
	theActiveHarmonyType.harmony_type=harmType;  // the parent harmony_type
	theActiveHarmonyType.num_harmony_steps=theHarmonyTypes[harmType].num_harmony_steps;
	theActiveHarmonyType.min_steps=theHarmonyTypes[harmType].min_steps;
	theActiveHarmonyType.max_steps=theHarmonyTypes[harmType].max_steps;  // this is 1 for some reason after prog edit
	strcpy(theActiveHarmonyType.harmony_type_desc, theHarmonyTypes[harmType].harmony_type_desc);
	strcpy(theActiveHarmonyType.harmony_degrees_desc, theHarmonyTypes[harmType].harmony_degrees_desc);
	for (int i=0; i<MAX_STEPS; ++i)
	{
		theActiveHarmonyType.harmony_steps[i]=theHarmonyTypes[harmType].harmony_steps[i];	
		theActiveHarmonyType.harmony_step_chord_type[i]=theHarmonyTypes[harmType].harmony_step_chord_type[i];
	}
}


void ModeScaleQuantMusicStructuresInitialize()
{
	copyHarmonyTypeToActiveHarmonyType(harmony_type);
	moduleVarsInitialized=true;  // prevents process() from doing anything before initialization and also prevents some access by ModuleWidget
} 


void ConstructCircle5ths(int circleRootKey, int mode)
{
    
    for (int i=0; i<MAX_CIRCLE_STATIONS; ++i)
    {
            const float rotate90 = (M_PI) / 2.0;
                        
            // construct root_key annulus sector
                        
            theCircleOf5ths.Circle5ths[i].startDegree = (M_PI * 2.0 * ((double)i - 0.5) / MAX_CIRCLE_STATIONS) - rotate90;
            theCircleOf5ths.Circle5ths[i].endDegree = 	(M_PI * 2.0 * ((double)i + 0.5) / MAX_CIRCLE_STATIONS) - rotate90;
                    
            double ax1= cos(theCircleOf5ths.Circle5ths[i].startDegree) * theCircleOf5ths.InnerCircleRadius + theCircleOf5ths.CircleCenter.x;
            double ay1= sin(theCircleOf5ths.Circle5ths[i].startDegree) * theCircleOf5ths.InnerCircleRadius + theCircleOf5ths.CircleCenter.y;
            double ax2= cos(theCircleOf5ths.Circle5ths[i].endDegree) * theCircleOf5ths.InnerCircleRadius + theCircleOf5ths.CircleCenter.x;
            double ay2= sin(theCircleOf5ths.Circle5ths[i].endDegree) * theCircleOf5ths.InnerCircleRadius + theCircleOf5ths.CircleCenter.y;
            double bx1= cos(theCircleOf5ths.Circle5ths[i].startDegree) * theCircleOf5ths.MiddleCircleRadius + theCircleOf5ths.CircleCenter.x;
            double by1= sin(theCircleOf5ths.Circle5ths[i].startDegree) * theCircleOf5ths.MiddleCircleRadius + theCircleOf5ths.CircleCenter.y;
            double bx2= cos(theCircleOf5ths.Circle5ths[i].endDegree) * theCircleOf5ths.MiddleCircleRadius + theCircleOf5ths.CircleCenter.x;
            double by2= sin(theCircleOf5ths.Circle5ths[i].endDegree) * theCircleOf5ths.MiddleCircleRadius + theCircleOf5ths.CircleCenter.y;

            theCircleOf5ths.Circle5ths[i].pt1=Vec(ax1, ay1);
            theCircleOf5ths.Circle5ths[i].pt2=Vec(bx1, by1);
            theCircleOf5ths.Circle5ths[i].pt3=Vec(ax2, ay2);
            theCircleOf5ths.Circle5ths[i].pt4=Vec(bx2, by2);

            Vec radialLine1=Vec(ax1,ay1).minus(theCircleOf5ths.CircleCenter);
            Vec radialLine2=Vec(ax2,ay2).minus(theCircleOf5ths.CircleCenter);
            Vec centerLine=(radialLine1.plus(radialLine2)).div(2.);
            theCircleOf5ths.Circle5ths[i].radialDirection=centerLine;
            theCircleOf5ths.Circle5ths[i].radialDirection=theCircleOf5ths.Circle5ths[i].radialDirection.normalize();
        
    }		
};

// should only be called after initialization
void ConstructDegreesSemicircle(int circleRootKey, int mode)
{
    const float rotate90 = (M_PI) / 2.0;
    float offsetDegree=((circleRootKey-mode+12)%12)*(2.0*M_PI/12.0);
    theCircleOf5ths.theDegreeSemiCircle.OffsetSteps=(circleRootKey-mode); 
    theCircleOf5ths.theDegreeSemiCircle.RootKeyCircle5thsPosition=-theCircleOf5ths.theDegreeSemiCircle.OffsetSteps+circle_root_key;
   
    int chord_type=0;
    
    for (int i=0; i<MAX_HARMONIC_DEGREES; ++i)
    {
            // construct degree annulus sector
            theCircleOf5ths.theDegreeSemiCircle.degreeElements[i].startDegree = (M_PI * 2.0 * ((double)i - 0.5) / MAX_CIRCLE_STATIONS) - rotate90 + offsetDegree;
            theCircleOf5ths.theDegreeSemiCircle.degreeElements[i].endDegree = (M_PI * 2.0 * ((double)i + 0.5) / MAX_CIRCLE_STATIONS) - rotate90 + offsetDegree;
                        
            double ax1= cos(theCircleOf5ths.theDegreeSemiCircle.degreeElements[i].startDegree) * theCircleOf5ths.MiddleCircleRadius + theCircleOf5ths.CircleCenter.x;
            double ay1= sin(theCircleOf5ths.theDegreeSemiCircle.degreeElements[i].startDegree) * theCircleOf5ths.MiddleCircleRadius + theCircleOf5ths.CircleCenter.y;
            double ax2= cos(theCircleOf5ths.theDegreeSemiCircle.degreeElements[i].endDegree) * theCircleOf5ths.MiddleCircleRadius + theCircleOf5ths.CircleCenter.x;
            double ay2= sin(theCircleOf5ths.theDegreeSemiCircle.degreeElements[i].endDegree) * theCircleOf5ths.MiddleCircleRadius + theCircleOf5ths.CircleCenter.y;
            double bx1= cos(theCircleOf5ths.theDegreeSemiCircle.degreeElements[i].startDegree) * theCircleOf5ths.OuterCircleRadius + theCircleOf5ths.CircleCenter.x;
            double by1= sin(theCircleOf5ths.theDegreeSemiCircle.degreeElements[i].startDegree) * theCircleOf5ths.OuterCircleRadius + theCircleOf5ths.CircleCenter.y;
            double bx2= cos(theCircleOf5ths.theDegreeSemiCircle.degreeElements[i].endDegree) * theCircleOf5ths.OuterCircleRadius + theCircleOf5ths.CircleCenter.x;
            double by2= sin(theCircleOf5ths.theDegreeSemiCircle.degreeElements[i].endDegree) * theCircleOf5ths.OuterCircleRadius + theCircleOf5ths.CircleCenter.y;

            theCircleOf5ths.theDegreeSemiCircle.degreeElements[i].pt1=Vec(ax1, ay1);
            theCircleOf5ths.theDegreeSemiCircle.degreeElements[i].pt2=Vec(bx1, by1);
            theCircleOf5ths.theDegreeSemiCircle.degreeElements[i].pt3=Vec(ax2, ay2);
            theCircleOf5ths.theDegreeSemiCircle.degreeElements[i].pt4=Vec(bx2, by2);

            Vec radialLine1=Vec(ax1,ay1).minus(theCircleOf5ths.CircleCenter);
            Vec radialLine2=Vec(ax2,ay2).minus(theCircleOf5ths.CircleCenter);
            Vec centerLine=(radialLine1.plus(radialLine2)).div(2.);
            theCircleOf5ths.theDegreeSemiCircle.degreeElements[i].radialDirection=centerLine;
            theCircleOf5ths.theDegreeSemiCircle.degreeElements[i].radialDirection=theCircleOf5ths.theDegreeSemiCircle.degreeElements[i].radialDirection.normalize();

            // set circle and degree elements correspondence interlinkage
            theCircleOf5ths.theDegreeSemiCircle.degreeElements[i].CircleIndex=(theCircleOf5ths.theDegreeSemiCircle.OffsetSteps+i+12)%12; 
                    
            if((i == 0)||(i == 1)||(i == 2)) 
                chord_type=0; // majpr
            else
            if((i == 3)||(i == 4)||(i == 5)) 
                chord_type=1; // minor
            else
            if(i == 6)
                chord_type=6; // diminished
            
            theCircleOf5ths.theDegreeSemiCircle.degreeElements[i].chordType=chord_type;
            theCircleOf5ths.Circle5ths[theCircleOf5ths.theDegreeSemiCircle.degreeElements[i].CircleIndex].chordType=chord_type;
            theCircleOf5ths.theDegreeSemiCircle.degreeElements[i].Degree=semiCircleDegrees[(i - theCircleOf5ths.theDegreeSemiCircle.RootKeyCircle5thsPosition+7)%7]; 
           
    }	

    //
    for (int i=1; i<8; ++i)  // for arabic steps  1-7 , i=1 for 1 based indexing
    {	
        for (int j=0; j<7; ++j)  // for semicircle steps
        {
            if (theCircleOf5ths.theDegreeSemiCircle.degreeElements[j].Degree==i)
            {
                arabicStepDegreeSemicircleIndex[i]=j;  
                break;
            }
        }
    }
              
   
};

void ConfigureModuleVars()
{
   ConstructCircle5ths(circle_root_key, mode);
   ConstructDegreesSemicircle(circle_root_key, mode); //int circleroot_key, int mode)
   init_module_vars();  // added in mirack impl
   init_notes();  // depends on mode and root_key		
}


char MSQscaleText[128];


