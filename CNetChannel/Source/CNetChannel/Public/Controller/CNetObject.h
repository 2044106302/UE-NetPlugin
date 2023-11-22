#pragma once
#include "CoreMinimal.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FCNetReceiveDelegate,uint32)

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
	
	FCNetReceiveDelegate CNetReceiveDelegate;

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


	virtual void LinkOutTime();

};