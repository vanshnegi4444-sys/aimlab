#pragma once
#include <cmath>

// WHY no glm here: we need exactly two small matrices (view + projection).
// Hand-rolling them is ~60 lines, keeps the dependency list tiny, and —
// more importantly for a learner — every number in the aim pipeline stays
// visible instead of hiding inside a library call.

// All matrices are float[16] in COLUMN-MAJOR order, which is what OpenGL
// expects from glUniformMatrix4fv (its `transpose` param must be GL_FALSE in
// a core profile, so we lay the memory out correctly ourselves).

constexpr float kPi = 3.14159265358979323846f;

inline float radians(float deg) { return deg * kPi / 180.0f; }
inline float degrees(float rad) { return rad * 180.0f / kPi; }

struct Vec3 {
    float x = 0, y = 0, z = 0;
};

inline Vec3 operator+(Vec3 a, Vec3 b) { return { a.x + b.x, a.y + b.y, a.z + b.z }; }
inline Vec3 operator-(Vec3 a, Vec3 b) { return { a.x - b.x, a.y - b.y, a.z - b.z }; }
inline Vec3 operator*(Vec3 v, float s) { return { v.x * s, v.y * s, v.z * s }; }

inline float dot(Vec3 a, Vec3 b) { return a.x * b.x + a.y * b.y + a.z * b.z; }

inline Vec3 cross(Vec3 a, Vec3 b) {
    return { a.y * b.z - a.z * b.y,
             a.z * b.x - a.x * b.z,
             a.x * b.y - a.y * b.x };
}

inline float length(Vec3 v) { return std::sqrt(dot(v, v)); }

inline Vec3 normalize(Vec3 v) {
    float l = length(v);
    return l > 0.0f ? v * (1.0f / l) : Vec3{};
}

struct Mat4 {
    float m[16] = {}; // column-major: m[col*4 + row]
};

inline Mat4 mat4Identity() {
    Mat4 r{};
    r.m[0] = r.m[5] = r.m[10] = r.m[15] = 1.0f;
    return r;
}

// Standard perspective projection. fovYdeg is the VERTICAL field of view.
inline Mat4 mat4Perspective(float fovYdeg, float aspect, float zNear, float zFar) {
    float f = 1.0f / std::tan(radians(fovYdeg) * 0.5f);
    Mat4 r{};
    r.m[0]  = f / aspect;
    r.m[5]  = f;
    r.m[10] = (zFar + zNear) / (zNear - zFar);
    r.m[11] = -1.0f;
    r.m[14] = (2.0f * zFar * zNear) / (zNear - zFar);
    // r.m[15] stays 0 — that is what makes it a projection, not an affine map.
    return r;
}

// Camera looks from `eye` toward `center`, with `up` as the world's up.
inline Mat4 mat4LookAt(Vec3 eye, Vec3 center, Vec3 up) {
    Vec3 f = normalize(center - eye); // forward
    Vec3 s = normalize(cross(f, up)); // right  (perpendicular to both)
    Vec3 u = cross(s, f);             // corrected up
    Mat4 r{};
    r.m[0] = s.x;  r.m[1] = u.x;  r.m[2] = -f.x;  r.m[3] = 0.0f;
    r.m[4] = s.y;  r.m[5] = u.y;  r.m[6] = -f.y;  r.m[7] = 0.0f;
    r.m[8] = s.z;  r.m[9] = u.z;  r.m[10] = -f.z; r.m[11] = 0.0f;
    r.m[12] = -dot(s, eye);
    r.m[13] = -dot(u, eye);
    r.m[14] =  dot(f, eye);
    r.m[15] = 1.0f;
    return r;
}

inline Mat4 mat4Translate(Vec3 t) {
    Mat4 r = mat4Identity();
    r.m[12] = t.x;
    r.m[13] = t.y;
    r.m[14] = t.z;
    return r;
}

inline Mat4 mat4RotateY(float deg) {
    Mat4 r = mat4Identity();
    float rad = radians(deg);
    float c = std::cos(rad);
    float s = std::sin(rad);
    r.m[0]  = c;
    r.m[2]  = -s;
    r.m[8]  = s;
    r.m[10] = c;
    return r;
}

