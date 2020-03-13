//
// Created by Göksu Güvendiren on 2019-05-14.
//

#include "Scene.hpp"


void Scene::buildBVH() {
    printf(" - Generating BVH...\n\n");
    this->bvh = new BVHAccel(objects, 1, BVHAccel::SplitMethod::NAIVE);
}

Intersection Scene::intersect(const Ray &ray) const
{
    return this->bvh->Intersect(ray);
}

bool Scene::trace(
        const Ray &ray,
        const std::vector<Object*> &objects,
        float &tNear, uint32_t &index, Object **hitObject)
{
    *hitObject = nullptr;
    for (uint32_t k = 0; k < objects.size(); ++k) {
        float tNearK = kInfinity;
        uint32_t indexK;
        Vector2f uvK;
        if (objects[k]->intersect(ray, tNearK, indexK) && tNearK < tNear) {
            *hitObject = objects[k];
            tNear = tNearK;
            index = indexK;
        }
    }


    return (*hitObject != nullptr);
}

// Implementation of the Whitted-syle light transport algorithm (E [S*] (D|G) L)
//
// This function is the function that compute the color at the intersection point
// of a ray defined by a position and a direction. Note that thus function is recursive (it calls itself).
//
// If the material of the intersected object is either reflective or reflective and refractive,
// then we compute the reflection/refracton direction and cast two new rays into the scene
// by calling the castRay() function recursively. When the surface is transparent, we mix
// the reflection and refraction color using the result of the fresnel equations (it computes
// the amount of reflection and refractin depending on the surface normal, incident view direction
// and surface refractive index).
//
// If the surface is duffuse/glossy we use the Phong illumation model to compute the color
// at the intersection point.
// Vector3f Scene::align(const Vector3f& second, const Vector3f& first, const Vector3f& n) const
// {
//     Vector3f v = crossProduct(normalize(first), normalize(n));
//     float s = mag(v);
//     float c = dotProduct(normalize(first), normalize(n));
//     if(s<=0.0001)
//         return second;
//     Vector3f fiRow(-v[2]*v[2]-v[1]*v[1], v[1]*v[0], v[2]*v[0]);
//     Vector3f seRow(v[0]*v[1], -v[2]*v[2]-v[0]*v[0], v[2]*v[1]);
//     Vector3f thRow(v[0]*v[2], v[1]*v[2], -v[1]*v[1]-v[0]*v[0]);
//     fiRow = fiRow / (1+c);
//     seRow = seRow / (1+c);
//     thRow = thRow / (1+c);
//     fiRow[0] += 1, fiRow[1] -= v[2], fiRow[2] += v[1];
//     seRow[0] += v[2], seRow[1] += 1, seRow[2] -= v[0];
//     thRow[0] -= v[1], thRow[1] += v[0], thRow[2] += 1; 
    
//     return Vector3f(dotProduct(fiRow, second), dotProduct(seRow, second), dotProduct(thRow, second));
//     // return v;
    


// }

#pragma omp declare reduction(VectorPlus: Vector3f: \
        omp_out += omp_in)

