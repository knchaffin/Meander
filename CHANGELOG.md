## Meander Change Log <a id="meander-change-log"></a>
## Significant Version Changes (Changelog)

### V2.0.18 (May 2022)
- Changed a font based on possible license issue.

### V2.0.17 (April/May 2022)
- Made sure variables are initialized.
- Switched to the Bravura.otf music notation font.
- Redid the "score" display area with the following changes:
  . Switched to Bravura music font.
  . Expanded the score display area on the low end.
  . All notes now draw as played, e.g., whole, half, quarter, eighth, sixteenth and thirtysecond notes.
  . Common practice rules were implemented for whether stemmed notes have stems up or stems down.
  . Common practice rules were implemented to horizontally offset notes slightly when stems and notes may collide.
  . Ledger lines were added for when played notes are above, between or below the treble and bass staffs.
  . Colors were tweaked based on user requests for visibility.
  
 ### V2.0.16 (April 2022)
- Cosmetic change only for light panel theme PNG file to fix slight misalignment with panel ports and controls for in browswer and library.

### V2.0.15 (March/April 2022)
- This change log file was added and the change log moved from the manual to here.
- Almost all global variables were moved to the module instance scope. Most, if not all globals are now read only.
- You can now add multiple instances of Meander to your patch.  Meander is no longer a "singleton".  Most changes are "under the hood" in Meander and while extensive, they do not change how Meander works or looks.  Existing V2 patches should continue to work fine.
- If you change the panel color theme or in one Meander instance in a patch, the theme is applied to all other instances of Meander in the patch and in the browser.  Changing the panel theme contrast changes all instances of Meander in the patch but does not show up in the browser view.
- The browser panel view methodology was changed.  For Meander now, the browser panel image is loaded from either a dark theme or light theme static PNG image file.  This results in a better user experience and more simple code as Meander no longer has to procedurally draw the panel image in the browser.
- If the meandering notes happen to meander out of the MIDI specification range, the note is clamped to either the low octave or high octave root note, depending on which way the note went out of bounds.  Several additional range and bound checks were added to prevent possible invalid notes or note names.
- For clarity, middle-C is denoted on the musical score display as C4.
- A dark theme panel fBm text display problem was fixed.
- "Root" output port values is always correctly initialized now.
- The manual now has a bare-bones example/demo patch image.

### V2.0.14 (February 2022)
- Fixed a segment fault that could occur if a cable was connected to the STEP input port inside the circle of 5ths.  Github issue #13.
- Several tweaks were made to the dark and light panel themes to make colors more readable over all contrasts and themes. Github issue #12.
- A couple of panel parameter display boxes were moved down slightly to avoid tooltips from obscuring the display, if tooltips are enabled. Github issue #12.
- Made a change to make the panel capture image in the web library correctly display all parameter displays.

### V2.0.13 (February 2022)
- Added light and dark panels with adjustable contrast (through Meander right-click options menu).
- Added "port labels" to all inports and outports. Right click on port to see port label info, including: port description, input or output voltage ranges and how CV maps to parameter range.
- Added 88 key piano keyboard on which all parts are shown as they are played, color coded the panel parts. This can be enabled/disabled.
- Musical staves display of playing notes can be enabled or disabled.
- The "Root" note is output in 1V/oct format for use by drones or for external quantizers control.
- "Poly Ext. Scale" output can now be in any of 4 types, chosen from the options menu.  The default is the "Poly External Scale" format implemented in Meander in 
2020 as a 12 poly channel chromatic scale format.  Currently, the Grande Quant module is fully compatible with this format.  Run this to Grande Quant, but do not run the Meander root out to Grande and the Quant quantizer will use the current Meander mode and scale for quantizing outside of Meander but coordinated with Meander.  The second output format is "Heptatonic Diatonic STD-12ch" format which is a 7 channel poly out with the scale notes on their own channels.  This is usefult for running into external sequential switches, allowing sequenced scale runs or riffs to be played by exernal modules but in tune with the Meander mode and root scale.  There are also 2 pentatonic scale output modes, one as 5 channel pentatonic and the other as 12ch pentatonic chromatic scale.  The pentatonic modes are modified subsets of the mode and root scale in Meander.  For best results, have Meander set to either the Ionian major or the Aeolian natural minor modes.  These pentatonic scale outputs are experimental and hopefully will find some uses.  The basic thing going on is that pentatonic melodic scales can play over either a major or minor scale and still sound good, ideally.  Musicians do this often.  A use case would be to run the pentatonic scale output into a sequential switch, allowing sequencing in the melodic pentatonic scale. 
- Meander can now act as a polyphonic quantizer for external modules, always quantizing to the current Meander mode and root scale.  In this use case, external modules can send up to 16 channels of notes to be quantized.  Meander will output the quantized notes to the same polyphonic channels, which can then be used to provide 1v/oct note data to other external sound source modules.
- Several cosmetic changes were made.  For all new features, care was taken to not change the Meander behavior for existing V2 patches.


