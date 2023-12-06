#include "jpeg.h"

#include <iostream>
#include <vector>
#include <array>

JPEGCompressor::JPEGCompressor()
{

}


void JPEGCompressor::compress_ppm(const PPMImage& image)
{
	if (image.has_value())
	{
		// Step 1. Color Space Conversion
		rgb_to_ycbcr(image);

		// Step 2. Chrominance Downsampling
		ycbcr->chrominance_downsampling(); 

		// Step 3. Discrete Cosine Transform + Quantization
		dct_coefficients.resize_matrices(ycbcr->height, ycbcr->width); 
		dct(DCTMode::Y);
		dct(DCTMode::Cr);
		dct(DCTMode::Cb);

		dct_coefficients.print_y(); 
		
	}
}


void JPEGCompressor::dct(DCTMode mode)
{
	const int block_size = 8;
	const int value_shift = 128;
	int h = ycbcr->height;
	int w = ycbcr->width;

	if (mode != DCTMode::Y)
	{
		h /= 2; 
		w /= 2; 
	}

	std::array<std::array<unsigned char, 8>, 8> block;
	std::array<std::array<int, 8>, 8> dct_block;

	// Iterate through the start index of each block [row, col]
	for (int row = 0; row < h; row += block_size)
	{
		for (int col = 0; col < w; col += block_size)
		{

			// Read block into array
			for (int block_row = 0; block_row < 8; block_row++)
				for (int block_col = 0; block_col < 8; block_col++)
				{
					if(mode == DCTMode::Y)
						block[block_row][block_col] = ycbcr->data[row + block_row][col + block_col].y - value_shift;

					else if (mode == DCTMode::Cr)
						block[block_row][block_col] = ycbcr->downsampledData[row + block_row][col + block_col].cr - value_shift;

					else if (mode == DCTMode::Cb)
						block[block_row][block_col] = ycbcr->downsampledData[row + block_row][col + block_col].cb - value_shift;
				}

			// send block to dct 
			dct_block = compute_dct_matrix(block);

			// quantize and then copy dct block into full matrix
			for (int r = 0; r < dct_block.size(); r++)
				for (int c = 0; c < dct_block[0].size(); c++)
				{
					if (mode == DCTMode::Y)
						dct_coefficients.y[row + r][col + c] = dct_block[r][c] /= luminance_q_matrix[r][c];

					if (mode == DCTMode::Cr)
						dct_coefficients.cr[row + r][col + c] = dct_block[r][c] /= chrominance_q_matrix[r][c];

					if (mode == DCTMode::Cb)
						dct_coefficients.cb[row + r][col + c] = dct_block[r][c] /= chrominance_q_matrix[r][c];
				}
		}
	}
}

std::array<std::array<int, 8>, 8> JPEGCompressor::compute_dct_matrix(const std::array<std::array<unsigned char, 8>, 8>& input_matrix)
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

	return dct_matrix;
}

void JPEGCompressor::rgb_to_ycbcr(const PPMImage& rgb_image)
{

	ycbcr.reset(new YCbCrImage(rgb_image.height, rgb_image.width, rgb_image.max_val, "YCbCr_" + rgb_image.name));

	for (int row = 0; row < rgb_image.height; row++)
	{
		for (int col = 0; col < rgb_image.width; col++)
		{
			double yr = ((65.738 * rgb_image.data[row][col].red) / 256);
			double yg = ((129.057 * rgb_image.data[row][col].green) / 256);
			double yb = ((25.064 * rgb_image.data[row][col].blue) / 256);
			ycbcr->data[row][col].y = 16 + yr + yg + yb;

			double cbr = ((37.945 * rgb_image.data[row][col].red) / 256);
			double cbg = ((74.494 * rgb_image.data[row][col].green) / 256);
			double cbb = ((112.439 * rgb_image.data[row][col].blue) / 256);
			ycbcr->data[row][col].cb = 128 - cbr - cbg + cbb;

			double crr = ((112.439 * rgb_image.data[row][col].red) / 256);
			double crg = ((94.154 * rgb_image.data[row][col].green) / 256);
			double crb = ((18.285 * rgb_image.data[row][col].blue) / 256);
			ycbcr->data[row][col].cr = 128 + crr - crg - crb;
		}
	}
}


DCTCoefficients::DCTCoefficients()
{
	
}


void DCTCoefficients::resize_matrices(int height, int width)
{
	y.resize(height, std::vector<int>(width));
	cr.resize(height / 2, std::vector<int>(width / 2));
	cb.resize(height / 2, std::vector<int>(width / 2));
}

void DCTCoefficients::print_y()
{
	for (int row = 0; row < y.size(); row++)
	{
		for (int col = 0; col < y[0].size(); col++)
		{
			std::cout << y[row][col] << " "; 
		}
		std::cout << std::endl; 
	}
}

void DCTCoefficients::print_cr()
{
	for (int row = 0; row < cr.size(); row++)
	{
		for (int col = 0; col < cr[0].size(); col++)
		{
			std::cout << cr[row][col] << " ";
		}
		std::cout << std::endl;
	}
}
