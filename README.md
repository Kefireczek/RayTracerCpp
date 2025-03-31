# RayTracerC++

A simple but effective **real-time ray tracer** written in C++ using **SFML** and **GLM**. The engine performs ray tracing with support for reflections, materials, light emission, tone mapping, and cosine-weighted importance sampling. It uses an accumulation buffer to progressively improve image quality over frames.

## Example

![obraz](https://github.com/user-attachments/assets/06d0fc46-99ba-46b7-b7ab-216239ee0fee)


## Features

- Recursive ray tracing with configurable bounce depth (default: 4)
- Support for diffuse and specular materials, including emissive objects
- Cosine-weighted random ray directions (importance sampling)
- Frame accumulation with temporal smoothing
- ACES filmic tone mapping and linear-to-sRGB conversion
- Rendering with SFML

## Requirements

- C++17 or newer
- [SFML](https://www.sfml-dev.org/) (3.0)
- [GLM](https://github.com/g-truc/glm)

## Future

As the program is running on single core on CPU, the perfomance is really limited. The plan for future is to port the ray tracing to **CUDA** or **HLSL shaders**.

## Based on

https://blog.demofox.org/2020/05/25/casual-shadertoy-path-tracing-1-basic-camera-diffuse-emissive/
https://www.youtube.com/watch?v=Qz0KTGYJtUk
