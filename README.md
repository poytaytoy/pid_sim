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

I found some old OpenGL render code from last year and used it to build a quick swerve-drive PID tuning sim. PID is a closed-loop controller that turns error into correction using three terms: P (current error), I (accumulated error), and D (rate of change). In the sim, I run two PID controllers: one drives translation toward the mouse cursor, and the other rotates the robot to face it, with a simple camera view for visualization.

## Preview

Here is a preview of it: 

<p align="center">
  <img src="https://github.com/user-attachments/assets/368a187a-7f4c-49fa-831b-568dfffedc70"
       alt="PID sim preview"
       width="900" />
</p>

How it essentially works is that you can see the effects of the robot's drive from tuning the PID of either the translation or rotation and also the camera view to see how the drive might be seeing the cursor and stuff. This is pretty similar to how the PID works in swerve drives:

Oh, and just one small clarification bit, but on the camera, you'd see the coloured walls and stuff, they're essentially the world barriers that are kinda out of view from the map, but they are as follows. 

<p align="center">
  <img src="https://github.com/user-attachments/assets/af465a15-aa16-4474-b511-44bf6d554a0b"
       alt="World barriers"
       width="300" />
</p>

The blue are the walls along the y-axis and the red are the walls along the x-axis. They're there for some visual clarity. 

## Video 

Anyways, here's a video of me playing around with it: 

<p align="center">
  <a href="https://youtu.be/Q9LbukTRu00">
    <img src="https://img.youtube.com/vi/Q9LbukTRu00/maxresdefault.jpg"
         alt="PID Sim Demo"
         width="900" />
  </a>
</p>

From the video, you can see that the P does most of the work. Things only behave normally when its set to some value. Meanwhile, I is complete chaos but that's expected since it's essentially an indefinite accumalation of the errors. Derivative acts as it should where only a quick change in the mouse point's location forces movement. But yeah, that's about it. 








