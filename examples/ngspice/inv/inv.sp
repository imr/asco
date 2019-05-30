*Digital inverter

.PARAM V_SUPPLY = '#V_SUPPLY#'
.PARAM INP_FREQ = '#INP_FREQ#'
.PARAM INP_PERIOD = '1/INP_FREQ'
.PARAM NO_PERIODS = '4'
.PARAM TMEAS_START = '(NO_PERIODS-1)*INP_PERIOD'
.PARAM TMEAS_STOP = '(NO_PERIODS)*INP_PERIOD'
.PARAM TMEAS_1 = 'TMEAS_STOP -3*INP_PERIOD/4'
.PARAM TMEAS_2 = 'TMEAS_STOP -1*INP_PERIOD/4'

*** *** SUPPLY VOLTAGES *** ***
VDD VDD 0 {V_SUPPLY}
VSS VSS 0 0

*** *** INPUT SIGNAL *** ***
VSIG IN VSS PULSE {V_SUPPLY} 0 'INP_PERIOD/2' 'INP_PERIOD/1000'
+               'INP_PERIOD/1000' 'INP_PERIOD/2' 'INP_PERIOD'

*** *** CIRCUIT *** ***
MP OUT IN VDD VDD PMOS W='#WP#'   L=#LMIN# pd='2*#WP#+2*#LMIN#' ps='2*#WP#+2*#LMIN#'
MN OUT IN VSS VSS NMOS W='#WP#/3' L=#LMIN# pd='2*#WP#+2*#LMIN#' ps='2*#WP#+2*#LMIN#'

CL OUT VSS 10p

*** *** ANALYSIS *** ***
.TRAN 'INP_PERIOD/1000' 'NO_PERIODS*INP_PERIOD'
*
*.PROBE TRAN V(IN)
*.PROBE TRAN V(OUT)
.OPTION POST PROBE ACCURATE
.include p.typ
.include n.typ
.END
