
Introduction
============

Hi there!

This is a mini raytracer that renders a bunch of spheres!
Nothing very fancy here.

We've provided the basic necessary structs and functions to implement 


Test objective
==============

The objective of this sample is to implement a raytracing acceleration structure (namely test_accelerator, which is to be located in the test_accelerator.h and test_accelerator.cpp)

test_accelerator inherits from accelerator, and you must implement the init and trace methods. A trivial implementation is provided as trivial_accelerator (in accelerator.h)

Your implementation should be fairly simple, easy to read, and should behave predictably relative to complexity.

You should not over-optimize, nor over-complexify.

For reference, our implementations took us roughly 90 minutes to write, debug and test.


Compiling
=========

This sample is standalone (as it does not depend on external libraries) and should compile easily with any c++11 compiler.

## Linux ######
A makefile is provided. A decently modern g++ will do (gcc 4.8 and above.)
'make' will compile in release mode (-O3 -DNDEBUG.)
Compile in debug with 'make VERSION=debug'
The release (resp. debug) executable is release/raytracer (resp. debug/raytracer)

## Windows ######
A VS2015 solution is provided.
Build the solution using the build menu.


Running
=======

The executable takes a few options:

  * -w _width_ to specify the width of the output image (default is 1024)
  * -h _height_ to specifiy the height of the output image (default is 1024)
  * -o _file_ to specify the output file name (default is out.ppm)
  * -n _nspheres_ to specify the number of spheres to render (default is 20000)
  * -s _scale_ to specify the scaling factor of the spheres (default is 0.5)
  * -nt _nthreads_ to specify the number of rendering threads (default is 1)

