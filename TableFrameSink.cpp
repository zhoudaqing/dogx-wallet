#include "Stdafx.h"
#include "TableFrameSink.h"
#include "GameConfig.h"
#include "FishFactory.h"
#include "PathManager.h"
#include "PosDefine.h"
#include "MathAide.h"

#define IDI_GAME_LOOP						1				//游戏循环定时器
#define IDI_FREEZE							2				//定屏
#define IDI_UPDATEBOSS						3

#define TIME_GAME_LOOP						50				//循环时间间隔(毫秒)
#define TIME_UPDATE_BOSS					1000			

// #define TIME_CHANGE_SCENE					420				//场景时间（秒）
#define TIME_CHANGE_SCENE_END				10	
#define TIME_CREATE_SCENE_FISH				10
#define TIME_BEGIN_CREATE_FISH				40
// #define TIME_SPECIAL						30
// #define TIME_SPECIAL1_BEGIN_TIME			240
// #define TIME_SPECIAL2_BEGIN_TIME			480

#define TIME_MAX_LIFE						30000

float CTableFrameSink::fWeakenTime = 0.0f;
LONGLONG CTableFrameSink::g_counter = 0;

//构造函数
CTableFrameSink::CTableFrameSink()
{
	m_pITableFrame=NULL;
	m_pGameServiceOption=NULL;
	m_pGameServiceAttrib=NULL;
	m_counter = 0;
}

//析构函数
CTableFrameSink::~CTableFrameSink(void)
{
}

VOID  CTableFrameSink::Release()
{
	m_pITableFrame->DismissGame();
}
//接口查询
void *  CTableFrameSink::QueryInterface(const IID & Guid, DWORD dwQueryVer)
{
	QUERYINTERFACE(ITableFrameSink,Guid,dwQueryVer);
	QUERYINTERFACE(ITableUserAction,Guid,dwQueryVer);
	QUERYINTERFACE_IUNKNOWNEX(ITableFrameSink,Guid,dwQueryVer);
	return NULL;
}

//初始化
bool  CTableFrameSink::Initialization(IUnknownEx * pIUnknownEx)
{
	//查询接口
	ASSERT(pIUnknownEx!=NULL);
	m_pITableFrame=QUERY_OBJECT_PTR_INTERFACE(pIUnknownEx,ITableFrame);
	if (m_pITableFrame==NULL) return false;

	//获取参数
	m_pGameServiceOption=m_pITableFrame->GetGameServiceOption();
	ASSERT(m_pGameServiceOption!=NULL);

	CGameConfig::GetInstance()->LoadConfig(TEXT(".\\SXYNServer\\Config.ini"), m_pGameServiceOption->szServerName);

	return true;
}

void CTableFrameSink::ClearChair(WORD wChairID)
{
	if(wChairID >= GAME_PLAYER) return;
	m_lPlayerBullet[wChairID] = 0;
	m_nPlayerCannonType[wChairID] =  CGameConfig::GetInstance()->nCannonType[0];
	m_nPlayerCannon[wChairID] = 0;
	m_nCannonChangeTime[wChairID] = 0;
	m_nPowerValue[wChairID] = 0;
	m_dwStartGameTime[wChairID] = GetTickCount();
	m_lPlayerStartScore[wChairID] = 0;
	m_lUserScore[wChairID] = 0;
}

void CTableFrameSink::ResetTable()
{
	CPathManager::GetInstance()->InitialisePath(".\\SXYNServer\\path\\");
	//清理定时器
	m_pITableFrame->KillGameTimer(IDI_GAME_LOOP);
	m_pITableFrame->KillGameTimer(IDI_FREEZE);
	m_pITableFrame->KillGameTimer(IDI_UPDATEBOSS);
	m_pITableFrame->SetStartMode(START_MODE_TIME_CONTROL);
	m_LastTickTime = GetTickCount();
	m_CurSceneTye = SCENE_STYLE_1;
	m_fElsape = 0.0f;
	m_fSpecialTime = 0.0f;
	m_fGroupTime = 0.0f;
	m_fSmallTime = 0.0f;
	m_fBigTime = 0.0f;
	m_fHugeTime = 0.0f;
	m_fSmallCleanTime = 0.0f;
	m_fBossTime = 0.0f;
	m_fBomTime = 0.0f;
	m_fUnknowTime = 0.0f;
	m_fFreezeTime = 0.0f;
	m_fBOSSTime = 0.0f;
	m_fSnakeTime = 0.0f;
	m_fPylonsTime = 0.0f;
	m_bSendScene = false;
	for (WORD i = 0; i < GAME_PLAYER; ++i)
	{
		ClearChair(i);
	}

	m_bFreeze = false;

	m_FishManager.Initialise();

	m_BulletManager.Initialise();

	CGameConfig::GetInstance()->SaveConfig();

	m_bAllowFire = false;
}

//复位桌子
void  CTableFrameSink::RepositionSink()
{
}

//用户断线
bool  CTableFrameSink::OnActionUserOffLine(WORD wChairID, IServerUserItem * pIServerUserItem) 
{
	return true;
}

//用户坐下
bool  CTableFrameSink::OnActionUserSitDown(WORD wChairID,IServerUserItem * pIServerUserItem, bool bLookonUser)
{
	if(!bLookonUser)
	{
		m_lPlayerBullet[wChairID] = 0;
		m_nPowerValue[wChairID] = 0;
		m_lUserScore[wChairID] = 0;
		m_lPlayerStartScore[wChairID] = pIServerUserItem->GetUserScore();
		m_dwStartGameTime[wChairID] = GetTickCount();
		if(m_pITableFrame->GetGameStatus() == GAME_STATUS_FREE)
		{
			m_pITableFrame->StartGame();
			m_pITableFrame->SetGameStatus(GAME_STATUS_PLAY);
			m_fElsape = TIME_BEGIN_CREATE_FISH;
			m_bSendScene = true;
		}
	}

	return true;
}

//用户起立
bool  CTableFrameSink::OnActionUserStandUp(WORD wChairID,IServerUserItem * pIServerUserItem, bool bLookonUser)
{
	if(!bLookonUser)
	{
		ClearChair(wChairID);

		WORD playerCount = 0;
		for(WORD i = 0; i < GAME_PLAYER; ++i)
		{
			if(m_pITableFrame->GetTableUserItem(i) != NULL)
				++playerCount;
		}

		if(playerCount == 0)
		{
			m_pITableFrame->ConcludeGame(GAME_STATUS_FREE);
			ResetTable();
		}
	}
	return true;
}


//游戏状态
bool  CTableFrameSink::IsUserPlaying(WORD wChairID)
{
	return true;
}

//游戏开始
bool  CTableFrameSink::OnEventGameStart()
{
	ResetTable();
	m_pITableFrame->SetGameTimer(IDI_GAME_LOOP, TIME_GAME_LOOP, DWORD(-1), 0);
	m_pITableFrame->SetGameTimer(IDI_UPDATEBOSS, TIME_UPDATE_BOSS, DWORD(-1), 0);
	return true;
}

//游戏结束
bool  CTableFrameSink::OnEventGameConclude( WORD wChairID, IServerUserItem * pIServerUserItem, BYTE cbReason )
{
	switch(cbReason)
	{
	case GER_NORMAL:
		{
			return true;
		}
	case GER_USER_LEAVE:
	case GER_NETWORK_ERROR:
		{
			ReturnBulletScore(wChairID);
			return true;
		}
	case GER_DISMISS:
		{
			for(wChairID = 0; wChairID < GAME_PLAYER; wChairID++)
			{
				ReturnBulletScore(wChairID);
			}
			return true;
		}
	}
	return false;
}

