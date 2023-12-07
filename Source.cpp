#include <iostream>

#include "ppm.h"
#include "YCbCr.h"
#include "jpeg.h"

int main()
{
	std::string image_path = "kodak06.ppm";
	PPMImage image(image_path);
	image.name = "Kodak06.ppm";


	if (image.has_value())
	{
		JPEGCompressor jpeg; 
		jpeg.compress_ppm(image); 

	}


	return 0;
}