inline Mat4 mat4Scale(Vec3 s) {
    Mat4 r = mat4Identity();
    r.m[0]  = s.x;
    r.m[5]  = s.y;
    r.m[10] = s.z;
    return r;
}

// Returns a*b (apply b first, then a). Used as proj * view.
inline Mat4 mat4Mul(const Mat4& a, const Mat4& b) {
    Mat4 r{};
    for (int c = 0; c < 4; ++c)
        for (int row = 0; row < 4; ++row) {
            float sum = 0.0f;
            for (int k = 0; k < 4; ++k)
                sum += a.m[k * 4 + row] * b.m[c * 4 + k];
            r.m[c * 4 + row] = sum;
        }
    return r;
}

// ---------------------------------------------------------------------------
// The camera: yaw/pitch angles driven DIRECTLY by raw mouse counts.
// This is the whole "Valorant feel" of the program, so read it slowly.
// ---------------------------------------------------------------------------
struct Camera {
    Vec3  pos      = { 0.0f, 0.0f, 0.0f }; // camera position (for movement drills)
    float yawDeg   = 0.0f; // 0   = looking down -Z (at the wall)
    float pitchDeg = 0.0f; // +   = looking up

    // Valorant's measured constant: ONE raw mouse count rotates the view by
    // 0.07 degrees when sensitivity is 1.0. (Community-measured from the real
    // game: sens 1.0 needs ~5143 counts for a 360, and 360/5143 = 0.07.)
    // So:  yaw += dx * sens * 0.07.  Same sens number => same cm/360 as Valorant.
    static constexpr float kDegPerCount = 0.07f;

    void applyMouse(int dx, int dy, float sens) {
        yawDeg += dx * sens * kDegPerCount;
        // WHY the minus sign: Windows raw input reports UPWARD motion as
        // NEGATIVE dy. Looking up must be POSITIVE pitch, hence -dy.
        pitchDeg += -dy * sens * kDegPerCount;

        // WHY clamp at +-89 and not +-90: at exactly 90 the forward vector
        // becomes parallel to the up vector and the view matrix degenerates
        // (division by ~zero inside lookAt -> NaN view -> black screen).
        if (pitchDeg >  89.0f) pitchDeg =  89.0f;
        if (pitchDeg < -89.0f) pitchDeg = -89.0f;

        // Keep yaw bounded so float precision never degrades in long sessions.
        if (yawDeg >  180.0f) yawDeg -= 360.0f;
        if (yawDeg < -180.0f) yawDeg += 360.0f;
    }

    Vec3 forward() const {
        float y = radians(yawDeg);
        float p = radians(pitchDeg);
        float cp = std::cos(p);
        return { std::sin(y) * cp, std::sin(p), -std::cos(y) * cp };
    }

    Vec3 right() const {
        Vec3 f = forward();
        return normalize(cross(f, { 0.0f, 1.0f, 0.0f }));
    }

    Vec3 eye() const { return pos; }

    Mat4 viewMatrix() const {
        return mat4LookAt(eye(), eye() + forward(), { 0.0f, 1.0f, 0.0f });
    }

    // Valorant locks the VERTICAL FOV and derives horizontal from the aspect
    // ratio. At 16:9 Valorant's horizontal FOV is 103 degrees, so:
    //     vFov = 2 * atan( tan(103deg/2) * 9/16 )  ≈  70.53 degrees
    // WHY this matters: sensitivity is specified in degrees-per-count, so the
    // view must subtend the same degrees-per-pixel as Valorant or the sens
    // numbers would not transfer. Computed at runtime from the definition
    // above instead of hardcoding 70.53, so the derivation stays visible.
    static float verticalFovDeg() {
        static const float v = degrees(2.0f * std::atan(std::tan(radians(103.0f) * 0.5f) * (9.0f / 16.0f)));
        return v; // ≈ 70.53
    }

    Mat4 projMatrix(float aspect) const {
        return mat4Perspective(verticalFovDeg(), aspect, 0.1f, 200.0f);
    }
};
