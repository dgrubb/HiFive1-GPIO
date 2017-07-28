/* Standard includes */
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

/* HiFive1/FE310 includes */
#include "platform.h"
#include "encoding.h"

#include "plic/plic_driver.h"
#include "sifive/devices/spi.h"

#define RTC_FREQUENCY 32768

/* 840nS period, 1.19MHz */
#define PWM_SCALE     0x0
#define PWM_FREQ      440

#define INPUT_OFFSET (INT_GPIO_BASE + PIN_19_OFFSET)

plic_instance_t g_plic;

void handle_m_time_interrupt()
{
}

void handle_m_ext_interrupt()
{
    plic_source int_num = PLIC_claim_interrupt(&g_plic);
    switch (int_num) {
        case 0:
            break;
        case PIN_19_OFFSET:
            GPIO_REG(GPIO_OUTPUT_VAL) ^= (0x1 << PIN_18_OFFSET);
            GPIO_REG(GPIO_RISE_IP)     = (0x1 << PIN_19_OFFSET);
            break;
    }
    PLIC_complete_interrupt(&g_plic, int_num);
}

void start_demo()
{
    /* Disable the machine and timer interrupts until setup is completed */
    clear_csr(mie, MIP_MEIP);
    clear_csr(mie, MIP_MTIP);

    /* Configure PWM  */
    PWM1_REG(PWM_CFG) = 0; // Clear the configuration register
    /* This is the real meat of things.
    */
    PWM1_REG(PWM_CFG) =
    (PWM_CFG_ENALWAYS) |
    (PWM_CFG_DEGLITCH) |
    (PWM_CFG_CMP2CENTER) |
    (PWM_CFG_CMP3CENTER) |
    (PWM_CFG_ZEROCMP) |
    (PWM_SCALE);
    PWM1_REG(PWM_COUNT) = 0;

    /* Enable PWM output pins. No particular reason why I enabled both
    * the blue and red LEDs except that I think the purple looks kind
    * of neat.
    */
    GPIO_REG(GPIO_IOF_SEL) |= (1 << BLUE_LED_OFFSET);
    GPIO_REG(GPIO_IOF_SEL) |= (1 << RED_LED_OFFSET);
    GPIO_REG(GPIO_IOF_EN)  |= (1 << BLUE_LED_OFFSET);
    GPIO_REG(GPIO_IOF_EN)  |= (1 << RED_LED_OFFSET);

    PWM1_REG(PWM_CMP0) = PWM_FREQ;
    PWM1_REG(PWM_CMP2) = PWM_FREQ/2;
    PWM1_REG(PWM_CMP3) = PWM_FREQ/2;

    /* Re-enable timers */
    set_csr(mie, MIP_MTIP);
    set_csr(mie, MIP_MEIP);
    set_csr(mstatus, MSTATUS_MIE);
}

void setup_GPIO()
{
    PLIC_init(&g_plic, PLIC_CTRL_ADDR, PLIC_NUM_INTERRUPTS, PLIC_NUM_PRIORITIES);
    PLIC_enable_interrupt (&g_plic, PIN_19_OFFSET);
    PLIC_set_threshold(&g_plic, 0);
    PLIC_set_priority(&g_plic, PIN_19_OFFSET, 1);

    GPIO_REG(GPIO_OUTPUT_EN) &= ~(0x1 << (PIN_19_OFFSET - INT_GPIO_BASE));
    GPIO_REG(GPIO_PULLUP_EN) &= ~(0x1 << (PIN_19_OFFSET - INT_GPIO_BASE));
    GPIO_REG(GPIO_INPUT_EN)  |=  (0x1 << (PIN_19_OFFSET - INT_GPIO_BASE));
    GPIO_REG(GPIO_RISE_IE)   |=  (0x1 << (PIN_19_OFFSET - INT_GPIO_BASE));

    // Use as a test output
    GPIO_REG(GPIO_OUTPUT_EN)  |=  (1 << PIN_18_OFFSET);
    GPIO_REG(GPIO_OUTPUT_VAL) |=  (1 << PIN_18_OFFSET);
}

int main()
{
    setup_GPIO();
    start_demo();

    while (1) {};

    return 0;
}
