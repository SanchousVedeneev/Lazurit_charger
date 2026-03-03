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

#define MDB_TABLE_REGULATOR_REG_NO (1300)
enum mdb_table_regulator
{
  tab_regul_RegU_k_P = MDB_TABLE_REGULATOR_REG_NO, // 1300
  tab_regul_RegU_k_Int,                            // 1301
  tab_regul_RegU_period,                           // 1302
  tab_regul_RegU_In,                               // 1303
  tab_regul_RegU_Fb,                               // 1304
  tab_regul_RegU_d,                                // 1305
  tab_regul_RegU_Out,                              // 1306
  tab_regul_RegU_Int,                              // 1307
  tab_regul_RegU_P,                                // 1308
  tab_regul_RegU_OutMax,                           // 1309
  tab_regul_RegU_OutMin,                           // 1310
  tab_regul_RegU_IntMax,                           // 1311
  tab_regul_RegU_IntMin,                           // 1312

  tab_regul_RegI_Iout_k_P,                         // 1313
  tab_regul_RegI_Iout_k_Int,                       // 1314
  tab_regul_RegI_Iout_period,                      // 1315
  tab_regul_RegI_Iout_In,                          // 1316
  tab_regul_RegI_Iout_Fb,                          // 1317
  tab_regul_RegI_Iout_d,                           // 1318
  tab_regul_RegI_Iout_Out,                         // 1319
  tab_regul_RegI_Iout_Int,                         // 1320
  tab_regul_RegI_Iout_P,                           // 1321
  tab_regul_RegI_Iout_OutMax,                      // 1322
  tab_regul_RegI_Iout_OutMin,                      // 1323
  tab_regul_RegI_Iout_IntMax,                      // 1324
  tab_regul_RegI_Iout_IntMin,                      // 1325

  tab_regul_RegI_IL3_k_P,                          // 1326
  tab_regul_RegI_IL3_k_Int,                        // 1327
  tab_regul_RegI_IL3_period,                       // 1328
  tab_regul_RegI_IL3_In,                           // 1329
  tab_regul_RegI_IL3_Fb,                           // 1330
  tab_regul_RegI_IL3_d,                            // 1331
  tab_regul_RegI_IL3_Out,                          // 1332
  tab_regul_RegI_IL3_Int,                          // 1333
  tab_regul_RegI_IL3_P,                            // 1334
  tab_regul_RegI_IL3_OutMax,                       // 1335
  tab_regul_RegI_IL3_OutMin,                       // 1336
  tab_regul_RegI_IL3_IntMax,                       // 1337
  tab_regul_RegI_IL3_IntMin,                       // 1338

  tab_regul_RegI_IL4_k_P,                          // 1339
  tab_regul_RegI_IL4_k_Int,                        // 1340
  tab_regul_RegI_IL4_period,                       // 1341
  tab_regul_RegI_IL4_In,                           // 1342
  tab_regul_RegI_IL4_Fb,                           // 1343
  tab_regul_RegI_IL4_d,                            // 1344
  tab_regul_RegI_IL4_Out,                          // 1345
  tab_regul_RegI_IL4_Int,                          // 1346
  tab_regul_RegI_IL4_P,                            // 1347
  tab_regul_RegI_IL4_OutMax,                       // 1348
  tab_regul_RegI_IL4_OutMin,                       // 1349
  tab_regul_RegI_IL4_IntMax,                       // 1350
  tab_regul_RegI_IL4_IntMin,                       // 1351

  tab_regul_ZI_Iout_in,                            // 1352
  tab_regul_ZI_Iout_step,                          // 1353
  tab_regul_ZI_Iout_out,                           // 1354
  tab_regul_ZI_Iout_period,                        // 1355
  tab_regul_ZI_Iout_settings                       // 1356

};
#define MDB_REGULATOR_BUF_COUNT (tab_regul_ZI_Iout_settings - MDB_TABLE_REGULATOR_REG_NO + 1)
uint16_t mdb_regulator_buf[MDB_REGULATOR_BUF_COUNT];
ModbusSS_table_t mdb_table_regulator = 
{
  .buf = (uint8_t *)mdb_regulator_buf,
  .quantity = MDB_REGULATOR_BUF_COUNT,
  .regNo = MDB_TABLE_REGULATOR_REG_NO,
  .type = ModbusSS_Holding
};

