#pragma once
#include "CoreMinimal.h"

// #include "CNetObject.generated.h"

class FCNetConnection;

class CNETCHANNEL_API UCNetController
{

	friend class FCNetConnection;
	friend class FCUDPNetConnection;
	friend class FCUDPNetConnectionManage;

protected:
	using Super = UCNetController;
	FCNetConnection* Conn;

public:

	template<class T>
	static T* Type() 
	{
		T* Temp = nullptr;
		return Temp;
	}


public:

	explicit UCNetController();

	virtual  ~UCNetController() {};


	
	virtual void Init();

	virtual void Tick(float DeltaTime) ;

	virtual void Close();

	virtual void ReceiveProtocol(uint32 InProtocol) ;

protected:

	void RequestObtainTheRemoteIdleConnectionNum();

	void RemoteIdleNum(int32 InNum);

	virtual void LinkFailure() ;

	virtual void Upgrade() ;

	virtual void LinkSucceed();


};