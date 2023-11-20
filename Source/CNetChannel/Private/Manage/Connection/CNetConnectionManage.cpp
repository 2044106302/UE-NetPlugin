#include "CNetConnectionManage.h"
#include "Sockets.h"
#include "SocketSubsystem.h"
#include "../../Log/CNetChannelLog.h"
#include "Connection/CNetConnection.h"
#include "EncryptionAndDecryption/EncryptionAndDecryption.h"
#include "Global/CNetGlobalInfo.h"



FCNetConnectionManage::FCNetConnectionManage(const ECNetType InNetType) : Connections(this),Socket(nullptr),bAsynchronousListening(true), bStop(false)
{
	NetType = InNetType;

}

FCNetConnectionManage::~FCNetConnectionManage()
{
	
}

bool FCNetConnectionManage::Init(uint32 InIP, uint16 InPort)
{
	bAsynchronousListening = FCNetGlobalInfo::Get()->GetConfig().bAsynchronousListening;

	Connections.RemoteConnections.SetNum(NetType == ECNetType::Server ?  FCNetGlobalInfo::Get()->GetConfig().MaxConnections : 0);
	
	
	do
	{
		ISocketSubsystem* SocketSubsystem = FCNetConnectionManage::GetSocketSubsystem();

		if (!SocketSubsystem) break;

		LocalAddr = SocketSubsystem->CreateInternetAddr();

		Socket = SocketSubsystem->CreateSocket(NAME_DGram,TEXT("b站/伴霜叶"));			//   插件  创作时间    :   大三   第一学期   在读  ---->>>  2023年  11 月    


		if(!Socket) break;

#if DEBUG_INFO
		UE_LOG(LogCNetChannel, Error, TEXT(" CreateSocket  Success"));
#endif

		
		int32 SendSize = 0;
		int32 RecvSize = 0;

		
		Socket->SetReuseAddr(true);
		Socket->SetSendBufferSize(FCNetGlobalInfo::Get()->GetConfig().SendDataSize, SendSize);
		Socket->SetReceiveBufferSize(FCNetGlobalInfo::Get()->GetConfig().ReceiveDataSize, RecvSize);

		// SocketLock.WriteUnlock();

		if (NetType == ECNetType::Server)
		{
			LocalAddr->SetAnyAddress();
			LocalAddr->SetPort(InPort == 0 ? FCNetGlobalInfo::Get()->GetConfig().LocalPort : InPort);

			// SocketLock.WriteLock();
			if (!SocketSubsystem->BindNextPort(Socket, *LocalAddr, 1, 1))
			{
				// SocketLock.WriteUnlock();
				break;
			}
			 // SocketLock.WriteUnlock();
			
		}else if(NetType == ECNetType::Client)
		{
			if (InIP == 0)
			{
				bool bBindAddr = false;
				LocalAddr->SetIp(TEXT("127.0.0.1"),bBindAddr);
			}
			else
			{
				LocalAddr->SetIp(InIP);
			}
			LocalAddr->SetPort(InPort == 0 ? FCNetGlobalInfo::Get()->GetConfig().LocalPort : InPort);

#if DEBUG_INFO
			UE_LOG(LogCNetChannel, Error, TEXT(" Set Ip Port  Success"));
#endif

		}


		return true;
	}
	while (false);
	
	
	return  false;
}

void FCNetConnectionManage::Tick(const float DeltaTime)
{
	if (Connections.LocalConnection.IsValid())
	{
		if (Connections.LocalConnection->GetState() == ECNetConnectionState::Listening) Connections.LocalConnection->Tick(DeltaTime);
		else if (NetType == ECNetType::Client) Connections.LocalConnection->CheckLoginTimeOut(DeltaTime);
		
		for (const auto& Val : Connections.RemoteConnections) if (Val->GetState() == ECNetConnectionState::Listening) Val->Tick(DeltaTime);

	}


}




FORCEINLINE FSocket* FCNetConnectionManage::GetSocket() const
{
	return Socket;
}

void FCNetConnectionManage::DestorySocket()
{

	if (Socket)
	{
		delete Socket;
		Socket = nullptr;
	}

}

void FCNetConnectionManage::Close()
{


	if(Connections.LocalConnection.IsValid()) Connections.LocalConnection->Close();

	for (const auto& Val : Connections.RemoteConnections)
	{
		if (Val.IsValid()) Val->Close();

	}


	if(Socket) Socket->Close();

	if (TaskManage.IsValid())
	{
		TaskManage->Destroy();
	}




	
}

void FCNetConnectionManage::Listening()
{
	
}

void FCNetConnectionManage::Send(TArray<uint8>& InData, const TSharedPtr<FCNetConnection>& InConn)
{

	CNetEncryptionAndDecryption::Encryption(InData);
	
}

int32 FCNetConnectionManage::GetIdleConnectionNums() const
{

	if (NetType == ECNetType::Server)
	{

		int32 Num = 0;

		for (const auto& Val : Connections.RemoteConnections)
		{
			if (Val->GetState() == ECNetConnectionState::Uninitialized)
			{
				Num++;
			}

		}
		return Num;
	}




	return 0;
}



bool FCNetConnectionManage::IsCompletePackage(TArray<uint8>& InData, TArray<uint8>& OutData,
                                              const TSharedPtr<FCNetConnection>& InConnection, FGuid& OutGuid)
{

	return  false;
	
}



void FCNetConnectionManage::VerificationConnectionInfo(TArray<uint8>& InData,const TSharedPtr<FCNetConnection>& InConnection)
{

}



 ISocketSubsystem* FCNetConnectionManage::GetSocketSubsystem()
{
	return  ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
}

FCNetConnectionManage::FCache::FCache()
	: TotalSize(0)
{
}

void FCNetConnectionManage::FCache::Reset()
{
	TotalSize = 0;
	Cache.Empty();
}

FCNetConnectionManage::FConn::FConn(FCNetConnectionManage* InOwner) : Owner(InOwner)
{
}



FORCEINLINE TSharedPtr<FCNetConnection> FCNetConnectionManage::FConn::GetAnEmptyConnection() const
{

	
	for (const auto& Val : Owner->Connections.RemoteConnections)
	{
		if (Val->GetState() == ECNetConnectionState::Uninitialized)
		{
			return Val;
		}
	}
	
	return nullptr;

}

TSharedPtr<FCNetConnection> FCNetConnectionManage::FConn::operator[](const FGuid& InGuid) const
{

	if (LocalConnection->GetGuid() == InGuid) return LocalConnection;

	for (const auto& Val : Owner->Connections.RemoteConnections)
	{
		if (Val->GetGuid() == InGuid)
		{
			return Val;
		}
	}

	return nullptr;
}

TSharedPtr<FCNetConnection> FCNetConnectionManage::FConn::operator[](const TSharedPtr<FInternetAddr>& InAddr) const
{
	for (const auto& Val : Owner->Connections.RemoteConnections)
	{
		if ( *Val->GetRemoteAddr() == *InAddr )
		{
			return Val;
		}
	}
	
	return nullptr;
}




