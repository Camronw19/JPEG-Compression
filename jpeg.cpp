#include "jpeg.h"

#include <iostream>
#include <vector>
#include <array>
#include <map>
#include <queue>

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

		// Step 4. Run Length Encoding
		rle_data.y =  run_length_encoding(dct_coefficients.y);
		rle_data.cr = run_length_encoding(dct_coefficients.cr);
		rle_data.cb = run_length_encoding(dct_coefficients.cb); 

		// Step 5. Huffman Coding
		calculate_huffman_freq(); 
		huffman_frequencies.print_y_freq(); 
		HuffmanTrees Trees(huffman_frequencies);
		
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

std::vector<std::pair<int, int>> JPEGCompressor::run_length_encoding(std::vector<std::vector<int>> &data)
{
	const int block_size = 8; 
	std::array<std::array<int, 8>, 8> block;
	std::vector<std::pair<int, int>> encoded_block; 
	std::vector<std::pair<int, int>> encoded_data; 

	for (int row = 0; row < data.size(); row += block_size)
	{
		for (int col = 0; col < data[0].size(); col += block_size)
		{

			// Read block into array
			for (int block_row = 0; block_row < 8; block_row++)
				for (int block_col = 0; block_col < 8; block_col++)
				{
					block[block_row][block_col] = data[row + block_row][col + block_col];
				}

			// encode block
			encoded_block = rle_block(block); 
			encoded_data.insert(encoded_data.end(), encoded_block.begin(), encoded_block.end()); 
			encoded_block.clear(); 
		}
	}

	return encoded_data; 
}

std::vector<std::pair<int, int>> JPEGCompressor::rle_block(const std::array<std::array<int, 8>, 8>& block)
{
	std::vector<int> zig_zag_order = { // Zigzag order for traversing the 8x8 block
	0, 1, 8, 16, 9, 2, 3, 10,
	17, 24, 32, 25, 18, 11, 4, 5,
	12, 19, 26, 33, 40, 48, 41, 34,
	27, 20, 13, 6, 7, 14, 21, 28,
	35, 42, 49, 56, 57, 50, 43, 36,
	29, 22, 15, 23, 30, 37, 44, 51,
	58, 59, 52, 45, 38, 31, 39, 46,
	53, 60, 61, 54, 47, 55, 62, 63
	};

	std::array<int, 64> flattened_block; 
	for (int i = 0; i < 64; ++i)
		flattened_block[i] = (block[zig_zag_order[i] / 8][zig_zag_order[i] % 8]);

	std::vector<std::pair<int, int>> encoded_block; 

	int count = 1;
	for (size_t i = 1; i < flattened_block.size(); ++i)
	{
		if (flattened_block[i] == flattened_block[i - 1])
		{
			count++;
		}
		else
		{
			encoded_block.emplace_back(count, flattened_block[i - 1]);
			count = 1;
		}
	}

	// Adding the last run
	encoded_block.emplace_back(count, flattened_block[flattened_block.size() - 1]);

	// End of block marker
	encoded_block.emplace_back(0, 0); 

	return encoded_block; 
}

void JPEGCompressor::calculate_huffman_freq()
{
	for (const auto& pair : rle_data.y)
		huffman_frequencies.y[pair]++; 

	for (const auto& pair : rle_data.cr)
		huffman_frequencies.cr[pair]++;

	for (const auto& pair : rle_data.cb)
		huffman_frequencies.cb[pair]++;
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

RLE::RLE()
{

}

void print_pairs(std::vector<std::pair<int, int>> &pairs)
{
	for (size_t i = 0; i < pairs.size(); i++)
		std::cout << "(" << pairs[i].first << "," << pairs[i].second << ") ";
}

void RLE::print_y()
{
	print_pairs(y);
}

void RLE::print_cr()
{
	print_pairs(cr);
}

void RLE::print_cb()
{
	print_pairs(cb); 
}

void print_freq_map(std::map<std::pair<int, int>, int>& freq_map)
{
	for (const auto& mapping : freq_map)
		std::cout << "(" << mapping.first.first << "," << mapping.first.second << ") " << "-> " << mapping.second << ", ";
}

void HuffmanFrequencies::print_y_freq()
{
	print_freq_map(y);
}

void HuffmanFrequencies::print_cr_freq()
{
	print_freq_map(cr);
}

void HuffmanFrequencies::print_cb_freq()
{
	print_freq_map(cb);
}

HuffmanNode::HuffmanNode(std::pair<int, int> sym, int freq)
{
	symbol = sym;
	frequency = freq;
	left = nullptr;
	right = nullptr;
}

int HuffmanNode::getFrequency() const
{
	return frequency;
}

std::pair<int, int> HuffmanNode::getSymbol() const
{
	return symbol;
}

void HuffmanNode::setLeftChildNode(HuffmanNode* L)
{
	left = L;
}

void HuffmanNode::setRightChildNode(HuffmanNode* R)
{
	right = R;
}

HuffmanNode* HuffmanNode::getLeftChildNode() const
{
	return left;
}

HuffmanNode* HuffmanNode::getRightChildNode() const
{
	return right;
}

HuffmanTrees::HuffmanTrees(HuffmanFrequencies& frequencies)
{
	rootY = buildTreeFromFrequencies(frequencies.y);
	rootCr = buildTreeFromFrequencies(frequencies.cr);
	rootCb = buildTreeFromFrequencies(frequencies.cb);
	
}

HuffmanNode* HuffmanTrees::buildTreeFromFrequencies(const std::map<std::pair<int, int>, int>& freqMap)
{
	std::priority_queue<HuffmanNode*, std::vector<HuffmanNode*>, CompareNodes> pq;

	for (const auto& pair : freqMap)
	{
		HuffmanNode* leafNode = new HuffmanNode(pair.first, pair.second);
		pq.push(leafNode);
	}
	
	while (pq.size() > 1)
	{
		// Extract two nodes with lowest frequencies
		HuffmanNode* left = pq.top();
		pq.pop();
		HuffmanNode* right = pq.top();
		pq.pop();

		// Create a new node with combined frequency
		HuffmanNode* newNode = new HuffmanNode(std::make_pair(-1, -1), left->getFrequency() + right->getFrequency());
		newNode->left = left;
		newNode->right = right;

		// Insert the new node back into the priority queue
		pq.push(newNode);
	}

	// The last remaining node in the priority queue is the root of the Huffman tree
	HuffmanNode* root = pq.top();

	// Clean up: Free memory for individual nodes that were allocated
	while (!pq.empty())
	{
		delete pq.top();
		pq.pop();
	}

	return root;
}