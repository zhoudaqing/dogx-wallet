#ifndef TABLE_FRAME_SINK_HEAD_FILE
#define TABLE_FRAME_SINK_HEAD_FILE

#include "Prereqs.h"
#include "EntityManager.h"
#include "Fish.h"
#include "Bullet.h"

typedef Entity_Manager<CFish> FishManager;
typedef Entity_Manager<CBullet> BulletManager;

typedef std::vector<DWORD>	IDList;

//��Ϸ������
class CTableFrameSink : public ITableFrameSink, public ITableUserAction
{
	//�������
public:
	ITableFrame						* m_pITableFrame;					//��ܽӿ�
	tagGameServiceOption			* m_pGameServiceOption;				//��Ϸ����
	tagGameServiceAttrib			* m_pGameServiceAttrib;				//��Ϸ����

	//��������
public:
	//���캯��
	CTableFrameSink();
	//��������
	virtual ~CTableFrameSink();

	//�����ӿ�
public:
	//�ͷŶ���
	virtual VOID  Release();
	//�ӿڲ�ѯ
	virtual void *  QueryInterface(const IID & Guid, DWORD dwQueryVer);

	//����ӿ�
public:
	//��λ�ӿ�
	virtual VOID RepositionSink();
	//���ýӿ�
	virtual bool Initialization(IUnknownEx * pIUnknownEx);

	//��ѯ�ӿ�
public:
	//��Ϸ״̬
    virtual bool IsUserPlaying(WORD wChairID);
	//��ѯ�޶�
	virtual SCORE QueryConsumeQuota(IServerUserItem * pIServerUserItem) { return 0L; }
	//���ٻ���
	virtual SCORE QueryLessEnterScore(WORD wChairID, IServerUserItem * pIServerUserItem) { return 0L; }
	//��ѯ�Ƿ�۷����
	virtual bool QueryBuckleServiceCharge(WORD wChairID) { return true; }

	//�����޶�
	virtual SCORE QueryConsumeQuotaScore(IServerUserItem * pIServerUserItem){return 0L;};
	//�����޶�
	virtual SCORE QueryConsumeQuotaMedal(IServerUserItem * pIServerUserItem){return 0L;};
	//���ٽ���(Ԫ��)
	virtual SCORE QueryLessEnterMedal(WORD wChairID, IServerUserItem * pIServerUserItem){return 0L;};

	//��Ϸ�¼�
public:
	//��Ϸ��ʼ
	virtual bool OnEventGameStart();
	//��Ϸ����
	virtual bool OnEventGameConclude(WORD wChairID, IServerUserItem * pIServerUserItem, BYTE cbReason);
	//���ͳ���
	virtual bool OnEventSendGameScene(WORD wChairID, IServerUserItem * pIServerUserItem, BYTE cbGameStatus, bool bSendSecret);

	//�¼��ӿ�
public:
	//ʱ���¼�
	virtual bool OnTimerMessage(DWORD dwTimerID, WPARAM dwBindParameter);
	//�����¼�
	virtual bool OnDataBaseMessage(WORD wRequestID, VOID * pData, WORD wDataSize) { return true; }
	//�����¼�
	virtual bool OnUserScroeNotify(WORD wChairID, IServerUserItem * pIServerUserItem, BYTE cbReason) { return true; }

	//����ӿ�
public:
	//��Ϸ��Ϣ
	virtual bool OnGameMessage(WORD wSubCmdID, VOID * pData, WORD wDataSize, IServerUserItem * pIServerUserItem);
	//�����Ϣ
	virtual bool OnFrameMessage(WORD wSubCmdID, VOID * pData, WORD wDataSize, IServerUserItem * pIServerUserItem);

	//�����ӿ�
public:
	//���û���
	virtual void SetGameBaseScore(LONG lBaseScore) { }

	//�����¼�
public:
	//�û�����
	virtual bool OnActionUserSitDown(WORD wChairID, IServerUserItem * pIServerUserItem, bool bLookonUser);
	//�û�����
	virtual bool OnActionUserStandUp(WORD wChairID, IServerUserItem * pIServerUserItem, bool bLookonUser);
	//�û�ͬ��
	virtual bool OnActionUserOnReady(WORD wChairID, IServerUserItem * pIServerUserItem, VOID * pData, WORD wDataSize) { return true; }

	//�û�����
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

////////////////////////////////////////////////////////////��Դ������èYZ������QQ����Ⱥ��285183716����ϵQQ��738961693

#endif