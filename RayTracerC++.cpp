#include <SFML/Graphics.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <cmath>
#include <vector>
#include <iostream>

const int WIDTH = 800;
const int HEIGHT = 800;
const int MAX_BOUNCE_COUNT = 4;
const int NUM_RAYS_PER_PIXEL = 8;

std::vector<glm::vec3> accumulationBuffer(WIDTH * HEIGHT, glm::vec3(0.0f));

struct Ray {
    glm::vec3 origin;
    glm::vec3 direction;
};

struct Material {
    glm::vec3 albedo;
    float specular = 0;
    float emissionStength = 0;
    glm::vec3 emissionColor = glm::vec3();

    Material(sf::Color c) {
        albedo = { c.r, c.g, c.b };
    }

    Material() {
        albedo = glm::vec3();
    }

    Material(glm::vec3 c) {
        albedo = c;
    }

    Material(glm::vec3 c, float eS, glm::vec3 eC) {
        albedo = c;
        emissionColor = eC;
        emissionStength = eS;
    }

    Material(glm::vec3 c, float s) {
        albedo = c;
        specular = s;
    }
};

struct Sphere {
    glm::vec3 center;
    float radius;
    Material material;
};

struct HitInfo
{
    bool didHit;
    float dist;
    glm::vec3 hitPoint;
    glm::vec3 normal;
    Material material;
};

HitInfo rayIntersectsSphere(const Ray& ray, const Sphere& sphere) {
    HitInfo hitInfo{};

    glm::vec3 oc = ray.origin - sphere.center;
    float a = glm::dot(ray.direction, ray.direction);
    float b = 2.0f * glm::dot(oc, ray.direction);
    float c = glm::dot(oc, oc) - sphere.radius * sphere.radius;

    float discriminant = b * b - 4 * a * c;

    if (discriminant >= 0) {
        float t0 = (-b - std::sqrt(discriminant)) / (2.0f * a);

        if (t0 >= 0) {
            hitInfo.didHit = true;
            hitInfo.dist = t0;
            hitInfo.hitPoint = ray.origin + ray.direction * t0;
            hitInfo.normal = glm::normalize(hitInfo.hitPoint - sphere.center);
            hitInfo.material = sphere.material;
        }
    }

    return hitInfo;
}

float RandomValue01(uint32_t& state) {
    state = state * 747796405 + 2891336453;
    uint32_t result = ((state >> ((state >> 28) + 4)) ^ state) * 277803737;
    result = (result >> 22) ^ result;
    return result / 4294967295.0;
}

float RandomValueNormalDistribution(uint32_t& state) {
    float theta = 2 * 3.1415926 * RandomValue01(state);
    float rho = sqrt(-2 * log(RandomValue01(state)));
    return rho * cos(theta);
}

glm::vec3 RandomUnitVectorCosineWeighted(uint32_t& state) {
    float z = RandomValue01(state) * 2.0f - 1.0f;
    float a = RandomValue01(state) * 6.28318;
    float r = sqrt(1.0f - z * z);
    float x = r * cos(a);
    float y = r * sin(a);
    return glm::vec3(x, y, z);
}

glm::vec3 GetPixelColor(int x, int y, sf::Image& image, uint32_t seed);
glm::vec3 TraceRay(Ray ray, uint32_t& rngState);
HitInfo CheckRayIntersections(const Ray& ray);

std::vector<Sphere> spheres = {
    {{0, 0, 5}, 1.0f, Material(glm::vec3(1, 0.75, 0.82), 1)},
    {{1, 1, 6}, 1.0f, Material(glm::vec3(0.75, 1, 0.80))},
    {{-2, 0, 7}, 1.0f, Material(glm::vec3(0.70, 0.80, 1))},
    {{-2, 0, 7}, 1.0f, Material(glm::vec3(0.70, 0.80, 1), 0.75)},
    {{3, 0, 3}, 1.0f, Material(glm::vec3(0.70, 0.40, 0.7), 0.5)},
    {{-2, 2, 4}, 1.0f, Material(glm::vec3(1, 0.80, 0.5), 0.25)},
    {{0, -15, 10}, 15.0f, Material(glm::vec3{1, 1, 1})},
    {{ 5, 2, 5}, 3.0f, Material(glm::vec3(), 2.0f, glm::vec3{1, 1, 1})},
    { { -3, 2, 10 }, 3.0f, Material(glm::vec3(), 2.0f, glm::vec3{1, 1, 1}) }
};


