#pragma once

#include <iostream>
#include <chrono>
#include <vector>
#include <numeric>
#include <iomanip>
#include <fstream>

class PerfTracker {
public:
    using Clock = std::chrono::high_resolution_clock;
    std::chrono::time_point<Clock> frameStart, cpuRenderStart, shadowPassStart;

    // Frame timings
    double frameTime = 0.0, cpuRenderTime = 0.0, gpuWaitTime = 0.0;
    double minFrame = 9999.0, maxFrame = 0.0, avgFrame = 0.0, fps = 0.0;
    double shadowPassTime = 0.0;

    // Frame counters
    size_t frameCount = 0;

    // Scene-specific draw calls
    int wallDrawCalls = 0;
    int pointLightDrawCalls = 0;
    int modelDrawCalls = 0;
    int shadowPassDrawCalls = 0;

    // Scene-specific triangles
    int wallTris = 0;
    int pointLightTris = 0;
    int modelTris = 0;

    // Lights
    int activePointLights = 0;
    int activeSpotLights = 0;

    // Memory
    long long totalVramAllocated = 0;
    long long dataUploadedThisFrame = 0;

    // CSV
    std::ofstream csvFile;
    bool csvEnabled = false;

private:
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
                csvFile << "FPS,Frame(ms),Min(ms),Max(ms),Avg(ms),CPU(ms),GPUWait(ms),"
                        << "ShadowPass(ms),WallsDC,PointsDC,ModelDC,ShadowDC,"
                        << "WallsTris,PointsTris,ModelTris,"
                        << "PointLights,SpotLights,"
                        << "VRAM(MB),Upload(KB)\n";
            } else {
                std::cerr << "[PerfTracker] Failed to open CSV file: " << csvPath << "\n";
            }
        }
    }

    void beginFrame() {
        frameStart = Clock::now();
        wallDrawCalls = pointLightDrawCalls = modelDrawCalls = shadowPassDrawCalls = 0;
        wallTris = pointLightTris = modelTris = 0;
        dataUploadedThisFrame = 0;
    }

    void beginCpuRender() {
        cpuRenderStart = Clock::now();
    }

    void endCpuRender() {
        auto cpuEnd = Clock::now();
        cpuRenderTime = std::chrono::duration<double, std::milli>(cpuEnd - cpuRenderStart).count();
    }

    void beginShadowPass() {
        shadowPassStart = Clock::now();
    }

    void endShadowPass() {
        auto shadowEnd = Clock::now();
        shadowPassTime = std::chrono::duration<double, std::milli>(shadowEnd - shadowPassStart).count();
    }

    void endFrame() {
        auto frameEnd = Clock::now();
        frameTime = std::chrono::duration<double, std::milli>(frameEnd - frameStart).count();
        gpuWaitTime = frameTime - cpuRenderTime;

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
                    << shadowPassTime << ","
                    << wallDrawCalls << ","
                    << pointLightDrawCalls << ","
                    << modelDrawCalls << ","
                    << shadowPassDrawCalls << ","
                    << wallTris << ","
                    << pointLightTris << ","
                    << modelTris << ","
                    << activePointLights << ","
                    << activeSpotLights << ","
                    << totalVramAllocated / (1024.0 * 1024.0) << ","
                    << dataUploadedThisFrame / 1024.0 << "\n";
        }
    }

    // Scene-specific counting
    void countWallDraw(int tris) { wallDrawCalls++; wallTris += tris; }
    void countPointLightDraw(int tris) { pointLightDrawCalls++; pointLightTris += tris; }
    void countModelDraw(int tris) { modelDrawCalls++; modelTris += tris; }
    void countShadowDraw() { shadowPassDrawCalls++; }

    void setActiveLights(int point, int spot) {
        activePointLights = point;
        activeSpotLights = spot;
    }

    // Memory tracking
    void trackVramAllocation(long long bytes) { totalVramAllocated += bytes; }
    void trackVramDeallocation(long long bytes) { totalVramAllocated -= bytes; }
    void trackDataUpload(long long bytes) { dataUploadedThisFrame += bytes; }

    void printStats() {
        std::cout << std::fixed << std::setprecision(2)
                  << "FPS: " << fps
                  << " | Frame: " << frameTime << "ms"
                  << " (CPU: " << cpuRenderTime << "ms, GPU Wait: " << gpuWaitTime << "ms)"
                  << " | Shadow Pass: " << shadowPassTime << "ms"
                  << " | Walls: " << wallDrawCalls << " DC, " << wallTris << " tris"
                  << " | Lights: " << pointLightDrawCalls << " DC, " << pointLightTris << " tris"
                  << " | Model: " << modelDrawCalls << " DC, " << modelTris << " tris"
                  << " | PLights: " << activePointLights << ", SLight: " << activeSpotLights
                  << std::endl;
    }
};
