//
// Created by 30547 on 2026/11/16.
//

#include "can.h"




CAN can[CAN_NUM + 1];
int CAN_Init_A(CAN_ID can_id, FDCAN_HandleTypeDef *hfdcan, uint16_t filter_goal_id, uint16_t filter_mask, p_can_process can_process_x)
{
    can[can_id].can_process = can_process_x;
    can[can_id].hfdcan = hfdcan;
    can[can_id].can_rx_lost = 0;
    can[can_id].can_tx_lost = 0;
    //若未来此处需要多个过滤器，可以考虑可变参数
    FDCAN_FilterTypeDef filter = {0};
    filter.IdType       = FDCAN_STANDARD_ID;
    filter.FilterIndex  = 0;
    filter.FilterType   = FDCAN_FILTER_MASK;
    filter.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
    filter.FilterID1    = filter_goal_id;
    filter.FilterID2    = filter_mask;
    if (HAL_FDCAN_ConfigFilter(hfdcan, &filter) != HAL_OK)
    {
        LED(0,200,200);//错误指示aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
        return -1;
    }
    if (HAL_FDCAN_ConfigGlobalFilter(hfdcan, FDCAN_REJECT, FDCAN_REJECT, FDCAN_REJECT_REMOTE, FDCAN_REJECT_REMOTE) != HAL_OK)
    {
        LED(0,200,200);//错误指示aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
        return -2;
    }
    if (HAL_FDCAN_ConfigRxFifoOverwrite(hfdcan, FDCAN_RX_FIFO0, FDCAN_RX_FIFO_OVERWRITE) != HAL_OK)
    {
        LED(0,200,200);//错误指示aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
        return -3;
    }
    if (HAL_FDCAN_ActivateNotification(hfdcan, FDCAN_IT_RX_FIFO0_NEW_MESSAGE|FDCAN_IT_RX_FIFO0_MESSAGE_LOST|FDCAN_IT_BUS_OFF, 0) != HAL_OK)
    {
        LED(0,200,200);//错误指示aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
        return -4;
    }
    if (HAL_FDCAN_Start(hfdcan) != HAL_OK)
    {
       LED(0,200,200);//错误指示aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
        return -5;
    }
    //覆盖模式，掩码模式，只用FIFO0，标准模式
    can[can_id].online = 1;
    return 1;
}
int CAN_Transmit_classic(uint32_t data_id, const uint8_t *data, uint8_t length, CAN_ID can_id)//这里填长度只是因为AI说方便未来支持其他协议
{
     if (data_id > 0x7FF || length > 8 || data == NULL || can_id < 1 || can_id > 3/*|| HAL_FDCAN_GetTxFifoFreeLevel(can[can_id].hfdcan) == 0*/)
    {
         LED(0,200,200);//错误指示aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
        return -1;
    }


    FDCAN_TxHeaderTypeDef tx_header = {0};

    tx_header.Identifier         = data_id;
    tx_header.IdType             = FDCAN_STANDARD_ID;
    tx_header.TxFrameType        = FDCAN_DATA_FRAME;
    tx_header.DataLength         = length;
    tx_header.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
    tx_header.BitRateSwitch      = FDCAN_BRS_OFF;
    tx_header.FDFormat           = FDCAN_CLASSIC_CAN;
    tx_header.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
    tx_header.MessageMarker      = 0;

    if (HAL_FDCAN_AddMessageToTxFifoQ(can[can_id].hfdcan, &tx_header, data) != HAL_OK)
    {
        can[can_id].can_tx_lost++;
        LED(0,200,200);//错误指示aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
        return -2;
    }
    else return 1;
}



void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs)
{
    CAN_ID can_id = CAN_NUM + 1;
    for (uint8_t i = 1; i < CAN_NUM + 1; i++)
    {
        if (can[i].hfdcan == hfdcan)
        {
            can_id = (CAN_ID)i;
            break;
        }
    }
    if (can_id == CAN_NUM + 1)   //错误指示aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    {
        LED(0,200,200);//错误指示aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
        return;
    }
    if (RxFifo0ITs & FDCAN_IT_RX_FIFO0_MESSAGE_LOST)
    {
        can[can_id].can_rx_lost++;
    }
    FDCAN_RxHeaderTypeDef rx_header = {0};
    uint8_t rx_data[8] = {0};
    uint8_t protect = 0;
    while (HAL_FDCAN_GetRxFifoFillLevel(hfdcan, FDCAN_RX_FIFO0) > 0)
    {
        protect++;
        if (protect >= 12)
        {
            break;
        }
        if (HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &rx_header, rx_data) != HAL_OK)
        {
            can[can_id].can_rx_lost++;
        }
        else if (can[can_id].can_process != NULL)
        {
            can[can_id].can_process(rx_header.Identifier, rx_data, rx_header.DataLength);
        }
    }
}



void HAL_FDCAN_ErrorStatusCallback(FDCAN_HandleTypeDef *hfdcan,uint32_t ErrorStatusITs)
{

    CAN_ID can_id = CAN_NUM + 1;
    for (uint8_t i = 1; i < CAN_NUM + 1; i++)
    {
        if (can[i].hfdcan == hfdcan)
        {
            can_id = (CAN_ID)i;
            break;
        }
    }
    if (can_id == CAN_NUM + 1)   //错误指示aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    {
        LED(0,200,200);//错误指示aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
        return;
    }
    if (ErrorStatusITs & FDCAN_IT_BUS_OFF)
    {
        can[can_id].online = 0;
    }
}