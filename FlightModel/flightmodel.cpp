// FlightModel.cpp : Defines the exported functions for the DLL.

#include "pch.h"
#include "framework.h"
#include "flightmodel.h"

FLIGHTMODEL_API int nFlightModel = 0;

FLIGHTMODEL_API void update_model(SharedState* s, float dt)
{
    // -------------------------------------------------------------------------
    // CICLICO — pitch & roll
    // -------------------------------------------------------------------------
    const float cyclic_gain = 120.0f;
    const float rate_damping = 8.0f;
    const float angle_spring = 6.0f;
    const float angle_damping = 6.0f;

    s->pitch_rate += s->cyclic_y * cyclic_gain * dt;
    s->roll_rate += s->cyclic_x * cyclic_gain * dt;

    s->pitch_rate -= s->pitch_rate * rate_damping * dt;
    s->roll_rate -= s->roll_rate * rate_damping * dt;

    s->pitch_rate -= (s->pitch * angle_spring + s->pitch_rate * angle_damping) * dt;
    s->roll_rate -= (s->roll * angle_spring + s->roll_rate * angle_damping) * dt;

    s->pitch += s->pitch_rate * dt;
    s->roll += s->roll_rate * dt;

    // -------------------------------------------------------------------------
    // COLLECTIVE — direct vertical speed
    // -------------------------------------------------------------------------
    //   collective = -1 → vspeed target = -5 m/s (descend)
    //   collective =  0 → vspeed target =  0     (neutral hovering)
    //   collective = +1 → vspeed target = +5 m/s (ascend)
    // The widget now sends directly -1..+1, zero = center slider.

    const float vspeed_max = 5.0f;
    const float vspeed_lag = 3.0f;

    float vspeed_target = s->collective * vspeed_max; 
    s->vertical_speed += (vspeed_target - s->vertical_speed) * vspeed_lag * dt;

    s->altitude += s->vertical_speed * dt;

    if (s->altitude <= 0.0f) {
        s->altitude = 0.0f;
        if (s->vertical_speed < 0.0f)
            s->vertical_speed = 0.0f;
    }

    // -------------------------------------------------------------------------
    // PEDALS — yaw rate
    // -------------------------------------------------------------------------
    const float pedal_yaw_rate = 60.0f;
    const float yaw_damping = 4.0f;

    float yaw_rate_target = s->pedals * pedal_yaw_rate;
    s->yaw_rate += (yaw_rate_target - s->yaw_rate) * yaw_damping * dt;
    s->yaw += s->yaw_rate * dt;
}

CFlightModel::CFlightModel()
{
    return;
}

