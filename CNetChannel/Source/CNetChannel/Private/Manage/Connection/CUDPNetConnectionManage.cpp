#include "CUDPNetConnectionManage.h"

#include "Sockets.h"
#include "../../Log/CNetChannelLog.h"
#include "SocketSubsystem.h"
#include "Connection/CUDPNetConnection.h"
#include "EncryptionAndDecryption/EncryptionAndDecryption.h"
#include "Global/CNetGlobalInfo.h"
#include "Protocols/CNetBaseProtocols.h"


FCUDPNetConnectionManage::FCUDPNetConnectionManage(const ECNetType InNetType) : Super(InNetType),MemoryPools( 10 +  10 * FCNetGlobalInfo::Get()->GetConfig().MaxConnections)
{

}

FCUDPNetConnectionManage::~FCUDPNetConnectionManage()
{

	Close();
}

bool FCUDPNetConnectionManage::Init(uint32 InIP, uint16 InPort)
{
	 if(!Super::Init(InIP, InPort)) return false;
	
	Connections.LocalConnection = MakeShared<FCUDPNetConnection>(NetType,this->AsShared());
	Connections.LocalConnection->SetState(ECNetConnectionState::LocalConnection);
	Connections.LocalConnection->Init();
	
	for (auto& Val : Connections.RemoteConnections)
	{
		
		Val = MakeShared<FCUDPNetConnection>(NetType,this->AsShared());
		Val->Init();
		
	}

#if DEBUG_INFO
	UE_LOG(LogCNetChannel, Error, TEXT(" Connections Num = %i "), Connections.RemoteConnections.Num());
#endif


	
	// 启用 异步监听
	 if ( bAsynchronousListening )
	 {
	 	TaskManage = MakeUnique<FCTaskManage>( this,FCNetGlobalInfo::Get()->GetConfig().ThreadNums );
	 }

	 Socket->SetNonBlocking();

	 if (NetType == ECNetType::Client) Connections.LocalConnection->ConnectVerification();

	return true;
}

void FCUDPNetConnectionManage::Tick(const float DeltaTime)
{
	Super::Tick(DeltaTime);

	Listening();

	if (bStop)
	{
		 TaskManage->Destroy();	
	}


	
}

void FCUDPNetConnectionManage::Close()
{
	Super::Close();

	bStop = true;

	DestorySocket();

	MemoryPools.Close();


}