void CTableFrameSink::ReturnBulletScore(WORD wChairID)
{
	IServerUserItem* pIServerUserItem = m_pITableFrame->GetTableUserItem(wChairID);
	if(pIServerUserItem != NULL)
	{
		SCORE score = m_lPlayerBullet[wChairID];
		if(CGameConfig::GetInstance()->GetCell() > 1)
		{
			score = m_lPlayerBullet[wChairID] * CGameConfig::GetInstance()->GetCell();
		}
		else if(CGameConfig::GetInstance()->GetCell() < -1)
		{
			score = m_lPlayerBullet[wChairID] / -CGameConfig::GetInstance()->GetCell();
		}
		m_lUserScore[wChairID] += score;

		tagScoreInfo ScoreInfo;
		ZeroMemory(&ScoreInfo,sizeof(ScoreInfo));
		ScoreInfo.lScore=score;
		ScoreInfo.cbType = SCORE_TYPE_DRAW;
		if(m_lUserScore[wChairID] > 0)
			ScoreInfo.cbType = SCORE_TYPE_WIN;
		else if(m_lUserScore[wChairID] < 0)
			ScoreInfo.cbType = SCORE_TYPE_LOSE;
		//pIServerUserItem->WriteUserScore(score, 0, 0, 0, ScoreInfo.cbType, (GetTickCount() - m_dwStartGameTime[wChairID]) / 1000);
		//RecordGameScore(pIServerUserItem, ScoreInfo);

		m_pITableFrame->WriteUserScore(wChairID, ScoreInfo);//, 0, (GetTickCount() - m_dwStartGameTime[wChairID]) / 1000);

		ClearChair(wChairID);
	}

	m_ClearBulletList.clear();
	std::map<DWORD, CBullet*>& BulletMap = m_BulletManager.GetEntityMap();
	std::map<DWORD, CBullet*>::iterator it = BulletMap.begin();
	while(it != BulletMap.end())
	{
		CBullet* bt = it->second;
		if(bt->GetChairID() == wChairID)
			m_ClearBulletList.push_back(bt->GetID());
		it++;
	}
	int n = m_ClearBulletList.size();
	for(int i = 0; i < n; ++i)
	{
		m_BulletManager.DeleteEntity(m_ClearBulletList[i]);
	}
	m_ClearBulletList.clear();
	
}

//发送场景
bool  CTableFrameSink::OnEventSendGameScene(WORD wChairID, IServerUserItem * pIServerUserItem, BYTE cbGameStatus, bool bSendSecret)
{
	switch (cbGameStatus)
	{
	case GAME_STATUS_FREE:		//空闲状态
	case GAME_STATUS_PLAY:
		{
			SendGameConfig(wChairID);
			SendPlayerInfo(wChairID);
			SendSceneInfo(wChairID);
			SendBOSSMul();
			m_pITableFrame->SendGameMessage(pIServerUserItem, TEXT("F1上分 F2加炮 F3减炮 F4设置 F5上全分 F6翻转 空格开炮 S锁定 Q取消锁定"), SMT_CHAT);

			for(WORD i = 0; i < GAME_PLAYER; ++i)
			{
				SendPowerValue(i);
			}
			return true;
		}
	}
	return false;
}

void CTableFrameSink::SendPowerValue(WORD wChairID)
{
	CMD_S_POWER cp;
	cp.UserID = wChairID;
	cp.nPowerValue = m_nPowerValue[wChairID];
	m_pITableFrame->SendTableData(INVALID_CHAIR, SUB_S_SEND_POWERVALUE, &cp, sizeof(CMD_S_POWER));
	m_pITableFrame->SendLookonData(INVALID_CHAIR, SUB_S_SEND_POWERVALUE, &cp, sizeof(CMD_S_POWER));

}

void CTableFrameSink::SendPlayerInfo(WORD wChairID)
{
	for(WORD i = 0; i < GAME_PLAYER; ++i)
	{
		IServerUserItem* pUser = m_pITableFrame->GetTableUserItem(i);
		if(pUser == NULL) continue;
		{
			CMD_S_USER_INFO cui;
			ZeroMemory(&cui, sizeof(CMD_S_USER_INFO));

			cui.wChairID = i;
			cui.lScore = m_lPlayerBullet[i];
			cui.nCannonMul = m_nPlayerCannon[i];
			cui.nCannonType = m_nPlayerCannonType[i];
	
			m_pITableFrame->SendTableData(wChairID, SUB_S_USER_INFO, &cui, sizeof(CMD_S_USER_INFO));
			m_pITableFrame->SendLookonData(wChairID, SUB_S_USER_INFO, &cui, sizeof(CMD_S_USER_INFO));
		}
	}
}

void CTableFrameSink::SendSceneInfo(WORD wChairID)
{
	std::map<DWORD, CFish*>& FishMap = m_FishManager.GetEntityMap();
	std::map<DWORD, CFish*>::iterator it = FishMap.begin();
	while(it != FishMap.end())
	{
		SendFish(it->second);
		it++;
	}

	std::map<DWORD, CBullet*>& BulletMap = m_BulletManager.GetEntityMap();
	std::map<DWORD, CBullet*>::iterator ib = BulletMap.begin();
	while(ib != BulletMap.end())
	{
		SendBullet(ib->second, false);
		ib++;
	}

	CMD_S_SWITCH_SCENE css;
	css.bSwitching = false;
	css.st = m_CurSceneTye;

	m_pITableFrame->SendTableData(wChairID, SUB_S_SWITCH_SCENE, &css, sizeof(CMD_S_SWITCH_SCENE));
	m_pITableFrame->SendLookonData(wChairID, SUB_S_SWITCH_SCENE, &css, sizeof(CMD_S_SWITCH_SCENE));
}

void CTableFrameSink::SendGameConfig(WORD wChairID)
{
	CMD_S_GAME_CONFIG csc;
	ZeroMemory(&csc, sizeof(CMD_S_GAME_CONFIG));

	_sntprintf(csc.m_szRoomName, 32, TEXT("%s"), m_pGameServiceOption->szServerName);
	csc.m_nMaxCannon = CGameConfig::GetInstance()->m_nMaxCannon;
	csc.nCell = CGameConfig::GetInstance()->GetCell();
	csc.nFreezeTime = CGameConfig::GetInstance()->m_nFreezeTime;
	csc.m_nRangeBombRadius = CGameConfig::GetInstance()->m_nRangeBombRadius;
	csc.nBuyOnce = CGameConfig::GetInstance()->m_nBuyOnce;
	csc.nFireInterval = CGameConfig::GetInstance()->nFireInterval;
	csc.m_nIonNeedPower = CGameConfig::GetInstance()->m_nIonNeedPower;

	for(WORD i = 0; i < MAX_CANNON; ++i)
	{
		csc.nBulletPrice[i] = CGameConfig::GetInstance()->nBulletPrice[i];
		csc.nBulletRadius[i] = CGameConfig::GetInstance()->nBulletRadius[i];
		csc.nCannonType[i] = CGameConfig::GetInstance()->nCannonType[i];
	}
	for(WORD i = 0; i < MAX_FISH_TYPE; ++i)
	{
		csc.fFishSpeed[i] = CGameConfig::GetInstance()->fFishSpeed[i];
		csc.FishSize[i][0] = CGameConfig::GetInstance()->FishSize[i].width_;
		csc.FishSize[i][1] = CGameConfig::GetInstance()->FishSize[i].height_;
		csc.nFishPrice[i] = CGameConfig::GetInstance()->nFishPrice[i];
		csc.nFishMinPrice[i] = CGameConfig::GetInstance()->nFishMinPrice[i];
	}
	m_pITableFrame->SendTableData(wChairID, SUB_S_GAME_CONFIG, &csc, sizeof(csc));
	m_pITableFrame->SendLookonData(wChairID, SUB_S_GAME_CONFIG, &csc, sizeof(csc));
}
//定时器事件
bool  CTableFrameSink::OnTimerMessage(DWORD wTimerID, WPARAM wBindParam)
{
	switch(wTimerID)
	{
	case IDI_GAME_LOOP:
		{
			DWORD timeNow = GetTickCount();
			float dt = (timeNow - m_LastTickTime) / 1000.0f;
			m_LastTickTime = timeNow;
			OnUpdate(dt);
			return true;
		}
	case IDI_FREEZE:
		{
			std::map<DWORD, CFish*> FishMap = m_FishManager.GetEntityMap();
			std::map<DWORD, CFish*>::iterator it = FishMap.begin();
			while(it != FishMap.end())
			{
				CFish* pFish= it->second;
				pFish->Pause(false);
				it++;
			}
			m_bFreeze = false;

			return true;
		}
	case IDI_UPDATEBOSS:
		{
			SendBOSSMul();
			return true;
		}
	}
	return true;
}

