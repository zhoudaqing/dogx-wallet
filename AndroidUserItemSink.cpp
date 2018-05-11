#include "Stdafx.h"
#include "AndroidUserItemSink.h"
#include "GameConfig.h"
#include "PosDefine.h"
#include "MathAide.h"

#define IDI_END_GAME			1			//结束游戏
#define IDI_CHANGE_ANGLE		2

//构造函数
CAndroidUserItemSink::CAndroidUserItemSink()
:m_lMoney(0)
,nTargetCannon(0)
,nTargetMoney(0)
,m_nCannonMul(0)
,m_dwLastFireTime(0)
,m_fDirection(0.0f)
,m_nLockID(0)
,m_nLockType(FISH_TYPE1)
,m_nFireInterVal(200)
,ChanageCannonTick(0)
{
	//接口变量
	m_pIAndroidUserItem=NULL;
	m_nState = EST_INVALID;
	RandSeed(GetTickCount());
	return;
}

//析构函数
CAndroidUserItemSink::~CAndroidUserItemSink()
{
}

//接口查询
void *  CAndroidUserItemSink::QueryInterface(REFGUID Guid, DWORD dwQueryVer)
{
	QUERYINTERFACE(IAndroidUserItemSink,Guid,dwQueryVer);
	QUERYINTERFACE_IUNKNOWNEX(IAndroidUserItemSink,Guid,dwQueryVer);
	return NULL;
}

//初始接口
bool  CAndroidUserItemSink::Initialization(IUnknownEx * pIUnknownEx)
{
	//查询接口
	m_pIAndroidUserItem=QUERY_OBJECT_PTR_INTERFACE(pIUnknownEx,IAndroidUserItem);
	if (m_pIAndroidUserItem==NULL) return false;

	return true;
}

//重置接口
bool  CAndroidUserItemSink::RepositionSink()
{
	m_lMoney= 0;
	m_nCannonMul = 0;
	m_dwLastFireTime = 0;
	m_fDirection = 0.0f;
	m_nState = EST_INVALID;
	m_nLockID = 0;
	m_nLockType = FISH_TYPE1;

	return true;
}

//时间消息
bool  CAndroidUserItemSink::OnEventTimer(UINT nTimerID)
{
	switch(nTimerID)
	{
	case IDI_END_GAME:
		{
			m_pIAndroidUserItem->SendSocketData(SUB_C_ENDGAME);
			return true;
		}
	case IDI_CHANGE_ANGLE:
		{
			AdjuestDirection();
			return true;
		}
	}
	return false;
}

//游戏消息
bool  CAndroidUserItemSink::OnEventGameMessage(WORD wSubCmdID, void * pData, WORD wDataSize)
{
	switch(wSubCmdID)
	{
	case SUB_S_ANDROID_UPDATA:
		{
			AndroidUpdata* pau = (AndroidUpdata*)pData;
			OnUpdate(pau->FishCount);
			break;
		}
	case SUB_S_GAME_CONFIG:
		return OnGameConfig(pData, wDataSize);
	case SUB_S_CHANGE_BULLET:
		return OnChangeBullet(pData, wDataSize);
	case SUB_S_CANNON_SET:
		return OnCannonSet(pData, wDataSize);
	case SUB_S_CATCH_FISH:
		return OnCatchFish(pData, wDataSize);
 	case SUB_S_CREATE_BULLET:
 		return OnCreateBullet(pData, wDataSize);
 	case SUB_S_CREATE_FISH:
 		return OnCreateFish(pData, wDataSize);
 	case SUB_S_LOCK_FISH:
 		return OnLockFish(pData, wDataSize);
	case SUB_S_SWITCH_SCENE:
		return OnSwitchScene(pData, wDataSize);
	case SUB_S_SEND_POWERVALUE:
		return true;
	}
	return true;
}

//游戏消息
bool  CAndroidUserItemSink::OnEventFrameMessage(WORD wSubCmdID, void * pData, WORD wDataSize)
{
	return true;
}

//场景消息
bool  CAndroidUserItemSink::OnEventSceneMessage(BYTE cbGameStatus, bool bLookonOther, void * pData, WORD wDataSize)
{
	return false;
}

