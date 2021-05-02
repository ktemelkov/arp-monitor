#include <napi.h>
#include <pcap/pcap.h>


/**
 * 
 */
class ArpMonitorWorker : public Napi::AsyncProgressQueueWorker<u_char> {
public:
  /**
   *
   */
  ArpMonitorWorker(const Napi::Env& env, Napi::String intfName, Napi::Array hwAddr, Napi::Function& progressCallback, ArpMonitorWorker** parentPointer);


  /**
   *
   */
  virtual ~ArpMonitorWorker();


  /**
   *
   */
  void Execute(const ExecutionProgress& progress);

  /**
   *
   */
  void OnProgress(const u_char* frame, size_t len);


  /**
   * Executed when the async work is complete
   * this function will be run inside the main event loop
   * so it is safe to use JS engine data again
   */
  void OnOK();


  /**
   *
   */
  void OnError(Napi::Error const &error);

  /**
   *
   */
  Napi::Promise Start() {
    return deferredStart.Promise();
  }


  /**
   *
   */
  Napi::Promise Stop() {
    this->stopArpMonitorThread = true;
    return deferredStop.Promise();
  }

protected:
  /**
   *
   */
  pcap_t* InitCapture();

private:
  std::string interfaceName;
  uint8_t hardwareAddress[6];
  uint32_t Xid;
  Napi::ObjectReference hosts;
  Napi::Promise::Deferred deferredStart;
  Napi::Promise::Deferred deferredStop;

  volatile bool stopArpMonitorThread;
  ArpMonitorWorker** parentPointerToThisWorker;
};
