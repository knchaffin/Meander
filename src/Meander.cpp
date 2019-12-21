#include <rack.hpp>   
   

#include "plugin.hpp"  
#include <sstream>
#include <iomanip>
#include <time.h>

#include <iostream>
#include <fstream>
#include <string>
#include <mutex>

#include "Common-Noise.hpp"



// the following will "comment out" all DEBUG() calls for speed.  Comment out the next line and DEBUG is compiled
#define DEBUG(format, ...) // DEBUG(format, ...)

static bool owned = false;


bool initialized=false;

bool Audit_enable=false;  

using namespace rack;

#define CV_MAX10 (10.0f)
#define CV_MAXn5 (5.0f)
#define AUDIO_MAX (6.0f)
#define VOCT_MAX (8.0f)
#define AMP_MAX (2.0f)       
 
#define MAX_NOTES 12
#define MAX_STEPS 16
#define MAX_CIRCLE_STATIONS 12
#define MAX_HARMONIC_DEGREES 7
#define MAX_AVAILABLE_HARMONY_PRESETS 51  // change this as new harmony presets are created

ParamWidget* CircleOf5thsOuterButton[MAX_CIRCLE_STATIONS];  
LightWidget* CircleOf5thsOuterButtonLight[MAX_CIRCLE_STATIONS]; 
LightWidget* CircleOf5thsInnerLight[MAX_CIRCLE_STATIONS];  // root_key lights
ParamWidget* CircleOf5thsSelectStepButton[MAX_STEPS];        
LightWidget* CircleOf5thsSelectStepButtonLight[MAX_STEPS];   

int time_sig_top, time_sig_bottom = 4;


// make it a power of 8
#define MAXSHORTSTRLEN 16

