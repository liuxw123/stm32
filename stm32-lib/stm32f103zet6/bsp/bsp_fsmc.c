#include "bsp/bsp_fsmc.h"
#include "user/logcat.h"
#include "fonts/fonts.h"
#include "user/ffile.h"

#define LOG_TAG "bsp_fsmc"


#define FSMC_DATA_PART1 __GPIO_PORT__(GPIO_BANK_D, GPIO_PIN0 | GPIO_PIN1 | GPIO_PIN8 | GPIO_PIN9 | GPIO_PIN10 | GPIO_PIN14 | GPIO_PIN15)
#define FSMC_DATA_PART2 __GPIO_PORT__(GPIO_BANK_E, GPIO_PIN7 | GPIO_PIN8 | GPIO_PIN9 | GPIO_PIN10 | GPIO_PIN11 | GPIO_PIN12 | GPIO_PIN13 | GPIO_PIN14 | GPIO_PIN15)
#define FSMC_NOE __GPIO_PORT__(GPIO_BANK_D, GPIO_PIN4)
#define FSMC_NWE __GPIO_PORT__(GPIO_BANK_D, GPIO_PIN5)
#define FSMC_RS __GPIO_PORT__(GPIO_BANK_G, GPIO_PIN0)
#define LCD_FSMC_NE4 __GPIO_PORT__(GPIO_BANK_G, GPIO_PIN12)


static void __msp_fsmc_init() {
    status_t ret = NO_ERROR;

    ret = hal_gpio_init(FSMC_DATA_PART1, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_HIGH);
    if (ret) return ret;

    ret = hal_gpio_init(FSMC_DATA_PART2, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_HIGH);
    if (ret) return ret;

    ret = hal_gpio_init(FSMC_NOE, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_HIGH);
    if (ret) return ret;

    ret = hal_gpio_init(FSMC_NWE, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_HIGH);
    if (ret) return ret;

    ret = hal_gpio_init(FSMC_RS, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_HIGH);
    if (ret) return ret;

    ret = hal_gpio_init(LCD_FSMC_NE4, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_HIGH);

    if (ret == NO_ERROR) __HAL_RCC_FSMC_CLK_ENABLE();
    return ret;
}

static void __msp_fsmc_deinit() {
    hal_gpio_deinit(FSMC_DATA_PART1);
    hal_gpio_deinit(FSMC_DATA_PART2);
    hal_gpio_deinit(FSMC_NOE);
    hal_gpio_deinit(FSMC_NWE);
    hal_gpio_deinit(FSMC_RS);
    hal_gpio_deinit(LCD_FSMC_NE4);
    __HAL_RCC_FSMC_CLK_DISABLE();
}


static status_t __lcd_fsmc_config() {
    FSMC_NORSRAM_TimingTypeDef timing = {
        .AddressSetupTime = 0x02,
        .AddressHoldTime = 0x00,
        .DataSetupTime = 0x05,
        .BusTurnAroundDuration = 0x00,
        .CLKDivision = 0x00,
        .DataLatency = 0x00,
        .AccessMode = FSMC_ACCESS_MODE_B,
    };
    SRAM_HandleTypeDef hsram4 = {
        .Instance = FSMC_NORSRAM_DEVICE,
        .Extended = FSMC_NORSRAM_EXTENDED_DEVICE,
        .Init.NSBank = FSMC_NORSRAM_BANK4,
        .Init.DataAddressMux = FSMC_DATA_ADDRESS_MUX_DISABLE,
        .Init.MemoryType = FSMC_MEMORY_TYPE_NOR,
        .Init.MemoryDataWidth = FSMC_NORSRAM_MEM_BUS_WIDTH_16,
        .Init.BurstAccessMode = FSMC_BURST_ACCESS_MODE_DISABLE,
        .Init.WaitSignalPolarity = FSMC_WAIT_SIGNAL_POLARITY_LOW,
        .Init.WrapMode = FSMC_WRAP_MODE_DISABLE,
        .Init.WaitSignalActive = FSMC_WAIT_TIMING_BEFORE_WS,
        .Init.WriteOperation = FSMC_WRITE_OPERATION_ENABLE,
        .Init.WaitSignal = FSMC_WAIT_SIGNAL_DISABLE,
        .Init.ExtendedMode = FSMC_EXTENDED_MODE_DISABLE,

        .Init.AsynchronousWait = FSMC_ASYNCHRONOUS_WAIT_DISABLE,
        .Init.WriteBurst = FSMC_WRITE_BURST_DISABLE,
        .MspInitCallback = __msp_fsmc_init,
        .MspDeInitCallback = __msp_fsmc_deinit,
    };

    if (HAL_SRAM_Init(&hsram4, &timing, NULL) != HAL_OK) { return EHW; }
    __HAL_AFIO_FSMCNADV_DISCONNECTED();

    return NO_ERROR;
}


status_t init_fsmc() {
    status_t ret = __lcd_fsmc_config();
    if (ret) {
        ALOGE("__lcd_fsmc_config failed.");
        return ret;
    }

    return ret;
}