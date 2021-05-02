#ifndef __ARP_PDU_H
#define __ARP_PDU_H


#define IS_VLAN_TAGGED(frame) ((frame)[12] == 0x81 && (frame)[13] == 0x00)


#pragma pack(1)

struct ETHERNET_FRAME_HEADER {
  uint8_t  abDest[6];   //!< Destination MAC address
  uint8_t  abSource[6]; //!< Source MAC address
  uint16_t usFrameType; //!< Ethernet frame type
};


struct ETHERNET_VLAN_FRAME_HEADER {
  uint8_t  abDest[6];     //!< Destination MAC address
  uint8_t  abSource[6];   //!< Source MAC address
  uint16_t usVLANTPID;    //!< VLAN TPID
  uint16_t usVLANTCI;     //!< VLAN TCI
  uint16_t usFrameType;   //!< Ethernet frame type
};


struct ARP_FRAME_HEADER {
  uint16_t usHardwareType;
  uint16_t usProtocolType;
  uint8_t ucHardwareSize;
  uint8_t ucProtocolSize;
  uint16_t usOpCode;
};


struct ARP_FRAME_IPV4_PAYLOAD {
  uint8_t  abSenderMAC[6];
  uint32_t ulSenderIP;
  uint8_t  abTargetMAC[6];
  uint32_t ulTargetIP;
};

#pragma pack()

#endif // __ARP_PDU_H
