//
// Created by goksu on 2/25/20.
//

#include <fstream>
#include "Scene.hpp"
#include "Renderer.hpp"


inline float deg2rad(const float& deg) { return deg * M_PI / 180.0; }

const float EPSILON = 0.00001;

// The main render function. This where we iterate over all pixels in the image,
// generate primary rays and cast these rays into the scene. The content of the
// framebuffer is saved to a file.
void Renderer::Render(const Scene& scene)
{
    Vector3f *framebuffer = new Vector3f[scene.width * scene.height];
    Vector3f *pix = framebuffer;
    float scale = tan(deg2rad(scene.fov * 0.5));
    float imageAspectRatio = scene.width / (float)scene.height;
    Vector3f orig(-5, 5, 10);
    for (uint32_t j = 0; j < scene.height; ++j) {
        for (uint32_t i = 0; i < scene.width; ++i) {
            //generate primary ray direction
            float x;
            float y;

            x = (static_cast<float>(i) - scene.width/2);
            y = -(static_cast<float>(j) - scene.height/2);

            x *= 2*scale/scene.width;
            y *= 2*scale/scene.height;
            x *= imageAspectRatio;



            Vector3f dir = Vector3f(x, y, -1); // Don't forget to normalize this direction!
            dir = normalize(dir);
            *(pix++) = scene.castRay(Ray(orig, dir), 0);
        }
        UpdateProgress(j / (float)scene.height);
    }

    // save framebuffer to file
    std::ofstream ofs;
    ofs.open("./out.ppm");
    ofs << "P6\n" << scene.width << " " << scene.height << "\n255\n";
    for (uint32_t i = 0; i < scene.height * scene.width; ++i) {
        char r = (char)(255 * clamp(0, 1, framebuffer[i].x));
        char g = (char)(255 * clamp(0, 1, framebuffer[i].y));
        char b = (char)(255 * clamp(0, 1, framebuffer[i].z));
        ofs << r << g << b;
    }

    ofs.close();

    delete [] framebuffer;
}