//游戏消息处理
bool  CTableFrameSink::OnGameMessage(WORD wSubCmdID,  void * pDataBuffer, WORD wDataSize, IServerUserItem * pIServerUserItem)
{
	switch(wSubCmdID)
	{
	case SUB_C_CHANGE_SCORE:
		{
			return OnChangeScore(pDataBuffer, wDataSize);
		}
	case SUB_C_CHANAGE_CANNON:
		{
			return OnChangeCannon(pDataBuffer, wDataSize);
		}
	case SUB_C_FIRE:
		{
			return OnFire(pDataBuffer, wDataSize, pIServerUserItem);
		}
	case SUB_C_BULLET_CAST:
		{
			return OnBulletCast(pDataBuffer, wDataSize);
		}
	case SUB_C_LOCK_FISH:
		{
			return OnLockFish(pDataBuffer, wDataSize);
		}
	case SUB_C_ENDGAME:
		{
			m_pITableFrame->PerformStandUpAction(pIServerUserItem);
			return true;
		}
	case SUB_C_SOTRAGESET:
		{
			if(CUserRight::IsGameCheatUser(pIServerUserItem->GetUserRight()))
			{
				return OnStorageSet(pDataBuffer, wDataSize);
			}
			return false;
		}
	case SUB_C_CHEATUSER:
		{
			if(CUserRight::IsGameCheatUser(pIServerUserItem->GetUserRight()))
			{
				return OnGameCheatUser(pDataBuffer, wDataSize);
			}
			return false;
		}
	}
	return false;
}

bool CTableFrameSink::OnChangeScore(void * pDataBuffer, WORD wDataSize)
{
	if(sizeof(CMD_C_CHANGE_BULLET) != wDataSize) return false;
	
	CMD_C_CHANGE_BULLET* pcc = (CMD_C_CHANGE_BULLET*)pDataBuffer;
	IServerUserItem * pIServerUserItem = m_pITableFrame->GetTableUserItem(pcc->wChairID);
	if(pIServerUserItem == NULL) return false;

	SCORE nBulletCount = pcc->nCount;
	SCORE lScore = nBulletCount;
	int nCell = CGameConfig::GetInstance()->GetCell();
	if(nBulletCount > 0)
	{
		if(CGameConfig::GetInstance()->GetCell() > 1)
			lScore = nBulletCount * nCell;
		else if(nCell < -1)
			lScore = -nBulletCount / nCell;

		if(lScore > pIServerUserItem->GetUserScore())
		{
			if(nCell > 0)
			{
				nBulletCount = pIServerUserItem->GetUserScore() / nCell;
				lScore = nBulletCount * nCell;
			}
			else
			{
				nBulletCount = pIServerUserItem->GetUserScore() * -CGameConfig::GetInstance()->GetCell();
				lScore = nBulletCount / -nCell;
			}
		}
	}
	else
	{
		if(-nBulletCount > m_lPlayerBullet[pcc->wChairID])
			nBulletCount = -m_lPlayerBullet[pcc->wChairID];
		
		if(nCell > 0)
		{
			lScore = nBulletCount * nCell;
		}
		else
		{
			lScore = nBulletCount / -nCell;
		}
	}

	m_lPlayerBullet[pcc->wChairID] += nBulletCount;

	SendBulletAndWriteScore(pcc->wChairID, nBulletCount, -lScore);

	return true;
}

void CTableFrameSink::SendBulletAndWriteScore(WORD wChairID, SCORE nBullet, SCORE score)
{
	IServerUserItem* pUser = m_pITableFrame->GetTableUserItem(wChairID);
	if(pUser != NULL)
	{
		m_lUserScore[wChairID] += score;
		// 		pUser->WriteUserScore(score, 0, 0, 0, score > 0 ? SCORE_TYPE_PRESENT : SCORE_TYPE_SERVICE , 0);
		tagScoreInfo ScoreInfo;
		ZeroMemory(&ScoreInfo,sizeof(ScoreInfo));
		ScoreInfo.lScore=score;
		ScoreInfo.cbType = SCORE_TYPE_DRAW;
		if(score > 0)
			ScoreInfo.cbType = SCORE_TYPE_SERVICE;
		else if(score < 0)
			ScoreInfo.cbType = SCORE_TYPE_PRESENT;
		m_pITableFrame->WriteUserScore(wChairID, ScoreInfo);//, 0, (GetTickCount() - m_dwStartGameTime[wChairID]) / 1000);
		m_dwStartGameTime[wChairID] = GetTickCount();
	}

	CMD_C_CHANGE_BULLET ccb;
	ccb.wChairID = wChairID;
	ccb.nCount = nBullet;

	m_pITableFrame->SendTableData(INVALID_CHAIR, SUB_S_CHANGE_BULLET, &ccb, sizeof(CMD_C_CHANGE_BULLET));
	m_pITableFrame->SendLookonData(INVALID_CHAIR, SUB_S_CHANGE_BULLET, &ccb, sizeof(CMD_C_CHANGE_BULLET));
}

//框架消息处理
bool  CTableFrameSink::OnFrameMessage(WORD wSubCmdID,  void * pDataBuffer, WORD wDataSize, IServerUserItem * pIServerUserItem)
{
	return false;
}

bool CTableFrameSink::HasRealPlayer()
{
	for(WORD i = 0; i < GAME_PLAYER; ++i)
	{
		IServerUserItem* pUser = m_pITableFrame->GetTableUserItem(i);
		if(pUser != NULL && !pUser->IsAndroidUser())
			return true;

		pUser = m_pITableFrame->EnumLookonUserItem(i);
		if(pUser != NULL && !pUser->IsAndroidUser())
			return true;
	}
	return false;
}

void CTableFrameSink::SendAdminInfo(IServerUserItem* pUser)
{
	if(pUser == NULL) return;
	if(CUserRight::IsGameCheatUser(pUser->GetUserRight()))
	{
		CMD_S_GAME_SETTING css;
		ZeroMemory(&css, sizeof(CMD_S_GAME_SETTING));

		css.StorageScore = CGameConfig::GetInstance()->StorageScore;
		css.bAllowFailStorage = CGameConfig::GetInstance()->bAllowFailStorage;
		css.nWeakenValue = CGameConfig::GetInstance()->nWeakenValue;
		css.nWeakenCycle = CGameConfig::GetInstance()->nWeakenCycle;
		css.bUsfs = CGameConfig::GetInstance()->bUsfs;
		for(int i = 0; i < SSS_COUNT; ++i)
		{
			css.StockScoreList[i] = CGameConfig::GetInstance()->StockScoreList[i];
			css.IncreaseProbability[i] = CGameConfig::GetInstance()->IncreaseProbability[i];
		}

		m_pITableFrame->SendTableData(pUser->GetChairID(), SUB_S_GAME_SETTING, &css, sizeof(CMD_S_GAME_SETTING));

		std::map<DWORD, ControlInfo> uc = CGameConfig::GetInstance()->getControlUserList();
		std::map<DWORD, ControlInfo>::iterator iu = uc.begin();
		while(iu != uc.end())
		{
			CMD_S_USER_CONTROL cuc;
			cuc.bClear = iu == uc.begin();
			cuc.UserID = iu->second.UserID;
			cuc.Probability = iu->second.Probability;
			cuc.Time = iu->second.Time;
			m_pITableFrame->SendTableData(pUser->GetChairID(), SUB_S_USER_CONTROL, &cuc, sizeof(CMD_S_USER_CONTROL));
			iu++;
		}
	}
}

