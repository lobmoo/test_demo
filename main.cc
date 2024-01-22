#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>


void saveFrames(const std::string& videoPath, const std::string& outputFolder) {
    // 打开视频文件
    cv::VideoCapture video(videoPath);
    if (!video.isOpened()) {
        std::cout << "无法打开视频文件" << std::endl;
        return;
    }

    // 创建输出文件夹
    if (mkdir(outputFolder.c_str(), 0777) != 0) {
        std::cout << "无法创建输出文件夹" << std::endl;
        return;
    }

    int frameCount = 0;
    while (true) {
        // 读取视频的一帧
        cv::Mat frame;
        video >> frame;

        if (frame.empty())
            break;

        // 生成输出文件名
        std::string outputFilePath = outputFolder + "/frame_" + std::to_string(frameCount) + ".jpg";

        // 保存帧为图像文件
        cv::imwrite(outputFilePath, frame);

        frameCount++;
    }

    // 释放视频对象
    video.release();
}

int main() {
    std::string videoPath = "/home/wwk/docker_project/video2image/AR0820_800W_H60_1.mp4";  // 替换为你的视频文件路径
    std::string outputFolder = "/home/wwk/docker_project/video2image/111";  // 替换为你想要保存帧的输出文件夹路径

    saveFrames(videoPath, outputFolder);

    return 0;
}
