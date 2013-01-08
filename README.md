Name: Jenis Modi

Date: Dec 15 2012

Website for more information: http://jenis23.blogspot.com/

Project Authors:
  
  <b>Jenis Modi (MS CS, WSU Vancouver)</b>
  
  <b>Dr. Wayne Cochran (WSU Vancouver)</b>

The project creates St helens chromadepth image as defined in Project document. It contains 1 "C" files, 1 vertex shader file and 1 fragment shader file. 

1. chromaTerrain.c : Generate chromadepth terrain image.
2. matrix.c: used for extra matrix related operation -- Didn't upload as it's created by Professor. Can help with creating similar file.

Additional feature: 

 Press 'z' or 'x' : Zoom in or out
 Press 'Esc': To Exit

The project is coded in C Language. I have a make file to execute my code. You can just use “make” command to compile the code and then ./chromaTerrain to run the code. The code is compiled with gnu99 option in the makefile. If this is not supported in user's machine, please change it to c99.

List of files:
***************

1. Makefile

2. chromaTerrain.c
3. matrix.c

4. vertex.vs
5. fragment.fs



I am using GL_TRIANGLES for surface normals. Please pass the image name in loadPGMFile method. You can use any image for this chromadepth. It is generic.
