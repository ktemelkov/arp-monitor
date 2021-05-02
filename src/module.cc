
#include <napi.h>
#include <chrono>

#include "interfaces.h"
#include "arpmonitor.h"

#ifdef WIN32
  #include <winsock2.h>
#endif


/**
 * 
 */
static void init_npcap_dll_path(const Napi::Env& env)
{
#ifdef WIN32
	BOOL(WINAPI *SetDllDirectory)(LPCTSTR);
	char sysdir_name[512] = {0};
	int len = 0;

	SetDllDirectory = (BOOL(WINAPI *)(LPCTSTR)) GetProcAddress(GetModuleHandle("kernel32.dll"), "SetDllDirectoryA");

	if (SetDllDirectory == NULL) {
  	printf("Error in SetDllDirectory!\n");
	} else {
		len = GetSystemDirectory(sysdir_name, 480);	//	be safe

		if (!len) {
			printf("Error in GetSystemDirectory!\n");
		} else {
			strcat(sysdir_name, "\\Npcap");
				
			if (SetDllDirectory(sysdir_name) == 0)
				printf("Error in SetDllDirectory(\"System32\\Npcap\")\n");
		}
	}		
#endif
}


/**
 * 
 */
Napi::Object Init(Napi::Env env, Napi::Object exports) {

  init_npcap_dll_path(env);

  unsigned int seed = (unsigned int)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
  srand(seed);

  exports.Set(Napi::String::New(env, "listInterfaces"), Napi::Function::New(env, ListInterfaces));

	ArpMonitor::Init(env, exports);

  return exports;
}

NODE_API_MODULE(NODE_GYP_MODULE_NAME, Init)
