#pragma once

class PID {
public:

    float P, I, D;

    float e_accum = 0.0f; 
    float e_back = 0.0f; 

    PID(float p_gain, float i_gain, float d_gain) : P(p_gain), I(i_gain), D(d_gain) {} 

    float calculate_error(float e, float dt) {

        //TODO Make the bounds 
        float return_e = P * e + I * e_accum + D * (e - e_back) / dt; 
        e_accum += e * dt; 
        e_back = e; 

        return return_e; 
    }
};