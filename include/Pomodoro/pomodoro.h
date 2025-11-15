#ifndef POMODORO_H
#define POMODORO_H

class Pomodoro {
private:
    int workDuration;       // minutes
    int breakDuration;      // minutes
    int longBreakDuration;  // minutes
    int cycleCount;

public:
    Pomodoro(int work = 25, int brk = 5, int longBrk = 15);

    void startWork();
    void startBreak();
    void startLongBreak();

    int getCycleCount() const;
};

#endif // POMODORO_H
