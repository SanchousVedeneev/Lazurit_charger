
#include "ProtocolMbRtuSlaveCtrl.h"

#include "Program.h"

uint8_t modbusBufRxTxRtu485[MODBUS_SS_BUF_CNT];

//--------------------  PROTOCOL ---------------------//float value = 0.0f;
//---1000
#define MDB_TABLE_BSP_REG_NO (1000)
enum mdb_table_bsp
{
  tab_bsp_din = MDB_TABLE_BSP_REG_NO,
  tab_bsp_dout_led_w1,
  tab_bsp_dout_led_w2,
  tab_bsp_dout,
  tab_bsp_analog = MDB_TABLE_BSP_REG_NO + 5, // ..1016
  tab_bsp_temp = MDB_TABLE_BSP_REG_NO + 17  // ..1018
};
uint16_t mdb_bsp_buf[60];
ModbusSS_table_t mdb_table_bsp = {
    .buf = (uint8_t *)mdb_bsp_buf,
    .quantity = 60,
    .regNo = MDB_TABLE_BSP_REG_NO,
    .type = ModbusSS_Holding};

//---1200
#define MDB_TABLE_PROGRAM_REG_NO (1200)
enum mdb_table_program
{
  tab_prg_cmd = MDB_TABLE_PROGRAM_REG_NO,
  tab_prg_param,
  tab_prg_step,
  tab_prg_target,
  tab_prg_ecode,
  tab_prg_flashCounter,
  tab_prg_analogVal_vodorod_i_SUI1,
  tab_prg_analogVal_vodorod_u_SUO,
  tab_prg_analogVal_vodorod_i_SUI2,
  tab_prg_analogVal_vodorod_u_in,
  tab_prg_analogVal_vodorod_i_SUI3,
  tab_prg_analogVal_out_u,
  tab_prg_analogVal_bat_i,
  tab_prg_analogVal_zu_u_CHO,
  tab_prg_analogVal_out_i,
  tab_prg_analogVal_zu_i_CHO,
  tab_prg_analogVal_zu_u_CH,
  tab_prg_pwm1, // ... 1222
  tab_prg_vodorod_iIn = 1223
};
#define MDB_PROGRAM_BUF_COUNT (tab_prg_vodorod_iIn - MDB_TABLE_PROGRAM_REG_NO + 1)
uint16_t mdb_program_buf[MDB_PROGRAM_BUF_COUNT];
ModbusSS_table_t mdb_table_program = {
    .buf = (uint8_t *)mdb_program_buf,
    .quantity = MDB_PROGRAM_BUF_COUNT,
    .regNo = MDB_TABLE_PROGRAM_REG_NO,
    .type = ModbusSS_Holding};

    #define MDB_TABLE_PARAM_REG_NO (1400)
enum mdb_table_param
{
  tab_param_analog_shift_1_11 = MDB_TABLE_PARAM_REG_NO, //1400 ... 1410
  tab_param_analog_kMul_1_11 = tab_param_analog_shift_1_11 + PRG_ANALOG_COUNT, // 1411 ... 1421
  tab_param_analog_av_order_1_11 = tab_param_analog_kMul_1_11 + PRG_ANALOG_COUNT, // 1422 ... 1432
  tab_param_analog_filter_N_1_11 = tab_param_analog_av_order_1_11 + PRG_ANALOG_COUNT, //1433 ... 1443

  tab_param_protect_control_B_1 =  tab_param_analog_filter_N_1_11 + PRG_ANALOG_COUNT, //1444 ... ,
  tab_param_protect_control_B_2, //1445
};
#define MDB_PARAM_BUF_COUNT (tab_param_protect_control_B_2 - tab_param_analog_shift_1_11 + 1)
uint16_t mdb_param_buf[MDB_PARAM_BUF_COUNT];
ModbusSS_table_t mdb_table_param = {
    .buf = (uint8_t *)mdb_param_buf,
    .quantity = MDB_PARAM_BUF_COUNT,
    .regNo = MDB_TABLE_PARAM_REG_NO,
    .type = ModbusSS_Holding};


//--------------------  PROTOCOL END---------------------//

//--------------------  TABLES ARRAY ---------------------//
ModbusSS_table_t *modbusTables[] = {
    &mdb_table_bsp,
    &mdb_table_program,
    &mdb_table_param
};
//--------------------  TABLES ARRAY END---------------------//

//--------------------  MODBUS STRUCT ---------------------//
ModbusSS_t modbusSS_rtu_rs485 = {
    .cbHoldingUpdate = protocolMbRtuSlaveCtrl_callback_H_WRITE,
    .cbHoldingRequest = NULL, // protocolMbRtuSlaveCtrl_callback_H_REQ, //modbusHoldingReq,
    .rtuTcp = MODBUS_SS_RTU,
    .bufRxTx = modbusBufRxTxRtu485,
    .slaveId = 1,
    .tables = modbusTables,
    .tablesCount = 3
};

