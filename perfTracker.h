#pragma once

#include <iostream>
#include <chrono>
#include <vector>
#include <algorithm>
#include <numeric>   
#include <iomanip> 
#include <fstream>  

class PerfTracker {
public:
    // Clock
    using Clock = std::chrono::high_resolution_clock;
    std::chrono::time_point<Clock> frameStart, cpuRenderStart;

    // Frame timings (in milliseconds)
    double frameTime = 0.0, cpuRenderTime = 0.0, gpuWaitTime = 0.0;
    double minFrame = 9999.0, maxFrame = 0.0, avgFrame = 0.0;
    double fps = 0.0;

    // Frame counters
    size_t frameCount = 0;
    int drawCalls = 0;
    int trisThisFrame = 0;

    // State change counters
    int shaderBinds = 0;
    int textureBinds = 0;

    // Memory tracking (in bytes)
    long long totalVramAllocated = 0;
    long long dataUploadedThisFrame = 0;

    // CSV
    std::ofstream csvFile;
    bool csvEnabled = false;

private:
    // For FPS smoothing
    std::vector<double> frameHistory;
    size_t historySize = 100;

public:
    void init(bool &save, size_t history = 100, const std::string &csvPath = "stats.csv") {
        historySize = history;
        frameHistory.resize(historySize, 0.0);

        if(!csvPath.empty() && save){
            csvFile.open(csvPath, std::ios::out);
            if (csvFile.is_open()) {
                csvEnabled = true;
                csvFile << "FPS,FrameTime(ms),MinFrame(ms),MaxFrame(ms),AvgFrame(ms),CPUTime(ms),GPUWait(ms),DrawCalls,Triangles,VRAM(MB),Upload(KB),\n";
            } else {
                std::cerr << "[PerfTracker] Failed to open CSV file: " << csvPath << "\n";
            }

        }
    }

    void beginFrame() {
        frameStart = Clock::now();
        // Reset per-frame counters
        drawCalls = 0;
        trisThisFrame = 0;
        shaderBinds = 0;
        textureBinds = 0;
        dataUploadedThisFrame = 0;
    }

    void beginCpuRender() {
        cpuRenderStart = Clock::now();
    }

    void endCpuRender() {
        auto cpuEnd = Clock::now();
        cpuRenderTime = std::chrono::duration<double, std::milli>(cpuEnd - cpuRenderStart).count();
    }

    void endFrame() {
        auto frameEnd = Clock::now();
        frameTime = std::chrono::duration<double, std::milli>(frameEnd - frameStart).count();
        gpuWaitTime = frameTime - cpuRenderTime;

        // Update history for smoothed FPS
        frameHistory[frameCount % historySize] = frameTime;
        frameCount++;

        double sum = std::accumulate(frameHistory.begin(), frameHistory.end(), 0.0);
        size_t currentHistorySize = (frameCount < historySize) ? frameCount : historySize;
        avgFrame = sum / currentHistorySize;
        fps = (avgFrame > 0.0) ? (1000.0 / avgFrame) : 0.0;

        minFrame = std::min(minFrame, frameTime);
        maxFrame = std::max(maxFrame, frameTime);

        if (csvEnabled) {
            csvFile << fps << ","
                    << frameTime << ","
                    << minFrame << ","
                    << maxFrame << ","
                    << avgFrame << ","
                    << cpuRenderTime << ","
                    << gpuWaitTime << ","
                    << drawCalls << ","
                    << trisThisFrame << ","
                    << totalVramAllocated / (1024.0 * 1024.0) << ","
                    << dataUploadedThisFrame / 1024.0 << ","
                    << "\n";
        }
    }

    // --- Counter Methods ---
    void countDrawCall() { drawCalls++; }
    void countTriangles(int tris) { trisThisFrame += tris; }
    void countShaderBind() { shaderBinds++; }
    void countTextureBind() { textureBinds++; }

    // --- Memory Tracking Methods ---
    void trackVramAllocation(long long bytes) { totalVramAllocated += bytes; }
    void trackVramDeallocation(long long bytes) { totalVramAllocated -= bytes; }
    void trackDataUpload(long long bytes) { dataUploadedThisFrame += bytes; }

    void printStats() {
        // Convert memory to MB for readability
        double vramMB = totalVramAllocated / (1024.0 * 1024.0);
        double uploadKB = dataUploadedThisFrame / 1024.0;

        std::cout << std::fixed << std::setprecision(4)
              << "FPS: " << fps
              << " | Frame: " << frameTime << "ms"
              << " (Min: " << minFrame << "ms, Max: " << maxFrame << "ms, Avg: " << avgFrame << "ms)" 
              << " | CPU: " << cpuRenderTime << "ms"
              << " | GPU Wait: " << gpuWaitTime << "ms"
              << " | Calls: " << drawCalls
              << " | Tris: " << (trisThisFrame / 1000) << "k"
              << " | VRAM: " << vramMB << "MB"
              << " | Upload: " << uploadKB << "KB"
              << std::endl;
    }
};