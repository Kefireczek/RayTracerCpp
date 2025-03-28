#include <SFML/Graphics.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <cmath>
#include <vector>
#include <iostream>

const int WIDTH = 500;
const int HEIGHT = 500;

struct Ray {
    glm::vec3 origin;
    glm::vec3 direction;
};

struct Sphere {
    glm::vec3 center;
    float radius;
    sf::Color color;
};


bool rayIntersectsSphere(const Ray& ray, const Sphere& sphere, float& distance) {
    glm::vec3 oc = ray.origin - sphere.center;
    float a = glm::dot(ray.direction, ray.direction);
    float b = 2.0f * glm::dot(oc, ray.direction);
    float c = glm::dot(oc, oc) - sphere.radius * sphere.radius;

    float discriminant = b * b - 4 * a * c;

    if (discriminant < 0) {
        return false;
    }

    float t0 = (-b - std::sqrt(discriminant)) / (2.0f * a);
    float t1 = (-b + std::sqrt(discriminant)) / (2.0f * a);

    distance = (t0 < t1) ? t0 : t1;
    return distance > 0;
}

int main() {
    sf::RenderWindow window(sf::VideoMode({ WIDTH, HEIGHT }), "Ray Tracer");
    sf::Image image({ WIDTH, HEIGHT }, sf::Color::Black);
    sf::Texture texture;

    std::vector<Sphere> spheres = {
        {{0, 0, 5}, 1.0f, sf::Color::Red},
        {{2, 1, 6}, 0.5f, sf::Color::Blue},
        {{-2, -1, 7}, 0.75f, sf::Color::Green}
    };


    for (int x = 0; x < WIDTH; x++) {
        for (int y = 0; y < HEIGHT; y++) {

            sf::Vector2u pos(x, y);
            // Normalizacja współrzędnych ekranu
            sf::Vector2f uv(
                ((float)pos.x / WIDTH) * 2 - 1,
                -((float)pos.y / HEIGHT) * 2 + 1
            );

            Ray ray;
            ray.origin = { 0,0,0 };
            ray.direction = glm::normalize(glm::vec3(uv.x, uv.y, 1.0f));

            sf::Color pixelColor = sf::Color::Black;
            float closestDistance = std::numeric_limits<float>::max();

            // Sprawdzanie przecięć ze wszystkimi sferami
            for (const auto& sphere : spheres) {
                float distance;
                if (rayIntersectsSphere(ray, sphere, distance)) {
                    if (distance < closestDistance) {
                        closestDistance = distance;
                        pixelColor = sphere.color;
                    }
                }
            }

            image.setPixel(pos, pixelColor);
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