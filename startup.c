#include <stdint.h>

extern uint32_t _sidata, _sdata, _edata, _sbss, _ebss, _estack;
extern int main(void);

void Reset_Handler(void) {
    /* .data aus dem Flash ins RAM kopieren */
    uint32_t *src  = &_sidata;
    uint32_t *dest = &_sdata;
    while (dest < &_edata) {
        *dest++ = *src++;
    }

    /* .bss nullen */
    dest = &_sbss;
    while (dest < &_ebss) {
        *dest++ = 0;
    }

    main();

    while (1);
}

/* Minimale Vektortabelle; fuer Messzwecke ausreichend. */
__attribute__((section(".isr_vector")))
void (*const g_pfnVectors[])(void) = {
    (void (*)(void))(&_estack),  /* Stack Pointer */
    Reset_Handler                /* Reset Handler */
};