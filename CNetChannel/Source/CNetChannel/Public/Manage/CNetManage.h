#pragma once
#include "CNetChannelTypes.h"

#include "../../Private/Manage/Connection/CNetConnectionManage.h"

#define DEBUG_INFO 0

class  CNETCHANNEL_API FCNetManage 
{
protected:

	using Super = FCNetManage;
	
	explicit  FCNetManage(const ECNetType InNetType);
	
	ECNetType NetType;
	
public:
	virtual ~FCNetManage();

	static FCNetManage* CreateNetManage(ECNetType InNetType);



	template<class T>
	bool RegisterController(T* InType)
	{
		if (bInit)
		{
			ConnectionManage->RegisterController(InType);

			return true;
		}

		return false;
	}

	FCNetConnection* GetLocalConnection() const;
	
	virtual bool Init(int32 InPort = 0);
	
	virtual  bool Init(const FString& InIP,uint16 InPort);

	virtual bool Init(uint32 InIp,uint16 InPort);

	virtual void Tick(float DeltaTime);
	
	virtual void Close();


protected:

	TSharedPtr<FCNetConnectionManage> ConnectionManage;
	
	uint8 bInit : 1;
};
