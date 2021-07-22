#ifndef CROSSRAND_H
#define CROSSRAND_H

#define CROSS_RAND_MAX 0x7FFF

// portable rand functions

static unsigned int cross_seed = 0;
inline void cross_srand(unsigned int value)
{
    cross_seed = value;
}

inline unsigned int cross_rand()
{
    cross_seed = cross_seed * 214013 + 2531011;
    return (cross_seed >> 16) & CROSS_RAND_MAX;
}

#endif // CROSSRAND_H
