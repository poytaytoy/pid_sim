## How to run: 

---

If you happen to have Nix for whatever reason, run: 

```nix
nix develop --extra-experimental-features "nix-command flakes"
```
This will provide you with all the dependencies to run the command that follows. 

If not, make sure the CMakeLists.txt points to the correct directories and run:

```bash 
mkdir build 
cd build 
cmake .. 

make && ./pid_sim
```

This should get the program running. 

## Motivation 

I found some old render code I made a year back with OpenGL, and figured I wanted to build a simulation with it. Anyhow, one challenge I've had in my time with FRC was tuning the PID for my team's swerve drive, so decided it'd be cool to put it to use. 

To start, les quickly go over what Proportional Integral Derivative (PID) is: 

<img width="889" height="134" alt="image" src="https://github.com/user-attachments/assets/aa303a18-5415-407f-8b9e-7eeb84543c97" />

It looks pretty scary but it essentially is that if you have some error adjusting to K_p is a direct multiplier to it, K_i is a multiplier to the accumalation of all previous errors (or the integration of it), meanwhile K_d is a multiplier to the instant rate of change (or derivative to the error). Tuning these values is a closed-loop control method where eventually, continuous apppication of the PID will turn the error to 0 on its own. 

The simulation tests its effect on a swerve drive + a simulated camera view (I work on a lot of vision and stuff sooooo). The two PID instance are applied to the error in the translation to the mouse cursor and the rotation to the cursor. 

Ok, there's 3 values to it. What does each of them do? Les start with the P. P is the proportion and directly multiplies to the error. 











