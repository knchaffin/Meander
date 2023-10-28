/*  Copyright (C) 2019-2022 Ken Chaffin
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTYMeanderDegreeOutRangeItem without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.
*/

//*********************************************************Globals************************************************

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
#define MAX_AVAILABLE_HARMONY_PRESETS 80  // change this as new harmony presets are created 

#define MAX_PARAMS 200
#define MAX_INPORTS 100
#define MAX_OUTPORTS 100

struct TinyPJ301MPort : SvgPort {
	TinyPJ301MPort() {
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/TinyPJ301M.svg")));
	}
};

// make it a power of 8
#define MAXSHORTSTRLEN 16

#define MAX_ROOT_KEYS 12

enum noteTypes 
{
	NOTE_TYPE_CHORD,
	NOTE_TYPE_MELODY,
	NOTE_TYPE_ARP,
	NOTE_TYPE_BASS,
	NOTE_TYPE_EXTERNAL
};

static const char MSQ_note_desig_sharps[MAX_NOTES][MAXSHORTSTRLEN]={"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"};
static const char MSQ_note_desig_flats[MAX_NOTES][MAXSHORTSTRLEN]={"C","Db","D","Eb","E","F","Gb","G","Ab","A","Bb","B"};

static const char MSQ_root_key_names[MAX_ROOT_KEYS][MAXSHORTSTRLEN] = { "C","Db","D","Eb","E","F","F#","G","Ab","A","Bb","B" };

#define MAX_MODES 7
static const int MSQ_num_modes=MAX_MODES;
static const char MSQ_mode_names[MAX_MODES][20] = {
	"Lydian (maj)",
	"Ionian/Major",
	"Mixolydian (maj)",
	"Dorian (min)",
	"Aeolian/NMinor",
	"Phrygian (min)",
	"Locrian (min/dim)"
};

static const int MSQ_mode_step_intervals[7][13] = {  // num mode scale notes, semitones to next note  7 modes
	{ 7, 2,2,2,1,2,2,1,0,0,0,0,0},                // Lydian  	        
	{ 7, 2,2,1,2,2,2,1,0,0,0,0,0},                // Major/Ionian      
	{ 7, 2,2,1,2,2,1,2,0,0,0,0,0},                // Mixolydian	   
	{ 7, 2,1,2,2,2,1,2,0,0,0,0,0},                // Dorian           
	{ 7, 2,1,2,2,1,2,2,0,0,0,0,0},                // NMinor/Aeolian   
	{ 7, 1,2,2,2,1,2,2,0,0,0,0,0},                // Phrygian         
	{ 7, 1,2,2,1,2,2,2,0,0,0,0,0}                 // Locrian            
}; 

static const int MSQ_root_key_signatures_chromaticForder[12][7]=  // chromatic order 0=natural, 1=sharp, -1=flat
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

static const char* MSQ_CircleNoteNames[MAX_NOTES] = {"C","G","D","A","E","B","F#","Db","Ab","Eb","Bb","F"};

static const char MSQ_circle_of_fifths_degrees[][MAXSHORTSTRLEN]= {
	"I", "V", "II", "vi", "iii", "vii", "IV"
};

static const char MSQ_circle_of_fifths_arabic_degrees[][MAXSHORTSTRLEN]= {
	"", "I", "II", "III", "IV", "V", "VI", "VII"
};

static const char MSQ_circle_of_fifths_degrees_UC[][MAXSHORTSTRLEN]= {
	"I", "V", "II", "VI", "III", "VII", "IV"
};

static const char MSQ_circle_of_fifths_degrees_LC[][MAXSHORTSTRLEN]= {
	"i", "v", "ii", "vi", "iii", "vii", "iv"
};

static const int MSQ_root_key_sharps_vertical_display_offset[6]={1, 4, 0, 3, 6, 2};  // left to right
static const int MSQ_root_key_flats_vertical_display_offset[6]={5, 2, 6, 3, 7, 4};   // right to left

// these are not static and may be written to by any module but will be read by all modules
int MSQ_panelTheme=0;
float MSQ_panelContrast=1.0;
float MSQ_panelContrastDefault = 1.0f;  // reset value 