//用户进入
void  CAndroidUserItemSink::OnEventUserEnter(IAndroidUserItem * pIAndroidUserItem, bool bLookonUser)
{
	return;
}

//用户离开
void  CAndroidUserItemSink::OnEventUserLeave(IAndroidUserItem * pIAndroidUserItem, bool bLookonUser)
{
	return;
}

//用户积分
void  CAndroidUserItemSink::OnEventUserScore(IAndroidUserItem * pIAndroidUserItem, bool bLookonUser)
{
	return;
}

//用户状态
void  CAndroidUserItemSink::OnEventUserStatus(IAndroidUserItem * pIAndroidUserItem, bool bLookonUser)
{
	return;
}

//用户段位
void  CAndroidUserItemSink::OnEventUserSegment(IAndroidUserItem * pIAndroidUserItem, bool bLookonUser)
{
	return;
}

bool CAndroidUserItemSink::OnGameConfig(void* data, WORD	wSize)
{
	if(sizeof(CMD_S_GAME_CONFIG) != wSize) return false;
	CMD_S_GAME_CONFIG* pCsc = (CMD_S_GAME_CONFIG*)data;

	_sntprintf(m_szRoomName, 32, TEXT("%s"), pCsc->m_szRoomName);
	CGameConfig::GetInstance()->m_nMaxCannon = pCsc->m_nMaxCannon;
	CGameConfig::GetInstance()->SetCell(pCsc->nCell);
	CGameConfig::GetInstance()->m_nFreezeTime = pCsc->nFreezeTime;
	CGameConfig::GetInstance()->m_nBuyOnce = pCsc->nBuyOnce;
	for(WORD i = 0; i < MAX_CANNON; ++i)
	{
		CGameConfig::GetInstance()->nBulletPrice[i] = pCsc->nBulletPrice[i];
		CGameConfig::GetInstance()->nBulletRadius[i] = pCsc->nBulletRadius[i];
		CGameConfig::GetInstance()->nCannonType[i] = pCsc->nCannonType[i];
	}
	for(WORD i = 0; i < MAX_FISH_TYPE; ++i)
	{
		CGameConfig::GetInstance()->FishSize[i] = Size(pCsc->FishSize[i][0], pCsc->FishSize[i][1]);
		CGameConfig::GetInstance()->fFishSpeed[i] = pCsc->fFishSpeed[i];
		CGameConfig::GetInstance()->nFishPrice[i] = pCsc->nFishPrice[i];
		CGameConfig::GetInstance()->nFishMinPrice[i] = pCsc->nFishMinPrice[i];
	}
	
	ReadConfigInformation();
	BankOperate();

	m_nState = EST_BUYBULLET;

	ChanageCannonTick = GetTickCount();
	nTargetCannon = RandInt(0, CGameConfig::GetInstance()->m_nMaxCannon-1);
	int nMaxPrice = CGameConfig::GetInstance()->nBulletPrice[nTargetCannon];
	nTargetMoney = RandInt(nMaxPrice*10, nMaxPrice*200);

	m_pIAndroidUserItem->SetGameTimer(IDI_END_GAME, RandInt(m_nRobotPlayTime[0], m_nRobotPlayTime[1]));
	AdjuestDirection();

	return true;
}


void CAndroidUserItemSink::OnUpdate(int fc)
{
	DWORD timeNow = GetTickCount();

	if(m_nState == EST_BUYBULLET)
	{
		if(timeNow- m_dwLastFireTime >= m_nFireInterVal && RandInt(0, 100) < 50)
			BuyBullet();
	}
	else if(m_nState == EST_PLAYING)
	{
		if(m_nCannonMul != nTargetCannon)
		{
			if(timeNow- m_dwLastFireTime >= m_nFireInterVal) ChanageCannon();
		}
		else if(timeNow - ChanageCannonTick >= 10000 && RandInt(0,100) > 95)
		{
			ChanageCannonTick = GetTickCount();
			nTargetCannon = RandInt(0, CGameConfig::GetInstance()->m_nMaxCannon-1);
		}

		if(timeNow- m_dwLastFireTime >= m_nFireInterVal)
		{
			if(fc > RandInt(10, 50) && (m_nLockID != 0 || RandInt(0, 100) < 50))
				Fire();
		}

		if(rand() % 1000 < 10) DoLockFish();
	}
}

