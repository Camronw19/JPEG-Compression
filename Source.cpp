#include <iostream>

#include "ppm.h"
#include "YCbCr.h"

YCbCrImage rgb_to_ycbcr(const PPMImage& rgb_image);

int main()
{
	std::string image_path = "kodak06.ppm";
	PPMImage image(image_path);
	image.name = "Kodak06.ppm";


	if (image.has_value())
	{
		std::cout << "Image read successfully" << '\n';
		YCbCrImage ycbcr_image = rgb_to_ycbcr(image);
		ycbcr_image.output_luminance();
		ycbcr_image.output_ycbcr();
		ycbcr_image.output_blueChrominance();
		ycbcr_image.output_RedChrominance();

		ycbcr_image.chrominance_downsampling();
	

		ycbcr_image.output_ycbcr();
		ycbcr_image.output_blueChrominance();
		ycbcr_image.output_RedChrominance();
		
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
			double yr = ((65.738 * rgb_image.data[row][col].red) / 256);
			double yg = ((129.057 * rgb_image.data[row][col].green) / 256);
			double yb = ((25.064 * rgb_image.data[row][col].blue) / 256);
			ycbcr_image.data[row][col].y = 16 + yr + yg + yb;

			double cbr = ((37.945 * rgb_image.data[row][col].red) / 256);
			double cbg = ((74.494 * rgb_image.data[row][col].green) / 256);
			double cbb = ((112.439 * rgb_image.data[row][col].blue) / 256);
			ycbcr_image.data[row][col].cb = 128 - cbr - cbg + cbb;

			double crr = ((112.439 * rgb_image.data[row][col].red) / 256);
			double crg = ((94.154 * rgb_image.data[row][col].green) / 256);
			double crb = ((18.285 * rgb_image.data[row][col].blue) / 256);
			ycbcr_image.data[row][col].cr = 128 + crr - crg - crb;
		}
	}

	return ycbcr_image;
}