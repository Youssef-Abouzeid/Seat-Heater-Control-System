/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "event_groups.h"

/* MCAL includes. */
#include "uart0.h"
#include "gpio.h"
#include "GPTM.h"
#include "adc.h"
#include "tm4c123gh6pm_registers.h"

/* HAL includes */
#include "potentiometer.h"


/* Definitions for the  Event Flags bits in the event group  */
#define mainDRIVER_INTERRUPT_BIT (1UL << 0UL)    /* Event bit 0, which is set by a SW1 Interrupt. */
#define mainPASSENGER_INTERRUPT_BIT (1UL << 1UL) /* Event bit 1, which is set by a SW2 Interrupt. */

#define mainDISABLED_INTENSITY_BIT (1UL << 0UL) /* Event bit 0 */
#define mainLOW_INTENSITY_BIT (1UL << 1UL)      /* Event bit 0 */
#define mainMEDIUM_INTENSITY_BIT (1UL << 2UL)   /* Event bit 1 */
#define mainHIGH_INTENSITY_BIT (1UL << 3UL)     /* Event bit 2 */


#define ENABLE_RUNTIME_MEASUREMENT TRUE
#define ENABLE_DIAGONSTICS TRUE

#define BUFFER_SIZE 256




/* Definitions for TaskIds*/
typedef enum
{
    DriverTask,
    PassengerTask

} TaskID;

/* Definitions for Level of Heating */
typedef enum
{
    OFF,
    LOW,
    MEDIUM,
    HIGH
} HeatingLevel_t;

typedef struct
{
    TaskID xxxGetTaskID;
    HeatingLevel_t xCurrentLevel;
    uint16 usCurrentTemp;
    uint8 ucTaskActive;
    uint8 *pcCurrentSeat;
    uint8 *pcHeatIntensity;
} SeatTyeInfo;

SeatTyeInfo SeatInfo[2];

typedef struct
{
    uint32 FailureTimeStamp;
    uint8 *pcCurrentSeat;
    HeatingLevel_t xCurrentLevel;
} DiagonsticsType;

DiagonsticsType Diagonstics[BUFFER_SIZE];
uint16_t usCounter = 0;


/* The HW setup function */
static void prvSetupHardware(void);

/* FreeRTOS tasks */
void vLevelSettingTempTask(void *pvParameters);
void vTempReadingTask(void *pvParameters);
void vControlTask(void *pvParameters);
void vHeatingElementTask(void *pvParameters);
void vDisplaytask(void *pvParameters);
void vRunTimeMeasurementsTask(void *pvParameters);
void vDiagonsticsTask(void *pvParameters);


/* Task RunTimeMeasurements */
uint32 ullTasksOutTime[9];
uint32 ullTasksInTime[9];
uint32 ullTasksExecutionTime[9];
uint32 ullTasksTotalTime[9];
uint32 ullResourceLockimeIn[5]={0};
uint32 ullResourceLockimeOut[5]={0};

/* Used to hold the handle of tasks */
TaskHandle_t xLevelSettingTempTaskHandle;
TaskHandle_t xControlTaskHandle;
TaskHandle_t xDriverTempReadingTaskHandle;
TaskHandle_t xPassengerTempReadingTaskHandle;
TaskHandle_t xHeatingElementTaskHandle;
TaskHandle_t xDisplaytaskHandle;
TaskHandle_t xRunTimeMeasurementsTaskHandle;
TaskHandle_t xDiagnosticsTaskHandle;



/* EventGroup Handle */
EventGroupHandle_t xButtonEvent;
EventGroupHandle_t xHeatingEvent;

/* CountingSemaphore Handle */
SemaphoreHandle_t xTempToControlTaskSync;
SemaphoreHandle_t xDisplayToTempTaskSync;
SemaphoreHandle_t xTempToDiagonsticsTaskSync;

/* Mutex Handle*/
SemaphoreHandle_t xMutex;