//读取配置
void CAndroidUserItemSink::ReadConfigInformation()
{
	//设置文件名
	TCHAR szPath[MAX_PATH] = TEXT("");
	TCHAR szConfigFileName[MAX_PATH] = TEXT("");
	TCHAR OutBuf[255] = TEXT("");
	GetCurrentDirectory(sizeof(szPath), szPath);
	_sntprintf(szConfigFileName, sizeof(szConfigFileName), _T("%s\\SXYNServer\\AndroidConfig.ini"), szPath);

	//分数限制
	ZeroMemory(OutBuf, sizeof(OutBuf));
	GetPrivateProfileString(m_szRoomName, TEXT("RobotScoreMin"), _T("10000"), OutBuf, 255, szConfigFileName);
	swscanf(OutBuf, _T("%I64d"), &m_lRobotScoreRange[0]);

	ZeroMemory(OutBuf, sizeof(OutBuf));
	GetPrivateProfileString(m_szRoomName, TEXT("RobotScoreMax"), _T("1000000"), OutBuf, 255, szConfigFileName);
	swscanf(OutBuf, _T("%I64d"), &m_lRobotScoreRange[1]);

	if (m_lRobotScoreRange[1] < m_lRobotScoreRange[0])	m_lRobotScoreRange[1] = m_lRobotScoreRange[0];

	//提款数额下限
	ZeroMemory(OutBuf, sizeof(OutBuf));
	GetPrivateProfileString(m_szRoomName, TEXT("RobotBankGetMin"), _T("20000000"), OutBuf, 255, szConfigFileName);
	swscanf(OutBuf, _T("%I64d"), &m_lRobotBankGetScore[0]);

	//提款数额上限
	ZeroMemory(OutBuf, sizeof(OutBuf));
	GetPrivateProfileString(m_szRoomName, TEXT("RobotBankGetMax"), _T("30000000"), OutBuf, 255, szConfigFileName);
	swscanf(OutBuf, _T("%I64d"), &m_lRobotBankGetScore[1]);

	//存款倍数
	m_nRobotBankStorageMul = GetPrivateProfileInt(m_szRoomName, _T("RobotBankStoMul"), 20, szConfigFileName);

	m_nRobotPlayTime[0] = GetPrivateProfileInt(m_szRoomName, _T("RobotPlayTimeMin"), 300, szConfigFileName);
	m_nRobotPlayTime[1] = GetPrivateProfileInt(m_szRoomName, _T("RobotPlayTimeMax"), 600, szConfigFileName);

	if (m_nRobotBankStorageMul<0||m_nRobotBankStorageMul>100) m_nRobotBankStorageMul =20;

	m_nFireInterVal = GetPrivateProfileInt(m_szRoomName, _T("MinFireInterval"), 200, szConfigFileName);
}

//银行操作
void CAndroidUserItemSink::BankOperate()
{
	//变量定义
	IServerUserItem *pUserItem = m_pIAndroidUserItem->GetMeUserItem();
	LONGLONG lRobotScore = pUserItem->GetUserScore();
	{
		//判断存取
		while (lRobotScore > m_lRobotScoreRange[1])
		{
			LONGLONG lSaveScore=0L;

			lSaveScore = LONGLONG(lRobotScore*m_nRobotBankStorageMul/100);
			if (lSaveScore > lRobotScore)  lSaveScore = lRobotScore;

			if (lSaveScore > 0)
				m_pIAndroidUserItem->PerformSaveScore(lSaveScore);

			lRobotScore = pUserItem->GetUserScore();
		}

		if (lRobotScore < m_lRobotScoreRange[0])
		{

			SCORE lScore=rand()%(m_lRobotBankGetScore[1] - m_lRobotBankGetScore[0]) + m_lRobotBankGetScore[0];
			if (lScore > 0)
				m_pIAndroidUserItem->PerformTakeScore(lScore);
		}
	}
}

