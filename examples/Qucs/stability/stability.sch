<Qucs Schematic 0.0.9>
<Properties>
  <View=0,0,590,573,1,0,60>
  <Grid=10,10,1>
  <DataSet=stability.dat>
  <DataDisplay=stability.dpl>
  <OpenDisplay=1>
</Properties>
<Symbol>
</Symbol>
<Components>
  <L L1 1 290 290 10 -26 0 1 "95.475 pH" 1>
  <GND * 1 290 340 0 0 0 0>
  <Pac P2 1 360 120 18 -26 0 1 "2" 1 "50 Ohm" 1 "0 dBm" 0 "1 GHz" 0 "26.85" 0>
  <GND * 1 360 150 0 0 0 0>
  <GND * 1 110 250 0 0 1 2>
  <Pac P1 1 110 220 17 -25 0 1 "1" 1 "50 Ohm" 1 "0 dBm" 0 "1 GHz" 0 "26.85" 0>
  <.SP SP1 1 430 290 0 53 0 0 "lin" 1 "1 GHz" 1 "20 GHz" 1 "191" 1 "yes" 0 "1" 0 "2" 0 "no" 0 "no" 0>
  <C C1 1 240 90 -26 17 0 0 "2 pF" 1>
  <R R1 1 190 140 -116 -20 1 1 "992.19 Ohm" 1 "16.85" 0 "0.0" 0 "0.0" 0 "26.85" 0 "european" 0>
  <Sub FET1 1 270 190 40 24 0 0 "LowNoiseFET.sch" 1>
  <Eqn Eqn1 1 100 330 -23 14 0 0 "NFmin=10*log10(Fmin)" 1 "Gain=max(dB_S21,12e9-BW/2:12e9+BW/2)" 1 "Stable=min(StabFact)" 1 "NoiseFigure=min(NFmin,12e9-BW/2:12e9+BW/2)" 1 "StabFact=Rollet(S)" 1 "dB_S21=dB(S[2,1])" 1 "BW=100e6" 1 "yes" 0>
</Components>
<Wires>
  <190 170 190 190 "" 0 0 0 "">
  <190 190 240 190 "" 0 0 0 "">
  <190 90 210 90 "" 0 0 0 "">
  <190 90 190 110 "" 0 0 0 "">
  <290 90 290 140 "" 0 0 0 "">
  <270 90 290 90 "" 0 0 0 "">
  <290 240 290 260 "" 0 0 0 "">
  <290 320 290 340 "" 0 0 0 "">
  <290 90 360 90 "" 0 0 0 "">
  <110 190 190 190 "" 0 0 0 "">
</Wires>
<Diagrams>
</Diagrams>
<Paintings>
  <Text 70 30 16 #000000 0 "Stabilization of a FET">
  <Text 60 490 10 #000000 0 "In order to stabilize the FET in the subcircuit R1, C1 and L1 are used.\n  Goals should be:\n    - NFmin as low as possible\n    - StabFact greater than 1 in the whole frequency range\n    - dB_S21 (gain) as high as possible">
</Paintings>
