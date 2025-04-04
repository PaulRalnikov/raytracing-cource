#include <iostream>
#include <fstream>
#include <cassert>
#include <vector>
#include "glm/vec3.hpp"
#include "glm/vec4.hpp"
#include "glm/gtx/quaternion.hpp"
#include "source/primitive.hpp"
#include "source/scene.hpp"
#include <algorithm>

glm::vec3 saturate(glm::vec3 const &color)
{
    return glm::clamp(color, glm::vec3(0.0), glm::vec3(1.0));
}
glm::vec3 aces_tonemap(glm::vec3 const &x)
{
    const float a = 2.51f;
    const float b = 0.03f;
    const float c = 2.43f;
    const float d = 0.59f;
    const float e = 0.14f;
    return saturate((x * (a * x + b)) / (x * (c * x + d) + e));
}

unsigned char color_to_byte(float color) {
    return std::round(std::clamp(color * 255, (float)0.0, (float)255.0));
}

glm::vec3 gamma_correction(glm::vec3 color) {
    float exp = 1 / 2.2;
    return glm::vec3(glm::pow(color.x, exp), glm::pow(color.y, exp), glm::pow(color.z, exp));
}

void write_to_output(std::string output_path, std::vector<glm::vec3> pixels, const Scene& scene) {
    std::vector<unsigned char> real_pixels(pixels.size() * 3);

    for (size_t i = 0; i < pixels.size(); i++) {
        pixels[i] = aces_tonemap(pixels[i]);
        pixels[i] = gamma_correction(pixels[i]);

        real_pixels[i * 3 + 0] = color_to_byte(pixels[i].x);
        real_pixels[i * 3 + 1] = color_to_byte(pixels[i].y);
        real_pixels[i * 3 + 2] = color_to_byte(pixels[i].z);
    }

    std::ofstream out;

    out.open(output_path);
    out << "P6\n";
    out << scene.width << ' ' << scene.height << '\n';
    out << 255 << '\n';
    out.close();

    out.open(output_path, std::ios::binary | std::ios::app);
    out.write(reinterpret_cast<const char *>(real_pixels.data()), real_pixels.size());
    out.close();
}

int main(int argc, char *argv[])
{
    assert(argc == 3);
    std::string input_path = argv[1];
    std::string output_path = argv[2];

    Scene scene;
    scene.readTxt(input_path);

    std::cout << scene.primitives.size() << std::endl;

    std::vector<glm::vec3> pixels(scene.width * scene.height);

    int cnt = 0;
    int mod = 10000 ;

    for (int x = 0; x < scene.width; x++) {
        for (int y = 0; y < scene.height; y++) {
            Ray ray = scene.ray_to_pixel(glm::vec2(x + 0.5, y + 0.5));
            pixels[y * scene.width + x] =  scene.raytrace(ray);
        }
    }

    write_to_output(output_path, pixels, scene);
}