void CAndroidUserItemSink::BuyBullet()
{
	IServerUserItem *pUserItem = m_pIAndroidUserItem->GetMeUserItem();

	CMD_C_CHANGE_BULLET ccb;
	ccb.wChairID = m_pIAndroidUserItem->GetChairID();
	ccb.nCount =/* RandInt(1, 5) **/ CGameConfig::GetInstance()->m_nBuyOnce;

	if(pUserItem->GetUserScore() < ccb.nCount) BankOperate();

	m_pIAndroidUserItem->SendSocketData(SUB_C_CHANGE_SCORE, &ccb, sizeof(CMD_C_CHANGE_BULLET));

	m_dwLastFireTime = GetTickCount();
}

void CAndroidUserItemSink::ChanageCannon()
{
	if(m_nCannonMul != nTargetCannon)
	{
		CMD_C_CHANGE_CANNON ccc;
		ccc.wChairID = m_pIAndroidUserItem->GetChairID();
		ccc.bAdd = m_nCannonMul < nTargetCannon;
		m_pIAndroidUserItem->SendSocketData(SUB_C_CHANAGE_CANNON, &ccc, sizeof(CMD_C_CHANGE_CANNON));
	}
	else
	{
		m_nState = EST_PLAYING;
	}
	m_dwLastFireTime = GetTickCount();
}

bool CAndroidUserItemSink::OnChangeBullet(void* data, WORD wSize)
{
	if(sizeof(CMD_C_CHANGE_BULLET) != wSize) return false;

	CMD_C_CHANGE_BULLET* pcc = (CMD_C_CHANGE_BULLET*)data;

	if(pcc->wChairID == m_pIAndroidUserItem->GetChairID())
	{
		m_lMoney += pcc->nCount;
		if(m_lMoney >= nTargetMoney)
		{
			m_nState = EST_PLAYING;
		}
	}

	return true;
}

bool CAndroidUserItemSink::OnCannonSet(void* data, WORD wSize)
{
	if(sizeof(CMD_S_CANNON_SET) != wSize) return false;

	CMD_S_CANNON_SET* pcd = (CMD_S_CANNON_SET*)data;

	if(pcd->dwChairID == m_pIAndroidUserItem->GetChairID())
	{
		m_nCannonMul = pcd->cannonMul;
		if(m_nCannonMul == nTargetCannon)
		{
			m_nState = EST_PLAYING;
			m_dwLastFireTime = GetTickCount();
		}
	}

	return true;
}

void CAndroidUserItemSink::AdjuestDirection()
{
	static const float FireAngle[3][12] = {{ 1.87f, 2.17f, 2.47f, 2.77f, 3.07f, 3.14f, 3.21f, 3.51f, 3.81f, 4.11f, 4.41f, 4.61f },
										   { 5.98f, 5.68f, 5.38f, 5.08f, 4.91f, 4.71f, 4.61f, 4.51f, 4.34f, 4.04f, 3.74f, 3.44f },
										   { 6.58f, 6.88f, 7.18f, 7.40f, 7.60f, 7.80f, 1.57f, 1.77f, 2.08f, 2.38f, 2.48f, 2.78f }};

	int idx = 0;
	int line = 0;
	if(m_pIAndroidUserItem->GetChairID() == 2)
	{
		line = 1;
	}
	else if(m_pIAndroidUserItem->GetChairID() == 5)
	{
		line = 2;
	}
	else if(m_pIAndroidUserItem->GetChairID() == 3 || m_pIAndroidUserItem->GetChairID() == 4)
	{
		m_fDirection -= M_PI;
	}

	for(int i = 0; i < 12; ++i)
	{
		if(FireAngle[line][i] == m_fDirection)
		{
			idx = i;
			break;
		}
	}

	idx += rand() % 11 - 5;
	if(idx < 0)
		idx = rand()%6;
	else if(idx >= 12)
		idx = 12-rand()%6-1;

	m_fDirection = FireAngle[line][idx];

	if(m_pIAndroidUserItem->GetChairID() == 3 || m_pIAndroidUserItem->GetChairID() == 4)
		m_fDirection += M_PI;

	m_pIAndroidUserItem->SetGameTimer(IDI_CHANGE_ANGLE, RandInt(2, 5));
}

