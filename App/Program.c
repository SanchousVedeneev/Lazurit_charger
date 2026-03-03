#include "Program.h"
#include "ProtocolMbRtuSlaveCtrl.h"
#include "FlashWorker.h"

Program_typedef programStruct;

/*----------------------------- PRIVATE FCN MACRO DCL ---------------------------------*/
__STATIC_INLINE uint8_t Program_analogInit();
__STATIC_INLINE void Program_pwmInit();
__STATIC_INLINE void Program_pwmOutsControl(bsp_pwm_outs_group_typedef group, uint8_t enable);
__STATIC_INLINE void Program_regulatorInit();
__STATIC_INLINE void Program_setDout(Program_dout_typedef dout);
__STATIC_INLINE void Program_resetDout(Program_dout_typedef dout);
__STATIC_INLINE uint8_t Program_checkDin(Program_din_typedef din);
__STATIC_INLINE void Program_fastStop();
__STATIC_INLINE uint8_t Program_setError(Program_ERROR_typedef error);

#define SET_PWM_STEP_UP(IDX,VALUE_1000) bsp_pwm_set_ccrPercentX10((IDX), 1000.0f - (VALUE_1000))
#define SET_PWM_STEP_DOWN(IDX,VALUE_1000) bsp_pwm_set_ccrPercentX10((IDX), (VALUE_1000))
/*----------------------------- PRIVATE FCN MACRO END ---------------------------------*/

// --------------------- EXTERN ---------------------//
extern bsp_analogIn_typedef bsp_analogIn_struct;
// --------------------- EXTERN END---------------------//

/*----------------------------- STEPS ---------------------------------*/
#define PRG_LED_FAULT_BLINK_PERIOD (200)
__STATIC_INLINE void __stepWaitInit()
{

//---------------- переключатель
    switch (programStruct.control.target)
    {
    case target_waitOp:
        programStruct.control.step = step_init; // ------->
        break;
    
    default:
        break;
    }
//---------------- переключатель конец
}
__STATIC_INLINE void __stepInit()
{
static uint8_t ledCounter = 1;
static uint16_t timer = 0;
//------------------  LED BLINK ---------------
if (((timer++) % 30) == 0)
{
    if (ledCounter < 24)
    {
        bsp_dInOut_setDout(bsp_dInOut_led_a1_y + ledCounter);
        bsp_dInOut_resetDout(bsp_dInOut_led_a1_y + ledCounter-1);
    }
    ledCounter++;
}

if(ledCounter < 25) return;

for (uint8_t i = 0; i < 24; i++)
{
    bsp_dInOut_resetDout(bsp_dInOut_led_a1_y + i);
}
//------------------  LED BLINK END  ---------------

//---------------- переключатель
    switch (programStruct.control.target)
    {
    case target_waitOp:
        programStruct.control.step = step_wait_op; // ------->
        break;
    
    default:
        break;
    }
//---------------- переключатель конец
}
__STATIC_INLINE void __stepDebug()
{
    static uint16_t cnt_led = 0;

    if (((cnt_led++) % 100)==0)
    {
        BSP_LED_TOGGLE(BSP_LED_RDY);
    }

    bsp_dInOut_setDouts1_10(programStruct.control.remote.dout.w16);

    Program_pwmOutsControl(bsp_pwm_outs_group_123, programStruct.control.remote.pwmEnable123);
    Program_pwmOutsControl(bsp_pwm_outs_group_456, programStruct.control.remote.pwmEnable456);

    SET_PWM_STEP_DOWN (0, programStruct.control.remote.pwmArray[0]);
    SET_PWM_STEP_DOWN (1, programStruct.control.remote.pwmArray[1]);

    SET_PWM_STEP_UP (3,programStruct.control.remote.pwmArray[3]);
    SET_PWM_STEP_UP (4,programStruct.control.remote.pwmArray[4]);

//---------------- переключатель
    switch (programStruct.control.target)
    {
    case target_reset:
        programStruct.control.step = step_reset; // ------->
        break;
    
    default:
        break;
    }
//---------------- переключатель конец

}
__STATIC_INLINE void __stepReset()
{
    //HAL_UART_Transmit(&huart,buf,5,100);
    bsp_sys_reset();

}
__STATIC_INLINE void __stepError()
{
    static uint16_t timerFaultBlink = 0;

    Program_fastStop();

    if(((timerFaultBlink++)%PRG_LED_FAULT_BLINK_PERIOD)==0){
        BSP_LED_TOGGLE(BSP_LED_FAULT);
    }

    //---------------- переключатель
    switch (programStruct.control.target)
    {
    case target_debug:
        programStruct.control.step = step_debug; // ------->
        break;

    default:
        break;
    }
    //---------------- переключатель конец
}
__STATIC_INLINE void __stepWaitOp()
{
    static uint16_t cnt = 0;


    if(((cnt++)%1000)==0){
        BSP_LED_TOGGLE(BSP_LED_RDY);
    }

//---------------- переключатель
    switch (programStruct.control.target)
    {
    case target_debug:
        programStruct.control.step = step_debug; // ------->
        break;
    default:
        break;
    }
//---------------- переключатель конец

}
/*----------------------------- STEPS END ---------------------------------*/

//------------  ФУНКЦИИ   ------------//
void Program_start()
{
    protocolMbRtuSlaveCtrl_init(1);
    protocolMbRtuSlaveCtrl_init(2);

    // Загрузка параметров
    Program_ParamSetToDefault();
     if (Program_ParamLoad() == 0)
     {
         // Неудачная попытка
         asm("NOP");
     }

    Program_pwmInit();
    Program_regulatorInit();
    Program_analogInit();

    bsp_sys_tick1k_start();

    asm("NOP");
    Program_switchTarget(target_waitOp);
}

