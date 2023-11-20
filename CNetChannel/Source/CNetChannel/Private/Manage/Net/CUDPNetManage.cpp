#include "CUDPNetManage.h"


#include "SocketSubsystem.h"
#include "Manage/Connection/CNetConnectionManage.h"
#include "Manage/Connection/CUDPNetConnectionManage.h"
#include "../../Log/CNetChannelLog.h"

FCUDPNetManage::FCUDPNetManage(const ECNetType InNetType) : Super(InNetType)
{
}

FCUDPNetManage::~FCUDPNetManage()
{
	Close();
}

bool FCUDPNetManage::Init(int32 InPort)
{
	return FCUDPNetManage::Init(TEXT(""),InPort);
}

bool FCUDPNetManage::Init(const FString& InIP, uint16 InPort)
{
	if (ISocketSubsystem* SocketSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM))
	{

		const TSharedPtr<FInternetAddr>  Addr = SocketSubsystem->CreateInternetAddr();

		bool bBindAddr = false;
		Addr->SetIp(*InIP, bBindAddr);
		uint32 UintIP;
		Addr->GetIp(UintIP);

		return FCUDPNetManage::Init(UintIP, InPort);
	}
	
	return false;
}

bool FCUDPNetManage::Init(uint32 InIp, uint16 InPort)
{
		
	ConnectionManage = MakeShareable(new FCUDPNetConnectionManage(NetType));

	if(!ConnectionManage.IsValid())
	{
#if DEBUG_INFO
		UE_LOG(LogCNetChannel, Error, TEXT("ConnectionManage.Is Not Valid"));
#endif // DEBUG_INFO


		return  false;
	}

	if (!ConnectionManage->Init(InIp,InPort))
	{
#if DEBUG_INFO
		UE_LOG(LogCNetChannel, Error, TEXT("ConnectionManage.INit "));
#endif // DEBUG_INFO


		return false;
	}


	bInit = true;
#if DEBUG_INFO
	UE_LOG(LogCNetChannel, Error, TEXT("FCUDPNetManage Init True"));
#endif // DEBUG_INFO


	
	return true;

}

void FCUDPNetManage::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bInit) return;
	
	
}

void FCUDPNetManage::Close()
{
	Super::Close();

	
}


