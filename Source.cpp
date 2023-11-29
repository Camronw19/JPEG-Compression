#include <iostream>

#include "ppm.h"
#include "YCbCr.h"

YCbCrImage rgb_to_ycbcr(const PPMImage& rgb_image);

int main()
{
	std::string image_path = "kodak06.ppm"; 
	PPMImage image(image_path); 
	image.name = "TEST"; 

	if (image.has_value())
	{
		std::cout << "Image read successfully"; 
	}



	return 0; 
}

YCbCrImage rgb_to_ycbcr(const PPMImage& rgb_image)
{

	YCbCrImage ycbcr_image(rgb_image.height, rgb_image.width, rgb_image.max_val, "YCbCr_" + rgb_image.name);

	for (int row = 0; row < rgb_image.height; row++)
	{
		for (int col = 0; col < rgb_image.width; col++)
		{
			ycbcr_image.data[row][col].y = 0;
			ycbcr_image.data[row][col].cb = 0;
			ycbcr_image.data[row][col].cr = 0;
		}
	}
}
