#include <SFML/Graphics.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <cmath>
#include <vector>
#include <iostream>

const int WIDTH = 1000;
const int HEIGHT = 500;
const int MAX_BOUNCE_COUNT = 30;

struct Ray {
    glm::vec3 origin;
    glm::vec3 direction;
};

struct Material {
    glm::vec3 color;
    float emissionStenght = 0;
    glm::vec3 emissionColor = glm::vec3();

    Material(sf::Color c) {
        color = { c.r, c.g, c.b };
    }

    Material() {
        color = glm::vec3();
    }

    Material(glm::vec3 c, float eS, glm::vec3 eC) {
        color = c;
        emissionColor = eC;
        emissionStenght = eS;
    }
};

struct Sphere {
    glm::vec3 center;
    float radius;
    Material material;
};

struct Plane {
    glm::vec3 point;   // Punkt na płaszczyźnie
    glm::vec3 normal;  // Wektor normalny płaszczyzny (powinien być znormalizowany)
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
        }
    }

    return hitInfo;
}

HitInfo rayIntersectsPlane(const Ray& ray, const Plane& plane) {
    HitInfo hitInfo{};

    // Obliczanie mianownika
    float denom = glm::dot(plane.normal, ray.direction);

    // Jeśli mianownik jest bliski zeru, promień jest równoległy do płaszczyzny
    if (std::abs(denom) > 1e-6) {
        // Obliczanie odległości do punktu przecięcia
        glm::vec3 p0l0 = plane.point - ray.origin;
        float t = glm::dot(p0l0, plane.normal) / denom;

        // Sprawdzamy czy przecięcie jest przed promieniem
        if (t >= 0) {
            hitInfo.didHit = true;
            hitInfo.dist = t;

            // Obliczanie punktu kolizji
            hitInfo.hitPoint = ray.origin + ray.direction * t;

            // Ustawiamy normalną (zwróć uwagę, że normalna może być odwrócona)
            hitInfo.normal = denom < 0 ? plane.normal : -plane.normal;
        }
    }

    // Jeśli odległość jest ujemna, przecięcie jest za promieniem
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

void GeneratePixel(int x, int y, sf::Image& image);
glm::vec3 TraceRay(Ray ray, uint32_t& rngState);
HitInfo CheckRayIntersections(const Ray& ray);

std::vector<Sphere> spheres = {
    {{0, 0, 5}, 1.0f, Material(sf::Color::Red)},
    {{1, 1, 6}, 1.0f, Material(sf::Color::Green)},
    {{-2, 0, 7}, 1.0f, Material(sf::Color::Blue)},
    {{ 0, 5, 5}, 3.0f, Material(glm::vec3(), 1.0f, glm::vec3{255, 255, 255})}
};

std::vector<Plane> planes = {
{{0, -2, 0}, {0, 1, 0}, Material({150, 150, 150})},  // Podłoga
{{0, 0, 10}, {0, 0, -1}, Material({200, 200, 200})}  // Tło
};

int main() {
    sf::RenderWindow window(sf::VideoMode({ WIDTH, HEIGHT }), "Ray Tracer");
    sf::Image image({ WIDTH, HEIGHT }, sf::Color::Black);
    sf::Texture texture;

    for (int x = 0; x < WIDTH; x++) {
        for (int y = 0; y < HEIGHT; y++) {

            GeneratePixel(x, y, image);
        }
    }


    if (!texture.loadFromImage(image)) {
        return 1;
    }
    sf::Sprite sprite(texture);


    while (window.isOpen()) {
        while (const std::optional event = window.pollEvent())
        {
            // Close window: exit
            if (event->is<sf::Event::Closed>())
                window.close();
        }

        window.clear();
        window.draw(sprite);
        window.display();
    }

    return 0;
}



void GeneratePixel(int x, int y, sf::Image& image)
{
    sf::Vector2u pos(x, y);

    //Random Seed for pixel
    uint32_t seed = pos.x * WIDTH + pos.y * HEIGHT;


    float aspectRatio = WIDTH / HEIGHT;
    
    // Normalizacja współrzędnych ekranu
    sf::Vector2f uv(
        ((float)pos.x / WIDTH) * 2 - 1,
        -((float)pos.y / HEIGHT) * 2 + 1
    );

    Ray ray;
    ray.origin = { 0,0,0 };
    ray.direction = glm::normalize(glm::vec3(uv.x, uv.y / aspectRatio, 1.0f) - ray.origin);

    glm::vec3 pixelColor = TraceRay(ray, seed);
    //std::cout << pixelColor.x << ", " << pixelColor.y << ", " << pixelColor.z << ", " << std::endl;
    float closestDistance = std::numeric_limits<float>::max();



    image.setPixel(pos, sf::Color(pixelColor.x, pixelColor.y, pixelColor.z));
}

glm::vec3 TraceRay(Ray ray, uint32_t& rngState) {
    glm::vec3 rayColor = glm::vec3(1);
    glm::vec3 incomingLight = glm::vec3(0);

    for (int i = 0; i < MAX_BOUNCE_COUNT; i++)
    {
        HitInfo hit = CheckRayIntersections(ray);
        if (hit.didHit) {
            ray.origin = hit.hitPoint;
            ray.direction = glm::normalize(hit.normal + RandomUnitVectorCosineWeighted(rngState));

            Material material = hit.material;
            glm::vec3 emittedLight = material.emissionColor * material.emissionStenght;
            incomingLight += emittedLight * rayColor;
            rayColor *= material.color;
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

    // Sprawdzanie przecięć ze wszystkimi sferami
    for (const auto& sphere : spheres) {
        HitInfo hit = rayIntersectsSphere(ray, sphere);
        if (hit.didHit && hit.dist < closestDistance) {
            closestDistance = hit.dist;
            closestHit = hit;
            closestHit.material = sphere.material;
        }
    }

    // Sprawdzanie przecięć ze wszystkimi płaszczyznami
    for (const auto& plane : planes) {
        HitInfo hit = rayIntersectsPlane(ray, plane);
        if (hit.didHit && hit.dist < closestDistance) {
            closestDistance = hit.dist;
            closestHit = hit;
            closestHit.material = plane.material;
        }
    }

    return closestHit;
}