void CTableFrameSink::OnUpdate(float dt)
{
	if(CGameConfig::GetInstance()->nWeakenCycle > 0)
	{
		static bool bWeaken = false;
		++m_counter;
		if(m_counter > g_counter)
		{
			++g_counter;
			fWeakenTime += dt;
		}
		else
		{
			m_counter = g_counter;
			if(!bWeaken)
				bWeaken = HasRealPlayer();
		}

		if(fWeakenTime > CGameConfig::GetInstance()->nWeakenCycle && bWeaken)
		{
			bWeaken = false;
			fWeakenTime = 0.0f;
			if(CGameConfig::GetInstance()->StorageScore > 0 && CGameConfig::GetInstance()->nWeakenValue > 0 && CGameConfig::GetInstance()->nWeakenValue <= 1000)
			{
				LONGLONG score = CGameConfig::GetInstance()->StorageScore * CGameConfig::GetInstance()->nWeakenValue / 1000;
				CGameConfig::GetInstance()->AddStorageScore(-score);
			}
		}
	}

	DistributeFish(dt);

	m_FishManager.OnUpdate(dt);

	m_BulletManager.OnUpdate(dt);

	CheckCasting();

	m_ClearFishList.clear();
	
	std::map<DWORD, CFish*>& FishMap = m_FishManager.GetEntityMap();
	std::map<DWORD, CFish*>::iterator it = FishMap.begin();
	while(it != FishMap.end())
	{
		CFish* pFish = it->second;
		if(pFish->EndPath())
		{
			m_ClearFishList.push_back(pFish->GetID());
		}
		it++;
	}
	int n = m_ClearFishList.size();
	for(int i = 0; i < n; ++i)
	{
		m_FishManager.DeleteEntity(m_ClearFishList[i]);
	}
	m_ClearFishList.clear();

	for(WORD i = 0; i < GAME_PLAYER; ++i)
	{
		IServerUserItem* pUser = m_pITableFrame->GetTableUserItem(i);
		if(pUser != NULL)
		{
			CGameConfig::GetInstance()->UpdataUserProb(pUser->GetGameID(), dt);
			if(pUser->IsAndroidUser())
			{
				AndroidUpdata au;
				au.FishCount = m_FishManager.GetCount();
				m_pITableFrame->SendTableData(i, SUB_S_ANDROID_UPDATA, &au, sizeof(AndroidUpdata));
			}
			else if(CUserRight::IsGameCheatUser(pUser->GetUserRight()))
			{
				SendAdminInfo(pUser);
			}
		}

		if(m_nCannonChangeTime[i] >= 0)
		{
			bool clear = false;
			if(m_nPlayerCannonType[i] >= CANNON_TYPE_ION_2 && GetTickCount() - m_nCannonChangeTime[i] > CGameConfig::GetInstance()->nIonTime)
			{
				m_nPlayerCannonType[i] = (CANNON_TYPE)(m_nPlayerCannonType[i] - 3);
				m_nCannonChangeTime[i] = 0;
				clear = true;
			}

			if(clear)
			{
				SendCannonSet(i);
			}
		}
	}
}

void CTableFrameSink::SendCannonSet(WORD wChairID)
{
	CMD_S_CANNON_SET ccd;
	ccd.dwChairID = wChairID;
	ccd.cannonMul = m_nPlayerCannon[wChairID];
	ccd.cannonType = m_nPlayerCannonType[wChairID];
	m_pITableFrame->SendTableData(INVALID_CHAIR, SUB_S_CANNON_SET, &ccd, sizeof(CMD_S_CANNON_SET));
	m_pITableFrame->SendLookonData(INVALID_CHAIR, SUB_S_CANNON_SET, &ccd, sizeof(CMD_S_CANNON_SET));
}

void CTableFrameSink::SendFish(CFish *pFish)
{
	CMD_S_CREATE_FISH csf;
	ZeroMemory(&csf, sizeof(CMD_S_CREATE_FISH));

	csf.nID = pFish->GetID();
	csf.nType = pFish->GetTypeID() % MAX_FISH_TYPE;
	csf.nPathID = pFish->GetPathID();
	csf.nPathType = pFish->GetPathType();
	csf.OffestX = pFish->GetOffest().x_;
	csf.OffestY = pFish->GetOffest().y_;
	csf.fDelay = pFish->GetDelay();
	csf.nData = pFish->GetData();
	csf.fSpeed = pFish->GetSpeed();
	csf.nElaspe = GetTickCount() - pFish->GetCreateTime();

	m_pITableFrame->SendTableData(INVALID_CHAIR, SUB_S_CREATE_FISH, &csf, sizeof(CMD_S_CREATE_FISH));
	m_pITableFrame->SendLookonData(INVALID_CHAIR, SUB_S_CREATE_FISH, &csf, sizeof(CMD_S_CREATE_FISH));
}

void CTableFrameSink::SendFish()
{
	CFish	fish;
	while(CFishFactory::GetInstance()->GetFish(&fish))
	{
		CFish* pFish = m_FishManager.NewEntity();

		pFish->SetTypeID(fish.GetTypeID());
		pFish->SetPathID(fish.GetPathID());
		pFish->SetPathType(fish.GetPathType());
		pFish->SetOffest(fish.GetOffest());
		pFish->SetDelay(fish.GetDelay());
		pFish->SetData(fish.GetData());
		pFish->SetSpeed(CGameConfig::GetInstance()->fFishSpeed[pFish->GetTypeID()]);
		if(pFish->GetPathType() >= SCENE1_FISH_PATH && pFish->GetPathType() <= SCENE3_FISH_PATH)
			pFish->SetSpeed(1.0f);
		pFish->SetPosition(Point(-500, -500));

		pFish->CaclutePath();

		SendFish(pFish);
	}
}

bool CTableFrameSink::OnChangeCannon(void * pDataBuffer, WORD wDataSize)
{
	if(sizeof(CMD_C_CHANGE_CANNON) != wDataSize) return false;

	CMD_C_CHANGE_CANNON* pcc = (CMD_C_CHANGE_CANNON*)pDataBuffer;
	if(m_nPlayerCannonType[pcc->wChairID] > CANNON_TYPE_NORMAL_4) return true;//离子炮或能量炮时禁止换炮

	int mul = m_nPlayerCannon[pcc->wChairID];

	if(pcc->bAdd) ++mul;
	else --mul;

	if(mul < 0) mul = CGameConfig::GetInstance()->m_nMaxCannon-1;
	if(mul >= CGameConfig::GetInstance()->m_nMaxCannon) mul = 0;

	m_nPlayerCannon[pcc->wChairID] = mul;

	CANNON_TYPE ct = m_nPlayerCannonType[pcc->wChairID];
	m_nPlayerCannonType[pcc->wChairID] = CGameConfig::GetInstance()->nCannonType[mul];
	if(ct >= CANNON_TYPE_ION_2)
		m_nPlayerCannonType[pcc->wChairID] = (CANNON_TYPE)(m_nPlayerCannonType[pcc->wChairID] + 3);

	SendCannonSet(pcc->wChairID);

	return true;
}	

