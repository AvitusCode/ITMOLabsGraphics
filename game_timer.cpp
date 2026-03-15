#include "game_timer.hpp"

#include <Windows.h>

namespace jd
{
    GameTimer::GameTimer()
        : deltaTime_{ 0.0 }
    {
        reset();
    }

    void GameTimer::reset()
    {
        baseTime_ = Clock::now();
        prevTime_ = baseTime_;
        currTime_ = baseTime_;
        stopTime_ = TimePoint{};
        pausedTime_ = Duration::zero();
        stopped_ = false;
    }

    void GameTimer::tick()
    {
        if (stopped_) [[unlikely]]
        {
            deltaTime_ = Duration::zero();
            return;
        }

        currTime_ = Clock::now();
        deltaTime_ = currTime_ - prevTime_;
        prevTime_ = currTime_;

        if (deltaTime_.count() < 0.0) [[unlikely]] {
            deltaTime_ = Duration::zero();
        }
    }

    void GameTimer::stop()
    {
        if (!stopped_)
        {
            stopTime_ = Clock::now();
            stopped_ = true;
        }
    }

    void GameTimer::start()
    {
        if (stopped_)
        {
            pausedTime_ += Clock::now() - stopTime_;
            prevTime_ = Clock::now();
            stopped_ = false;
        }
    }

    double GameTimer::deltaTime() const
    {
        return deltaTime_.count();
    }

    double GameTimer::totalTime() const
    {
        if (!stopped_) [[likely]] {
            return std::chrono::duration<double>(Clock::now() - baseTime_ - pausedTime_).count();
        }
        else {
            return std::chrono::duration<double>(stopTime_ - baseTime_ - pausedTime_).count();
        }
    }

    void GameTimer::limitFPS(int targetFPS)
    {
        targetFPS <<= 1;
        static const double targetFrameTime = 1.0 / targetFPS;
        const double frameTime = std::chrono::duration<double>(Clock::now() - currTime_).count();

        if (frameTime < targetFrameTime)
        {
            DWORD sleepMs = static_cast<DWORD>((targetFrameTime - frameTime) * 1000);
            if (sleepMs > 0) {
                Sleep(sleepMs);
            }
        }
    }

}