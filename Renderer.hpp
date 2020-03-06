//
// Created by goksu on 2/25/20.
//
//#include <eigen3/Eigen/Eigen>
#include <optional>
#include "Scene.hpp"

#pragma once
struct hit_payload
{
    float tNear;
    uint32_t index;
    Vector2f uv;
    Object* hit_obj;
};

class Renderer
{
public:
    void Render(const Scene& scene, int sp);

private:
    int spp;
};