struct CircleElement
{
	// this data is mostly static, once created
	int chordType=0;  // may be overridden by degree semicircle
	float startDegree; // of annular segment
	float endDegree;   // of annular segment
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
	float startDegree;  // of pie slice
	float endDegree;
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


bool circleChanged=true;
int harmonyStepsChanged=0; 

int semiCircleDegrees[]={1, 5, 2, 6, 3, 7, 4};  // default order if starting at C
int circleDegreeLookup[]= {0, 0, 2, 4, 6, 1, 3, 5};  // to convert from arabic roman equivalents to circle degrees
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

int mode_step_intervals[7][13]=
{  // num mode scale notes, semitones to next note  7 modes
	{ 7, 2,2,2,1,2,2,1,0,0,0,0,0},                // Lydian  	        
	{ 7, 2,2,1,2,2,2,1,0,0,0,0,0},                // Major/Ionian     
	{ 7, 2,2,1,2,2,1,2,0,0,0,0,0},                // Mixolydian	   
	{ 7, 2,1,2,2,2,1,2,0,0,0,0,0},                // Dorian           
	{ 7, 2,1,2,2,1,2,2,0,0,0,0,0},                // NMinor/Aeolian   
	{ 7, 1,2,2,2,1,2,2,0,0,0,0,0},                // Phrygian         
	{ 7, 1,2,2,1,2,2,2,0,0,0,0,0}                 // Locrian            
}; 



int root_key_signatures_chromaticForder[12][7]=  // chromatic order 0=natural, 1=sharp, -1=flat
{  // F  C  G  D  A  E  B  in order of root_key signature sharps and flats starting with F
  	{ 0, 0, 0, 0, 0, 0, 0},  // C  - 0
	{ 0, 0,-1,-1,-1,-1,-1},  // Db - 1
	{ 1, 1, 0, 0, 0, 0, 0},  // D  - 2
	{ 0, 0, 0, 0,-1,-1,-1},  // Eb - 3
	{ 1, 1, 1, 1, 0, 0, 0},  // E  - 4
	{ 0, 0, 0, 0, 0, 0,-1},  // F  - 5
	{ 1, 1, 1, 1, 1, 1, 0},  // F# - 6
	{ 1, 0, 0, 0, 0, 0, 0},  // G  - 7
	{ 0, 0, 0,-1,-1,-1,-1},  // Ab - 8
	{ 1, 1, 1, 0, 0, 0, 0},  // A  - 9
	{ 0, 0, 0, 0, 0,-1,-1},  // Bb - 10
	{ 1, 1, 1, 1, 1, 0, 0},  // B  - 11
	
};

int root_key_sharps_vertical_display_offset[]={1, 4, 0, 3, 6, 2};  // left to right
int root_key_flats_vertical_display_offset[]={5, 2, 6, 3, 7, 4};   // right to left


#define MAX_MODES 7
int num_modes=MAX_MODES;
char mode_names[MAX_MODES][16];

int  mode=1;  // Ionian/Major

enum noteTypes
{
	NOTE_TYPE_CHORD,
	NOTE_TYPE_MELODY,
	NOTE_TYPE_ARP,
	NOTE_TYPE_BASS,
	NOTE_TYPE_EXTERNAL
};


struct note
{
	int note;
	int noteType; // NOTE_TYPE_CHORD etc.
	int time32s;
	int length;  // 1/1,2,4,8
	int countInBar;
};

int bar_note_count=0;  // how many notes have been played in bar.  Use it as index into  played_notes_circular_buffer[]

struct note played_notes_circular_buffer[256];  // worst case maximum of 256 harmony, melody and bass notes can be played per bar.  


const char* noteNames[MAX_NOTES] = {"C","C#/Db","D","D#/Eb","E","F","F#/Gb","G","G#/Ab","A","A#/Bb","B"};
const char* CircleNoteNames[MAX_NOTES] = {"C","G","D","A","E","B","F#","Db","Ab","Eb","Bb","F"};

#define MAX_ROOT_KEYS 12
int circle_root_key=0; // root_key position on the circle 0,1,2... CW
int root_key=0;  // 0 initially

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

int mode_root_key_signature_offset[]={3,0,4,1,5,2,6};  // index into mode_natural_roots[] using the IDPLyMALo = 1,2,3,4,5,6,7 rule for Meander mode ordering

char root_key_name[MAXSHORTSTRLEN];
char root_key_names[MAX_ROOT_KEYS][MAXSHORTSTRLEN];

#define MAX_NOTES_CANDIDATES 130
int  notes[MAX_NOTES_CANDIDATES];

int  num_notes=0;
int  root_key_notes[MAX_ROOT_KEYS][MAX_NOTES_CANDIDATES];

int  num_root_key_notes[MAX_ROOT_KEYS];

int meter_numerator=4;  // need to unify with sig_top...
int meter_denominator=4;

char   note_desig[MAX_NOTES][MAXSHORTSTRLEN];
char   note_desig_sharps[MAX_NOTES][MAXSHORTSTRLEN];
char   note_desig_flats[MAX_NOTES][MAXSHORTSTRLEN];

struct HarmonyParms
{
	bool enabled=true;
	float volume=10.0f;  // 0-10 V
	int chord_on_note_divisor=1;  // 1, 2, 4, 8   doHarmony() on these boundaries
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
	int bar_harmony_chords_counted_note=0;
	bool enable_all_7ths=false;
	bool enable_V_7ths=false;
	struct note last[4];
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
	bool destutter=true;
	bool stutter_detected=false;
	int last_stutter_step=0;
	int last_chord_note_index=0;
	int last_step=1; 
	int bar_melody_counted_note=0;
	struct note last[1];
}; 

struct ArpParms
{
	bool enabled=false;
	bool chordal=true;
	bool scaler=false;
	int count=3;
	int increment=16;  // 8, 16, 32
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
	int bass_on_note_divisor=1;  // 1, 2, 4, 8   doHarmony() on these boundaries
	bool octave_enabled=true;  // play bass as 2 notes an octave apart
	float volume=10.0f;  // 0-10 V
	int bar_bass_counted_note=0;
	bool syncopate=false;
	bool accent=false;
	struct note last[4];
}; 


struct MeanderState
{
	HarmonyParms  theHarmonyParms;
	MelodyParms theMelodyParms;
	BassParms theBassParms;
	ArpParms theArpParms;
	bool userControllingHarmonyFromCircle=false;
	int last_harmony_chord_root_note=0;
	int last_harmony_step=0;
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
	int    harmony_type;  // used by theActiveHarmonyType
	char   harmony_type_desc[64]; 
	char   harmony_degrees_desc[64]; 
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
unsigned char circle_of_fifths_degrees[][MAXSHORTSTRLEN]= {
	"I", "V", "II", "vi", "iii", "vii", "IV"
};

unsigned char circle_of_fifths_arabic_degrees[][MAXSHORTSTRLEN]= {
	"", "I", "II", "III", "IV", "V", "IV", "VII"
};

unsigned char circle_of_fifths_degrees_UC[][MAXSHORTSTRLEN]= {
	"I", "V", "II", "VI", "III", "VII", "IV"
};

unsigned char circle_of_fifths_degrees_LC[][MAXSHORTSTRLEN]= {
	"i", "v", "ii", "vi", "iii", "vii", "iv"
};

int  step_chord_notes[MAX_STEPS][MAX_NOTES_CANDIDATES];
int  num_step_chord_notes[MAX_STEPS]={};

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


void init_vars()
{
	DEBUG("init_vars()");

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
	
	strcpy(note_desig[0],"C");
	strcpy(note_desig[1],"Db");
	strcpy(note_desig[2],"D");
	strcpy(note_desig[3],"Eb");
	strcpy(note_desig[4],"E");
	strcpy(note_desig[5],"F");
	strcpy(note_desig[6],"F#");
	strcpy(note_desig[7],"G");
	strcpy(note_desig[8],"Ab");
	strcpy(note_desig[9],"A");
	strcpy(note_desig[10],"Bb");
	strcpy(note_desig[11],"B");

	strcpy(note_desig_sharps[0],"C");
	strcpy(note_desig_sharps[1],"C#");
	strcpy(note_desig_sharps[2],"D");
	strcpy(note_desig_sharps[3],"D#");
	strcpy(note_desig_sharps[4],"E");
	strcpy(note_desig_sharps[5],"F");
	strcpy(note_desig_sharps[6],"F#");
	strcpy(note_desig_sharps[7],"G");
	strcpy(note_desig_sharps[8],"G#");
	strcpy(note_desig_sharps[9],"A");
	strcpy(note_desig_sharps[10],"A#");
	strcpy(note_desig_sharps[11],"B");

	strcpy(note_desig_flats[0],"C");
	strcpy(note_desig_flats[1],"Db");
	strcpy(note_desig_flats[2],"D");
	strcpy(note_desig_flats[3],"Eb");
	strcpy(note_desig_flats[4],"E");
	strcpy(note_desig_flats[5],"F");
	strcpy(note_desig_flats[6],"Gb");
	strcpy(note_desig_flats[7],"G");
	strcpy(note_desig_flats[8],"Ab");
	strcpy(note_desig_flats[9],"A");
	strcpy(note_desig_flats[10],"Bb");
	strcpy(note_desig_flats[11],"B");


	strcpy(root_key_names[0],"C");
	strcpy(root_key_names[1],"Db");
	strcpy(root_key_names[2],"D");
	strcpy(root_key_names[3],"Eb");
	strcpy(root_key_names[4],"E");
	strcpy(root_key_names[5],"F");
	strcpy(root_key_names[6],"F#");
	strcpy(root_key_names[7],"G");
	strcpy(root_key_names[8],"Ab");
	strcpy(root_key_names[9],"A");
	strcpy(root_key_names[10],"Bb");
	strcpy(root_key_names[11],"B");
				
			
	strcpy(mode_names[0],"Lydian");
	strcpy(mode_names[1],"Ionian/Major");
	strcpy(mode_names[2],"Mixolydian");
	strcpy(mode_names[3],"Dorian");
	strcpy(mode_names[4],"Aeolian/NMinor");
	strcpy(mode_names[5],"Phrygian");
	strcpy(mode_names[6],"Locrian");

		
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
	strcpy(chord_type_name[2],"7th");
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
	DEBUG("init_notes()");
	notes[0]=root_key;  
	int nmn=mode_step_intervals[mode][0];  // number of mode notes
	DEBUG("notes[%d]=%d %s", 0, notes[0], note_desig[notes[0]%MAX_NOTES]);  
	num_notes=0;                                                                
	for (int i=1;i<127;++i)                                                         
	{     
		notes[i]=notes[i-1]+                                                    
			mode_step_intervals[mode][((i-1)%nmn)+1];  
		
		DEBUG("notes[%d]=%d %s", i, notes[i], note_desig[notes[i]%MAX_NOTES]);      
		++num_notes;                                                            
		if (notes[i]>=127) break;                                               
	}     
	DEBUG("num_notes=%d", num_notes);
															

	for (int j=0;j<12;++j)
	{
		DEBUG("root_key=%s", root_key_names[j]);
	
		root_key_notes[j][0]=j;
		num_root_key_notes[j]=1;
		

		int num_mode_notes=10*mode_step_intervals[mode][0]; // the [0] entry is the notes per scale value, times 10 ocatves of midi

	
		if (true)
		{
			DEBUG("  num_mode_notes=%d", num_mode_notes);
			DEBUG("root_key_notes[%d][0]=%d %s", j, root_key_notes[j][0], note_desig[root_key_notes[j][0]]);  
		}

		int nmn=mode_step_intervals[mode][0];  // number of mode notes
		for (int i=1;i<num_mode_notes ;++i)
		{
			root_key_notes[j][i]=root_key_notes[j][i-1]+
		   		mode_step_intervals[mode][((i-1)%nmn)+1];  
					
			DEBUG("root_key_notes[%d][%d]=%d %s", j, i, root_key_notes[j][i], note_desig[root_key_notes[j][i]%MAX_NOTES]);  
			
			++num_root_key_notes[j];
		}
		DEBUG("    num_root_key_notes[%d]=%d", j, num_root_key_notes[j]);
	
	}

	char  strng[128];
	strcpy(strng,"");
	for (int i=0;i<mode_step_intervals[mode][0];++i)
	{
		strcat(strng,note_desig[notes[i]%MAX_NOTES]);
	}
	DEBUG("mode=%d root_key=%d root_key_notes[%d]=%s", mode, root_key, root_key, strng);
}

void AuditHarmonyData(int source)
{
	 if (!Audit_enable)
	   return;
	 DEBUG("AuditHarmonyData()-begin-source=%d", source);
	 for (int j=1;j<MAX_AVAILABLE_HARMONY_PRESETS;++j)
      {
		if ((theHarmonyTypes[j].num_harmony_steps<1)||(theHarmonyTypes[j].num_harmony_steps>MAX_STEPS))
		{
			DEBUG("  warning-theHarmonyTypes[%d].num_harmony_steps=%d", j, theHarmonyTypes[j].num_harmony_steps);
		}
		for (int i=0;i<MAX_STEPS;++i)
          {
         	if ((theHarmonyTypes[j].harmony_steps[i]<1)||(theHarmonyTypes[j].harmony_steps[i]>MAX_HARMONIC_DEGREES))
			{ 
				DEBUG("  warning-theHarmonyTypes[%d].harmony_steps[%d]=%d", j, i, theHarmonyTypes[j].harmony_steps[i]);
			}
          }
      }
	  DEBUG("AuditHarmonyData()-end");
}

void init_harmony()
{
	DEBUG("init_harmony");
   // int i,j;
  
    
	  for (int j=0;j<MAX_HARMONY_TYPES;++j)
      {
		theHarmonyTypes[j].num_harmony_steps=1;  // just so it is initialized
		theHarmonyTypes[j].min_steps=1;
	    theHarmonyTypes[j].max_steps=theHarmonyTypes[j].num_harmony_steps;
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
		strcpy(theHarmonyTypes[1].harmony_type_desc, "50's Classic R&R do-wop" );
		strcpy(theHarmonyTypes[1].harmony_degrees_desc, "I - VI - ii - V" );
	    DEBUG(theHarmonyTypes[1].harmony_type_desc);
        theHarmonyTypes[1].num_harmony_steps=4;  // 1-7
		theHarmonyTypes[1].min_steps=1;
	    theHarmonyTypes[1].max_steps=theHarmonyTypes[1].num_harmony_steps;
		theHarmonyTypes[1].harmony_steps[0]=1;
		for (int i=1; i<theHarmonyTypes[1].num_harmony_steps; ++i)
		   theHarmonyTypes[1].harmony_steps[i]=(semiCircleDegrees[theHarmonyTypes[1].num_harmony_steps-i])%7;
 	        		
	
    // (harmony_type==2)             /* typical elementary classical */
		strcpy(theHarmonyTypes[2].harmony_type_desc, "elem.. classical 1" );
		strcpy(theHarmonyTypes[2].harmony_degrees_desc, "I - IV - I - V" );
	    DEBUG(theHarmonyTypes[2].harmony_type_desc);
        theHarmonyTypes[2].num_harmony_steps=4;
		theHarmonyTypes[2].min_steps=1;
	    theHarmonyTypes[2].max_steps=theHarmonyTypes[2].num_harmony_steps;
        theHarmonyTypes[2].harmony_steps[0]=1;
        theHarmonyTypes[2].harmony_steps[1]=4;
	    theHarmonyTypes[2].harmony_steps[2]=1;
        theHarmonyTypes[2].harmony_steps[3]=5;
	
	// (harmony_type==3)             /* typical romantic */   // basically alternating between two root_keys, one major and one minor
		strcpy(theHarmonyTypes[3].harmony_type_desc, "romantic - alt root_keys" );
		strcpy(theHarmonyTypes[3].harmony_degrees_desc, "I - IV - V - I - v1 - ii - iii - vi" );
	    DEBUG(theHarmonyTypes[3].harmony_type_desc);
        theHarmonyTypes[3].num_harmony_steps=8;
		theHarmonyTypes[3].min_steps=1;
	    theHarmonyTypes[3].max_steps=theHarmonyTypes[3].num_harmony_steps;
        theHarmonyTypes[3].harmony_steps[0]=1;
        theHarmonyTypes[3].harmony_steps[1]=4;
        theHarmonyTypes[3].harmony_steps[2]=5;
        theHarmonyTypes[3].harmony_steps[3]=1;
        theHarmonyTypes[3].harmony_steps[4]=6;
        theHarmonyTypes[3].harmony_steps[5]=2;
        theHarmonyTypes[3].harmony_steps[6]=3;
        theHarmonyTypes[3].harmony_steps[7]=6;
	
    // (harmony_type==4)             /* custom                 */
		theHarmonyTypes[4].num_harmony_steps=12;
		theHarmonyTypes[4].min_steps=1;
	    theHarmonyTypes[4].max_steps=theHarmonyTypes[4].num_harmony_steps;
        for (int i=0;i<MAX_STEPS;++i)
           theHarmonyTypes[4].harmony_steps[i] = 1; // must not be 0
		
    // (harmony_type==5)             /* elementary classical 2 */
		strcpy(theHarmonyTypes[5].harmony_type_desc, "elem. classical 2" );
		strcpy(theHarmonyTypes[5].harmony_degrees_desc, "I - IV - V" );
	    DEBUG(theHarmonyTypes[5].harmony_type_desc);
        theHarmonyTypes[5].num_harmony_steps=3;
		theHarmonyTypes[5].min_steps=1;
	    theHarmonyTypes[5].max_steps=theHarmonyTypes[5].num_harmony_steps;
        theHarmonyTypes[5].harmony_steps[0]=1;
        theHarmonyTypes[5].harmony_steps[1]=4;
        theHarmonyTypes[5].harmony_steps[2]=5;

    // (harmony_type==6)             /* elementary classical 3 */
		strcpy(theHarmonyTypes[6].harmony_type_desc, "elem. classical 3" );
		strcpy(theHarmonyTypes[6].harmony_degrees_desc, "I - IV - V - IV" );
	    DEBUG("theHarmonyTypes[6].harmony_type_desc");
        theHarmonyTypes[6].num_harmony_steps=4;
		theHarmonyTypes[6].min_steps=1;
	    theHarmonyTypes[6].max_steps=theHarmonyTypes[6].num_harmony_steps;
        theHarmonyTypes[6].harmony_steps[0]=1;
        theHarmonyTypes[6].harmony_steps[1]=4;
        theHarmonyTypes[6].harmony_steps[2]=5;
        theHarmonyTypes[6].harmony_steps[3]=4;

    // (harmony_type==7)             /* strong 1 "house of rising sun"*/  
		strcpy(theHarmonyTypes[7].harmony_type_desc, "strong 1" );
		strcpy(theHarmonyTypes[7].harmony_degrees_desc, "I - iii - IV - VI" );
	    DEBUG(theHarmonyTypes[7].harmony_type_desc);
        theHarmonyTypes[7].num_harmony_steps=4;
		theHarmonyTypes[7].min_steps=1;
	    theHarmonyTypes[7].max_steps=theHarmonyTypes[7].num_harmony_steps;
        theHarmonyTypes[7].harmony_steps[0]=1;
        theHarmonyTypes[7].harmony_steps[1]=3;
        theHarmonyTypes[7].harmony_steps[2]=4;
        theHarmonyTypes[7].harmony_steps[3]=6;
       
     // (harmony_type==8)  // strong random  the harmony chord stays fixed and only the melody varies.  Good for checking harmony meander
	 	strcpy(theHarmonyTypes[8].harmony_type_desc, "strong random melody" );
		strcpy(theHarmonyTypes[8].harmony_degrees_desc, "I" );
	    DEBUG(theHarmonyTypes[8].harmony_type_desc);
        theHarmonyTypes[8].num_harmony_steps=1;
		theHarmonyTypes[8].min_steps=1;
	    theHarmonyTypes[8].max_steps=theHarmonyTypes[8].num_harmony_steps;
        theHarmonyTypes[8].harmony_steps[0]=1;

		//semiCircleDegrees[]={1, 5, 2, 6, 3, 7, 4}; 

     // (harmony_type==9)  // harmonic+   C, G, D,...  CW by 5ths
	     strcpy(theHarmonyTypes[9].harmony_type_desc, "harmonic+ CW 5ths" );
		 strcpy(theHarmonyTypes[9].harmony_degrees_desc, "I - V - ii - vi - iii - vii - IV" );
	     DEBUG(theHarmonyTypes[9].harmony_type_desc);
         theHarmonyTypes[9].num_harmony_steps=7;  // 1-7
		 theHarmonyTypes[9].min_steps=1;
	     theHarmonyTypes[9].max_steps=theHarmonyTypes[9].num_harmony_steps;
         for (int i=0;i<theHarmonyTypes[9].num_harmony_steps;++i)
           theHarmonyTypes[9].harmony_steps[i] = 1+semiCircleDegrees[i]%7;

     // (harmony_type==10)  // harmonic-  C, F#, B,...  CCW by 4ths
	    strcpy(theHarmonyTypes[10].harmony_type_desc, "harmonic- CCW 4ths" );
		strcpy(theHarmonyTypes[10].harmony_degrees_desc, "I - IV - vii - iii - VI - ii - V" );
	    DEBUG(theHarmonyTypes[10].harmony_type_desc);
        theHarmonyTypes[10].num_harmony_steps=7;  // 1-7
		theHarmonyTypes[10].min_steps=1;
	    theHarmonyTypes[10].max_steps=theHarmonyTypes[10].num_harmony_steps;
        for (int i=0;i<theHarmonyTypes[10].num_harmony_steps;++i)
           theHarmonyTypes[10].harmony_steps[i] = 1+(semiCircleDegrees[7-i])%7;

     // (harmony_type==11)  // tonal+  // C, D, E, F, ...
	    strcpy(theHarmonyTypes[11].harmony_type_desc, "tonal+" );
		strcpy(theHarmonyTypes[11].harmony_degrees_desc, "I - ii - iii - IV - V - vi - vii" );
	    DEBUG(theHarmonyTypes[11].harmony_type_desc);
        theHarmonyTypes[11].num_harmony_steps=7;  // 1-7
		theHarmonyTypes[11].min_steps=1;
	    theHarmonyTypes[11].max_steps=theHarmonyTypes[11].num_harmony_steps;
        for (int i=0;i<theHarmonyTypes[11].num_harmony_steps;++i)
		    theHarmonyTypes[11].harmony_steps[i] = 1+ i%7;

     // (harmony_type==12)  // tonal-  // C, B, A, ...
	     strcpy(theHarmonyTypes[12].harmony_type_desc, "tonal-" );
		 strcpy(theHarmonyTypes[12].harmony_degrees_desc, "I - vii - vi - V - IV - iii - ii" );
	     DEBUG(theHarmonyTypes[12].harmony_type_desc);
		 theHarmonyTypes[12].num_harmony_steps=7;  // 1-7
		 theHarmonyTypes[12].min_steps=1;
	     theHarmonyTypes[12].max_steps=theHarmonyTypes[12].num_harmony_steps;
         for (int i=0;i<theHarmonyTypes[12].num_harmony_steps;++i)
		     theHarmonyTypes[12].harmony_steps[i] = 1+ (7-i)%7;

    //semiCircleDegrees[]={1, 5, 2, 6, 3, 7, 4}; 
        
    // (harmony_type==13)             /* 12 bar blues */
	    strcpy(theHarmonyTypes[13].harmony_type_desc, "12 bar blues 1" );
		strcpy(theHarmonyTypes[13].harmony_degrees_desc, "I - I - I - I - IV - IV - I - I - V - VI - I" );
	    DEBUG(theHarmonyTypes[13].harmony_type_desc);
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
        theHarmonyTypes[13].harmony_steps[9]=6;
        theHarmonyTypes[13].harmony_steps[10]=1;
		// harmony_steps[13][11]=0;  // alternate
        theHarmonyTypes[13].harmony_steps[11]=5;
       

    // (harmony_type==14)             /* minor 12 bar blues */
		strcpy(theHarmonyTypes[14].harmony_type_desc, "12 bar blues 2" );
		strcpy(theHarmonyTypes[14].harmony_degrees_desc, "I - I - I -I - IV - IV - I - IV - IV - I - I" );
	    DEBUG(theHarmonyTypes[14].harmony_type_desc);
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
        theHarmonyTypes[14].harmony_steps[8]=4;
        theHarmonyTypes[14].harmony_steps[9]=4;
        theHarmonyTypes[14].harmony_steps[10]=1;
        theHarmonyTypes[14].harmony_steps[11]=1;
       
    // (harmony_type==15)             /* country 1 */
		strcpy(theHarmonyTypes[15].harmony_type_desc, "country 1" );
		strcpy(theHarmonyTypes[15].harmony_degrees_desc, "I - IV - V - I - I - IV - V - I" );
	    DEBUG(theHarmonyTypes[15].harmony_type_desc);
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
	    DEBUG(theHarmonyTypes[16].harmony_type_desc);
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
	    DEBUG(theHarmonyTypes[17].harmony_type_desc);
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
		strcpy(theHarmonyTypes[18].harmony_degrees_desc, "I - vi - IV - V" );
	    DEBUG(theHarmonyTypes[18].harmony_type_desc);
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
		strcpy(theHarmonyTypes[19].harmony_type_desc, "rock" );
		strcpy(theHarmonyTypes[19].harmony_degrees_desc, "I - IV" );
	    DEBUG(theHarmonyTypes[19].harmony_type_desc);
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
	    DEBUG(theHarmonyTypes[20].harmony_type_desc);
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
	    DEBUG(theHarmonyTypes[21].harmony_type_desc);
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
       
	
		// (harmony_type==22)             /* random coming home */  // I + n and descend by 4ths
		strcpy(theHarmonyTypes[22].harmony_type_desc, "random coming home" );
		strcpy(theHarmonyTypes[22].harmony_degrees_desc, "I - IV - vii - iii - vi - ii - V" );
	    DEBUG(theHarmonyTypes[22].harmony_type_desc);
        theHarmonyTypes[22].num_harmony_steps=4;  // 1-7
		theHarmonyTypes[22].min_steps=1;
	    theHarmonyTypes[22].max_steps=theHarmonyTypes[22].num_harmony_steps;
		theHarmonyTypes[22].harmony_steps[0]=1;
		for (int i=1; i<theHarmonyTypes[22].num_harmony_steps; ++i)
		   theHarmonyTypes[22].harmony_steps[i]=(semiCircleDegrees[theHarmonyTypes[22].num_harmony_steps-i])%7;

		// (harmony_type==23)             /* random coming home */  // I + n and descend by 4ths
		strcpy(theHarmonyTypes[23].harmony_type_desc, "random order" );
		strcpy(theHarmonyTypes[23].harmony_degrees_desc, "I - IV - V" );
	    DEBUG(theHarmonyTypes[23].harmony_type_desc);
        theHarmonyTypes[23].num_harmony_steps=3;  // 1-7
		theHarmonyTypes[23].min_steps=1;
	    theHarmonyTypes[23].max_steps=theHarmonyTypes[23].num_harmony_steps;
		theHarmonyTypes[23].harmony_steps[0]=1;
		theHarmonyTypes[23].harmony_steps[1]=4;
		theHarmonyTypes[23].harmony_steps[2]=5;

		// (harmony_type==24)             /* Hallelujah */  // 
		strcpy(theHarmonyTypes[24].harmony_type_desc, "Hallelujah" );
		strcpy(theHarmonyTypes[24].harmony_degrees_desc, "I-vi-I-vi-IV-V-I-I-I-IV-V-vi-IV-V-iii-v1" );
	    DEBUG(theHarmonyTypes[24].harmony_type_desc);
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
		strcpy(theHarmonyTypes[25].harmony_degrees_desc, "I - V - vi - iii - IV - I - IV - V" );
	    DEBUG(theHarmonyTypes[25].harmony_type_desc);
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
		strcpy(theHarmonyTypes[26].harmony_type_desc, "Pop Rock Classic-1" );
		strcpy(theHarmonyTypes[26].harmony_degrees_desc, "I - V - vi - IV" );
	    DEBUG(theHarmonyTypes[26].harmony_type_desc);
        theHarmonyTypes[26].num_harmony_steps=4;  // 1-8
		theHarmonyTypes[26].min_steps=1;
	    theHarmonyTypes[26].max_steps=theHarmonyTypes[26].num_harmony_steps;
		theHarmonyTypes[26].harmony_steps[0]=1;
		theHarmonyTypes[26].harmony_steps[1]=5;
		theHarmonyTypes[26].harmony_steps[2]=6;
		theHarmonyTypes[26].harmony_steps[3]=4;
		
		// (harmony_type==27)             /* Andalusion Cadence 1*/  // 
		strcpy(theHarmonyTypes[27].harmony_type_desc, "Andalusion Cadence 1" );
		strcpy(theHarmonyTypes[27].harmony_degrees_desc, "i - VII - VI - V" );
	    DEBUG(theHarmonyTypes[27].harmony_type_desc);
        theHarmonyTypes[27].num_harmony_steps=4;  // 1-8
		theHarmonyTypes[27].min_steps=1;
	    theHarmonyTypes[27].max_steps=theHarmonyTypes[27].num_harmony_steps;
		theHarmonyTypes[27].harmony_steps[0]=1;
		theHarmonyTypes[27].harmony_steps[1]=7;
		theHarmonyTypes[27].harmony_steps[2]=6;
		theHarmonyTypes[27].harmony_steps[3]=5;
		
		// (harmony_type==28)             /* 16 bar blues*/  // 
		strcpy(theHarmonyTypes[28].harmony_type_desc, "16 Bar Blues" );
		strcpy(theHarmonyTypes[28].harmony_degrees_desc, "I-I-I-I-I-I-I-I-IV-IV-I-I-V-IV-I-I" );
	    DEBUG(theHarmonyTypes[28].harmony_type_desc);
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
		strcpy(theHarmonyTypes[29].harmony_type_desc, "Black" );
		strcpy(theHarmonyTypes[29].harmony_degrees_desc, "I-I-V-V" );
	    DEBUG(theHarmonyTypes[29].harmony_type_desc);
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

		// (harmony_type==30)             /*IV */  // 
		strcpy(theHarmonyTypes[30].harmony_type_desc, "IV" );
		strcpy(theHarmonyTypes[30].harmony_degrees_desc, "I-V" );
	    DEBUG(theHarmonyTypes[30].harmony_type_desc);
        theHarmonyTypes[30].num_harmony_steps=2;  // 1-8
		theHarmonyTypes[30].min_steps=1;
	    theHarmonyTypes[30].max_steps=theHarmonyTypes[30].num_harmony_steps;
		theHarmonyTypes[30].harmony_steps[0]=1;
		theHarmonyTypes[30].harmony_steps[1]=5;

		// (harmony_type==31)             /* Markov Chain  Bach 1*/  // 
		strcpy(theHarmonyTypes[31].harmony_type_desc, "Markov Chain-Bach 1" );
		strcpy(theHarmonyTypes[31].harmony_degrees_desc, "I-ii-iii-IV-V-vi-vii" );
	    DEBUG(theHarmonyTypes[31].harmony_type_desc);
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
		strcpy(theHarmonyTypes[32].harmony_degrees_desc, "I-ii-IV-V" );
	    DEBUG(theHarmonyTypes[32].harmony_type_desc);
        theHarmonyTypes[32].num_harmony_steps=4;  // 1-8
		theHarmonyTypes[32].min_steps=1;
	    theHarmonyTypes[32].max_steps=theHarmonyTypes[32].num_harmony_steps;
		theHarmonyTypes[32].harmony_steps[0]=1;
		theHarmonyTypes[32].harmony_steps[1]=2;
		theHarmonyTypes[32].harmony_steps[2]=4;
		theHarmonyTypes[32].harmony_steps[3]=5;
		
		// (harmony_type==33)             /* Classical */  // 
		strcpy(theHarmonyTypes[33].harmony_type_desc, "Classical" );
		strcpy(theHarmonyTypes[33].harmony_degrees_desc, "I-V-I-vi-ii-V-I" );
	    DEBUG(theHarmonyTypes[33].harmony_type_desc);
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
		strcpy(theHarmonyTypes[34].harmony_degrees_desc, "I-ii-V-I" );
	    DEBUG(theHarmonyTypes[34].harmony_type_desc);
        theHarmonyTypes[34].num_harmony_steps=4;  // 1-8
		theHarmonyTypes[34].min_steps=1;
	    theHarmonyTypes[34].max_steps=theHarmonyTypes[34].num_harmony_steps;
		theHarmonyTypes[34].harmony_steps[0]=1;
		theHarmonyTypes[34].harmony_steps[1]=2;
		theHarmonyTypes[34].harmony_steps[2]=5;
		theHarmonyTypes[34].harmony_steps[3]=1;

		// (harmony_type==35)             /*Classical Tonal */  // 
		strcpy(theHarmonyTypes[35].harmony_type_desc, "Classical Tonal" );
		strcpy(theHarmonyTypes[35].harmony_degrees_desc, "I-V-I-IV-I" );
	    DEBUG(theHarmonyTypes[35].harmony_type_desc);
        theHarmonyTypes[35].num_harmony_steps=5;  // 1-8
		theHarmonyTypes[35].min_steps=1;
	    theHarmonyTypes[35].max_steps=theHarmonyTypes[35].num_harmony_steps;
		theHarmonyTypes[35].harmony_steps[0]=1;
		theHarmonyTypes[35].harmony_steps[1]=5;
		theHarmonyTypes[35].harmony_steps[2]=1;
		theHarmonyTypes[35].harmony_steps[3]=4;
		theHarmonyTypes[35].harmony_steps[4]=1;

		// (harmony_type==36)             /*Sensitive */  // 
		strcpy(theHarmonyTypes[36].harmony_type_desc, "Sensitive" );
		strcpy(theHarmonyTypes[36].harmony_degrees_desc, "VI-IV-I-V" );
	    DEBUG(theHarmonyTypes[36].harmony_type_desc);
        theHarmonyTypes[36].num_harmony_steps=4;  // 1-8
		theHarmonyTypes[36].min_steps=1;
	    theHarmonyTypes[36].max_steps=theHarmonyTypes[36].num_harmony_steps;
		theHarmonyTypes[36].harmony_steps[0]=6;
		theHarmonyTypes[36].harmony_steps[1]=4;
		theHarmonyTypes[36].harmony_steps[2]=1;
		theHarmonyTypes[36].harmony_steps[3]=5;
		
		// (harmony_type==37)             /*Jass */  // 
		strcpy(theHarmonyTypes[37].harmony_type_desc, "Jazz" );
		strcpy(theHarmonyTypes[37].harmony_degrees_desc, "ii-V-I" );
	    DEBUG(theHarmonyTypes[37].harmony_type_desc);
        theHarmonyTypes[37].num_harmony_steps=3;  // 1-8
		theHarmonyTypes[37].min_steps=1;
	    theHarmonyTypes[37].max_steps=theHarmonyTypes[37].num_harmony_steps;
		theHarmonyTypes[37].harmony_steps[0]=2;
		theHarmonyTypes[37].harmony_steps[1]=5;
		theHarmonyTypes[37].harmony_steps[2]=1;

		// (harmony_type==38)             /*Pop */  // 
		strcpy(theHarmonyTypes[38].harmony_type_desc, "Pop" );
		strcpy(theHarmonyTypes[38].harmony_degrees_desc, "I-IV-ii-V" );
	    DEBUG(theHarmonyTypes[38].harmony_type_desc);
        theHarmonyTypes[38].num_harmony_steps=4;  // 1-8
		theHarmonyTypes[38].min_steps=1;
	    theHarmonyTypes[38].max_steps=theHarmonyTypes[38].num_harmony_steps;
		theHarmonyTypes[38].harmony_steps[0]=1;
		theHarmonyTypes[38].harmony_steps[1]=4;
		theHarmonyTypes[38].harmony_steps[2]=2;
		theHarmonyTypes[38].harmony_steps[3]=5;

		// (harmony_type==39)             /*Pop */  // 
		strcpy(theHarmonyTypes[39].harmony_type_desc, "Pop" );
		strcpy(theHarmonyTypes[39].harmony_degrees_desc, "I-ii-iii-IV-V" );
	    DEBUG(theHarmonyTypes[39].harmony_type_desc);
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
		strcpy(theHarmonyTypes[40].harmony_degrees_desc, "I-iii-IV-iv" );  // can't really do a IV and iv together
	    DEBUG(theHarmonyTypes[40].harmony_type_desc);
        theHarmonyTypes[40].num_harmony_steps=4;  // 1-8
		theHarmonyTypes[40].min_steps=1;
	    theHarmonyTypes[40].max_steps=theHarmonyTypes[40].num_harmony_steps;
		theHarmonyTypes[40].harmony_steps[0]=1;
		theHarmonyTypes[40].harmony_steps[1]=3;
		theHarmonyTypes[40].harmony_steps[2]=4;
		theHarmonyTypes[40].harmony_steps[3]=4;

		// (harmony_type==41)             /*Andalusian Cadence 2 */  // 
		strcpy(theHarmonyTypes[41].harmony_type_desc, "Andalusian Cadence 2" );
		strcpy(theHarmonyTypes[41].harmony_degrees_desc, "VI-V-IV-III" );
	    DEBUG(theHarmonyTypes[41].harmony_type_desc);
        theHarmonyTypes[41].num_harmony_steps=4;  // 1-8
		theHarmonyTypes[41].min_steps=1;
	    theHarmonyTypes[41].max_steps=theHarmonyTypes[41].num_harmony_steps;
		theHarmonyTypes[41].harmony_steps[0]=6;
		theHarmonyTypes[41].harmony_steps[1]=5;
		theHarmonyTypes[41].harmony_steps[2]=4;
		theHarmonyTypes[41].harmony_steps[3]=3;
	
		// (harmony_type==42)             /* Markov Chain  Bach 2*/  // 
		strcpy(theHarmonyTypes[42].harmony_type_desc, "Markov Chain-Bach 2" );
		strcpy(theHarmonyTypes[42].harmony_degrees_desc, "I-ii-iii-IV-V-vi-vii" );
	    DEBUG(theHarmonyTypes[42].harmony_type_desc);
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
		strcpy(theHarmonyTypes[43].harmony_degrees_desc, "I-ii-iii-IV-V-vi-vii" );
	    DEBUG(theHarmonyTypes[43].harmony_type_desc);
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
		strcpy(theHarmonyTypes[44].harmony_degrees_desc, "I-ii-iii-IV-V-vi-vii" );
	    DEBUG(theHarmonyTypes[44].harmony_type_desc);
        theHarmonyTypes[44].num_harmony_steps=7;  // 1-8
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
		strcpy(theHarmonyTypes[45].harmony_degrees_desc, "I-ii-iii-IV-V-vi-vii" );
	    DEBUG(theHarmonyTypes[45].harmony_type_desc);
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
		strcpy(theHarmonyTypes[46].harmony_degrees_desc, "I-ii-iii-IV-V-vi-vii" );
	    DEBUG(theHarmonyTypes[46].harmony_type_desc);
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
		strcpy(theHarmonyTypes[47].harmony_degrees_desc, "I-ii-iii-IV-V-vi-vii" );
	    DEBUG(theHarmonyTypes[47].harmony_type_desc);
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
		strcpy(theHarmonyTypes[48].harmony_type_desc, "Markov Chain-I-IV-V" );
		strcpy(theHarmonyTypes[48].harmony_degrees_desc, "I-ii-iii-IV-V-vi-vii" );
	    DEBUG(theHarmonyTypes[48].harmony_type_desc);
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
		strcpy(theHarmonyTypes[49].harmony_degrees_desc, "I-VI-II-V" );
	    DEBUG(theHarmonyTypes[49].harmony_type_desc);
        theHarmonyTypes[49].num_harmony_steps=4;  // 1-8
		theHarmonyTypes[49].min_steps=1;
	    theHarmonyTypes[49].max_steps=theHarmonyTypes[49].num_harmony_steps;
		theHarmonyTypes[49].harmony_steps[0]=1;
		theHarmonyTypes[49].harmony_steps[1]=6; 
		theHarmonyTypes[49].harmony_steps[2]=2;
		theHarmonyTypes[49].harmony_steps[3]=5;

		// (harmony_type==50)             /*Jazz 3 */  // 
		strcpy(theHarmonyTypes[50].harmony_type_desc, "Jazz 3" );
		strcpy(theHarmonyTypes[50].harmony_degrees_desc, "III-VI-II-V" );
	    DEBUG(theHarmonyTypes[50].harmony_type_desc);
        theHarmonyTypes[50].num_harmony_steps=4;  // 1-8
		theHarmonyTypes[50].min_steps=1;
	    theHarmonyTypes[50].max_steps=theHarmonyTypes[50].num_harmony_steps;
		theHarmonyTypes[50].harmony_steps[0]=3;
		theHarmonyTypes[50].harmony_steps[1]=6;
		theHarmonyTypes[50].harmony_steps[2]=2;
		theHarmonyTypes[50].harmony_steps[3]=5;

		// (harmony_type==51)             /*Jazz 4 */  // 
		strcpy(theHarmonyTypes[51].harmony_type_desc, "Jazz 4" );
		strcpy(theHarmonyTypes[51].harmony_degrees_desc, "I-IV-III-VI" );
	    DEBUG(theHarmonyTypes[51].harmony_type_desc);
        theHarmonyTypes[51].num_harmony_steps=4;  // 1-8
		theHarmonyTypes[51].min_steps=1;
	    theHarmonyTypes[51].max_steps=theHarmonyTypes[51].num_harmony_steps;
		theHarmonyTypes[51].harmony_steps[0]=1;
		theHarmonyTypes[51].harmony_steps[1]=4;
		theHarmonyTypes[51].harmony_steps[2]=3;
		theHarmonyTypes[51].harmony_steps[3]=6;


		// End of preset harmony types
}

void copyHarmonyTypeToActiveHarmonyType(int harmType)
{
	theActiveHarmonyType.harmony_type=harmType;  // the parent harmony_type
	theActiveHarmonyType.num_harmony_steps=theHarmonyTypes[harmType].num_harmony_steps;
	theActiveHarmonyType.min_steps=theHarmonyTypes[harmType].min_steps;
	theActiveHarmonyType.max_steps=theHarmonyTypes[harmType].max_steps;
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
	DEBUG("setup_harmony-begin"); 
    int i,j,k;
    int circle_position=0;
	int circleDegree=0;
		
    DEBUG("theHarmonyTypes[%d].num_harmony_steps=%d", harmony_type, theActiveHarmonyType.num_harmony_steps);   	
    for(i=0;i<theActiveHarmonyType.num_harmony_steps;++i)              /* for each of the harmony steps         */
     {           
	   DEBUG("step=%d", i);                                /* build proper chord notes              */
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
		   if (j==7)
		   {
	  		   DEBUG("  warning circleposition could not be found 1");
		   }
	   }
	 
	   DEBUG("  circle_position=%d  num_root_key_notes[circle_position]=%d", circle_position, num_root_key_notes[circle_position]);

       for(j=0;j<num_root_key_notes[circle_position];++j)
        {
			int root_key_note=root_key_notes[circle_of_fifths[circle_position]][j];
			DEBUG("root_key_note=%d %s", root_key_note, note_desig[root_key_note%MAX_NOTES]);
			
			int thisStepChordType=theCircleOf5ths.Circle5ths[circle_position].chordType;

			if (true)  // attempting to handle 7ths
			{
				if  ((theMeanderState.theHarmonyParms.enable_all_7ths)|| // override all chord to 7th
				    ((theMeanderState.theHarmonyParms.enable_V_7ths)&&(circleDegree==5)))  // override V chord to 7th
				{	
					if (true)
					{
						if (thisStepChordType==0)  // maj
							thisStepChordType=3; // 7maj
						else
						if (thisStepChordType==1)  // min
							thisStepChordType=4; // 7min
						else
						if (thisStepChordType==6)  // dim
							thisStepChordType=5; // dim7
						theCircleOf5ths.Circle5ths[circle_position].chordType=thisStepChordType;
					}
				}
			}
			
          	if ((root_key_note%MAX_NOTES)==circle_of_fifths[circle_position])
		    {
				DEBUG("  root_key_note=%d %s", root_key_note, note_desig[root_key_note%MAX_NOTES]);
             	for (k=0;k<chord_type_num_notes[thisStepChordType];++k)
				{  
					step_chord_notes[i][num_step_chord_notes[i]]=(int)((int)root_key_note+(int)chord_type_intervals[thisStepChordType][k]);
					DEBUG("    step_chord_notes[%d][%d]= %d %s", i, num_step_chord_notes[i], step_chord_notes[i][num_step_chord_notes[i]], note_desig[step_chord_notes[i][num_step_chord_notes[i]]%MAX_NOTES]);
					++num_step_chord_notes[i];
				}
			}   
       }
		
	   if (true)  // if this is not done, step_chord_notes[0] begins with root note.   If done, chord spread is limited but smoother wandering through innversions
	   {
		    DEBUG("refactor:");
			for (j=0;j<num_step_chord_notes[i];++j)
			{
				step_chord_notes[i][j]=step_chord_notes[i][j+((11-circle_of_fifths[circle_position])/3)];
				DEBUG("step_chord_notes[%d][%d]= %d %s", i, j, step_chord_notes[i][j], note_desig[step_chord_notes[i][j]%MAX_NOTES]);
			}
			num_step_chord_notes[i]-=((11-circle_of_fifths[circle_position])/3);
	   }
     }
	 AuditHarmonyData(1);
	 DEBUG("setup_harmony-end");
}


void MeanderMusicStructuresInitialize()
{
	DEBUG("MeanderMusicStructuresInitialize()");
	
	init_vars();
	init_notes();
	init_harmony();
	copyHarmonyTypeToActiveHarmonyType(harmony_type);
	setup_harmony();
	initialized=true;  // prevents process() from doing anything before initialization
} 


