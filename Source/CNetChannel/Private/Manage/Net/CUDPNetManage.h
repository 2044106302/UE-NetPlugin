#pragma once
#include "Manage/CNetManage.h"
#include "../Private/Manage/Connection/CNetConnectionManage.h"

class  FCUDPNetManage : public FCNetManage
{
	
public:
	 explicit  FCUDPNetManage( const ECNetType InNetType);
	~FCUDPNetManage();

	virtual bool Init(int32 InPort = 0) override;
	
	virtual  bool Init(const FString& InIP,uint16 InPort) override;

	virtual bool Init(uint32 InIp,uint16 InPort) override;

	virtual void Tick(float DeltaTime) override;
	
	virtual void Close() override;
	
};