bool CTableFrameSink::OnFire(void * pDataBuffer, WORD wDataSize, IServerUserItem* pIServerUserItem)
{
	if(sizeof(CMD_C_FIRE) != wDataSize) return false;

	if(m_bAllowFire && HasRealPlayer())
	{
		CMD_C_FIRE* pf = (CMD_C_FIRE*)pDataBuffer;

		if(pf->wChairID != pIServerUserItem->GetChairID()) return false;

		SCORE pirce = CGameConfig::GetInstance()->nBulletPrice[m_nPlayerCannon[pf->wChairID]];
		if(m_lPlayerBullet[pf->wChairID] >= pirce)
		{
			if(!pIServerUserItem->IsAndroidUser())
				CGameConfig::GetInstance()->AddStorageScore(pirce);

			m_lPlayerBullet[pf->wChairID] -= pirce;

			CBullet* pBullet = m_BulletManager.NewEntity();

			pBullet->SetChairID(pf->wChairID);
			pBullet->SetUserID(pIServerUserItem->GetGameID());
			pBullet->SetMultiply(pirce);
			pBullet->SetPosition(g_CannonPos[pf->wChairID]);
			pBullet->SetDirection(pf->fDirection);
			pBullet->SetTypeID(m_nPlayerCannonType[pf->wChairID]);
			pBullet->SetSpeed(CGameConfig::GetInstance()->fBulletSpeed[m_nPlayerCannon[pf->wChairID]]);
			pBullet->Create();

			SendBullet(pBullet);
		}
	}

	return true;
}

void CTableFrameSink::SendBullet(CBullet* bullet, bool bNew)
{
	if(bullet == NULL) return;

	CMD_S_CREATE_BULLET ccb;
	ZeroMemory(&ccb, sizeof(CMD_S_CREATE_BULLET));

	ccb.wChairID = bullet->GetChairID();
	ccb.dwID = bullet->GetID();
	ccb.nType = bullet->GetTypeID();
	ccb.nMul = bullet->GetMultiply();
	ccb.fSpeed = bullet->GetSpeed();
	ccb.fDirection = bullet->GetDirection();
	ccb.posX = bullet->GetPosition().x_;
	ccb.posY = bullet->GetPosition().y_;
	ccb.nElaspe = GetTickCount() - bullet->GetCreateTime();
	ccb.bNew = bNew;

	m_pITableFrame->SendTableData(INVALID_CHAIR, SUB_S_CREATE_BULLET, &ccb, sizeof(CMD_S_CREATE_BULLET));
	m_pITableFrame->SendLookonData(INVALID_CHAIR, SUB_S_CREATE_BULLET, &ccb, sizeof(CMD_S_CREATE_BULLET));
}

void CTableFrameSink::SendBOSSMul()
{
	CMD_S_BOSS_ADDMUL cca;
	cca.nFishMul = CGameConfig::GetInstance()->nBOSSCur;
	m_pITableFrame->SendTableData(INVALID_CHAIR, SUB_S_BOSS_ADDMUL, &cca, sizeof(CMD_S_BOSS_ADDMUL));
	m_pITableFrame->SendLookonData(INVALID_CHAIR, SUB_S_BOSS_ADDMUL, &cca, sizeof(CMD_S_BOSS_ADDMUL));
}

void CTableFrameSink::CheckCasting()
{
	DWORD TimeNow = GetTickCount();
	std::list<DWORD> CLearBulletList;
	std::map<DWORD, CBullet*>& BulletMap = m_BulletManager.GetEntityMap();
	std::map<DWORD, CBullet*>::iterator ib = BulletMap.begin();
	while(ib != BulletMap.end())
	{	
		CBullet* bullet = ib->second;
		if(bullet != NULL && TimeNow - bullet->GetCreateTime() > TIME_MAX_LIFE)
		{
			CLearBulletList.push_back(ib->first);
		}
		ib++;
	}
	std::list<DWORD>::iterator ic= CLearBulletList.begin();
	while(ic != CLearBulletList.end())
	{
		m_BulletManager.DeleteEntity(*ic);
		ic++;
	}
	CLearBulletList.clear();

	if(!HasRealPlayer())
	{
		m_BulletManager.Cleanup();
	}

	std::map<DWORD, IDList>::iterator it = m_CastBuletBuffer.begin();
	while(it != m_CastBuletBuffer.end())
	{
		CBullet* pBullet = m_BulletManager.GetEntity(it->first);
		if(pBullet != NULL)
		{
			int n = it->second.size();
			for(int i = 0; i < n; ++i)
			{
				CFish* pFish = m_FishManager.GetEntity(it->second[i]);
				if(pFish != NULL)
				{
					pFish->SetProb(CGameConfig::GetInstance()->nFishProbability[pFish->GetTypeID()] / n);
					if(CGameConfig::GetInstance()->CacluteProbability(pFish, pBullet))
					{
						if(pFish->GetTypeID() > FISH_TYPE20)
							CheckSpecialFish(pFish, pBullet);
						else
						{
							SCORE score = CGameConfig::GetInstance()->GetFishPrice(pFish, pBullet);
							IServerUserItem* pUser = m_pITableFrame->GetTableUserItem(pBullet->GetChairID());
							if(pUser != NULL && (pUser->IsAndroidUser() || (score < CGameConfig::GetInstance()->StorageScore||CGameConfig::GetInstance()->CanCast(pUser->GetGameID()))))
							{
								CatchFish(pFish, pBullet, score);

								m_FishManager.DeleteEntity(pFish->GetID());

								if(!pUser->IsAndroidUser())
									CGameConfig::GetInstance()->AddStorageScore(-score);

								m_lPlayerBullet[pBullet->GetChairID()] += score;
							}
						}
					}
				}
			}
			it->second.clear();
			m_BulletManager.DeleteEntity(it->first);
		}
		it++;
	}
	m_CastBuletBuffer.clear();
}

