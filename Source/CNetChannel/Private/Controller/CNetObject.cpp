#include "Controller/CNetObject.h"
#include "Protocols/CNetBaseProtocols.h"


UCNetController::UCNetController() : Conn(nullptr)
{
}

void UCNetController::Init()
{
}

void UCNetController::Tick(float DeltaTime)
{

}

void UCNetController::Close()
{

}

void UCNetController::ReceiveProtocol(uint32 InProtocol)
{
	switch (InProtocol)
	{
	case CNetP_IdleNumRequest:
	{

		int32 Num = Conn->GetOwner().Pin()->GetIdleConnectionNums();
		NET_PROTOCOLS_SEND(CNetP_IdleNumResponse, Num);

		break;
	}
	case CNetP_IdleNumResponse:
	{
		int32 Num = 0;

		NET_PROTOCOLS_RECEIVE(CNetP_IdleNumResponse, Num)

		RemoteIdleNum(Num);
	}
	default:
		break;
	}

}

void UCNetController::RequestObtainTheRemoteIdleConnectionNum()
{
	NET_PROTOCOLS_SEND(CNetP_IdleNumRequest)
}

void UCNetController::RemoteIdleNum(int32 InNum)
{

}

void UCNetController::LinkFailure()
{
}

void UCNetController::Upgrade()
{
}

void UCNetController::LinkSucceed()
{

}

