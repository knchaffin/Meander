# Meander
Meander plugin module for VCV Rack.  

![Meander](./res/Meander.png)

## General: 
Meander is fundamentally a musical "expert" system that has quite a few rules for what makes western music sound good and
applies those rules to "sequence" other modules.  Meander has no audio sound generation or modification capabilities, so even
though it is basically a complex application (which Meander is and has been over its 30+ year history), it is lightweight in
terms of the load it puts on the CPU and DSP.  Meander has its own internal clock, so no inputs are required in order to start 
making music.

## Caveat:
Anything I say here about music theory and practice as well as how I have implemented that theory and practice in Meander is my own understanding, which is surely not absolutely correct and musicians might choose to debate with me on that.  There are also a lot of exceptions to the "rules" in music.  If fact, it is these exceptions that give composers and musicians their own distinctive sound.  In the end, it is all about what sounds good or entertaining or evokes certain feelings.  From my perspective, there are no absolute rules in music.  So, take what I say with a grain of salt. 

Meander is limited to western music heptatonic (7) note scales, primarily so that the chord rules can be uniformly applied. Meander is founded on the 7 modes and 12 roots (~keys) for 84 combinations of mode and root.  The Circle of 5ths is the visualization device for seeing the mode and root scales.  The proper key signature notation is displayed inside of the circle of 5ths.

The Meander module panel is generated procedurally at runtime, rather an relying on an SVG file.  It has an SVG file but that only has the logo text.

Only one instance of Meander can be loaded from the VCV Rack plugin browser.  If a second or other instance is loaded, the additional instances are disabled and the user is notified of that via a warning on the added panel image.  This is due to the complex issues of porting a complex stand alone Windows application to Rack, and the complications of having extensive global memory data and code.

All Meander panel control parameters can be controlled by an external 0-10V CV via the input jack just to the left of each parameter knob or button.  No external control is necessary to use Meander.  The control is there to allow you to do (almost) anything you can dream up.

The mode and root are selected by the control knobs on the far left side of the panel.  As you rotate these knobs, the circle of 5ths will rotate to show the chords that should be played for this mode and root. The root will always be at the I degree position and is also designated by a red light just inside of the inner circle. Only the colored segments should be played.  Each colored segment is marked with the chord "degree", which are the Roman numberals I-VII.  The degrees are color coded as to whether the chord will be played as a major, minor or diminished chord.  Major chord degrees are designated with uppercase Roman numerals whereas minor chord degrees are designated with lower case Roman numerals. A diminished chord degree is designated with a lowercase Roman numeral and a superscript "degree" symbol.  All of this is done automatically by Meander, following music theory common practice. 

Meander has 3 main music "parts" sub-panels: Harmony, Melody and Bass. The harmony drives the melody and the bass parts.  The melody drives the arpeggiator (arp) sub-part.  The arepeggiator is a melodic arpeggiator rather than a harmonic arpeggiator.  Each of the parts can be enabled or disabled via the "Enable" buttons at the top of the subpanels.  If the part is not enabled, Meander still composes that part (so that the harmony part is always available to the bass generator) but does not play that part.

Each part has three output ports at the bottom of the panel.  Those are the 1V/octave, the gate and the volume outputs.  The 1v/Oct output is typically  connected to a VCO V/OCT input, whereas the Gate output is typcially connected to an ADSR gate input, which is connected to a VCA. Meander follows the Gate voltage standard, but not the common practice.  The gate off state is 0V.  The gate on state is >=2.1V .  The gate on voltage also carries the volume for the note in the range of 2.1V-10V.  Thus, the gate output can also be used to control volume by modulating a VCA or controlling an ADSR Sustain parameter, or by channel level on a mixer, etc.  The gate could also be used as a CV for anything else you choose to use it for.  The volume output is totally optional for anything you might want to use it for.  It passes the part Volume setting out, but it may also be modulated by such things as bass part note accents.  It can safely be ignored until you become more experienced with Meander.

A big aspect of Meander is that the harmony, melody and bass parts meander according to some fairly complex fractal based math.  You do not have to worry about that unless you want to.  Meander uses "fractal Brownian motion" for producing meandering patterns.  The specic type of fBm variation is called 1/f noise and is made up of 1D quintic interpoldated Perlin noise, where the 1-D is time.

Each of the three parts is discussed following in more detail:

## Harmony

Harmony is made up of chords made up of notes from the mode and root that is selected, as well as determined by the current circle of 5ths degree position chord type.  A harmonic progression is the movement on the circle in steps over time.  At the bottom of the Harmony sub-panel is the "Presets" control which allows you to select between 50+ ready-made harmonic progressions.  Each progression is made up of from 1 to 16 steps, designated by the Roman numberal degrees, corresponding to to the degree positions on the circle for the current mode and root.  I.E., the same progression can be played in any of the 84 mode and root "scales".  As the pogression plays, you can watch the circle and see which chords are playing for each step.

The music theory behind the circle-of-fifths is beyond this manual, but the basic theory is that chords next to each other on the colored part of the circle always share one note between them.  Each degree going CW around the circle represents a 5th interval, thus the name circle of fifths. Going CCW, the interval is a 4th.  The shared note between two chords going CW is a 5th above the tonic of the first chord.  Basically, the further away from each other two chords are on the circle, the more dissonance there will be.  A common progression is to start out on the I position and then jump several position CW on the circle and then walk back CCW on the circle back to the I position.  Each step CCW gives a feeling of resolution of tension back to the I position.  Theee are a myriad ways to form the progression, but there are a few progressions that almost all popular western music is composed of.  Meander has 50+ such presets.  One of the most common progressions in popular music is I-V-vi-IV , which is #26 in the presets.  That same progression can be played in any of the 84 mode and root combinations, but may have s distinctly different feel in a different mode.  Not all music is based on chord progressions, but a lot is, particularly popular music.

The fBm fractal noise results in harmony meandering, by allowing chords to wander over a range from a fraction of an octave to several octaves.  Rather than meandering in octave jumps, the chords meander through chord inversions across one or more octaves.  The playing chords shown inside the circle are in inversion notation if inverted.  If you see a chord such as G/D, that means a Gmaj chord where the G root is played above the D note in the major triad.  These inversions also allow the chord progression around the circle of 5ths to sound less melodic.  These are also the reasons that musicians use chord inversions.

The "Chords on 1/ " control determine when the chords play.  1/1=whole note, 1/2=half note, 1/4=quarter note, and so forth.

## Melody

Melody is driven by the harmony part chords.  The melody notes can either be chordal where they are members of the current playing or last chord played, or they can be scaler where they are members of the current scale (mode and root).  Meander does not use "accidentals or naturals" notes that are not members of the current scale.

The "Note Length 1/" control determines whether the melody plays on whole, half, quarter, eighths, sixteenths, or 32nd notes.

The Arp or Arpeggiator settings are part of the melody.  In an arpeggiation, the melody note is the first note in the arpeggio and the other notes are either chordal (current chord members) or scaler (current scale members) and follow the "Pattern" control of notes moving up or down or up and then down by 0, 1 or 2 notes per step.  The Arp "Count" is the number of arpeggio notes which are separated by 1/n notes.  For example, if the melody note is every 1/4 note, you can fit up to 3 arp notes of 1/16 length between every melody note.  The Arp "Decay" control causes the arp notes volume to decay each note.  It is up to you to make use of the arp note volume by either the melody volume, or the volume over gate.

