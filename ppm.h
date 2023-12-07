#pragma once
#include <string>

struct PPMPixel
{
	unsigned char red;
	unsigned char green;
	unsigned char blue;

	PPMPixel();
	PPMPixel(unsigned char, unsigned char, unsigned char);
};

class PPMImage
{
public:
	int height;
	int width;
	int max_val;
	bool image_has_value;
	std::string name;
	PPMPixel** data;

	PPMImage();
	PPMImage(std::string path); 
	PPMImage(int height, int width, int max_val, std::string name);
	PPMImage(const PPMImage& other);
	~PPMImage();

	void resize_data_array(int old_height, int new_width, int new_height);
	void read_ppm(std::string path);
	void output_ppm();
	bool has_value() const;
};
