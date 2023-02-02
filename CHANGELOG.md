## Meander Change Log <a id="meander-change-log"></a>
## Significant Version Changes (Changelog)

### V2.0.27 (Feb 2023)
- Slight improvement in external Harmony octal radix CV control.

### V2.0.26 (Jan 2023) 
- Bug-fix for Meander to Meander Mode and Root CV values for chaining instances.
- Bug-fix melody arp several problems fixed that krept in at some point over the past few months.  No users have reported these.  Existing patches with arp enabled may sound slightly different but should definitely sound better.

### V2.0.25 (Dec 2022)
- Redid module parameter "randomization" scheme.  Some parameters are now designated as not-randomizable.  This allows Meander context menu randomization to just randomize things that make sense to vary within a song, but leave other high level parameters unchanged.
- Made Arp count and notes-on parameter subject to "smart" randomization so that values are rationally based on the Melody notes-on note duration parameter.  
- Applied smart Arp parameter changes based rationally on Melody notes-on changes.
- Added a RAND input port and button to the upper left corner area of the panel.  Clicking the button or sending a momentary trigger into the input port calls the VCV Rack API module parameter randomization.  Only parameters that make sense to randomize are randomized.  You might want to send a slow clock signal to this port and say randomize for each measure or after every N measures.  Clicking on the RAND button during performance is designed to not cause any surprizing transitions.

### V2.0.24 (Oct/Nov 2022)
- Added panel harmony 4-voice octave chords option.  These are the triad with the tonic being raised an octave and added as 4th note.  Clicking button toggles between Nice 7ths, V 7ths, 4-voice octave chords or triad chords (no button lit).
- Added panel harmony toggle between "Tonic on ch1" and "Bass on ch1" of the 1v/oct chords output.  This is so the bass can always be reliably located on ch1 if desired, for extracting basslines, etc.
- Added panel Mode output for chaining to other Meander instance Mode input. The Mode input handling was modified to detect input from another Meander instance and interpret correctly rather than default CV handling.
- Modified panel Root input handling so Meander can chain Root to other Meander instances and interpret this correctly rather than default CV handling.
- Added circle of 5ths inner circle 1V/Deg output in addition to the existing 1V/Deg input.  This can feed to another Meander instance 1V/Deg input.  The default 1V/Deg output range is 1-7V (1V= Degree I, 2V=Degree II .... 7V=Degree VII) for connection to another Meander instance 1V/Deg circle degree input.
- Added an options menu "Harmonic Degree Output Range" toggle between 1-7V and 0-6V.  At the present time, only the Aaron Static DiatonicCV module can understand harmonic degree and requires a 0-6V range where (0V=Degree I, 1v= Degree II, .... 6v=Degree VII.
- A few panel cosmetic changes were made for readability.
- Relabeled the Melody part "Degree" input as "1V/DEG".  These scale degrees (such as 3rds. 5ths, etc.)

### V2.0.23 (Oct 2022)
- Panel lit buttonw were changed from 3D to flat shading based on suggestion by David Grande.
- 21 new harmonic progression presets were added, #60-#80 .  Many of these are targeted towards ambient music.  See the end of the manual for the updated preset list and descriptions.
- Addition of the new presets breaks the preset CV input control due to a different scaling required for more parameter choices.  I'm not sure anyone is using this as it doesn't make a lot of sense to CV select the preset.  I've done that via sequencing, but the results were not too functional.

### V2.0.22 (Aug 2022)
- Slight change to harmony circle degree keyboard playing and sequencing to retrigger a degree playing, even if the degree is the same as the previous degree.

### V2.0.21 (July/Aug 2022)
- Meander internal clock now has range of 3.75-960 BPM.  Settable via parameter knob or CV in BPM V/Oct format.
- Elapsed time is displayed on panel in harmony preset area in format of Bars.Beats and Minutes.Seconds. Reset or RUN toggle resets elapsed time.
- Arp patterns description texts were redone to be more intuitive.  + and - were replaced with UP and DN.  No functional change.
- Several harmonic progression presets were corrected since they previously did not work correctly.  There is a slight chance that existing patches might behave differently, if you have changed the preset number of steps param.  No changes should sound bad.

### V2.0.20 (June 2022)
- Added the ability to edit the "#4: custom" harmonic progression preset and have the edited data saved into the patch via JSON data at the Meander module instance level.  Note: Each patch can only have 1 user created custom progression, but multiple Meander instances in the same patch can each have their own user created custom progression.  Also note: Once the custom preset is edited, do not change the "Progression Presets" setting or the custom edit will be lost and if you return to the custom preset, it will have been reinitialized to a 16 bar "I" progression.  If you accidently do this, until you save the patch, you can reload it to restore the custom progression, assuming you have previously saved the patch with your custom progression.  Therefore, always sve a custom progression patch as soon as you have the progression as you want it.  You can always edit this custom progression later and save the patch to preserve the edit.
- The custom progression edit code was redone to behave much more intuitively and as WYSIWYG.
- As you edit the custom progression, the #4: custom progression steps description will be updated dynamically.
- As Meander plays a progression, the progression step circle of 5ths "degree" Roman numeral for the current step will be displayed inside the circle of 5ths inner circle, just above the mode key signature staves.  Likewise, as you edit the custom progression, the selected step degree Roman numeral will also be displayed in this inner circle.
-As you edit the custom progression, the step chord will be played momentatarily, assuming you have Meander wired up to sound generators, etc.
-To begin editing, switch to the "#4: Custom" progression preset and set the Harmony "Steps" knob from 1 to 16, as desired.  Then, click on Meander "RUN" to stop the auto playback engine.  Click on a "Set Step" step button and then click on a valid (colored) circle of 5ths annular ring step button to play that chord and set the step to that degree. This action also disables the "Harmony Chords Enable" setting  Once all edits are complete, click on "RUN" and "Harmony Chords Enable" to resume play.   If the custom progresssion is as you want it, save the patch, which will save the custom progression.  If the progression is not exactly as you want it, continue these editing steps until it is and then save it.  Remember, once edited, never turn the "Progression Presets" knob in this patch or the custom progression will be reininitialized. if you accidently do this, reload the patch immediately to reload the custom progression data.  But, the custom data will not be in the patch until you save it.

### V2.0.19 (May 2022)
- Font changes only.

### V2.0.18 (May 2022)
- Deleted one font from the repository and changed Meander to use a different font.
- Fixed problem with pentatonic scale output option.

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
 
