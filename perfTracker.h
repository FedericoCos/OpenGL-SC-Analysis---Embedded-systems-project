#pragma once

#include <iostream>
#include <chrono>
#include <vector>
#include <algorithm>
#include <cstring>

class PerfTracker{
public:
    using Clock = std::chrono::high_resolution_clock;
    std::chrono::time_point<Clock> frameStart;
    std::chrono::time_point<Clock> cpuStart;

    double frameTime = 0.0;
    double cpuTime = 0.0;
    double minFrame = 9999.0, maxFrame = 0.0;
    double avgFrame = 0.0;
    size_t frameCount = 0;
    double fps = 0.0;

    // Draw statistics
    int drawCalls = 0;
    int trisThisFrame = 0;

    // GPU timing support -> not available in ES2.0
    /* bool hasTimerQuery = false;
    GLuint timerQuery = 0;
    GLuint disjointAvailable = 0;
    double gpuTimeMs = -1.0; */

    // For FPS smoothing
    std::vector<double> frameHistory;
    size_t maxHistory = 100;

    void init(){
        frameHistory.resize(maxHistory);
    }

    void beginFrame(){
        frameStart = Clock::now();
        drawCalls = 0;
        trisThisFrame = 0;
        cpuStart = frameStart;
    }

    void markCpuRenderEnd(){
        auto cpuEnd = Clock::now();
        cpuTime = std::chrono::duration<double, std::milli>(cpuEnd-cpuStart).count();
    }

    void endFrame(){
        auto frameEnd = Clock::now();
        frameTime = std::chrono::duration<double, std::milli>(frameEnd - frameStart).count();
        cpuTime = frameTime;

        frameCount++;
        frameHistory[frameCount % maxHistory] = frameTime;

        double sum = 0.0;
        for(auto t : frameHistory){
            sum += t;
        }
        avgFrame = sum / maxHistory;
        fps = (avgFrame > 0.0) ? (1000.0 / avgFrame) : 0.0;

        minFrame = std::min(minFrame, frameTime);
        maxFrame = std::max(maxFrame, frameTime);
    }


    void countDrawCall(){
        drawCalls++;
    }

    void countTrinagles(int tris){
        trisThisFrame += tris;
    }

    void printStats(){
        std::cout << "FPS: " << fps
                  << " | Frame: " << frameTime << " ms" 
                  <<"(avg: " << avgFrame << "ms, min: " << minFrame << " ms, max: " << maxFrame << " ms)"
                  << " | CPU: " << cpuTime << " ms"
                  << " | DrawCalls: " << drawCalls
                  << " | Tris: " << trisThisFrame << std::endl; 
    }

private:


};