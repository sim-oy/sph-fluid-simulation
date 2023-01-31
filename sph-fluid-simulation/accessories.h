#ifndef ACCESSORIES_H
#define ACCESSORIES_H

#include <stdint.h>
#include <CL/cl.h>

typedef struct {
    cl_float2* pos;
    cl_float2* vel;
    float* ran;
} particle;

void DrawParticles(particle particles, uint8_t* windowBuffer, int n, int window_width, int window_height);
void GenerateParticles(particle particles, int n);
float Randf();

#endif