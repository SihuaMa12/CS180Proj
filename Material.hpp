//
// Created by LEI XU on 5/16/19.
//

#ifndef RAYTRACING_MATERIAL_H
#define RAYTRACING_MATERIAL_H

#include "Vector.hpp"
#include <random>
#include <cmath>

enum MaterialType { DIFFUSE_AND_GLOSSY, REFLECTION_AND_REFRACTION, REFLECTION };



class Material{
public:
    MaterialType m_type;
    Vector3f m_color;
    Vector3f m_emission;
    float ior;
    float Kd, Ks;
    float specularExponent;
    //Texture tex;

    inline Material(MaterialType t=DIFFUSE_AND_GLOSSY, Vector3f c=Vector3f(1,1,1), Vector3f e=Vector3f(0,0,0));
    inline MaterialType getType();
    inline float Sample_f(const Ray& wo, Ray& wi, float& pdf, const Vector3f& normal);
    inline Vector3f getColor();
    inline Vector3f getColorAt(double u, double v);
    inline Vector3f getEmission();


};

inline Vector3f align(const Vector3f& second, const Vector3f& first, const Vector3f& n)
{
    Vector3f v = crossProduct(normalize(first), normalize(n));
    float s = mag(v);
    float c = dotProduct(normalize(first), normalize(n));
    if(s<=0.0001)
        return second;
    Vector3f fiRow(-v[2]*v[2]-v[1]*v[1], v[1]*v[0], v[2]*v[0]);
    Vector3f seRow(v[0]*v[1], -v[2]*v[2]-v[0]*v[0], v[2]*v[1]);
    Vector3f thRow(v[0]*v[2], v[1]*v[2], -v[1]*v[1]-v[0]*v[0]);
    fiRow = fiRow / (1+c);
    seRow = seRow / (1+c);
    thRow = thRow / (1+c);
    fiRow[0] += 1, fiRow[1] -= v[2], fiRow[2] += v[1];
    seRow[0] += v[2], seRow[1] += 1, seRow[2] -= v[0];
    thRow[0] -= v[1], thRow[1] += v[0], thRow[2] += 1; 
    
    return Vector3f(dotProduct(fiRow, second), dotProduct(seRow, second), dotProduct(thRow, second));
}


inline float Material::Sample_f(const Ray& wo, Ray& wi, float& pdf,const Vector3f& normal)
{
    // std::cout << "called " << std::endl;
    // Vector3f dir = -wo.direction;
    //should later switch to sampler
    std::default_random_engine re(std::random_device{}());
    std::uniform_real_distribution<float> dist(0.0, 1.0);
    float store1 = 0, store2 = 0;
    float phi = 0;
    float theta = 0;
    // std::cout << m_type <<std::endl;
    switch(m_type)
    {
        case DIFFUSE_AND_GLOSSY:
        {
            // std::cout << "here" << std::endl;
            store1 = dist(re);
            store2 = dist(re);
            theta = std::asin(store1);
            phi = 2 * M_PI * store2;
            Vector3f newone(0,0,0);
            newone[0] = std::sin(theta) * std::cos(phi);
            newone[1] = std::sin(theta) * std::sin(phi);
            newone[2] = std::cos(theta);
            // std::cout << newone << std::endl;
            wi.direction = align(normalize(newone), Vector3f(0,0,1), normal);
            // if(dotProduct(normal, wi.direction) < 0)
                // std::cout << "Found" << std::endl;
            pdf = std::cos(theta) / M_PI;
            // std::cout << "test: " << dotProduct(normal, wi.direction) << std::endl;
            // std::cout << "before: " << wo.direction << " after: " << wi.direction << std::endl;
            return  Kd / M_PI;
            break;

        }
    }
}

Material::Material(MaterialType t, Vector3f c, Vector3f e){
    m_type = t;
    m_color = c;
    m_emission = e;
    specularExponent = 10;
    Kd = 0.6;
    Ks = 0;
    ior = 2;
}

MaterialType Material::getType(){return m_type;}
Vector3f Material::getColor(){return m_color;}
Vector3f Material::getEmission() {return m_emission;}

Vector3f Material::getColorAt(double u, double v) {
    return Vector3f();
}
#endif //RAYTRACING_MATERIAL_H
