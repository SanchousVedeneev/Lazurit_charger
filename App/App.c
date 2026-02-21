#include "App.h"
#include "BSP.h"
#include "Program.h"

extern bsp_dInOut_typedef bsp_dInOut_struct;

static void app_hardware_init(void);

void app_main(void)
{
  app_hardware_init();
  Program_start();
  
  while (1) // основной цикл
  {
    HAL_Delay(500);
  }
}

static void app_hardware_init(void)
{
  bsp_dInOut_readWrite_hw_start();
  HAL_Delay(10);
}








