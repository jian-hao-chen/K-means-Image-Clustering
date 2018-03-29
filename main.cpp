#include "CImg.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <cmath>

#define NUM_TOTAL_IMAGE 1048
#define NUM_CLASS 6

using namespace cimg_library;
typedef CImg<unsigned char> Image;
typedef struct Feature Feature;

struct Feature
{
    std::string name;
    float mean[3];
    float var[3];
    int histogram[256];
    int label;
};

int read_dataset(char *list, Image *dst, Feature *feature_vector)
{
    std::map<int, std::string> dict;
    dict[0] = "airplane";
    dict[1] = "pyramid";
    dict[2] = "sailing";
    dict[3] = "starfish";
    dict[4] = "sunflower";
    dict[5] = "tower";

    int idx = 0;
    char buffer[200];
    std::fstream fin;
    fin.open(list, std::ios::in);
    while (fin.getline(buffer, sizeof(buffer)))
    {
        dst[idx].load(buffer);
        feature_vector[idx].name = buffer;

        for (int i = 0; i < NUM_CLASS; i++)
        {
            // each feature_vector[idx].name have "./ObjectDataset/" which consists
            // of 15 charaters, so find the label at the 16th character.
            if (feature_vector[idx].name.find(dict[i], 15) != std::string::npos)
            {
                feature_vector[idx].label = i;
                break;
            }
        }

        idx++;
    }

    return 0;
}

int feature_extract(Image *src, Feature *dst, int num_image)
{
    for (int i = 0; i < num_image; i++)
    {
        for (int channel = 0; channel < 3; channel++)
        {
            dst[i].mean[channel] = (float)(src[i].get_channel(channel).mean());
            dst[i].var[channel] = (float)(src[i].get_channel(channel).variance());
        }
    }

    CImg<float> temp;

    for (int i = 0; i < num_image; i++)
    {
        temp = src[i].get_histogram(256);
        for (int scale = 0; scale < 256; scale++)
        {
            dst[i].histogram[scale] = temp(scale, 0, 0, 0); // Image(x, y, z, channel);
        }
    }
    return 0;
}

int initialize(Feature *feature_arr, std::vector<Feature> *classes)
{
    // pick one feature of each class into vector as initial pattern.
    int current_class = 0, idx = 0;
    while (current_class < NUM_CLASS)
    {
        if (feature_arr[idx].label == current_class)
        {
            classes[current_class].push_back(feature_arr[idx]);
            current_class++;
        }
        idx++;
    }

    return 0;
}

int k_means(Feature *feature_arr, std::vector<Feature> *classes)
{
    return 0;
}

float mse(Feature x1, Feature x2)
{
    // loss function for k-means feature comparison
    float result = 0;

    for (int channel = 0; channel < 3; channel++)
    {
        result += std::pow((x1.mean[channel] - x2.mean[channel]), 2);
        result += std::pow((x1.var[channel] - x2.var[channel]), 2);
    }

    for (int scale = 0; scale < 256; scale++)
    {
        result += std::pow((x1.histogram[scale] - x2.histogram[scale]), 2);
    }

    return std::sqrt(result);
}

int main()
{
    int ret;
    Image src[NUM_TOTAL_IMAGE];
    Feature feature_vector[NUM_TOTAL_IMAGE];
    std::vector<Feature> classes[NUM_CLASS];
    /* 
        There are 6 classes.
        0: airplane,
        1: pyramid,
        2: sailing,
        3: starfish,
        4: sunflower,
        5: tower
    */

    ret = read_dataset("list.txt", src, feature_vector);
    ret = feature_extract(src, feature_vector, NUM_TOTAL_IMAGE);
    ret = initialize(feature_vector, classes);

    // shuffle the dataset to validation k-means algorithm.
    std::random_shuffle(feature_vector, feature_vector + NUM_TOTAL_IMAGE);

    if (ret == 0)
        std::cout << "Done." << std::endl;

    getchar();
    return 0;
}