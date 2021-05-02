#include <chrono>
#include <thread>
#include <list>

#if defined __linux__ || defined __APPLE__
  #include <arpa/inet.h>
#endif

#ifdef __clang__
  #pragma clang diagnostic ignored "-Wmissing-field-initializers"
#endif

#include "arpmonitorworker.h"
#include "arp_pdu.h"


static const char ARP_MONITOR_PCAP_FILTER_FORMAT[] = "arp";

/**
 *
 */
ArpMonitorWorker::ArpMonitorWorker(const Napi::Env& env, Napi::String intfName, Napi::Array hwAddr, Napi::Function& progressCallback, ArpMonitorWorker** parentPointer)
    : Napi::AsyncProgressQueueWorker<u_char>(progressCallback)
    , deferredStart(Napi::Promise::Deferred::New(env))
    , deferredStop(Napi::Promise::Deferred::New(env)) {

  interfaceName = intfName.Utf8Value();
  hosts.Reset(Napi::Array::New(Env()), 1);
  parentPointerToThisWorker = parentPointer;

  stopArpMonitorThread = false;

  Xid = htonl((uint32_t)rand());

  for (int i=0; i < 6; i++)
    hardwareAddress[i] = (uint32_t)hwAddr.Get((uint32_t)i).ToNumber();
}


/**
 *
 */
ArpMonitorWorker::~ArpMonitorWorker() {
}


/**
 *
 */
void ArpMonitorWorker::Execute(const ExecutionProgress& progress) {

  pcap_t* pcapHandle = InitCapture();

  if (pcapHandle != nullptr) {
    progress.Send(nullptr, 0);

    while (!this->stopArpMonitorThread) {
      pcap_pkthdr hdr = {{0}};
      const u_char* frame = pcap_next(pcapHandle, &hdr);

      if (frame != nullptr) {
        progress.Send(frame, hdr.len);
      } else {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
      }
    }

    pcap_close(pcapHandle);
  }
}


/**
 *
 */
void ArpMonitorWorker::ArpMonitorWorker::OnProgress(const u_char* frame, size_t len) {
  Napi::HandleScope scope(Env());

  if (frame == nullptr) {
    // we are starting the monitor
    this->deferredStart.Resolve(Env().Undefined());
  } else {
    size_t ethHeaderSize = IS_VLAN_TAGGED(frame) ? sizeof(ETHERNET_VLAN_FRAME_HEADER) : sizeof(ETHERNET_FRAME_HEADER);
    ARP_FRAME_HEADER* pArpHeader = (ARP_FRAME_HEADER*)(frame + ethHeaderSize);
    ARP_FRAME_IPV4_PAYLOAD* pArpPayload = (ARP_FRAME_IPV4_PAYLOAD*)(frame + ethHeaderSize + sizeof(ARP_FRAME_HEADER));

    if (ntohs(pArpHeader->usProtocolType) == 0x0800 
          && ntohs(pArpHeader->usHardwareType) == 0x0001) {

      Napi::Object host = Napi::Object::New(Env());
      Napi::Array mac = Napi::Array::New(Env());

      host.Set("MAC", mac);

      for (int i = 0; i < 6; i++)
          mac.Set(i, (int)pArpPayload->abSenderMAC[i]);

      char ip[128] = {0};
      uint8_t* ipArray = (uint8_t*)&pArpPayload->ulSenderIP;
      snprintf(ip, sizeof(ip)-1, "%d.%d.%d.%d", ipArray[0], ipArray[1], ipArray[2], ipArray[3]);

      host.Set("IPAddress", ip);

      Callback().Call({ host });
    }
  }
}


/**
 * Executed when the async work is complete
 * this function will be run inside the main event loop
 * so it is safe to use JS engine data again
 */
void ArpMonitorWorker::OnOK() {
  Napi::HandleScope scope(Env());

  *this->parentPointerToThisWorker = nullptr;
  deferredStop.Resolve(Env().Undefined());
}


/**
 *
 */
void ArpMonitorWorker::OnError(Napi::Error const &error) {
  Napi::HandleScope scope(Env());

  *this->parentPointerToThisWorker = nullptr;
  deferredStart.Reject(error.Value());
}


/**
 *
 */
pcap_t* ArpMonitorWorker::InitCapture() {

  bpf_program filter = {0};
  char filterString[256] = {0};
  snprintf(filterString, 255, ARP_MONITOR_PCAP_FILTER_FORMAT, ntohl(Xid), ntohl(Xid));

  char errbuf[PCAP_ERRBUF_SIZE] = {0};
  pcap_t* pcapHandle = pcap_create(interfaceName.c_str(), errbuf);

  if (pcapHandle == nullptr)
    SetError(errbuf);
  else {
    if (pcap_set_promisc(pcapHandle, 1) != 0)
      SetError("Unable to set promiscuous mode");
    else if (pcap_set_buffer_size(pcapHandle, 0) != 0)
      SetError("Unable to set buffer size");
    else if (pcap_set_snaplen(pcapHandle, 1500) != 0)
      SetError("Unable to set snaplen");
    else if (pcap_set_timeout(pcapHandle, 1000) != 0)
      SetError("Unable to set read timeout");
    else if (pcap_activate(pcapHandle) < 0)
      SetError("Unable to start packet capture");
    else if (pcap_setnonblock(pcapHandle, 1, errbuf) == -1)
      SetError(errbuf);
    else if (pcap_compile(pcapHandle, &filter, filterString, 1, PCAP_NETMASK_UNKNOWN) == -1)
      SetError(pcap_geterr(pcapHandle));
    else {
      int res = pcap_setfilter(pcapHandle, &filter);
      pcap_freecode(&filter);

      if (res != 0)
        SetError(pcap_geterr(pcapHandle));
      else {
        return pcapHandle;
      }
    }

    pcap_close(pcapHandle);
  }

  return nullptr;
}