/*Share Resource*/
TaskID xCurrentTaskID;

int main()
{

    /* Setup the hardware for use with the Tiva C board. */
    prvSetupHardware();

    /* Create EventGroup */
    xButtonEvent = xEventGroupCreate();
    xHeatingEvent = xEventGroupCreate();

    /* Create Semaphores */
    xTempToControlTaskSync = xSemaphoreCreateBinary();
    xDisplayToTempTaskSync = xSemaphoreCreateCounting(1, 1);
    xTempToDiagonsticsTaskSync = xSemaphoreCreateBinary();


    /* Create Mutex */
    xMutex = xSemaphoreCreateMutex();

    /* Create Tasks here */
    xTaskCreate(vLevelSettingTempTask,
                "Driver Seat Buttons",
                configMINIMAL_STACK_SIZE,
                NULL,
                3,
                &xLevelSettingTempTaskHandle);

    xTaskCreate(vControlTask, "Control Task", configMINIMAL_STACK_SIZE, NULL, 2, &xControlTaskHandle);

    xTaskCreate(vTempReadingTask, "Driver Seat Temp Sensor", configMINIMAL_STACK_SIZE, (void *)DriverTask, 2, &xDriverTempReadingTaskHandle);
    xTaskCreate(vTempReadingTask, "Passenger Seat Temp Sensor", configMINIMAL_STACK_SIZE, (void *)PassengerTask, 2, &xPassengerTempReadingTaskHandle);

    xTaskCreate(vHeatingElementTask, "Heating Element Task", configMINIMAL_STACK_SIZE, NULL, 3, &xHeatingElementTaskHandle);

    xTaskCreate(vDisplaytask, "Display Task", configMINIMAL_STACK_SIZE, NULL, 1, &xDisplaytaskHandle);

#if(ENABLE_RUNTIME_MEASUREMENT == TRUE)
    xTaskCreate(vRunTimeMeasurementsTask, "Run time",configMINIMAL_STACK_SIZE, NULL, 1, &xRunTimeMeasurementsTaskHandle);
#endif

    xTaskCreate(vDiagonsticsTask, "Diagnostics Task", configMINIMAL_STACK_SIZE, NULL, 1, &xDiagnosticsTaskHandle);


    vTaskSetApplicationTaskTag( xLevelSettingTempTaskHandle, ( void * ) 1 );
    vTaskSetApplicationTaskTag( xControlTaskHandle, ( void * ) 2 );
    vTaskSetApplicationTaskTag( xDriverTempReadingTaskHandle, ( void * ) 3 );
    vTaskSetApplicationTaskTag( xPassengerTempReadingTaskHandle, ( void * ) 4 );
    vTaskSetApplicationTaskTag( xHeatingElementTaskHandle, ( void * ) 5 );
    vTaskSetApplicationTaskTag( xDisplaytaskHandle, ( void * ) 6 );
    vTaskSetApplicationTaskTag( xRunTimeMeasurementsTaskHandle, ( void * ) 7 );
    vTaskSetApplicationTaskTag( xDiagnosticsTaskHandle, ( void * ) 8 );


    /* Now all the tasks have been started - start the scheduler.

    NOTE : Tasks run in system mode and the scheduler runs in Supervisor mode.
    The processor MUST be in supervisor mode when vTaskStartScheduler is
    called.  The demo applications included in the FreeRTOS.org download switch
    to supervisor mode prior to main being called.  If you are not using one of
    these demo application projects then ensure Supervisor mode is used here. */
    vTaskStartScheduler();

    /* Should never reach here!  If you do then there was not enough heap
    available for the idle task to be created. */
    for (;;)
        ;
}

static void prvSetupHardware(void)
{
    /* Place here any needed HW initialization such as GPIO, UART, etc.  */
    UART0_Init();
    GPIO_BuiltinButtonsLedsInit();
    GPTM_WTimer0Init();
    ADC_Init();
}