void CTableFrameSink::CheckSpecialFish(CFish* pFish, CBullet* pBullet)
{
	if(pFish == NULL || pBullet == NULL || pFish->GetTypeID() <= FISH_TYPE20) return;

	m_ClearFishList.clear();
	
	SCORE score = 0;
	std::map<DWORD, CFish*> FishMap = m_FishManager.GetEntityMap();

	if(pFish->GetTypeID() == FISH_DISH3 || pFish->GetTypeID() == FISH_DISH4)
	{
		m_ClearFishList.push_back(pFish->GetID());
		score = CGameConfig::GetInstance()->GetFishPrice(pFish, pBullet);
	}
	else if(pFish->GetTypeID() == FISH_KING)
	{
		m_ClearFishList.push_back(pFish->GetID());
		score += CGameConfig::GetInstance()->GetFishPrice(pFish, pBullet);

		std::map<DWORD, CFish*>::iterator i = FishMap.begin();
		while(i != FishMap.end())
		{
			CFish* pFish1 = i->second;
			if(pFish1->GetTypeID() == pFish->GetData())
			{
				m_ClearFishList.push_back(pFish1->GetID());
				score += CGameConfig::GetInstance()->GetFishPrice(pFish1, pBullet);
			}
			i++;
		}
	}
	else if(pFish->GetTypeID() == FISH_RANGEBOM)
	{
		m_ClearFishList.push_back(pFish->GetID());
		std::map<DWORD, CFish*>::iterator i = FishMap.begin();
		while(i != FishMap.end())
		{
			CFish* pFish1 = i->second;
			if(CMathAide::CalcDistance(pFish1->GetPosition().x_, pFish1->GetPosition().y_, pFish->GetPosition().x_, pFish->GetPosition().y_) 
				< CGameConfig::GetInstance()->m_nRangeBombRadius)
			{
				m_ClearFishList.push_back(pFish1->GetID());
				score += CGameConfig::GetInstance()->GetFishPrice(pFish1, pBullet);
			}
			i++;
		}
	}
	else if(pFish->GetTypeID() == FISH_BOM)
	{
		m_ClearFishList.push_back(pFish->GetID());
		std::map<DWORD, CFish*>::iterator i = FishMap.begin();
		while(i != FishMap.end())
		{
			CFish* pFish1 = i->second;
			m_ClearFishList.push_back(pFish1->GetID());
			score += CGameConfig::GetInstance()->GetFishPrice(pFish1, pBullet);
			i++;
		}
	}
	else if(pFish->GetTypeID() == FISH_UNKNOWN)
	{
		m_ClearFishList.push_back(pFish->GetID());
		score = CGameConfig::GetInstance()->GetFishPrice(pFish, pBullet);
	}
	else if(pFish->GetTypeID() == FISH_FREEZE)
	{
		m_ClearFishList.push_back(pFish->GetID());
	}
	else if(pFish->GetTypeID() >= FISH_SNAKE && pFish->GetTypeID() <= FISH_SNAKE_TAIL)
	{
		DWORD id = pFish->GetID();
		m_ClearFishList.push_back(id);
		score = CGameConfig::GetInstance()->GetFishPrice(pFish, pBullet);
		while(true)
		{
			CFish* pf = FishMap[--id];
			if(pf == NULL || pf->GetTypeID() < FISH_SNAKE || pf->GetTypeID() > FISH_SNAKE_TAIL) break;

			m_ClearFishList.push_back(pf->GetID());
			score += CGameConfig::GetInstance()->GetFishPrice(pf, pBullet);
		}
		id = pFish->GetID();
		while(true)
		{
			CFish* pf = FishMap[++id];
			if(pf == NULL || pf->GetTypeID() < FISH_SNAKE || pf->GetTypeID() > FISH_SNAKE_TAIL) break;

			m_ClearFishList.push_back(pf->GetID());
			score += CGameConfig::GetInstance()->GetFishPrice(pf, pBullet);
		}
	}
	else if(pFish->GetTypeID() == FISH_PYLONS)
	{
		score = 0;
		m_ClearFishList.push_back(pFish->GetID());
	}
	else //FISH_BOSS
	{
		m_ClearFishList.push_back(pFish->GetID());
		score = CGameConfig::GetInstance()->GetFishPrice(pFish, pBullet);
	}

	IServerUserItem* pUser = m_pITableFrame->GetTableUserItem(pBullet->GetChairID());
	if(pUser != NULL && (pUser->IsAndroidUser() || (score < CGameConfig::GetInstance()->StorageScore||CGameConfig::GetInstance()->CanCast(pUser->GetGameID()))))
	{
		if(m_ClearFishList.size() > 0 && pFish != NULL)
		{
			CatchFish(pFish, pBullet, score);

			if(!pUser->IsAndroidUser())
				CGameConfig::GetInstance()->AddStorageScore(-score);
			m_lPlayerBullet[pBullet->GetChairID()] += score;

			int ns = m_ClearFishList.size();
			for(int n = 0; n < ns; ++n)
			{
				m_FishManager.DeleteEntity(m_ClearFishList[n]);
			}
		}
		m_ClearFishList.clear();
	}
}

void CTableFrameSink::CatchFish(CFish* pFish, CBullet* pBullet, SCORE score)
{
	CMD_S_CATCH_FISH ccf;
	ZeroMemory(&ccf, sizeof(CMD_S_CATCH_FISH));

	WORD wc = pBullet->GetChairID();

	ccf.wChairID = wc;
	ccf.lScore = score;
	ccf.wFishID = pFish->GetID();
	ccf.nMul = pBullet->GetMultiply();
	ccf.nType = pBullet->GetTypeID();

	m_pITableFrame->SendTableData(INVALID_CHAIR, SUB_S_CATCH_FISH, &ccf, sizeof(CMD_S_CATCH_FISH));
	m_pITableFrame->SendLookonData(INVALID_CHAIR, SUB_S_CATCH_FISH, &ccf, sizeof(CMD_S_CATCH_FISH));

	IServerUserItem* pUser = m_pITableFrame->GetTableUserItem(wc);
	SendCatchInfo(pUser, pFish, score);

	if(pFish->GetTypeID() == FISH_FREEZE)
	{
		std::map<DWORD, CFish*> FishMap = m_FishManager.GetEntityMap();
		std::map<DWORD, CFish*>::iterator it = FishMap.begin();
		while(it != FishMap.end())
		{
			CFish* pFish= it->second;
			pFish->Pause(true);
			it++;
		}
		m_bFreeze = true;
		m_pITableFrame->SetGameTimer(IDI_FREEZE, CGameConfig::GetInstance()->m_nFreezeTime, 1, 0);
	}
	else if(pFish->GetTypeID() == FISH_BOSS)
	{
		CGameConfig::GetInstance()->ResetBOSS();
	}
	else if(pFish->GetTypeID() == FISH_PYLONS)
	{	
		score = CGameConfig::GetInstance()->nPylonsPower;
	}
	if(score > 0 && m_nPlayerCannonType[wc] < CANNON_TYPE_ION_2 && pBullet->GetTypeID() < CANNON_TYPE_ION_2)
	{
		m_nPowerValue[wc] += score * CGameConfig::GetInstance()->m_nPowerPrice / pBullet->GetMultiply();
		if(m_nPowerValue[wc] >= CGameConfig::GetInstance()->m_nIonNeedPower)
		{
			m_nPowerValue[wc] = 0;
			m_nPlayerCannonType[wc] = (CANNON_TYPE)(m_nPlayerCannonType[wc] + 3);
			m_nCannonChangeTime[wc] = GetTickCount();
			SendCannonSet(wc);
		}
		SendPowerValue(wc);
	}
}

bool CTableFrameSink::OnBulletCast(void* pDataBuffer, WORD wDataSize)
{
	if(sizeof(CMD_C_BULLET_CAST) != wDataSize) return false;

	CMD_C_BULLET_CAST* pbc = (CMD_C_BULLET_CAST*)pDataBuffer;

	CBullet* pBullet = m_BulletManager.GetEntity(pbc->dwID);

	if(pBullet != NULL && m_CastBuletBuffer.find(pbc->dwID) == m_CastBuletBuffer.end())
	{
		IDList fishMap;
		for(int i = 0; i < MAX_CATCH_FISH; ++i)
		{
			CFish* pFish = m_FishManager.GetEntity(pbc->dwFishID[i]);
			if(pbc->dwFishID[i] != 0 &&  pFish != NULL)
			{
				fishMap.push_back(pbc->dwFishID[i]);

				if(pFish->GetTypeID() == FISH_BOSS && pBullet->GetMultiply() == CGameConfig::GetInstance()->nBulletPrice[CGameConfig::GetInstance()->m_nMaxCannon-1])
				{
					if(CGameConfig::GetInstance()->nBOSSCur < CGameConfig::GetInstance()->nBOSSMax)
						CGameConfig::GetInstance()->nBOSSCur++;

					SendBOSSMul();
				}
			}
		}
		m_CastBuletBuffer[pbc->dwID] = fishMap;
	}

	return true;
}