__STATIC_INLINE void Program_setDout(Program_dout_typedef dout)
{
    bsp_dInOut_setDout(bsp_dInOut_out1 + dout);
}
__STATIC_INLINE void Program_resetDout(Program_dout_typedef dout)
{
    bsp_dInOut_resetDout(bsp_dInOut_out1 + dout);
}
__STATIC_INLINE uint8_t Program_checkDin(Program_din_typedef din)
{
    return bsp_dInOut_readDin(bsp_dInOut_in1 + din);
}
__STATIC_INLINE void Program_fastStop()
{
    /* снять все импульсы, выключить все контакторы*/
    Program_pwmOutsControl(bsp_pwm_outs_group_123, 0);
    Program_pwmOutsControl(bsp_pwm_outs_group_456, 0);
    bsp_dInOut_setDouts1_10(0);
}

/* ЗАДАТЬ НАСТРОЙКИ ПО УМОЛЧАНИЮ */
void Program_ParamSetToDefault()
{
 
    // Настройка обработки аналоговых датчиков (вписать после калибровки датчиков)
    for (uint8_t i = 0; i < PRG_ANALOG_COUNT; i++)
    {
        programStruct.setupParam.analog_av_order[i] = 8;  // 1 - фильтр отключен
        programStruct.setupParam.analog_filter_N[i] = 200; // 1 - Фильтр отключен
    }

    programStruct.setupParam.analog_kMul[prg_analog_Uzpt]  = 0.001f;               
    programStruct.setupParam.analog_shift[prg_analog_Uzpt] = 1930.0f;               

    programStruct.setupParam.analog_kMul[prg_analog_Uout_power_block]  = 0.001f;    
    programStruct.setupParam.analog_shift[prg_analog_Uout_power_block] = 1930.0f;   

    programStruct.setupParam.analog_kMul[prg_analog_Uout]  = 0.001f;                
    programStruct.setupParam.analog_shift[prg_analog_Uout] = 1930.0f;               

    programStruct.setupParam.analog_kMul[prg_analog_IL3]   = 0.001f;                
    programStruct.setupParam.analog_shift[prg_analog_IL3]  = 1930.0f;               

    programStruct.setupParam.analog_kMul[prg_analog_IL4]   = 0.001f;               
    programStruct.setupParam.analog_shift[prg_analog_IL4]  = 1930.0f;              

    programStruct.setupParam.analog_kMul[prg_analog_Iout1]  = 0.001f;              
    programStruct.setupParam.analog_shift[prg_analog_Iout1] = 1930.0f;                    

    programStruct.setupParam.analog_kMul[prg_analog_Iout2]  = 0.001f;                
    programStruct.setupParam.analog_shift[prg_analog_Iout2] = 1930.0f;              

    // Настройка регуляторов (вписать после наладки преобразователя)
    programStruct.setupParam.RegU_in     = 450.0f;
    programStruct.setupParam.RegU_k_Int  = 0.001f;
    programStruct.setupParam.RegU_k_P    = 0.001f;
    programStruct.setupParam.RegU_OutMax = 0.001f;

    programStruct.setupParam.RegI_Iout_k_Int  = 0.001f;
    programStruct.setupParam.RegI_Iout_k_P    = 0.001f;
    programStruct.setupParam.RegI_Iout_OutMax = 0.001f;

    programStruct.setupParam.RegI_IL3_k_Int   = 0.001f;
    programStruct.setupParam.RegI_IL3_k_P     = 0.001f;
    programStruct.setupParam.RegI_IL3_OutMax  = 0.001f;

    programStruct.setupParam.RegI_IL4_k_Int   = 0.001f;
    programStruct.setupParam.RegI_IL4_k_P     = 0.001f;
    programStruct.setupParam.RegI_IL4_OutMax  = 0.001f;

    programStruct.setupParam.ZI_Iout_settings = 5.0f;

    // Настройки зарядника
    programStruct.setupParam.f_PWM = 4000;

    // Настройки проверок на ошибки
    programStruct.setupParam.check_Uzpt_low   = 470.0f;
    programStruct.setupParam.check_Uzpt_high  = 630.0f;
    programStruct.setupParam.check_IL3_high   = 140.0f;
    programStruct.setupParam.check_IL4_high   = 140.0f;
    programStruct.setupParam.check_Iout1_high = 140.0f;
    programStruct.setupParam.check_Iout2_high = 140.0f;
    programStruct.setupParam.check_Uout_high  = 720.0f;
    programStruct.setupParam.check_Uakb_no    = 300.0f;
    programStruct.setupParam.check_Uakb_no    = 690.0f;
}

#define PROGRAM_PARAM_SIZE_BYTE sizeof(Program_PARAM_typedef)
uint8_t Program_ParamLoad()
{
   programStruct.sys.flash_counter = FlashWorker_getAvailableRecords(PROGRAM_PARAM_SIZE_BYTE);
   return FlashWorker_load((void*)(&programStruct.setupParam),PROGRAM_PARAM_SIZE_BYTE);
}

#define SAVE_COUNT_MAX_PER_SESSION (10)
uint8_t Program_ParamSave()
{
    static uint8_t saveCounter = 0;

    if (++saveCounter >= SAVE_COUNT_MAX_PER_SESSION)
    {
        return 0;
    }

    if (programStruct.control.step == step_debug)
    {
        FlashWorker_save((void *)(&programStruct.setupParam), PROGRAM_PARAM_SIZE_BYTE);
        programStruct.sys.flash_counter = FlashWorker_getAvailableRecords(PROGRAM_PARAM_SIZE_BYTE);
        return 1;
    }
    return 0;
}

__INLINE uint8_t Program_GoDebug()
{
    if (
        (programStruct.control.step != step_wait_op) &&
        (programStruct.control.step != step_error))
    {
        return 0;
    }
    /*
        Перед отладкой обнулить REMOTE структуру!
    */
    programStruct.control.remote.dout.w16 = 0;
    for (uint8_t i = 0; i < 6; i++)
    {
        programStruct.control.remote.pwmArray[i] = 0;
    }

    programStruct.control.remote.pwmEnable123 = 0;
    programStruct.control.remote.pwmEnable456 = 0;


   /*
        Перед отладкой обнулить REMOTE структуру!
    */
    Program_switchTarget(target_debug);
    return 1;
}

__INLINE uint8_t Program_set_dout_debug(uint16_t douts)
{
    if(programStruct.control.step == step_debug){

        programStruct.control.remote.dout.w16 = douts;
        return 1;
    }
    return 0;
}

