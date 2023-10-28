// these are module scope vars


/*  Copyright (C) 2019-2022 Ken Chaffin
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
int harmonyPresetChanged=0; 
int savedHarmonySteps = 0;

int semiCircleDegrees[7]={1, 5, 2, 6, 3, 7, 4};  // default order if starting at C
int circleDegreeLookup[8]= {0, 0, 2, 4, 6, 1, 3, 5};  // to convert from arabic roman equivalents to circle degrees
int arabicStepDegreeSemicircleIndex[8];  // where is 1, 2... step in degree semicircle  // [8] so 1 based indexing can be used



//*******************************************


struct HarmonyProgressionStepSettings  // not currently utilized
{
	int CircleOf5thsPosition=0;
	int ChordType=0;
	int Inversion=1;
	int Spread=0;
	int NumNotes=3; 
};

struct HarmonyProgressionStepSettings HarmonicProgression[MAX_STEPS];  // up to 16 steps in a progression

int NumHarmonicProgressionSteps=4;  // what is this?


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
// any mode and root_key is equivalent to a maj key by transposing down these numbers of major scale semitones
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

int mode_root_key_signature_offset[7]={3,0,4,1,5,2,6};  // index into mode_natural_roots[] using the IDPLyMALo = 1,2,3,4,5,6,7 rule for Meander mode ordering

char root_key_name[MAXSHORTSTRLEN];

#define MAX_NOTES_CANDIDATES 130
int  notes[MAX_NOTES_CANDIDATES];

int  num_notes=0;
int  root_key_notes[MAX_ROOT_KEYS][MAX_NOTES_CANDIDATES];

int  num_root_key_notes[MAX_ROOT_KEYS];

int meter_numerator=4;  // need to unify with sig_top...
int meter_denominator=4;

//char   note_desig[MAX_NOTES][MAXSHORTSTRLEN];
//char   note_desig_sharps[MAX_NOTES][MAXSHORTSTRLEN];
//char   note_desig_flats[MAX_NOTES][MAXSHORTSTRLEN];

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
	int STEP_inport_connected_to_Meander_trigger_port=0;
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


struct MeanderState
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
	bool renderKeyboardEnabled=true;
	bool renderScoreEnabled=true;
	bool RootInputSuppliedByRootOutput=false;
	bool ModeInputSuppliedByModeOutput=false;
}	theMeanderState;

 
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

int  step_chord_notes[MAX_STEPS][MAX_NOTES_CANDIDATES];
int  num_step_chord_notes[MAX_STEPS]={};

// Markov 1st order row to column transition probabiliites
float MarkovProgressionTransitionMatrixTemplate[8][8]={  // 8x8 so degrees can be 1 indexed
	{0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00},  // dummy
	{0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00},  // I
	{0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00},  // II
	{0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00},  // III
	{0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00},  // IV
	{0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00},  // V
	{0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00},  // VI
	{0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00}}; // VII
//   dummy  I     II    III   IV    V     VI    VII 

// Markov 1st order row to column transition probabiliites
float MarkovProgressionTransitionMatrix_I_IV_V[8][8]={  // 8x8 so degrees can be 1 indexed
	{0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00},  // dummy
	{0.00, 0.00, 0.00, 0.00, 0.60, 0.40, 0.00, 0.00},  // I
	{0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00},  // II
	{0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00},  // III
	{0.00, 0.20, 0.00, 0.00, 0.00, 0.80, 0.00, 0.00},  // IV
	{0.00, 0.70, 0.00, 0.00, 0.30, 0.00, 0.00, 0.00},  // V
	{0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00},  // VI
	{0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00}}; // VII
//   dummy  I     II    III   IV    V     VI    VII 

// Markov 1st order row to column transition probabiliites
float MarkovProgressionTransitionMatrixBach1[8][8]={  // 8x8 so degrees can be 1 indexed
	{0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00},  // dummy
	{0.00, 0.00, 0.18, 0.01, 0.20, 0.41, 0.09, 0.12},  // I
	{0.00, 0.01, 0.00, 0.03, 0.00, 0.89, 0.00, 0.07},  // II
	{0.00, 0.06, 0.06, 0.00, 0.25, 0.19, 0.31, 0.13},  // III
	{0.00, 0.22, 0.14, 0.00, 0.00, 0.48, 0.00, 0.15},  // IV
	{0.00, 0.80, 0.00, 0.02, 0.06, 0.00, 0.10, 0.20},  // V
	{0.00, 0.03, 0.54, 0.03, 0.14, 0.19, 0.00, 0.08},  // VI
	{0.00, 0.81, 0.00, 0.01, 0.03, 0.15, 0.00, 0.00}}; // VII
//   dummy  I     II    III   IV    V     VI    VII 

// Markov 1st order row to column transition probabiliites
float MarkovProgressionTransitionMatrixBach2[8][8]={  // 8x8 so degrees can be 1 indexed
	{0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00},  // dummy
	{0.00, 0.00, 0.15, 0.01, 0.28, 0.41, 0.09, 0.06},  // I
	{0.00, 0.01, 0.00, 0.00, 0.00, 0.71, 0.01, 0.25},  // II
	{0.00, 0.03, 0.03, 0.00, 0.52, 0.06, 0.32, 0.03},  // III
	{0.00, 0.22, 0.13, 0.00, 0.00, 0.39, 0.02, 0.23},  // IV
	{0.00, 0.82, 0.01, 0.00, 0.07, 0.00, 0.09, 0.00},  // V
	{0.00, 0.15, 0.29, 0.05, 0.11, 0.32, 0.00, 0.09},  // VI
	{0.00, 0.91, 0.00, 0.01, 0.02, 0.04, 0.03, 0.00}}; // VII
//   dummy  I     II    III   IV    V     VI    VII 

// Markov 1st order row to column transition probabiliites
float MarkovProgressionTransitionMatrixMozart1[8][8]={  // 8x8 so degrees can be 1 indexed
	{0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00},  // dummy
	{0.00, 0.00, 0.08, 0.00, 0.07, 0.68, 0.06, 0.11},  // I
	{0.00, 0.37, 0.00, 0.00, 0.00, 0.46, 0.00, 0.17},  // II
	{0.00, 0.00, 0.00, 0.00, 1.00, 0.00, 0.00, 0.00},  // III
	{0.00, 0.42, 0.10, 0.00, 0.00, 0.39, 0.00, 0.09},  // IV
	{0.00, 0.82, 0.00, 0.00, 0.05, 0.00, 0.07, 0.05},  // V
	{0.00, 0.14, 0.51, 0.00, 0.16, 0.05, 0.00, 0.14},  // VI
	{0.00, 0.76, 0.01, 0.00, 0.00, 0.23, 0.00, 0.00}}; // VII
//   dummy  I     II    III   IV    V     VI    VII 

// Markov 1st order row to column transition probabiliites
float MarkovProgressionTransitionMatrixMozart2[8][8]={  // 8x8 so degrees can be 1 indexed
	{0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00},  // dummy
	{0.00, 0.00, 0.13, 0.00, 0.15, 0.62, 0.05, 0.05},  // I
	{0.00, 0.49, 0.00, 0.01, 0.00, 0.40, 0.01, 0.09},  // II
	{0.00, 0.67, 0.00, 0.00, 0.00, 0.00, 0.33, 0.00},  // III
	{0.00, 0.64, 0.14, 0.00, 0.00, 0.15, 0.00, 0.07},  // IV
	{0.00, 0.94, 0.00, 0.00, 0.01, 0.00, 0.04, 0.01},  // V
	{0.00, 0.11, 0.51, 0.00, 0.14, 0.20, 0.00, 0.04},  // VI
	{0.00, 0.82, 0.00, 0.01, 0.01, 0.16, 0.00, 0.00}}; // VII
//   dummy  I     II    III   IV    V     VI    VII 

// Markov 1st order row to column transition probabiliites
float MarkovProgressionTransitionMatrixPalestrina1[8][8]={  // 8x8 so degrees can be 1 indexed
	{0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00},  // dummy
	{0.00, 0.00, 0.15, 0.13, 0.28, 0.14, 0.22, 0.08},  // I
	{0.00, 0.08, 0.00, 0.15, 0.13, 0.28, 0.14, 0.22},  // II
	{0.00, 0.22, 0.08, 0.00, 0.15, 0.13, 0.28, 0.14},  // III
	{0.00, 0.14, 0.22, 0.08, 0.00, 0.15, 0.13, 0.28},  // IV
	{0.00, 0.28, 0.14, 0.22, 0.08, 0.00, 0.15, 0.13},  // V
	{0.00, 0.13, 0.28, 0.14, 0.22, 0.08, 0.00, 0.15},  // VI
	{0.00, 0.15, 0.13, 0.28, 0.14, 0.22, 0.08, 0.00}}; // VII
//   dummy  I     II    III   IV    V     VI    VII 

// Markov 1st order row to column transition probabiliites
float MarkovProgressionTransitionMatrixBeethoven1[8][8]={  // 8x8 so degrees can be 1 indexed
	{0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00},  // dummy
	{0.00, 0.00, 0.10, 0.01, 0.13, 0.52, 0.02, 0.22},  // I
	{0.00, 0.06, 0.00, 0.02, 0.00, 0.87, 0.00, 0.05},  // II
	{0.00, 0.00, 0.00, 0.00, 0.00, 0.67, 0.33, 0.00},  // III
	{0.00, 0.33, 0.03, 0.07, 0.00, 0.40, 0.03, 0.13},  // IV
	{0.00, 0.56, 0.22, 0.01, 0.04, 0.00, 0.07, 0.11},  // V
	{0.00, 0.06, 0.44, 0.00, 0.06, 0.11, 0.00, 0.33},  // VI
	{0.00, 0.80, 0.00, 0.00, 0.03, 0.17, 0.00, 0.00}}; // VII
//   dummy  I     II    III   IV    V     VI    VII 

// Markov 1st order row to column transition probabiliites
float MarkovProgressionTransitionMatrixTraditional1[8][8]={  // 8x8 so degrees can be 1 indexed
	{0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00},  // dummy
	{0.00, 0.00, 0.00, 0.25, 0.25, 0.25, 0.25, 0.00},  // I
	{0.00, 0.00, 0.00, 0.00, 0.00, 0.50, 0.00, 0.50},  // II
	{0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 1.00, 0.00},  // III
	{0.00, 0.25, 0.25, 0.00, 0.00, 0.25, 0.00, 0.25},  // IV
	{0.00, 0.50, 0.00, 0.00, 0.00, 0.00, 0.50, 0.00},  // V
	{0.00, 0.00, 0.50, 0.00, 0.50, 0.00, 0.00, 0.00},  // VI
	{0.00, 0.50, 0.00, 0.00, 0.00, 0.50, 0.00, 0.00}}; // VII
//   dummy  I     II    III   IV    V     VI    VII 

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
	int nmn=Meander_mode_step_intervals[mode][0];  // number of mode notes
	num_notes=0;                                                                
	for (int i=1;i<127;++i)                                                         
	{     
		notes[i]=notes[i-1]+                                                    
			Meander_mode_step_intervals[mode][((i-1)%nmn)+1];  
				    
		++num_notes;                                                            
		if (notes[i]>=127) break;                                               
	}     
																

	for (int j=0;j<12;++j)
	{
		root_key_notes[j][0]=j;
		num_root_key_notes[j]=1;
		

		int num_mode_notes=10*Meander_mode_step_intervals[mode][0]; // the [0] entry is the notes per scale value, times 10 ocatves of midi

		int nmn=Meander_mode_step_intervals[mode][0];  // number of mode notes
		for (int i=1;i<num_mode_notes ;++i)
		{
			root_key_notes[j][i]=root_key_notes[j][i-1]+
		   		Meander_mode_step_intervals[mode][((i-1)%nmn)+1];  
			++num_root_key_notes[j];
		}
			
	}

	char  strng[128];
	strcpy(strng,"");
	for (int i=0;i<Meander_mode_step_intervals[mode][0];++i)
	{
		strcat(strng,note_desig[notes[i]%MAX_NOTES]);
	}
}

void init_custom_harmony()
{
	   	// (harmony_type==4)             /* custom                 */
	    strcpy(theHarmonyTypes[4].harmony_type_desc, "custom" );
		strcpy(theHarmonyTypes[4].harmony_degrees_desc, "I-I-I-I-I-I-I-I-I-I-I-I-I-I-I-I" );
	    theHarmonyTypes[4].num_harmony_steps=16;
		theHarmonyTypes[4].min_steps=1;
	    theHarmonyTypes[4].max_steps=16;
	    for (int i=0;i<theHarmonyTypes[4].num_harmony_steps;++i)
		   theHarmonyTypes[4].harmony_steps[i] = 1; // must not be 0
}

 
void init_harmony()
{
      for (int j=0;j<MAX_HARMONY_TYPES;++j)  // In general, this info will be overridden below by specific harmony type.  Custom type is the exception.
      {
		theHarmonyTypes[j].num_harmony_steps=1;  // just so it is initialized
		theHarmonyTypes[j].min_steps=1;
	    theHarmonyTypes[j].max_steps=16;
		if (j==4) // custom
			strcpy(theHarmonyTypes[j].harmony_type_desc, "custom");
		else
			strcpy(theHarmonyTypes[j].harmony_type_desc, "");
		strcpy(theHarmonyTypes[j].harmony_degrees_desc, "");
        for (int i=0;i<MAX_STEPS;++i)
          {
            theHarmonyTypes[j].harmony_step_chord_type[i]=0; // set to major as a default, may be overridden by specific types
			theHarmonyTypes[j].harmony_steps[i]=1;  // put a valid step in so that if an out of range value is accessed it will not be invalid
          }
      }

	  //semiCircleDegrees[]={1, 5, 2, 6, 3, 7, 4}; 

    // (harmony_type==1)             /* typical classical */  // I + n and descend by 4ths
		strcpy(theHarmonyTypes[1].harmony_type_desc, "50's Classic R&R do-wop and jazz" );
		strcpy(theHarmonyTypes[1].harmony_degrees_desc, "I - VI - II - V" );
	    theHarmonyTypes[1].num_harmony_steps=4;  // steps fixed at 4
		theHarmonyTypes[1].min_steps=4;
	    theHarmonyTypes[1].max_steps=4;
		theHarmonyTypes[1].harmony_steps[0]=1;
		for (int i=1; i<theHarmonyTypes[1].num_harmony_steps; ++i)
		   theHarmonyTypes[1].harmony_steps[i]=(semiCircleDegrees[theHarmonyTypes[1].num_harmony_steps-i])%7;
 	        		
	
    // (harmony_type==2)             /* typical elementary classical */
		strcpy(theHarmonyTypes[2].harmony_type_desc, "elem.. classical 1" );
		strcpy(theHarmonyTypes[2].harmony_degrees_desc, "I - IV - I - V" );
	    theHarmonyTypes[2].num_harmony_steps=4; // steps fixed at 4
		theHarmonyTypes[2].min_steps=4;
	    theHarmonyTypes[2].max_steps=4;
        theHarmonyTypes[2].harmony_steps[0]=1;
        theHarmonyTypes[2].harmony_steps[1]=4;
	    theHarmonyTypes[2].harmony_steps[2]=1;
        theHarmonyTypes[2].harmony_steps[3]=5;
	
	// (harmony_type==3)             /* typical romantic */   // basically alternating between two root_keys, one major and one minor
		strcpy(theHarmonyTypes[3].harmony_type_desc, "romantic - alt root_keys" );
		strcpy(theHarmonyTypes[3].harmony_degrees_desc, "I - IV - V - I - VI - II - III - VI" );
	    theHarmonyTypes[3].num_harmony_steps=8;  // steps fixed at 8
		theHarmonyTypes[3].min_steps=8;
	    theHarmonyTypes[3].max_steps=theHarmonyTypes[3].num_harmony_steps;
        theHarmonyTypes[3].harmony_steps[0]=1;
        theHarmonyTypes[3].harmony_steps[1]=4;
        theHarmonyTypes[3].harmony_steps[2]=5;
        theHarmonyTypes[3].harmony_steps[3]=1;
        theHarmonyTypes[3].harmony_steps[4]=6;
        theHarmonyTypes[3].harmony_steps[5]=2;
        theHarmonyTypes[3].harmony_steps[6]=3;
        theHarmonyTypes[3].harmony_steps[7]=6;
/*	
    // (harmony_type==4)             // custom                
        strcpy(theHarmonyTypes[4].harmony_type_desc, "custom" );
		strcpy(theHarmonyTypes[4].harmony_degrees_desc, "I-I-I-I-I-I-I-I-I-I-I-I-I-I-I-I" );
	    theHarmonyTypes[4].num_harmony_steps=16;
		theHarmonyTypes[4].min_steps=1;
	    theHarmonyTypes[4].max_steps=theHarmonyTypes[4].num_harmony_steps;
		for (int i=0;i<theHarmonyTypes[4].num_harmony_steps;++i)
           theHarmonyTypes[4].harmony_steps[i] = 1; // must not be 0
*/	
    // (harmony_type==5)             /* elementary classical 2 */
		strcpy(theHarmonyTypes[5].harmony_type_desc, "the classic  I - IV - V" );
		strcpy(theHarmonyTypes[5].harmony_degrees_desc, "I - IV - V - I" );
	    theHarmonyTypes[5].num_harmony_steps=4; // steps fixed at 4
		theHarmonyTypes[5].min_steps=4;
	    theHarmonyTypes[5].max_steps=theHarmonyTypes[5].num_harmony_steps;
        theHarmonyTypes[5].harmony_steps[0]=1;
        theHarmonyTypes[5].harmony_steps[1]=4;
        theHarmonyTypes[5].harmony_steps[2]=5;
		theHarmonyTypes[5].harmony_steps[3]=1;

    // (harmony_type==6)             /* elementary classical 3 */
		strcpy(theHarmonyTypes[6].harmony_type_desc, "elem. classical 3" );
		strcpy(theHarmonyTypes[6].harmony_degrees_desc, "I - IV - V - IV" );
	    theHarmonyTypes[6].num_harmony_steps=4; // steps fixed at 4
		theHarmonyTypes[6].min_steps=4;
	    theHarmonyTypes[6].max_steps=theHarmonyTypes[6].num_harmony_steps;
        theHarmonyTypes[6].harmony_steps[0]=1;
        theHarmonyTypes[6].harmony_steps[1]=4;
        theHarmonyTypes[6].harmony_steps[2]=5;
        theHarmonyTypes[6].harmony_steps[3]=4;

    // (harmony_type==7)             /* strong 1 */  
		strcpy(theHarmonyTypes[7].harmony_type_desc, "strong return by 4ths" );
		strcpy(theHarmonyTypes[7].harmony_degrees_desc, "I - III - VI - IV - V" );
		theHarmonyTypes[7].num_harmony_steps=5; // steps fixed at 5
		theHarmonyTypes[7].min_steps=5;
	    theHarmonyTypes[7].max_steps=theHarmonyTypes[7].num_harmony_steps;
        theHarmonyTypes[7].harmony_steps[0]=1;
        theHarmonyTypes[7].harmony_steps[1]=3;
        theHarmonyTypes[7].harmony_steps[2]=6;
        theHarmonyTypes[7].harmony_steps[3]=4;
		theHarmonyTypes[7].harmony_steps[4]=5;
       
     // (harmony_type==8)  // strong random  the harmony chord stays fixed and only the melody varies.  Good for checking harmony meander
	 	strcpy(theHarmonyTypes[8].harmony_type_desc, "stay on I" );
		strcpy(theHarmonyTypes[8].harmony_degrees_desc, "I" );
	    theHarmonyTypes[8].num_harmony_steps=1; // steps fixed at 1
		theHarmonyTypes[8].min_steps=1;
	    theHarmonyTypes[8].max_steps=theHarmonyTypes[8].num_harmony_steps;
        theHarmonyTypes[8].harmony_steps[0]=1;

		//semiCircleDegrees[]={1, 5, 2, 6, 3, 7, 4}; 

     // (harmony_type==9)  // harmonic+   C, G, D,...  CW by 5ths
	     strcpy(theHarmonyTypes[9].harmony_type_desc, "harmonic+ CW 5ths" );
		 strcpy(theHarmonyTypes[9].harmony_degrees_desc, "I - V - II - VI - III - VII - IV" );
	     theHarmonyTypes[9].num_harmony_steps=7;  // 7-7
	     theHarmonyTypes[9].min_steps=7;  // fixed at 7 steps
	     theHarmonyTypes[9].max_steps=theHarmonyTypes[9].num_harmony_steps;
         for (int i=0;i<theHarmonyTypes[9].num_harmony_steps;++i)
       	   theHarmonyTypes[9].harmony_steps[i] = semiCircleDegrees[i]%8;

     // (harmony_type==10)  // harmonic-  C, F#, B,...  CCW by 4ths
	    strcpy(theHarmonyTypes[10].harmony_type_desc, "circle- CCW up by 4ths" );
		strcpy(theHarmonyTypes[10].harmony_degrees_desc, "I - IV - VII - III - VI - II - V" );
	    theHarmonyTypes[10].num_harmony_steps=7;  // 7-7
		theHarmonyTypes[10].min_steps=7;  // fixed at 7 steps
	    theHarmonyTypes[10].max_steps=7;
	 
		for (int i=0;i<theHarmonyTypes[10].num_harmony_steps;++i) 
		   theHarmonyTypes[10].harmony_steps[i] = (semiCircleDegrees[theHarmonyTypes[10].num_harmony_steps-i])%8; 
				   
     // (harmony_type==11)  // tonal+  // C, D, E, F, ...
	    strcpy(theHarmonyTypes[11].harmony_type_desc, "tonal+" );
		strcpy(theHarmonyTypes[11].harmony_degrees_desc, "I - II - III - IV - V - VI - VII" );
	    theHarmonyTypes[11].num_harmony_steps=7;  // 1-7
		theHarmonyTypes[11].min_steps=1;
	    theHarmonyTypes[11].max_steps=theHarmonyTypes[11].num_harmony_steps;
        for (int i=0;i<theHarmonyTypes[11].num_harmony_steps;++i)

		    theHarmonyTypes[11].harmony_steps[i] = 1+ i%7;

     // (harmony_type==12)  // tonal-  // C, B, A, ...
	     strcpy(theHarmonyTypes[12].harmony_type_desc, "tonal-" );
		 strcpy(theHarmonyTypes[12].harmony_degrees_desc, "I - VII - VI - V - IV - III - II" );
	     theHarmonyTypes[12].num_harmony_steps=7;  // 7
		 theHarmonyTypes[12].min_steps=7;  // fixed at 7 steps
	     theHarmonyTypes[12].max_steps=theHarmonyTypes[12].num_harmony_steps;
         for (int i=0;i<theHarmonyTypes[12].num_harmony_steps;++i)
		     theHarmonyTypes[12].harmony_steps[i] = 1+ (7-i)%7;

    //semiCircleDegrees[]={1, 5, 2, 6, 3, 7, 4}; 
        
    // (harmony_type==13)             /* 12 bar blues classical*/
	    strcpy(theHarmonyTypes[13].harmony_type_desc, "12 bar blues 1 traditional" );
		strcpy(theHarmonyTypes[13].harmony_degrees_desc, "I - I - I - I - IV - IV - I - I - V - V - I - I" );
	    meter_numerator=3;
        meter_denominator=4;
        theHarmonyTypes[13].num_harmony_steps=12;
		theHarmonyTypes[13].min_steps=1;
	    theHarmonyTypes[13].max_steps=theHarmonyTypes[13].num_harmony_steps; 
        theHarmonyTypes[13].harmony_steps[0]=1;
        theHarmonyTypes[13].harmony_steps[1]=1;
        theHarmonyTypes[13].harmony_steps[2]=1;
        theHarmonyTypes[13].harmony_steps[3]=1;
        theHarmonyTypes[13].harmony_steps[4]=4;
        theHarmonyTypes[13].harmony_steps[5]=4;
        theHarmonyTypes[13].harmony_steps[6]=1;
        theHarmonyTypes[13].harmony_steps[7]=1;
        theHarmonyTypes[13].harmony_steps[8]=5;
        theHarmonyTypes[13].harmony_steps[9]=5;
        theHarmonyTypes[13].harmony_steps[10]=1;
	    theHarmonyTypes[13].harmony_steps[11]=1;
       

    // (harmony_type==14)             /* shuffle  12 bar blues */
		strcpy(theHarmonyTypes[14].harmony_type_desc, "12 bar blues 2 shuffle" );
		strcpy(theHarmonyTypes[14].harmony_degrees_desc, "I - I - I - I - IV - IV - I - I - V - IV - I - I" );
	    meter_numerator=3;
        meter_denominator=4;
        theHarmonyTypes[14]. num_harmony_steps=12;
		theHarmonyTypes[14].min_steps=1;
	    theHarmonyTypes[14].max_steps=theHarmonyTypes[14].num_harmony_steps;
        theHarmonyTypes[14].harmony_steps[0]=1;
        theHarmonyTypes[14].harmony_steps[1]=1;
        theHarmonyTypes[14].harmony_steps[2]=1;
        theHarmonyTypes[14].harmony_steps[3]=1;
        theHarmonyTypes[14].harmony_steps[4]=4;
        theHarmonyTypes[14].harmony_steps[5]=4;
        theHarmonyTypes[14].harmony_steps[6]=1;
        theHarmonyTypes[14].harmony_steps[7]=1;
        theHarmonyTypes[14].harmony_steps[8]=5;
        theHarmonyTypes[14].harmony_steps[9]=4;
        theHarmonyTypes[14].harmony_steps[10]=1;
        theHarmonyTypes[14].harmony_steps[11]=1;
       
    // (harmony_type==15)             /* country 1 */
		strcpy(theHarmonyTypes[15].harmony_type_desc, "country 1" );
		strcpy(theHarmonyTypes[15].harmony_degrees_desc, "I - IV - V - I - I - IV - V - I" );
	    meter_numerator=4;
        meter_denominator=4;
        theHarmonyTypes[15].num_harmony_steps=8;
		theHarmonyTypes[15].min_steps=1;
	    theHarmonyTypes[15].max_steps=theHarmonyTypes[15].num_harmony_steps;
        theHarmonyTypes[15].harmony_steps[0]=1;
        theHarmonyTypes[15].harmony_steps[1]=4;
        theHarmonyTypes[15].harmony_steps[2]=5;
        theHarmonyTypes[15].harmony_steps[3]=1;
        theHarmonyTypes[15].harmony_steps[4]=1;
        theHarmonyTypes[15].harmony_steps[5]=4;
        theHarmonyTypes[15].harmony_steps[6]=5;
        theHarmonyTypes[15].harmony_steps[7]=1;
        

    // (harmony_type==16)             /* country 2 */
	    strcpy(theHarmonyTypes[16].harmony_type_desc, "country 2" );
		strcpy(theHarmonyTypes[16].harmony_degrees_desc, "I - I - V - V - IV - IV - I - I" );
	    meter_numerator=4;
        meter_denominator=4;
        theHarmonyTypes[16].num_harmony_steps=8;
		theHarmonyTypes[16].min_steps=1;
	    theHarmonyTypes[16].max_steps=theHarmonyTypes[16].num_harmony_steps;
        theHarmonyTypes[16].harmony_steps[0]=1;
        theHarmonyTypes[16].harmony_steps[1]=1;
        theHarmonyTypes[16].harmony_steps[2]=5;
        theHarmonyTypes[16].harmony_steps[3]=5;
        theHarmonyTypes[16].harmony_steps[4]=4;
        theHarmonyTypes[16].harmony_steps[5]=4;
        theHarmonyTypes[16].harmony_steps[6]=1;
        theHarmonyTypes[16].harmony_steps[7]=1;

    // (harmony_type==17)             /* country 3 */
	    strcpy(theHarmonyTypes[17].harmony_type_desc, "country 3" );
		strcpy(theHarmonyTypes[17].harmony_degrees_desc, "I - IV - I - V - I - IV - V - I" );
	    meter_numerator=4;
        meter_denominator=4;
        theHarmonyTypes[17].num_harmony_steps=8;
		theHarmonyTypes[17].min_steps=1;
	    theHarmonyTypes[17].max_steps=theHarmonyTypes[17].num_harmony_steps;
        theHarmonyTypes[17].harmony_steps[0]=1;
        theHarmonyTypes[17].harmony_steps[1]=4;
        theHarmonyTypes[17].harmony_steps[2]=1;
        theHarmonyTypes[17].harmony_steps[3]=5;
        theHarmonyTypes[17].harmony_steps[4]=1;
        theHarmonyTypes[17].harmony_steps[5]=4;
        theHarmonyTypes[17].harmony_steps[6]=5;
        theHarmonyTypes[17].harmony_steps[7]=1;
        

    // (harmony_type==18)             /* 50's r&r  */
		strcpy(theHarmonyTypes[18].harmony_type_desc, "50's R&R" );
		strcpy(theHarmonyTypes[18].harmony_degrees_desc, "I - VI - IV - V" );
	    meter_numerator=4;
        meter_denominator=4;
        theHarmonyTypes[18].num_harmony_steps=4;
		theHarmonyTypes[18].min_steps=1;
	    theHarmonyTypes[18].max_steps=theHarmonyTypes[18].num_harmony_steps;
        theHarmonyTypes[18].harmony_steps[0]=1;
        theHarmonyTypes[18].harmony_steps[1]=6;
        theHarmonyTypes[18].harmony_steps[2]=4;
        theHarmonyTypes[18].harmony_steps[3]=5;
       

    // (harmony_type==19)             /* Rock1     */
		strcpy(theHarmonyTypes[19].harmony_type_desc, "rock plagal cadence" );  // a plagal cadence
		strcpy(theHarmonyTypes[19].harmony_degrees_desc, "I - IV" );
	    meter_numerator=4;
        meter_denominator=4;
        theHarmonyTypes[19].num_harmony_steps=2;
		theHarmonyTypes[19].min_steps=1;
	    theHarmonyTypes[19].max_steps=theHarmonyTypes[19].num_harmony_steps;
        theHarmonyTypes[19].harmony_steps[0]=1;
        theHarmonyTypes[19].harmony_steps[1]=4;
      
    // (harmony_type==20)             /* Folk1     */
		strcpy(theHarmonyTypes[20].harmony_type_desc, "folk 1" );
		strcpy(theHarmonyTypes[20].harmony_degrees_desc, "I - V - I - V" );
	    meter_numerator=4;
        meter_denominator=4;
        theHarmonyTypes[20].num_harmony_steps=4;
		theHarmonyTypes[20].min_steps=1;
	    theHarmonyTypes[20].max_steps=theHarmonyTypes[20].num_harmony_steps;
        theHarmonyTypes[20].harmony_steps[0]=1;
        theHarmonyTypes[20].harmony_steps[1]=5;
        theHarmonyTypes[20].harmony_steps[2]=1;
        theHarmonyTypes[20].harmony_steps[3]=5;
        

    // (harmony_type==21)             /* folk2 */
		strcpy(theHarmonyTypes[21].harmony_type_desc, "folk 2" );
		strcpy(theHarmonyTypes[21].harmony_degrees_desc, "I - I - I - V - V - V - I" );
	    meter_numerator=4;
        meter_denominator=4;
        theHarmonyTypes[21].num_harmony_steps=8;
		theHarmonyTypes[21].min_steps=1;
	    theHarmonyTypes[21].max_steps=theHarmonyTypes[21].num_harmony_steps;
        theHarmonyTypes[21].harmony_steps[0]=1;
        theHarmonyTypes[21].harmony_steps[1]=1;
        theHarmonyTypes[21].harmony_steps[2]=1;
        theHarmonyTypes[21].harmony_steps[3]=5;
        theHarmonyTypes[21].harmony_steps[4]=5;
        theHarmonyTypes[21].harmony_steps[5]=5;
        theHarmonyTypes[21].harmony_steps[6]=5;
        theHarmonyTypes[21].harmony_steps[7]=1;
       
	
		// (harmony_type==22)             /* random coming home by 4ths */  // I + n and descend by 4ths
		strcpy(theHarmonyTypes[22].harmony_type_desc, "random coming home by 4ths" );
		strcpy(theHarmonyTypes[22].harmony_degrees_desc, "I - VI - II - V" );
	    theHarmonyTypes[22].num_harmony_steps=5;  // 5-5
		theHarmonyTypes[22].min_steps=5;  // steps fixed at 5
	    theHarmonyTypes[22].max_steps=theHarmonyTypes[22].num_harmony_steps;
		theHarmonyTypes[22].harmony_steps[0]=1;
		for (int i=1; i<theHarmonyTypes[22].num_harmony_steps; ++i)
		   theHarmonyTypes[22].harmony_steps[i]=(semiCircleDegrees[theHarmonyTypes[22].num_harmony_steps-i])%7;

		// (harmony_type==23)             /* random coming home */  // I + n and descend by 4ths
		strcpy(theHarmonyTypes[23].harmony_type_desc, "random order" );
		strcpy(theHarmonyTypes[23].harmony_degrees_desc, "I - IV - V" );
	    theHarmonyTypes[23].num_harmony_steps=3;  // 1-7
		theHarmonyTypes[23].min_steps=1;
	    theHarmonyTypes[23].max_steps=theHarmonyTypes[23].num_harmony_steps;
		theHarmonyTypes[23].harmony_steps[0]=1;
		theHarmonyTypes[23].harmony_steps[1]=4;
		theHarmonyTypes[23].harmony_steps[2]=5;

		// (harmony_type==24)             /* Hallelujah */  // 
		strcpy(theHarmonyTypes[24].harmony_type_desc, "Hallelujah" );
		strcpy(theHarmonyTypes[24].harmony_degrees_desc, "I - VI - I - VI - IV - V - I - I - I - IV - V - VI - IV - V - III - VI" );
	    theHarmonyTypes[24].num_harmony_steps=16;  // 1-8
		theHarmonyTypes[24].min_steps=1;
	    theHarmonyTypes[24].max_steps=theHarmonyTypes[24].num_harmony_steps;
		theHarmonyTypes[24].harmony_steps[0]=1;
		theHarmonyTypes[24].harmony_steps[1]=6;
		theHarmonyTypes[24].harmony_steps[2]=1;
		theHarmonyTypes[24].harmony_steps[3]=6;
		theHarmonyTypes[24].harmony_steps[4]=4;
		theHarmonyTypes[24].harmony_steps[5]=5;
		theHarmonyTypes[24].harmony_steps[6]=1;
		theHarmonyTypes[24].harmony_steps[7]=1;

		theHarmonyTypes[24].harmony_steps[8]=1;
		theHarmonyTypes[24].harmony_steps[9]=4;
		theHarmonyTypes[24].harmony_steps[10]=5;
		theHarmonyTypes[24].harmony_steps[11]=6;
		theHarmonyTypes[24].harmony_steps[12]=4;
		theHarmonyTypes[24].harmony_steps[13]=5;
		theHarmonyTypes[24].harmony_steps[14]=3;
		theHarmonyTypes[24].harmony_steps[15]=6;
		
		// (harmony_type==25)             /* Pachelbel Canon*/  // 
		strcpy(theHarmonyTypes[25].harmony_type_desc, "Canon - DMaj" );
		strcpy(theHarmonyTypes[25].harmony_degrees_desc, "I - V - VI - III - IV - I - IV - V" );
	    theHarmonyTypes[25].num_harmony_steps=8;  // 1-8
		theHarmonyTypes[25].min_steps=1;
	    theHarmonyTypes[25].max_steps=theHarmonyTypes[25].num_harmony_steps;
		theHarmonyTypes[25].harmony_steps[0]=1;
		theHarmonyTypes[25].harmony_steps[1]=5;
		theHarmonyTypes[25].harmony_steps[2]=6;
		theHarmonyTypes[25].harmony_steps[3]=3;
		theHarmonyTypes[25].harmony_steps[4]=4;
		theHarmonyTypes[25].harmony_steps[5]=1;
		theHarmonyTypes[25].harmony_steps[6]=4;
		theHarmonyTypes[25].harmony_steps[7]=5;

		// (harmony_type==26)             /* Pop Rock Classic-1*/  // 
		strcpy(theHarmonyTypes[26].harmony_type_desc, "Pop Rock Classic Sensitive" );
		strcpy(theHarmonyTypes[26].harmony_degrees_desc, "I - V - VI - IV" );
	    theHarmonyTypes[26].num_harmony_steps=4;  // 1-8
		theHarmonyTypes[26].min_steps=1;
	    theHarmonyTypes[26].max_steps=theHarmonyTypes[26].num_harmony_steps;
		theHarmonyTypes[26].harmony_steps[0]=1;
		theHarmonyTypes[26].harmony_steps[1]=5;
		theHarmonyTypes[26].harmony_steps[2]=6;
		theHarmonyTypes[26].harmony_steps[3]=4;
		
		// (harmony_type==27)             /* Andalusion Cadence 1*/  // 
		strcpy(theHarmonyTypes[27].harmony_type_desc, "Andalusion Cadence 1" );
		strcpy(theHarmonyTypes[27].harmony_degrees_desc, "I - VII - VI - V" );
	    theHarmonyTypes[27].num_harmony_steps=4;  // 1-8
		theHarmonyTypes[27].min_steps=1;
	    theHarmonyTypes[27].max_steps=theHarmonyTypes[27].num_harmony_steps;
		theHarmonyTypes[27].harmony_steps[0]=1;
		theHarmonyTypes[27].harmony_steps[1]=7;
		theHarmonyTypes[27].harmony_steps[2]=6;
		theHarmonyTypes[27].harmony_steps[3]=5;
		
		// (harmony_type==28)             /* 16 bar blues*/  // 
		strcpy(theHarmonyTypes[28].harmony_type_desc, "16 Bar Blues" );
		strcpy(theHarmonyTypes[28].harmony_degrees_desc, "I - I - I - I - I - I - I - I - IV - IV - I - I - V - IV - I - I" );
	    theHarmonyTypes[28].num_harmony_steps=16;  // 1-8
		theHarmonyTypes[28].min_steps=1;
	    theHarmonyTypes[28].max_steps=theHarmonyTypes[28].num_harmony_steps;
		theHarmonyTypes[28].harmony_steps[0]=1;
		theHarmonyTypes[28].harmony_steps[1]=1;
		theHarmonyTypes[28].harmony_steps[2]=1;
		theHarmonyTypes[28].harmony_steps[3]=1;
		theHarmonyTypes[28].harmony_steps[4]=1;
		theHarmonyTypes[28].harmony_steps[5]=1;
		theHarmonyTypes[28].harmony_steps[6]=1;
		theHarmonyTypes[28].harmony_steps[7]=1;

		theHarmonyTypes[28].harmony_steps[8]=4;
		theHarmonyTypes[28].harmony_steps[9]=4;
		theHarmonyTypes[28].harmony_steps[10]=1;
		theHarmonyTypes[28].harmony_steps[11]=1;
		theHarmonyTypes[28].harmony_steps[12]=5;
		theHarmonyTypes[28].harmony_steps[13]=4;
		theHarmonyTypes[28].harmony_steps[14]=1;
		theHarmonyTypes[28].harmony_steps[15]=1;
		
		
		// (harmony_type==29)             /* Black */  // 
		strcpy(theHarmonyTypes[29].harmony_type_desc, "Black Stones" );
		strcpy(theHarmonyTypes[29].harmony_degrees_desc, "I - VII - III - VII - I - I - I - I - I - VII - III - VII - IV - IV - V - V" );
	    theHarmonyTypes[29].num_harmony_steps=16;  // 1-8
		theHarmonyTypes[29].min_steps=1;
	    theHarmonyTypes[29].max_steps=theHarmonyTypes[29].num_harmony_steps;
		theHarmonyTypes[29].harmony_steps[0]=1;
		theHarmonyTypes[29].harmony_steps[1]=7;
		theHarmonyTypes[29].harmony_steps[2]=3;
		theHarmonyTypes[29].harmony_steps[3]=7;
		theHarmonyTypes[29].harmony_steps[4]=1;
		theHarmonyTypes[29].harmony_steps[5]=1;
		theHarmonyTypes[29].harmony_steps[6]=1;
		theHarmonyTypes[29].harmony_steps[7]=1;

		theHarmonyTypes[29].harmony_steps[8]=1;
		theHarmonyTypes[29].harmony_steps[9]=7;
		theHarmonyTypes[29].harmony_steps[10]=3;
		theHarmonyTypes[29].harmony_steps[11]=7;
		theHarmonyTypes[29].harmony_steps[12]=4;
		theHarmonyTypes[29].harmony_steps[13]=4;
		theHarmonyTypes[29].harmony_steps[14]=5;
		theHarmonyTypes[29].harmony_steps[15]=5;

		// (harmony_type==30)             /*V-I */  // 
		strcpy(theHarmonyTypes[30].harmony_type_desc, "V - I" ); 
		strcpy(theHarmonyTypes[30].harmony_degrees_desc, "V - I" );
	    theHarmonyTypes[30].num_harmony_steps=2;  // 1-8
		theHarmonyTypes[30].min_steps=1;
	    theHarmonyTypes[30].max_steps=theHarmonyTypes[30].num_harmony_steps;
		theHarmonyTypes[30].harmony_steps[0]=5;
		theHarmonyTypes[30].harmony_steps[1]=1;

		// (harmony_type==31)             /* Markov Chain  Bach 1*/  // 
		strcpy(theHarmonyTypes[31].harmony_type_desc, "Markov Chain-Bach 1" );
		strcpy(theHarmonyTypes[31].harmony_degrees_desc, "I - II - III - IV - V - VI - VII" );
	    theHarmonyTypes[31].num_harmony_steps=7;  // 1-8
		theHarmonyTypes[31].min_steps=1;
	    theHarmonyTypes[31].max_steps=theHarmonyTypes[31].num_harmony_steps;
		theHarmonyTypes[31].harmony_steps[0]=1;
		theHarmonyTypes[31].harmony_steps[1]=2;
		theHarmonyTypes[31].harmony_steps[2]=3;
		theHarmonyTypes[31].harmony_steps[3]=4;
		theHarmonyTypes[31].harmony_steps[4]=5;
		theHarmonyTypes[31].harmony_steps[5]=6;
		theHarmonyTypes[31].harmony_steps[6]=7;

		// (harmony_type==32)             /* Pop */  // 
		strcpy(theHarmonyTypes[32].harmony_type_desc, "Pop " );
		strcpy(theHarmonyTypes[32].harmony_degrees_desc, "I - II - IV - V" );
	    theHarmonyTypes[32].num_harmony_steps=4;  // 1-8
		theHarmonyTypes[32].min_steps=1;
	    theHarmonyTypes[32].max_steps=theHarmonyTypes[32].num_harmony_steps;
		theHarmonyTypes[32].harmony_steps[0]=1;
		theHarmonyTypes[32].harmony_steps[1]=2;
		theHarmonyTypes[32].harmony_steps[2]=4;
		theHarmonyTypes[32].harmony_steps[3]=5;
		
		// (harmony_type==33)             /* Classical */  // 
		strcpy(theHarmonyTypes[33].harmony_type_desc, "Classical" );
		strcpy(theHarmonyTypes[33].harmony_degrees_desc, "I - V - I - VI - II - V - I" );
	    theHarmonyTypes[33].num_harmony_steps=7;  // 1-8
		theHarmonyTypes[33].min_steps=1;
	    theHarmonyTypes[33].max_steps=theHarmonyTypes[33].num_harmony_steps;
		theHarmonyTypes[33].harmony_steps[0]=1;
		theHarmonyTypes[33].harmony_steps[1]=5;
		theHarmonyTypes[33].harmony_steps[2]=1;
		theHarmonyTypes[33].harmony_steps[3]=6;
		theHarmonyTypes[33].harmony_steps[4]=2;
		theHarmonyTypes[33].harmony_steps[5]=5;
		theHarmonyTypes[33].harmony_steps[6]=1;

		// (harmony_type==34)             /*Mozart */  // 
		strcpy(theHarmonyTypes[34].harmony_type_desc, "Mozart " );
		strcpy(theHarmonyTypes[34].harmony_degrees_desc, "I - II - V - I" );
	    theHarmonyTypes[34].num_harmony_steps=4;  // 1-8
		theHarmonyTypes[34].min_steps=1;
	    theHarmonyTypes[34].max_steps=theHarmonyTypes[34].num_harmony_steps; 
		theHarmonyTypes[34].harmony_steps[0]=1;
		theHarmonyTypes[34].harmony_steps[1]=2;
		theHarmonyTypes[34].harmony_steps[2]=5;
		theHarmonyTypes[34].harmony_steps[3]=1;

		// (harmony_type==35)             /*Classical Tonal */  // 
		strcpy(theHarmonyTypes[35].harmony_type_desc, "Classical Tonal" );
		strcpy(theHarmonyTypes[35].harmony_degrees_desc, "I - V - I - IV" );
	    theHarmonyTypes[35].num_harmony_steps=4;  // 1-8
		theHarmonyTypes[35].min_steps=1;
	    theHarmonyTypes[35].max_steps=theHarmonyTypes[35].num_harmony_steps;
		theHarmonyTypes[35].harmony_steps[0]=1;
		theHarmonyTypes[35].harmony_steps[1]=5;
		theHarmonyTypes[35].harmony_steps[2]=1;
		theHarmonyTypes[35].harmony_steps[3]=4;
		

		// (harmony_type==36)             /*Sensitive */  // 
		strcpy(theHarmonyTypes[36].harmony_type_desc, "Sensitive" );
		strcpy(theHarmonyTypes[36].harmony_degrees_desc, "VI - IV - I - V" );
	    theHarmonyTypes[36].num_harmony_steps=4;  // 1-8
		theHarmonyTypes[36].min_steps=1;
	    theHarmonyTypes[36].max_steps=theHarmonyTypes[36].num_harmony_steps;
		theHarmonyTypes[36].harmony_steps[0]=6;
		theHarmonyTypes[36].harmony_steps[1]=4;
		theHarmonyTypes[36].harmony_steps[2]=1;
		theHarmonyTypes[36].harmony_steps[3]=5;
		
		// (harmony_type==37)             /*Jazz */  // 
		strcpy(theHarmonyTypes[37].harmony_type_desc, "Jazz" );
		strcpy(theHarmonyTypes[37].harmony_degrees_desc, "II - V - I" );
	    theHarmonyTypes[37].num_harmony_steps=3;  // 1-8
		theHarmonyTypes[37].min_steps=1;
	    theHarmonyTypes[37].max_steps=theHarmonyTypes[37].num_harmony_steps;
		theHarmonyTypes[37].harmony_steps[0]=2;
		theHarmonyTypes[37].harmony_steps[1]=5;
		theHarmonyTypes[37].harmony_steps[2]=1;

		// (harmony_type==38)             /*Pop */  // 
		strcpy(theHarmonyTypes[38].harmony_type_desc, "Pop and jazz" );
		strcpy(theHarmonyTypes[38].harmony_degrees_desc, "I - IV - II - V" );
	    theHarmonyTypes[38].num_harmony_steps=4;  // 1-8
		theHarmonyTypes[38].min_steps=1;
	    theHarmonyTypes[38].max_steps=theHarmonyTypes[38].num_harmony_steps;
		theHarmonyTypes[38].harmony_steps[0]=1;
		theHarmonyTypes[38].harmony_steps[1]=4;
		theHarmonyTypes[38].harmony_steps[2]=2;
		theHarmonyTypes[38].harmony_steps[3]=5;

		// (harmony_type==39)             /*Pop */  // 
		strcpy(theHarmonyTypes[39].harmony_type_desc, "Pop" );
		strcpy(theHarmonyTypes[39].harmony_degrees_desc, "I - II - III - IV - V" );
	    theHarmonyTypes[39].num_harmony_steps=5;  // 1-8
		theHarmonyTypes[39].min_steps=1;
	    theHarmonyTypes[39].max_steps=theHarmonyTypes[39].num_harmony_steps;
		theHarmonyTypes[39].harmony_steps[0]=1;
		theHarmonyTypes[39].harmony_steps[1]=2;
		theHarmonyTypes[39].harmony_steps[2]=3;
		theHarmonyTypes[39].harmony_steps[3]=4;
		theHarmonyTypes[39].harmony_steps[4]=5;

		// (harmony_type==40)             /*Pop */  // 
		strcpy(theHarmonyTypes[40].harmony_type_desc, "Pop" );
		strcpy(theHarmonyTypes[40].harmony_degrees_desc, "I - III - IV - IV" );  // can't really do a IV and iv together
	    theHarmonyTypes[40].num_harmony_steps=4;  // 1-8
		theHarmonyTypes[40].min_steps=1;
	    theHarmonyTypes[40].max_steps=theHarmonyTypes[40].num_harmony_steps;
		theHarmonyTypes[40].harmony_steps[0]=1;
		theHarmonyTypes[40].harmony_steps[1]=3;
		theHarmonyTypes[40].harmony_steps[2]=4;
		theHarmonyTypes[40].harmony_steps[3]=4;

		// (harmony_type==41)             /*Andalusian Cadence 2 */  // 
		strcpy(theHarmonyTypes[41].harmony_type_desc, "Andalusian Cadence 2" );
		strcpy(theHarmonyTypes[41].harmony_degrees_desc, "VI - V - IV - III" );
	    theHarmonyTypes[41].num_harmony_steps=4;  // 1-8
		theHarmonyTypes[41].min_steps=1;
	    theHarmonyTypes[41].max_steps=theHarmonyTypes[41].num_harmony_steps;
		theHarmonyTypes[41].harmony_steps[0]=6;
		theHarmonyTypes[41].harmony_steps[1]=5;
		theHarmonyTypes[41].harmony_steps[2]=4;
		theHarmonyTypes[41].harmony_steps[3]=3;
	
		// (harmony_type==42)             /* Markov Chain  Bach 2*/  // 
		strcpy(theHarmonyTypes[42].harmony_type_desc, "Markov Chain - Bach 2" );
		strcpy(theHarmonyTypes[42].harmony_degrees_desc, "I - II - III - IV - V - VI - VII" );
	    theHarmonyTypes[42].num_harmony_steps=7;  // 1-8
		theHarmonyTypes[42].min_steps=1;
	    theHarmonyTypes[42].max_steps=theHarmonyTypes[42].num_harmony_steps;
		theHarmonyTypes[42].harmony_steps[0]=1;
		theHarmonyTypes[42].harmony_steps[1]=2;
		theHarmonyTypes[42].harmony_steps[2]=3;
		theHarmonyTypes[42].harmony_steps[3]=4;
		theHarmonyTypes[42].harmony_steps[4]=5;
		theHarmonyTypes[42].harmony_steps[5]=6;
		theHarmonyTypes[42].harmony_steps[6]=7;

		// (harmony_type==43)             /* Markov Chain Mozart 1*/  // 
		strcpy(theHarmonyTypes[43].harmony_type_desc, "Markov Chain-Mozart 1" );
		strcpy(theHarmonyTypes[43].harmony_degrees_desc, "I - II - III - IV - V - VI - VII" );
	    theHarmonyTypes[43].num_harmony_steps=7;  // 1-8
		theHarmonyTypes[43].min_steps=1;
	    theHarmonyTypes[43].max_steps=theHarmonyTypes[43].num_harmony_steps;
		theHarmonyTypes[43].harmony_steps[0]=1;
		theHarmonyTypes[43].harmony_steps[1]=2;
		theHarmonyTypes[43].harmony_steps[2]=3;
		theHarmonyTypes[43].harmony_steps[3]=4;
		theHarmonyTypes[43].harmony_steps[4]=5;
		theHarmonyTypes[43].harmony_steps[5]=6;
		theHarmonyTypes[43].harmony_steps[6]=7;

		// (harmony_type==44)             /* Markov Chain Mozart 2*/  // 
		strcpy(theHarmonyTypes[44].harmony_type_desc, "Markov Chain-Mozart 2" );
		strcpy(theHarmonyTypes[44].harmony_degrees_desc, "I - II - III - IV - V - VI - VII" );
	    theHarmonyTypes[44].num_harmony_steps=7;  // 1 - 8
		theHarmonyTypes[44].min_steps=1;
	    theHarmonyTypes[44].max_steps=theHarmonyTypes[44].num_harmony_steps;
		theHarmonyTypes[44].harmony_steps[0]=1;
		theHarmonyTypes[44].harmony_steps[1]=2;
		theHarmonyTypes[44].harmony_steps[2]=3;
		theHarmonyTypes[44].harmony_steps[3]=4;
		theHarmonyTypes[44].harmony_steps[4]=5;
		theHarmonyTypes[44].harmony_steps[5]=6;
		theHarmonyTypes[44].harmony_steps[6]=7;

		// (harmony_type==45)             /* Markov Chain Palestrina 1*/  // 
		strcpy(theHarmonyTypes[45].harmony_type_desc, "Markov Chain-Palestrina 1" );
		strcpy(theHarmonyTypes[45].harmony_degrees_desc, "I - II - III - IV - V - VI - VII" );
	    theHarmonyTypes[45].num_harmony_steps=7;  // 1-8
		theHarmonyTypes[45].min_steps=1;
	    theHarmonyTypes[45].max_steps=theHarmonyTypes[45].num_harmony_steps;
		theHarmonyTypes[45].harmony_steps[0]=1;
		theHarmonyTypes[45].harmony_steps[1]=2;
		theHarmonyTypes[45].harmony_steps[2]=3;
		theHarmonyTypes[45].harmony_steps[3]=4;
		theHarmonyTypes[45].harmony_steps[4]=5;
		theHarmonyTypes[45].harmony_steps[5]=6;
		theHarmonyTypes[45].harmony_steps[6]=7;

		// (harmony_type==46)             /* Markov Chain Beethoven 1*/  // 
		strcpy(theHarmonyTypes[46].harmony_type_desc, "Markov Chain-Beethoven 1" );
		strcpy(theHarmonyTypes[46].harmony_degrees_desc, "I - II - III - IV - V - VI - VII" );
	    theHarmonyTypes[46].num_harmony_steps=7;  // 1-8
		theHarmonyTypes[46].min_steps=1;
	    theHarmonyTypes[46].max_steps=theHarmonyTypes[46].num_harmony_steps;
		theHarmonyTypes[46].harmony_steps[0]=1;
		theHarmonyTypes[46].harmony_steps[1]=2;
		theHarmonyTypes[46].harmony_steps[2]=3;
		theHarmonyTypes[46].harmony_steps[3]=4;
		theHarmonyTypes[46].harmony_steps[4]=5;
		theHarmonyTypes[46].harmony_steps[5]=6;
		theHarmonyTypes[46].harmony_steps[6]=7;

		// (harmony_type==47)             /* Markov Chain Traditional 1*/  // 
		strcpy(theHarmonyTypes[47].harmony_type_desc, "Markov Chain-Traditional 1" );
		strcpy(theHarmonyTypes[47].harmony_degrees_desc, "I - II - III - IV - V - VI - VII" );
	    theHarmonyTypes[47].num_harmony_steps=7;  // 1-8
		theHarmonyTypes[47].min_steps=1;
	    theHarmonyTypes[47].max_steps=theHarmonyTypes[47].num_harmony_steps;
		theHarmonyTypes[47].harmony_steps[0]=1;
		theHarmonyTypes[47].harmony_steps[1]=2;
		theHarmonyTypes[47].harmony_steps[2]=3;
		theHarmonyTypes[47].harmony_steps[3]=4;
		theHarmonyTypes[47].harmony_steps[4]=5;
		theHarmonyTypes[47].harmony_steps[5]=6;
		theHarmonyTypes[47].harmony_steps[6]=7;

		// (harmony_type==48)             /* Markov Chain I-IV-V*/  // 
		strcpy(theHarmonyTypes[48].harmony_type_desc, "Markov Chain- I - IV - V" );
		strcpy(theHarmonyTypes[48].harmony_degrees_desc, "I - II - III - IV - V - VI - VII" );
	    theHarmonyTypes[48].num_harmony_steps=7;  // 1-8
		theHarmonyTypes[48].min_steps=1;
	    theHarmonyTypes[48].max_steps=theHarmonyTypes[48].num_harmony_steps;
		theHarmonyTypes[48].harmony_steps[0]=1;
		theHarmonyTypes[48].harmony_steps[1]=2;
		theHarmonyTypes[48].harmony_steps[2]=3;
		theHarmonyTypes[48].harmony_steps[3]=4;
		theHarmonyTypes[48].harmony_steps[4]=5;
		theHarmonyTypes[48].harmony_steps[5]=6;
		theHarmonyTypes[48].harmony_steps[6]=7;

		// (harmony_type==49)             /* Jazz 2 */  // 
		strcpy(theHarmonyTypes[49].harmony_type_desc, "Jazz 2" );
		strcpy(theHarmonyTypes[49].harmony_degrees_desc, "I - VI - II - V" );
	    theHarmonyTypes[49].num_harmony_steps=4;  // 1-8
		theHarmonyTypes[49].min_steps=1;
	    theHarmonyTypes[49].max_steps=theHarmonyTypes[49].num_harmony_steps;
		theHarmonyTypes[49].harmony_steps[0]=1;
		theHarmonyTypes[49].harmony_steps[1]=6; 
		theHarmonyTypes[49].harmony_steps[2]=2;
		theHarmonyTypes[49].harmony_steps[3]=5;

		// (harmony_type==50)             /*Jazz 3 */  // 
		strcpy(theHarmonyTypes[50].harmony_type_desc, "Jazz 3" );
		strcpy(theHarmonyTypes[50].harmony_degrees_desc, "III - VI - II - V" );
	    theHarmonyTypes[50].num_harmony_steps=4;  // 1-8
		theHarmonyTypes[50].min_steps=1;
	    theHarmonyTypes[50].max_steps=theHarmonyTypes[50].num_harmony_steps;
		theHarmonyTypes[50].harmony_steps[0]=3;
		theHarmonyTypes[50].harmony_steps[1]=6;
		theHarmonyTypes[50].harmony_steps[2]=2;
		theHarmonyTypes[50].harmony_steps[3]=5;

		// (harmony_type==51)             /*Jazz 4 */  // 
		strcpy(theHarmonyTypes[51].harmony_type_desc, "Jazz 4" );
		strcpy(theHarmonyTypes[51].harmony_degrees_desc, "I - IV - III - VI" );
	    theHarmonyTypes[51].num_harmony_steps=4;  // 1-8
		theHarmonyTypes[51].min_steps=1;
	    theHarmonyTypes[51].max_steps=theHarmonyTypes[51].num_harmony_steps;
		theHarmonyTypes[51].harmony_steps[0]=1;
		theHarmonyTypes[51].harmony_steps[1]=4;
		theHarmonyTypes[51].harmony_steps[2]=3;
		theHarmonyTypes[51].harmony_steps[3]=6;

		// (harmony_type==52)             /* I-VI */  // 
		strcpy(theHarmonyTypes[52].harmony_type_desc, "I-VI alt maj/ rel. min" );
		strcpy(theHarmonyTypes[52].harmony_degrees_desc, "I - VI" );
	    theHarmonyTypes[52].num_harmony_steps=2;  // 1-8
		theHarmonyTypes[52].min_steps=1;
	    theHarmonyTypes[52].max_steps=theHarmonyTypes[52].num_harmony_steps;
		theHarmonyTypes[52].harmony_steps[0]=1;
		theHarmonyTypes[52].harmony_steps[1]=6;
		
		// (harmony_type==53)             /* 12 bar blues variation 1*/
	    strcpy(theHarmonyTypes[53].harmony_type_desc, "12 bar blues variation 1" );
		strcpy(theHarmonyTypes[53].harmony_degrees_desc, "I - I - I - I - IV - IV - I - I - V - IV - I - V" );
	    theHarmonyTypes[53].num_harmony_steps=12;
		theHarmonyTypes[53].min_steps=1;
	    theHarmonyTypes[53].max_steps=theHarmonyTypes[53].num_harmony_steps; 
        theHarmonyTypes[53].harmony_steps[0]=1;
        theHarmonyTypes[53].harmony_steps[1]=1;
        theHarmonyTypes[53].harmony_steps[2]=1;
        theHarmonyTypes[53].harmony_steps[3]=1;
        theHarmonyTypes[53].harmony_steps[4]=4;
        theHarmonyTypes[53].harmony_steps[5]=4;
        theHarmonyTypes[53].harmony_steps[6]=1;
        theHarmonyTypes[53].harmony_steps[7]=1;
        theHarmonyTypes[53].harmony_steps[8]=5;
        theHarmonyTypes[53].harmony_steps[9]=4;
        theHarmonyTypes[53].harmony_steps[10]=1;
	    theHarmonyTypes[53].harmony_steps[11]=5;

		// (harmony_type==54)             /* 12 bar blues variation 2*/
	    strcpy(theHarmonyTypes[54].harmony_type_desc, "12 bar blues variation 2" );
		strcpy(theHarmonyTypes[54].harmony_degrees_desc, "I - I - I - I - IV - IV - I - I - IV - V - I - V" );
	    theHarmonyTypes[54].num_harmony_steps=12;
		theHarmonyTypes[54].min_steps=1;
	    theHarmonyTypes[54].max_steps=theHarmonyTypes[54].num_harmony_steps; 
        theHarmonyTypes[54].harmony_steps[0]=1;
        theHarmonyTypes[54].harmony_steps[1]=1;
        theHarmonyTypes[54].harmony_steps[2]=1;
        theHarmonyTypes[54].harmony_steps[3]=1;
        theHarmonyTypes[54].harmony_steps[4]=4;
        theHarmonyTypes[54].harmony_steps[5]=4;
        theHarmonyTypes[54].harmony_steps[6]=1;
        theHarmonyTypes[54].harmony_steps[7]=1;
        theHarmonyTypes[54].harmony_steps[8]=4;
        theHarmonyTypes[54].harmony_steps[9]=5;
        theHarmonyTypes[54].harmony_steps[10]=1;
	    theHarmonyTypes[54].harmony_steps[11]=5;

		// (harmony_type==55)             /* 12 bar blues turnaround 1*/
	    strcpy(theHarmonyTypes[55].harmony_type_desc, "12 bar blues turnaround 1" );
		strcpy(theHarmonyTypes[55].harmony_degrees_desc, "I - IV - I - I - IV - IV - I - I - V - IV - I - V" );
	    theHarmonyTypes[55].num_harmony_steps=12;
		theHarmonyTypes[55].min_steps=1;
	    theHarmonyTypes[55].max_steps=theHarmonyTypes[55].num_harmony_steps; 
        theHarmonyTypes[55].harmony_steps[0]=1;
        theHarmonyTypes[55].harmony_steps[1]=4;
        theHarmonyTypes[55].harmony_steps[2]=1;
        theHarmonyTypes[55].harmony_steps[3]=1;
        theHarmonyTypes[55].harmony_steps[4]=4;
        theHarmonyTypes[55].harmony_steps[5]=4;
        theHarmonyTypes[55].harmony_steps[6]=1;
        theHarmonyTypes[55].harmony_steps[7]=1;
        theHarmonyTypes[55].harmony_steps[8]=5;
        theHarmonyTypes[55].harmony_steps[9]=4;
        theHarmonyTypes[55].harmony_steps[10]=1;
	    theHarmonyTypes[55].harmony_steps[11]=5;

		// (harmony_type==56)             /* 8 bar blues traditional*/
	    strcpy(theHarmonyTypes[56].harmony_type_desc, "8 bar blues traditional" );
		strcpy(theHarmonyTypes[56].harmony_degrees_desc, "I - V - IV - IV - I - V - I - V" );
	    theHarmonyTypes[56].num_harmony_steps=8;
		theHarmonyTypes[56].min_steps=1;
	    theHarmonyTypes[56].max_steps=theHarmonyTypes[56].num_harmony_steps; 
        theHarmonyTypes[56].harmony_steps[0]=1;
        theHarmonyTypes[56].harmony_steps[1]=5;
        theHarmonyTypes[56].harmony_steps[2]=4;
        theHarmonyTypes[56].harmony_steps[3]=4;
        theHarmonyTypes[56].harmony_steps[4]=1;
        theHarmonyTypes[56].harmony_steps[5]=5;
        theHarmonyTypes[56].harmony_steps[6]=1;
        theHarmonyTypes[56].harmony_steps[7]=5;

		// (harmony_type==57)             /* 8 bar blues variation 1*/
	    strcpy(theHarmonyTypes[57].harmony_type_desc, "8 bar blues variation 1" );
		strcpy(theHarmonyTypes[57].harmony_degrees_desc, "I - I - I - I - IV - IV - V - I" );
	    theHarmonyTypes[57].num_harmony_steps=8;
		theHarmonyTypes[57].min_steps=1;
	    theHarmonyTypes[57].max_steps=theHarmonyTypes[57].num_harmony_steps; 
        theHarmonyTypes[57].harmony_steps[0]=1;
        theHarmonyTypes[57].harmony_steps[1]=1;
        theHarmonyTypes[57].harmony_steps[2]=1;
        theHarmonyTypes[57].harmony_steps[3]=1;
        theHarmonyTypes[57].harmony_steps[4]=4;
        theHarmonyTypes[57].harmony_steps[5]=4;
        theHarmonyTypes[57].harmony_steps[6]=5;
        theHarmonyTypes[57].harmony_steps[7]=1;

		// (harmony_type==58)             /* 8 bar blues variation 2*/
	    strcpy(theHarmonyTypes[58].harmony_type_desc, "8 bar blues variation 2" );
		strcpy(theHarmonyTypes[58].harmony_degrees_desc, "I - I - I - I - IV - IV - V - V" );
	    theHarmonyTypes[58].num_harmony_steps=8;
		theHarmonyTypes[58].min_steps=1;
	    theHarmonyTypes[58].max_steps=theHarmonyTypes[58].num_harmony_steps; 
        theHarmonyTypes[58].harmony_steps[0]=1;
        theHarmonyTypes[58].harmony_steps[1]=1;
        theHarmonyTypes[58].harmony_steps[2]=1;
        theHarmonyTypes[58].harmony_steps[3]=1;
        theHarmonyTypes[58].harmony_steps[4]=4;
        theHarmonyTypes[58].harmony_steps[5]=4;
        theHarmonyTypes[58].harmony_steps[6]=5;
        theHarmonyTypes[58].harmony_steps[7]=5;

		// (harmony_type==59)             /* ii-V-I */
	    strcpy(theHarmonyTypes[59].harmony_type_desc, "II - V - I cadential" );
		strcpy(theHarmonyTypes[59].harmony_degrees_desc, "II - V - I" );
	    theHarmonyTypes[59].num_harmony_steps=3;
		theHarmonyTypes[59].min_steps=1;
	    theHarmonyTypes[59].max_steps=theHarmonyTypes[59].num_harmony_steps; 
        theHarmonyTypes[59].harmony_steps[0]=2;
        theHarmonyTypes[59].harmony_steps[1]=5;
        theHarmonyTypes[59].harmony_steps[2]=1;

		//********* Cycle Progressions of 7 steps where each step is a fixed degree and 7 steps returns to the starting point going around the circle n-cycle times

		// (harmony_type==60)             /* 5ths cycle progression */
	    strcpy(theHarmonyTypes[60].harmony_type_desc, "5ths cycle 1-loop" );
		strcpy(theHarmonyTypes[60].harmony_degrees_desc, "I - V - II - VI - III - VII - IV" );
	    theHarmonyTypes[60].num_harmony_steps=7;
		theHarmonyTypes[60].min_steps=1;
	    theHarmonyTypes[60].max_steps=theHarmonyTypes[60].num_harmony_steps; 
        theHarmonyTypes[60].harmony_steps[0]=1;
        theHarmonyTypes[60].harmony_steps[1]=5;
        theHarmonyTypes[60].harmony_steps[2]=2;
		theHarmonyTypes[60].harmony_steps[3]=6;
        theHarmonyTypes[60].harmony_steps[4]=3;
        theHarmonyTypes[60].harmony_steps[5]=7;
		theHarmonyTypes[60].harmony_steps[6]=4;

		// (harmony_type==61)             /* 2nds cycle progression */
	    strcpy(theHarmonyTypes[61].harmony_type_desc, "2nds cycle 2-loop" );
		strcpy(theHarmonyTypes[61].harmony_degrees_desc, "I - II - III - IV - V - VI - VII" );
	    theHarmonyTypes[61].num_harmony_steps=7;
		theHarmonyTypes[61].min_steps=1;
	    theHarmonyTypes[61].max_steps=theHarmonyTypes[61].num_harmony_steps; 
        theHarmonyTypes[61].harmony_steps[0]=1;
        theHarmonyTypes[61].harmony_steps[1]=2;
        theHarmonyTypes[61].harmony_steps[2]=3;
		theHarmonyTypes[61].harmony_steps[3]=4;
        theHarmonyTypes[61].harmony_steps[4]=5;
        theHarmonyTypes[61].harmony_steps[5]=6;
		theHarmonyTypes[61].harmony_steps[6]=7;

		// (harmony_type==62)             /* 6ths cycle progression */
	    strcpy(theHarmonyTypes[62].harmony_type_desc, "6ths cycle 3-loop" );
		strcpy(theHarmonyTypes[62].harmony_degrees_desc, "I - VI - IV - II - VII - V - III" );
	    theHarmonyTypes[62].num_harmony_steps=7;
		theHarmonyTypes[62].min_steps=1;
	    theHarmonyTypes[62].max_steps=theHarmonyTypes[62].num_harmony_steps; 
        theHarmonyTypes[62].harmony_steps[0]=1;
        theHarmonyTypes[62].harmony_steps[1]=6;
        theHarmonyTypes[62].harmony_steps[2]=4;
		theHarmonyTypes[62].harmony_steps[3]=2;
        theHarmonyTypes[62].harmony_steps[4]=7;
        theHarmonyTypes[62].harmony_steps[5]=5;
		theHarmonyTypes[62].harmony_steps[6]=3;
       
        // (harmony_type==63)             /* 3rds cycle progression */
	    strcpy(theHarmonyTypes[63].harmony_type_desc, "3rds cycle 4-loop" );
		strcpy(theHarmonyTypes[63].harmony_degrees_desc, "I - III- V - VII - II - IV - VI" );
	    theHarmonyTypes[63].num_harmony_steps=7;
		theHarmonyTypes[63].min_steps=1;
	    theHarmonyTypes[63].max_steps=theHarmonyTypes[63].num_harmony_steps; 
        theHarmonyTypes[63].harmony_steps[0]=1;
        theHarmonyTypes[63].harmony_steps[1]=3;
        theHarmonyTypes[63].harmony_steps[2]=5;
		theHarmonyTypes[63].harmony_steps[3]=7;
        theHarmonyTypes[63].harmony_steps[4]=2;
        theHarmonyTypes[63].harmony_steps[5]=4;
		theHarmonyTypes[63].harmony_steps[6]=6;

		// (harmony_type==64)             /* 7ths cycle progression */
	    strcpy(theHarmonyTypes[64].harmony_type_desc, "7ths cycle 5-loop" );
		strcpy(theHarmonyTypes[64].harmony_degrees_desc, "I - VII - VI - V - IV - III - II" );
	    theHarmonyTypes[64].num_harmony_steps=7;
		theHarmonyTypes[64].min_steps=1;
	    theHarmonyTypes[64].max_steps=theHarmonyTypes[64].num_harmony_steps; 
        theHarmonyTypes[64].harmony_steps[0]=1;
        theHarmonyTypes[64].harmony_steps[1]=7;
        theHarmonyTypes[64].harmony_steps[2]=6;
		theHarmonyTypes[64].harmony_steps[3]=5;
        theHarmonyTypes[64].harmony_steps[4]=4;
        theHarmonyTypes[64].harmony_steps[5]=3;
		theHarmonyTypes[64].harmony_steps[6]=2;

		// (harmony_type==65)             /* 4ths cycle progression */
	    strcpy(theHarmonyTypes[65].harmony_type_desc, "4ths cycle 6-loop" );
		strcpy(theHarmonyTypes[65].harmony_degrees_desc, "I - IV - VI - III - VI - II - V" );
	    theHarmonyTypes[65].num_harmony_steps=7;
		theHarmonyTypes[65].min_steps=1;
	    theHarmonyTypes[65].max_steps=theHarmonyTypes[65].num_harmony_steps; 
        theHarmonyTypes[65].harmony_steps[0]=1;
        theHarmonyTypes[65].harmony_steps[1]=4;
        theHarmonyTypes[65].harmony_steps[2]=7;
		theHarmonyTypes[65].harmony_steps[3]=3;
        theHarmonyTypes[65].harmony_steps[4]=6;
        theHarmonyTypes[65].harmony_steps[5]=2;
		theHarmonyTypes[65].harmony_steps[6]=5;

		//***************

		// (harmony_type==66)             /* 12 bar ratchet progression*/
	    strcpy(theHarmonyTypes[66].harmony_type_desc, "12 bar ratchet 1" );
		strcpy(theHarmonyTypes[66].harmony_degrees_desc, "I-V-I-II-I-VI-I-III-I-VII-I-IV" );
	    theHarmonyTypes[66].num_harmony_steps=12;
		theHarmonyTypes[66].min_steps=1;
	    theHarmonyTypes[66].max_steps=theHarmonyTypes[66].num_harmony_steps; 
        theHarmonyTypes[66].harmony_steps[0]=1;
        theHarmonyTypes[66].harmony_steps[1]=5;
        theHarmonyTypes[66].harmony_steps[2]=1;
        theHarmonyTypes[66].harmony_steps[3]=2;
        theHarmonyTypes[66].harmony_steps[4]=1;
        theHarmonyTypes[66].harmony_steps[5]=6;
        theHarmonyTypes[66].harmony_steps[6]=1;
        theHarmonyTypes[66].harmony_steps[7]=3;
        theHarmonyTypes[66].harmony_steps[8]=1;
        theHarmonyTypes[66].harmony_steps[9]=7;
        theHarmonyTypes[66].harmony_steps[10]=1;
	    theHarmonyTypes[66].harmony_steps[11]=4;

		// (harmony_type==67)             /* 12 bar ratchet progression*/
	    strcpy(theHarmonyTypes[67].harmony_type_desc, "12 bar ratchet 2" );
		strcpy(theHarmonyTypes[67].harmony_degrees_desc, "I-IV-I-VII-I-III-I-VI-I-II-I-V" );
	    theHarmonyTypes[67].num_harmony_steps=12;
		theHarmonyTypes[67].min_steps=1;
	    theHarmonyTypes[67].max_steps=theHarmonyTypes[67].num_harmony_steps; 
        theHarmonyTypes[67].harmony_steps[0]=1;
        theHarmonyTypes[67].harmony_steps[1]=4;
        theHarmonyTypes[67].harmony_steps[2]=1;
        theHarmonyTypes[67].harmony_steps[3]=7;
        theHarmonyTypes[67].harmony_steps[4]=1;
        theHarmonyTypes[67].harmony_steps[5]=3;
        theHarmonyTypes[67].harmony_steps[6]=1;
        theHarmonyTypes[67].harmony_steps[7]=6;
        theHarmonyTypes[67].harmony_steps[8]=1;
        theHarmonyTypes[67].harmony_steps[9]=2;
        theHarmonyTypes[67].harmony_steps[10]=1;
	    theHarmonyTypes[67].harmony_steps[11]=5;

		// (harmony_type==68)             /* stay on I*/
		strcpy(theHarmonyTypes[68].harmony_type_desc, "stay on I" );
		strcpy(theHarmonyTypes[68].harmony_degrees_desc, "I" );
	    theHarmonyTypes[68].num_harmony_steps=1; // steps fixed at 1
		theHarmonyTypes[68].min_steps=1;
	    theHarmonyTypes[68].max_steps=theHarmonyTypes[68].num_harmony_steps;
        theHarmonyTypes[68].harmony_steps[0]=1;

		// (harmony_type==69)             /* stay on II*/
		strcpy(theHarmonyTypes[69].harmony_type_desc, "stay on II" );
		strcpy(theHarmonyTypes[69].harmony_degrees_desc, "II" );
	    theHarmonyTypes[69].num_harmony_steps=1; // steps fixed at 1
		theHarmonyTypes[69].min_steps=1;
	    theHarmonyTypes[69].max_steps=theHarmonyTypes[69].num_harmony_steps;
        theHarmonyTypes[69].harmony_steps[0]=2;

		// (harmony_type==70)             /* stay on III*/
		strcpy(theHarmonyTypes[70].harmony_type_desc, "stay on III" );
		strcpy(theHarmonyTypes[70].harmony_degrees_desc, "III" );
	    theHarmonyTypes[70].num_harmony_steps=1; // steps fixed at 1
		theHarmonyTypes[70].min_steps=1;
	    theHarmonyTypes[70].max_steps=theHarmonyTypes[70].num_harmony_steps;
        theHarmonyTypes[70].harmony_steps[0]=3;

		// (harmony_type==71)             /* stay on IV*/
		strcpy(theHarmonyTypes[71].harmony_type_desc, "stay on IV" );
		strcpy(theHarmonyTypes[71].harmony_degrees_desc, "IV" );
	    theHarmonyTypes[71].num_harmony_steps=1; // steps fixed at 1
		theHarmonyTypes[71].min_steps=1;
	    theHarmonyTypes[71].max_steps=theHarmonyTypes[71].num_harmony_steps;
        theHarmonyTypes[71].harmony_steps[0]=4;

		// (harmony_type==72)             /* stay on V*/
		strcpy(theHarmonyTypes[72].harmony_type_desc, "stay on V" );
		strcpy(theHarmonyTypes[72].harmony_degrees_desc, "V" );
	    theHarmonyTypes[72].num_harmony_steps=1; // steps fixed at 1
		theHarmonyTypes[72].min_steps=1;
	    theHarmonyTypes[72].max_steps=theHarmonyTypes[72].num_harmony_steps;
        theHarmonyTypes[72].harmony_steps[0]=5;

		// (harmony_type==73)             /* stay on VI*/
		strcpy(theHarmonyTypes[73].harmony_type_desc, "stay on VI" );
		strcpy(theHarmonyTypes[73].harmony_degrees_desc, "VI" );
	    theHarmonyTypes[73].num_harmony_steps=1; // steps fixed at 1
		theHarmonyTypes[73].min_steps=1;
	    theHarmonyTypes[73].max_steps=theHarmonyTypes[73].num_harmony_steps;
        theHarmonyTypes[73].harmony_steps[0]=6;

		// (harmony_type==74)             /* stay on VII*/
		strcpy(theHarmonyTypes[74].harmony_type_desc, "stay on VII" );
		strcpy(theHarmonyTypes[74].harmony_degrees_desc, "VII" );
	    theHarmonyTypes[74].num_harmony_steps=1; // steps fixed at 1
		theHarmonyTypes[74].min_steps=1;
	    theHarmonyTypes[74].max_steps=theHarmonyTypes[74].num_harmony_steps;
        theHarmonyTypes[74].harmony_steps[0]=7;

		// Avoids

		// (harmony_type==75)             /* by 4ths avoid V */
	    strcpy(theHarmonyTypes[75].harmony_type_desc, "by 4ths avoid V" );
		strcpy(theHarmonyTypes[75].harmony_degrees_desc, "I - IV - VII - III - VI - II" );
	    theHarmonyTypes[75].num_harmony_steps=6;
		theHarmonyTypes[75].min_steps=1;
	    theHarmonyTypes[75].max_steps=theHarmonyTypes[75].num_harmony_steps; 
        theHarmonyTypes[75].harmony_steps[0]=1;
        theHarmonyTypes[75].harmony_steps[1]=4;
        theHarmonyTypes[75].harmony_steps[2]=7;
		theHarmonyTypes[75].harmony_steps[3]=3;
        theHarmonyTypes[75].harmony_steps[4]=6;
        theHarmonyTypes[75].harmony_steps[5]=2;

		// (harmony_type==76)             /* by 5ths avoid IV */
	    strcpy(theHarmonyTypes[76].harmony_type_desc, "by 5ths avoid IV" );
		strcpy(theHarmonyTypes[76].harmony_degrees_desc, "I - V - II - VI - III - VII" );
	    theHarmonyTypes[76].num_harmony_steps=6;
		theHarmonyTypes[76].min_steps=1;
	    theHarmonyTypes[76].max_steps=theHarmonyTypes[76].num_harmony_steps; 
        theHarmonyTypes[76].harmony_steps[0]=1;
        theHarmonyTypes[76].harmony_steps[1]=5;
        theHarmonyTypes[76].harmony_steps[2]=2;
		theHarmonyTypes[76].harmony_steps[3]=6;
        theHarmonyTypes[76].harmony_steps[4]=3;
        theHarmonyTypes[76].harmony_steps[5]=7;

		// (harmony_type==77)             /* by 4ths avoid I */
	    strcpy(theHarmonyTypes[77].harmony_type_desc, "by 4ths avoid I" );
		strcpy(theHarmonyTypes[77].harmony_degrees_desc, "IV - VII - III - VI - II - V" );
	    theHarmonyTypes[77].num_harmony_steps=6;
		theHarmonyTypes[77].min_steps=1;
	    theHarmonyTypes[77].max_steps=theHarmonyTypes[77].num_harmony_steps; 
        theHarmonyTypes[77].harmony_steps[0]=4;
        theHarmonyTypes[77].harmony_steps[1]=7;
        theHarmonyTypes[77].harmony_steps[2]=3;
		theHarmonyTypes[77].harmony_steps[3]=6;
        theHarmonyTypes[77].harmony_steps[4]=2;
        theHarmonyTypes[77].harmony_steps[5]=5;
	
		// (harmony_type==78)             /* by 5ths avoid I */  
	    strcpy(theHarmonyTypes[78].harmony_type_desc, "by 5ths avoid I" );
		strcpy(theHarmonyTypes[78].harmony_degrees_desc, "V - II - VI - III - VII - IV" );
	    theHarmonyTypes[78].num_harmony_steps=6;
		theHarmonyTypes[78].min_steps=1;
	    theHarmonyTypes[78].max_steps=theHarmonyTypes[78].num_harmony_steps; 
        theHarmonyTypes[78].harmony_steps[0]=5;
        theHarmonyTypes[78].harmony_steps[1]=2;
        theHarmonyTypes[78].harmony_steps[2]=6;
		theHarmonyTypes[78].harmony_steps[3]=3;
        theHarmonyTypes[78].harmony_steps[4]=7;
        theHarmonyTypes[78].harmony_steps[5]=4;

		// (harmony_type==79)             /* 14 bar I-VI by 4ths*/
	    strcpy(theHarmonyTypes[79].harmony_type_desc, "14 bar I-VI by 4ths" );
		strcpy(theHarmonyTypes[79].harmony_degrees_desc, "I-VI-IV-II-VII-V-III-I-VI-IV-II-VII-V-III" );
	    theHarmonyTypes[79].num_harmony_steps=14;
		theHarmonyTypes[79].min_steps=1;
	    theHarmonyTypes[79].max_steps=theHarmonyTypes[79].num_harmony_steps; 
        theHarmonyTypes[79].harmony_steps[0]=1;
        theHarmonyTypes[79].harmony_steps[1]=6;
        theHarmonyTypes[79].harmony_steps[2]=4;
        theHarmonyTypes[79].harmony_steps[3]=2;
        theHarmonyTypes[79].harmony_steps[4]=7;
        theHarmonyTypes[79].harmony_steps[5]=5;
        theHarmonyTypes[79].harmony_steps[6]=3;
        theHarmonyTypes[79].harmony_steps[7]=1;
        theHarmonyTypes[79].harmony_steps[8]=6;
        theHarmonyTypes[79].harmony_steps[9]=4;
        theHarmonyTypes[79].harmony_steps[10]=2;
	    theHarmonyTypes[79].harmony_steps[11]=7;
		theHarmonyTypes[79].harmony_steps[12]=5;
	    theHarmonyTypes[79].harmony_steps[13]=3;

		// (harmony_type==80)             /* 14 bar I-VI by 5ths*/
	    strcpy(theHarmonyTypes[80].harmony_type_desc, "14 bar I-VI by 5ths" );
		strcpy(theHarmonyTypes[80].harmony_degrees_desc, "I-VI-V-III-II-VII-VI-IV-III-I-VII-V-IV-II" );
	    theHarmonyTypes[80].num_harmony_steps=14;
		theHarmonyTypes[80].min_steps=1;
	    theHarmonyTypes[80].max_steps=theHarmonyTypes[80].num_harmony_steps; 
        theHarmonyTypes[80].harmony_steps[0]=1;
        theHarmonyTypes[80].harmony_steps[1]=6;
        theHarmonyTypes[80].harmony_steps[2]=5;
        theHarmonyTypes[80].harmony_steps[3]=3;
        theHarmonyTypes[80].harmony_steps[4]=2;
        theHarmonyTypes[80].harmony_steps[5]=7;
        theHarmonyTypes[80].harmony_steps[6]=6;
        theHarmonyTypes[80].harmony_steps[7]=4;
        theHarmonyTypes[80].harmony_steps[8]=3;
        theHarmonyTypes[80].harmony_steps[9]=1;
        theHarmonyTypes[80].harmony_steps[10]=7;
	    theHarmonyTypes[80].harmony_steps[11]=5;
		theHarmonyTypes[80].harmony_steps[12]=4;
	    theHarmonyTypes[80].harmony_steps[13]=2;
		

		// End of preset harmony types 
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

void setup_harmony()
{
	int i,j,k;
    int circle_position=0;
	int circleDegree=0;
		
    for(i=0;i<theActiveHarmonyType.num_harmony_steps;++i)              /* for each of the harmony steps         */
     {           
	                              /* build proper chord notes              */
	   num_step_chord_notes[i]=0;
	   //find semicircle degree that matches step degree
	   for (int j=0; j<7; ++j)
	   {
		   if (theCircleOf5ths.theDegreeSemiCircle.degreeElements[j].Degree==theActiveHarmonyType.harmony_steps[i])
		   {
			   circleDegree=theCircleOf5ths.theDegreeSemiCircle.degreeElements[j].Degree;
			   circle_position=theCircleOf5ths.theDegreeSemiCircle.degreeElements[j].CircleIndex;
			   break;
		   }
		}
	 
	   int thisStepChordType=theCircleOf5ths.Circle5ths[circle_position].chordType;

		if (true)  // attempting to handle 7ths
		{
			if  ((theMeanderState.theHarmonyParms.enable_all_7ths)|| (theMeanderState.theHarmonyParms.enable_V_7ths))  // override V chord to 7th
			//	 ((theMeanderState.theHarmonyParms.enable_V_7ths)&&(circleDegree==5)))  // override V chord to 7th
			{	
				if ((theMeanderState.theHarmonyParms.enable_V_7ths)&&(circleDegree==5))
				{
					if (thisStepChordType==0)  // maj
						thisStepChordType=2; // 7dom  .  A dom7 sounds better than a maj7
					else
					if (thisStepChordType==1)  // min
						thisStepChordType=4; // 7min
					else
					if (thisStepChordType==6)  // dim
						thisStepChordType=5; // dim7
					theCircleOf5ths.Circle5ths[circle_position].chordType=thisStepChordType;
				}
				else
				if (theMeanderState.theHarmonyParms.enable_all_7ths)  // actually only use most popular 7ths
				{ 
					if (circleDegree==2)  // II
					{
						if (thisStepChordType==1)  // min
							thisStepChordType=4;   // 7thmin  
					}
					else
					if (circleDegree==4)  // IV
					{
						if (thisStepChordType==0)  // maj
						//	thisStepChordType=2;   // 7thdom  
							thisStepChordType=3;   // 7thmaj  
					}
					else
					if (circleDegree==5)  // V
					{
						if (thisStepChordType==0)  // maj
							thisStepChordType=2;   // 7thdom  
					}
					else
					if (circleDegree==7)  // VII
					{
						if (thisStepChordType==6)  // dim
							thisStepChordType=5;   // 7thdim  
					}
					theCircleOf5ths.Circle5ths[circle_position].chordType=thisStepChordType;
				}
				
			}
		}

       for(j=0;j<num_root_key_notes[circle_position];++j)
        {
			int root_key_note=root_key_notes[circle_of_fifths[circle_position]][j];
			int thisStepChordType=theCircleOf5ths.Circle5ths[circle_position].chordType;
			
          	if ((root_key_note%MAX_NOTES)==circle_of_fifths[circle_position])
		    {
				for (k=0;k<chord_type_num_notes[thisStepChordType];++k)
				{  
					step_chord_notes[i][num_step_chord_notes[i]]=(int)((int)root_key_note+(int)chord_type_intervals[thisStepChordType][k]);
					++num_step_chord_notes[i];
				}
			}   
       }
		
	   if (true)  // if this is not done, step_chord_notes[0] begins with root note.   If done, chord spread is limited but smoother wandering through innversions
	   {
		    for (j=0;j<num_step_chord_notes[i];++j)
			{
				step_chord_notes[i][j]=step_chord_notes[i][j+((11-circle_of_fifths[circle_position])/3)];
			}
			num_step_chord_notes[i]-=((11-circle_of_fifths[circle_position])/3);
	   }
     }
}


void MeanderMusicStructuresInitialize()
{
	copyHarmonyTypeToActiveHarmonyType(harmony_type);
	setup_harmony();
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
   init_harmony();  // sets up original progressions
   setup_harmony();  // calculate harmony notes
    
}

char MeanderScaleText[128];