protocolMbRtuSlaveCtrl_typedef modbusRtu_ctrlStruct; // protocol control struct
//--------------------  MODBUS STRUCT END---------------------//

//------------------------ EXTERN ------------------------
extern bsp_dInOut_typedef bsp_dInOut_struct;
extern bsp_analogIn_typedef bsp_analogIn_struct;
extern Program_typedef programStruct;
//---------------------- EXTERN END-----------------------

//------------------------ REGULAR FCN ------------------------
void protocolMbRtuSlaveCtrl_init(uint8_t portNo)
{
  HAL_Delay(100);
  bsp_rs485_setPortToModbusRtu(portNo, modbusBufRxTxRtu485, MODBUS_SS_BUF_CNT);
}

__INLINE void protocolMbRtuSlaveCtrl_update_tables()
{
  // BSP -----------------------------//
  uint16_t regNo = MDB_TABLE_BSP_REG_NO;
  ModbusSS_SetWord(&mdb_table_bsp, regNo++, bsp_dInOut_struct.in.w16); // 1000
  for (uint8_t i = 0; i < 3; i++)
  {
    ModbusSS_SetWord(&mdb_table_bsp, regNo++, bsp_dInOut_struct.out.w16[i]); // 1001 - 1003
  }

  regNo = MDB_TABLE_BSP_REG_NO + 5; // 1005
  for (uint8_t i = 0; i < 12; i++)
  {
    ModbusSS_SetWord(&mdb_table_bsp, regNo++, bsp_analogIn_struct.rawDataUI[i]); // 1005 - 1016
  }
  // ModbusSS_SetWord(&mdb_table_bsp, regNo++, bsp_analogIn_getTemp(1)); // 1017
  // ModbusSS_SetWord(&mdb_table_bsp, regNo, bsp_analogIn_getTemp(2));   // 1018
  ModbusSS_SetWord(&mdb_table_bsp, regNo++, bsp_analogIn_struct.currentTemp[0]); // 1017
  ModbusSS_SetWord(&mdb_table_bsp, regNo,   bsp_analogIn_struct.currentTemp[1]); // 1018
  // BSP END-----------------------------//

  // PROGRAM -----------------------------//
  // ModbusSS_SetWord(&mdb_table_program, tab_prg_cmd, modbusRtu_ctrlStruct.cmd);      // 1200
  // ModbusSS_SetWord(&mdb_table_program, tab_prg_param, modbusRtu_ctrlStruct.param);    // 1201
  ModbusSS_SetWord(&mdb_table_program, tab_prg_target, programStruct.control.target);   // 1202
  ModbusSS_SetWord(&mdb_table_program, tab_prg_step, programStruct.control.step);       // 1203
  ModbusSS_SetWord(&mdb_table_program, tab_prg_ecode, programStruct.control.errorCode); // 1204
  ModbusSS_SetWord(&mdb_table_program, tab_prg_flashCounter, programStruct.sys.flash_counter); // 1205
  
  uint16_t analogStartIdx = tab_prg_analogVal_vodorod_i_SUI1;
  uint16_t analogStopIdx = tab_prg_analogVal_zu_u_CH;
  int16_t val = 0.0f;

  for (uint8_t i = 0; i < (analogStopIdx - analogStartIdx + 1); i++)
  {
      val = Program_analogGetByIdx(i)->value;
      ModbusSS_SetWord(&mdb_table_program, analogStartIdx + i, val); // 1206
  }
  
  for (uint8_t i = 0; i < 6; i++)
  {
      ModbusSS_SetWord(&mdb_table_program, tab_prg_pwm1 + i ,programStruct.control.remote.pwmArray[i]);
  }
 

    // PARAM -----------------------------//
  //-----Analog param
  for (uint8_t i = 0; i < PRG_ANALOG_COUNT; i++)
  {
    //ModbusSS_SetWord(&mdb_table_param, tab_param_analog_shift_1_11 + i, Program_analogGetByIdx(i)->shift);
    ModbusSS_SetWord(&mdb_table_param, tab_param_analog_shift_1_11 + i, programStruct.setupParam.analog_shift[i]);
  }
  float analog_kMul = 100000.0f;
  for (uint8_t i = 0; i < PRG_ANALOG_COUNT; i++)
  {
    //ModbusSS_SetWord(&mdb_table_param, tab_param_analog_kMul_1_11 + i, Program_analogGetByIdx(i)->kMul * kMul);
    val = (programStruct.setupParam.analog_kMul[i] * analog_kMul);
    ModbusSS_SetWord(&mdb_table_param, tab_param_analog_kMul_1_11 + i, val);
  }
  for (uint8_t i = 0; i < PRG_ANALOG_COUNT; i++)
  {
    //ModbusSS_SetWord(&mdb_table_param, tab_param_analog_av_order_1_11 + i, Program_analogGetByIdx(i)->order);
    ModbusSS_SetWord(&mdb_table_param, tab_param_analog_av_order_1_11 + i, programStruct.setupParam.analog_av_order[i]);
  }
  for (uint8_t i = 0; i < PRG_ANALOG_COUNT; i++)
  {
    //ModbusSS_SetWord(&mdb_table_param, tab_param_analog_filter_N_1_11 + i, Program_analogGetByIdx(i)->analogFilterN);
    ModbusSS_SetWord(&mdb_table_param, tab_param_analog_filter_N_1_11 + i, programStruct.setupParam.analog_filter_N[i]);
  }
}
//------------------------ REGULAR FCN END------------------------

