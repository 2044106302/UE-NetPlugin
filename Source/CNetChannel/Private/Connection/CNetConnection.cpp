#include "Connection/CNetConnection.h"
#include "../Manage/Connection/CNetConnectionManage.h"
#include "Protocols/CNetBaseProtocols.h"
#include "CNetProtocolsDefinition.h"
#include "SocketSubsystem.h"
#include "Global/CNetGlobalInfo.h"

FCNetConnection::FCNetConnection(const ECNetType InNetType, const TWeakPtr<FCNetConnectionManage>& InOwner) :
	 NetType(InNetType) ,Owner(InOwner), State(ECNetConnectionState::Uninitialized),bStartSendHearBeat(false),HeartTime(0.0f),LastTime(0.0f),RequiredReconnectionTime(0.0f),TimeoutLink(0.0f)
{
	
}

FCNetConnection::~FCNetConnection()
{

}

void FCNetConnection::Init()
{
	
}


void FCNetConnection::Tick(const float DeltaTime)
{
	// if (Controller) Controller->Tick();

	CheckBatches(DeltaTime);


	if (NetType == ECNetType::Client && bStartSendHearBeat)
	{
		
		HeartTime += DeltaTime;
		if (HeartTime >= FCNetGlobalInfo::Get()->GetConfig().HeartBeatInterval)
		{
			SendHeartBeat();
			HeartTime = 0.0;
		}
		
	}

	
	const double CurrentTime = FPlatformTime::Seconds();

	 // UE_LOG(LogCNetChannel,Error,TEXT("CurrentTime  = %f || LastTime = %f ||  FCNetGlobalInfo::Get()->GetConfig().OutLinkTime = %f"),CurrentTime,LastTime,FCNetGlobalInfo::Get()->GetConfig().OutLinkTime);
	
	if (CurrentTime - LastTime > FCNetGlobalInfo::Get()->GetConfig().OutLinkTime)
	{
		UE_LOG(LogCNetChannel, Error, TEXT("CurrentTime  = %f || LastTime = %f ||  FCNetGlobalInfo::Get()->GetConfig().OutLinkTime = %f"), CurrentTime, LastTime, FCNetGlobalInfo::Get()->GetConfig().OutLinkTime);
		UE_LOG(LogCNetChannel,Error,TEXT("FCNetGlobalInfo::Get()->GetConfig().OutLinkTime"));
		Close();
	}

	if (Controller.IsValid()) Controller->Tick(DeltaTime);
	
}

void FCNetConnection::Close()
{

	if (NetType == ECNetType::Client)
	{
		FCNetConnection* Conn = this;
		NET_PROTOCOLS_SEND(CNetP_Close)

#if DEBUG_INFO
			UE_LOG(LogTemp, Error, TEXT("				CNetP_Close         send    to          Server				"));
#endif

	}
	else if(NetType == ECNetType::Server)
	{
		RemoteAddr = FCNetConnectionManage::GetSocketSubsystem()->CreateInternetAddr();
		UE_LOG(LogCNetChannel, Warning, TEXT(" Has Client  Close   "));
	}

	if (Controller) Controller->Close();

	State = ECNetConnectionState::Uninitialized;

	bStartSendHearBeat = false;

}

void FCNetConnection::CheckBatches(const float DeltaTime)
{
	TArray<FGuid> WaitRemoveBatches;

	for (auto& Val : Batches)
	{
		// All Send Success ?
		if (Val.Value.bAck)
		{
			WaitRemoveBatches.Add(Val.Key);
			continue;
		}
		
		if (!FCNetGlobalInfo::Get()->GetConfig().bReliable) continue;

		// Reliable

		for (auto& TmpSequence : Val.Value.Sequence)
		{
			if (!TmpSequence.Value.bStartUpRepackaging) continue;
			
			if (TmpSequence.Value.bAck) continue;
			
			TmpSequence.Value.CurrentTime += DeltaTime;

			if (TmpSequence.Value.CurrentTime < FCNetGlobalInfo::Get()->GetConfig().RepackagingTime) continue;

			TmpSequence.Value.CurrentTime = 0.f;
			TmpSequence.Value.RepeatCount++;

			Owner.Pin()->Send(TmpSequence.Value.Package,this->AsShared());
			
		}
		
		
	}

	for (auto& Val : WaitRemoveBatches)
	{
		Batches.Remove(Val);
	}
	
}

FORCEINLINE void FCNetConnection::StartSendHeartBeat()
{
	
	State = ECNetConnectionState::Listening;
	bStartSendHearBeat = true;
	ResetHeartBeat();
	
}

FORCEINLINE void FCNetConnection::SendHeartBeat()
{
	FCNetConnection* Conn = this;
	NET_PROTOCOLS_SEND(CNetP_HeartBeat)

#if DEBUG_INFO
		UE_LOG(LogCNetChannel, Warning, TEXT("Client            Send                    Heart                      Beat   !!!!!!!!!!!!!"));
#endif
	
	
}

FORCEINLINE void FCNetConnection::ResponseHeartBeat()
{
	FCNetConnection* Conn = this;
	NET_PROTOCOLS_SEND(CNetP_HeartBeatResponse)
}

void FCNetConnection::Send(TArray<uint8>& InData, bool bInForceSend)
{
	if (NetType == ECNetType::Server)
	{
		UE_LOG(LogCNetChannel, Warning, TEXT("Server            Send                "));
	}

}