		void ConstructCircle5ths(int circleRootKey, int mode)
		{
			DEBUG("ConstructCircle5ths()");

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
			DEBUG("ConstructDegreesSemicircle()");
			const float rotate90 = (M_PI) / 2.0;
			float offsetDegree=((circleRootKey-mode+12)%12)*(2.0*M_PI/12.0);
			theCircleOf5ths.theDegreeSemiCircle.OffsetSteps=(circleRootKey-mode); 
			DEBUG("theCircleOf5ths.theDegreeSemiCircle.OffsetSteps=%d", theCircleOf5ths.theDegreeSemiCircle.OffsetSteps);
			theCircleOf5ths.theDegreeSemiCircle.RootKeyCircle5thsPosition=-theCircleOf5ths.theDegreeSemiCircle.OffsetSteps+circle_root_key;
			DEBUG("RootKeyCircle5thsPositions=%d", theCircleOf5ths.theDegreeSemiCircle.RootKeyCircle5thsPosition);

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
					DEBUG("theCircleOf5ths.theDegreeSemiCircle.degreeElements[%d].CircleIndex=%d", i, theCircleOf5ths.theDegreeSemiCircle.degreeElements[i].CircleIndex); 
				
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
					DEBUG("theCircleOf5ths.theDegreeSemiCircle.degreeElements[%d].Degree=%d", i, theCircleOf5ths.theDegreeSemiCircle.degreeElements[i].Degree);
			}	

