#pragma once
#include <memory>
#include <vector>
#include <opencv2/core.hpp>
#include <opencv2/video/background_segm.hpp>

struct Detection {
    cv::Rect box;
    float score = 1.0f;
};

// Pluggable interface: later replace with a TensorRT / CUDA-based detector
// without touching the pipeline code.
class Detector {
public:
    virtual ~Detector() = default;
    virtual std::vector<Detection> detect(const cv::Mat& bgr) = 0;
};

// Phase 1 placeholder: motion-based detector (background subtraction).
// Not accurate, but enough to prove the pipeline end to end.
class MotionDetector : public Detector {
public:
    MotionDetector();
    std::vector<Detection> detect(const cv::Mat& bgr) override;
private:
    cv::Ptr<cv::BackgroundSubtractorMOG2> bg_;
};

std::unique_ptr<Detector> make_default_detector();
