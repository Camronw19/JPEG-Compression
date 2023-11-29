#pragma once
#include <string>

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

	YCbCrImage();
	YCbCrImage(int height, int width, int max_val, std::string name);
	YCbCrImage(const YCbCrImage& other);
	~YCbCrImage();

	void resize_data_array(int old_height, int new_width, int new_height);
	void output_ycbcr();
	bool has_value();
};
