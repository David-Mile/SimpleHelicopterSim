#pragma once

struct SharedState {
    float cyclic_x = 0.0f;
    float cyclic_y = 0.0f;

    float collective = 0.0f;
    float pedals = 0.0f;

    float pitch = 0.0f;
    float roll = 0.0f;
    float yaw = 0.0f;

    float pitch_rate = 0.0f;
    float roll_rate = 0.0f;
    float yaw_rate = 0.0f;

    float altitude = 0.0f;
    float vertical_speed = 0.0f;
};