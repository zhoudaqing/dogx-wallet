#include "Stdafx.h"
#include <afxdllx.h>

////////////////////////////////////////////////////////////本源码由网猫YZ发布，QQ交流群：285183716，联系QQ：738961693

static AFX_EXTENSION_MODULE GameServerDLL={NULL,NULL};

//DLL 导出主函数
extern "C" int APIENTRY DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	UNREFERENCED_PARAMETER(lpReserved);
	if (dwReason==DLL_PROCESS_ATTACH)
	{
		if (!AfxInitExtensionModule(GameServerDLL, hInstance)) return 0;
		new CDynLinkLibrary(GameServerDLL);
	}
	else if (dwReason==DLL_PROCESS_DETACH)
	{
		AfxTermExtensionModule(GameServerDLL);
	}
	return 1;
}

////////////////////////////////////////////////////////////本源码由网猫YZ发布，QQ交流群：285183716，联系QQ：738961693
