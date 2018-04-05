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
#define MAX_EPOCH 20
#define HISTORGAM_SCALE 64

using namespace cimg_library;
typedef CImg<unsigned char> Image;
typedef struct Feature Feature;

struct Feature
{
    std::string name;
    float mean[3];
    float var[3];
    float histogram[3][256];
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
    CImg<float> temp;
    float area;

    for (int i = 0; i < num_image; i++)
    {
        area = src[i].width() * src[i].height();

        for (int channel = 0; channel < 3; channel++)
        {
            dst[i].mean[channel] = src[i].get_channel(channel).mean() / 256.0;
            dst[i].var[channel] = std::sqrt((src[i].get_channel(channel).variance())) / 256.0;

            temp = src[i].get_channel(channel).get_histogram(256, 0, 255);
            for (int scale = 0; scale < 256; scale++)
            {
                // Image(x, y, z, channel), should be normalized.
                // Because the sizes of pictures are different.
                dst[i].histogram[channel][scale] = temp(scale, 0, 0, 0) / area;
            }
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

float Euclidean_dist(Feature x1, Feature x2)
{
    // loss function for k-means feature comparison
    float result = 0;

    for (int channel = 0; channel < 3; channel++)
    {
        result += std::pow((x1.mean[channel] - x2.mean[channel]), 2);
        result += std::pow((x1.var[channel] - x2.var[channel]), 2);
        for (int scale = 0; scale < 256; scale++)
        {
            result += std::pow((x1.histogram[channel][scale] - x2.histogram[channel][scale]), 2);
        }
    }

    return std::sqrt(result);
}

float get_accuracy(std::vector<Feature> *classes)
{
    // accuracy = (num_correct / all_element)
    float acc = 0;
    for (int _class = 0; _class < NUM_CLASS; _class++)
    {
        // the first element is temporary cluster center, not a real picture.
        // so the index start from 1.
        for (int i = 1; i < classes[_class].size(); i++)
        {
            if (classes[_class][i].label == _class)
                acc++;
        }
    }

    return acc / NUM_TOTAL_IMAGE;
}

void update(std::vector<Feature> *classes)
{
    // update the cluster centers.
    for (int _class = 0; _class < NUM_CLASS; _class++)
    {
        // Feature = { name, mean[3], var[3], histogram[256], label }
        Feature temp = {"", {0}, {0}, {0}, _class};
        int num_element = classes[_class].size();

        for (int i = 1; i < num_element; i++)
        {
            for (int channel = 0; channel < 3; channel++)
            {
                temp.mean[channel] += classes[_class][i].mean[channel] / num_element;
                temp.var[channel] += classes[_class][i].var[channel] / num_element;
                for (int scale = 0; scale < 256; scale++)
                {
                    temp.histogram[channel][scale] += classes[_class][i].histogram[channel][scale] / num_element;
                }
            }
        }

        // assign temp and erase other elements.
        classes[_class][0] = temp;
        classes[_class].erase(classes[_class].begin() + 1, classes[_class].end());
    }
}

int k_means(Feature *feature_arr, std::vector<Feature> *classes)
{
    float distance; // the distance between 2 feature vector
    float temp;
    float accuracy;
    int min_class;

    for (int epoch = 0; epoch < MAX_EPOCH; epoch++)
    {
        for (int idx = 0; idx < NUM_TOTAL_IMAGE; idx++)
        {
            distance = 999999;
            for (int _class = 0; _class < NUM_CLASS; _class++)
            {
                // the first element in each "classes" vector is a temporary cluster
                // center, so compare "feature vectors" with classes[_class][0].
                temp = Euclidean_dist(feature_arr[idx], classes[_class][0]);
                if (temp < distance)
                {
                    distance = temp;
                    min_class = _class;
                }
            }
            classes[min_class].push_back(feature_arr[idx]);
        }

        accuracy = get_accuracy(classes);
        std::cout << "Epoch: " << epoch << ", Accuracy: " << accuracy << std::endl;
        if (epoch == MAX_EPOCH - 1)
            break;
        update(classes);
    }
    return 0;
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

    ret = k_means(feature_vector, classes);

    if (ret == 0)
        std::cout << "Done." << std::endl;

    getchar();
    return 0;
}