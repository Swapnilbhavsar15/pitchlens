#include "detector.h"
#include <opencv2/imgproc.hpp>

MotionDetector::MotionDetector()
    : bg_(cv::createBackgroundSubtractorMOG2(300, 32.0, false)) {}

std::vector<Detection> MotionDetector::detect(const cv::Mat& bgr) {
    cv::Mat small, mask;
    cv::resize(bgr, small, {}, 0.5, 0.5);
    bg_->apply(small, mask);
    cv::morphologyEx(mask, mask, cv::MORPH_OPEN,
                     cv::getStructuringElement(cv::MORPH_ELLIPSE, {3, 3}));
    cv::dilate(mask, mask, cv::Mat(), {-1, -1}, 2);

    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(mask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    std::vector<Detection> out;
    for (const auto& c : contours) {
        cv::Rect r = cv::boundingRect(c);
        if (r.area() < 40) continue;  // drop noise
        out.push_back({cv::Rect(r.x * 2, r.y * 2, r.width * 2, r.height * 2), 1.0f});
    }
    return out;
}

std::unique_ptr<Detector> make_default_detector() {
    return std::make_unique<MotionDetector>();
}
