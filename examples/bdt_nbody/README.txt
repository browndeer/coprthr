
This is a simple N-Body demo used to simulate the dynamics of stars in
a galaxy subject to gravitational force.  The program includes an OpenGL
display of the particles.

To build, type:

make

To run, type:

./bdt_nbody.x --cl-device gpu --nparticle 16384 --nthread 64 --nburst 2



