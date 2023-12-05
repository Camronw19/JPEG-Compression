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

void YCbCrImage::output_luminance()
{

	std::ofstream Luminance("Luminance_" + name, std::ios::binary | std::ios::out);

	if (Luminance.is_open())
	{
		Luminance << "P5" << "\n";
		Luminance << width << " " << height << '\n';
		Luminance << std::to_string(max_val) << "\n";

		for (int row = 0; row < height; row++)
			for (int col = 0; col < width; col++)
			{
				Luminance << data[row][col].y;
			}

		Luminance.close();
		std::cout << "Image sucessfully created\n";
	}
	else
	{
		std::cout << "Unable to create output file" << std::endl;
	}

}

void YCbCrImage::output_RedChrominance()
{

	std::ofstream RedChrominance("RedChrominance_" + name, std::ios::binary | std::ios::out);

	if (RedChrominance.is_open())
	{
		RedChrominance << "P6" << "\n";
		RedChrominance << width << " " << height << '\n';
		RedChrominance << std::to_string(max_val) << "\n";

		for (int row = 0; row < height; row++)
			for (int col = 0; col < width; col++)
			{
				RedChrominance << data[row][col].cr;
				RedChrominance << 0;
				RedChrominance << 0;
			}

		RedChrominance.close();
		std::cout << "Image sucessfully created\n";
	}
	else
	{
		std::cout << "Unable to create output file" << std::endl;
	}

}

void YCbCrImage::output_blueChrominance()
{

	std::ofstream BlueChrominance("BlueChrominance_" + name, std::ios::binary | std::ios::out);

	if (BlueChrominance.is_open())
	{
		BlueChrominance << "P6" << "\n";
		BlueChrominance << width << " " << height << '\n';
		BlueChrominance << std::to_string(max_val) << "\n";

		for (int row = 0; row < height; row++)
			for (int col = 0; col < width; col++)
			{
				BlueChrominance << 0;
				BlueChrominance << 0;
				BlueChrominance << data[row][col].cb;
			}

		BlueChrominance.close();
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

void YCbCrImage::chrominance_downsampling()
{
	int newHeight = height / 2;
	int newWidth = width / 2;

	//Allocate memory for 2d array
	downsampledData = new downsampledPixel * [newHeight];
	for (int row = 0; row < newHeight; row++)
		downsampledData[row] = new downsampledPixel[newWidth];


	for (int row = 0; row < newHeight; row++) {
		for (int col = 0; col < newWidth; col++) {

			unsigned int crSum = 0;
			unsigned int cbSum = 0;

			for (int i = 0; i < 2; i++) {
				for (int j = 0; j < 2; j++) {

					crSum += data[2 * row + i][2 * col + j].cr;
					cbSum += data[2 * row + i][2 * col + j].cb;
				}
			}

			downsampledData[row][col].cr = crSum / 4;
			downsampledData[row][col].cb = cbSum / 4;

		}
	}

	std::cout << "Chrominance downsampling complete" << '\n';

}

void YCbCrImage::rescale_chrominance_data()
{

	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {

			int downscaled_i = i / 2;
			int downscaled_j = j / 2;

			data[i][j].cr = (data[downscaled_i][downscaled_j].cr + data[downscaled_i + 1][downscaled_j].cr + data[downscaled_i][downscaled_j + 1].cr + data[downscaled_i + 1][downscaled_j + 1].cr) / 4;
			data[i][j].cb = (data[downscaled_i][downscaled_j].cb + data[downscaled_i + 1][downscaled_j].cb + data[downscaled_i][downscaled_j + 1].cb + data[downscaled_i + 1][downscaled_j + 1].cb) / 4;
		}
	}

	std::cout << "Chrominance Data rescaled" << '\n';
	name = "Post_downsampling_" + name;
}