void CTableFrameSink::DistributeFish(float dt)
{
	m_fElsape += dt;
	
	if(m_fElsape < CGameConfig::GetInstance()->nSceneChangeTime)
	{
		if(m_fElsape > TIME_CHANGE_SCENE_END && !m_bAllowFire)
		{
			m_bAllowFire = true;
		}

		if(m_fElsape > TIME_CREATE_SCENE_FISH && !m_bSendScene)
		{
			CFishFactory::GetInstance()->CreateSceneFish(m_CurSceneTye);
			m_bSendScene = true;
		}

		if(m_fElsape > TIME_BEGIN_CREATE_FISH && !m_bFreeze)
		{

			/*if(m_fElsape > TIME_SPECIAL1_BEGIN_TIME && m_fElsape < TIME_SPECIAL1_BEGIN_TIME + TIME_SPECIAL)
			{
				m_fSpecialTime += dt;
				if(m_fSpecialTime > TIME_CREATE_SPECIAL_FISH)
				{
					CFishFactory::GetInstance()->CreateSpecialFish(0);
					m_fSpecialTime = 0;
				}
			}
			else if(m_fElsape > TIME_SPECIAL2_BEGIN_TIME && m_fElsape < TIME_SPECIAL2_BEGIN_TIME + TIME_SPECIAL)
			{
				m_fSpecialTime += dt;
				if(m_fSpecialTime > TIME_CREATE_SPECIAL_FISH)
				{
					CFishFactory::GetInstance()->CreateSpecialFish(1);
					m_fSpecialTime = 0;
				}
			}
			else*/
			{
				m_fGroupTime+=dt;
				if(m_fGroupTime > CGameConfig::GetInstance()->nCreateGroupFishTime)
				{
					CFishFactory::GetInstance()->CreateGroupFish(CGameConfig::GetInstance()->nCreateGroupFishMin,
																 CGameConfig::GetInstance()->nCreateGroupFishMax);
					m_fGroupTime = 0.0f;
				}

				m_fSmallTime+=dt;
				if(m_fSmallTime > CGameConfig::GetInstance()->nCreateSmallFishTime)
				{
					CFishFactory::GetInstance()->CreateSmallFish(CGameConfig::GetInstance()->nCreateSmallFishCount);
					m_fSmallTime = 0;
				}

				m_fBigTime+=dt;
				if(m_fBigTime > CGameConfig::GetInstance()->nCreateBigFishTime)
				{
					CFishFactory::GetInstance()->CreateBigFish(CGameConfig::GetInstance()->nCreateBigFishCount);
					m_fBigTime = 0;
				}

				m_fHugeTime+=dt;
				if(m_fHugeTime > CGameConfig::GetInstance()->nCreateHugeFishTime)
				{
					CFishFactory::GetInstance()->CreateHugeFish(CGameConfig::GetInstance()->nCreateHugeFishCount);
					m_fHugeTime = 0;
				}

				m_fSmallCleanTime+=dt;
				if(m_fSmallCleanTime > CGameConfig::GetInstance()->nCreateSmallCleanSweepFishTime)
				{
					CFishFactory::GetInstance()->CreateSmallCleanSweepFish();
					m_fSmallCleanTime = 0;
				}

				m_fBossTime+=dt;
				if(m_fBossTime > CGameConfig::GetInstance()->nCreateBossFishTime)
				{
					CFishFactory::GetInstance()->CreateBossFish(CGameConfig::GetInstance()->nCreateBossFishCount);
					m_fBossTime = 0;
				}

				m_fBomTime+=dt;
				if(m_fBomTime > CGameConfig::GetInstance()->nCreateBomFishTime)
				{
					CFishFactory::GetInstance()->CreateSweepFish();
					m_fBomTime = 0;
				}

				m_fUnknowTime+=dt;
				if(m_fUnknowTime > CGameConfig::GetInstance()->nCreateUnknowFishTime)
				{
					CFishFactory::GetInstance()->CreateUnknowFish();
					m_fUnknowTime = 0;
				}

				m_fFreezeTime+=dt;
				if(m_fFreezeTime > CGameConfig::GetInstance()->nCreateFreezeFishTime)
				{
					CFishFactory::GetInstance()->CreateFreezeFish();
					m_fFreezeTime = 0;
				}

				m_fBOSSTime+=dt;
				if(m_fBOSSTime > CGameConfig::GetInstance()->nCreateBOSSTime)
				{
					CFishFactory::GetInstance()->CreateBOSS();
					m_fBOSSTime = 0;
				}

				m_fSnakeTime+= dt;
				if(m_fSnakeTime > CGameConfig::GetInstance()->nCreateSnakeTime)
				{
					CFishFactory::GetInstance()->CreateSnakeFish(CGameConfig::GetInstance()->nSnakeMin, CGameConfig::GetInstance()->nSnakeMax);
					m_fSnakeTime = 0;
				}
				
				m_fPylonsTime+=dt;
				if(m_fPylonsTime > CGameConfig::GetInstance()->nCreatePylonsTime)
				{
					CFishFactory::GetInstance()->CreatePylons();
					m_fPylonsTime = 0;
				}
			}
		}
	}
	else
	{
		m_fElsape = 0;
		m_fSpecialTime = 0.0f;
		m_fGroupTime = 0.0f;
		m_fSmallTime = 0.0f;
		m_fBigTime = 0.0f;
		m_fHugeTime = 0.0f;
		m_fSmallCleanTime = 0.0f;
		m_fBossTime = 0.0f;
		m_fBomTime = 0.0f;
		m_fUnknowTime = 0.0f;
		m_fFreezeTime = 0.0f;
		m_fSnakeTime = 0.0f;
		m_fBOSSTime = 0.0f;
		m_fPylonsTime = 0.0f;
		m_bFreeze = false;
		m_bAllowFire = false;
		m_bSendScene = false;
		CMD_S_SWITCH_SCENE css;
		css.st = m_CurSceneTye;
		css.bSwitching = true;
		m_pITableFrame->SendTableData(INVALID_CHAIR, SUB_S_SWITCH_SCENE, &css, sizeof(CMD_S_SWITCH_SCENE));
		m_pITableFrame->SendLookonData(INVALID_CHAIR, SUB_S_SWITCH_SCENE, &css, sizeof(CMD_S_SWITCH_SCENE));
		
		m_CurSceneTye = static_cast<SceneStyle>((m_CurSceneTye + 1) % SCENE_STYLE_COUNT);

		m_FishManager.Cleanup();

		m_BulletManager.Cleanup();
	}

	SendFish();
}


bool CTableFrameSink::OnLockFish(void* pDataBuffer, WORD wDataSize)
{
	if(sizeof(CMD_C_LOCK_FISH) != wDataSize) return false;

	CMD_C_LOCK_FISH* pcl = (CMD_C_LOCK_FISH*)pDataBuffer;

	IServerUserItem* pUser = m_pITableFrame->GetTableUserItem(pcl->wChairID);
	if(pUser != NULL && pUser->IsAndroidUser() && pcl->nTypeID != MAX_FISH_TYPE)
	{
		FISH_TYPE ft;
		DWORD fid = LockFish(&ft, pcl->dwFishID, pcl->nTypeID);
		if(ft == FISH_KING)
		{
			CFish* pFish = m_FishManager.GetEntity(fid);
			if(pFish != NULL)
				ft = (FISH_TYPE)pFish->GetData();
		}
		pcl->dwFishID = fid;
		pcl->nTypeID = ft;
	}

	m_pITableFrame->SendTableData(INVALID_CHAIR, SUB_S_LOCK_FISH, pcl, sizeof(CMD_C_LOCK_FISH));
	m_pITableFrame->SendLookonData(INVALID_CHAIR, SUB_S_LOCK_FISH, pcl, sizeof(CMD_C_LOCK_FISH));

	return true;
}

bool CTableFrameSink::OnStorageSet(void* pDataBuffer, WORD wDataSize)
{
	if(sizeof(CMD_S_GAME_SETTING) != wDataSize) return false;

	CMD_S_GAME_SETTING* pcc = (CMD_S_GAME_SETTING*)pDataBuffer;

	CGameConfig::GetInstance()->StorageScore = pcc->StorageScore;
	CGameConfig::GetInstance()->bAllowFailStorage = pcc->bAllowFailStorage;
	CGameConfig::GetInstance()->nWeakenCycle = pcc->nWeakenCycle;
	CGameConfig::GetInstance()->nWeakenValue = pcc->nWeakenValue;
	CGameConfig::GetInstance()->bUsfs = pcc->bUsfs;

	for (int i = 0; i < SSS_COUNT; ++i)
	{
		CGameConfig::GetInstance()->StockScoreList[i] = pcc->StockScoreList[i];
		CGameConfig::GetInstance()->IncreaseProbability[i] = pcc->IncreaseProbability[i];
	}

	CGameConfig::GetInstance()->SaveConfig();

	return true;
}

