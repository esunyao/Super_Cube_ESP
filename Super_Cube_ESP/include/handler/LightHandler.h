//
// Created by Esuny on 2024/10/13.
//

#ifndef SUPER_CUBE_ESP_LIGHTHANDLER_H
#define SUPER_CUBE_ESP_LIGHTHANDLER_H

#include "super_cube.h"

class super_cube;
class LightHandler {

public:
    LightHandler(super_cube *superCube_);

    void lightInitiation();

private:
    super_cube *superCube;
};


#endif //SUPER_CUBE_ESP_LIGHTHANDLER_H