Vector3f Scene::castar(const Ray &ray, int spp) const{
    Vector3f returned(0,0,0);

    

    

    #pragma omp parallel for reduction(VectorPlus:returned)
    for(int we = 0; we < spp; we++)
    {
        Ray copRay = ray;
        // std::cout << "ray: " << ray << std::endl;
        // std::cout << "copRay: " << copRay << std::endl;
        float Beta = 1;
        for(int i = 0;i < maxDepth;i++)
        {
            // if(i > 0)
                // std::cout << i << "th : " << Beta << std::endl;
            Intersection intersection = Scene::intersect(copRay);

            Material *m = intersection.m;
            Object *hitObject = intersection.obj;
            

            Vector2f uv;
            uint32_t index = 0;

            if(intersection.happened)
            {
                Vector3f hitPoint = intersection.coords;
                Vector3f N = intersection.normal;
                Vector2f st;
                float pdf = 0;
                // std::cout << Beta << std::endl;

                //get Normal
                hitObject->getSurfaceProperties(hitPoint, copRay.direction, index, uv, N, st);

                //initialize wi
                Ray wi(hitPoint, Vector3f(0,0,0));


                Vector3f shadowPointOrig = (dotProduct(copRay.direction, N) < 0) ?
                                                hitPoint + N * EPSILON :
                                                hitPoint - N * EPSILON ;
                        
                Vector3f reN = (dotProduct(copRay.direction, N) < 0) ?
                                        N:
                                        -N;


                switch(m->getType())
                {
                    case DIFFUSE_AND_GLOSSY:
                    {
                        
                        
                                        

                        Vector3f lightAmt(0,0,0);
                        Vector3f specularColor(0,0,0);
                        for (uint32_t i = 0; i < get_lights().size(); ++i)
                        {
                            auto area_ptr = dynamic_cast<AreaLight*>(this->get_lights()[i].get());
                            if (area_ptr)
                            {
                                // Do nothing for this assignment
                            }
                            else
                            {
                                Vector3f lightDir = get_lights()[i]->position - hitPoint;
                                // square of the distance between hitPoint and the light
                                // float lightDistance2 = dotProduct(lightDir, lightDir);
                                lightDir = normalize(lightDir);
                                // float LdotN = std::max(0.f, dotProduct(lightDir, N));
                                float LdotN = AbsDot(normalize(lightDir), normalize(N));
                                Object *shadowHitObject = nullptr;
                                // float tNearShadow = kInfinity;
                                // is the point in shadow, and is the nearest occluding object closer to the object than the light itself?
                                bool inShadow = bvh->Intersect(Ray(shadowPointOrig, lightDir)).happened;
                                lightAmt += (1 - inShadow) * get_lights()[i]->intensity * LdotN;
                                // Vector3f reflectionDirection = reflect(-lightDir, N);
                                // float cool = AbsDot(normalize(-copRay.direction), normalize(reflectionDirection));
                                // specularColor += powf(cool,m->specularExponent) * get_lights()[i]->intensity;
                            }
                        }

                        
                        Vector3f seeing = Beta * (lightAmt * (hitObject->evalDiffuseColor(st) * m->Kd + specularColor * m->Ks));
                        // if(seeing[0] > 1)
                        // std::cout <<seeing << std::endl;
                        
                        returned += seeing;
                        // std::cout << i << " th: " << returned << std::endl;

                        wi.origin = shadowPointOrig;

                        
                        float f = m->Sample_f(Ray(copRay.origin, -copRay.direction), wi, pdf, reN);
                        // if(dotProduct(reN, wi.direction) < 0 || dotProduct(reN, -copRay.direction) < 0)
                            // std::cout << "crap" <<std::endl;

                        copRay = Ray(shadowPointOrig, wi.direction);
                        // std::cout << copRay.origin << " " << shadowPointOrig << std::endl;

                        Beta *= f * AbsDot(normalize(wi.direction), N) / pdf;
                        // std::cout << i << " " << Beta << std::endl;
                        break;

                        

                    }

                    case REFLECTION:
                    {
                        wi.origin = shadowPointOrig;
                        wi.direction = reflect(copRay.direction, reN);
                        copRay = Ray(shadowPointOrig, wi.direction);
                        // Beta *= AbsDot(normalize(wi.direction), N);
                        break;
                    }
                }

            }
            else
            {
                if(i==0)
                    returned += Beta * backgroundColor;
                
                break;
            }
            
        }

        


    }

    returned = returned / spp;

    // std::cout << returned;




    

    return returned;
}


