#ifndef __ARPMONITOR_H
#define __ARPMONITOR_H


#include <napi.h>

class ArpMonitorWorker;


/**
 *
 */
class ArpMonitor : public Napi::ObjectWrap<ArpMonitor> {
public:
  ArpMonitor(const Napi::CallbackInfo& info);
  virtual ~ArpMonitor();

  static Napi::Object Init(Napi::Env env, Napi::Object exports);
  static Napi::Value CreateNewItem(const Napi::CallbackInfo& info);

private:
  Napi::Value Start(const Napi::CallbackInfo& info);
  Napi::Value Stop(const Napi::CallbackInfo& info);

  static void ArpMonitorCallback(const Napi::CallbackInfo& info);

  template <typename T> static void DefaultFini(Napi::Env, T* data) {
    delete data;
  }

private:
  ArpMonitorWorker* _worker;
};


#endif // __ARPMONITOR_H