glm::vec3 GetPixelColor(int x, int y, sf::Image& image, uint32_t seed)
{
    glm::vec2 pos(x, y);

    float aspectRatio = (float)WIDTH / HEIGHT;

    glm::vec2 jitter = glm::vec2(RandomValue01(seed), RandomValue01(seed)) - 0.5f;
    pos += jitter;

    // Normalizacja współrzędnych ekranu
    glm::vec2 uv(
        ((float)pos.x / WIDTH) * 2 - 1,
        -((float)pos.y / HEIGHT) * 2 + 1
    );

    Ray ray;
    ray.origin = { 0,0,0 };

    ray.direction = glm::normalize(glm::vec3(uv.x, uv.y / aspectRatio, 1.0f) - ray.origin);

    glm::vec3 totalIncomingLight = glm::vec3(0);

    for (int i = 0; i < NUM_RAYS_PER_PIXEL; i++)
    {
        totalIncomingLight += TraceRay(ray, seed);
    }

    totalIncomingLight /= NUM_RAYS_PER_PIXEL;

    return totalIncomingLight;
}

glm::vec3 TraceRay(Ray ray, uint32_t& rngState) {
    glm::vec3 rayColor = glm::vec3(1.0f);
    glm::vec3 incomingLight = glm::vec3(0.0f);

    for (int i = 0; i < MAX_BOUNCE_COUNT; i++)
    {
        HitInfo hit = CheckRayIntersections(ray);
        if (hit.didHit) {
            ray.origin = hit.hitPoint;

            if (RandomValue01(rngState) > hit.material.specular) {
                ray.direction = glm::normalize(hit.normal + RandomUnitVectorCosineWeighted(rngState));
            }
            else {
                ray.direction = glm::reflect(ray.direction, hit.normal);
            }
            

            Material material = hit.material;
            glm::vec3 emittedLight = material.emissionColor * material.emissionStength;
            incomingLight += emittedLight * rayColor;
            rayColor *= material.albedo;

            float p = std::max(rayColor.x, std::max(rayColor.y, rayColor.z));
            if (RandomValue01(rngState) > p) {
                break;
            }

            rayColor *= 1.0f / p;
        }
        else {
            break;
        }
    }

    return incomingLight;
}

HitInfo CheckRayIntersections(const Ray& ray) {
    HitInfo closestHit{};
    closestHit.didHit = false;
    float closestDistance = std::numeric_limits<float>::max();

    for (const auto& sphere : spheres) {
        HitInfo hit = rayIntersectsSphere(ray, sphere);
        if (hit.didHit && hit.dist < closestDistance) {
            closestDistance = hit.dist;
            closestHit = hit;
        }
    }

    return closestHit;
}

glm::vec3 ACESToneMapping(const glm::vec3& x) {
    float a = 2.51f;
    float b = 0.03f;
    float c = 2.43f;
    float d = 0.59f;
    float e = 0.14f;
    return glm::clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0f, 1.0f);
}

glm::vec3 LinearToSRGB(glm::vec3 color) {
    glm::vec3 higher = glm::pow(color, glm::vec3(1.0f / 2.4f)) * 1.055f - 0.055f;
    glm::vec3 lower = color * 12.92f;

    glm::vec3 result;
    result.x = (color.x < 0.0031308f) ? lower.x : higher.x;
    result.y = (color.y < 0.0031308f) ? lower.y : higher.y;
    result.z = (color.z < 0.0031308f) ? lower.z : higher.z;

    return result;
}

sf::Color ConvertColor(const glm::vec3& hdrColor) {

    glm::vec3 mapped = ACESToneMapping(hdrColor);

    glm::vec3 gammaCorrected = LinearToSRGB(mapped);

    int r = static_cast<int>(gammaCorrected.r * 255.0f);
    int g = static_cast<int>(gammaCorrected.g * 255.0f);
    int b = static_cast<int>(gammaCorrected.b * 255.0f);

    return sf::Color(r, g, b);
}

int main() {
    sf::RenderWindow window(sf::VideoMode({ WIDTH, HEIGHT }), "Ray Tracer");
    sf::Image newFrame({ WIDTH, HEIGHT }, sf::Color::Black);
    sf::Texture texture;


    uint32_t iFrame = 0;

    while (window.isOpen()) {

        while (const std::optional event = window.pollEvent())
        {
            // Close window: exit
            if (event->is<sf::Event::Closed>())
                window.close();
        }

        for (int x = 0; x < WIDTH; x++) {
            for (int y = 0; y < HEIGHT; y++) {

                uint32_t rngState = (uint32_t)(uint32_t(x) * uint32_t(1973) + uint32_t(y) * uint32_t(9277) + uint32_t(iFrame) * uint32_t(26699)) | uint32_t(1);

                glm::vec3 newColor = GetPixelColor(x, y, newFrame, rngState);

                sf::Vector2u pos = sf::Vector2u(x, y);

                float weight = 1.0f / (iFrame + 1);

                accumulationBuffer[y * WIDTH + x] = accumulationBuffer[y * WIDTH + x] * (1.0f - weight) + newColor * weight;

                glm::vec3 accColor = accumulationBuffer[y * WIDTH + x];
                sf::Color color = ConvertColor(accColor);

                newFrame.setPixel(pos, color);
            }
        }


        if (!texture.loadFromImage(newFrame)) {
            return 1;
        }
        sf::Sprite sprite(texture);

        window.clear();
        window.draw(sprite);
        window.display();

        iFrame++;
    }

    return 0;
}