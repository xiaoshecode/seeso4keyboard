//
// Created by YongGyu Lee on 2021/09/16.
//

#include "camera_thread.h"

#include <iostream>
#include <thread>
#include <utility>
#include <ctime>
#include <string>
namespace sample
{

    CameraThread::CameraThread()
    {

        thread_ = std::thread([this]()
                              { run_impl(); });
    }

    CameraThread::~CameraThread()
    {
        join();
        writer_.release();
    }

    bool CameraThread::run(int camera_index)
    {
        auto lck = pause_wait();
        camera_index_ = camera_index;
        if (!check_status())
            return false;
        lck.unlock();

        pause_ = false;
        cv_.notify_all();

        return true;
    }

    void CameraThread::pause()
    {
        pause_ = true;
        cv_.notify_all();
    }

    std::unique_lock<std::mutex> CameraThread::pause_wait()
    {
        pause_ = true;
        cv_.notify_all();
        std::unique_lock<std::mutex> lck(mutex_);
        return lck;
    }

    void CameraThread::resume()
    {
        pause_ = false;
        cv_.notify_all();
    }

    void CameraThread::run_impl()
    {
        std::unique_lock<std::mutex> lck(mutex_);

        while (true)
        {
            cv_.wait(lck, [this]() -> bool
                     { return !pause_ || stop_; });

            if (stop_)
                break;
            if (!video_.read(frame_) || frame_.empty()) {
                std::cerr << "Camera stream ended or failed.\n";
                pause_ = true;
                continue;
            }
            if (writer_.isOpened())
                writer_ << frame_;
            on_frame_(std::move(frame_));
        }
    }

    void CameraThread::join()
    {
        // writer_->release();
        stop_.store(true, std::memory_order_release);
        cv_.notify_all();

        if (thread_.joinable())
            thread_.join();
    }

    bool CameraThread::check_status()
    {
        video_.open(camera_index_);
        if (!video_.isOpened())
        {
            std::cerr << "Failed to open camera\n";
            return false;
        }
        else if ((video_ >> frame_, frame_.empty()))
        {
            std::cerr << "Camera is opened, but failed to get a frame. Try changing the camera_index\n";
            return false;
        }

        std::string video_file = "recording-" + std::to_string((int)time(0)) + ".avi";
        auto fps = video_.get(cv::VideoCaptureProperties::CAP_PROP_FPS);
        auto cap_w = video_.get(cv::VideoCaptureProperties::CAP_PROP_FRAME_WIDTH);
        auto cap_h = video_.get(cv::VideoCaptureProperties::CAP_PROP_FRAME_HEIGHT);
        writer_.open(video_file, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'), 25, cv::Size(cap_w, cap_h), true);
        printf("%d FPS, <%d,%d>", 25, static_cast<int>(cap_w), static_cast<int>(cap_h));
        return true;
    }

} // namespace sample
