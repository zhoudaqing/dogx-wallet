#pragma once



#ifndef VC_EXTRALEAN

#define VC_EXTRALEAN		// ��Windows ͷ���ų�����ʹ�õ�����

#endif

// ���������ʹ��������ָ����ƽ̨֮ǰ��ƽ̨�����޸�����Ķ��塣

// �йز�ͬƽ̨����Ӧֵ��������Ϣ����ο�MSDN��

#ifndef WINVER				// ����ʹ���ض���Windows 95 ��Windows NT 4 ����߰汾�Ĺ��ܡ�

#define WINVER 0x0400		// ���˸���Ϊ�����Windows 98 ��Windows 2000 ����߰汾�ĺ��ʵ�ֵ��

#endif



#ifndef _WIN32_WINNT		// ����ʹ���ض���Windows NT 4 ����߰汾�Ĺ��ܡ�

#define _WIN32_WINNT 0x0400	// ���˸���Ϊ�����Windows 2000 ����߰汾�ĺ��ʵ�ֵ��

#endif						



#ifndef _WIN32_WINDOWS		// ����ʹ���ض���Windows 98 ����߰汾�Ĺ��ܡ�

#define _WIN32_WINDOWS 0x0410 // ���˸���Ϊ�����Windows Me ����߰汾�ĺ��ʵ�ֵ��

#endif



#ifndef _WIN32_IE			// ����ʹ���ض���IE 4.0 ����߰汾�Ĺ��ܡ�

#define _WIN32_IE 0x0400	// ���˸���Ϊ�����IE 5.0 ����߰汾�ĺ��ʵ�ֵ��

#endif



#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// ĳЩCString ���캯����Ϊ��ʽ��


#include <afxwin.h>         // MFC ��������ͱ�׼���

#include <afxext.h>         // MFC ��չ



#ifndef _AFX_NO_OLE_SUPPORT

#include <afxole.h>         // MFC OLE ��

#include <afxodlgs.h>       // MFC OLE �Ի�����

#include <afxdisp.h>        // MFC �Զ�����

#endif // _AFX_NO_OLE_SUPPORT



#include <afxdtctl.h>		// MFC ��Internet Explorer 4 �����ؼ���֧��

#ifndef _AFX_NO_AFXCMN_SUPPORT

#include <afxcmn.h>			// MFC ��Windows �����ؼ���֧��

#endif // _AFX_NO_AFXCMN_SUPPORT

#include "Shlwapi.h" 


////////////////////////////////////////////////////////////��Դ������èYZ������QQ����Ⱥ��285183716����ϵQQ��738961693

#define GAME_SERVICE_DLL

//���ͷ�ļ�
#include "..\..\..\..\ϵͳģ��\���������\��Ϸ����\GameServiceHead.h"



#ifndef _DEBUG
#ifndef _UNICODE
#pragma comment (lib,"../../../../ϵͳģ��/���ӿ�/Ansi/ServiceCore.lib")
#pragma comment (lib,"../../../../ϵͳģ��/���ӿ�/Ansi/KernelEngine.lib")
#else
#pragma comment (lib,"../../../../ϵͳģ��/���ӿ�/Unicode/ServiceCore.lib")
#pragma comment (lib,"../../../../ϵͳģ��/���ӿ�/Unicode/KernelEngine.lib")
#endif
#else
#ifndef _UNICODE
#pragma comment (lib,"../../../../ϵͳģ��/���ӿ�/Ansi/ServiceCoreD.lib")
#pragma comment (lib,"../../../../ϵͳģ��/���ӿ�/Ansi/KernelEngineD.lib")
#else
#pragma comment (lib,"../../../../ϵͳģ��/���ӿ�/Unicode/ServiceCoreD.lib")
#pragma comment (lib,"../../../../ϵͳģ��/���ӿ�/Unicode/KernelEngineD.lib")
#endif
#endif



