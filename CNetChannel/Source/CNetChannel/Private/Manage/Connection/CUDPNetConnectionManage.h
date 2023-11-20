#pragma once

#include "CNetConnectionManage.h"


class FCNetConnection;

class FCUDPNetConnectionManage : public FCNetConnectionManage
{
public:

	explicit FCUDPNetConnectionManage(const ECNetType InNetType);

	virtual ~FCUDPNetConnectionManage();


	virtual bool Init(uint32 InIP,uint16 InPort) override;

	virtual  void Tick(const float DeltaTime) override;

	virtual void Close() override;

	virtual void Listening() override;

	virtual void Send(TArray<uint8>& InData, const TSharedPtr<FCNetConnection>& InConn) override;
	

	virtual bool IsCompletePackage(TArray<uint8>& InData,TArray<uint8>& OutData, const TSharedPtr<FCNetConnection>& InConnection, FGuid& OutGuid) override;
	
	virtual void VerificationConnectionInfo(TArray<uint8>& InData, const TSharedPtr<FCNetConnection>& InConnection) override;




	class FProcessMessage : public IQueuedWork
	{
	public:
		FProcessMessage();

		FProcessMessage(FCNetConnectionManage* InOwner,TSharedPtr<FInternetAddr>& InRemoteAddr, uint8* InData,int32 InCount);

		void Init(FCNetConnectionManage* InOwner, TSharedPtr<FInternetAddr>& InRemoteAddr, uint8* InData, int32 InCount);

	protected:
		FCNetConnectionManage* Owner;
		TArray<uint8> Buffer;
		TSharedPtr<FInternetAddr> RemoteAddr;

	public:

		virtual void DoThreadedWork() override;

		virtual void Abandon() override;

		bool FromMemoryPool;
		bool Repaid;
	};

	struct FMemoryPools
	{

		FMemoryPools(uint32 InMaxSize);

		~FMemoryPools();

		void Close();

		FProcessMessage* AllocatePackage();

	protected:

		FProcessMessage* Head;

		uint32 MaxSize;
		FProcessMessage* Flag;

	}MemoryPools;

};


