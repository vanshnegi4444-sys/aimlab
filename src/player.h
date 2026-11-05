#pragma once
#include "camera.h"
#include "map_mesh.h"

struct PlayerInput {
    bool forward  = false;
    bool backward = false;
    bool left     = false;
    bool right    = false;
    bool jump     = false;
    bool crouch   = false;
    bool walk     = false;
};

class Player {
public:
    static constexpr float kRunSpeed     = 5.4f;   // Valorant run speed: 5.4 m/s
    static constexpr float kWalkSpeed    = 3.0f;   // Shift walk: 3.0 m/s
    static constexpr float kCrouchSpeed  = 1.5f;   // Ctrl crouch: 1.5 m/s
    static constexpr float kJumpImpulse  = 4.8f;   // Valorant jump impulse
    static constexpr float kGravity      = 18.0f;  // Snappy FPS gravity
    static constexpr float kDeadzoneVel  = 1.85f;  // Accuracy deadzone (< 34% max run speed)
    static constexpr float kEyeHeight    = 1.62f;  // Valorant standing eye height
    static constexpr float kCrouchEye    = 1.05f;  // Valorant crouching eye height

    Player();

    void spawn(Vec3 footPos, float yawDeg);
    void update(double dt, const PlayerInput& input, const Camera& cam, const MapMesh* map);

    Vec3 eyePos() const {
        return { pos.x, pos.y + currentEyeHeight, pos.z };
    }

    float speed() const {
        return std::sqrt(vel.x * vel.x + vel.z * vel.z);
    }

    bool isAccurate() const {
        return speed() < kDeadzoneVel && isGrounded;
    }

    Vec3  pos{};              // foot position
    Vec3  vel{};              // world velocity
    float currentEyeHeight = kEyeHeight;
    bool  isGrounded       = true;
    bool  isCrouched       = false;
    bool  isWalking        = false;
};

