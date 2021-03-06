import("stdfaust.lib");
maxdelay = 1;
samplerate = ma.SR;
maxdelaySamples = samplerate * maxdelay;

scaleL = vslider("Left", 1, -1, 1, 2) : si.smooth(0.999);
scaleR = vslider("Right", 1, -1, 1, 2) : si.smooth(0.999);
phaseOffsetFine =  vslider("Phase Fine", 0, 0, 0.010, 0.00001) * samplerate : si.smooth(0.999);
phaseOffsetRough =  vslider("Phase Rough", 0, 0, maxdelay, 0.001) * samplerate : si.smooth(0.999);
phaseLROffset = vslider("Left/Right Offset", 0, -1, 1, 0.001) : si.smooth(0.999);

scaleIn(in) = (((in * 2) ^ 4) * 0.0625), 0 : max;
phaseOffset = (phaseOffsetFine + phaseOffsetRough, maxdelaySamples : min, 0 : max);
phaseLOffset = scaleIn((phaseLROffset * -1, 0) : max) * samplerate;
phaseROffset = scaleIn((phaseLROffset, 0) : max) * samplerate;
scale(l, r) = l * scaleL, r * scaleR;
totalOffsetL = phaseOffset + phaseLOffset;
totalOffsetR = phaseOffset + phaseROffset;
phase(l, r) =
(de.fdelay(totalOffsetL, maxdelaySamples * 2, l), l : select2(totalOffsetL < 0.3)),
(de.fdelay(totalOffsetR, maxdelaySamples * 2, r), r : select2(totalOffsetR < 0.3));
process = scale : phase;