#define PWM_REMOTE_MAX_PERCENT_STEP_UP_X10 (250)
#define PWM_REMOTE_MAX_PERCENT_STEP_DOWN_X10 (1000)
__INLINE uint8_t Program_set_pwm_debug(uint8_t channel_IDx, uint16_t pwm1000Perc)
{
    if (channel_IDx > 5)
    {
        return 0;
    }
        
    if (programStruct.control.step == step_debug)
    {
        if (channel_IDx < 3)
        {
            if (pwm1000Perc > PWM_REMOTE_MAX_PERCENT_STEP_DOWN_X10)
            {
                pwm1000Perc = PWM_REMOTE_MAX_PERCENT_STEP_DOWN_X10;
            }  
        }
        else
        {
            if (pwm1000Perc > PWM_REMOTE_MAX_PERCENT_STEP_UP_X10)
            {
                pwm1000Perc = PWM_REMOTE_MAX_PERCENT_STEP_UP_X10;
            }   
        }
        programStruct.control.remote.pwmArray[channel_IDx] = pwm1000Perc;
        return 1;
    }
    return 0;
}

#define PROGRAM_SETUP_PWM_3500HZ (3500)
#define PROGRAM_SETUP_PWM_4000HZ (4000)
#define PROGRAM_SETUP_PWM_4200HZ (4200)
#define PROGRAM_SETUP_PWM_4800HZ (4800)
#define PROGRAM_SETUP_PWM_5000HZ (5000)
#define PROGRAM_SETUP_PWM_5600HZ (5600)
uint8_t Program_set_PWM(uint16_t value)
{
    if (programStruct.control.step != step_debug)
    {
        return 0;
    }

    switch (value)
    {
    case PROGRAM_SETUP_PWM_3500HZ:
        programStruct.setupParam.f_PWM = PROGRAM_SETUP_PWM_3500HZ;
        break;
    case PROGRAM_SETUP_PWM_4000HZ:
        programStruct.setupParam.f_PWM = PROGRAM_SETUP_PWM_4000HZ;
        break;
    case PROGRAM_SETUP_PWM_4200HZ:
        programStruct.setupParam.f_PWM = PROGRAM_SETUP_PWM_4200HZ;
        break;
    case PROGRAM_SETUP_PWM_4800HZ:
        programStruct.setupParam.f_PWM = PROGRAM_SETUP_PWM_4800HZ;
        break;
    case PROGRAM_SETUP_PWM_5000HZ:
        programStruct.setupParam.f_PWM = PROGRAM_SETUP_PWM_5000HZ;
        break;
    case PROGRAM_SETUP_PWM_5600HZ:
        programStruct.setupParam.f_PWM = PROGRAM_SETUP_PWM_5600HZ;
        break;
    default:
        programStruct.setupParam.f_PWM = PROGRAM_SETUP_PWM_4000HZ;
        break;
    }
    return 1;
}

#define PROGRAM_SETUP_REGU_IN_MIN (100) // для отладки (после отладки повысить до 420)
#define PROGRAM_SETUP_REGU_IN_MAX (690)
uint8_t Program_setup_RegU_in(uint16_t value)
{
    if (programStruct.control.step != step_debug)
    {
        return 0;
    }

    if (value < PROGRAM_SETUP_REGU_IN_MIN)
    {
        value = PROGRAM_SETUP_REGU_IN_MIN;
    }
    else if (value > PROGRAM_SETUP_REGU_IN_MAX)
    {
        value = PROGRAM_SETUP_REGU_IN_MAX;
    }
    programStruct.setupParam.RegU_in = value;
    return 1;
}

#define PROGRAM_SETUP_REGU_K_INT_MIN (0.001f)
#define PROGRAM_SETUP_REGU_K_INT_MAX (20.0f)
uint8_t Program_setup_RegU_k_Int(float value)
{
    if (programStruct.control.step != step_debug)
    {
        return 0;
    }

    if (value < PROGRAM_SETUP_REGU_K_INT_MIN)
    {
        value = PROGRAM_SETUP_REGU_K_INT_MIN;
    }
    else if (value > PROGRAM_SETUP_REGU_K_INT_MAX)
    {
        value = PROGRAM_SETUP_REGU_K_INT_MAX;
    }
    programStruct.setupParam.RegU_k_Int = value;
    return 1;
}

#define PROGRAM_SETUP_REGU_K_P_MIN (0.001f)
#define PROGRAM_SETUP_REGU_K_P_MAX (2.0f)
uint8_t Program_setup_RegU_k_P(float value)
{
    if (programStruct.control.step != step_debug)
    {
        return 0;
    }

    if (value < PROGRAM_SETUP_REGU_K_P_MIN)
    {
        value = PROGRAM_SETUP_REGU_K_P_MIN;
    }
    else if (value > PROGRAM_SETUP_REGU_K_P_MAX)
    {
        value = PROGRAM_SETUP_REGU_K_P_MAX;
    }
    programStruct.setupParam.RegU_k_P = value;
    return 1;
}

#define PROGRAM_SETUP_REGU_OUTMAX_MIN (10)
#define PROGRAM_SETUP_REGU_OUTMAX_MAX (250)
uint8_t Program_setup_RegU_OutMax(uint16_t value)
{
    if (programStruct.control.step != step_debug)
    {
        return 0;
    }

    if (value < PROGRAM_SETUP_REGU_OUTMAX_MIN)
    {
        value = PROGRAM_SETUP_REGU_OUTMAX_MIN;
    }
    else if (value > PROGRAM_SETUP_REGU_OUTMAX_MAX)
    {
        value = PROGRAM_SETUP_REGU_OUTMAX_MAX;
    }
    programStruct.setupParam.RegU_OutMax = value;
    return 1;
}

