# Meander
Meander plugin module for VCV Rack.  First release V1.0.1 1/13/2020

![Meander](./res/Meander.png)

Meander is fundamentally a musical "expert" system that has quite a few rules for what makes western music sound good and
applies those rules to "sequence" other modules.  Meander has no audio sound generation or modification capabilities, so even
though it is basically a complex application (which Meander is and has been over its 30+ year history), it is lightweight in
terms of the load it puts on the CPU and DSP.  Meander has its own internal clock, so no inputs are required in order to start 
making music.

Meander is limited to western music heptatonic (7) note scales, primarily so that the chord rules can be uniformly applied. Meander is founded on the 7 modes and 12 roots (~keys) for 84 combinations of mode and root.  The Circle of 5ths is the visualization device for seeing the mode and root scales.  The proper key signature notation is displayed inside of the circle of 5ths.

The Meander module panel is generated procedurally at runtime, rather an relying on an SVG file.  It has an SVG file but that only has the logo text.

Only one instance of Meander can be loaded from the VCV Rack plugin manager.  If a second or other instance is loaded, the additional instances are disabled and the user is notified of that via a warning on the added panel image.  This is due to the complex issues of porting a complex stand alone Windows application to Rack, and the complications of having extensive global memory data and code.

