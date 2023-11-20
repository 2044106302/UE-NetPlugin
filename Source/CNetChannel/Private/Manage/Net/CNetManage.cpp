#include "Manage/CNetManage.h"

#include "CUDPNetManage.h"
#include "SocketSubsystem.h"
#include "Log/CNetChannelLog.h"
#include "Manage/Connection/CNetConnectionManage.h"

FCNetManage::FCNetManage(const ECNetType InNetType) : NetType(InNetType),bInit(false)
{
}

FCNetManage::~FCNetManage()
{
	
}

FCNetManage* FCNetManage::CreateNetManage(ECNetType InNetType)
{
	
	return new FCUDPNetManage(InNetType);
	
}


bool FCNetManage::Init(int32 InPort)
{
	return false;
}

bool FCNetManage::Init(const FString& InIP, uint16 InPort)
{
	return false;
}

bool FCNetManage::Init(uint32 InIp, uint16 InPort)
{

	
	return  false;
}


void FCNetManage::Tick(float DeltaTime)
{
	if (!bInit) return;

	ConnectionManage->Tick(DeltaTime);
	
}

void FCNetManage::Close()
{

	ConnectionManage->Close();

}
