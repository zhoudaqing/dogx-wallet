#ifndef GAME_SERVER_MANAGER_HEAD_FILE
#define GAME_SERVER_MANAGER_HEAD_FILE

////////////////////////////////////////////////////////////本源码由网猫YZ发布，QQ交流群：285183716，联系QQ：738961693

//游戏服务器管理类
class CGameServiceManager : public IGameServiceManager
{
	//控件变量
protected:
	HINSTANCE							m_hDllInstance;						//DLL 句柄

	//变量定义
protected:
	tagGameServiceAttrib				m_GameServiceAttrib;				//服务属性

	//组件变量
	CGameServiceManagerHelper			m_AndroidServiceHelper;				//机器人服务

	//函数定义
public:
	//构造函数
	CGameServiceManager(void);
	//析构函数
	virtual ~CGameServiceManager(void);

	//基础接口
public:
	//释放对象
	virtual VOID  Release() { }
	//接口查询
	virtual void *  QueryInterface(const IID & Guid, DWORD dwQueryVer);

	//参数接口
public:
	//组件属性
	virtual bool GetServiceAttrib(tagGameServiceAttrib & GameServiceAttrib);
	//调整参数
	virtual bool RectifyParameter(tagGameServiceOption & GameServiceOption);

	//创建接口
public:
	//创建桌子
	virtual VOID * CreateTableFrameSink(REFGUID Guid, DWORD dwQueryVer);
	//创建机器
	virtual VOID * CreateAndroidUserItemSink(REFGUID Guid, DWORD dwQueryVer);
	//创建数据
	virtual VOID * CreateGameDataBaseEngineSink(REFGUID Guid, DWORD dwQueryVer);
};

////////////////////////////////////////////////////////////本源码由网猫YZ发布，QQ交流群：285183716，联系QQ：738961693

#endif