#define PROGRAM_SETUP_REGI_IOUT_K_INT_MIN (0.001f)
#define PROGRAM_SETUP_REGI_IOUT_K_INT_MAX (20.0f)
uint8_t Program_setup_RegI_Iout_k_Int(float value)
{
    if (programStruct.control.step != step_debug)
    {
        return 0;
    }

    if (value < PROGRAM_SETUP_REGI_IOUT_K_INT_MIN)
    {
        value = PROGRAM_SETUP_REGI_IOUT_K_INT_MIN;
    }
    else if (value > PROGRAM_SETUP_REGI_IOUT_K_INT_MAX)
    {
        value = PROGRAM_SETUP_REGI_IOUT_K_INT_MAX;
    }
    programStruct.setupParam.RegI_Iout_k_Int = value;
    return 1;
}

#define PROGRAM_SETUP_REGI_IOUT_K_P_MIN (0.001f)
#define PROGRAM_SETUP_REGI_IOUT_K_P_MAX (2.0f)
uint8_t Program_setup_RegI_Iout_k_P(float value)
{
    if (programStruct.control.step != step_debug)
    {
        return 0;
    }

    if (value < PROGRAM_SETUP_REGI_IOUT_K_P_MIN)
    {
        value = PROGRAM_SETUP_REGI_IOUT_K_P_MIN;
    }
    else if (value > PROGRAM_SETUP_REGI_IOUT_K_P_MAX)
    {
        value = PROGRAM_SETUP_REGI_IOUT_K_P_MAX;
    }
    programStruct.setupParam.RegI_Iout_k_P = value;
    return 1;
}

#define PROGRAM_SETUP_REGI_IOUT_MAX_MIN (0)
#define PROGRAM_SETUP_REGI_IOUT_MAX_MAX (250)
uint8_t Program_setup_RegI_Iout_OutMax(uint16_t value)
{
    if (programStruct.control.step != step_debug)
    {
        return 0;
    }

    if (value < PROGRAM_SETUP_REGI_IOUT_MAX_MIN)
    {
        value = PROGRAM_SETUP_REGI_IOUT_MAX_MIN;
    }
    else if (value > PROGRAM_SETUP_REGI_IOUT_MAX_MAX)
    {
        value = PROGRAM_SETUP_REGI_IOUT_MAX_MAX;
    }
    programStruct.setupParam.RegI_Iout_OutMax = value;
    return 1;
}

#define PROGRAM_SETUP_REGI_IL_K_INT_MIN (0.001f)
#define PROGRAM_SETUP_REGI_IL_K_INT_MAX (20.0f)
uint8_t Program_setup_RegI_IL3_k_Int(float value)
{
    if (programStruct.control.step != step_debug)
    {
        return 0;
    }

    if (value < PROGRAM_SETUP_REGI_IL_K_INT_MIN)
    {
        value = PROGRAM_SETUP_REGI_IL_K_INT_MIN;
    }
    else if (value > PROGRAM_SETUP_REGI_IL_K_INT_MAX)
    {
        value = PROGRAM_SETUP_REGI_IL_K_INT_MAX;
    }
    programStruct.setupParam.RegI_IL3_k_Int = value;
    return 1;
}

#define PROGRAM_SETUP_REGI_IL_K_P_MIN (0.001f)
#define PROGRAM_SETUP_REGI_IL_K_P_MAX (2.0f)
uint8_t Program_setup_RegI_IL3_k_P(float value)
{
    if (programStruct.control.step != step_debug)
    {
        return 0;
    }

    if (value < PROGRAM_SETUP_REGI_IL_K_P_MIN)
    {
        value = PROGRAM_SETUP_REGI_IL_K_P_MIN;
    }
    else if (value > PROGRAM_SETUP_REGI_IL_K_P_MAX)
    {
        value = PROGRAM_SETUP_REGI_IL_K_P_MAX;
    }
    programStruct.setupParam.RegI_IL3_k_P = value;
    return 1;
}

#define PROGRAM_SETUP_REGI_IL_OUTMAX_MIN (0)
#define PROGRAM_SETUP_REGI_IL_OUTMAX_MAX (1250)
uint8_t Program_setup_RegI_IL3_OutMax(uint16_t value)
{
    if (programStruct.control.step != step_debug)
    {
        return 0;
    }

    if (value < PROGRAM_SETUP_REGI_IL_OUTMAX_MIN)
    {
        value = PROGRAM_SETUP_REGI_IL_OUTMAX_MIN;
    }
    else if (value > PROGRAM_SETUP_REGI_IL_OUTMAX_MAX)
    {
        value = PROGRAM_SETUP_REGI_IL_OUTMAX_MAX;
    }
    programStruct.setupParam.RegI_IL3_OutMax = value;
    return 1;
}

uint8_t Program_setup_RegI_IL4_k_Int(float value)
{
    if (programStruct.control.step != step_debug)
    {
        return 0;
    }

    if (value < PROGRAM_SETUP_REGI_IL_K_INT_MIN)
    {
        value = PROGRAM_SETUP_REGI_IL_K_INT_MIN;
    }
    else if (value > PROGRAM_SETUP_REGI_IL_K_INT_MAX)
    {
        value = PROGRAM_SETUP_REGI_IL_K_INT_MAX;
    }
    programStruct.setupParam.RegI_IL4_k_Int = value;
    return 1;
}
uint8_t Program_setup_RegI_IL4_k_P(float value)
{
    if (programStruct.control.step != step_debug)
    {
        return 0;
    }

    if (value < PROGRAM_SETUP_REGI_IL_K_P_MIN)
    {
        value = PROGRAM_SETUP_REGI_IL_K_P_MIN;
    }
    else if (value > PROGRAM_SETUP_REGI_IL_K_P_MAX)
    {
        value = PROGRAM_SETUP_REGI_IL_K_P_MAX;
    }
    programStruct.setupParam.RegI_IL4_k_P = value;
    return 1;
}
uint8_t Program_setup_RegI_IL4_OutMax(uint16_t value)
{
    if (programStruct.control.step != step_debug)
    {
        return 0;
    }

    if (value < PROGRAM_SETUP_REGI_IL_OUTMAX_MIN)
    {
        value = PROGRAM_SETUP_REGI_IL_OUTMAX_MIN;
    }
    else if (value > PROGRAM_SETUP_REGI_IL_OUTMAX_MAX)
    {
        value = PROGRAM_SETUP_REGI_IL_OUTMAX_MAX;
    }
    programStruct.setupParam.RegI_IL4_OutMax = value;
    return 1;
}