FORCEINLINE ECNetConnectionState FCNetConnection::GetState() const
{
	return State;

}

FORCEINLINE void FCNetConnection::SetState(ECNetConnectionState InState)
{
	
	if (InState == ECNetConnectionState::Listening) ResetHeartBeat();

	State = InState;
}





FORCEINLINE void FCNetConnection::SetGuid(const FGuid& InGuid) 
{
	UE_LOG(LogCNetChannel,Warning,TEXT("  Has Client Join    "));
	ID = InGuid;
}

FORCEINLINE TSharedPtr<FInternetAddr> FCNetConnection::GetRemoteAddr()
{
	if ( !RemoteAddr.IsValid() )
	{
		RemoteAddr = FCNetConnectionManage::GetSocketSubsystem()->CreateInternetAddr();

		if ( NetType == ECNetType::Client && State == ECNetConnectionState::LocalConnection)
		{

			
			bool bTemp = false;
			RemoteAddr->SetIp(*FCNetGlobalInfo::Get()->GetConfig().TargetIP, bTemp);
			RemoteAddr->SetPort(FCNetGlobalInfo::Get()->GetConfig().TargetPort);

		}
	}

	return RemoteAddr;
}


FORCEINLINE void FCNetConnection::AcceptRemoteAddr(const TSharedPtr<FInternetAddr>& InRemoteAddr)
{
	RemoteAddr = InRemoteAddr;
}

FORCEINLINE void FCNetConnection::DataEnQueue(const TArray<uint8>& InData)
{
	DataQueue.Enqueue(InData);
}

FORCEINLINE void FCNetConnection::DataDeQueue(TArray<uint8>& InData)
{
	DataQueue.Dequeue(InData);
}



FORCEINLINE  const TSharedPtr<UCNetController>& FCNetConnection::GetController() const
{
	return  Controller;
}

void FCNetConnection::ResetHeartBeat() const
{
	LastTime = FPlatformTime::Seconds();
}

void FCNetConnection::Analysis(TArray<uint8>& InData)
{
	
}

FORCEINLINE void FCNetConnection::ConnectVerification()
{
	FCNetConnection* Conn = this;
	FString LocalVersion = FCNetGlobalInfo::Get()->GetConfig().Version;

#if DEBUG_INFO
	UE_LOG(LogCNetChannel, Warning, TEXT("Client Will Send Version = %s    to Server"), *LocalVersion);
#endif


	NET_PROTOCOLS_SEND(CNetP_LinkRequest,LocalVersion);

	SetState(ECNetConnectionState::Verify);
	
}

void FCNetConnection::CheckLoginTimeOut(float DeltaTime)
{
	if (NetType != ECNetType::Client) return;

	if (State != ECNetConnectionState::Listening && State != ECNetConnectionState::Uninitialized)
	{
		RequiredReconnectionTime += DeltaTime;

		if (RequiredReconnectionTime >= FCNetGlobalInfo::Get()->GetConfig().HeartBeatInterval)
		{
			RequiredReconnectionTime = 0.0;
			ConnectVerification();
		}
		
		TimeoutLink += DeltaTime;
		
		// UE_LOG(LogCNetChannel, Error, TEXT("TimeoutLink  ==== %f"), TimeoutLink);
		if (TimeoutLink >= FCNetGlobalInfo::Get()->GetConfig().OutLinkTime)
		{
			TimeoutLink = 0.0f;

			if (Controller) Controller->LinkFailure();
	
		}
		
	}
	else
	{
		RequiredReconnectionTime = 0.0f;
		TimeoutLink = 0.0f;
	}

}



void FCNetConnection::SendBatch(const FGuid& InDataID, uint32 InIndex,  uint32 InProtocol)
{
	
	if (Owner.IsValid())
	{

		if (FBatch* Batch = Batches.Find(InDataID))
		{

			if (FBatch::FElement* Element = Batch->Sequence.Find(InIndex) )
			{

				FCNetPackageHead* PackageHead = reinterpret_cast<FCNetPackageHead*>(Element->Package.GetData());
				
				PackageHead->Protocol = InProtocol;

				PackageHead = reinterpret_cast<FCNetPackageHead*>(Element->Package.GetData());
				
#if DEBUG_INFO
				UE_LOG(LogCNetChannel, Warning, TEXT("SendBatch  Head Protocol = %i"), PackageHead->Protocol);
#endif
				Owner.Pin()->Send(Element->Package,this->AsShared());
				Element->bStartUpRepackaging = true;
				
			}
			
		}
		
	}
	
}

void FCNetConnection::ReceiveSuccess(const FGuid& InDataID, const uint32 InIndex,const bool bAck, const bool bAllSuccessful)
{
	if (FBatch* Batch = Batches.Find(InDataID))
	{
		if (bAllSuccessful)
		{
			Batch->bAck = true;
		}
		else
		{
			if (FBatch::FElement * Element = Batch->Sequence.Find(InIndex))
			{
				Element->bAck = bAck;
			}
		}
		
		
	}
	
}


bool FCNetConnection::Receive(TArray<uint8>& InData)
{

	if (DataQueue.IsEmpty()) return false;

	return DataQueue.Dequeue(InData);
	
}

const TWeakPtr<FCNetConnectionManage>& FCNetConnection::GetOwner() const
{
	return Owner;

	
}
