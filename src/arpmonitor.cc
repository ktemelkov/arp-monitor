#include "arpmonitor.h"
#include "arpmonitorworker.h"


/**
 *
 */
ArpMonitor::ArpMonitor(const Napi::CallbackInfo& info) : Napi::ObjectWrap<ArpMonitor>(info) {
  this->_worker = nullptr;
}


/**
 *
 */
ArpMonitor::~ArpMonitor() {
  if (this->_worker) {
    delete this->_worker;
    this->_worker = nullptr;
  }
}


/**
 *
 */
Napi::Object ArpMonitor::Init(Napi::Env env, Napi::Object exports) {
    // This method is used to hook the accessor and method callbacks
    Napi::Function func = DefineClass(env, "ArpMonitor", {
        InstanceMethod<&ArpMonitor::Start>("start"),
        InstanceMethod<&ArpMonitor::Stop>("stop"),
        StaticMethod<&ArpMonitor::CreateNewItem>("CreateNewItem"),
    });

    Napi::FunctionReference* constructor = new Napi::FunctionReference();

    // Create a persistent reference to the class constructor. This will allow
    // a function called on a class prototype and a function
    // called on instance of a class to be distinguished from each other.
    *constructor = Napi::Persistent(func);
    exports.Set("ArpMonitor", func);

    // Store the constructor as the add-on instance data. This will allow this
    // add-on to support multiple instances of itself running on multiple worker
    // threads, as well as multiple instances of itself running in different
    // contexts on the same thread.
    //
    // By default, the value set on the environment here will be destroyed when
    // the add-on is unloaded using the `delete` operator, but it is also
    // possible to supply a custom deleter.
    env.SetInstanceData<Napi::FunctionReference, ArpMonitor::DefaultFini>(constructor);

    return exports;
}


/**
 * 
 */
Napi::Value ArpMonitor::CreateNewItem(const Napi::CallbackInfo& info) {
  // Retrieve the instance data we stored during `Init()`. We only stored the
  // constructor there, so we retrieve it here to create a new instance of the
  // JS class the constructor represents.
  Napi::FunctionReference* constructor = info.Env().GetInstanceData<Napi::FunctionReference>();
  return constructor->New({});
}


/**
 * 
 */
Napi::Value ArpMonitor::Start(const Napi::CallbackInfo& info) {
  const Napi::Env env = info.Env();

  if (this->_worker != nullptr) {
    Napi::Promise::Deferred deferred = Napi::Promise::Deferred::New(env);
    deferred.Reject(Napi::Error::New(env, "ArpMonitor already started.").Value());

    return deferred.Promise();
  }

  if (info.Length() < 2) {
    Napi::Promise::Deferred deferred = Napi::Promise::Deferred::New(env);
    deferred.Reject(Napi::Error::New(env, "No callback supplied to start ArpMonitor.").Value());

    return deferred.Promise();
  }

  Napi::Object intf = info[0].As<Napi::Object>();
  Napi::Function progressCallback = info[1].As<Napi::Function>();

  _worker = new ArpMonitorWorker(env, intf.Get("name").As<Napi::String>(), intf.Get("hardwareAddress").As<Napi::Array>(), progressCallback, &this->_worker);
  _worker->Queue();

  return _worker->Start();
}


/**
 * 
 */
Napi::Value ArpMonitor::Stop(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  if (this->_worker != nullptr) {
    return this->_worker->Stop();
  }

  Napi::Promise::Deferred deferred = Napi::Promise::Deferred::New(env);
  deferred.Reject(Napi::Error::New(env, "Cannot stop ArpMonitor if it has not been started.").Value());

  return deferred.Promise();
}
