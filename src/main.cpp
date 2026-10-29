// pitchlens - Phase 1: a threaded capture -> process -> output pipeline.
//   capture thread  : reads frames, stamps them
//   process thread  : runs the Detector, draws boxes, records stats
// Usage: pitchlens --input clip.mp4 [--output out.mp4] [--show]
#include <atomic>
#include <chrono>
#include <iostream>
#include <string>
#include <thread>

#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>

#include "bounded_queue.h"
#include "detector.h"
#include "frame.h"

using Clock = std::chrono::steady_clock;

struct Args {
    std::string input;
    std::string output;
    bool show = false;
};

static bool parse_args(int argc, char** argv, Args& a) {
    for (int i = 1; i < argc; ++i) {
        std::string s = argv[i];
        if (s == "--input" && i + 1 < argc) a.input = argv[++i];
        else if (s == "--output" && i + 1 < argc) a.output = argv[++i];
        else if (s == "--show") a.show = true;
        else { std::cerr << "Unknown argument: " << s << "\n"; return false; }
    }
    return !a.input.empty();
}

int main(int argc, char** argv) {
    Args args;
    if (!parse_args(argc, argv, args)) {
        std::cerr << "Usage: pitchlens --input clip.mp4 [--output out.mp4] [--show]\n";
        return 1;
    }

    cv::VideoCapture cap(args.input);
    if (!cap.isOpened()) {
        std::cerr << "Cannot open input: " << args.input << "\n";
        return 1;
    }
    const double fps = cap.get(cv::CAP_PROP_FPS) > 0 ? cap.get(cv::CAP_PROP_FPS) : 25.0;
    const cv::Size size(static_cast<int>(cap.get(cv::CAP_PROP_FRAME_WIDTH)),
                        static_cast<int>(cap.get(cv::CAP_PROP_FRAME_HEIGHT)));

    cv::VideoWriter writer;
    if (!args.output.empty())
        writer.open(args.output, cv::VideoWriter::fourcc('m', 'p', '4', 'v'), fps, size);

    BoundedQueue<Frame> queue(8);

    std::thread capture_thread([&] {
        long long idx = 0;
        cv::Mat img;
        while (cap.read(img)) {
            Frame f;
            f.image = img.clone();
            f.index = idx++;
            f.media_ts_ms = cap.get(cv::CAP_PROP_POS_MSEC);
            f.captured_at = Clock::now();
            if (!queue.push(std::move(f))) break;
        }
        queue.close();
    });

    auto detector = make_default_detector();
    long long processed = 0;
    double latency_sum_ms = 0.0;
    const auto t0 = Clock::now();

    while (auto f = queue.pop()) {
        auto dets = detector->detect(f->image);
        for (const auto& d : dets)
            cv::rectangle(f->image, d.box, {0, 255, 0}, 2);

        const double latency_ms =
            std::chrono::duration<double, std::milli>(Clock::now() - f->captured_at).count();
        latency_sum_ms += latency_ms;
        ++processed;

        cv::putText(f->image, "frame " + std::to_string(f->index), {20, 40},
                    cv::FONT_HERSHEY_SIMPLEX, 1.0, {255, 255, 255}, 2);
        if (writer.isOpened()) writer.write(f->image);
        if (args.show) {
            cv::imshow("pitchlens", f->image);
            if (cv::waitKey(1) == 27) break;  // ESC
        }
    }
    queue.close();
    capture_thread.join();

    const double secs = std::chrono::duration<double>(Clock::now() - t0).count();
    if (processed > 0) {
        std::cout << "Frames: " << processed
                  << " | Throughput: " << processed / secs << " FPS"
                  << " | Avg latency: " << latency_sum_ms / processed << " ms\n";
    }
    return 0;
}
