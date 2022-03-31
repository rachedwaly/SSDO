MyRenderer
SSDO feature
Described in the following paper: Approximating Dynamic Global Illumination in Image Space

==================================================
link to paper : https://people.mpi-inf.mpg.de/~ritschel/Papers/SSDO.pdf

Author of base code: Tamy Boubekeur (tamy.boubekeur@telecom-paris.fr)
Author of the features: Mohamed Rached Waly 

### Building

This is a standard CMake project. Building it consists in running:

```
cd <path-to-MyRenderer-directory>
mkdir Build
cd Build
cmake ..
cd ..
cmake --build Build
```

The resuling MyRenderer executable is automatically copied to the root MyRenderer directory, so that resources (shaders, meshes) can be loaded easily. By default, the program is compile in Debug mode. For a high performance Release binary, just us:

```
cmake --build Build --config Release
```

### Running

To run the program
```
cd <path-to-MyRenderer-directory>
./MyRenderer [[file.off] [material-directory-path]]
```
Note that a collection of example meshes are provided in the Resources/Models directory and a collection of materials are provided in the Resources/Materials directory

When starting to edit the source code, rerun 

```
cmake --build Build --config Release
```
each time you want to recompile MyRenderer

The resulting binary to use is always the onelocated in the MyRenderer directory, you can safely ignore whatever is generated in the Build director

You can run the executable "MyRenderer" directly    
### Commands for the executable:

    Mouse command
	Left button: rotate came
	Middle button: zo
	Right button: pan cametKeyboard command
	ESC: quit the progr
	H: print this he
	F: decrease field of vi
	G: increase field of vi
    S: toggle screen space feature can be SSDO or SS
	F2: activate SSAO mode in the sce
	F1: activate standard Deffered mode in the sce
	F3: activate SSDO mode in the sce
	F4: show only the SSAO texture on the red channe
	B/N decrease/increase bias in blocker test in SSDO/SSAO mode
	J/K decrease/increase radius in SSDO/SSAO mode
	Y/U decrease/increase indirect Light bounce in SSDO mode


    
