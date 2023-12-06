#include "ppm.h"

#include <iostream>
#include <string>
#include <optional>
#include <fstream>
#include <regex>


//===================PPMPixel===================//
PPMPixel::PPMPixel() :red(0), green(0), blue(0) {}
PPMPixel::PPMPixel(unsigned char r, unsigned char g, unsigned char b) :red(r), green(g), blue(b) {}


//===================PPMImage===================//
PPMImage::PPMImage()
	:height(0), width(0), max_val(0), name(""), image_has_value(false)
{
	data = new PPMPixel * [height];
	for (int row = 0; row < height; row++)
		data[row] = new PPMPixel[width];
}

PPMImage::PPMImage(std::string path)
	:height(0), width(0), max_val(0), name(""), image_has_value(false)
{
	this->read_ppm(path); 
}


PPMImage::PPMImage(int h, int w, int mv, std::string n)
	:height(h), width(w), max_val(mv), name(n), image_has_value(false)
{
	data = new PPMPixel * [height];
	for (int row = 0; row < height; row++)
		data[row] = new PPMPixel[width];
}

PPMImage::~PPMImage()
{
	for (int row = 0; row < height; row++)
		delete[] data[row];
	delete[] data;
}

PPMImage::PPMImage(const PPMImage& other)
	: height(other.height), width(other.width), max_val(other.max_val), name(other.name), image_has_value(false)
{
	data = new PPMPixel * [height];
	for (int row = 0; row < height; row++)
	{
		data[row] = new PPMPixel[width];
		memcpy(data[row], other.data[row], sizeof(PPMPixel) * width);
	}
}

void PPMImage::read_ppm(std::string file_path)
{
	std::ifstream input_file(file_path, std::ios::binary | std::ios::in);

	if (input_file.is_open())
	{
		// Read header information
		std::string line;
		std::regex formatRegex("^P6\\s+(\\d+)\\s+(\\d+)\\s+(\\d+)\\s*$");
		std::smatch match;
		bool header_found = false;
		int old_height = height;

		while (std::getline(input_file, line))
		{
			// Skip comments
			if (line[0] == '#')
				continue;

			else
			{
				if (std::regex_match(line, match, formatRegex))
				{
					width = std::stoi(match[1]);
					height = std::stoi(match[2]);
					max_val = std::stoi(match[3]);
					header_found = true;
					break;
				}
			}
		}

		if (!header_found)
			return;

		// Read PPM image into image object
		resize_data_array(old_height, width, height);

		for (int row = 0; row < height; row++)
		{
			for (int col = 0; col < width; col++)
			{
				char pixel_value;
				input_file.read(&pixel_value, 1);
				data[row][col].red = static_cast<unsigned char>(pixel_value);
				input_file.read(&pixel_value, 1);
				data[row][col].green = static_cast<unsigned char>(pixel_value);
				input_file.read(&pixel_value, 1);
				data[row][col].blue = static_cast<unsigned char>(pixel_value);
			}
		}

		name = file_path;
		std::cout << "File succesfully opened and read - " << name << std::endl;
		input_file.close();

		image_has_value = true;
	}
	else
	{

	}
}

void PPMImage::output_ppm()
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
				out_file << data[row][col].red;
				out_file << data[row][col].green;
				out_file << data[row][col].blue;
			}

		out_file.close();
		std::cout << "Image sucessfully created\n";
	}
	else
	{
		std::cout << "Unable to create output file" << std::endl;
	}
}

bool PPMImage::has_value() const
{
	if (image_has_value)
		return true;
	else
		return false;
}

void PPMImage::resize_data_array(int old_height, int new_width, int new_height)
{
	for (int row = 0; row < old_height; row++)
		delete[] data[row];
	delete data;

	data = new PPMPixel * [new_height];
	for (int row = 0; row < new_height; row++)
		data[row] = new PPMPixel[new_width];
}