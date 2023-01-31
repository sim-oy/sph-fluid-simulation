#include "accessories.h"
#include <stdio.h>
#include <stdlib.h>

void DrawParticles(particle particles, uint8_t* windowBuffer, int n, int window_width, int window_height) {
    for (int i = 0; i < n; i++) {
        if (particles.pos[i].x < 0 || particles.pos[i].x >= 1.0 || particles.pos[i].y < 0 || particles.pos[i].y >= 1.0) {
            continue;
        }

        int x = (int)(particles.pos[i].x * window_width);
        int y = (int)(particles.pos[i].y * window_height);

        int index = (y * window_width + x) * 4;

        windowBuffer[index] = 255;
        windowBuffer[index + 1] = 255;
        windowBuffer[index + 2] = 255;
        windowBuffer[index + 3] = 255;
    }
}

void GenerateParticles(particle particles, int n) {
    for (int i = 0; i < n; i++) {
        particles.pos[i].x = Randf() * 0.2;
        particles.pos[i].y = Randf() * 0.2;
        particles.vel[i].x = 0;
        particles.vel[i].y = 0;
        particles.ran[i] = 1.0f / 100.0f;
    }
}

float Randf() {
    return (float)rand() / (float)(RAND_MAX);
}