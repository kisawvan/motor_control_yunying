#include "freertos_demo.h"
#include <stdio.h>

/* 启动任务相关配置 */
void startTask(void *args);                                 /* 启动任务执行函数 */
#define START_TASK_NAME "start"                             /* 启动任务的名称 */
#define START_TASK_STACK_DEPTH 128                          /* 启动任务栈大小 = 2 * 128 字节 */
#define START_TASK_PRIORITY 4                               /* 启动任务优先级 */
TaskHandle_t startTaskTCB;                                  /* 启动任务的控制块 */

/* 3个子任务相关配置 */
/* --------------------Task1-------------------- */
void taskFunc1(void *args);                                 /* 任务1执行函数 */
#define TASK_NAME_1 "task1"                                 /* 任务1的名称 */
#define TASK1_PRIORITY 1                                    /* 任务1优先级 */
TaskHandle_t task1TCB;                                      /* 任务1的控制块 */
/* --------------------Task2-------------------- */
void taskFunc2(void *args);                                 /* 任务2执行函数 */
#define TASK_NAME_2 "task2"                                 /* 任务2的名称 */
#define TASK2_PRIORITY 2                                    /* 任务2优先级 */
TaskHandle_t task2TCB;                                      /* 任务2的控制块 */


void FreeRTOS_Start(void)
{
    /* 1. 创建启动任务 */
    xTaskCreate(
        startTask,
        START_TASK_NAME,
        START_TASK_STACK_DEPTH,
        NULL,
        START_TASK_PRIORITY,
        &startTaskTCB
    );

    /* 2. 启动任务调度器 */
    vTaskStartScheduler();
}

void startTask(void *args)
{
    // 1. 代码进入临界区，防止代码中断
    vPortEnterCritical();      
    printf("执行启动任务...");   

    // 2. 创建3个任务
    xTaskCreate(                                        /* 任务1 */
        taskFunc1,
        TASK_NAME_1,
        START_TASK_STACK_DEPTH,
        NULL,
        TASK1_PRIORITY,
        &task1TCB
    );
    xTaskCreate(                                        /* 任务2 */
        taskFunc2,
        TASK_NAME_2,
        START_TASK_STACK_DEPTH,
        NULL,
        TASK2_PRIORITY,
        &task2TCB
    );
    
    // 3. 代码退出临界区
    printf("启动任务执行完毕...\n回收启动任务...");
    vPortExitCritical();

    // 4. 回收启动任务
    vTaskDelete(NULL);
}

void taskFunc1(void *args)                                  /* 任务1执行函数 */
{
    while (1)
    {
        printf("Task1...");
        vTaskDelay(500);
    }
}

void taskFunc2(void *args)                                  /* 任务2执行函数 */
{
    while (1)
    {
        printf("Task2...");
        vTaskDelay(500);
    }
    
}



extern void xPortSysTickHandler(void);
void        SysTick_Handler(void)
{
    if(xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED)
    {
        xPortSysTickHandler();
    }
}