void CAndroidUserItemSink::Fire()
{
	if(m_lMoney > CGameConfig::GetInstance()->nBulletPrice[m_nCannonMul])
	{
		CMD_C_FIRE cf;
		cf.wChairID = m_pIAndroidUserItem->GetChairID();
		cf.fDirection = m_fDirection;

		m_pIAndroidUserItem->SendSocketData(SUB_C_FIRE, &cf, sizeof(CMD_C_FIRE));
		m_dwLastFireTime = GetTickCount();
	}
	else
	{
		m_nState = EST_BUYBULLET;
		nTargetCannon = RandInt(0, CGameConfig::GetInstance()->m_nMaxCannon-1);
		int nMaxPrice = CGameConfig::GetInstance()->nBulletPrice[nTargetCannon];
		nTargetMoney = RandInt(nMaxPrice*10, nMaxPrice*200);
	}
}

bool CAndroidUserItemSink::OnCreateBullet(void* data, WORD wSize)
{
	if(sizeof(CMD_S_CREATE_BULLET) != wSize) return false;

	CMD_S_CREATE_BULLET *pcb = (CMD_S_CREATE_BULLET*)data;

	if(pcb->wChairID == m_pIAndroidUserItem->GetChairID())
	{
		m_lMoney -= pcb->nMul;
	}

	return true;
}

bool CAndroidUserItemSink::OnCatchFish(void* data, WORD wSize)
{
	if(sizeof(CMD_S_CATCH_FISH) != wSize) return false;

	CMD_S_CATCH_FISH* pcf = (CMD_S_CATCH_FISH*)data;

	if(pcf->wChairID == m_pIAndroidUserItem->GetChairID())
	{
		m_lMoney += pcf->lScore;
	}

	if(pcf->wFishID == m_nLockID)
		UnLock();

	return true;
}

bool CAndroidUserItemSink::OnCreateFish(void* data, WORD wSize)
{
	if(sizeof(CMD_S_CREATE_FISH) != wSize) return false;

	CMD_S_CREATE_FISH* psc = (CMD_S_CREATE_FISH*)data;

	return true;
}

void CAndroidUserItemSink::DoLockFish()
{
	DWORD wMeChairID = m_pIAndroidUserItem->GetChairID();
	CMD_C_LOCK_FISH clf;
	clf.wChairID = wMeChairID;
	clf.nTypeID = m_nLockType == MAX_FISH_TYPE ? FISH_TYPE1 : m_nLockType;
	clf.dwFishID = m_nLockID;
	m_pIAndroidUserItem->SendSocketData(SUB_C_LOCK_FISH, &clf, sizeof(CMD_C_LOCK_FISH));
}

void CAndroidUserItemSink::UnLock()
{
	CMD_C_LOCK_FISH clf;
	clf.wChairID = m_pIAndroidUserItem->GetChairID();
	clf.nTypeID = MAX_FISH_TYPE;
	clf.dwFishID = 0;
	m_pIAndroidUserItem->SendSocketData(SUB_C_LOCK_FISH, &clf, sizeof(CMD_C_LOCK_FISH));
}

bool CAndroidUserItemSink::OnLockFish(void* data, WORD wSize)
{
	if(sizeof(CMD_C_LOCK_FISH) != wSize) return false;

	CMD_C_LOCK_FISH* pcl = (CMD_C_LOCK_FISH*)data;

	if(pcl->wChairID == m_pIAndroidUserItem->GetChairID())
	{
		m_nLockID = pcl->dwFishID;
		m_nLockType = pcl->nTypeID;
	}

	return true;
}

bool CAndroidUserItemSink::OnSwitchScene(void* data, WORD wSize)
{
	if(sizeof(CMD_S_SWITCH_SCENE) != wSize) return false;

	CMD_S_SWITCH_SCENE* pss = (CMD_S_SWITCH_SCENE*)data;

	UnLock();

	return true;
}


