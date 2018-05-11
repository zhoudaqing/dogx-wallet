#ifndef GAME_SERVER_MANAGER_HEAD_FILE
#define GAME_SERVER_MANAGER_HEAD_FILE

////////////////////////////////////////////////////////////��Դ������èYZ������QQ����Ⱥ��285183716����ϵQQ��738961693

//��Ϸ������������
class CGameServiceManager : public IGameServiceManager
{
	//�ؼ�����
protected:
	HINSTANCE							m_hDllInstance;						//DLL ���

	//��������
protected:
	tagGameServiceAttrib				m_GameServiceAttrib;				//��������

	//�������
	CGameServiceManagerHelper			m_AndroidServiceHelper;				//�����˷���

	//��������
public:
	//���캯��
	CGameServiceManager(void);
	//��������
	virtual ~CGameServiceManager(void);

	//�����ӿ�
public:
	//�ͷŶ���
	virtual VOID  Release() { }
	//�ӿڲ�ѯ
	virtual void *  QueryInterface(const IID & Guid, DWORD dwQueryVer);

	//�����ӿ�
public:
	//�������
	virtual bool GetServiceAttrib(tagGameServiceAttrib & GameServiceAttrib);
	//��������
	virtual bool RectifyParameter(tagGameServiceOption & GameServiceOption);

	//�����ӿ�
public:
	//��������
	virtual VOID * CreateTableFrameSink(REFGUID Guid, DWORD dwQueryVer);
	//��������
	virtual VOID * CreateAndroidUserItemSink(REFGUID Guid, DWORD dwQueryVer);
	//��������
	virtual VOID * CreateGameDataBaseEngineSink(REFGUID Guid, DWORD dwQueryVer);
};

////////////////////////////////////////////////////////////��Դ������èYZ������QQ����Ⱥ��285183716����ϵQQ��738961693

#endif