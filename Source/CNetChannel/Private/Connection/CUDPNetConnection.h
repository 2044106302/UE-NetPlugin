#pragma once
#include "Connection/CNetConnection.h"

class FCUDPNetConnection : public FCNetConnection
{
public:

	explicit FCUDPNetConnection(const ECNetType InNetType,const TWeakPtr<FCNetConnectionManage>& InOwner);

	virtual ~FCUDPNetConnection();

	virtual void Init() override;

	virtual void Send(TArray<uint8>& InData, bool bInForceSend) override;
	
	virtual void Close() override;

	virtual void Tick(const float DeltaTime) override;
	
	virtual bool Receive(TArray<uint8>& InData) override;
	
	virtual void Analysis(TArray<uint8>& InData) override;
	
};