#define PROGRAM_SETUP_ZI_I_OUT_MIN (5)
#define PROGRAM_SETUP_ZI_I_OUT_MAX (40)
uint8_t Program_setup_ZI_Iout_settings(uint16_t value)
{
    if (programStruct.control.step != step_debug)
    {
        return 0;
    }

     if (value < PROGRAM_SETUP_ZI_I_OUT_MIN)
    {
        value = PROGRAM_SETUP_ZI_I_OUT_MIN;
    }
    else if (value > PROGRAM_SETUP_ZI_I_OUT_MAX)
    {
        value = PROGRAM_SETUP_ZI_I_OUT_MAX;
    }
    programStruct.setupParam.ZI_Iout_settings = value;
    return 1;
}

#define PROGRAM_SETUP_UZPT_LOW_MIN (450)
#define PROGRAM_SETUP_UZPT_LOW_MAX (520)
uint8_t Program_check_Uzpt_low(uint16_t value)
{
    if (programStruct.control.step != step_debug)
    {
        return 0;
    }

    if (value < PROGRAM_SETUP_UZPT_LOW_MIN)
    {
        value = PROGRAM_SETUP_UZPT_LOW_MIN;
    }
    else if (value > PROGRAM_SETUP_UZPT_LOW_MAX)
    {
        value = PROGRAM_SETUP_UZPT_LOW_MAX;
    }
    programStruct.setupParam.check_Uzpt_low = value;
    return 1;
}

#define PROGRAM_SETUP_UZPT_HIGH_MIN (570)
#define PROGRAM_SETUP_UZPT_HIGH_MAX (650)
uint8_t Program_check_Uzpt_high(uint16_t value)
{
    if (programStruct.control.step != step_debug)
    {
        return 0;
    }

    if (value < PROGRAM_SETUP_UZPT_HIGH_MIN)
    {
        value = PROGRAM_SETUP_UZPT_HIGH_MIN;
    }
    else if (value > PROGRAM_SETUP_UZPT_HIGH_MAX)
    {
        value = PROGRAM_SETUP_UZPT_HIGH_MAX;
    }
    programStruct.setupParam.check_Uzpt_high = value;
    return 1;
}

#define PROGRAM_SETUP_IL_HIGH_MIN (50)
#define PROGRAM_SETUP_IL_HIGH_MAX (145)
uint8_t Program_check_IL3_high(uint16_t value)
{
    if (programStruct.control.step != step_debug)
    {
        return 0;
    }

    if (value < PROGRAM_SETUP_IL_HIGH_MIN)
    {
        value = PROGRAM_SETUP_IL_HIGH_MIN;
    }
    else if (value > PROGRAM_SETUP_IL_HIGH_MAX)
    {
        value = PROGRAM_SETUP_IL_HIGH_MAX;
    }
    programStruct.setupParam.check_IL3_high = value;
    return 1;
}
uint8_t Program_check_IL4_high(uint16_t value)
{
    if (programStruct.control.step != step_debug)
    {
        return 0;
    }

    if (value < PROGRAM_SETUP_IL_HIGH_MIN)
    {
        value = PROGRAM_SETUP_IL_HIGH_MIN;
    }
    else if (value > PROGRAM_SETUP_IL_HIGH_MAX)
    {
        value = PROGRAM_SETUP_IL_HIGH_MAX;
    }
    programStruct.setupParam.check_IL4_high = value;
    return 1;
}

#define PROGRAM_SETUP_IOUT12_HIGH_MIN (50)
#define PROGRAM_SETUP_IOUT12_HIGH_MAX (145)
uint8_t Program_check_Iout1_high(uint16_t value)
{
    if (programStruct.control.step != step_debug)
    {
        return 0;
    }

    if (value < PROGRAM_SETUP_IOUT12_HIGH_MIN)
    {
        value = PROGRAM_SETUP_IOUT12_HIGH_MIN;
    }
    else if (value > PROGRAM_SETUP_IOUT12_HIGH_MAX)
    {
        value = PROGRAM_SETUP_IOUT12_HIGH_MAX;
    }
    programStruct.setupParam.check_Iout1_high = value;
    return 1;
}
uint8_t Program_check_Iout2_high(uint16_t value)
{
    if (programStruct.control.step != step_debug)
    {
        return 0;
    }

    if (value < PROGRAM_SETUP_IOUT12_HIGH_MIN)
    {
        value = PROGRAM_SETUP_IOUT12_HIGH_MIN;
    }
    else if (value > PROGRAM_SETUP_IOUT12_HIGH_MAX)
    {
        value = PROGRAM_SETUP_IOUT12_HIGH_MAX;
    }
    programStruct.setupParam.check_Iout2_high = value;
    return 1;
}

#define PROGRAM_SETUP_UOUT_HIGH_MIN (650)
#define PROGRAM_SETUP_UOUT_HIGH_MAX (730)
uint8_t Program_check_Uout_high(uint16_t value)
{
    if (programStruct.control.step != step_debug)
    {
        return 0;
    }

    if (value < PROGRAM_SETUP_UOUT_HIGH_MIN)
    {
        value = PROGRAM_SETUP_UOUT_HIGH_MIN;
    }
    else if (value > PROGRAM_SETUP_UOUT_HIGH_MAX)
    {
        value = PROGRAM_SETUP_UOUT_HIGH_MAX;
    }
    programStruct.setupParam.check_Uout_high = value;
    return 1;
}

#define PROGRAM_SETUP_UAKB_NO_MIN (20)
#define PROGRAM_SETUP_UAKB_NO_MAX (690)
uint8_t Program_check_Uakb_no(uint16_t value)
{
    if (programStruct.control.step != step_debug)
    {
        return 0;
    } 

    if (value < PROGRAM_SETUP_UAKB_NO_MIN)
    {
        value = PROGRAM_SETUP_UAKB_NO_MIN;
    }
    else if (value > PROGRAM_SETUP_UAKB_NO_MAX)
    {
        value = PROGRAM_SETUP_UAKB_NO_MAX;
    }
    programStruct.setupParam.check_Uakb_no = value;
    return 1;
}

