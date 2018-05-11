#ifndef TABLE_FRAME_SINK_HEAD_FILE
#define TABLE_FRAME_SINK_HEAD_FILE

#include "Prereqs.h"
#include "EntityManager.h"
#include "Fish.h"
#include "Bullet.h"

typedef Entity_Manager<CFish> FishManager;
typedef Entity_Manager<CBullet> BulletManager;

typedef std::vector<DWORD>	IDList;

//游戏桌子类
class CTableFrameSink : public ITableFrameSink, public ITableUserAction
{
	//组件变量
public:
	ITableFrame						* m_pITableFrame;					//框架接口
	tagGameServiceOption			* m_pGameServiceOption;				//游戏配置
	tagGameServiceAttrib			* m_pGameServiceAttrib;				//游戏属性

	//函数定义
public:
	//构造函数
	CTableFrameSink();
	//析构函数
	virtual ~CTableFrameSink();

	//基础接口
public:
	//释放对象
	virtual VOID  Release();
	//接口查询
	virtual void *  QueryInterface(const IID & Guid, DWORD dwQueryVer);

	//管理接口
public:
	//复位接口
	virtual VOID RepositionSink();
	//配置接口
	virtual bool Initialization(IUnknownEx * pIUnknownEx);

	//查询接口
public:
	//游戏状态
    virtual bool IsUserPlaying(WORD wChairID);
	//查询限额
	virtual SCORE QueryConsumeQuota(IServerUserItem * pIServerUserItem) { return 0L; }
	//最少积分
	virtual SCORE QueryLessEnterScore(WORD wChairID, IServerUserItem * pIServerUserItem) { return 0L; }
	//查询是否扣服务费
	virtual bool QueryBuckleServiceCharge(WORD wChairID) { return true; }

	//消费限额
	virtual SCORE QueryConsumeQuotaScore(IServerUserItem * pIServerUserItem){return 0L;};
	//消费限额
	virtual SCORE QueryConsumeQuotaMedal(IServerUserItem * pIServerUserItem){return 0L;};
	//最少奖牌(元宝)
	virtual SCORE QueryLessEnterMedal(WORD wChairID, IServerUserItem * pIServerUserItem){return 0L;};

	//游戏事件
public:
	//游戏开始
	virtual bool OnEventGameStart();
	//游戏结束
	virtual bool OnEventGameConclude(WORD wChairID, IServerUserItem * pIServerUserItem, BYTE cbReason);
	//发送场景
	virtual bool OnEventSendGameScene(WORD wChairID, IServerUserItem * pIServerUserItem, BYTE cbGameStatus, bool bSendSecret);

	//事件接口
public:
	//时间事件
	virtual bool OnTimerMessage(DWORD dwTimerID, WPARAM dwBindParameter);
	//数据事件
	virtual bool OnDataBaseMessage(WORD wRequestID, VOID * pData, WORD wDataSize) { return true; }
	//积分事件
	virtual bool OnUserScroeNotify(WORD wChairID, IServerUserItem * pIServerUserItem, BYTE cbReason) { return true; }

	//网络接口
public:
	//游戏消息
	virtual bool OnGameMessage(WORD wSubCmdID, VOID * pData, WORD wDataSize, IServerUserItem * pIServerUserItem);
	//框架消息
	virtual bool OnFrameMessage(WORD wSubCmdID, VOID * pData, WORD wDataSize, IServerUserItem * pIServerUserItem);

	//比赛接口
public:
	//设置基数
	virtual void SetGameBaseScore(LONG lBaseScore) { }

	//动作事件
public:
	//用户坐下
	virtual bool OnActionUserSitDown(WORD wChairID, IServerUserItem * pIServerUserItem, bool bLookonUser);
	//用户起来
	virtual bool OnActionUserStandUp(WORD wChairID, IServerUserItem * pIServerUserItem, bool bLookonUser);
	//用户同意
	virtual bool OnActionUserOnReady(WORD wChairID, IServerUserItem * pIServerUserItem, VOID * pData, WORD wDataSize) { return true; }