/* Task to handle button presses and adjust heating levels */
void vLevelSettingTempTask(void *pvParameters)
{
    EventBits_t xBitsToWaitFor = (mainDRIVER_INTERRUPT_BIT | mainPASSENGER_INTERRUPT_BIT);
    EventBits_t xEventGroupValue;
    HeatingLevel_t xDriverPrevLevel = OFF;
    HeatingLevel_t xPassengerPrevLevel = OFF;
    for (;;)
    {
        /* Block to wait for event bits to become set within the event group. */
        xEventGroupValue = xEventGroupWaitBits(xButtonEvent,   /* The event group to read. */
                                               xBitsToWaitFor, /* Bits to test. */
                                               pdTRUE,         /* Clear bits on exit if the unblock condition is met. */
                                               pdFALSE,        /* Don't wait for all bits. */
                                               portMAX_DELAY); /* Don't time out. */

        if ((xEventGroupValue & mainDRIVER_INTERRUPT_BIT) != 0)
        {
            xDriverPrevLevel = (xDriverPrevLevel == HIGH) ? OFF : ++xDriverPrevLevel;
            SeatInfo[DriverTask].xCurrentLevel = xDriverPrevLevel;
            SeatInfo[DriverTask].ucTaskActive = (SeatInfo[DriverTask].xCurrentLevel == OFF) ? FALSE : TRUE;
        }
        else if ((xEventGroupValue & mainPASSENGER_INTERRUPT_BIT) != 0)
        {
            xPassengerPrevLevel = (xPassengerPrevLevel == HIGH) ? OFF : ++xPassengerPrevLevel;
            SeatInfo[PassengerTask].xCurrentLevel = xPassengerPrevLevel;
            SeatInfo[PassengerTask].ucTaskActive = (SeatInfo[PassengerTask].xCurrentLevel == OFF) ? FALSE : TRUE;
        }
    }
}

void vTempReadingTask(void *pvParameters)
{
    TaskID xxGetTaskID = (TaskID)pvParameters;
    TickType_t xPreviousWakeTime = xTaskGetTickCount();
    TickType_t xPeriodicity = pdMS_TO_TICKS(40 + (xxGetTaskID * 20));

    for (;;)
    {
        vTaskDelayUntil(&xPreviousWakeTime, xPeriodicity);
        ullResourceLockimeIn[0] = GPTM_WTimer0Read();
        if (xSemaphoreTake(xDisplayToTempTaskSync, portMAX_DELAY) == pdTRUE)
        {
            xCurrentTaskID = xxGetTaskID;
            SeatInfo[xxGetTaskID].pcCurrentSeat = (xxGetTaskID == DriverTask) ? "Driver" : "Passenger";

#if (ENABLE_DIAGONSTICS == TRUE)
            if (ADC_to_Temperature() < 5 || ADC_to_Temperature() >= 40)
            {
                GPIO_RedLedOn();
                Diagonstics[usCounter].FailureTimeStamp = GPTM_WTimer0Read();
                Diagonstics[usCounter].pcCurrentSeat = SeatInfo[xxGetTaskID].pcCurrentSeat;
                Diagonstics[usCounter].xCurrentLevel = SeatInfo[xxGetTaskID].xCurrentLevel;
                ullResourceLockimeOut[xxGetTaskID] = GPTM_WTimer0Read();
                xSemaphoreGive(xTempToDiagonsticsTaskSync);
                vTaskSuspend(NULL);
            }
            else
#endif
            {
                SeatInfo[xxGetTaskID].usCurrentTemp = ADC_to_Temperature(); // ADC_to_temp()
                ullResourceLockimeOut[xxGetTaskID] = GPTM_WTimer0Read();
                xSemaphoreGive(xTempToControlTaskSync);
            }
        }
    }
}


