#include "utils/bsp_utils.h"

status_t gpio_rcc_enable(uint32_t gpiox, bool enable) {
    switch (gpiox) {
    case GPIOA_BASE:
        if (enable && __HAL_RCC_GPIOA_IS_CLK_DISABLED())
            __HAL_RCC_GPIOA_CLK_ENABLE();
        else if ((!enable) && __HAL_RCC_GPIOA_IS_CLK_ENABLED())
            __HAL_RCC_GPIOA_CLK_DISABLE();
        break;
    case GPIOB_BASE:
        if (enable && __HAL_RCC_GPIOB_IS_CLK_DISABLED())
            __HAL_RCC_GPIOB_CLK_ENABLE();
        else if ((!enable) && __HAL_RCC_GPIOB_IS_CLK_ENABLED())
            __HAL_RCC_GPIOB_CLK_DISABLE();
        break;
    case GPIOC_BASE:
        if (enable && __HAL_RCC_GPIOC_IS_CLK_DISABLED())
            __HAL_RCC_GPIOC_CLK_ENABLE();
        else if ((!enable) && __HAL_RCC_GPIOC_IS_CLK_ENABLED())
            __HAL_RCC_GPIOC_CLK_DISABLE();
        break;
    case GPIOD_BASE:
        if (enable && __HAL_RCC_GPIOD_IS_CLK_DISABLED())
            __HAL_RCC_GPIOD_CLK_ENABLE();
        else if ((!enable) && __HAL_RCC_GPIOD_IS_CLK_ENABLED())
            __HAL_RCC_GPIOD_CLK_DISABLE();
        break;
    case GPIOE_BASE:
        if (enable && __HAL_RCC_GPIOE_IS_CLK_DISABLED())
            __HAL_RCC_GPIOE_CLK_ENABLE();
        else if ((!enable) && __HAL_RCC_GPIOE_IS_CLK_ENABLED())
            __HAL_RCC_GPIOE_CLK_DISABLE();
        break;
    case GPIOF_BASE:
        if (enable && __HAL_RCC_GPIOF_IS_CLK_DISABLED())
            __HAL_RCC_GPIOF_CLK_ENABLE();
        else if ((!enable) && __HAL_RCC_GPIOF_IS_CLK_ENABLED())
            __HAL_RCC_GPIOF_CLK_DISABLE();
        break;
    case GPIOG_BASE:
        if (enable && __HAL_RCC_GPIOG_IS_CLK_DISABLED())
            __HAL_RCC_GPIOG_CLK_ENABLE();
        else if ((!enable) && __HAL_RCC_GPIOG_IS_CLK_ENABLED())
            __HAL_RCC_GPIOG_CLK_DISABLE();
        break;
    default:
        return EBADPARAM;
    }

    return NO_ERROR;
}