//------------------------------- MODBUS CALLBACKS -------------------------------------------//
#define PROTOCOL_MB_RTU_SLAVE_CTRL_CMD_OK (0xAAAA)
#define PROTOCOL_MB_RTU_SLAVE_CTRL_CMD_FAIL (0xF00F)
__weak void protocolMbRtuSlaveCtrl_callback_H_WRITE(ModbusSS_table_t *table, uint16_t reg, uint16_t quantity)
{
  uint16_t response = PROTOCOL_MB_RTU_SLAVE_CTRL_CMD_FAIL;
  uint16_t param = 0;
  uint8_t idx = 0;
  int16_t sign_val = 0;
  float analog_kMul = 0.000001;
  float value = 0.0f;
  asm("NOP");

  if (table == &mdb_table_program) // Диапазон PROGRAM
  {
    switch (reg)
    {
    case tab_prg_cmd:
      modbusRtu_ctrlStruct.cmd = ModbusSS_GetWord(&mdb_table_program, reg);
      switch (modbusRtu_ctrlStruct.cmd)
      {
      case protocol_cmd_debug:
        /* code */
        if (Program_GoDebug())
        {
          response = PROTOCOL_MB_RTU_SLAVE_CTRL_CMD_OK;
        }
        break;
      case protocol_cmd_save_param:
        if (Program_ParamSave())
        {
          response = PROTOCOL_MB_RTU_SLAVE_CTRL_CMD_OK;
        }
        break;
      case protocol_cmd_reset:
        if (Program_GoReset())
        {
          response = PROTOCOL_MB_RTU_SLAVE_CTRL_CMD_OK;
        }
        break;
      case protocol_cmd_pwmOuts123:
      case protocol_cmd_pwmOuts456:
        param = ModbusSS_GetWord(&mdb_table_program, tab_prg_param);
        if (Program_set_pwmOuts_debug(modbusRtu_ctrlStruct.cmd - protocol_cmd_pwmOuts123, param))
        {
          response = PROTOCOL_MB_RTU_SLAVE_CTRL_CMD_OK;
        }
        break;
      default:
        response = PROTOCOL_MB_RTU_SLAVE_CTRL_CMD_FAIL;
        break;
      }
      break;
    case tab_prg_analogVal_vodorod_i_SUI1 ... tab_prg_analogVal_zu_u_CH:
      idx = reg - tab_prg_analogVal_vodorod_i_SUI1;
      float value = ModbusSS_GetWord(&mdb_table_program, reg);
      if (value == 0.0f)
      {
        if (Program_analogSetZero(idx))
        {
          response = PROTOCOL_MB_RTU_SLAVE_CTRL_CMD_OK;
        }
      }
      else
      {
        if (Program_analogCalibKMul(idx, value))
        {
          response = PROTOCOL_MB_RTU_SLAVE_CTRL_CMD_OK;
        }
      }
      break;
    case tab_prg_pwm1:
    case tab_prg_pwm1 + 1:
    case tab_prg_pwm1 + 2:
    case tab_prg_pwm1 + 3:
    case tab_prg_pwm1 + 4:
    case tab_prg_pwm1 + 5:

      if (Program_set_pwm_debug(reg - tab_prg_pwm1, ModbusSS_GetWord(&mdb_table_program, reg)))
      {
        response = PROTOCOL_MB_RTU_SLAVE_CTRL_CMD_OK;
      }
      break;
    default:
      response = PROTOCOL_MB_RTU_SLAVE_CTRL_CMD_FAIL;
      break;
    }
  }
  else if (table == &mdb_table_bsp) // Диапазон BSP
  {
    switch (reg)
    {
    case tab_bsp_dout:
      if (Program_set_dout_debug(ModbusSS_GetWord(&mdb_table_bsp, reg)))
      {
        response = PROTOCOL_MB_RTU_SLAVE_CTRL_CMD_OK;
      }
      break;
    default:
      response = PROTOCOL_MB_RTU_SLAVE_CTRL_CMD_FAIL;
      break;
    }
  }
  else if (table == &mdb_table_param)
  {
    value = ModbusSS_GetWord(&mdb_table_param, reg);
        switch (reg)
    {
    case tab_param_analog_shift_1_11 ... tab_param_analog_shift_1_11 + PRG_ANALOG_COUNT - 1:
      idx = reg - tab_param_analog_shift_1_11;
      if(Program_analogSetShift(idx, value)){
        response = PROTOCOL_MB_RTU_SLAVE_CTRL_CMD_OK;
      }
      break;
    case tab_param_analog_kMul_1_11 ... tab_param_analog_kMul_1_11 + PRG_ANALOG_COUNT - 1:
      idx = reg - tab_param_analog_kMul_1_11;
      sign_val = value;
      if(Program_analogSetKMul(idx, sign_val * analog_kMul)){
        response = PROTOCOL_MB_RTU_SLAVE_CTRL_CMD_OK;
      }
      break;
    case tab_param_analog_av_order_1_11 ... tab_param_analog_av_order_1_11 + PRG_ANALOG_COUNT - 1:
      idx = reg - tab_param_analog_av_order_1_11;
      if(Program_analogSetAvOrder(idx, value)){
        response = PROTOCOL_MB_RTU_SLAVE_CTRL_CMD_OK;
      }
      break;
    case tab_param_analog_filter_N_1_11 ... tab_param_analog_filter_N_1_11 + PRG_ANALOG_COUNT - 1:
      idx = reg - tab_param_analog_filter_N_1_11;
      if(Program_analogSetFilterN(idx, value)){
        response = PROTOCOL_MB_RTU_SLAVE_CTRL_CMD_OK;
      }
      break;
    case tab_param_protect_control_B_1:
      programStruct.setupParam.protect_control &= 0xFFFFFFFFFFFF0000;
      programStruct.setupParam.protect_control |= (uint64_t)value;
      response = PROTOCOL_MB_RTU_SLAVE_CTRL_CMD_OK;
      break;
    case tab_param_protect_control_B_2:
      programStruct.setupParam.protect_control &= 0xFFFFFFFF0000FFFF;
      programStruct.setupParam.protect_control |= ((uint64_t)value << 16);
      response = PROTOCOL_MB_RTU_SLAVE_CTRL_CMD_OK;
      break;
    }
  }
  else // Вне диапазона
  {
    response = PROTOCOL_MB_RTU_SLAVE_CTRL_CMD_FAIL;
  }

  ModbusSS_SetWord(&mdb_table_program, tab_prg_cmd, response);
}

