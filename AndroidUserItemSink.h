#ifndef ANDROID_USER_ITEM_SINK_HEAD_FILE
#define ANDROID_USER_ITEM_SINK_HEAD_FILE

#include "EntityManager.h"
#include "Fish.h"

////////////////////////////////////////////////////////////��Դ������èYZ������QQ����Ⱥ��285183716����ϵQQ��738961693
enum AndroidState
{
	EST_INVALID = 0,
	EST_BUYBULLET,
	EST_PLAYING,
};


//��������
class CAndroidUserItemSink : public IAndroidUserItemSink
{
	//�ؼ�����
protected:
	IAndroidUserItem *				m_pIAndroidUserItem;				//�û��ӿ�

	//��������
public:
	//���캯��
	CAndroidUserItemSink();
	//��������
	virtual ~CAndroidUserItemSink();

	//�����ӿ�
public:
	//�ͷŶ���
	virtual VOID  Release() { }
	//�Ƿ���Ч
	virtual bool  IsValid() { return AfxIsValidAddress(this,sizeof(CAndroidUserItemSink))?true:false; }
	//�ӿڲ�ѯ
	virtual void *  QueryInterface(const IID & Guid, DWORD dwQueryVer);

	//���ƽӿ�
public:
	//��ʼ�ӿ�
	virtual bool Initialization(IUnknownEx * pIUnknownEx);
	//���ýӿ�
	virtual bool RepositionSink();

	//��Ϸ�¼�
public:
	//ʱ����Ϣ
	virtual bool  OnEventTimer(UINT nTimerID);
	//��Ϸ��Ϣ
	virtual bool  OnEventGameMessage(WORD wSubCmdID, void * pData, WORD wDataSize);
	//��Ϸ��Ϣ
	virtual bool  OnEventFrameMessage(WORD wSubCmdID, void * pData, WORD wDataSize);
	//������Ϣ
	virtual bool  OnEventSceneMessage(BYTE cbGameStatus, bool bLookonOther, void * pData, WORD wDataSize);

	//�û��¼�
public:
	//�û�����
	virtual void  OnEventUserEnter(IAndroidUserItem * pIAndroidUserItem, bool bLookonUser);
	//�û��뿪
	virtual void  OnEventUserLeave(IAndroidUserItem * pIAndroidUserItem, bool bLookonUser);
	//�û�����
	virtual void  OnEventUserScore(IAndroidUserItem * pIAndroidUserItem, bool bLookonUser);
	//�û�״̬
	virtual void  OnEventUserStatus(IAndroidUserItem * pIAndroidUserItem, bool bLookonUser);
	//�û���λ
	virtual void  OnEventUserSegment(IAndroidUserItem * pIAndroidUserItem, bool bLookonUser);

protected:
	//��ȡ����
	void ReadConfigInformation();
	//���в���
	void BankOperate();

	TCHAR							m_szRoomName[32];						//���÷���
	//�����˴�ȡ��
	LONGLONG						m_lRobotScoreRange[2];					//���Χ
	LONGLONG						m_lRobotBankGetScore[2];				//�������
	int								m_nRobotBankStorageMul;					//����
	int								m_nRobotPlayTime[2];					//��������Ϸʱ��

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
