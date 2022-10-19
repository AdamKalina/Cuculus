# Cuculus

Cuculus (latin for Cuckoo) is command line application for exporting .SIG (Brainlab) to .EDF based of [EDFlib](https://gitlab.com/Teuniz/EDFlib) by Teuniz and [sigtoedf](https://github.com/Frederik-D-Weber/sigtoedf) by Frederik-D-Weber.

I am mostly self-taught programmer, so my style is not particulary clean. Constructive criticism is welcomed!

I made some minor changes to edflib.c by Teuniz - mainly new function for converting Windows-1250 coding to ASCII.

TO DO - there are still more events to explore and export.

Cuckoo is part of complex environment around BrainLab .SIG files, because we are still actively recording with this old system (yes, even in 2022). There is also EEGLE Nest - recording database and EEGLE reader (work in progress).
