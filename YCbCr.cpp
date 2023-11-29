#include "YCbCr.h"

#include <iostream>
#include <string>
#include <optional>
#include <fstream>
#include <regex>


//===================YCbCrPixel===================//
YCbCrPixel::YCbCrPixel() :y(0), cr(0), cb(0) {}
YCbCrPixel::YCbCrPixel(unsigned char y, unsigned char cr, unsigned char cb) :y(y), cr(cr), cb(cb) {}


//===================YCbCrImage===================//
YCbCrImage::YCbCrImage()
	:height(0), width(0), max_val(0), name(""), image_has_value(false)
{
	data = new YCbCrPixel * [height];
	for (int row = 0; row < height; row++)
		data[row] = new YCbCrPixel[width];
}


YCbCrImage::YCbCrImage(int h, int w, int mv, std::string n)
	:height(h), width(w), max_val(mv), name(n), image_has_value(false)
{
	data = new YCbCrPixel * [height];
	for (int row = 0; row < height; row++)
		data[row] = new YCbCrPixel[width];
}

YCbCrImage::~YCbCrImage()
{
	for (int row = 0; row < height; row++)
		delete[] data[row];
	delete[] data;
}

YCbCrImage::YCbCrImage(const YCbCrImage& other)
	: height(other.height), width(other.width), max_val(other.max_val), name(other.name), image_has_value(false)
{
	data = new YCbCrPixel * [height];
	for (int row = 0; row < height; row++)
	{
		data[row] = new YCbCrPixel[width];
		memcpy(data[row], other.data[row], sizeof(YCbCrPixel) * width);
	}
}

void YCbCrImage::output_ycbcr()
{
	std::ofstream out_file(name, std::ios::binary | std::ios::out);

	if (out_file.is_open())
	{
		out_file << "P6" << "\n";
		out_file << width << " " << height << '\n';
		out_file << std::to_string(max_val) << "\n";

		for (int row = 0; row < height; row++)
			for (int col = 0; col < width; col++)
			{
				out_file << data[row][col].y;
				out_file << data[row][col].cb;
				out_file << data[row][col].cr;
			}

		out_file.close();
		std::cout << "Image sucessfully created\n";
	}
	else
	{
		std::cout << "Unable to create output file" << std::endl;
	}
}

bool YCbCrImage::has_value()
{
	if (image_has_value)
		return true;
	else
		return false;
}

void YCbCrImage::resize_data_array(int old_height, int new_width, int new_height)
{
	for (int row = 0; row < old_height; row++)
		delete[] data[row];
	delete data;

	data = new YCbCrPixel * [new_height];
	for (int row = 0; row < new_height; row++)
		data[row] = new YCbCrPixel[new_width];
}