#define PROGRAM_SETUP_UAKB_HIGH_MIN (420)
#define PROGRAM_SETUP_UAKB_HIGH_MAX (690)
uint8_t Program_check_Uakb_high(uint16_t value)
{
    if (programStruct.control.step != step_debug)
    {
        return 0;
    } 

    if (value < PROGRAM_SETUP_UAKB_HIGH_MIN)
    {
        value = PROGRAM_SETUP_UAKB_HIGH_MIN;
    }
    else if (value > PROGRAM_SETUP_UAKB_HIGH_MAX)
    {
        value = PROGRAM_SETUP_UAKB_HIGH_MAX;
    }
    programStruct.setupParam.check_Uakb_high = value;
    return 1;
}

__INLINE uint8_t Program_GoReset()
{
    if(programStruct.control.step == step_debug)
    {
        Program_switchTarget(target_reset);
        return 1;
    }
    return 0;
}

__INLINE uint8_t Program_LoadDefaultParam_debug()
{
    if (programStruct.control.step == step_debug)
    {
        Program_ParamSetToDefault();
        return 1;
    }
    return 0;
}

__STATIC_INLINE uint8_t Program_setError(Program_ERROR_typedef error)
{
    if (error == error_noError)
    {
        return 0;
    }
        
    if (programStruct.setupParam.protect_control & (uint16_t)(1 << (error - 1)))
    {
        return 0;
    }
    programStruct.control.errorCode = error;
    Program_switchTarget(target_error);
    return 1;
}

__INLINE void Program_switchTarget(Program_TARGET_typedef newTarget)
{
    programStruct.control.target = newTarget;
}
//------------  ФУНКЦИИ КОНЕЦ ------------//

//------------   Задача 1 кГц   ------------//
#define PRG_UPDATE_MDB (50)
void bsp_sys_tick_1k_callback()
{
    // static uint16_t count_1k_mdb = 0;
    static uint16_t timer_mdb = 0;

    // Проверка нажатия Аварийного стопа
    // В отладочном режиме аварийный стоп не работает!
    // if ((Program_checkDin(prg_din3_ALARM_STOP) == PRG_DIN_ALARM_STOP_VAL) &&
    //     (programStruct.control.target != target_debug))
    // {
    //     if (Program_setError(error_fastStop))
    //     {
    //         programStruct.control.step = step_error;
    //     }
    // }


    if (++timer_mdb > PRG_UPDATE_MDB)
    {
        protocolMbRtuSlaveCtrl_update_tables();
        timer_mdb = 0;
    }
    
    
    // // Формируем и сбрасываем ошибку связи Modbus с панелью (возможно стоит переделать)
    // if(programStruct.PanParam.mdb_rx == PAN_MDB_STATE_ERR)
    // {
    //     if(count_1k_mdb >= (programStruct.setupParam.timeout_mdb_pan*1000))
    //         programStruct.PanParam.mdb_state = PAN_MDB_STATE_ERR;
    //     else
    //         count_1k_mdb++;
    // }
    // else if(programStruct.PanParam.mdb_rx == PAN_MDB_STATE_OK)
    // {
    //     programStruct.PanParam.mdb_state = PAN_MDB_STATE_OK;
    //     count_1k_mdb = 0;
    // }
    // programStruct.PanParam.mdb_rx = PAN_MDB_STATE_ERR;

    asm("NOP");
    switch (programStruct.control.step)
    {
    case step_waitInit:
        __stepWaitInit();
        break;
    case step_init:
        __stepInit();
        break;
    case step_wait_op:
        __stepWaitOp();
        break;
    case step_debug:
        __stepDebug();
        break;
    case step_reset:
        __stepReset();
        break;
    case step_error:
        __stepError();
        break;
    default:
        break;
    }
}
//------------   Задача 1 кГц КОНЕЦ  ------------//

//----------------------- PWM ----------------------
__STATIC_INLINE void Program_pwmOutsControl(bsp_pwm_outs_group_typedef group, uint8_t enable)
{
    if (enable == 0)
    {
        bsp_pwm_disable_outs_VT(group);
        return;
    }

    if (group == bsp_pwm_outs_group_123)
    {
        bsp_pwm_enable_out_VT_CONCEPT(0, bsp_pwm_outs_type_high);
        bsp_pwm_enable_out_VT_CONCEPT(1, bsp_pwm_outs_type_high);
    }
    else
    {
        bsp_pwm_enable_out_VT_CONCEPT(3, bsp_pwm_outs_type_low);
        bsp_pwm_enable_out_VT_CONCEPT(4, bsp_pwm_outs_type_low);
    }
}

__STATIC_INLINE void Program_pwmInit()
{
  bsp_pwm_set_tim (bsp_pwm_tim_mode_up, 200, BSP_PWM_POSITIVE_POLARITY);

  switch (programStruct.setupParam.f_PWM)
  {
  case PROGRAM_SETUP_PWM_3500HZ:
    bsp_pwm_set_freq (bsp_pwm_outs_group_123, bsp_pwm_freq_3500_hz, 1);
    bsp_pwm_set_freq (bsp_pwm_outs_group_456, bsp_pwm_freq_3500_hz, 1);
    break;
  case PROGRAM_SETUP_PWM_4000HZ:
    bsp_pwm_set_freq (bsp_pwm_outs_group_123, bsp_pwm_freq_4000_hz, 1);
    bsp_pwm_set_freq (bsp_pwm_outs_group_456, bsp_pwm_freq_4000_hz, 1);
    break;
  case PROGRAM_SETUP_PWM_4200HZ:
    bsp_pwm_set_freq (bsp_pwm_outs_group_123, bsp_pwm_freq_4200_hz, 1);
    bsp_pwm_set_freq (bsp_pwm_outs_group_456, bsp_pwm_freq_4200_hz, 1);
    break;
  case PROGRAM_SETUP_PWM_4800HZ:
    bsp_pwm_set_freq (bsp_pwm_outs_group_123, bsp_pwm_freq_4800_hz, 1);
    bsp_pwm_set_freq (bsp_pwm_outs_group_456, bsp_pwm_freq_4800_hz, 1);
    break;
  case PROGRAM_SETUP_PWM_5000HZ:
    bsp_pwm_set_freq (bsp_pwm_outs_group_123, bsp_pwm_freq_5000_hz, 1);
    bsp_pwm_set_freq (bsp_pwm_outs_group_456, bsp_pwm_freq_5000_hz, 1);
    break;
  case PROGRAM_SETUP_PWM_5600HZ:
    bsp_pwm_set_freq (bsp_pwm_outs_group_123, bsp_pwm_freq_5000_hz, 1);
    bsp_pwm_set_freq (bsp_pwm_outs_group_456, bsp_pwm_freq_5000_hz, 1);
    break;
  default:
    bsp_pwm_set_freq (bsp_pwm_outs_group_123, bsp_pwm_freq_4000_hz, 1);
    bsp_pwm_set_freq (bsp_pwm_outs_group_456, bsp_pwm_freq_4000_hz, 1);
    break;
  }

  SET_PWM_STEP_DOWN(0, 0.0f);
  SET_PWM_STEP_DOWN(1, 0.0f);

  SET_PWM_STEP_UP(3, 0.0f);
  SET_PWM_STEP_UP(4, 0.0f);

  bsp_pwm_start_IRQ_123_IRQ_456();
}

