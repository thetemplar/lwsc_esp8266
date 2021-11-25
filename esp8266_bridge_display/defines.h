#ifndef DEFINES_H_
#define DEFINES_H_

#pragma once

#include <map>

#define VERSION 0x03
//uint8_t VERSION;
#define START_TTL 0x05
#define MSG_TYPE 0x00


struct RxControl {
  signed rssi: 8;
  unsigned rate: 4;
  unsigned is_group: 1;
  unsigned: 1;
  unsigned sig_mode: 2;
  unsigned legacy_length: 12;
  unsigned damatch0: 1;
  unsigned damatch1: 1;
  unsigned bssidmatch0: 1;
  unsigned bssidmatch1: 1;
  unsigned MCS: 7;
  unsigned CWB: 1;
  unsigned HT_length: 16;
  unsigned Smoothing: 1;
  unsigned Not_Sounding: 1;
  unsigned: 1;
  unsigned Aggregation: 1;
  unsigned STBC: 2;
  unsigned FEC_CODING: 1;
  unsigned SGI: 1;
  unsigned rxend_state: 8;
  unsigned ampdu_cnt: 8;
  unsigned channel: 4;
  unsigned: 12;
};

struct LenSeq {
  uint16_t length;
  uint16_t seq;
  uint8_t  address3[6];
};

struct sniffer_buf {
  struct RxControl rx_ctrl;
  uint8_t buf[36];
  uint16_t cnt;
  struct LenSeq lenseq[1];
};

struct sniffer_buf2 {
  struct RxControl rx_ctrl;
  uint8_t buf[112];
  uint16_t cnt;
  uint16_t len;
};

struct msgData {
  uint32_t dst;
  uint32_t src;
  uint32_t trs;
  uint16_t seq;
  uint8_t ver;
  uint8_t ttl;
  uint8_t type;
  uint8_t dataLength;
  uint8_t data[251];
};

enum MsgTypes : uint8_t
{
  MSG_Unknown = 0x00,
  //0x0...  Data
	MSG_Fire = 0x01,
  MSG_Fire_Ack = 0x02,
  //0x2... Network
  MSG_RequestRssi = 0x20,
  MSG_SendRssi = 0x21,
  //0xF... System
  MSG_KeepAlive = 0xFA,
  MSG_Blink = 0xFB,
  MSG_ERROR = 0xFF
};

struct WifiLog {
  uint32_t Id;
  uint32_t Duration;
  uint8_t RelaisBitmask;
  uint8_t Type;
  int8_t Rssi;
  uint8_t Seq;
  uint32_t Timestamp;
};

struct MachineFunction {
  char Name[38];
  uint8_t RelaisBitmask;
  int32_t Duration; //-1 = toggle
  
  uint32_t SymbolX;
  uint32_t SymbolY;
  uint8_t Rotation;
};

struct MachineData {
  char Name[38];
  char ShortName[9];
  uint32_t Id;
  int8_t Rssi;
  uint8_t Distance;
  uint8_t Disabled;
  uint32_t Relais1Counter;
  uint32_t Relais2Counter;

  uint32_t SymbolX;
  uint32_t SymbolY;

  MachineFunction Functions[5];
  
  std::map<uint32_t, int8_t> RssiMap; //Not persistent!
};



#endif