bool CTableFrameSink::OnGameCheatUser(void* pDataBuffer, WORD wDataSize)
{
	if(sizeof(CMD_C_CHEATUSER) != wDataSize) return false;

	CMD_C_CHEATUSER* pcc = (CMD_C_CHEATUSER*)pDataBuffer;

	if(pcc->Add)
		CGameConfig::GetInstance()->InsertUser(pcc->dwID, pcc->prob, pcc->time);
	else
		CGameConfig::GetInstance()->RemoveUser(pcc->dwID);

	CGameConfig::GetInstance()->SaveConfig();

	return true;
}

void CTableFrameSink::SendCatchInfo(IServerUserItem* pUser, CFish* pFish, SCORE score)
{
	if(pUser == NULL || pFish == NULL || score <= 0) return;

	TCHAR szInfo[256];
	TCHAR szFishName[32];

	switch(pFish->GetTypeID())
	{
	case FISH_TYPE18:
		_sntprintf(szFishName, sizeof(szFishName), TEXT("乌贼王"));
		break;
	case FISH_TYPE19:
		_sntprintf(szFishName, sizeof(szFishName), TEXT("银龙王"));
		break;
	case FISH_TYPE20:
		_sntprintf(szFishName, sizeof(szFishName), TEXT("金龙王"));
		break;
	case FISH_UNKNOWN:
		_sntprintf(szFishName, sizeof(szFishName), TEXT("宝箱"));
		break;
	case FISH_RANGEBOM:
		_sntprintf(szFishName, sizeof(szFishName), TEXT("范围炸弹"));
		break;
	case FISH_BOM:
		_sntprintf(szFishName, sizeof(szFishName), TEXT("深水炸弹"));
		break;
	case FISH_BOSS:
		_sntprintf(szFishName, sizeof(szFishName), TEXT("幽灵船"));
		break;
	case FISH_SNAKE:
	case FISH_SNAKE_TAIL:
	case FISH_SNAKE_HEAD:
		_sntprintf(szFishName, sizeof(szFishName), TEXT("金蛇王"));
		break;
	default:
		return;
	}

	_sntprintf(szInfo, sizeof(szInfo), TEXT("恭喜玩家 『 %s 』 打中了 『 %s 』 获得奖励 %I64d"), pUser->GetNickName(), szFishName, score);

	m_pITableFrame->SendGameMessage(szInfo, SMT_GLOBAL);
}

DWORD CTableFrameSink::LockFish(FISH_TYPE* lockFishKind, int lastLockFishID, FISH_TYPE lastLockFishKind)
{
	CFish* fish = NULL;
	std::map<DWORD, CFish*>::iterator iter;
	std::map<DWORD, CFish*> FishMap = m_FishManager.GetEntityMap();
	FISH_TYPE nextFishKind = lastLockFishKind;
	if (nextFishKind != MAX_FISH_TYPE) 
	{
		bool bExistFishKind[MAX_FISH_TYPE];
		memset(bExistFishKind, 0, sizeof(bExistFishKind));
		for (iter = FishMap.begin(); iter != FishMap.end(); ++iter) 
		{
			fish = iter->second;
			if (fish->GetID() == lastLockFishID) continue;
			if (fish->GetTypeID() < FISH_TYPE13) continue;
			if (!fish->InsideScreen()) continue;
			bExistFishKind[fish->GetTypeID()] = true;
		}
		for (int i = FISH_TYPE13; i < MAX_FISH_TYPE; ++i)
		{
			nextFishKind = static_cast<FISH_TYPE>((nextFishKind + 1) % MAX_FISH_TYPE);
			if (nextFishKind < FISH_TYPE13) nextFishKind = FISH_TYPE13;
			if (bExistFishKind[nextFishKind]) break;
		}
	}

	for (iter = FishMap.begin(); iter != FishMap.end(); ++iter)
	{
		fish = iter->second;
		if (fish->GetID() == lastLockFishID) continue;
		if (fish->GetTypeID() < FISH_TYPE13) continue;
		if (!fish->InsideScreen()) continue;
		if (nextFishKind == MAX_FISH_TYPE || nextFishKind == fish->GetTypeID()) 
		{
			if(lockFishKind != NULL) *lockFishKind = (FISH_TYPE)fish->GetTypeID();
			return fish->GetID();
		}
	}
	if (lastLockFishID > 0)
	{
		if(lockFishKind != NULL) *lockFishKind = lastLockFishKind;
		return lastLockFishID;
	} 
	else
	{
		if(lockFishKind != NULL) *lockFishKind = MAX_FISH_TYPE;
		return 0;
	}
}
// #include "..\..\..\开发库\Include\DataBasePacket.h"
// void CTableFrameSink::RecordGameScore( IServerUserItem * pIServerUserItem, tagScoreInfo & ScoreInfo )
// {
// 	if (!CServerRule::IsRecordGameScore(m_pGameServiceOption->dwServerRule))
// 		return;
// 
// 	if (pIServerUserItem->IsAndroidUser())
// 		return;
// 
// 	if (m_pITableFrame->GetRecordDatabaseEngine() == NULL)
// 		return;
// 
// 	LOG_DEBUG((USER, TEXT("User[%d] ScoreDiff[%I64d]"), pIServerUserItem->GetGameID(), ScoreInfo.lScore));
// 
// 
// 	//变量定义
// 	DBR_GR_GameScoreRecord GameScoreRecord;
// 	ZeroMemory(&GameScoreRecord,sizeof(GameScoreRecord));
// 
// 	//设置变量
// 	GameScoreRecord.wTableID=m_pITableFrame->GetTableID();
// 	WORD wChairID = pIServerUserItem->GetChairID();
// 	if (wChairID < GAME_PLAYER)
// 	{
// 		GameScoreRecord.dwPlayTimeCount=(DWORD)time(NULL)-(DWORD)m_dwDrawStartTime[wChairID];
// 		//游戏时间
// 		GameScoreRecord.SystemTimeStart=m_SystemTimeStart[wChairID];
// 	}
// 	GetLocalTime(&GameScoreRecord.SystemTimeConclude);
// 
// 	GameScoreRecord.	wUserCount=1;							//用户数目
// 	GameScoreRecord.wAndroidCount=0;						//机器数目
// 
// 	GameScoreRecord.wRecordCount = 1;
// 
// 	//用户信息
// 	tagGameScoreRecord * pGameScoreRecord=GameScoreRecord.GameScoreRecord;
// 	pGameScoreRecord->wChairID=pIServerUserItem->GetChairID();
// 	pGameScoreRecord->dwUserID=pIServerUserItem->GetUserID();
// 	pGameScoreRecord->cbAndroid=(pIServerUserItem->IsAndroidUser()?TRUE:FALSE);
// 
// 	//成绩信息
// 	pGameScoreRecord->lScore=ScoreInfo.lScore;
// 	pGameScoreRecord->lGrade=ScoreInfo.lGrade;
// 	pGameScoreRecord->lRevenue=ScoreInfo.lRevenue;
// 
// 	GameScoreRecord.lWasteCount-=(pGameScoreRecord->lScore+pGameScoreRecord->lRevenue);
// 	GameScoreRecord.lRevenueCount+=pGameScoreRecord->lRevenue;
// 	//附加信息
// 	pGameScoreRecord->dwUserMemal=0;
// 	pGameScoreRecord->dwPlayTimeCount=0;
// 
// 	//
// 	//投递数据
// 	WORD wHeadSize=sizeof(GameScoreRecord)-sizeof(GameScoreRecord.GameScoreRecord);
// 	WORD wDataSize=sizeof(GameScoreRecord.GameScoreRecord[0])*GameScoreRecord.wRecordCount;
// 	m_pITableFrame->GetRecordDatabaseEngine()->PostDataBaseRequest(DBR_GR_GAME_SCORE_RECORD,0,&GameScoreRecord,wHeadSize+wDataSize);
// 
// }