/* Task to control heating elements based on desired temperature */
void vControlTask(void *pvParameters)
{
    TaskID xxGetTaskID;
    TickType_t xPreviousWakeTime = xTaskGetTickCount();
    TickType_t xPeriodicity = pdMS_TO_TICKS(50);
    uint16 usDesired_Temp;
    sint16 sTemp_Diff;
    for (;;)
    {
        vTaskDelayUntil(&xPreviousWakeTime, xPeriodicity);
        xxGetTaskID = xCurrentTaskID;
        usDesired_Temp = 20 + (SeatInfo[xxGetTaskID].xCurrentLevel * 5);
        sTemp_Diff = usDesired_Temp - SeatInfo[xxGetTaskID].usCurrentTemp;

        if (SeatInfo[xxGetTaskID].usCurrentTemp >= usDesired_Temp || SeatInfo[xxGetTaskID].xCurrentLevel == OFF)
        {
            xEventGroupSetBits(xHeatingEvent, mainDISABLED_INTENSITY_BIT);
        }
        else
        {
            if (sTemp_Diff >= 10)
            {
                xEventGroupSetBits(xHeatingEvent, mainHIGH_INTENSITY_BIT);
            }
            else if (sTemp_Diff >= 5 && sTemp_Diff < 10)
            {
                xEventGroupSetBits(xHeatingEvent, mainMEDIUM_INTENSITY_BIT);
            }
            else if (sTemp_Diff >= 1 && sTemp_Diff < 5)
            {
                xEventGroupSetBits(xHeatingEvent, mainLOW_INTENSITY_BIT);
            }
        }
    }
}

/* Task to adjust heating intensity based on temperature difference */
void vHeatingElementTask(void *pvParameters)
{
    TaskID xxGetTaskID;
    EventBits_t xBitsToWaitFor = (mainDISABLED_INTENSITY_BIT | mainLOW_INTENSITY_BIT | mainMEDIUM_INTENSITY_BIT | mainHIGH_INTENSITY_BIT);
    EventBits_t xEventGroupValue;
    for (;;)
    {
        /* Block to wait for event bits to become set within the event group. */
        xEventGroupValue = xEventGroupWaitBits(xHeatingEvent,  /* The event group to read. */
                                               xBitsToWaitFor, /* Bits to test. */
                                               pdTRUE,         /* Clear bits on exit if the unblock condition is met. */
                                               pdFALSE,        /* Don't wait for all bits. */
                                               portMAX_DELAY); /* Don't time out. */
        xxGetTaskID = xCurrentTaskID;
        if ((xEventGroupValue & mainDISABLED_INTENSITY_BIT) != 0)
        {
            SeatInfo[xxGetTaskID].pcHeatIntensity = "DISABLED";
            GPIO_GreenLedOff();
            GPIO_BlueLedOff();
            GPIO_RedLedOff();
        }
        else if ((xEventGroupValue & mainLOW_INTENSITY_BIT) != 0)
        {
            SeatInfo[xxGetTaskID].pcHeatIntensity = "LOW";
            GPIO_GreenLedOn();
            GPIO_BlueLedOff();
            GPIO_RedLedOff();
        }
        else if ((xEventGroupValue & mainMEDIUM_INTENSITY_BIT) != 0)
        {
            SeatInfo[xxGetTaskID].pcHeatIntensity = "MEDIUM";
            GPIO_GreenLedOn();
            GPIO_BlueLedOn();
            GPIO_RedLedOff();
        }

        else if ((xEventGroupValue & mainHIGH_INTENSITY_BIT) != 0)
        {
            SeatInfo[xxGetTaskID].pcHeatIntensity = "HIGH";
            GPIO_GreenLedOn();
            GPIO_BlueLedOff();
            GPIO_RedLedOn();
        }
    }
}

