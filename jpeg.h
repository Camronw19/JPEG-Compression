#pragma once
#include <memory> 

#include "ppm.h"
#include "YCbCr.h"
#include <vector>
#include <map>

enum DCTMode
{
	Y, 
	Cr, 
	Cb
};


struct DCTCoefficients
{
public: 

	std::vector<std::vector<int>> y; 
	std::vector<std::vector<int>> cr;
	std::vector<std::vector<int>> cb;
	
	DCTCoefficients();

	void resize_matrices(int height, int width); 
	void print_y(); 
	void print_cr();

};

struct RLE
{
public:
	std::vector<std::pair<int, int>> y;
	std::vector<std::pair<int, int>> cr;
	std::vector<std::pair<int, int>> cb;

	RLE(); 
	void print_y();
	void print_cr();
	void print_cb();
};

struct HuffmanFrequencies
{
public: 
	std::map<std::pair<int, int>, int> y;
	std::map<std::pair<int, int>, int> cr;
	std::map<std::pair<int, int>, int> cb;

	HuffmanFrequencies() {}; 
	void print_y_freq();
	void print_cr_freq();
	void print_cb_freq();
};

struct HuffmanNode
{

	int frequency; 
	std::pair<int, int> symbol; 
	HuffmanNode* left; 
	HuffmanNode* right;

public:

	HuffmanNode(std::pair<int, int> sym, int freq);
	
	int getFrequency() const;
	std::pair<int, int> getSymbol() const;
	void setLeftChildNode(HuffmanNode* L);
	void setRightChildNode(HuffmanNode* R);
	HuffmanNode* getLeftChildNode() const;
	HuffmanNode* getRightChildNode() const;
};

class HuffmanTrees
{
private:
	
	HuffmanNode* rootY;
	HuffmanNode* rootCr;
	HuffmanNode* rootCb;

	std::map<std::pair<int, int>, std::string> codesY;
	std::map<std::pair<int, int>, std::string> codesCr;
	std::map<std::pair<int, int>, std::string> codesCb;
	
public:

	
	HuffmanTrees(HuffmanFrequencies& frequencies);

	HuffmanNode* buildTreeFromFrequencies(const std::map<std::pair<int, int>, int>& freqMap, std::map<std::pair<int, int>, std::string>& codes);
	void generateHuffmanCodes(HuffmanNode* root, std::map<std::pair<int, int>, std::string>& codes, std::string code = "");

	std::map<std::pair<int, int>, std::string>& getYCodes() { return codesY; }
	std::map<std::pair<int, int>, std::string>& getCrCodes() { return codesCr; }
	std::map<std::pair<int, int>, std::string>& getCbCodes() { return codesCb; }
};

struct CompareNodes
{
	bool operator()(const HuffmanNode* lhs, const HuffmanNode* rhs) const
	{
		return lhs->getFrequency() > rhs->getFrequency(); 
	}
};

class JPEGCompressor
{
private: 

	// private data members 
	std::unique_ptr<YCbCrImage> ycbcr; 
	DCTCoefficients dct_coefficients; 
	RLE rle_data; 
	HuffmanFrequencies huffman_frequencies; 

	const std::array<std::array<int, 8>, 8> luminance_q_matrix = { {
		{16, 11, 10, 16, 24, 40, 51, 61},
		{12, 12, 14, 19, 26, 58, 60, 55},
		{14, 13, 16, 24, 40, 57, 69, 56},
		{14, 17, 22, 29, 51, 87, 80, 62},
		{18, 22, 37, 56, 68, 109, 103, 77},
		{24, 35, 55, 64, 81, 104, 113, 92},
		{49, 64, 78, 87, 103, 121, 120, 101},
		{72, 92, 95, 98, 112, 100, 103, 99}
	} };
	const std::array<std::array<int, 8>, 8> chrominance_q_matrix = { {
	   {17, 18, 24, 47, 99, 99, 99, 99},
	   {18, 21, 26, 66, 99, 99, 99, 99},
	   {24, 26, 56, 99, 99, 99, 99, 99},
	   {47, 66, 99, 99, 99, 99, 99, 99},
	   {99, 99, 99, 99, 99, 99, 99, 99},
	   {99, 99, 99, 99, 99, 99, 99, 99},
	   {99, 99, 99, 99, 99, 99, 99, 99},
	   {99, 99, 99, 99, 99, 99, 99, 99}
   } };

	// helper methods
	void rgb_to_ycbcr(const PPMImage& rgb_image);

	void dct(DCTMode mode);
	std::array<std::array<int, 8>, 8> compute_dct_matrix(const std::array<std::array<unsigned char, 8>, 8>& input_matrix);

	std::vector<std::pair<int, int>> run_length_encoding(std::vector<std::vector<int>>& data);
	std::vector<std::pair<int, int>> rle_block(const std::array<std::array<int, 8>, 8>& block);

	void calculate_huffman_freq();

public: 
	JPEGCompressor();
	void compress_ppm(const PPMImage& image); 
};
