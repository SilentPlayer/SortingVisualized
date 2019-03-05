#include <vector>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
#include <algorithm>
#include <string>
#include <sys/stat.h> // for mkdir

int maxSize = 0;

// randomize colors
cv::Mat randomColors(cv::Mat img)
{
    // create a perfect rainbow
    for (int i = 0; i < img.rows; i++)
    {
        for (int j = 0; j < img.cols; j++)
        {
            img.at<cv::Vec3b>(i, j)[0] = float(j) / float(img.rows) * 179.;
            img.at<cv::Vec3b>(i, j)[1] = 255;
            img.at<cv::Vec3b>(i, j)[2] = 255;
        }
    }

    //shuffle the colors randomly
    std::vector<int> seeds;
    for (int i = 0; i < img.cols; i++)
    {
        seeds.push_back(i);
    }

    cv::Mat output = img.clone();
    for (int x = 0; x < img.rows; x++)
    {
        std::random_shuffle(std::begin(seeds), std::end(seeds));
        for (int j = 0; j < img.cols; j++)
        {
            output.at<cv::Vec3b>(x, j)[0] = img.at<cv::Vec3b>(x, seeds[j])[0]; // only Hue is different
        }
    }
    return output;
}

void swapPixels(cv::Mat img, std::vector<int> places, int row)
{
    cv::Vec3b temp = img.at<cv::Vec3b>(row, places[0]);
    img.at<cv::Vec3b>(row, places[0]) = img.at<cv::Vec3b>(row, places[1]);
    img.at<cv::Vec3b>(row, places[1]) = temp;
}

// modified from: https://www.geeksforgeeks.org/bubble-sort/
std::vector<std::vector<int>> bubbleSort(cv::Mat img, int row)
{
    std::vector<int> temp;
    std::vector<std::vector<int>> swaps;
    int n = img.cols;
    for (int i = 0; i < n; i++)
    {
        // Last i elements are already in place
        for (int j = 0; j < n - i; j++)
        {
            if (img.at<cv::Vec3b>(row, j)[0] > img.at<cv::Vec3b>(row, j + 1)[0])
            {
                temp.push_back(j);
                temp.push_back(j + 1);
                swapPixels(img, temp, row);
                swaps.push_back(temp);
                temp.clear();
            }
        }
    }
    if (swaps.size() > maxSize)
    {
        maxSize = swaps.size();
    }

    return swaps;
}

int main(int argc, char **argv)
{
    std::string dirPath;
    if(argc == 1){
        dirPath  = "test";
    }
    if(argc == 2){
        try
        {
            dirPath = argv[1];
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
        }
        
    }

    cv::Mat imageRGB, imageHSV;
    imageRGB = cv::Mat::zeros(300, 300, CV_8UC3);

    // convert to HSV   CV_BGR2HSV
    cv::cvtColor(imageRGB, imageHSV, CV_BGR2HSV);
    imageHSV = randomColors(imageHSV);
    cv::cvtColor(imageHSV, imageRGB, CV_HSV2BGR);
    cv::imwrite("initial_hsv_1.png", imageRGB);

    // need to copy Mat since it can't be passed by reference and gets modified during the sorting
    cv::Mat copyHSV = imageHSV.clone();

    std::vector<std::vector<std::vector<int>>> moves;
    //get array with all swaps
    for (int i = 0; i < copyHSV.rows; i++)
    {
        moves.push_back(bubbleSort(copyHSV, i));
    }

    int movieImageStep = maxSize / 120; // ERROR?!
    int movieImageFrame = 0, currentMove = 0;
    
    mkdir(dirPath.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

    // for(int i=0; i<moves.size(); i++){
    //     for(int j=0; j<moves[i].size(); j++){
    //         swapPixels(imageHSV, moves[i][j], i);
    //     }
    // }

    while (currentMove < maxSize)
    {
        for (int i = 0; i < imageHSV.cols; i++)
        {
            if (currentMove < moves[i].size() - 1)
            {
                swapPixels(imageHSV, moves[i][currentMove], i);
            }
        }

        if (currentMove % movieImageStep == 0)
        {
            cv::cvtColor(imageHSV, imageRGB, CV_HSV2BGR);
            std::string path = dirPath + "/" + "test" + std::to_string(movieImageFrame) + ".png";
            cv::imwrite(path, imageRGB);
            movieImageFrame++;
        }
        currentMove++;
    }

    cv::cvtColor(copyHSV, imageRGB, CV_HSV2BGR);

    cv::Mat initial = cv::imread("initial_hsv_1.png");
    cv::namedWindow("random", cv::WINDOW_AUTOSIZE);
    cv::namedWindow("sorted", cv::WINDOW_AUTOSIZE);
    cv::imshow("random", initial);
    cv::imshow("sorted", imageRGB);

    cv::waitKey(0);
    return 0;
}