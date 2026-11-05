#include "player.h"
#include <cmath>
#include <algorithm>

Player::Player() {
    pos = { 0.0f, 1.0f, 0.0f };
    vel = { 0.0f, 0.0f, 0.0f };
    currentEyeHeight = kEyeHeight;
    isGrounded = true;
}

void Player::spawn(Vec3 footPos, float /*yawDeg*/) {
    pos = footPos;
    vel = { 0.0f, 0.0f, 0.0f };
    currentEyeHeight = kEyeHeight;
    isGrounded = true;
}

void Player::update(double dt, const PlayerInput& input, const Camera& cam, const MapMesh* map) {
    float dtF = static_cast<float>(dt);
    if (dtF <= 0.0f || dtF > 0.1f) dtF = 0.016f; // clamp delta time for stability

    // Crouch transition
    isCrouched = input.crouch;
    isWalking  = input.walk;

    float targetEye = isCrouched ? kCrouchEye : kEyeHeight;
    currentEyeHeight += (targetEye - currentEyeHeight) * std::min(1.0f, dtF * 14.0f);

    // Speed target
    float maxSpd = kRunSpeed;
    if (isCrouched) maxSpd = kCrouchSpeed;
    else if (isWalking) maxSpd = kWalkSpeed;

    // Movement direction from camera yaw
    float radYaw = radians(cam.yawDeg);
    Vec3 fwd   = {  std::sin(radYaw), 0.0f, -std::cos(radYaw) };
    Vec3 right = {  std::cos(radYaw), 0.0f,  std::sin(radYaw) };

    Vec3 moveWish = { 0.0f, 0.0f, 0.0f };
    if (input.forward)  moveWish = moveWish + fwd;
    if (input.backward) moveWish = moveWish - fwd;
    if (input.right)    moveWish = moveWish + right;
    if (input.left)     moveWish = moveWish - right;

    float wishLen = length(moveWish);
    if (wishLen > 1e-4f) {
        moveWish = moveWish * (1.0f / wishLen);
        Vec3 targetVel = moveWish * maxSpd;
        // Fast Valorant acceleration (instant counter-strafe feel)
        float accel = isGrounded ? 55.0f : 18.0f;
        vel.x += (targetVel.x - vel.x) * std::min(1.0f, dtF * accel);
        vel.z += (targetVel.z - vel.z) * std::min(1.0f, dtF * accel);
    } else {
        // Snappy counter-strafe braking / friction
        float friction = isGrounded ? 48.0f : 6.0f;
        vel.x -= vel.x * std::min(1.0f, dtF * friction);
        vel.z -= vel.z * std::min(1.0f, dtF * friction);
        if (std::abs(vel.x) < 0.05f) vel.x = 0.0f;
        if (std::abs(vel.z) < 0.05f) vel.z = 0.0f;
    }

    // Jump
    if (isGrounded && input.jump && !isCrouched) {
        vel.y = kJumpImpulse;
        isGrounded = false;
    }

    // Apply gravity
    if (!isGrounded) {
        vel.y -= kGravity * dtF;
    }

    // Horizontal movement test with floor + wall collision
    Vec3 newPos = pos;
    newPos.x += vel.x * dtF;
    newPos.z += vel.z * dtF;

    if (map && map->isLoaded()) {
        // Resolve cylinder collision against vertical walls first
        map->resolveWallCollision(newPos.x, newPos.z, pos.y, 0.35f);

        float floorY = 0.0f;
        if (map->getFloorHeight(newPos.x, newPos.z, pos.y, floorY)) {
            pos.x = newPos.x;
            pos.z = newPos.z;
            if (isGrounded) {
                pos.y += (floorY - pos.y) * std::min(1.0f, dtF * 25.0f);
            }
        } else {
            // Slide along X or Z axis
            float fx = 0.0f, fz = 0.0f;
            if (map->getFloorHeight(newPos.x, pos.z, pos.y, fx)) {
                pos.x = newPos.x;
                vel.z = 0.0f;
                if (isGrounded) pos.y += (fx - pos.y) * std::min(1.0f, dtF * 25.0f);
            } else if (map->getFloorHeight(pos.x, newPos.z, pos.y, fz)) {
                pos.z = newPos.z;
                vel.x = 0.0f;
                if (isGrounded) pos.y += (fz - pos.y) * std::min(1.0f, dtF * 25.0f);
            } else {
                // Safety fallback: if current position itself is off-mesh, allow free movement to re-enter
                float curFloor = 0.0f;
                if (!map->getFloorHeight(pos.x, pos.z, pos.y, curFloor)) {
                    pos.x = newPos.x;
                    pos.z = newPos.z;
                } else {
                    vel.x = 0.0f;
                    vel.z = 0.0f;
                }
            }
        }

        // Vertical collision (landing from jump or step-down)
        if (!isGrounded) {
            pos.y += vel.y * dtF;
            float groundY = 0.0f;
            if (map->getFloorHeight(pos.x, pos.z, pos.y, groundY)) {
                if (pos.y <= groundY + 0.05f && vel.y <= 0.0f) {
                    pos.y = groundY;
                    vel.y = 0.0f;
                    isGrounded = true;
                }
            }
        }
    } else {
        // Flat floor fallback at y = 0
        pos.x = newPos.x;
        pos.z = newPos.z;
        pos.y += vel.y * dtF;
        if (pos.y <= 0.0f) {
            pos.y = 0.0f;
            vel.y = 0.0f;
            isGrounded = true;
        }
    }
}