void vDisplaytask(void *pvParameters)
{
    TaskID xGetTaskID;
    TickType_t xPreviousWakeTime = xTaskGetTickCount();
    TickType_t xPeriodicity = pdMS_TO_TICKS(80);
    for (;;)
    {
        vTaskDelayUntil(&xPreviousWakeTime, xPeriodicity);
        xGetTaskID = xCurrentTaskID;
        if (SeatInfo[xGetTaskID].ucTaskActive == TRUE) //|| SeatInfo[xGetTaskID].usCurrentTemp != 0)
        {
            ullResourceLockimeIn[2] = GPTM_WTimer0Read();
            if (xSemaphoreTake(xMutex, portMAX_DELAY) == pdTRUE)
            {
                UART0_SendString("\r\nSeat:\tCurrent Temp:\tHeating level:\tHeating Intensity:\r\n");
                UART0_SendString("--------------------------------------------------\r\n");
                UART0_SendString(SeatInfo[xGetTaskID].pcCurrentSeat);
                UART0_SendString("\t\t");
                UART0_SendInteger(SeatInfo[xGetTaskID].usCurrentTemp);
                UART0_SendString("\t\t");
                UART0_SendInteger(SeatInfo[xGetTaskID].xCurrentLevel);
                UART0_SendString("\t\t");
                UART0_SendString(SeatInfo[xGetTaskID].pcHeatIntensity);
                UART0_SendString("\r\n");
                ullResourceLockimeOut[2] = GPTM_WTimer0Read();
                xSemaphoreGive(xMutex);
            }
        }
        xSemaphoreGive(xDisplayToTempTaskSync);
    }
}

void vRunTimeMeasurementsTask(void *pvParameters)
{

    TickType_t xPreviousWakeTime = xTaskGetTickCount();
    TickType_t xPeriodicity = pdMS_TO_TICKS(450);
    for (;;)
    {
        uint8 ucCounter, ucCPU_Load;
        uint32 ullTotalTasksTime = 0;
        vTaskDelayUntil(&xPreviousWakeTime, xPeriodicity);

        for(ucCounter = 1; ucCounter < 9; ucCounter++)
        {
            ullTotalTasksTime += ullTasksTotalTime[ucCounter];
        }
        ucCPU_Load = (ullTotalTasksTime * 100) /  GPTM_WTimer0Read();

        ullResourceLockimeIn[3] = GPTM_WTimer0Read();
        if(xSemaphoreTake(xMutex,portMAX_DELAY) == pdTRUE)
        {
            taskENTER_CRITICAL();
            UART0_SendString("LevelSettingTempTask execution time:\tResource Lock Time: ");
            UART0_SendString("------------------------------------------------------------\r\n");
            UART0_SendInteger(ullTasksExecutionTime[1] / 10);
            UART0_SendString(" msec \r\n");

            UART0_SendString("ControlTask execution time:\tResource Lock Time: ");
            UART0_SendString("------------------------------------------------------------\r\n");
            UART0_SendInteger(ullTasksExecutionTime[2] / 10);
            UART0_SendString(" msec \r\n");

            UART0_SendString("DriverTempReadingTask execution time:\tResource Lock Time: ");
            UART0_SendString("------------------------------------------------------------\r\n");
            UART0_SendInteger(ullTasksExecutionTime[3] / 10);
            UART0_SendString(" msec\t\t");
            UART0_SendInteger((ullResourceLockimeOut[0] - ullResourceLockimeIn[0]) / 10);
            UART0_SendString(" msec\r\n");


            UART0_SendString("PassengerTempReadingTask execution time:\tResource Lock Time: ");
            UART0_SendString("------------------------------------------------------------\r\n");
            UART0_SendInteger(ullTasksExecutionTime[4] / 10);
            UART0_SendString(" msec\t\t");
            UART0_SendInteger((ullResourceLockimeOut[1] - ullResourceLockimeIn[1]) / 10);
            UART0_SendString(" msec\r\n");

            UART0_SendString("HeatingElementTask execution time:\tResource Lock Time: ");
            UART0_SendString("------------------------------------------------------------\r\n");
            UART0_SendInteger(ullTasksExecutionTime[5] / 10);
            UART0_SendString(" msec \r\n");

            UART0_SendString("Displaytask execution time:\tResource Lock Time: ");
            UART0_SendString("------------------------------------------------------------\r\n");
            UART0_SendInteger(ullTasksExecutionTime[6] / 10);
            UART0_SendString(" msec\t\t");
            UART0_SendInteger((ullResourceLockimeOut[2] - ullResourceLockimeIn[2]) / 10);
            UART0_SendString(" msec\r\n");

            UART0_SendString("RunTimeMeasurementsTask execution time:\tResource Lock Time: ");
            UART0_SendString("------------------------------------------------------------\r\n");
            UART0_SendInteger(ullTasksExecutionTime[7] / 10);
            UART0_SendString(" msec\t\t");
            UART0_SendInteger((ullResourceLockimeOut[3] - ullResourceLockimeIn[3]) / 10);
            UART0_SendString(" msec\r\n");

            UART0_SendString("DiagnosticsTask execution time:\tResource Lock Time: ");
            UART0_SendString("------------------------------------------------------------\r\n");
            UART0_SendInteger(ullTasksExecutionTime[8] / 10);
            UART0_SendString(" msec\t\t");
            UART0_SendInteger((ullResourceLockimeOut[4] - ullResourceLockimeIn[4]) / 10);
            UART0_SendString(" msec\r\n");


            UART0_SendString("CPU Load is ");
            UART0_SendInteger(ucCPU_Load);
            UART0_SendString("% \r\n");
            ullResourceLockimeOut[3] = GPTM_WTimer0Read();
            xSemaphoreGive(xMutex);
            taskEXIT_CRITICAL();

        }
    }


}

