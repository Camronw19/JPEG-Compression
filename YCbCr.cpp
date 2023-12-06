#include "YCbCr.h"

#include <iostream>
#include <string>
#include <optional>
#include <fstream>
#include <regex>
#include <array>
#include <cmath>
#include <vector>


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


void YCbCrImage::luminanace_dct()
{
	// *optimize by using pointer arithmitic* // 

	// create dct matrix and resize to image dimensions
	std::vector<std::vector<int>> dct_matrix;
	dct_matrix.reserve(height); 
	for (int row = 0; row < height; row++)
	{
		dct_matrix.emplace_back(); 
		dct_matrix.back().reserve(width);
	}

	std::array<std::array<unsigned char, 8>, 8> block;
	std::array<std::array<int, 8>, 8> dct_block;

	int block_size = 8; 
	int value_shift = 128; 

	// Iterate through the start index of each block [row, col]
	for (int row = 0; row < height; row += block_size)
	{
		for (int col = 0; col < width; col += block_size)
		{

			// Read block into array
			for (int block_row = 0; block_row < 8; block_row++)
				for (int block_col = 0; block_col < 8; block_col++)
					block[block_row][block_col] = data[row + block_row][col + block_col].y - value_shift;

			// send block to dct 
			dct_block = compute_dct_matrix(block); 

			// copy dct block into full matrix
			for (int r = 0; r < dct_block.size(); r++)
				for (int c = 0; c < dct_block[0].size(); c++)
					dct_matrix[row + r][col + c] = dct_block[r][c]; 

		}
	}
}			

std::array<std::array<int, 8>, 8> YCbCrImage::compute_dct_matrix(const std::array<std::array<unsigned char, 8>, 8>& input_matrix)
{
	std::array<std::array<int, 8>, 8> dct_matrix;

	constexpr double pi = 3.14159265358979323846;
	const int height = 8; 
	const int width = 8;

	float ci, cj, dct1, sum; 

	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			// ci and cj depends on frequency as well as
			// number of row and columns of specified matrix

			if (i == 0)
				ci = 1 / sqrt(height);
			else
				ci = sqrt(2) / sqrt(height);
			if (j == 0)
				cj = 1 / sqrt(width);
			else
				cj = sqrt(2) / sqrt(width);

			// sum will temporarily store the sum of cosine signals
			sum = 0;
			for (int k = 0; k < height; k++)
			{
				for (int l = 0; l < width; l++)
				{
					dct1 = input_matrix[k][l] *
						cos((2 * k + 1) * i * pi / (2 * height)) *
						cos((2 * l + 1) * j * pi / (2 * width));
					sum = sum + dct1;
				}
			}
			dct_matrix[i][j] = ci * cj * sum;
		}
	}


	for (int i = 0; i < dct_matrix.size(); i++)
	{
		for (int j = 0; j < dct_matrix[0].size(); j++)
		{
			std::cout << dct_matrix[i][j] << " "; 
		}
		std::cout << std::endl;
	}

	return dct_matrix; 
}