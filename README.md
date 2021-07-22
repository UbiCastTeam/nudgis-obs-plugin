# Nudgis OBS Plugin

This Plugin is design to stream on Nudgis using Obs Studio. 

## Installation

### Building 

In the source folder of the plugin (nudgis-obs-plugin) :

```bash
mkdir build
cd build
cmake ..
```

### Compiling and executing
In the build folder : 

```bash
make
sudo cp nudgis-obs-plugin.so /usr/lib/x86_64-linux-gnu/obs-plugins/
obs
```
