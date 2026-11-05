#pragma once
#include "scenarios.h"
#include <vector>

struct RoutineItem {
    ScenarioId scenario;
    int setIndex;       // 1-based (e.g. 1 of 4)
    int totalSetsInBlock; // e.g. 4
    int blockNumber;    // 1-based (e.g. 1 of 6)
    int totalBlocks;    // 6
    const char* blockName;
    const char* focusCue;
    const char* trackerReason;
};

class RoutineManager {
public:
    RoutineManager();

    void start();
    void stop();
    bool isActive() const { return m_active; }

    int currentStepIndex() const { return m_currentIndex; }
    int totalSteps() const { return static_cast<int>(m_items.size()); }

    const RoutineItem& currentItem() const;

    // Advances to next set in the routine. Returns false if routine is completed.
    bool advance();

    // Reset current step to beginning
    void reset();

private:
    bool m_active = false;
    int m_currentIndex = 0;
    std::vector<RoutineItem> m_items;
};

