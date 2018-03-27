#include "CImg.h"
#include <iostream>
#include <fstream>

#define NUM_TOTAL_IMAGE 1048

using namespace cimg_library;
typedef CImg<unsigned char> Image;
typedef struct Feature F;

struct Feature
{
    float mean[3];
    float var[3];
    int histogram[256];
};

int read_dataset(char *list, Image *dst)
{
    int idx = 0;
    char buffer[200];
    std::fstream fin;
    fin.open(list, std::ios::in);
    while (fin.getline(buffer, sizeof(buffer)))
    {
        dst[idx].load(buffer);
        idx++;
    }
    return 0;
}

int cvt_2_grayscale(Image *src, Image *dst, int num_image)
{
    for (int i = 0; i < num_image; i++)
    {
        dst[i] = src[i].get_RGBtoYCbCr().get_channel(0);
    }
    return 0;
}

int main()
{
    int ret;
    Image src[NUM_TOTAL_IMAGE], gray[NUM_TOTAL_IMAGE];
    ret = read_dataset("list.txt", src);
    ret = cvt_2_grayscale(src, gray, NUM_TOTAL_IMAGE);

    if (ret == 0)
        std::cout << "Done." << std::endl;

    getchar();
    return 0;
}