	//用户断线
	virtual bool  OnActionUserOffLine(WORD wChairID, IServerUserItem * pIServerUserItem) ;

protected:
	SceneStyle	m_CurSceneTye;

	SCORE		m_lPlayerBullet[GAME_PLAYER];
	CANNON_TYPE	m_nPlayerCannonType[GAME_PLAYER];
	WORD		m_nPlayerCannon[GAME_PLAYER];
	DWORD		m_nCannonChangeTime[GAME_PLAYER];
	int			m_nPowerValue[GAME_PLAYER];
	DWORD		m_dwStartGameTime[GAME_PLAYER];
	SCORE		m_lPlayerStartScore[GAME_PLAYER];
	DWORD		m_LastTickTime;

	float		m_fElsape;
	float		m_fSpecialTime;
	float		m_fGroupTime;
	float		m_fSmallTime;
	float		m_fBigTime;
	float		m_fHugeTime;
	float		m_fSmallCleanTime;
	float		m_fBossTime;
	float		m_fBomTime;
	float		m_fUnknowTime;
	float		m_fFreezeTime;
	float		m_fBOSSTime;
	bool		m_bSendScene;
	float		m_fSnakeTime;
	float		m_fPylonsTime;

	static float fWeakenTime;
	static LONGLONG g_counter;
	LONGLONG m_counter;

	FishManager	m_FishManager;
	BulletManager m_BulletManager;

	IDList m_ClearFishList;
	IDList m_ClearBulletList;

	std::map<DWORD, IDList> m_CastBuletBuffer;

	bool		m_bFreeze;

	bool		m_bAllowFire;

protected:
	void ResetTable();

	void ClearChair(WORD wChairID);

	bool HasRealPlayer();

	void OnUpdate(float dt);

	void SendFish();

	void SendFish(CFish *pFish);

	void SendBulletAndWriteScore(WORD wChairID, SCORE nBullet, SCORE score);

	void ReturnBulletScore(WORD wChairID);

	void SendBullet(CBullet* bullet, bool bNew = true);
	
	void SendGameConfig(WORD wChairID);

	void SendSceneInfo(WORD wChairID);

	void SendPlayerInfo(WORD wChairID);

	void CheckCasting();

	void SendPowerValue(WORD wChairID);

	void CheckSpecialFish(CFish* pFish, CBullet* pBullet);

	void CatchFish(CFish* pFish, CBullet* pBullet, SCORE score);

	void DistributeFish(float dt);

	void SendCannonSet(WORD wChairID);

	void SendBOSSMul();

	void SendAdminInfo(IServerUserItem* pUser);

	void SendCatchInfo(IServerUserItem* pUser, CFish* pFish, SCORE score);

protected:
	bool	OnChangeScore(void * pDataBuffer, WORD wDataSize);
	bool	OnChangeCannon(void * pDataBuffer, WORD wDataSize);
	bool	OnFire(void * pDataBuffer, WORD wDataSize, IServerUserItem* pUser);
	bool	OnBulletCast(void* pDataBuffer, WORD wDataSize);
	bool	OnLockFish(void* pDataBuffer, WORD wDataSize);
	bool	OnStorageSet(void* pDataBuffer, WORD wDataSize);
	bool	OnGameCheatUser(void* pDataBuffer, WORD wDataSize);
	DWORD LockFish(FISH_TYPE* lockFishKind = NULL, int lastLockFishID = 0, FISH_TYPE lastLockFishKind = MAX_FISH_TYPE);
	LONGLONG	m_lUserScore[GAME_PLAYER];
	//	void RecordGameScore(IServerUserItem * pIServerUserItem, tagScoreInfo & ScoreInfo);
};

////////////////////////////////////////////////////////////本源码由网猫YZ发布，QQ交流群：285183716，联系QQ：738961693

#endif