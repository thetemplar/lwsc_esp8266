typedef void (*freedom_outside_cb_t)(uint8 status);
int wifi_register_send_pkt_freedom_cb(freedom_outside_cb_t cb);
void wifi_unregister_send_pkt_freedom_cb(void);
int wifi_send_pkt_freedom(uint8 *buf, int len, bool sys_seq); 

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
  //0x2... Version
  MSG_RequestBuildDate = 0x30,
  MSG_SendBuildDate = 0x31,
  //0xF... System
  MSG_KeepAlive = 0xFA,
  MSG_Blink = 0xFB,
  MSG_ERROR = 0xFF
};

uint8_t beacon_raw[] = {
  0x80, 0x00,             // 0-1: Frame Control
  0x00, 0x00,             // 2-3: Duration
  0xef, 0x50, 0xff, 0xff, 0xff, 0xff,       // 4-9: Destination address (broadcast)
  0xef, 0x50, 0x00, 0x00, 0x00, 0x00,       // 10-15: Source address
  0xef, 0x50, 0x00, 0x00, 0x00, 0x00,       // 16-21: BSSID (transmitter)
  0x00, 0x00,             // 22-23: Sequence / fragment number
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,     // 24-31: Timestamp (GETS OVERWRITTEN TO 0 BY HARDWARE)
  0x64, 0x00,             // 32-33: Beacon interval
  0x31, 0x04,             // 34-35: Capability info
  0x00, 0x00,             // 36-37: SSID (hidden)
  0xDD, 0x05, 0x07, 0xFA, VERSION, START_TTL, MSG_Unknown //38-44++
};