// If In Game thread Can  Use asynchronous
void FCUDPNetConnectionManage::Listening()
{


	if (!Socket){ UE_LOG(LogCNetChannel,Error,TEXT(" Listening :   The Socket is nulllptr")); return;}



#if PLATFORM_WINDOWS || PLATFORM_MAC || PLATFORM_LINUX
	uint8 RecvData[8196 * 1024];

	// Data.SetNum(8196 * 1024);
#elif PLATFORM_IOS || PLATFORM_ANDROID
	uint8 RecvData[8196 * 10];

	// Data.SetNum(8196 * 10);
#else
	uint8 RecvData[8196 * 100];
	// Data.SetNum(8196 * 100);
#endif

	int32 BytesRead = 0;
																									
	TSharedPtr<FInternetAddr> RemoteAddr = GetSocketSubsystem()->Get()->CreateInternetAddr();

	
	const bool bReceiveForm = Socket->RecvFrom(RecvData, FCNetGlobalInfo::Get()->GetConfig().ReceiveDataSize > 0 ? 
		FCNetGlobalInfo::Get()->GetConfig().ReceiveDataSize : 10240, BytesRead, *RemoteAddr);



	if (!bReceiveForm) return;

	

	if (BytesRead < sizeof FCNetBaseHead) return;


	CNetEncryptionAndDecryption::Decryption(RecvData,BytesRead);



	if (bAsynchronousListening)
	{
		
		FProcessMessage* WaitProcessMessage =  MemoryPools.AllocatePackage();

		if (!WaitProcessMessage)
		{
			WaitProcessMessage = new FProcessMessage(this, RemoteAddr, RecvData, BytesRead);
			WaitProcessMessage->FromMemoryPool = false;

		}

		if (TaskManage.IsValid())
		{
			WaitProcessMessage->Init(this, RemoteAddr, RecvData, BytesRead);
			TaskManage->AddQueuedWork(WaitProcessMessage);
		}


	}
	else {

		TArray<uint8> Data = TArray<uint8>(RecvData, BytesRead);

		FGuid ID;
		TArray<uint8> CompleteData;


		// As for Server. Get a connection to serve the remote end
		const TFunction<TSharedPtr<FCNetConnection>(bool&)> GetConn = [&](bool& bInIsJoinConn) ->TSharedPtr<FCNetConnection>
			{
				TSharedPtr<FCNetConnection> Conn = Connections[RemoteAddr];
				bInIsJoinConn = Conn.IsValid();
				Conn = bInIsJoinConn ? Conn : Connections.GetAnEmptyConnection();
				Conn->AcceptRemoteAddr(RemoteAddr);
				return  Conn;
			};

		// Catch
		bool bIsJoinConn = false;
		const TSharedPtr<FCNetConnection> Conn = NetType == ECNetType::Client ? Connections.LocalConnection : GetConn(bIsJoinConn);


		if (IsCompletePackage(Data, CompleteData, Conn, ID))
		{

			if (NetType == ECNetType::Server)
			{
				// Joined
				if (bIsJoinConn)
				{
					if (Conn->GetState() == ECNetConnectionState::Listening)
					{

						Conn->Analysis(CompleteData);
					}
					else
					{

						VerificationConnectionInfo(CompleteData, Conn);
					}
				}	// Didn't join
				else
				{
					VerificationConnectionInfo(CompleteData, Conn);
				}

			}
			else if (NetType == ECNetType::Client)
			{
				// Already connected
				if (Connections.LocalConnection->GetState() == ECNetConnectionState::Listening)
				{
					Connections.LocalConnection->Analysis(CompleteData);
				}
				else // Not connected yet
				{
					VerificationConnectionInfo(CompleteData, Conn);
				}

			}

		}


		if (ID != FGuid())
		{
			Caches.Remove(ID);
		}

	}







}

void FCUDPNetConnectionManage::Send(TArray<uint8>& InData, const TSharedPtr<FCNetConnection>& InConn)
{
	Super::Send(InData, InConn);

	if (Socket)
	{
		int32 BytesSend = 0;
		if (!Socket->SendTo(InData.GetData(), InData.Num(), BytesSend, *InConn->GetRemoteAddr()))
		{

#if DEBUG_INFO
			UE_LOG(LogCNetChannel, Warning, TEXT(" SendTo Error "));
#endif // DEBUG_INFO
			
		}
	}


}



