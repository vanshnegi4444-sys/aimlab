#include "routine.h"

RoutineManager::RoutineManager() {
    // Block 1: Warmup (2 sets of Gridshot)
    for (int i = 1; i <= 2; ++i) {
        m_items.push_back({
            ScenarioId::Gridshot,
            i, 2, 1, 6,
            "Block 1: Warm-up",
            "Relaxed grip, smooth rhythmic flicks. Focus on clean hand flow, not rushing.",
            "Eliminate early-game whiff variance (stabilizes 8%-38% HS% swings)."
        });
    }

    // Block 2: Head Height Discipline (4 sets of Headshot Line)
    for (int i = 1; i <= 4; ++i) {
        m_items.push_back({
            ScenarioId::HeadshotLine,
            i, 4, 2, 6,
            "Block 2: Crosshair Placement",
            "Keep crosshair strictly at head height. Swipe horizontally only; no dipping low.",
            "Directly cures your 71.5% body hit rate in ranked."
        });
    }

    // Block 3: Counter-Strafing & Stop to Shoot (3 sets of CounterStrafeDrill)
    for (int i = 1; i <= 3; ++i) {
        m_items.push_back({
            ScenarioId::CounterStrafeDrill,
            i, 3, 3, 6,
            "Block 3: Counter-Strafing",
            "A/D strafe. When target pops, tap opposite key to full stop BEFORE clicking.",
            "Fixes your #1 leak: shooting while moving, causing negative DD Delta (-10)."
        });
    }

    // Block 4: First-Bullet Accuracy (3 sets of FirstShotPlacement)
    for (int i = 1; i <= 3; ++i) {
        m_items.push_back({
            ScenarioId::FirstShotPlacement,
            i, 3, 4, 6,
            "Block 4: First-Bullet Trade",
            "One bullet per target! Micro-adjust directly to center of head, then click.",
            "Wins first-contact Vandal trades and raises ADR from 134."
        });
    }

    // Block 5: Angle Holding & Reaction (2 sets of AngleHold + 2 sets of
    // Peeker's Advantage, exactly as the D3 daily routine specifies)
    for (int i = 1; i <= 2; ++i) {
        m_items.push_back({
            ScenarioId::AngleHold,
            i, 4, 5, 6,
            "Block 5: Angle Holding",
            "Hold crosshair slightly off the edge. Click the instant the bot peeks into crosshair.",
            "Master defensive duel timing so you aren't shot first."
        });
    }
    for (int i = 3; i <= 4; ++i) {
        m_items.push_back({
            ScenarioId::PeekersAdvantage,
            i, 4, 5, 6,
            "Block 5: Peeker's Advantage",
            "Swing out with A/D, stop, shoot before the bot's 200 ms reaction. Swing back to hide.",
            "Wins the timing of attacking duels: swing, stop, first bullet."
        });
    }

    // Block 6: Duel Tracking (2 sets of ValorantStrafe)
    for (int i = 1; i <= 2; ++i) {
        m_items.push_back({
            ScenarioId::ValorantStrafe,
            i, 2, 6, 6,
            "Block 6: Duel Tracking",
            "Stay locked on the AD-strafing bot's head. Smooth out panic mouse shakes.",
            "Wins close-quarters duels against agile opponents."
        });
    }
}

void RoutineManager::start() {
    m_active = true;
    m_currentIndex = 0;
}

void RoutineManager::stop() {
    m_active = false;
}

void RoutineManager::reset() {
    m_currentIndex = 0;
}

const RoutineItem& RoutineManager::currentItem() const {
    if (m_currentIndex >= 0 && m_currentIndex < static_cast<int>(m_items.size())) {
        return m_items[m_currentIndex];
    }
    static RoutineItem s_fallback{
        ScenarioId::Gridshot, 1, 1, 1, 1,
        "Custom Practice", "Focus on clean mechanics", "Personalized aim improvement"
    };
    return s_fallback;
}

bool RoutineManager::advance() {
    if (!m_active) return false;
    ++m_currentIndex;
    if (m_currentIndex >= static_cast<int>(m_items.size())) {
        m_active = false; // Finished routine!
        return false;
    }
    return true;
}

