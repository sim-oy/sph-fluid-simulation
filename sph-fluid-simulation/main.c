#include "main.h"
#include "OpenCL.h"
#include "accessories.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <time.h>
#include <Cl/cl.h>
#include <SFML/Graphics.h>

int main(void) {
    printf("start\n");

    const int n2 = 10;
    const int window_width = 500;
    const int window_height = 500;
    const int rounding = 256;
    const int n = (n2 % rounding == 0 ? n2 : (n2 - n2 % rounding) + rounding);
    printf("n = %d\n", n);

    particle particles = {
        .pos = (cl_float2*)malloc(n * sizeof(cl_float2)),
        .vel = (cl_float2*)malloc(n * sizeof(cl_float2)),
        .mss = (float*)malloc(n * sizeof(float))
    };
    GenerateParticles(particles, n);

    uint8_t* windowBuffer = (uint8_t*)malloc(window_width * window_height * 4 * sizeof(uint8_t));

    sfVideoMode mode = { window_width, window_height, 32 };
    sfRenderWindow* window;
    sfEvent event;
    sfTexture* Texture = sfTexture_create(window_width, window_height);
    sfSprite* sprite;

    window = sfRenderWindow_create(mode, "My Window", sfClose, NULL);
    if (!window)
        return -1;
    sfRenderWindow_setVerticalSyncEnabled(window, sfFalse);
    sprite = sfSprite_create();

    CLInit(&particles, n * 5);

    clock_t oa_tim_strt = 0, oa_tim_end = 0;
    int frames = 0;
    double times[FRAMES_PER_PRINT] = { 0 };

    while (sfRenderWindow_isOpen(window))
    {
        oa_tim_strt = clock();
        while (sfRenderWindow_pollEvent(window, &event))
        {
            if (event.type == sfEvtClosed)
                sfRenderWindow_close(window);
        }
        sfRenderWindow_clear(window, sfBlack);

        //printf("%llu\n", sizeof(windowBuffer));
        memset(windowBuffer, 0, window_width * window_height * 4 * sizeof(uint8_t));
        DrawParticles(particles, windowBuffer, n, window_width, window_height);

        CLRun(&particles, n * 5, rounding);

        sfTexture_updateFromPixels(Texture, windowBuffer, window_width, window_height, 0, 0);
        sfSprite_setTexture(sprite, Texture, sfFalse);
        sfRenderWindow_drawSprite(window, sprite, NULL);
        sfRenderWindow_display(window);

        oa_tim_end = clock();
        double elapsedTime_s = ((double)(oa_tim_end - oa_tim_strt)) / CLOCKS_PER_SEC;
        times[frames] = elapsedTime_s;
        if (frames >= FRAMES_PER_PRINT - 1) {
            double avg_elapsedTime_s = DoubleArraySum(times, FRAMES_PER_PRINT) / (double)FRAMES_PER_PRINT;
            printf("time: ms %d\t fps: %.1lf\n", (int)(avg_elapsedTime_s * 1000), 1.0 / avg_elapsedTime_s);
            frames = 0;
        }
        else {
            frames++;
        }
    }
    sfRenderWindow_destroy(window);

    free(windowBuffer);
    free(particles.pos);
    free(particles.vel);
    free(particles.mss);

    printf("end\n");
    return 0;
}

double DoubleArraySum(double array[], int len) {
    double sum = 0;
    for (int i = 0; i < len; i++) {
        sum += array[i];
    }
    return sum;
}