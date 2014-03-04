ilansh 302514401

Implementation:
---
We've used ex0 model to generate a circle, but in each draw() function call we passed the position argument to the vertex shader.

The Ball struct holds all the data relevant to each ball, as well as the rules how to update it when it moves and collides with other balls and walls.

When two balls collide, each ball shrinks its size to the average point of where their both radii won't collide.

We implemented the bonus as well, by adding another light source.
The shader was implemented by calculating the direction the light should be projected on the ball from a fixed source, and then placing the light center a half radius away from the center of the ball.
The intensity of the light was calculated using the closer light source.

Files sent:
---
Readme.txt - this file

shaders/SimpleShader.frag
shaders/SimpleShader.vert

ShaderIO.cpp
ShaderIO.h
Model.cpp
ex1.h
ShaderIO.h
Model.h
Makefile
ex1.cpp


website used:
---
http://open.gl/