// base colors remain fixed
NVGcolor MSQ_panelHarmonyPartBaseColor=nvgRGBA((unsigned char)255,(unsigned char)0,(unsigned char)0,(unsigned char)255);  // red
NVGcolor MSQ_panelMelodyPartBaseColor=nvgRGBA((unsigned char)0,(unsigned char)0,(unsigned char)0,(unsigned char)255);  // black
NVGcolor MSQ_panelArpPartBaseColor=nvgRGBA((unsigned char)0,(unsigned char)0,(unsigned char)255,(unsigned char)255);  // blue
NVGcolor MSQ_panelBassPartBaseColor=nvgRGBA((unsigned char)0,(unsigned char)255,(unsigned char)0,(unsigned char)255);  // green

// working colors depent on theme and conttrast.
NVGcolor MSQ_panelcolor=nvgRGBA((unsigned char)230,(unsigned char)230,(unsigned char)230,(unsigned char)255);  // whiteish
NVGcolor MSQ_panelTextColor=nvgRGBA((unsigned char)0,(unsigned char)0,(unsigned char)0,(unsigned char)255);  // black
NVGcolor MSQ_paramTextColor=nvgRGBA((unsigned char)255,(unsigned char)255,(unsigned char)0,(unsigned char)255);  // yellow
NVGcolor MSQ_panelLineColor=nvgRGBA((unsigned char)255,(unsigned char)255,(unsigned char)255,(unsigned char)255);  // white

NVGcolor MSQ_panelHarmonyPartColor=nvgRGBA((unsigned char)255,(unsigned char)0,(unsigned char)0,(unsigned char)255);  // red
NVGcolor MSQ_panelMelodyPartColor=nvgRGBA((unsigned char)0,(unsigned char)0,(unsigned char)0,(unsigned char)255);  // black
NVGcolor MSQ_panelArpPartColor=nvgRGBA((unsigned char)0,(unsigned char)0,(unsigned char)255,(unsigned char)255);  // blue
NVGcolor MSQ_panelBassPartColor=nvgRGBA((unsigned char)0,(unsigned char)255,(unsigned char)0,(unsigned char)255);  // green

NVGcolor MSQ_panelHarmonyKeyboardColor=nvgRGBA((unsigned char)0xff,(unsigned char)0x80,(unsigned char)0x80,(unsigned char)255);  // redish
NVGcolor MSQ_panelMelodyKeyboardColor=nvgRGBA((unsigned char)0x80,(unsigned char)0x80,(unsigned char)0x80,(unsigned char)255);  // blackish
NVGcolor MSQ_panelArpKeyboardColor=nvgRGBA((unsigned char)0x80,(unsigned char)0x80,(unsigned char)255,(unsigned char)255);  // blueish
NVGcolor MSQ_panelBassKeyboardColor=nvgRGBA((unsigned char)0x80,(unsigned char)0xff,(unsigned char)0x80,(unsigned char)255);  // greenish

// Bravura unicode music font glyph codes
const std::string MSQ_noteWhole = u8"\ue1D2";
const std::string MSQ_noteHalfUp = u8"\ue1D3";
const std::string MSQ_noteHalfDown = u8"\ue1D4";
const std::string MSQ_noteQuarterUp = u8"\ue1d5";
const std::string MSQ_noteQuarterDown = u8"\ue1d6";
const std::string MSQ_noteEighthUp = u8"\ue1D7";
const std::string MSQ_noteEighthDown = u8"\ue1D8";
const std::string MSQ_noteSixteenthUp = u8"\ue1D9";
const std::string MSQ_noteSixteenthDown = u8"\ue1DA";
const std::string MSQ_noteThirtysecondthUp = u8"\ue1DB";
const std::string MSQ_noteThirtysecondthDown = u8"\ue1DC";
const std::string MSQ_staffFiveLines = u8"\ue014";
const std::string MSQ_gClef = u8"\ue050";
const std::string MSQ_fClef = u8"\ue062";
const std::string MSQ_ledgerLine = u8"\ue022";
const std::string MSQ_flat = u8"\ue260";
const std::string MSQ_sharp = u8"\ue262";
const std::string MSQ_timeSig0 = u8"\ue080";
const std::string MSQ_timeSig1 = u8"\ue081";
const std::string MSQ_timeSig2 = u8"\ue082";
const std::string MSQ_timeSig3 = u8"\ue083";
const std::string MSQ_timeSig4 = u8"\ue084";
const std::string MSQ_timeSig5 = u8"\ue085";
const std::string MSQ_timeSig6 = u8"\ue086";
const std::string MSQ_timeSig7 = u8"\ue087";
const std::string MSQ_timeSig8 = u8"\ue088";
const std::string MSQ_timeSig9 = u8"\ue089";
const std::string MSQ_staff1Line = u8"\ue010";  // short line

