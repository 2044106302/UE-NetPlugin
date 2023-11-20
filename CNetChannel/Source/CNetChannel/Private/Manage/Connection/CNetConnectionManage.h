#pragma once
#include "CNetChannelTypes.h"
#include "Connection/CNetConnection.h"
#include "../../Manage/Task/CTaskManage.h"



class ISocketSubsystem;
class FSocket;

class  FCNetConnectionManage : public TSharedFromThis<FCNetConnectionManage>
{


protected:
	
	using Super = FCNetConnectionManage;
	
	explicit FCNetConnectionManage(const ECNetType InNetType);
	
	ECNetType NetType;

public:

	virtual  ~FCNetConnectionManage();

	virtual bool Init(uint32 InIP,uint16 InPort);

	virtual  void Tick(const float DeltaTime);
	
	
	FORCEINLINE FSocket* GetSocket() const;

	FORCEINLINE void DestorySocket();

	const ECNetType GetNetType() const { return NetType; }

	virtual void Close();
	
	virtual void Listening();

	virtual void Send(TArray<uint8>& InData, const TSharedPtr<FCNetConnection>& InConn);

	int32 GetIdleConnectionNums() const;


	template<class T>
	void RegisterController(T* InType) const
	{
		if (Connections.LocalConnection.IsValid()) Connections.LocalConnection->RegisterController(InType);

		for (const auto& Val : Connections.RemoteConnections) if (Val.IsValid()) Val->RegisterController(InType);
	}

	template<uint32 InProtocols, typename ...ParamTypes>
	void MulticastToClient(ParamTypes &...InParams) 
	{
		for (const auto& Val : Connections.RemoteConnections)
		{
			if (Val->GetState() == ECNetConnectionState::Listening)
			{
				FCNetConnection Conn = Val.Get();
				NET_PROTOCOLS_SEND(InProtocols,InParams...)

			}

		}

	}


	virtual bool IsCompletePackage(TArray<uint8>& InData,TArray<uint8>& OutData, const TSharedPtr<FCNetConnection>& InConnection, FGuid& OutGuid);
	
	virtual void VerificationConnectionInfo(TArray<uint8>& InData, const TSharedPtr<FCNetConnection>& InConnection);

	
	static ISocketSubsystem* GetSocketSubsystem();

protected:
	struct FCache
    {
    	FCache();

    	uint32 TotalSize;
    	TArray<uint8> Cache;

    	void Reset();
    };
    
public:
    TMap<FGuid,FCache> Caches;

public:
	struct FConn
	{
		FConn(FCNetConnectionManage* InOwner);

		// bool AssignConnection(TSharedPtr<FInternetAddr> InAddr) const;

		FORCEINLINE TSharedPtr<FCNetConnection> GetAnEmptyConnection() const;
		
		TSharedPtr<FCNetConnection> operator[](const FGuid& InGuid) const;
		TSharedPtr<FCNetConnection> operator[](const TSharedPtr<FInternetAddr>& InAddr) const;
		
		TSharedPtr<FCNetConnection> LocalConnection;

		TArray<TSharedPtr<FCNetConnection>> RemoteConnections;
		
	private:
		FCNetConnectionManage* Owner;
		
	}Connections;
	
protected:
	
	FSocket* Socket;


	TSharedPtr<FInternetAddr> LocalAddr;
	
	TUniquePtr<FCTaskManage> TaskManage;



	uint8 bAsynchronousListening : 1;

	// TAtomic<bool> bComplete;

public:
	TAtomic<bool>  bStop ;



};