			//
			DEBUG("");
			DEBUG("Map arabic steps to semicircle steps:");
			for (int i=1; i<8; ++i)  // for arabic steps  1-7 , i=1 for 1 based indexing
			{	
				DEBUG("arabic step=%d", i);
				for (int j=0; j<7; ++j)  // for semicircle steps
				{
					if (theCircleOf5ths.theDegreeSemiCircle.degreeElements[j].Degree==i)
					{
						arabicStepDegreeSemicircleIndex[i]=j;  
						DEBUG("  arabicStepDegreeSemicircleIndex=%d circleposition=%d", arabicStepDegreeSemicircleIndex[i], theCircleOf5ths.theDegreeSemiCircle.degreeElements[arabicStepDegreeSemicircleIndex[i]].CircleIndex);
						break;
					}
				}
			}

						
			DEBUG("");
			DEBUG("SemiCircle degrees:");
			for (int i=0; i<7; ++i)
			{
				DEBUG("theCircleOf5ths.theDegreeSemiCircle.degreeElements[%d].Degree=%d %s", i, theCircleOf5ths.theDegreeSemiCircle.degreeElements[i].Degree, circle_of_fifths_arabic_degrees[theCircleOf5ths.theDegreeSemiCircle.degreeElements[i].Degree]);
			}