bool FCUDPNetConnectionManage::IsCompletePackage(TArray<uint8>& InData, TArray<uint8>& OutData,
	const TSharedPtr<FCNetConnection>& InConnection, FGuid& OutGuid)
{


	do
	{

		if( !InConnection.IsValid() || !InData.Num() ) break;


		
		if (!FCNetGlobalInfo::Get()->GetConfig().bReliable)
		{
			OutData = InData;
			break;
		}

		FCNetPackageHead* PackageHead = reinterpret_cast<FCNetPackageHead*>(InData.GetData());
		
		if (!PackageHead || PackageHead->bAck) break;
	
		// Force Data 											+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

		if (PackageHead->bForceSend)
		{
			if ( FCache* InCache = Caches.Find(PackageHead->PackageID ) )
			{
				Caches.Remove(PackageHead->PackageID);
			}
			else
			{
				Caches.Add(PackageHead->PackageID, FCache());

				if ( FCache* UPDCache = Caches.Find(PackageHead->PackageID ) )
				{
					const int32 BoySize = InData.Num() - sizeof(FCNetPackageHead);
					const int32 Flag = UPDCache->Cache.AddUninitialized(BoySize);
					// Remove Head
					FMemory::Memcpy(&UPDCache->Cache[Flag], InData.GetData() + sizeof FCNetPackageHead, BoySize);
					OutData = UPDCache->Cache;
					OutGuid = PackageHead->PackageID;
					
				}
			}
			break;
		}

		// Normal Data 			+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

		switch ( PackageHead->Protocol )
		{
			default:
				break;
			case CNetP_PackageRequestSend:
			{

				PackageHead->Protocol = CNetP_PackageResponse;

				TArray<uint8> ResponseData;
					
				FCNetIOStream IOStream(ResponseData);
				IOStream << *PackageHead;
					
				Caches.Add(PackageHead->PackageID, FCache());

				Caches[PackageHead->PackageID].TotalSize = PackageHead->PackageSize;

				Send(ResponseData,InConnection->AsShared());

#if DEBUG_INFO
				UE_LOG(LogCNetChannel, Error, TEXT("IsCompletePackage CNetP_PackageRequestSend"));
#endif

				break;
				
			}
			case CNetP_PackageResponse:
			{
				if (const TSharedPtr<FCNetConnection> Conn = Connections[PackageHead->ConnID])
				{

					Conn->SendBatch(PackageHead->PackageID, PackageHead->PackageIndex, CNetP_PackageReceive);
				}
#if DEBUG_INFO
				UE_LOG(LogCNetChannel, Error, TEXT("IsCompletePackage CNetP_PackageResponse"));
#endif

	
				break;
			}
			case CNetP_PackageReceive:
			{
				if (FCache* Cache = Caches.Find(PackageHead->PackageID))
				{
					const int32 DataSize = InData.Num() - sizeof(FCNetPackageHead);
					const int32 Flag = Cache->Cache.AddUninitialized(DataSize);
					
					FMemory::Memcpy(&Cache->Cache[Flag], InData.GetData()  + sizeof(FCNetPackageHead), DataSize);
					
					if (Cache->Cache.Num() < static_cast<int32>(Cache->TotalSize) )
					{
						// Repacking
						PackageHead->Protocol = CNetP_PackageContinueSend;
						PackageHead->PackageIndex++;
					}
					else
					{
						PackageHead->Protocol = CNetP_PackageReceiveComplete;
						OutData = Cache->Cache;
						OutGuid = PackageHead->PackageID;
					}
					
					TArray<uint8> ResponseData;
					FCNetIOStream Stream(ResponseData);
					Stream << *PackageHead;

					Send(ResponseData,InConnection->AsShared());
#if DEBUG_INFO
					UE_LOG(LogCNetChannel, Error, TEXT("IsCompletePackage CNetP_PackageReceive"));
#endif

				}
				
				break;
			}
			case CNetP_PackageContinueSend:
			{
				if (const TSharedPtr<FCNetConnection> Conn = Connections[PackageHead->ConnID])
				{
					
					Conn->ReceiveSuccess(PackageHead->PackageID, PackageHead->PackageIndex - 1, true,false);
					
					Conn->SendBatch(PackageHead->PackageID, PackageHead->PackageIndex, CNetP_PackageReceive);
					
				}
#if DEBUG_INFO
				UE_LOG(LogCNetChannel, Error, TEXT("IsCompletePackage CNetP_PackageContinueSend"));
#endif
				
				break;
			}
			case CNetP_PackageReceiveComplete :
			{

				if (const TSharedPtr<FCNetConnection> Conn = Connections[PackageHead->ConnID])
				{
				
					Conn->ReceiveSuccess(PackageHead->PackageID, PackageHead->PackageIndex - 1, true,true);
				
				}
#if DEBUG_INFO
				UE_LOG(LogCNetChannel, Error, TEXT("IsCompletePackage CNetP_PackageReceiveComplete"));
#endif	
				break;
					
			}
			

			
		}
		

	}while (false);

	
	
	return !!OutData.Num();
}



