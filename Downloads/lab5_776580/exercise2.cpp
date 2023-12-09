#define cimg_use_jpeg
#include "CImg/CImg.h"

#include <iostream>

using namespace cimg_library;
int main(){
  CImg<unsigned char> img("image.jpg");  // Load image file "image.jpg" at object img

  std::cout << "Image width: " << img.width() << "Image height: " << img.height() << "Number of slices: "
      << img.depth() << "Number of channels: " << img.spectrum()
      << std::endl;  //dump some characteristics of the loaded image

  int i = 100;
  int j = 100;
  std::cout << std::hex << (int) img(i, j, 0, 0) << std::endl;  //print pixel value for channel 0 (red) 
  std::cout << std::hex << (int) img(i, j, 0, 1) << std::endl;  //print pixel value for channel 1 (green) 
  std::cout << std::hex << (int) img(i, j, 0, 2) << std::endl;  //print pixel value for channel 2 (blue) 
  
  //img.display("My first CImg code");             // Display the image in a display window


  // ! ############# TASK 2 #############
    CImg<unsigned char> img2("image.jpg");  // Load image file "image.jpg" at object img
    int height = img2.height();
    for(int i = 0; i < img2.width(); i++){
        img2(i,height/2, 0, 0) = 0;
        img2(i,height/2, 0, 1) = 0;
        img2(i,height/2, 0, 2) = 255;
    }  

    img2.display("Blue line");             // Display the image in a display window

  return 0;

}