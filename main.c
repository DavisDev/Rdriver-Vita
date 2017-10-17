/* 
	Rdriver Vita.
	Driver return launch for Play Station Vita.
	
	Licensed by GNU General Public License v3.0
	
	Designed By:
	- DevDavisNunez (https://twitter.com/DevDavisNunez).
	Inspired on:
	- Rinnegatamante (https://github.com/Rinnegatamante/AutoBoot).
	
*/

#include <vitasdk.h>
#include <taihen.h>
#include <libk/stdio.h>
#include <libk/string.h>

#define MAIN_APP "APPMANP01"

//static char titleid[10];

int launchAppByTitleid(const char *titleid) {
	char uri[32];
	sprintf(uri, "psgm:play?titleid=%s", titleid);
	
	do{
		sceKernelDelayThread(10000);
		int ret = sceAppMgrLaunchAppByUri(0xFFFFF, uri);
		if (ret == 0) break;
	}while(1);

	return 0;
}
volatile int trigger = 0;
int autoboot_thread(SceSize args, void *argp){
	
	// Waiting a bit to let enso setup
	sceKernelDelayThread(10000000);
	
	launchAppByTitleid(MAIN_APP);
	
	do{
		sceKernelDelayThread(10000);
		if(trigger)
			launchAppByTitleid(MAIN_APP);
	}while(1);
	
	return 0;
}

// sceKernelExitProcess
static tai_hook_ref_t SceLibKernel_sceKernelExitProcess_ref;
static SceUID SceLibKernel_sceKernelExitProcess_hook_uid = -1;

// sceKernelExitProcess
int SceLibKernel_sceKernelExitProcess_hook_func(int res){
	int tmp;
	trigger = 1;
	tmp = TAI_CONTINUE(int, SceLibKernel_sceKernelExitProcess_ref, res);
	return tmp;
}

void _start() __attribute__ ((weak, alias ("module_start")));
int module_start(SceSize argc, const void *args) {
	
	//IoOpen
	SceLibKernel_sceKernelExitProcess_hook_uid = taiHookFunctionImport(
		&SceLibKernel_sceKernelExitProcess_ref,  // Output a reference
		TAI_MAIN_MODULE,         // Name of module being hooked
		0xCAE9ACE6,        // NID specifying SceLibKernel, a wrapper library
		0x7595D9AA,        // NID specifying sceKernelExitProcess
		SceLibKernel_sceKernelExitProcess_hook_func);  // Name of the hook function
	
	// Starting secondary thread to delay boot until enso is setup
	SceUID thid = sceKernelCreateThread("autoboot_thread", autoboot_thread, 0x40, 0x100000, 0, 0, NULL);
	if (thid >= 0) sceKernelStartThread(thid, 0, NULL);

	return SCE_KERNEL_START_SUCCESS;

}

int module_stop(SceSize argc, const void *args) {
	return SCE_KERNEL_STOP_SUCCESS;
}