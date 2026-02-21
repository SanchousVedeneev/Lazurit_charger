#include "ProtocolMbRtuSlaveCtrl.h"

#include "Program.h"

uint8_t modbusBufRxTxRtu485[MODBUS_SS_BUF_CNT];

//--------------------  PROTOCOL ---------------------
//---1000
#define MDB_TABLE_BSP_REG_NO (1000)
enum mdb_table_bsp
{
  tab_bsp_din = MDB_TABLE_BSP_REG_NO,        // 1000
  tab_bsp_dout_led_w1,                       // 1001
  tab_bsp_dout_led_w2,                       // 1002
  tab_bsp_dout,                              // 1003
  tab_bsp_rezerv,                            // 1004
  tab_bsp_analog,                            // 1005 ... 1016
  tab_bsp_temp_1 = tab_bsp_analog + 11 + 1,  // 1017
  tab_bsp_temp_2                             // 1018
};
#define MDB_BSP_BUF_COUNT (tab_bsp_temp_2 - MDB_TABLE_BSP_REG_NO + 1)
uint16_t mdb_bsp_buf[MDB_BSP_BUF_COUNT];
ModbusSS_table_t mdb_table_bsp = 
{
  .buf = (uint8_t *)mdb_bsp_buf,
  .quantity = MDB_BSP_BUF_COUNT,
  .regNo = MDB_TABLE_BSP_REG_NO,
  .type = ModbusSS_Holding
};

//---1200
#define MDB_TABLE_PROGRAM_REG_NO (1200)
enum mdb_table_program
{
  tab_prg_cmd = MDB_TABLE_PROGRAM_REG_NO,   // 1200
  tab_prg_param,                            // 1201
  tab_prg_step,                             // 1202
  tab_prg_target,                           // 1203
  tab_prg_ecode,                            // 1204
  tab_prg_flashCounter,                     // 1205
  tab_prg_analogVal_Uzpt,                   // 1206
  tab_prg_analogVal_Uout_power_block,       // 1207
  tab_prg_analogVal_Uout,                   // 1208
  tab_prg_analogVal_I_L3,                   // 1209
  tab_prg_analogVal_I_L4,                   // 1210
  tab_prg_analogVal_Iout1,                  // 1211
  tab_prg_analogVal_Iout2,                  // 1212
  tab_prg_pwm1_6,                           // 1213 ... 1218
};
#define MDB_PROGRAM_BUF_COUNT ((tab_prg_pwm1_6 + 5) - MDB_TABLE_PROGRAM_REG_NO + 1)
uint16_t mdb_program_buf[MDB_PROGRAM_BUF_COUNT];
ModbusSS_table_t mdb_table_program = 
{
  .buf = (uint8_t *)mdb_program_buf,
  .quantity = MDB_PROGRAM_BUF_COUNT,
  .regNo = MDB_TABLE_PROGRAM_REG_NO,
  .type = ModbusSS_Holding
};

//---1400
#define MDB_TABLE_PARAM_REG_NO (1400)
enum mdb_table_param
{
  tab_param_analog_shift_1_7    = MDB_TABLE_PARAM_REG_NO,                           // 1400 ... 1406
  tab_param_analog_kMul_1_7     = tab_param_analog_shift_1_7 + PRG_ANALOG_COUNT,    // 1407 ... 1413
  tab_param_analog_av_order_1_7 = tab_param_analog_kMul_1_7 + PRG_ANALOG_COUNT,     // 1414 ... 1420
  tab_param_analog_filter_N_1_7 = tab_param_analog_av_order_1_7 + PRG_ANALOG_COUNT, // 1421 ... 1427

