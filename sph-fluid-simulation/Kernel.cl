#ifndef PARTICLES_COUNT
#define PARTICLES_COUNT -1
#endif

typedef struct {
	float2 pos[PARTICLES_COUNT];
	float2 vel[PARTICLES_COUNT];
	float mss[PARTICLES_COUNT];
} particle;





__kernel void Calc(__global particle* particles) {
    unsigned int i = get_global_id(0);

    float sumX = 0, sumY = 0;
    #pragma unroll 64
    for (int j = 0; j < PARTICLES_COUNT; j++)
    {
        if (i == j)
            continue;

        float distanceX = particles->pos[j].x - particles->pos[i].x;
        float distanceY = particles->pos[j].y - particles->pos[i].y;

        float rij = pown(sqrt(distanceX * distanceX + distanceY * distanceY), 3);

        float fij_x = -distanceX / rij;
        float fij_y = -distanceY / rij;

        sumX += fij_x;
        sumY += fij_y;
    }
    float timestep = 0.000000000001f;

    particles->vel[i].x += sumX * timestep;
    particles->vel[i].y += sumY * timestep;

    // gravity
    //particles[i].vy += 0.0001;

    // boundary
    //float collisionFriction = 0.5f;
    float collisionFriction = 1;

    if (particles->pos[i].x < 0.0f) {
        particles->vel[i].x = fabs(particles->vel[i].x) * collisionFriction;
    }
    else if (particles->pos[i].x > 1.0f) {
        particles->vel[i].x = -fabs(particles->vel[i].x * collisionFriction);
    }
    if (particles->pos[i].y < 0.0f) {
        particles->vel[i].y = fabs(particles->vel[i].y * collisionFriction);
    }
    else if (particles->pos[i].y > 1.0f) {
        particles->vel[i].y = -fabs(particles->vel[i].y * collisionFriction);
    }
    //float environmentFriction = 0.9998f;
    float environmentFriction = 1.0f;
    particles->vel[i].x *= environmentFriction;
    particles->vel[i].y *= environmentFriction;
}







__kernel void Move(__global particle* particles) {
	unsigned int i = get_global_id(0);
	particles->pos[i].x += particles->vel[i].x;
	particles->pos[i].y += particles->vel[i].y;
}

/*
__kernel void Calc(__global particle* particles) {
    unsigned int i = get_global_id(0);

    float sumX = 0, sumY = 0;
    float range = particles->ran[i];
    #pragma unroll 64
    for (int j = 0; j < PARTICLES_COUNT; j++)
    {
        if (i == j)
            continue;

        float distanceX = particles->pos[j].x - particles->pos[i].x;
        float distanceY = particles->pos[j].y - particles->pos[i].y;

        if (fabs(distanceX) > range || fabs(distanceY) > range)
            continue;

        float x2_y2 = distanceX * distanceX + distanceY * distanceY;

        if (x2_y2 >= range * range)
            continue;

        float rdist = rsqrt(x2_y2 + 0.000001);

        //suuntavektorit
        float sx = distanceX * rdist;
        float sy = distanceY * rdist;

        float f_xy = range * (cospi(1 / (range * rdist)) + 1);

        sumX += -sx * f_xy;
        sumY += -sy * f_xy;
    }
    float timestep = 0.0001f;

    particles->vel[i].x += sumX * timestep;
    particles->vel[i].y += sumY * timestep;

    // gravity
    //particles[i].vy += 0.0001;

    // boundary
    float collisionFriction = 0.5f;
    //float collisionFriction = 1;

    if (particles->pos[i].x < 0) {
        particles->vel[i].x *= fabs(collisionFriction);
    }
    else if (particles->pos[i].x > 1.0) {
        particles->vel[i].x *= -fabs(collisionFriction);
    }
    if (particles->pos[i].y < 0) {
        particles->vel[i].y *= fabs(collisionFriction);
    }
    else if (particles->pos[i].y > 1.0) {
        particles->vel[i].y *= -fabs(collisionFriction);
    }
    float environmentFriction = 0.9998;
    particles->vel[i].x *= environmentFriction;
    particles->vel[i].y *= environmentFriction;
}
*/