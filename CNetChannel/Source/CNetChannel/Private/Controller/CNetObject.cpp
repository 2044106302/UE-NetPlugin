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

	if(CNetReceiveDelegate.IsBound()) CNetReceiveDelegate.Broadcast(InProtocol);
	
}

void UCNetController::LinkOutTime() {
	
}