//---1400
#define MDB_TABLE_PARAM_REG_NO (1400)
enum mdb_table_param
{
  tab_param_analog_shift_1_7    = MDB_TABLE_PARAM_REG_NO,                            // 1400 ... 1406
  tab_param_analog_kMul_1_7     = tab_param_analog_shift_1_7 + PRG_ANALOG_COUNT,     // 1407 ... 1413
  tab_param_analog_av_order_1_7 = tab_param_analog_kMul_1_7 + PRG_ANALOG_COUNT,      // 1414 ... 1420
  tab_param_analog_filter_N_1_7 = tab_param_analog_av_order_1_7 + PRG_ANALOG_COUNT,  // 1421 ... 1427
  tab_param_protect_control     =  tab_param_analog_filter_N_1_7 + PRG_ANALOG_COUNT, // 1428
  tab_param_f_PWM,              // 1429

  tab_param_RegU_in,            // 1430
  tab_param_RegU_k_Int,         // 1431
  tab_param_RegU_k_P,           // 1432
  tab_param_RegU_OutMax,        // 1433

  tab_param_RegI_Iout_k_Int,    // 1434
  tab_param_RegI_Iout_k_P,      // 1435
  tab_param_RegI_Iout_OutMax,   // 1436

  tab_param_RegI_IL3_k_Int,     // 1437
  tab_param_RegI_IL3_k_P,       // 1438
  tab_param_RegI_IL3_OutMax,    // 1439

  tab_param_RegI_IL4_k_Int,     // 1440
  tab_param_RegI_IL4_k_P,       // 1441
  tab_param_RegI_IL4_OutMax,    // 1442

  tab_param_ZI_Iout_settings,   // 1443

