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
	int bar_harmony_chords_counted_note=0;
	bool enable_all_7ths=false;
	bool enable_V_7ths=false;
    bool enable_staccato=false;
	int pending_step_edit=0;
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
    bool enable_staccato=true;
	struct note last[1];
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
char circle_of_fifths_degrees[][MAXSHORTSTRLEN]= {
	"I", "V", "II", "vi", "iii", "vii", "IV"
};

char circle_of_fifths_arabic_degrees[][MAXSHORTSTRLEN]= {
	"", "I", "II", "III", "IV", "V", "IV", "VII"
};

char circle_of_fifths_degrees_UC[][MAXSHORTSTRLEN]= {
	"I", "V", "II", "VI", "III", "VII", "IV"
};

char circle_of_fifths_degrees_LC[][MAXSHORTSTRLEN]= {
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
        strcpy(theHarmonyTypes[4].harmony_type_desc, "custom" );
		theHarmonyTypes[4].num_harmony_steps=4;
		theHarmonyTypes[4].min_steps=1;
	    theHarmonyTypes[4].max_steps=16;
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

void ConfigureGlobals()
{
    ConstructCircle5ths(circle_root_key, mode);
    ConstructDegreesSemicircle(circle_root_key, mode); //int circleroot_key, int mode)
    init_notes();  // depends on mode and root_key			
    init_harmony();  // sets up original progressions
    AuditHarmonyData(3);
    setup_harmony();  // calculate harmony notes
}





