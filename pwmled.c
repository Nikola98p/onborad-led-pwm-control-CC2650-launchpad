/* XDCtools Header files */
#include <xdc/std.h>
#include <xdc/runtime/System.h>

/* BIOS Header files */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>

/* TI-RTOS Header files */
#include <ti/drivers/GPIO.h>
#include <ti/drivers/PWM.h>

#include "Board.h"

#define TASKSTACKSIZE   512

Task_Struct tsk0Struct;
UInt8 tsk0Stack[TASKSTACKSIZE];
Task_Handle task;


Void pwmLEDFxn(UArg arg0, UArg arg1) // PWM Control
{
    PWM_Handle pwm1;
    PWM_Params params;
    uint16_t   pwmPeriod = 300000;      // Period and duty in microseconds
    uint16_t   duty = 0;                //init duty value
    uint16_t   dutyInc = 1000;

    PWM_Params_init(&params);
    params.dutyUnits = PWM_DUTY_US;
    params.dutyValue = 0;
    params.periodUnits = PWM_PERIOD_US;
    params.periodValue = pwmPeriod;
    pwm1 = PWM_open(Board_PWM0, &params);

    if (pwm1 == NULL) {
        System_abort("Board_PWM0 did not open");
    }
    PWM_start(pwm1);
    /* Loop forever incrementing the PWM duty */
    while (1) {
        PWM_setDuty(pwm1, duty);

        duty = (duty + dutyInc);
        if (duty == pwmPeriod || (!duty)) {
            dutyInc =- dutyInc;
        }

        Task_sleep((UInt) arg0);
    }
}


int main(void)
{
    Task_Params tskParams;

    // Call board init functions.
    Board_initGeneral();
    Board_initGPIO();
    Board_initPWM();

    // Construct LED Task thread
    Task_Params_init(&tskParams);
    tskParams.stackSize = TASKSTACKSIZE;
    tskParams.stack = &tsk0Stack;
    tskParams.arg0 = 100;               //varies the intensity of the on-board LEDs
    Task_construct(&tsk0Struct, (Task_FuncPtr)pwmLEDFxn, &tskParams, NULL);

    /* Obtain instance handle */
    task = Task_handle(&tsk0Struct);

    /* Turn on user LED */
    GPIO_write(Board_LED0, Board_LED_ON);

    /* SysMin will only print to the console when you call flush or exit */
    System_flush();

    /* Start BIOS */
    BIOS_start();

    return (0);
}