void FCUDPNetConnectionManage::VerificationConnectionInfo(TArray<uint8>& InData,const TSharedPtr<FCNetConnection>& InConnection)
{

	if (!InConnection.IsValid() || InData.Num() < sizeof FCNetBaseHead)return;
	
	const FCNetBaseHead& BaseHead = *reinterpret_cast<FCNetBaseHead*>(InData.GetData());

	if (BaseHead.ParamNum > 0) 	InConnection->DataEnQueue(InData);
	

	FCNetConnection* Conn = InConnection.Get();
	


	if (NetType == ECNetType::Server)  // ---------------------------------------------------------------------------------------------------------------------------Server
	{
		switch (BaseHead.ProtocolsNumber)
		{
			default:
				break;
			case CNetP_LinkRequest:
			{

					FString RemoteVersion;
					NET_PROTOCOLS_RECEIVE(CNetP_LinkRequest, RemoteVersion);

#if DEBUG_INFO
					UE_LOG(LogCNetChannel, Error, TEXT("Remote Version = %s || Local Version = %s"), *RemoteVersion, *FCNetGlobalInfo::Get()->GetConfig().Version);
#endif
				
					if (RemoteVersion.IsEmpty() || RemoteVersion != FCNetGlobalInfo::Get()->GetConfig().Version)
					{
						NET_PROTOCOLS_SEND(CNetP_Upgrade);
#if DEBUG_INFO
						UE_LOG(LogCNetChannel, Error, TEXT("--------------------------------Please update if the version is different "));
#endif
						InConnection->Close();
					}
					else
					{
						
						NET_PROTOCOLS_SEND(CNetP_LinkResponse)
						Conn->SetState(ECNetConnectionState::Verify);
					}
#if DEBUG_INFO
					UE_LOG(LogCNetChannel, Error, TEXT("--------------------------------CNetP_LinkRequest"));
#endif
				break;
			}
			case CNetP_LinkWantJoin:
			{

				// None Guid
				FGuid ID = FGuid();

				NET_PROTOCOLS_RECEIVE(CNetP_LinkWantJoin,ID);

				if (ID != FGuid())
				{
					InConnection->SetGuid(ID);
					InConnection->SetState(ECNetConnectionState::WaitJoin);

					NET_PROTOCOLS_SEND(CNetP_LinkWelcomJoin)

					NET_PROTOCOLS_SEND(CNetP_LinkJoinSuccess)
#if DEBUG_INFO
					UE_LOG(LogCNetChannel, Error, TEXT("------------------------------CNetP_LinkWantJoin"));
#endif
				}
				else
				{
					NET_PROTOCOLS_SEND(CNetP_LinkFailure)

						InConnection->Close();
#if DEBUG_INFO
						UE_LOG(LogCNetChannel, Error, TEXT("------------------------------CNetP_LinkFailure"));
#endif
				}

				break;
			}
		case CNetP_LinkJoinSuccess:

				InConnection->SetState(ECNetConnectionState::Listening);
#if DEBUG_INFO
				UE_LOG(LogCNetChannel, Error, TEXT("-+--------------------------CCNetP_LinkJoinSuccess"));
#endif
				break;
			}

	}
	else // ---------------------------------------------------------------------------------------------------------------------------Client
	{

		switch (BaseHead.ProtocolsNumber)
		{
		default:
			break;
			case CNetP_LinkResponse:
			{
				FGuid ID = InConnection->GetGuid();
				NET_PROTOCOLS_SEND(CNetP_LinkWantJoin, ID);
#if DEBUG_INFO
				UE_LOG(LogCNetChannel, Error, TEXT("---------------------CNetP_LinkResponse"));
#endif
				break;
			}
			case CNetP_LinkWelcomJoin:
			{
				NET_PROTOCOLS_SEND(CNetP_LinkJoinSuccess);

				InConnection->StartSendHeartBeat();
#if DEBUG_INFO
				UE_LOG(LogCNetChannel, Error, TEXT("---------------------CNetP_LinkWelcomJoin"));
#endif
				break;
			}
			case CNetP_Upgrade:
			{

				NET_PROTOCOLS_RECEIVE(CNetP_Upgrade)

				InConnection->Close();
#if DEBUG_INFO
				UE_LOG(LogCNetChannel, Error, TEXT("---------------------CNetP_Upgrade"));
#endif

				break;
			}
			case CNetP_LinkFailure:
			{

				NET_PROTOCOLS_RECEIVE(CNetP_LinkFailure)

				InConnection->Close();
#if DEBUG_INFO
				UE_LOG(LogCNetChannel, Error, TEXT("---------------------CNetP_LinkFailure"));
#endif


			}

		
		}

	}




}



