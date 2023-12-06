#pragma once
#include <string>
#include <array>

struct downsampledPixel
{
	unsigned char cr;
	unsigned char cb;
};

struct YCbCrPixel
{
	unsigned char y;
	unsigned char cr;
	unsigned char cb;

	YCbCrPixel();
	YCbCrPixel(unsigned char, unsigned char, unsigned char);
};

class YCbCrImage
{
public:
	int height;
	int width;
	int max_val;
	bool image_has_value;
	std::string name;
	YCbCrPixel** data;
	downsampledPixel** downsampledData;

	YCbCrImage();
	YCbCrImage(int height, int width, int max_val, std::string name);
	YCbCrImage(const YCbCrImage& other);
	~YCbCrImage();

	void resize_data_array(int old_height, int new_width, int new_height);
	void output_ycbcr();
	void output_luminance();
	void output_RedChrominance();
	void output_blueChrominance();
	bool has_value();
	void chrominance_downsampling();
	void rescale_chrominance_data();
	void luminanace_dct(); 
	std::array<std::array<int, 8>, 8> compute_dct_matrix(const std::array<std::array<unsigned char, 8>, 8>& block);
	
};
