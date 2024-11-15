//
// Created by Esuny on 2024/11/16.
//

#ifndef SUPER_CUBE_ESP_ATTITUDESERVICE_H
#define SUPER_CUBE_ESP_ATTITUDESERVICE_H
#include <super_cube.h>
#include "utils/MPU6050.h"

class AttitudeService {
public:
    AttitudeService(super_cube *cube);
    void setup();
    void update();
    void getMotion6(int16_t* ax, int16_t* ay, int16_t* az, int16_t* gx, int16_t* gy, int16_t* gz);

private:
    super_cube *superCube;
    MPU6050 accelgyro;
};

#endif //SUPER_CUBE_ESP_ATTITUDESERVICE_H
