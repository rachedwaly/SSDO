MyRenderer
Base code for INF584 "Image Synthesis" Assignments
==================================================

Author: Tamy Boubekeur (tamy.boubekeur@telecom-paris.fr)

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

The resulting binary to use is always the onelocated in the MyRenderer directory, you can safely ignore whatever is generated in the Build directory. 
