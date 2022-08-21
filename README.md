# Welcome to display-zero

**display-zero** is a ST7735S display driver with an ultimate goal — reach 0% CPU load. It is achieved by utilizing low-level features of bcm2835 chip, famous by it's usage in Raspberry Pi boards.

### Build
```
mkdir build
cd build
cmake ..
cmake --build .
```

### Usage
```
./display-zero -p kona300x500.gif -o kona300x500 #prepare raw file
./display-zero kona300x500                       #display raw file
```
