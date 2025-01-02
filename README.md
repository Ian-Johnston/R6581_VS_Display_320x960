Advantest R6581 DMM VFD reverse engineered and now with ability to drive a 320x960 TFT LCD.

R6581 reverse engineering By Mickle T, he developed the orignal OLED conversion, EEVBlog thread here:
https://www.eevblog.com/forum/repair/advantest-r6581-vfd-replacement/

My own input here is to import the original project to Visual Studio 2022 / VisualGDB and then convert
to 320x960 TFT LCD.

The .HEX file is located in /VisualGDB/Debug

Uses VisualGDB available here:
https://visualgdb.com

NOTES:
1. Function prototypes in the headers are missing, I couldn't be bothered! VS2022 seems to compile just fine and produce working code without them.

Ian.