void vDiagonsticsTask(void *pvParameters)
{
    for (;;)
    {
        ullResourceLockimeIn[4] = GPTM_WTimer0Read();
        if (xSemaphoreTake(xTempToDiagonsticsTaskSync, portMAX_DELAY))
        {
            if (usCounter == BUFFER_SIZE - 1)
            {
                usCounter = 0;
            }
            else
            {
                UART0_SendString("\r\nTemprature Sensor Disabled:\r\n");
                UART0_SendString("\r\nFailure Time Stamp(ms):\t\tSeat:\t\tHeating Level:\r\n");
                UART0_SendString("------------------------------------------------------------\r\n");
                UART0_SendInteger(Diagonstics[usCounter].FailureTimeStamp);
                UART0_SendString("\t\t\t\t");
                UART0_SendString(Diagonstics[usCounter].pcCurrentSeat);
                UART0_SendString("\t\t");
                UART0_SendInteger(Diagonstics[usCounter].xCurrentLevel);
                UART0_SendString("\r\n");
                usCounter++;
            }
            ullResourceLockimeOut[4] = GPTM_WTimer0Read();
        }
    }
}

void GPIOPortF_Handler(void)
{
    BaseType_t pxHigherPriorityTaskWoken = pdFALSE;
    if(GPIO_PORTF_RIS_REG & (1<<0))           /* PF0 handler code Driver - Seat SW2 */
    {
        xEventGroupSetBitsFromISR(xButtonEvent, mainDRIVER_INTERRUPT_BIT,&pxHigherPriorityTaskWoken);
        GPIO_PORTF_ICR_REG   |= (1<<0);       /* Clear Trigger flag for PF0 (Interrupt Flag) */
    }
    else if(GPIO_PORTF_RIS_REG & (1<<4))      /* PF0 handler code Passenger - Seat SW1 */
    {
        xEventGroupSetBitsFromISR(xButtonEvent, mainPASSENGER_INTERRUPT_BIT,&pxHigherPriorityTaskWoken);
        GPIO_PORTF_ICR_REG   |= (1<<4);       /* Clear Trigger flag for PF4 (Interrupt Flag) */
    }
}


/*-----------------------------------------------------------*/