__STATIC_INLINE void Program_regulatorInit()
{
    // Регулятор напряжения
    programStruct.control.sau.RegU.In     = programStruct.setupParam.RegU_in;
    programStruct.control.sau.RegU.k_Int  = programStruct.setupParam.RegU_k_Int;
    programStruct.control.sau.RegU.k_P    = programStruct.setupParam.RegU_k_P;
    programStruct.control.sau.RegU.IntMax = programStruct.setupParam.RegU_OutMax;
    programStruct.control.sau.RegU.IntMin = 0.0f;
    programStruct.control.sau.RegU.OutMax = programStruct.setupParam.RegU_OutMax;
    programStruct.control.sau.RegU.OutMin = 0.0f;
    programStruct.control.sau.RegU.period = 1.0f / (float)programStruct.setupParam.f_PWM;

    // Регулятор тока Iout
    programStruct.control.sau.RegI[Iout].k_Int  = programStruct.setupParam.RegI_Iout_k_Int;
    programStruct.control.sau.RegI[Iout].k_P    = programStruct.setupParam.RegI_Iout_k_P;
    programStruct.control.sau.RegI[Iout].IntMax = programStruct.setupParam.RegI_Iout_OutMax;
    programStruct.control.sau.RegI[Iout].IntMin = 0.0f;
    programStruct.control.sau.RegI[Iout].OutMax = programStruct.setupParam.RegI_Iout_OutMax;
    programStruct.control.sau.RegI[Iout].OutMin = 0.0f;
    programStruct.control.sau.RegI[Iout].period = 1.0f / (float)programStruct.setupParam.f_PWM;

    // Регулятор тока IL3
    programStruct.control.sau.RegI[IL3].k_Int  = programStruct.setupParam.RegI_IL3_k_Int;
    programStruct.control.sau.RegI[IL3].k_P    = programStruct.setupParam.RegI_IL3_k_P;
    programStruct.control.sau.RegI[IL3].IntMax = programStruct.setupParam.RegI_IL3_OutMax;
    programStruct.control.sau.RegI[IL3].IntMin = 0.0f;
    programStruct.control.sau.RegI[IL3].OutMax = programStruct.setupParam.RegI_IL3_OutMax;
    programStruct.control.sau.RegI[IL3].OutMin = 0.0f;
    programStruct.control.sau.RegI[IL3].period = 1.0f / (float)programStruct.setupParam.f_PWM;

    // Регулятор тока IL4
    programStruct.control.sau.RegI[IL4].k_Int  = programStruct.setupParam.RegI_IL4_k_Int;
    programStruct.control.sau.RegI[IL4].k_P    = programStruct.setupParam.RegI_IL4_k_P;
    programStruct.control.sau.RegI[IL4].IntMax = programStruct.setupParam.RegI_IL4_OutMax;
    programStruct.control.sau.RegI[IL4].IntMin = 0.0f;
    programStruct.control.sau.RegI[IL4].OutMax = programStruct.setupParam.RegI_IL4_OutMax;
    programStruct.control.sau.RegI[IL4].OutMin = 0.0f;
    programStruct.control.sau.RegI[IL4].period = 1.0f / (float)programStruct.setupParam.f_PWM;

    // Задачтик интенсивности 
    programStruct.control.sau.ZI_Iout.period = 1.0f / (float)programStruct.setupParam.f_PWM;
    programStruct.control.sau.ZI_Iout.settings = programStruct.setupParam.ZI_Iout_settings;
    dsp_intensSetterSetup(&programStruct.control.sau.ZI_Iout);

    asm("Nop");
}

uint8_t Program_set_pwmOuts_debug(bsp_pwm_outs_group_typedef group, uint8_t onOff)
{
    if (programStruct.control.step != step_debug)
    {
        return 0;
    }

    if (group == bsp_pwm_outs_group_123)
    {
        programStruct.control.remote.pwmEnable123 = onOff;
    }
    else if (group == bsp_pwm_outs_group_456)
    {
        programStruct.control.remote.pwmEnable456 = onOff;
    }
    else
    {
        return 0;
    }
    return 1;
}

void bsp_pwm_123_callback()
{
    if (programStruct.control.step == step_debug)
    {
        return;
    }
}
//----------------------- PWM END----------------------

//------------   АЦП   ------------//
Program_AIN_typedef* Program_analogGetByIdx(Program_ANALOG_ENUM_typedef idx)
{
    if (idx >= PRG_ANALOG_COUNT)
    {
        return NULL;
    }
    return &programStruct.analog.aIn[idx];
}

uint8_t Program_analogSetZero(Program_ANALOG_ENUM_typedef idx)
{  
    if (programStruct.control.step != step_debug)
    {
        return 0;
    }
    
    if (idx >= PRG_ANALOG_COUNT)
    {
        return 0;
    }

    programStruct.analog.aIn[idx].shift = programStruct.analog.aIn[idx].valueRaw;
    programStruct.setupParam.analog_shift[idx] = programStruct.analog.aIn[idx].shift;
    return 1;
}

