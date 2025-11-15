#include "pomodoro.h"
#include <iostream>
#include <thread>
#include <chrono>

Pomodoro::Pomodoro(int work, int brk, int longBrk)
    : workDuration(work), breakDuration(brk), longBreakDuration(longBrk), cycleCount(0) {}

void Pomodoro::startWork() {
    std::cout << "Work session started for " << workDuration << " minutes." << std::endl;
    std::this_thread::sleep_for(std::chrono::minutes(workDuration));
    cycleCount++;
    std::cout << "Work session completed!" << std::endl;
}

void Pomodoro::startBreak() {
    std::cout << "Break started for " << breakDuration << " minutes." << std::endl;
    std::this_thread::sleep_for(std::chrono::minutes(breakDuration));
    std::cout << "Break finished!" << std::endl;
}

void Pomodoro::startLongBreak() {
    std::cout << "Long break started for " << longBreakDuration << " minutes." << std::endl;
    std::this_thread::sleep_for(std::chrono::minutes(longBreakDuration));
    std::cout << "Long break finished!" << std::endl;
}

int Pomodoro::getCycleCount() const {
    return cycleCount;
}