### V2.0.12
- Initial commit for Rack v2

### V1.0.11
- A clock outport was added, adjacent to the clock inport.  If no external clock is connected, the Meander generated clock is output.  If an external clock is connected, the external clock is output (as pass-through).
- For octal radix degree.octave voltage control of the harmony and melody, you can now send a <1V or >=8V (typically from a sequencer) and Meander will ignore that step.  This allows for rhythmic chord and melody ostinatos.  A value of 0.0 is recommended as skip step designator.
- All note duration gate signals were tested for all use cases and a few tweaks were made.
- All textual notes played displays were checked and some changes made to correctly display note even if octal radix degree control of harmony or melody is being used.
- Arp was fixed to track melody when melody is controlled by octal radix degree voltages.

### V1.0.10
- A bug was corrected so that note lengths are correct for all settings.  Legato and Staccato are now correctly handled. Staccato note lengths via the gate outputs are now 50% of the note length designation (1/4, 1/8,, etc.).  Legato (defualt) notes are 95% of the note length designation.
- 8 new harmonic progressions were added for a new total of 59.  A few slight tweaks were made to existing progressions to fit tradition better.
- An appendix was added at the end of this manual that lists the harmonic progression description and Roman number step degrees.  I've added some annotations about the progressions if you are interested in knowing more about harmonic theory.  I'm no expert, but I have worked with Meander for 32 years or so now.  I keep learning though.
- The panel clock input text was changed to "EXT 8x BPM" to remind users that the clock should be an 8X clock.
- Expanded the Harmony Presets text displays to allow maximum length in the space provided.

### V1.0.9
- Corrected issue with melody when arp is enabled.  It now sets the molody note duration to the arp notes duration if arp is enabled.  This makes sure the melody and arp gate output catches all notes.
- The harmony chord output port now always puts the chord root or tonic note in channel 0.  This enables external modules to extract the root bass note from the chord.  Works for triads and tetrad 7ths.
- The 12 bar blues progression (#13) was corrected to use the most standard or traditional form.
- Standardized all progression degree steps to upper case Roman numerals with a space and dash in between steps.
- Correct the #7 "strong" progression to actually be a strong progressions with each step approaching the tonic by 4ths.
- Corrected the #22 "random coming home" progression to always return to the tonic via steps of a 4th.
- When the BPM CV input is connected, Meander now sets the Meander BPM to track the external clock (ex. from CLOCKED) even if Meander is receiving an external clock.


### V1.0.8
- Support added for "Poly External Scale" output. https://aria.dog/modules/poly-external-scale/ Currently, only Aria Salvatrice's modules can interpret this data.  I specfically tested this with Aria's QQQQ quad quantizer module.  Basically, Meander sends its mode and root scale info out the "Poly Ext. Scale" out port.  QQQQ can receive this data and set up a quantizer that matches Meander's current scale.  Thus, QQQQ can be used in scale sequencers and arpeggiators you might build outside of Meander in the current VCV Rack patch.  QQQQ also adds to Meander by displaying the Meander current mode and root (key) scale notes on a piano keyboard display.  This graphical representation matches the Meander Mode parameter scale that is displayed on the panel below the mode name.

### V1.0.7
- Corrected several problems related to module browser and Libray panel appearance.

### V1.0.6
- Corrected a bug that caused the Meander module panel image to draw incorrectly in the plugin/module browser on some systems.

### V1.0.5
- Added CV Degree and Gate inputs to melody section.  Allows playing of the melody engine via octal radix degree.octave values from sequencers or standard note values from a MIDI keyboard or modules such as TWELVE-KEY.
- Changed Harmony circle Degree and Gate inputs format to accept same control as the melody.  Allows playing of the harmony  engine via octal radix degree.octave values from sequencers or standard note values from a MIDI keyboard or modules such as TWELVE-KEY.
- Harmony STEP input can now properly control stochastic progressions such as those with "Markov", "random" or "rand" in their names.
- Harmony circle "STEP" can now be advanced via the Meander "1ms Clocked Trigger Pulses".  Warning, this introduces a 1/32nd note delay in the harmonic progression.  In most cases this is not noticeable, but beware.  
- All play modes now handle 7th chords if selected.
- All target octaves have been unified to play in the correct octave and display correctly on the panel.

### V1.0.4
- All button parameter internal variable states are now saved and restored in save and autosave and load via json.
- Run and Reset logic was improved to behave predictably.
- "All 7ths" was changed to "~Nice 7ths".  Attempts to only play "nice" sounding 7ths, which are V7, ii7, viidim7 and IVM7.  These sound a lot more harmonious and less dissonant.  A better choice is V7ths which almost always sound good and are extensively used in music, particlularly in jazz and the blues.

### V1.0.3
- Added a STEP button inside of the circle of 5ths to allow the harmony progression to be manually advanced, or via CV.
- Panel cleanup.
 