__weak void protocolMbRtuSlaveCtrl_callback_H_READ(ModbusSS_table_t *table, uint16_t reg, uint16_t quantity)
{

  asm("NOP");
  //             if (table == &modbusTableHolding1)
  // {
  //   for (int r = reg; r < reg + quantity; r++)
  //   {
  //     asm("NOP");
  //     switch (r)
  //     {
  //     case MBP_AI1_X:
  //       ModbusSS_SetWord(table, r, bsp_ai_read_cache(BSP_AI1));
  //       asm("NOP");
  //       break;
  //     case MBP_AI2_Y:
  //       ModbusSS_SetWord(table, r, bsp_ai_read_cache(BSP_AI2));
  //       asm("NOP");
  //       break;
  //     case MBP_DI_STATE:
  //       ModbusSS_SetWord(table, r, bsp_di_get_cache_pack16());
  //       asm("NOP");
  //       break;
  //     default:
  //       break;
  //     }
  //   }
  // }
}
//------------------------------- MODBUS CALLBACKS END-------------------------------------------//

//------------------------------- HW CALLBACK -------------------------------------------//
void bsp_rs485_callback_rxBlockReady(uint8_t portNo)
{

  int32_t blockSizeByte = 0;
  if ((blockSizeByte = ModbusSS_ParseRxData(&modbusSS_rtu_rs485)) == 0)
  {
    // bug with reset modbus!!!
    asm("NOP");
  }
  else if (blockSizeByte != -1)
  {
    asm("NOP");
    bsp_rs485_sendBlock(portNo, modbusSS_rtu_rs485.bufRxTx, blockSizeByte);
    asm("NOP");
  }
}
//------------------------------- HW CALLBACK END-------------------------------------------//