uint8_t Program_analogCalibKMul(Program_ANALOG_ENUM_typedef idx, float value)
{
    if (programStruct.control.step != step_debug)
    {
        return 0;
    }

    if (idx >= PRG_ANALOG_COUNT)
    {
        return 0;
    }

    float kMul = programStruct.analog.aIn[idx].kMul;

    if (kMul == 0.0f) 
    {
        return 0;
    }
    float currentVal = programStruct.analog.aIn[idx].value / kMul;

    programStruct.analog.aIn[idx].kMul = value / currentVal;
    programStruct.setupParam.analog_kMul[idx] = programStruct.analog.aIn[idx].kMul;
    return 1;
}

uint8_t Program_analogSetShift(Program_ANALOG_ENUM_typedef idx, float value)
{
    if (programStruct.control.step != step_debug)
    {
        return 0;
    }

    if (idx >= PRG_ANALOG_COUNT)
    {
        return 0;
    }

    programStruct.setupParam.analog_shift[idx] = value;
    return 1;
}

uint8_t Program_analogSetKMul(Program_ANALOG_ENUM_typedef idx, float value)
{

    if (programStruct.control.step != step_debug)
    {
        return 0;
    }

    if (idx >= PRG_ANALOG_COUNT)
    {
        return 0;
    }

    programStruct.setupParam.analog_kMul[idx] = value;
    return 1;
}

uint8_t Program_analogSetAvOrder(Program_ANALOG_ENUM_typedef idx, uint8_t order)
{
    if (programStruct.control.step != step_debug)
    {
        return 0;
    }

    if (idx >= PRG_ANALOG_COUNT)
    {
        return 0;
    }

    if (order < 1) 
    {
        order = 1;
    }
    else if (order > PROGRAM_ADC_MAX_FILTER_ORDER)
    {
        order = PROGRAM_ADC_MAX_FILTER_ORDER;
    }
    programStruct.setupParam.analog_av_order[idx] = order;
    return 1;
}

uint8_t Program_analogSetFilterN(Program_ANALOG_ENUM_typedef idx, uint16_t filterN)
{

    if (programStruct.control.step != step_debug)
    {
        return 0;
    }

    if (idx >= PRG_ANALOG_COUNT)
    {
        return 0;
    }

    if(filterN < 1 ) 
    {
        filterN = 1;
    }
    else if (filterN > PROGRAM_ADC_MAX_FILTERN)
    {
        filterN = PROGRAM_ADC_MAX_FILTERN;
    }
    programStruct.setupParam.analog_filter_N[idx] = filterN;
    return 1;
}

__STATIC_INLINE uint8_t Program_analogInit()
{
    for (uint8_t i = 0; i < PRG_ANALOG_COUNT; i++)
    {
        programStruct.analog.aIn[i].bspIdx = -1;
        programStruct.analog.aIn[i].order = programStruct.setupParam.analog_av_order[i];
        programStruct.analog.aIn[i].analogFilterN = programStruct.setupParam.analog_filter_N[i];
        programStruct.analog.aIn[i].kMul = programStruct.setupParam.analog_kMul[i];
        programStruct.analog.aIn[i].shift = programStruct.setupParam.analog_shift[i];
    }

    programStruct.analog.aIn[prg_analog_Uzpt].bspIdx             = 0;
    programStruct.analog.aIn[prg_analog_Uout_power_block].bspIdx = 1;
    programStruct.analog.aIn[prg_analog_Uout].bspIdx             = 2;
    programStruct.analog.aIn[prg_analog_IL3].bspIdx              = 3;
    programStruct.analog.aIn[prg_analog_IL4].bspIdx              = 4;
    programStruct.analog.aIn[prg_analog_Iout1].bspIdx            = 5;
    programStruct.analog.aIn[prg_analog_Iout2].bspIdx            = 6;
    bsp_analogIn_start();

    return 1;
}

void bsp_analogIn_ready_callback()
{
    uint8_t count = PRG_ANALOG_COUNT;
    uint8_t bspIdx = 0;
    float value = 0.0f;
    float valueLast = 0.0f;
    float kFilter = 0.0f;
    float data = 0.0f;
    float sum = 0.0f;

    for (uint8_t ch = 0; ch < count; ch++)
    {
        if (programStruct.analog.aIn[ch].bspIdx == -1)
        {
            continue;
        }
        bspIdx = programStruct.analog.aIn[ch].bspIdx;
        data = bsp_analogIn_struct.rawDataUI[bspIdx];

        programStruct.analog.aIn[ch].buf[programStruct.analog.aIn[ch].bufIdx++] = data;

        if (programStruct.analog.aIn[ch].bufIdx == programStruct.analog.aIn[ch].order)
        {
            programStruct.analog.aIn[ch].bufIdx = 0;
        }

        sum = 0.0f;
        for (uint8_t idx = 0; idx < programStruct.analog.aIn[ch].order; idx++)
        {
            sum += programStruct.analog.aIn[ch].buf[idx];
        }

        programStruct.analog.aIn[ch].valueRaw = (sum / programStruct.analog.aIn[ch].order);
        value = (programStruct.analog.aIn[ch].valueRaw - programStruct.analog.aIn[ch].shift) * programStruct.analog.aIn[ch].kMul;
        valueLast = programStruct.analog.aIn[ch].valueLast;

        // Формула: Yavg(i) = Yavg(i-1) + a * ( X(i) - Yavg(i-1) );
        // a = 2/(N + 1) -> Коэффициент фильтра;
        // N -> количество точек для усреднения, N >= 1;
        // N = 1 -> фильтр отключен;
        kFilter = 2.0f / ((float)programStruct.analog.aIn[ch].analogFilterN + 1.0f);
        value = valueLast + kFilter * (value - valueLast);

        programStruct.analog.aIn[ch].value = value;
        programStruct.analog.aIn[ch].valueLast = value;
    }
}
//------------   АЦП End  ------------//

