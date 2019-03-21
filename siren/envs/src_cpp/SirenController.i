%module my_module

%include "opencv-swig/lib/opencv.i"
%cv_instantiate_all_defaults

%{
#include "RGBImage.hpp"
#include "GTImage.hpp"
#include "BoardState.hpp"
%}

%include "RGBImage.hpp"
#include "GTImage.hpp"
#include "BoardState.hpp"
