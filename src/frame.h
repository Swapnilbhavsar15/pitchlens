#pragma once
#include <chrono>
#include <opencv2/core.hpp>

// A frame travelling through the pipeline. Carries both the media
// timestamp (position in the video) and the wall-clock capture time,
// so we can measure end-to-end latency later.
struct Frame {
    cv::Mat image;
    long long index = 0;
    double media_ts_ms = 0.0;
    std::chrono::steady_clock::time_point captured_at;
};