			DEBUG("");
			DEBUG("circle position chord types");
			for (int i=0; i<12; ++i)
			{
				DEBUG("theCircleOf5ths.Circle5ths[%d].chordType=%d", i, theCircleOf5ths.Circle5ths[i].chordType);
			}	

			DEBUG("");
			DEBUG("circle indices");	
			for (int i=0; i<MAX_HARMONIC_DEGREES; ++i)
			{
				DEBUG("theCircleOf5ths.theDegreeSemiCircle.degreeElements[%d].CircleIndex=%d", i, theCircleOf5ths.theDegreeSemiCircle.degreeElements[i].CircleIndex); 
			}
			DEBUG("");	
		};


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
		BUTTON_BASS_AGOGIC_PARAM,
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

	void userPlaysCircleDegree(int circle_position, float octaveOffset)  // C=0
	{
		DEBUG("userPlaysCircleDegree(%d)", circle_position);
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
				outputs[OUT_HARMONY_GATE_OUTPUT].setVoltage(theMeanderState.theHarmonyParms.volume);
				outputs[OUT_HARMONY_TRIGGER_OUTPUT].setVoltage(CV_MAX10);  // this should trigger ADSR, but should be a pulse rather than stay high
				harmonyTriggerPulse.trigger(1e-3f);  // 1ms duration  need to use .process to detect this and then send it to output
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
			outputs	[OUT_HARMONY_GATE_OUTPUT].setVoltage(theMeanderState. theHarmonyParms.volume);
			harmonyTriggerPulse.trigger(1e-3f);  // 1ms duration  need to use .process to detect this and then send it to output
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
		
		CircleStepStates[step] = true;

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
					harmonyTriggerPulse.trigger(1e-3f);  // 1ms duration  need to use .process to detect this and then send it to output	
				}
		}
		if (theMeanderState.theHarmonyParms.enabled)
		{ 
			outputs[OUT_HARMONY_GATE_OUTPUT].setVoltage(theMeanderState. theHarmonyParms.volume);

			// output some fBm noise
			outputs[OUT_FBM_GATE_OUTPUT].setChannels(1);  // set polyphony  
			outputs[OUT_FBM_GATE_OUTPUT].setVoltage((float)fBmrand ,0); 

		    harmonyTriggerPulse.trigger(1e-3f);  // 1ms duration  need to use .process to detect this and then send it to output
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
				outputs[OUT_MELODY_GATE_OUTPUT].setVoltage(theMeanderState.theMelodyParms.volume);

				// output some fBm noise
				outputs[OUT_FBM_CV_OUTPUT].setChannels(1);  // set polyphony  
				outputs[OUT_FBM_CV_OUTPUT].setVoltage((float)fBmrand ,0); 

				melodyTriggerPulse.trigger(1e-3f);  // 1ms duration  need to use .process to detect this and then send it to output
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
			theMeanderState.theArpParms.last[theMeanderState.theArpParms.note_count].length=theMeanderState.theArpParms.increment;
			theMeanderState.theArpParms.last[theMeanderState.theArpParms.note_count].time32s=barts_count;
			theMeanderState.theArpParms.last[theMeanderState.theArpParms.note_count].countInBar=bar_note_count;
			if (bar_note_count<256)
			played_notes_circular_buffer[bar_note_count++]=theMeanderState.theArpParms.last[theMeanderState.theArpParms.note_count];
		}
	
		outputs[OUT_MELODY_CV_OUTPUT].setChannels(1);  // set polyphony  may need to deal with unset channel voltages
		outputs[OUT_MELODY_CV_OUTPUT].setVoltage(((note_to_play/12.0) -4.0) ,0);  // (note, channel) -4 since midC=c4=0voutputs[OUT_MELODY_CV_OUTPUT].setVoltage((note_to_play -4 + theMeanderState.theMelodyParms.target_octave,0);  // (note, channel) -4 since midC=c4=0v
		outputs[OUT_MELODY_GATE_OUTPUT].setVoltage(volume);
		melodyTriggerPulse.trigger(1e-3f);  // 1ms duration  need to use .process to detect this and then send it to output			
	}


	void doBass()
	{
		DEBUG("doBass()");
	//	clock_t current_cpu_t= clock();  // cpu clock ticks since program began
	//	double current_cpu_time_double= (double)(current_cpu_t) / (double)CLOCKS_PER_SEC;
		//	DEBUG("current_cpu_time=%ld", (long)current_cpu_t);
		//	DEBUG("current_cpu_time_double=%.3lf", (double)current_cpu_time_double);

				
		if (theMeanderState.theBassParms.enabled) 
		{
			++theMeanderState.theBassParms.bar_bass_counted_note;
			if ((theMeanderState.theBassParms.syncopate)&&(theMeanderState.theBassParms.bar_bass_counted_note==2))  // experimenting with bass patterns
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
			if (!theMeanderState.theBassParms.accent)
			{
				outputs[OUT_BASS_GATE_OUTPUT].setVoltage(theMeanderState.theBassParms.volume);
			}
			else
			{
				if (theMeanderState.theBassParms.bar_bass_counted_note==1)  // experimenting with bass patterns
					outputs[OUT_BASS_GATE_OUTPUT].setVoltage(theMeanderState.theBassParms.volume);
				else
					outputs[OUT_BASS_GATE_OUTPUT].setVoltage(0.8*theMeanderState.theBassParms.volume);
			}	
			
			bassTriggerPulse.trigger(1e-3f);  // 1ms duration  need to use .process to detect this and then send it to output
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

	
	dsp::SchmittTrigger MelodyDestutterToggle;
	dsp::SchmittTrigger MelodyEnableChordalToggle;
	dsp::SchmittTrigger MelodyEnableScalerToggle;

	dsp::SchmittTrigger BassSyncopateToggle;
	dsp::SchmittTrigger BassAccentToggle;
	
	dsp::SchmittTrigger RunToggle;
		
	
	dsp::SchmittTrigger CircleStepToggles[MAX_STEPS];
	dsp::SchmittTrigger CircleStepSetToggles[MAX_STEPS];
	bool CircleStepStates[MAX_STEPS]={};
	bool CircleStepSetStates[MAX_STEPS]={};

	rack::dsp::PulseGenerator harmonyTriggerPulse; 
	rack::dsp::PulseGenerator melodyTriggerPulse; 
	rack::dsp::PulseGenerator bassTriggerPulse; 

	rack::dsp::PulseGenerator barTriggerPulse; 

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
				outputs[OUT_HARMONY_GATE_OUTPUT].setVoltage(0);
				outputs[OUT_MELODY_GATE_OUTPUT].setVoltage(0);
				outputs[OUT_BASS_GATE_OUTPUT].setVoltage(0);
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
			
					if (theMeanderState.theHarmonyParms.chord_on_note_divisor==1)
						doHarmony();
					if (theMeanderState.theBassParms.bass_on_note_divisor==1)
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
					if (theMeanderState.theHarmonyParms.chord_on_note_divisor==2)
						doHarmony();
					if (theMeanderState.theBassParms.bass_on_note_divisor==2)
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
			
					if (theMeanderState.theHarmonyParms.chord_on_note_divisor==4)
						doHarmony();
					if (theMeanderState.theBassParms.bass_on_note_divisor==4)
						doBass();
					if (theMeanderState.theMelodyParms.note_length_divisor==4)
					{
						doMelody();
						melodyPlayed=true;
					}
					if ((theMeanderState.theArpParms.enabled)&&(theMeanderState.theArpParms.increment==4)&&(!melodyPlayed))
						doArp();
					clockPulse4ts.trigger(trigger_length);
					// Pulse the output gate 
				//	barTriggerPulse.trigger(1e-3f);  // 1ms duration  need to use .process to detect this and then send it to output
				}
					 
		 		// i8ts
				if (i8ts_count == i8ts_count_limit)
				{
					i8ts_count = 0;  
					if (theMeanderState.theHarmonyParms.chord_on_note_divisor==8)
						doHarmony();
					if (theMeanderState.theBassParms.bass_on_note_divisor==8)
						doBass();
					if (theMeanderState.theMelodyParms.note_length_divisor==8)
					{
						doMelody();
						melodyPlayed=true;
					}
					if ((theMeanderState.theArpParms.enabled)&&(theMeanderState.theArpParms.increment==8)&&(!melodyPlayed))
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
					if ((theMeanderState.theArpParms.enabled)&&(theMeanderState.theArpParms.increment==16)&&(!melodyPlayed))
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
				if ((theMeanderState.theArpParms.enabled)&&(theMeanderState.theArpParms.increment==32)&&(!melodyPlayed))
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
		outputs[OUT_HARMONY_TRIGGER_OUTPUT].setVoltage( harmonyTriggerPulse.process( 1.0 / APP->engine->getSampleRate() ) ? CV_MAX10 : 0.0 ); 
		if (theMeanderState.theMelodyParms.enabled && !theMeanderState.theMelodyParms.stutter_detected)
		outputs[OUT_MELODY_TRIGGER_OUTPUT].setVoltage( melodyTriggerPulse.process( 1.0 / APP->engine->getSampleRate() ) ? CV_MAX10 : 0.0 ); 
		outputs[OUT_BASS_TRIGGER_OUTPUT].setVoltage( bassTriggerPulse.process( 1.0 / APP->engine->getSampleRate() ) ? CV_MAX10 : 0.0 ); 
		
		
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
		
		//***************
			 
		

		for (int i=0; i<12; ++i) {
			if (CircleStepToggles[i].process(params[BUTTON_CIRCLESTEP_C_PARAM+i].getValue())) {
				CircleStepStates[i] = !CircleStepStates[i];
				lights[LIGHT_LEDBUTTON_CIRCLESTEP_1+i].value=CircleStepStates[i] ? 1.0f : 0.0f;	
			
				userPlaysCircleDegree(i, 0); 
				theMeanderState.userControllingHarmonyFromCircle=true;
				theMeanderState. theHarmonyParms.enabled=false;
				lights[LIGHT_LEDBUTTON_HARMONY_ENABLE].value = theMeanderState. theHarmonyParms.enabled ? 1.0f : 0.0f; 

				for (int j=0; j<12; ++j) {
					if (j!=i) {
						CircleStepStates[j] = false;
						lights[LIGHT_LEDBUTTON_CIRCLESTEP_1+j].value=0.0f;
					}
				}
			
			}
		}
			
		if (!running)
		{
			for (int i=0; i<theActiveHarmonyType.num_harmony_steps; ++i) {
				if (CircleStepSetToggles[i].process(params[BUTTON_HARMONY_SETSTEP_1_PARAM+i].getValue())) {

					int current_circle_position=0;
					if (true)
					{
						int degreeStep=(theActiveHarmonyType.harmony_steps[i])%8;  
						
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
					userPlaysCircleDegree(current_circle_position, 0);  // testing play
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

			userPlaysCircleDegree(theCirclePosition, octave); 

			
			theMeanderState.userControllingHarmonyFromCircle=true;
			theMeanderState. theHarmonyParms.enabled=false;
		

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

				theMeanderState.theArpParms.increment=(int)std::pow(2,melody_note_length_divisor+1);
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
			if ((ivalue)!=theMeanderState.theHarmonyParms.chord_on_note_divisor)
			{
				theMeanderState.theHarmonyParms.chord_on_note_divisor=ivalue;  
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
			}

			fvalue=params[CONTROL_BASS_DIVISOR_PARAM].getValue();
			ivalue=(int)fvalue;
			ivalue=pow(2,ivalue);
			if ((ivalue)!=theMeanderState.theBassParms.bass_on_note_divisor)
			{
				theMeanderState.theBassParms.bass_on_note_divisor=ivalue;  
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
			if (ivalue!=theMeanderState.theArpParms.increment)
			{
				theMeanderState.theArpParms.increment=ivalue;  
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
		configParam(BUTTON_BASS_AGOGIC_PARAM, 0.f, 1.f, 0.f, "");
					
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
	
	Meander *module; 
	int frame = 0;
	std::shared_ptr<Font> font; 
	
	MeanderProgressionPresetLineDisplay() {
		font = APP->window->loadFont(asset::plugin(pluginInstance, "res/EurostileBold.ttf"));
	}

	void draw(const DrawArgs &ctx) override {

		if (module == NULL) { 
			return;
		} 

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
		pos = Vec(-35, 20); // this is the offset if any in the passed box position, particularly x indention -7.3=box height
		snprintf(text, sizeof(text), "%s           ",  theActiveHarmonyType.harmony_degrees_desc);
		nvgText(ctx.vg, pos.x, pos.y, text, NULL);
	
	} 

};

struct RootKeySelectLineDisplay : TransparentWidget {
	
	Meander *module;
	int frame = 0;
	std::shared_ptr<Font> font;

	RootKeySelectLineDisplay() {
		font = APP->window->loadFont(asset::plugin(pluginInstance, "res/EurostileBold.ttf"));
	}

	void draw(const DrawArgs &ctx) override {

		if (module == NULL) {
			return;
		}

		Vec pos = Vec(18,-11); // this is the offset if any in the passed box position, particularly x indention -7.3=box height
	
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
	
	Meander *module;
	int frame = 0;
	std::shared_ptr<Font> font;

	ScaleSelectLineDisplay() {
		font = APP->window->loadFont(asset::plugin(pluginInstance, "res/EurostileBold.ttf"));
	}

	void draw(const DrawArgs &ctx) override {

		if (module == NULL) {
			return;
		}


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

  Meander *module; 

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
	Meander* module;
	struct CircleOf5thsDisplay : TransparentWidget 
	{
		Meander* module;
				
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
			snprintf(text, sizeof(text), "%d", (int)theMeanderState.theHarmonyParms.chord_on_note_divisor);
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
			snprintf(text, sizeof(text), "%d", (int)theMeanderState.theBassParms.bass_on_note_divisor);
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
			snprintf(text, sizeof(text), "%d", theMeanderState.theArpParms.increment);
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
		//	if (!module)
		//		return; 
								
			DrawCircle5ths(args, root_key);  // has to be done each frame as panel redraws as SVG and needs to be blanked and cirecles redrawn
			DrawDegreesSemicircle(args,  root_key);
			updatePanelText(args);
		}

		
	
	};  // end struct CircleOf5thsDisplay

	MeanderWidget(Meander* module) 
	{
		DEBUG("MeanderWidget()");
		setModule(module);
		this->module = module;
	
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Meander.svg")));
					
		rack::random::init();  // must be called per thread

	//	 if (module)   // during preview, module is null
		 if (true)   // must be executed in order to see ModuleWidget panel display in preview
		 {
			if (module) 
			if(!module->instanceRunning) {
				box.size.x = mm2px(5.08 * 32);
				int middle = box.size.x / 2 + 7;
				addChild(new RSLabelCentered(middle+97, (box.size.y / 2)-20, "DISABLED", 28));	
				addChild(new RSLabelCentered(middle+97, (box.size.y / 2)+50, "DISABLED", 28));	
				addChild(new RSLabelCentered(middle+97, (box.size.y / 2) -11, "ONLY ONE INSTANCE OF MEANDER REQUIRED",7));
				addChild(new RSLabelCentered(middle+97, (box.size.y / 2) +20, "ONLY ONE INSTANCE OF MEANDER REQUIRED",7));
				return;
			}

			MeanderProgressionPresetLineDisplay *ProgressionPresetDisplay = createWidget<MeanderProgressionPresetLineDisplay>(mm2px(Vec(186.1, (128.93-37.5))));  // From SVG file  186.1, (panelheight - boxY - boxheight)
		   	ProgressionPresetDisplay->module = module;
			ProgressionPresetDisplay->box.size = mm2px(Vec(46, 7.3));  // from SVG file
			addChild(ProgressionPresetDisplay);

			
			RootKeySelectLineDisplay *MeanderRootKeySelectDisplay = createWidget<RootKeySelectLineDisplay>(mm2px(Vec(39.0, (128.93-56.5))));  // From SVG file  186.1, (panelheight - boxY - boxheight)
		   	MeanderRootKeySelectDisplay->module = module;
			MeanderRootKeySelectDisplay->box.size = mm2px(Vec(13.8, 7.2));  // from SVG file
			addChild(MeanderRootKeySelectDisplay);

			ScaleSelectLineDisplay *MeanderScaleSelectDisplay = createWidget<ScaleSelectLineDisplay>(mm2px(Vec(26.5, (128.93-42.9))));  // From SVG file  186.1, (panelheight - boxY - boxheight)
		   	MeanderScaleSelectDisplay->module = module;
			MeanderScaleSelectDisplay->box.size = mm2px(Vec(13.8, 7.2));  // from SVG file
			addChild(MeanderScaleSelectDisplay);

			CircleOf5thsDisplay *display = new CircleOf5thsDisplay();
			display->module = module;
			
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
			if (module) {
			BPMdisplay->value = &module->tempo;
			}
			addChild(BPMdisplay); 
			
			//SIG TOP DISPLAY 
			SigDisplayWidget *SigTopDisplay = new SigDisplayWidget();
			SigTopDisplay->box.pos = mm2px(Vec(42.2-4.5,50.9-5.6+3.0));
			SigTopDisplay->box.size = mm2px(Vec(8.3, 5.6));
			if (module) {
		
			SigTopDisplay->value = &time_sig_top;
			}
			addChild(SigTopDisplay);
			//SIG TOP KNOB
		
			//SIG BOTTOM DISPLAY    
			SigDisplayWidget *SigBottomDisplay = new SigDisplayWidget();
			SigBottomDisplay->box.pos = mm2px(Vec(42.2-4.5,56-5.6+3.0));
			SigBottomDisplay->box.size = mm2px(Vec(8.3, 5.6));
			if (module) {
			SigBottomDisplay->value = &time_sig_bottom;
			}
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

			addParam(createParamCentered<LEDButton>(mm2px(Vec(305,  53.217)), module, Meander::BUTTON_BASS_AGOGIC_PARAM));
		
			{
				auto w= createParamCentered<Trimpot>(mm2px(Vec(305, 61.217)), module, Meander::CONTROL_BASS_DIVISOR_PARAM);
				w->snap=true;
				addParam(w); 
			} 

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

			addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(266.982, 107.333)), module, Meander::OUT_MELODY_GATE_OUTPUT));
			addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(201.789, 107.616)), module, Meander::OUT_HARMONY_GATE_OUTPUT));
			addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(325.12, 107.616)), module, Meander::OUT_BASS_GATE_OUTPUT));
			
			addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(266.899, 115.813)), module, Meander::OUT_MELODY_CV_OUTPUT));
			
			addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(325.266, 115.817)), module, Meander::OUT_BASS_CV_OUTPUT));
			addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(202.176, 115.909)), module, Meander::OUT_HARMONY_CV_OUTPUT));
			addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(78.74, 122.291)), module, Meander::OUT_CLOCK_BEATX2_OUTPUT));
			addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(37.143, 122.537)), module, Meander::OUT_CLOCK_BAR_OUTPUT));
			addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(99.342, 122.573)), module, Meander::OUT_CLOCK_BEATX4_OUTPUT));
			addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(121.073, 122.573)), module, Meander::OUT_CLOCK_BEATX8_OUTPUT));
			addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(57.856, 122.856)), module, Meander::OUT_CLOCK_BEAT_OUTPUT));
			addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(325.402, 123.984)), module, Meander::OUT_BASS_TRIGGER_OUTPUT));
			addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(202.071, 124.267)), module, Meander::OUT_HARMONY_TRIGGER_OUTPUT));
			addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(266.982, 124.549)), module, Meander::OUT_MELODY_TRIGGER_OUTPUT));

						
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
		else
		{

			DEBUG("Warning! module is null in MeanderWidget()");
		}
	}    // end MeanderWidget(Meander* module)  
 
		
	void step() override   // note, this is a widget step() which is not deprecated and is a GUI call.  This advances UI by one "frame"
	{  
		if(!module) return;
	
		ModuleWidget::step();
	} 
};

Model* modelMeander = createModel<Meander, MeanderWidget>("Meander");


