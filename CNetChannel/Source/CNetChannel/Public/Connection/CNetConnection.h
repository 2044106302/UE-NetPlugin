#pragma once
#include "CNetChannelTypes.h"
#include "Controller/CNetObject.h"
#include "CoreUObject/Public/UObject/StrongObjectPtr.h"
#include "UObject/StrongObjectPtr.h"

class FInternetAddr;
class FCNetConnectionManage;
class  FCNetConnection : public TSharedFromThis<FCNetConnection>
{
protected:
	
	using Super = FCNetConnection;

	explicit FCNetConnection(const ECNetType InNetType,const TWeakPtr<FCNetConnectionManage>& InOwner);

public:
	virtual  ~FCNetConnection();

	virtual void Init();
	
	virtual void Tick(const float DeltaTime);

	virtual void Close();

	FORCEINLINE void CheckBatches(const float DeltaTime);

	FORCEINLINE void StartSendHeartBeat();
	
	FORCEINLINE void SendHeartBeat();

	FORCEINLINE void ResponseHeartBeat();

	FORCEINLINE void ResetHeartBeat() const;

	virtual void Analysis(TArray<uint8>& InData);

	FORCEINLINE void ConnectVerification();
	
	void CheckLoginTimeOut(float DeltaTime);
	
	virtual void Send(TArray<uint8>& InData, bool bInForceSend);

	virtual bool Receive(TArray<uint8>& InData);

	FORCEINLINE const TWeakPtr<FCNetConnectionManage>&  GetOwner() const;

	FORCEINLINE ECNetConnectionState GetState() const;

	FORCEINLINE void SetState(ECNetConnectionState InState) ;

	const FGuid& GetGuid() const { return ID; };

	FORCEINLINE void  SetGuid(const FGuid& InGuid) ;
	
	FORCEINLINE TSharedPtr<FInternetAddr> GetRemoteAddr() ;

	FORCEINLINE void AcceptRemoteAddr(const TSharedPtr<FInternetAddr>& InRemoteAddr);


	FORCEINLINE void DataEnQueue(const TArray<uint8>& InData);

	FORCEINLINE void DataDeQueue(TArray<uint8>& InData);


	template<class T>
	void RegisterController(T* InType) 
	{
		SpawnController(InType);
	}


	template<class T>
	void SpawnController(T* InType)
	{

		if (Controller) return;

		const auto SpawnObject = [this]()
		{
				if (Controller) return;



				Controller = MakeShared<T>();

				if (Controller)
				{
					Controller->Conn = this;
					Controller->Init();
				}


		};

		if (IsInGameThread())
		{
			SpawnObject();
		}
		else
		{
			FFunctionGraphTask::CreateAndDispatchWhenReady([SpawnObject]()
				{
					SpawnObject();
				}, TStatId(), nullptr, ENamedThreads::GameThread);
		}


		ID = FGuid::NewGuid();

	}

	
	
	

	FORCEINLINE  const TSharedPtr<UCNetController>& GetController() const;

	void SendBatch(const FGuid& InDataID, uint32 InIndex, uint32 InProtocol);

	void ReceiveSuccess(const FGuid& InDataID, const uint32 InIndex, const bool bAck, const bool bAllSuccessful);

protected:

	struct FBatch
	{
		FBatch()
			:bAck(false)
		{}
		struct FElement
		{
			FElement()
				:bStartUpRepackaging(false)
				,bAck(false)
				,RepeatCount(0)
				,CurrentTime(0.f)
			{}

			bool bStartUpRepackaging;
			bool bAck;
			uint8 RepeatCount;
			float CurrentTime;
		
			TArray<uint8> Package;
		};

		bool bAck;
		TMap<int32,FElement> Sequence;
	};
	TMap<FGuid,FBatch> Batches;


protected:

	TQueue<TArray<uint8>> DataQueue;

	TSharedPtr<UCNetController> Controller;

	ECNetType NetType;

	TWeakPtr<FCNetConnectionManage> Owner;

	TSharedPtr<FInternetAddr> RemoteAddr;
	
	ECNetConnectionState State;

	FGuid ID;

protected:


	uint8 bStartSendHearBeat : 1;

	double HeartTime;
	mutable  double LastTime;
	

	double RequiredReconnectionTime;
	double TimeoutLink;


};