  tab_param_check_Uzpt_low,     // 1444
  tab_param_check_Uzpt_high,    // 1445
  tab_param_check_IL3_high,     // 1446
  tab_param_check_IL4_high,     // 1447
  tab_param_check_Iout1_high,   // 1448
  tab_param_check_Iout2_high,   // 1449
  tab_param_check_Uout_high,    // 1450
  tab_param_check_Uakb_no,      // 1451
  tab_param_check_Uakb_high     // 1452
};
#define MDB_PARAM_BUF_COUNT (tab_param_check_Uakb_high - tab_param_analog_shift_1_7 + 1)
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
    &mdb_table_regulator,
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
    .tablesCount = 4
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
  uint16_t regNo = 0;
  uint16_t analogStartIdx = 0;
  uint16_t analogStopIdx = 0;
  float k_1000 = 1000.0f;
  float k_100000 = 100000.0f;
  static uint8_t mdb_table_update = 0;

  if (mdb_table_update == 0)
  {
    // BSP -----------------------------//
    ModbusSS_SetWord(&mdb_table_bsp, tab_bsp_din, bsp_dInOut_struct.in.w16);
    ModbusSS_SetWord(&mdb_table_bsp, tab_bsp_dout_led_w1, bsp_dInOut_struct.out.w16[0]);
    ModbusSS_SetWord(&mdb_table_bsp, tab_bsp_dout_led_w2, bsp_dInOut_struct.out.w16[1]);
    ModbusSS_SetWord(&mdb_table_bsp, tab_bsp_dout, bsp_dInOut_struct.out.w16[2]);

    regNo = tab_bsp_analog;
    for (uint8_t i = 0; i < 12; i++)
    {
      ModbusSS_SetWord(&mdb_table_bsp, regNo++, bsp_analogIn_struct.rawDataUI[i]); // 1005 - 1016
    }

    ModbusSS_SetWord(&mdb_table_bsp, tab_bsp_temp_1, bsp_analogIn_getTemp(1));
    ModbusSS_SetWord(&mdb_table_bsp, tab_bsp_temp_2, bsp_analogIn_getTemp(2));
    // ModbusSS_SetWord(&mdb_table_bsp, regNo++, bsp_analogIn_struct.currentTemp[0]);
    // ModbusSS_SetWord(&mdb_table_bsp, regNo,   bsp_analogIn_struct.currentTemp[1]);
    // BSP END-----------------------------//
  }
  else if (mdb_table_update == 1)
  {
    // PROGRAM -----------------------------//
    // ModbusSS_SetWord(&mdb_table_program, tab_prg_cmd, modbusRtu_ctrlStruct.cmd);        // 1200
    // ModbusSS_SetWord(&mdb_table_program, tab_prg_param, modbusRtu_ctrlStruct.param);    // 1201
    ModbusSS_SetWord(&mdb_table_program, tab_prg_target, programStruct.control.target);
    ModbusSS_SetWord(&mdb_table_program, tab_prg_step, programStruct.control.step);
    ModbusSS_SetWord(&mdb_table_program, tab_prg_ecode, programStruct.control.errorCode);
    ModbusSS_SetWord(&mdb_table_program, tab_prg_flashCounter, programStruct.sys.flash_counter);

    analogStartIdx = tab_prg_analogVal_Uzpt;
    analogStopIdx = tab_prg_analogVal_Iout2;

    for (uint8_t i = 0; i < (analogStopIdx - analogStartIdx + 1); i++)
    {
      ModbusSS_SetWord(&mdb_table_program, analogStartIdx + i, (int16_t)Program_analogGetByIdx(i)->value);
    }

    for (uint8_t i = 0; i < 6; i++)
    {
      ModbusSS_SetWord(&mdb_table_program, tab_prg_pwm1_6 + i, programStruct.control.remote.pwmArray[i]);
    }
  }
  else if (mdb_table_update == 2)
  {
    // REGULATOR -----------------------------//
    ModbusSS_SetWord(&mdb_table_regulator, tab_regul_RegU_k_P, programStruct.control.sau.RegU.k_P * k_1000);
    ModbusSS_SetWord(&mdb_table_regulator, tab_regul_RegU_k_Int, programStruct.control.sau.RegU.k_Int * k_1000);
    ModbusSS_SetWord(&mdb_table_regulator, tab_regul_RegU_period, programStruct.control.sau.RegU.period * k_100000);
    ModbusSS_SetWord(&mdb_table_regulator, tab_regul_RegU_In, programStruct.control.sau.RegU.In);
    ModbusSS_SetWord(&mdb_table_regulator, tab_regul_RegU_Fb, programStruct.control.sau.RegU.Fb);
    ModbusSS_SetWord(&mdb_table_regulator, tab_regul_RegU_d, (int16_t)programStruct.control.sau.RegU.d);
    ModbusSS_SetWord(&mdb_table_regulator, tab_regul_RegU_Out, programStruct.control.sau.RegU.Out);
    ModbusSS_SetWord(&mdb_table_regulator, tab_regul_RegU_Int, programStruct.control.sau.RegU.Int);
    ModbusSS_SetWord(&mdb_table_regulator, tab_regul_RegU_P, (int16_t)programStruct.control.sau.RegU.P);
    ModbusSS_SetWord(&mdb_table_regulator, tab_regul_RegU_OutMax, programStruct.control.sau.RegU.OutMax);
    ModbusSS_SetWord(&mdb_table_regulator, tab_regul_RegU_OutMin, programStruct.control.sau.RegU.OutMin);
    ModbusSS_SetWord(&mdb_table_regulator, tab_regul_RegU_IntMax, programStruct.control.sau.RegU.IntMax);
    ModbusSS_SetWord(&mdb_table_regulator, tab_regul_RegU_IntMin, programStruct.control.sau.RegU.IntMin);

    ModbusSS_SetWord(&mdb_table_regulator, tab_regul_RegI_Iout_k_P, programStruct.control.sau.RegI[Iout].k_P * k_1000);
    ModbusSS_SetWord(&mdb_table_regulator, tab_regul_RegI_Iout_k_Int, programStruct.control.sau.RegI[Iout].k_Int * k_1000);
    ModbusSS_SetWord(&mdb_table_regulator, tab_regul_RegI_Iout_period, programStruct.control.sau.RegI[Iout].period * k_100000);
    ModbusSS_SetWord(&mdb_table_regulator, tab_regul_RegI_Iout_In, programStruct.control.sau.RegI[Iout].In);
    ModbusSS_SetWord(&mdb_table_regulator, tab_regul_RegI_Iout_Fb, programStruct.control.sau.RegI[Iout].Fb);
    ModbusSS_SetWord(&mdb_table_regulator, tab_regul_RegI_Iout_d, (int16_t)programStruct.control.sau.RegI[Iout].d);
    ModbusSS_SetWord(&mdb_table_regulator, tab_regul_RegI_Iout_Out, programStruct.control.sau.RegI[Iout].Out);
    ModbusSS_SetWord(&mdb_table_regulator, tab_regul_RegI_Iout_Int, programStruct.control.sau.RegI[Iout].Int);
    ModbusSS_SetWord(&mdb_table_regulator, tab_regul_RegI_Iout_P, (int16_t)programStruct.control.sau.RegI[Iout].P);
    ModbusSS_SetWord(&mdb_table_regulator, tab_regul_RegI_Iout_OutMax, programStruct.control.sau.RegI[Iout].OutMax);
    ModbusSS_SetWord(&mdb_table_regulator, tab_regul_RegI_Iout_OutMin, programStruct.control.sau.RegI[Iout].OutMin);
    ModbusSS_SetWord(&mdb_table_regulator, tab_regul_RegI_Iout_IntMax, programStruct.control.sau.RegI[Iout].IntMax);
    ModbusSS_SetWord(&mdb_table_regulator, tab_regul_RegI_Iout_IntMin, programStruct.control.sau.RegI[Iout].IntMin);

    ModbusSS_SetWord(&mdb_table_regulator, tab_regul_RegI_IL3_k_P, programStruct.control.sau.RegI[IL3].k_P * k_1000);
    ModbusSS_SetWord(&mdb_table_regulator, tab_regul_RegI_IL3_k_Int, programStruct.control.sau.RegI[IL3].k_Int * k_1000);
    ModbusSS_SetWord(&mdb_table_regulator, tab_regul_RegI_IL3_period, programStruct.control.sau.RegI[IL3].period * k_100000);
    ModbusSS_SetWord(&mdb_table_regulator, tab_regul_RegI_IL3_In, programStruct.control.sau.RegI[IL3].In);
    ModbusSS_SetWord(&mdb_table_regulator, tab_regul_RegI_IL3_Fb, programStruct.control.sau.RegI[IL3].Fb);
    ModbusSS_SetWord(&mdb_table_regulator, tab_regul_RegI_IL3_d, (int16_t)programStruct.control.sau.RegI[IL3].d);
    ModbusSS_SetWord(&mdb_table_regulator, tab_regul_RegI_IL3_Out, programStruct.control.sau.RegI[IL3].Out * k_1000);
    ModbusSS_SetWord(&mdb_table_regulator, tab_regul_RegI_IL3_Int, programStruct.control.sau.RegI[IL3].Int * k_1000);
    ModbusSS_SetWord(&mdb_table_regulator, tab_regul_RegI_IL3_P, (int16_t)programStruct.control.sau.RegI[IL3].P * k_1000);
    ModbusSS_SetWord(&mdb_table_regulator, tab_regul_RegI_IL3_OutMax, programStruct.control.sau.RegI[IL3].OutMax * k_1000);
    ModbusSS_SetWord(&mdb_table_regulator, tab_regul_RegI_IL3_OutMin, programStruct.control.sau.RegI[IL3].OutMin * k_1000);
    ModbusSS_SetWord(&mdb_table_regulator, tab_regul_RegI_IL3_IntMax, programStruct.control.sau.RegI[IL3].IntMax * k_1000);
    ModbusSS_SetWord(&mdb_table_regulator, tab_regul_RegI_IL3_IntMin, programStruct.control.sau.RegI[IL3].IntMin * k_1000);

    ModbusSS_SetWord(&mdb_table_regulator, tab_regul_RegI_IL4_k_P, programStruct.control.sau.RegI[IL4].k_P * k_1000);
    ModbusSS_SetWord(&mdb_table_regulator, tab_regul_RegI_IL4_k_Int, programStruct.control.sau.RegI[IL4].k_Int * k_1000);
    ModbusSS_SetWord(&mdb_table_regulator, tab_regul_RegI_IL4_period, programStruct.control.sau.RegI[IL4].period * k_100000);
    ModbusSS_SetWord(&mdb_table_regulator, tab_regul_RegI_IL4_In, programStruct.control.sau.RegI[IL4].In);
    ModbusSS_SetWord(&mdb_table_regulator, tab_regul_RegI_IL4_Fb, programStruct.control.sau.RegI[IL4].Fb);
    ModbusSS_SetWord(&mdb_table_regulator, tab_regul_RegI_IL4_d, (int16_t)programStruct.control.sau.RegI[IL4].d);
    ModbusSS_SetWord(&mdb_table_regulator, tab_regul_RegI_IL4_Out, programStruct.control.sau.RegI[IL4].Out * k_1000);
    ModbusSS_SetWord(&mdb_table_regulator, tab_regul_RegI_IL4_Int, programStruct.control.sau.RegI[IL4].Int * k_1000);
    ModbusSS_SetWord(&mdb_table_regulator, tab_regul_RegI_IL4_P, (int16_t)programStruct.control.sau.RegI[IL4].P * k_1000);
    ModbusSS_SetWord(&mdb_table_regulator, tab_regul_RegI_IL4_OutMax, programStruct.control.sau.RegI[IL4].OutMax * k_1000);
    ModbusSS_SetWord(&mdb_table_regulator, tab_regul_RegI_IL4_OutMin, programStruct.control.sau.RegI[IL4].OutMin * k_1000);
    ModbusSS_SetWord(&mdb_table_regulator, tab_regul_RegI_IL4_IntMax, programStruct.control.sau.RegI[IL4].IntMax * k_1000);
    ModbusSS_SetWord(&mdb_table_regulator, tab_regul_RegI_IL4_IntMin, programStruct.control.sau.RegI[IL4].IntMin * k_1000);

    ModbusSS_SetWord(&mdb_table_regulator, tab_regul_ZI_Iout_in, programStruct.control.sau.ZI_Iout.in);
    ModbusSS_SetWord(&mdb_table_regulator, tab_regul_ZI_Iout_step, programStruct.control.sau.ZI_Iout.step * k_100000);
    ModbusSS_SetWord(&mdb_table_regulator, tab_regul_ZI_Iout_out, programStruct.control.sau.ZI_Iout.out);
    ModbusSS_SetWord(&mdb_table_regulator, tab_regul_ZI_Iout_period, programStruct.control.sau.ZI_Iout.period * k_100000);
    ModbusSS_SetWord(&mdb_table_regulator, tab_regul_ZI_Iout_settings, programStruct.control.sau.ZI_Iout.settings);
  }
  else if (mdb_table_update == 3)
  {
    // PARAM -----------------------------//
    for (uint8_t i = 0; i < PRG_ANALOG_COUNT; i++)
    {
      ModbusSS_SetWord(&mdb_table_param, tab_param_analog_shift_1_7 + i, programStruct.setupParam.analog_shift[i]);
    }
    for (uint8_t i = 0; i < PRG_ANALOG_COUNT; i++)
    {
      ModbusSS_SetWord(&mdb_table_param, tab_param_analog_kMul_1_7 + i, (int16_t)(programStruct.setupParam.analog_kMul[i] * k_100000));
    }
    for (uint8_t i = 0; i < PRG_ANALOG_COUNT; i++)
    {
      ModbusSS_SetWord(&mdb_table_param, tab_param_analog_av_order_1_7 + i, programStruct.setupParam.analog_av_order[i]);
    }
    for (uint8_t i = 0; i < PRG_ANALOG_COUNT; i++)
    {
      ModbusSS_SetWord(&mdb_table_param, tab_param_analog_filter_N_1_7 + i, programStruct.setupParam.analog_filter_N[i]);
    }

    ModbusSS_SetWord(&mdb_table_param, tab_param_protect_control, programStruct.setupParam.protect_control);

    ModbusSS_SetWord(&mdb_table_param, tab_param_f_PWM, programStruct.setupParam.f_PWM);

    ModbusSS_SetWord(&mdb_table_param, tab_param_RegU_in, programStruct.setupParam.RegU_in);
    ModbusSS_SetWord(&mdb_table_param, tab_param_RegU_k_Int, programStruct.setupParam.RegU_k_Int * k_1000);
    ModbusSS_SetWord(&mdb_table_param, tab_param_RegU_k_P, programStruct.setupParam.RegU_k_Int * k_1000);
    ModbusSS_SetWord(&mdb_table_param, tab_param_RegU_OutMax, programStruct.setupParam.RegU_OutMax);

    ModbusSS_SetWord(&mdb_table_param, tab_param_RegI_Iout_k_Int, programStruct.setupParam.RegI_Iout_k_Int * k_1000);
    ModbusSS_SetWord(&mdb_table_param, tab_param_RegI_Iout_k_P, programStruct.setupParam.RegI_Iout_k_P * k_1000);
    ModbusSS_SetWord(&mdb_table_param, tab_param_RegI_Iout_OutMax, programStruct.setupParam.RegI_Iout_OutMax);

    ModbusSS_SetWord(&mdb_table_param, tab_param_RegI_IL3_k_Int, programStruct.setupParam.RegI_IL3_k_Int * k_1000);
    ModbusSS_SetWord(&mdb_table_param, tab_param_RegI_IL3_k_P, programStruct.setupParam.RegI_IL3_k_P * k_1000);
    ModbusSS_SetWord(&mdb_table_param, tab_param_RegI_IL3_OutMax, programStruct.setupParam.RegI_IL3_OutMax * k_1000);

    ModbusSS_SetWord(&mdb_table_param, tab_param_RegI_IL4_k_Int, programStruct.setupParam.RegI_IL4_k_Int * k_1000);
    ModbusSS_SetWord(&mdb_table_param, tab_param_RegI_IL4_k_P, programStruct.setupParam.RegI_IL4_k_P * k_1000);
    ModbusSS_SetWord(&mdb_table_param, tab_param_RegI_IL4_OutMax, programStruct.setupParam.RegI_IL4_OutMax * k_1000);

    ModbusSS_SetWord(&mdb_table_param, tab_param_ZI_Iout_settings, programStruct.setupParam.ZI_Iout_settings);

    ModbusSS_SetWord(&mdb_table_param, tab_param_check_Uzpt_low,   programStruct.setupParam.check_Uzpt_low);
    ModbusSS_SetWord(&mdb_table_param, tab_param_check_Uzpt_high,  programStruct.setupParam.check_Uzpt_high);
    ModbusSS_SetWord(&mdb_table_param, tab_param_check_IL3_high,   programStruct.setupParam.check_IL3_high);
    ModbusSS_SetWord(&mdb_table_param, tab_param_check_IL4_high,   programStruct.setupParam.check_IL4_high);
    ModbusSS_SetWord(&mdb_table_param, tab_param_check_Iout1_high, programStruct.setupParam.check_Iout1_high);
    ModbusSS_SetWord(&mdb_table_param, tab_param_check_Iout2_high, programStruct.setupParam.check_Iout2_high);
    ModbusSS_SetWord(&mdb_table_param, tab_param_check_Uout_high,  programStruct.setupParam.check_Uout_high);
    ModbusSS_SetWord(&mdb_table_param, tab_param_check_Uakb_no,    programStruct.setupParam.check_Uakb_no);
    ModbusSS_SetWord(&mdb_table_param, tab_param_check_Uakb_high,  programStruct.setupParam.check_Uakb_high);
  }

  if (++mdb_table_update > 3)
  {
    mdb_table_update = 0;
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
  float analog_kMul = 0.00001;
  float k_x001 = 0.001;
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
    case tab_prg_pwm1_6 ... tab_prg_pwm1_6 + 5:
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
      programStruct.setupParam.protect_control = value;
      response = PROTOCOL_MB_RTU_SLAVE_CTRL_CMD_OK;
      break;
    case tab_param_f_PWM:
      if (Program_set_PWM(value))
      {
        response = PROTOCOL_MB_RTU_SLAVE_CTRL_CMD_OK;
      }
      else
      {
        response = PROTOCOL_MB_RTU_SLAVE_CTRL_CMD_FAIL;
      }
      break;
    case tab_param_RegU_in:
      if (Program_setup_RegU_in(value))
      {
        response = PROTOCOL_MB_RTU_SLAVE_CTRL_CMD_OK;
      }
      else
      {
        response = PROTOCOL_MB_RTU_SLAVE_CTRL_CMD_FAIL;
      }
      break;
    case tab_param_RegU_k_Int:
      if (Program_setup_RegU_k_Int(value*k_x001))
      {
        response = PROTOCOL_MB_RTU_SLAVE_CTRL_CMD_OK;
      }
      else
      {
        response = PROTOCOL_MB_RTU_SLAVE_CTRL_CMD_FAIL;
      }
      break;
    case tab_param_RegU_k_P:
      programStruct.setupParam.RegU_k_P = value*k_x001;
      response = PROTOCOL_MB_RTU_SLAVE_CTRL_CMD_OK;
      break;
    case tab_param_RegU_OutMax:
      programStruct.setupParam.RegU_OutMax = value;
      response = PROTOCOL_MB_RTU_SLAVE_CTRL_CMD_OK;
      break;
    case tab_param_RegI_Iout_k_Int:
      programStruct.setupParam.RegI_Iout_k_Int = value*k_x001;
      response = PROTOCOL_MB_RTU_SLAVE_CTRL_CMD_OK;
      break;
    case tab_param_RegI_Iout_k_P:
      programStruct.setupParam.RegI_Iout_k_P = value*k_x001;
      response = PROTOCOL_MB_RTU_SLAVE_CTRL_CMD_OK;
      break;
    case tab_param_RegI_Iout_OutMax:
      programStruct.setupParam.RegI_Iout_OutMax = value;
      response = PROTOCOL_MB_RTU_SLAVE_CTRL_CMD_OK;
      break;
    case tab_param_RegI_IL3_k_Int:
      programStruct.setupParam.RegI_IL3_k_Int = value*k_x001;
      response = PROTOCOL_MB_RTU_SLAVE_CTRL_CMD_OK;
      break;
    case tab_param_RegI_IL3_k_P:
      programStruct.setupParam.RegI_IL3_k_P = value*k_x001;
      response = PROTOCOL_MB_RTU_SLAVE_CTRL_CMD_OK;
      break;
    case tab_param_RegI_IL3_OutMax:
      programStruct.setupParam.RegI_IL3_OutMax = value;
      response = PROTOCOL_MB_RTU_SLAVE_CTRL_CMD_OK;
      break;
    case tab_param_RegI_IL4_k_Int:
      programStruct.setupParam.RegI_IL4_k_Int = value*k_x001;
      response = PROTOCOL_MB_RTU_SLAVE_CTRL_CMD_OK;
      break;
    case tab_param_RegI_IL4_k_P:
      programStruct.setupParam.RegI_IL4_k_P = value*k_x001;
      response = PROTOCOL_MB_RTU_SLAVE_CTRL_CMD_OK;
      break;
    case tab_param_RegI_IL4_OutMax:
      programStruct.setupParam.RegI_IL4_OutMax = value;
      response = PROTOCOL_MB_RTU_SLAVE_CTRL_CMD_OK;
      break;
    case tab_param_ZI_Iout_settings:
      programStruct.setupParam.ZI_Iout_settings = value;
      response = PROTOCOL_MB_RTU_SLAVE_CTRL_CMD_OK;
      break;
    case tab_param_check_Uzpt_low:
      if (Program_check_Uzpt_low(value))
      {
        response = PROTOCOL_MB_RTU_SLAVE_CTRL_CMD_OK;
      }
      else
      {
        response = PROTOCOL_MB_RTU_SLAVE_CTRL_CMD_FAIL;
      }
      break;
    case tab_param_check_Uzpt_high:
      if (Program_check_Uzpt_high(value))
      {
        response = PROTOCOL_MB_RTU_SLAVE_CTRL_CMD_OK;
      }
      else
      {
        response = PROTOCOL_MB_RTU_SLAVE_CTRL_CMD_FAIL;
      }
      break;
    case tab_param_check_IL3_high:
      if (Program_check_IL3_high(value))
      {
        response = PROTOCOL_MB_RTU_SLAVE_CTRL_CMD_OK;
      }
      else
      {
        response = PROTOCOL_MB_RTU_SLAVE_CTRL_CMD_FAIL;
      }
      break;
    case tab_param_check_IL4_high:
      if (Program_check_IL4_high(value))
      {
        response = PROTOCOL_MB_RTU_SLAVE_CTRL_CMD_OK;
      }
      else
      {
        response = PROTOCOL_MB_RTU_SLAVE_CTRL_CMD_FAIL;
      }
      break;
    case tab_param_check_Iout1_high:
      if (Program_check_Iout1_high(value))
      {
        response = PROTOCOL_MB_RTU_SLAVE_CTRL_CMD_OK;
      }
      else
      {
        response = PROTOCOL_MB_RTU_SLAVE_CTRL_CMD_FAIL;
      }
      break;
    case tab_param_check_Iout2_high:
      if (Program_check_Iout2_high(value))
      {
        response = PROTOCOL_MB_RTU_SLAVE_CTRL_CMD_OK;
      }
      else
      {
        response = PROTOCOL_MB_RTU_SLAVE_CTRL_CMD_FAIL;
      }
      break;
    case tab_param_check_Uout_high:
      if (Program_check_Uout_high(value))
      {
        response = PROTOCOL_MB_RTU_SLAVE_CTRL_CMD_OK;
      }
      else
      {
        response = PROTOCOL_MB_RTU_SLAVE_CTRL_CMD_FAIL;
      }
      break;
    case tab_param_check_Uakb_no:
      if (Program_check_Uakb_no(value))
      {
        response = PROTOCOL_MB_RTU_SLAVE_CTRL_CMD_OK;
      }
      else
      {
        response = PROTOCOL_MB_RTU_SLAVE_CTRL_CMD_FAIL;
      }
      break;
    case tab_param_check_Uakb_high:
      if (Program_check_Uakb_high(value))
      {
        response = PROTOCOL_MB_RTU_SLAVE_CTRL_CMD_OK;
      }
      else
      {
        response = PROTOCOL_MB_RTU_SLAVE_CTRL_CMD_FAIL;
      }
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
    BSP_LED_TOGGLE(BSP_LED_LINK);
  }
}
//------------------------------- HW CALLBACK END-------------------------------------------//




