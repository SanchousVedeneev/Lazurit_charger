
#ifndef __PROGRAM__H__
#define __PROGRAM__H__

#include "main.h"
#include "DSP.h"
#include "BSP.h"

//Steps
typedef enum{
    step_waitInit = 0,
    step_init,
    step_debug,
    step_reset,
    step_error,
    step_wait_op
}Program_STEP_typedef;

//Targets
typedef enum{
    target_debug = 1,
    target_waitOp,
    target_reset,
    target_error
}Program_TARGET_typedef;

//Errors
typedef enum{
    error_noError = 0,
    error_fastStop,
}Program_ERROR_typedef;

typedef enum
{
	prg_din1_UNUSED,       // 0
	prg_din2_UNUSED,       // 1
	prg_din3_UNUSED,       // 2
	prg_din4_UNUSED,       // 3
	prg_din5_UNUSED,       // 4
	prg_din6_UNUSED,       // 5
	prg_din7_UNUSED,       // 6
	prg_din8_UNUSED,       // 7
	prg_din9_UNUSED,       // 8
	prg_din10_UNUSED       // 9
} Program_din_typedef;

#define PRG_DIN_PUSK_VAL (1)
#define PRG_DIN_STOP_VAL (0)
#define PRG_DIN_ALARM_STOP_VAL (0)
#define PRG_DIN_KT_ON (0)

typedef enum
{
	prg_dout1_KM1_KM2,    // 0
	prg_dout2_KM3,        // 1
	prg_dout3_KM4,        // 2
	prg_dout4_fan_power,  // 3
	prg_dout5_fan_cap,    // 4
	prg_dout6_fan_L,      // 5
	prg_dout7,            // 6
	prg_dout8,            // 7
	prg_dout9,            // 8
	prg_dout10            // 9
} Program_dout_typedef;

typedef struct
{
    union
    {
        uint16_t w16;
        struct
        {
            uint8_t dout1 : 1;
            uint8_t dout2 : 1;
            uint8_t dout3 : 1;
            uint8_t dout4 : 1;
            uint8_t dout5 : 1;
            uint8_t dout6 : 1;
            uint8_t dout7 : 1;
            uint8_t dout8 : 1;
            uint8_t dout9 : 1;
            uint8_t dout10 : 1;
            uint8_t        : 6;
        }bits; 
    }dout;
    uint16_t pwmArray[6];
    uint8_t pwmEnable123;
    uint8_t pwmEnable456;
}Program_REMOTE_typedef;


typedef struct
{
    uint8_t vodorodStart;
    dsp_regulator_typedef vodorod_RegI[3];
    dsp_regulator_typedef vodorod_RegU;
    dsp_intensSetter_typedef vodorod_ZI;
}Program_SAU_typedef;

#define PROGRAM_ADC_MAX_FILTER_ORDER (16)
#define PROGRAM_ADC_MAX_FILTERN (250)
typedef struct
{
       float value;
       float valueLast;
       float valueRaw;
       float buf[PROGRAM_ADC_MAX_FILTER_ORDER];
       uint8_t bufIdx;
       float shift;
       float kMul;
       uint8_t order;
       uint16_t analogFilterN;
       int8_t bspIdx;
}Program_AIN_typedef;

typedef enum{
    prg_analog_Uzpt,
    prg_analog_Uout_power_block,
    prg_analog_Uout,
    prg_analog_I_L3,
    prg_analog_I_L4,
    prg_analog_Iout1,
    prg_analog_Iout2
}Program_ANALOG_ENUM_typedef;

#define PRG_ANALOG_COUNT (prg_analog_Iout2 + 1)

typedef struct
{
    Program_AIN_typedef aIn[PRG_ANALOG_COUNT];
}Program_ANALOG_typedef;

typedef struct
{
    Program_STEP_typedef step; // Current step (dc charge, work ...)
    Program_TARGET_typedef target; // Current target (start, stop ...)
    Program_ERROR_typedef errorCode;
    Program_REMOTE_typedef remote;
    Program_SAU_typedef sau;
}Program_CONTROL_typedef;

typedef struct
{
       int16_t flash_counter;
}Program_SYS_typedef;

/*
    Структура сохраняется во Flash память 
    с помощью FlashSaver после команды SaveParam
    в отладочном режиме
*/
typedef struct
{
    uint16_t analog_shift[PRG_ANALOG_COUNT];    //+
    float analog_kMul[PRG_ANALOG_COUNT];        //+
    uint8_t analog_av_order[PRG_ANALOG_COUNT];  //+
    uint16_t analog_filter_N[PRG_ANALOG_COUNT]; //+

    uint16_t protect_control; // по умолчанию =0
}Program_PARAM_typedef;


#define PAN_MDB_STATE_OK (1)
#define PAN_MDB_STATE_ERR (0)
typedef struct
{
    uint8_t mdb_rx;
    uint8_t mdb_state;
}Panel_Param_typedef;


typedef struct
{
    Program_CONTROL_typedef control;
    Program_PARAM_typedef setupParam;
    Program_SYS_typedef sys;
    Program_ANALOG_typedef analog;
    Panel_Param_typedef PanParam;
}Program_typedef;

void Program_start();

Program_AIN_typedef* Program_analogGetByIdx(Program_ANALOG_ENUM_typedef idx);
uint8_t Program_analogSetZero(Program_ANALOG_ENUM_typedef idx);
uint8_t Program_analogCalibKMul(Program_ANALOG_ENUM_typedef idx, float value);
uint8_t Program_analogSetShift(Program_ANALOG_ENUM_typedef idx, float value);
uint8_t Program_analogSetKMul(Program_ANALOG_ENUM_typedef idx, float value);
uint8_t Program_analogSetAvOrder(Program_ANALOG_ENUM_typedef idx, uint8_t order);
uint8_t Program_analogSetFilterN(Program_ANALOG_ENUM_typedef idx, uint16_t filterN);

void Program_ParamSetToDefault();
uint8_t Program_ParamLoad();
uint8_t Program_ParamSave();
void Program_switchTarget(Program_TARGET_typedef newTarget);
uint8_t Program_GoDebug();

uint8_t Program_set_dout_debug(uint16_t douts);
uint8_t Program_set_pwm_debug(uint8_t channel_IDx, uint16_t pwm1000Perc);
uint8_t Program_set_pwmOuts_debug(bsp_pwm_outs_group_typedef group, uint8_t onOff);
uint8_t Program_LoadDefaultParam_debug();

uint8_t Program_GoReset();

#define setBit(reg, bit)   (reg |=  (1 << bit))
#define resetBit(reg, bit) (reg &= ~(1 << bit))

#endif