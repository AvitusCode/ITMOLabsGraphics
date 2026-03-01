#pragma once
#include <chrono>

namespace jd
{
    class GameTimer
    {
    public:
        GameTimer();

        void reset();
        void tick();
        void stop();
        void start();

        double deltaTime() const;
        double totalTime() const;

        void limitFPS(int targetFPS);

    private:
        using Clock = std::chrono::high_resolution_clock;
        using TimePoint = std::chrono::time_point<Clock>;
        using Duration = std::chrono::duration<double>;

        TimePoint baseTime_;
        TimePoint prevTime_;
        TimePoint currTime_;
        TimePoint stopTime_;

        Duration deltaTime_;  // Time between frames
        Duration pausedTime_; // Time when game was on pause
        bool stopped_;
    };
}