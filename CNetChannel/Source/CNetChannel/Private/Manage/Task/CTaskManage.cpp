#include "CTaskManage.h"

#include "Log/CNetChannelLog.h"
#include "../Connection/CNetConnectionManage.h"
#include "Sockets.h"





FCTaskManage::FCTaskManage(FCNetConnectionManage* InOwner, const int32 InThreadNums) : Owner(InOwner)
{
	ThreadPool = FQueuedThreadPool::Allocate();


	if (ThreadPool)
	{
		ThreadPool->Create(InThreadNums, 0, TPri_Normal);
	}
}

FCTaskManage::~FCTaskManage()
{
	if (ThreadPool)
	{
		delete ThreadPool;

		ThreadPool = nullptr;
	}
}

bool FCTaskManage::AddQueuedWork(IQueuedWork* InQueueWork) const
{
	if (ThreadPool)
	{
		ThreadPool->AddQueuedWork(InQueueWork);
		return true;
	}

	return false;
}

void FCTaskManage::Destroy() 
{
	


	if (ThreadPool)
	{

		Owner->bStop = true;
		
		ThreadPool->Destroy();

		delete ThreadPool;

		ThreadPool = nullptr;
		
	}
	
}

int32 FCTaskManage::GetNumThreads() const
{
	return ThreadPool ? ThreadPool->GetNumThreads() : 0;
}

void FCTaskManage::RetractQueuedWork(IQueuedWork* InQueueWork) const
{
	if (ThreadPool)
	{
		ThreadPool->RetractQueuedWork(InQueueWork);
	}
}
