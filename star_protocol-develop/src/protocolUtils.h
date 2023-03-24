/**
 * @file protocolUtils.h
 * @brief  星广播平台终端控制报文工具
 * @author Bill
 * @version 0.0.1
 * @date 2021-05-28
 */

#pragma once

#include "utils.h"

namespace star_protocol
{

int getTlvLength(const uint8_t *data, int &bytes);
int setTlvLength(int length, uint8_t *buf, int bufSize, int &bytes);

}