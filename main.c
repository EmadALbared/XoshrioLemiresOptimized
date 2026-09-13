/* xoshiro128** + Lemire, Latenzmessung auf Cortex-M4 (STM32F407).
 * Uebersetzen mit -O2. Ausgabe ueber Semihosting. */

#include <stdint.h>
#include <stdio.h>

#define DEMCR      (*(volatile uint32_t*)0xE000EDFC)
#define DWT_CTRL   (*(volatile uint32_t*)0xE0001000)
#define DWT_CYCCNT (*(volatile uint32_t*)0xE0001004)

#define BARRIER() __asm__ volatile ("" ::: "memory")
#define INLINE static inline __attribute__((always_inline))

#define N     10000u      /* Iterationen */
#define RANGE 3329u       /* Zielbereich fuer Lemire */

static volatile uint32_t sink;
extern void initialise_monitor_handles(void);

INLINE uint32_t rotl(uint32_t x, int k) {
    return (x << k) | (x >> (32 - k));
}

INLINE uint32_t lemire(uint32_t r, uint32_t range) {
    return (uint32_t)(((uint64_t)r * (uint64_t)range) >> 32);
}

int main(void) {
    uint32_t t0, t1, empty, only_next, with_lemire;
    uint32_t s0, s1, s2, s3, res, t, acc;
    uint32_t i;

    initialise_monitor_handles();

    DEMCR    |= (1u << 24);          /* TRCENA    */
    DWT_CYCCNT = 0;
    DWT_CTRL |= (1u << 0);           /* CYCCNTENA */

    /* --- Leerschleife (Referenz) --- */
    BARRIER(); t0 = DWT_CYCCNT; BARRIER();
    for (i = 0; i < N; i++) { __asm__ volatile (""); }
    BARRIER(); t1 = DWT_CYCCNT; BARRIER();
    empty = t1 - t0;

    /* --- nur next() --- */
    s0 = 0x12345678u; s1 = 0x9ABCDEF0u;
    s2 = 0xDEADBEEFu; s3 = 0x09876543u;
    acc = 0;
    BARRIER(); t0 = DWT_CYCCNT; BARRIER();
    for (i = 0; i < N; i++) {
        res = rotl(s1 * 5u, 7) * 9u;
        t   = s1 << 9;
        s2 ^= s0; s3 ^= s1; s1 ^= s2; s0 ^= s3;
        s2 ^= t;  s3 = rotl(s3, 11);
        acc ^= res;
    }
    BARRIER(); t1 = DWT_CYCCNT; BARRIER();
    only_next = t1 - t0 - empty;
    sink = acc;

    /* --- next() + Lemire --- */
    s0 = 0x12345678u; s1 = 0x9ABCDEF0u;
    s2 = 0xDEADBEEFu; s3 = 0x09876543u;
    acc = 0;
    BARRIER(); t0 = DWT_CYCCNT; BARRIER();
    for (i = 0; i < N; i++) {
        res = rotl(s1 * 5u, 7) * 9u;
        t   = s1 << 9;
        s2 ^= s0; s3 ^= s1; s1 ^= s2; s0 ^= s3;
        s2 ^= t;  s3 = rotl(s3, 11);
        acc ^= lemire(res, RANGE);
    }
    BARRIER(); t1 = DWT_CYCCNT; BARRIER();
    with_lemire = t1 - t0 - empty;
    sink = acc;

    printf("Iterationen   : %lu\n", (unsigned long)N);
    printf("next()        : %lu.%02lu Zyklen\n",
           (unsigned long)(only_next / N),
           (unsigned long)((only_next % N) * 100 / N));
    printf("next()+Lemire : %lu.%02lu Zyklen\n",
           (unsigned long)(with_lemire / N),
           (unsigned long)((with_lemire % N) * 100 / N));
    printf("Lemire allein : %lu.%02lu Zyklen\n",
           (unsigned long)((with_lemire - only_next) / N),
           (unsigned long)(((with_lemire - only_next) % N) * 100 / N));
    fflush(stdout);

    while (1) { __asm__ volatile ("nop"); }
    return 0;
}