# pitchlens

A small C++ prototype exploring real-time video analysis for football:
multi-stage capture/processing pipelines, ball and player detection, and
(planned) a virtual offside line. Inspired by the kind of officiating
technology used in modern football (goal-line and offside systems).


## Status

- [x] Phase 1: threaded capture -> process -> output pipeline with timestamps, back-pressure and basic latency/throughput stats

## Build

Requirements: C++17 compiler, CMake >= 3.16, OpenCV 4.x

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

## Run

```bash
./build/pitchlens --input clip.mp4 --output out.mp4        # write result video
./build/pitchlens --input clip.mp4 --show                  # live window (ESC quits)
```

## Design

```
capture thread --> BoundedQueue<Frame> --> process thread (Detector) --> output
```

- `Frame` carries the media timestamp and wall-clock capture time.
- `Detector` is an interface, so the placeholder `MotionDetector` can be swapped for a TensorRT/CUDA detector.
- The bounded queue applies back-pressure instead of growing memory.