  tab_param_protect_control     =  tab_param_analog_filter_N_1_7 + PRG_ANALOG_COUNT, // 1428
};
#define MDB_PARAM_BUF_COUNT (tab_param_protect_control - tab_param_analog_shift_1_7 + 1)
uint16_t mdb_param_buf[MDB_PARAM_BUF_COUNT];
ModbusSS_table_t mdb_table_param = 
{
  .buf = (uint8_t *)mdb_param_buf,
  .quantity = MDB_PARAM_BUF_COUNT,
  .regNo = MDB_TABLE_PARAM_REG_NO,
  .type = ModbusSS_Holding
};
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
  uint16_t regNo = 0;
  ModbusSS_SetWord(&mdb_table_bsp, tab_bsp_din,         bsp_dInOut_struct.in.w16);        // 1000
  ModbusSS_SetWord(&mdb_table_bsp, tab_bsp_dout_led_w1, bsp_dInOut_struct.out.w16[0]);    // 1001
  ModbusSS_SetWord(&mdb_table_bsp, tab_bsp_dout_led_w2, bsp_dInOut_struct.out.w16[1]);    // 1001
  ModbusSS_SetWord(&mdb_table_bsp, tab_bsp_dout,        bsp_dInOut_struct.out.w16[2]);    // 1002

  regNo = tab_bsp_analog;
  for (uint8_t i = 0; i < 12; i++)
  {
    ModbusSS_SetWord(&mdb_table_bsp, regNo++, bsp_analogIn_struct.rawDataUI[i]);          // 1005 - 1016
  }

  ModbusSS_SetWord(&mdb_table_bsp, tab_bsp_temp_1, bsp_analogIn_getTemp(1));              // 1017
  ModbusSS_SetWord(&mdb_table_bsp, tab_bsp_temp_2, bsp_analogIn_getTemp(2));              // 1018
  // ModbusSS_SetWord(&mdb_table_bsp, regNo++, bsp_analogIn_struct.currentTemp[0]);       // 1017
  // ModbusSS_SetWord(&mdb_table_bsp, regNo,   bsp_analogIn_struct.currentTemp[1]);       // 1018
  // BSP END-----------------------------//

  // PROGRAM -----------------------------//
  // ModbusSS_SetWord(&mdb_table_program, tab_prg_cmd, modbusRtu_ctrlStruct.cmd);        // 1200
  // ModbusSS_SetWord(&mdb_table_program, tab_prg_param, modbusRtu_ctrlStruct.param);    // 1201
  ModbusSS_SetWord(&mdb_table_program, tab_prg_target, programStruct.control.target);            // 1202
  ModbusSS_SetWord(&mdb_table_program, tab_prg_step, programStruct.control.step);                // 1203
  ModbusSS_SetWord(&mdb_table_program, tab_prg_ecode, programStruct.control.errorCode);          // 1204
  ModbusSS_SetWord(&mdb_table_program, tab_prg_flashCounter, programStruct.sys.flash_counter);   // 1205
  
  uint16_t analogStartIdx = tab_prg_analogVal_Uzpt;
  uint16_t analogStopIdx  = tab_prg_analogVal_Iout2;

  for (uint8_t i = 0; i < (analogStopIdx - analogStartIdx + 1); i++)
  {
    ModbusSS_SetWord(&mdb_table_program, analogStartIdx + i, (int16_t)Program_analogGetByIdx(i)->value);    // 1206 ... 1212
  }
  
  for (uint8_t i = 0; i < 6; i++)
  {
    ModbusSS_SetWord(&mdb_table_program, tab_prg_pwm1_6 + i, programStruct.control.remote.pwmArray[i]);     // 1213 ... 1218
  }
 
    // PARAM -----------------------------//
  //-----Analog param
  for (uint8_t i = 0; i < PRG_ANALOG_COUNT; i++)
  {
    ModbusSS_SetWord(&mdb_table_param, tab_param_analog_shift_1_7 + i, programStruct.setupParam.analog_shift[i]);
  }
  float analog_kMul = 100000.0f;
  for (uint8_t i = 0; i < PRG_ANALOG_COUNT; i++)
  {
    ModbusSS_SetWord(&mdb_table_param, tab_param_analog_kMul_1_7 + i, (int16_t)(programStruct.setupParam.analog_kMul[i] * analog_kMul));
  }
  for (uint8_t i = 0; i < PRG_ANALOG_COUNT; i++)
  {
    ModbusSS_SetWord(&mdb_table_param, tab_param_analog_av_order_1_7 + i, programStruct.setupParam.analog_av_order[i]);
  }
  for (uint8_t i = 0; i < PRG_ANALOG_COUNT; i++)
  {
    ModbusSS_SetWord(&mdb_table_param, tab_param_analog_filter_N_1_7 + i, programStruct.setupParam.analog_filter_N[i]);
  }
}
//------------------------ REGULAR FCN END------------------------

//------------------------------- MODBUS CALLBACKS -------------------------------------------//
#define PROTOCOL_MB_RTU_SLAVE_CTRL_CMD_OK   (0xAAAA)
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
    case tab_prg_analogVal_Uzpt ... tab_prg_analogVal_Iout2:
      idx = reg - tab_prg_analogVal_Uzpt;
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
    case tab_prg_pwm1_6:
    case tab_prg_pwm1_6 + 1:
    case tab_prg_pwm1_6 + 2:
    case tab_prg_pwm1_6 + 3:
    case tab_prg_pwm1_6 + 4:
    case tab_prg_pwm1_6 + 5:
      if (Program_set_pwm_debug(reg - tab_prg_pwm1_6, ModbusSS_GetWord(&mdb_table_program, reg)))
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
  else if (table == &mdb_table_param) // Диапазон PARAM
  {
    value = ModbusSS_GetWord(&mdb_table_param, reg);
    switch (reg)
    {
    case tab_param_analog_shift_1_7 ... tab_param_analog_shift_1_7 + PRG_ANALOG_COUNT - 1:
      idx = reg - tab_param_analog_shift_1_7;
      if (Program_analogSetShift(idx, value))
      {
        response = PROTOCOL_MB_RTU_SLAVE_CTRL_CMD_OK;
      }
      break;
    case tab_param_analog_kMul_1_7 ... tab_param_analog_kMul_1_7 + PRG_ANALOG_COUNT - 1:
      idx = reg - tab_param_analog_kMul_1_7;
      sign_val = value;
      if (Program_analogSetKMul(idx, sign_val * analog_kMul))
      {
        response = PROTOCOL_MB_RTU_SLAVE_CTRL_CMD_OK;
      }
      break;
    case tab_param_analog_av_order_1_7 ... tab_param_analog_av_order_1_7 + PRG_ANALOG_COUNT - 1:
      idx = reg - tab_param_analog_av_order_1_7;
      if (Program_analogSetAvOrder(idx, value))
      {
        response = PROTOCOL_MB_RTU_SLAVE_CTRL_CMD_OK;
      }
      break;
    case tab_param_analog_filter_N_1_7 ... tab_param_analog_filter_N_1_7 + PRG_ANALOG_COUNT - 1:
      idx = reg - tab_param_analog_filter_N_1_7;
      if (Program_analogSetFilterN(idx, value))
      {
        response = PROTOCOL_MB_RTU_SLAVE_CTRL_CMD_OK;
      }
      break;
    case tab_param_protect_control:
      programStruct.setupParam.protect_control = (uint32_t)value;
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
  }
}
//------------------------------- HW CALLBACK END-------------------------------------------//




