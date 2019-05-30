*Chebyshev Band Pass Filter

*** *** FILTER CIRCUIT *** ***
C1 1 0 #C1#  $ #CSMD_50p80p#
L1 1 0 #L1#  $ #LBOND_350p450p#

L2 1 2 #L2#  $ #LBOND_60n100n#
C2 2 3 #C2#  $ #CSMD_300f340f#

C3 3 0 #C3#  $ #CSMD_50p80p#
L3 3 0 #L3#  $ #LBOND_350p450p#


*** *** PORT EMULATION *** ***
*** http://eesof.tm.agilent.com/docs/iccap2002/MDLGBOOK/5SIMULATIONS/5Spice2Spar.pdf
*** S11=2*V(1)/Vin-1   S21=2*V(3)/Vin
Vin 10 0 dc 1 ac 1
R1  10 1 50
R3   3 0 50

*** *** ANALYSIS *** ***
.AC DEC 1000 800e6 1200E6
.control
set noaskquit
run
let S11db = db(2*v(1)/V(10)-1)
let S21db = db(2*v(3)/V(10))
.endc
.END
