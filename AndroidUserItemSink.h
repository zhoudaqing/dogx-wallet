#ifndef ANDROID_USER_ITEM_SINK_HEAD_FILE
#define ANDROID_USER_ITEM_SINK_HEAD_FILE

#include "EntityManager.h"
#include "Fish.h"

////////////////////////////////////////////////////////////本源码由网猫YZ发布，QQ交流群：285183716，联系QQ：738961693
enum AndroidState
{
	EST_INVALID = 0,
	EST_BUYBULLET,
	EST_PLAYING,
};


//机器人类
class CAndroidUserItemSink : public IAndroidUserItemSink
{
	//控件变量
protected:
	IAndroidUserItem *				m_pIAndroidUserItem;				//用户接口

	//函数定义
public:
	//构造函数
	CAndroidUserItemSink();
	//析构函数
	virtual ~CAndroidUserItemSink();

	//基础接口
public:
	//释放对象
	virtual VOID  Release() { }
	//是否有效
	virtual bool  IsValid() { return AfxIsValidAddress(this,sizeof(CAndroidUserItemSink))?true:false; }
	//接口查询
	virtual void *  QueryInterface(const IID & Guid, DWORD dwQueryVer);

	//控制接口
public:
	//初始接口
	virtual bool Initialization(IUnknownEx * pIUnknownEx);
	//重置接口
	virtual bool RepositionSink();

	//游戏事件
public:
	//时间消息
	virtual bool  OnEventTimer(UINT nTimerID);
	//游戏消息
	virtual bool  OnEventGameMessage(WORD wSubCmdID, void * pData, WORD wDataSize);
	//游戏消息
	virtual bool  OnEventFrameMessage(WORD wSubCmdID, void * pData, WORD wDataSize);
	//场景消息
	virtual bool  OnEventSceneMessage(BYTE cbGameStatus, bool bLookonOther, void * pData, WORD wDataSize);

	//用户事件
public:
	//用户进入
	virtual void  OnEventUserEnter(IAndroidUserItem * pIAndroidUserItem, bool bLookonUser);
	//用户离开
	virtual void  OnEventUserLeave(IAndroidUserItem * pIAndroidUserItem, bool bLookonUser);
	//用户积分
	virtual void  OnEventUserScore(IAndroidUserItem * pIAndroidUserItem, bool bLookonUser);
	//用户状态
	virtual void  OnEventUserStatus(IAndroidUserItem * pIAndroidUserItem, bool bLookonUser);
	//用户段位
	virtual void  OnEventUserSegment(IAndroidUserItem * pIAndroidUserItem, bool bLookonUser);

protected:
	//读取配置
	void ReadConfigInformation();
	//银行操作
	void BankOperate();

	TCHAR							m_szRoomName[32];						//配置房间
	//机器人存取款
	LONGLONG						m_lRobotScoreRange[2];					//最大范围
	LONGLONG						m_lRobotBankGetScore[2];				//提款数额
	int								m_nRobotBankStorageMul;					//存款倍数
	int								m_nRobotPlayTime[2];					//机器人游戏时间

	LONGLONG						m_lMoney;
	int								m_nCannonMul;

	DWORD							m_dwLastFireTime;
	float							m_fDirection;
	int								m_nFireInterVal;

	bool OnGameConfig(void* data, WORD	wSize);
	bool OnCannonSet(void* data, WORD wSize);
	bool OnChangeBullet(void* data, WORD wSize);
	bool OnCreateBullet(void* data, WORD wSize);
	bool OnCatchFish(void* data, WORD wSize);
	bool OnCreateFish(void* data, WORD wSize);
	bool OnLockFish(void* data, WORD wSize);
	bool OnSwitchScene(void* data, WORD wSize);

	int				nTargetCannon;
	LONGLONG		nTargetMoney;

	AndroidState	m_nState;

	DWORD		m_nLockID;
	FISH_TYPE	m_nLockType;
	DWORD		ChanageCannonTick;

	void OnUpdate(int fc);
	void BuyBullet();
	void ChanageCannon();
	void Fire();
	void AdjuestDirection();

	void DoLockFish();
	void UnLock();
};

#endif
