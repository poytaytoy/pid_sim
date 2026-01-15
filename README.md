## How to run: 

---

If you happen to have Nix for whatever reason, run: 

```nix
nix develop --extra-experimental-features "nix-command flakes"
```

If not, make sure the CMakeLists.txt contains points to the correct directories and run:


```bash 
mkdir build 
cd build 
cmake .. 

make && ./pid_sim
```

This should get the program running. 

## Motivation 








