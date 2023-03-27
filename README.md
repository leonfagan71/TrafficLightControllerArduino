# TrafficLightControllerArduino

## Controller for traffic lights with msec timing, phase changes in auto/manual modes with triggers etc.

# Bugs:
- a bucket load
- Don't set phase 1 in manual mode when the button pin isn't set to ground...


# Notes
```
Please be aware that this hasn't been extensively tested, if you use it and it breaks something. It's your fault.

This can control up to four phases, timings can  be set via serial. I have a program that will control this.

```
When building this, I wrote the following notes:
```
FOUR LOOP PHASES=12 relays
	LOOP 0
		state=MANUAL/AUTO
		green_timing=5000
		amber_timing1=7500
		amber_timing2=2500
		red_timing=7500
		RELAY_1=ON
		RELAY_2=OFF
		RELAY_3=OFF
		phase="RED"

Sending from Computer to arduino
L0_AUTO_ON=Auto enable
L0_AUTO_OFF=Auto Disable
L0_TRIG = If not in auto, trigger change
L0_PA500 = Set Trigger time for amber 1 phase to 500ms
L0_PF2500 = Set trigger time for Amber 2 phase to 2500ms
L0_PG2500 = Set trigger time for Green phase to 2500ms
L0_PR30000 = Set trigger time for Red phase to 30 seconds
L0_GA = Returns "L0_GA=2500" Gets trigger time for amber 1 phase in ms
L0_GF = Returns "L0_GF=2500" Gets trigger time for amber 2 phase in ms
L0_GR = Returns "L0_GR=2500" Gets trigger time for red phase in ms
L0_GG = Returns "L0_GG=2500" Gets trigger time for green phase in ms
L0_GS = Returns "LO_GS=Auto/Manual" Gets state of phasing, if auto or manual


L0_TRIGR = Trigger in manual mode as RED phase.
L0_TRIGA = Trigger in manual mode as Amber2 phase, if coming from Red maybe go to amber1 phase
L0_TRIGG = Trigger in manual mode as GREEN phase.


L0_AUTO_OFF
L0_AUTO_ON


L0_PA007500//Amber1 phase 7500ms
L0_PR005000//Red phase 5000ms
L0_PG005000//green phase 5000ms
L0_PF002500//Amber2 phase time 2500ms

L1_PA7500//Amber1 phase 7500ms
L1_PR5000//Red phase 5000ms
L1_PG5000//green phase 5000ms
L1_PF2500//Amber2 phase time 2500ms
```
