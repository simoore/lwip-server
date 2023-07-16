#include "stm32h7xx_hal.h"

static TIM_HandleTypeDef TimHandle;

/// Called during HAL init to initialize the tick timer.
extern "C" HAL_StatusTypeDef HAL_InitTick(uint32_t TickPriority) {
    RCC_ClkInitTypeDef clkconfig;
    uint32_t uwTimclock, uwAPB1Prescaler;
    uint32_t uwPrescalerValue;
    uint32_t pFLatency;

    // Configure the TIM6 IRQ priority
    if (TickPriority < (1UL << __NVIC_PRIO_BITS)) {
        HAL_NVIC_SetPriority(TIM6_DAC_IRQn, TickPriority, 0U);
        HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);
        uwTickPrio = TickPriority;
    } else {
        return HAL_ERROR;
    }

    __HAL_RCC_TIM6_CLK_ENABLE();
    HAL_RCC_GetClockConfig(&clkconfig, &pFLatency);
    uwAPB1Prescaler = clkconfig.APB1CLKDivider;

    if (uwAPB1Prescaler == RCC_HCLK_DIV1) {
        uwTimclock = HAL_RCC_GetPCLK1Freq();
    } else {
        uwTimclock = 2UL * HAL_RCC_GetPCLK1Freq();
    }

    // Compute the prescaler value to have TIM6 counter clock equal to 1MHz
    uwPrescalerValue = (uint32_t)((uwTimclock / 1000000U) - 1U);

    // Initialize TIMx peripheral as follow:
    // Period = [(TIM6CLK/1000) - 1]. to have a (1/1000) s time base.
    // Prescaler = (uwTimclock/1000000 - 1) to have a 1MHz counter clock.
    // ClockDivision = 0
    // Counter direction = Up
    TimHandle.Instance = TIM6;
    TimHandle.Init.Period = (1000000U / 1000U) - 1U;
    TimHandle.Init.Prescaler = uwPrescalerValue;
    TimHandle.Init.ClockDivision = 0;
    TimHandle.Init.CounterMode = TIM_COUNTERMODE_UP;
    if (HAL_TIM_Base_Init(&TimHandle) == HAL_OK) {
        return HAL_TIM_Base_Start_IT(&TimHandle);
    }
    return HAL_ERROR;
}

/// Disable the tick increment by disabling the interrupts on TIM6. 
extern "C" void HAL_SuspendTick(void) {
    __HAL_TIM_DISABLE_IT(&TimHandle, TIM_IT_UPDATE);
}

/// Resume tick increments by enabling interrupts on TIM6.
extern "C" void HAL_ResumeTick(void) {
    __HAL_TIM_ENABLE_IT(&TimHandle, TIM_IT_UPDATE);
}

/// When the period elapsed interrupt fires, increment the tick.
extern "C" void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    static_cast<void>(htim);
    HAL_IncTick();
}

/// TIM6 interrupt vector.
extern "C" void TIM6_DAC_IRQHandler(void) {
    HAL_TIM_IRQHandler(&TimHandle);
}