FCUDPNetConnectionManage::FProcessMessage::FProcessMessage() : Repaid(true)
{
	FromMemoryPool = true;
}

FCUDPNetConnectionManage::FProcessMessage::FProcessMessage(FCNetConnectionManage* InOwner, TSharedPtr<FInternetAddr>& InRemoteAddr, uint8* InData, int32 InCount) : Owner(InOwner), RemoteAddr(InRemoteAddr)
{
	if (InData) Buffer = TArray<uint8>(InData, InCount);

}

void FCUDPNetConnectionManage::FProcessMessage::Init(FCNetConnectionManage* InOwner, TSharedPtr<FInternetAddr>& InRemoteAddr, uint8* InData, int32 InCount)
{
	Repaid = false;
	Owner = InOwner;
	RemoteAddr = InRemoteAddr;
	if (InData) Buffer = TArray<uint8>(InData, InCount);
}


void FCUDPNetConnectionManage::FProcessMessage::DoThreadedWork()
{
	if (!Owner) return;

	FGuid ID;
	TArray<uint8> CompleteData;


	// As for Server. Get a connection to serve the remote end
	const TFunction<TSharedPtr<FCNetConnection>(bool&)> GetConn = [&](bool& bInIsJoinConn) ->TSharedPtr<FCNetConnection>
		{
			TSharedPtr<FCNetConnection> Conn = Owner->Connections[RemoteAddr];
			bInIsJoinConn = Conn.IsValid();
			Conn = bInIsJoinConn ? Conn : Owner->Connections.GetAnEmptyConnection();
			Conn->AcceptRemoteAddr(RemoteAddr);
			return  Conn;
		};

	// Catch
	bool bIsJoinConn = false;
	const TSharedPtr<FCNetConnection> Conn = Owner->GetNetType() == ECNetType::Client ? Owner->Connections.LocalConnection : GetConn(bIsJoinConn);


	if (Owner->IsCompletePackage(Buffer, CompleteData, Conn, ID))
	{

		if (Owner->GetNetType() == ECNetType::Server)
		{
			// Joined
			if (bIsJoinConn)
			{
				if (Conn->GetState() == ECNetConnectionState::Listening)
				{

					Conn->Analysis(CompleteData);
				}
				else
				{

					Owner->VerificationConnectionInfo(CompleteData, Conn);
				}
			}	// Didn't join
			else
			{
				Owner->VerificationConnectionInfo(CompleteData, Conn);
			}

		}
		else if (Owner->GetNetType() == ECNetType::Client)
		{
			// Already connected
			if (Owner->Connections.LocalConnection->GetState() == ECNetConnectionState::Listening)
			{
				Owner->Connections.LocalConnection->Analysis(CompleteData);
			}
			else // Not connected yet
			{
				Owner->VerificationConnectionInfo(CompleteData, Conn);
			}

		}

	}


	if (ID != FGuid())
	{
		Owner->Caches.Remove(ID);
	}

	Abandon();

}

void FCUDPNetConnectionManage::FProcessMessage::Abandon()
{	
	Repaid = true;

	if (!FromMemoryPool) delete this;

}

FCUDPNetConnectionManage::FMemoryPools::FMemoryPools(uint32 InMaxSize) : MaxSize(InMaxSize)
{
	Head = new FCUDPNetConnectionManage::FProcessMessage[MaxSize];

	Flag = Head;
}

FCUDPNetConnectionManage::FMemoryPools::~FMemoryPools()
{
	Close();

}

void FCUDPNetConnectionManage::FMemoryPools::Close()
{

	if (Head) delete[] Head;
	Head = nullptr;

}

FCUDPNetConnectionManage::FProcessMessage* FCUDPNetConnectionManage::FMemoryPools::AllocatePackage()
{
	Flag = Head;

	if (Flag)
	{

		for (uint32 i = 0; i < MaxSize; i++)
		{
			if (Flag->Repaid)
			{
				return Flag;
			}

			Flag++;

		}

	}

	return nullptr;
}

