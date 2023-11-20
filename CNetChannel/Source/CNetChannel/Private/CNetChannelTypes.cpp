#include "CNetChannelTypes.h"

FCNetBaseHead::FCNetBaseHead() :
	ProtocolsNumber(0)
	,ParamNum(0)
{
}

FCNetPackageHead::FCNetPackageHead()
	:Protocol(0)
	,PackageIndex(0)
	,PackageID(FGuid::NewGuid())
	,PackageSize(0)
	,bAck(false)
	,bForceSend(false)
{
}

FCNetConfigInfo::FCNetConfigInfo()
	: Version(TEXT("0.0.0.1")),
		LocalIP(TEXT("127.0.0.1")),
		LocalPort(9999),
		TargetIP(TEXT("127.0.0.1")),
		TargetPort(9999),
		bReliable(true),
		bAsynchronousListening(true),
		ReceiveDataSize(10240),
		SendDataSize(1024),
		MaxConnections(3),
		ThreadNums(1),
		OutLinkTime(50.f),
		HeartBeatInterval(10.f),
		RepackagingTime(3.0f)
		
{
	
}