Vector3f Scene::castRay(const Ray &ray, int depth) const
{
    if (depth > this->maxDepth) {
        return Vector3f(0.0,0.0,0.0);
    }
    Intersection intersection = Scene::intersect(ray);
    Material *m = intersection.m;
    Object *hitObject = intersection.obj;
    Vector3f hitColor = this->backgroundColor;
//    float tnear = kInfinity;
    Vector2f uv;
    uint32_t index = 0;
    if(intersection.happened) {

        Vector3f hitPoint = intersection.coords;
        Vector3f N = intersection.normal; // normal
        Vector2f st; // st coordinates
        hitObject->getSurfaceProperties(hitPoint, ray.direction, index, uv, N, st);
//        Vector3f tmp = hitPoint;
        switch (m->getType()) {
            case REFLECTION_AND_REFRACTION:
            {
                Vector3f reflectionDirection = normalize(reflect(ray.direction, N));
                Vector3f refractionDirection = normalize(refract(ray.direction, N, m->ior));
                Vector3f reflectionRayOrig = (dotProduct(reflectionDirection, N) < 0) ?
                                             hitPoint - N * EPSILON :
                                             hitPoint + N * EPSILON;
                Vector3f refractionRayOrig = (dotProduct(refractionDirection, N) < 0) ?
                                             hitPoint - N * EPSILON :
                                             hitPoint + N * EPSILON;
                Vector3f reflectionColor = castRay(Ray(reflectionRayOrig, reflectionDirection), depth + 1);
                Vector3f refractionColor = castRay(Ray(refractionRayOrig, refractionDirection), depth + 1);
                float kr;
                fresnel(ray.direction, N, m->ior, kr);
                hitColor = reflectionColor * kr + refractionColor * (1 - kr);
                break;
            }
            case REFLECTION:
            {
                float kr;
                fresnel(ray.direction, N, m->ior, kr);
                Vector3f reflectionDirection = reflect(ray.direction, N);
                Vector3f reflectionRayOrig = (dotProduct(reflectionDirection, N) < 0) ?
                                             hitPoint - N * EPSILON :
                                             hitPoint + N * EPSILON;
                hitColor = castRay(Ray(reflectionRayOrig, reflectionDirection),depth + 1) * kr;
                // if(depth == 0)
                    // std::cout << hitColor << std::endl;
                break;
            }
            default:
            {
                // std::cout << "defar" << std::endl;
                // [comment]
                // We use the Phong illumation model int the default case. The phong model
                // is composed of a diffuse and a specular reflection component.
                // [/comment]
                Vector3f lightAmt = 0, specularColor = 0;
                Vector3f shadowPointOrig = (dotProduct(ray.direction, N) < 0) ?
                                           hitPoint + N * EPSILON :
                                           hitPoint - N * EPSILON;
                // [comment]
                // Loop over all lights in the scene and sum their contribution up
                // We also apply the lambert cosine law
                // [/comment]
                for (uint32_t i = 0; i < get_lights().size(); ++i)
                {
                    auto area_ptr = dynamic_cast<AreaLight*>(this->get_lights()[i].get());
                    if (area_ptr)
                    {
                        // Do nothing for this assignment
                    }
                    else
                    {
                        Vector3f lightDir = get_lights()[i]->position - hitPoint;
                        // square of the distance between hitPoint and the light
                        float lightDistance2 = dotProduct(lightDir, lightDir);
                        lightDir = normalize(lightDir);
                        // float LdotN = std::max(0.f, dotProduct(lightDir, N));
                        float LdotN = AbsDot(lightDir, N);
                        Object *shadowHitObject = nullptr;
                        // float tNearShadow = kInfinity;
                        // is the point in shadow, and is the nearest occluding object closer to the object than the light itself?
                        bool inShadow = bvh->Intersect(Ray(shadowPointOrig, lightDir)).happened;
                        lightAmt += (1 - inShadow) * get_lights()[i]->intensity * LdotN;
                        Vector3f reflectionDirection = reflect(-lightDir, N);
                        float cool = AbsDot(normalize(-ray.direction), normalize(reflectionDirection));
                        specularColor += powf(cool,m->specularExponent) * get_lights()[i]->intensity;
                    }
                }
                hitColor = lightAmt * (hitObject->evalDiffuseColor(st) * m->Kd + specularColor * m->Ks);
                // if(depth == 0)
                //     std::cout << hitColor << std::endl;
                break;
            }
        }
    }
    

    return hitColor;
}