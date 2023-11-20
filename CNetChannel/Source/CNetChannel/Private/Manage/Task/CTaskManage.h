#pragma once

class FCNetConnectionManage;


class FCTaskManage
{
public:

	FCTaskManage(FCNetConnectionManage* InOwner,const int32 InThreadNums);

	~FCTaskManage();

	bool AddQueuedWork(IQueuedWork* InQueueWork) const;

	void Destroy();

	int32 GetNumThreads() const;

	void RetractQueuedWork(IQueuedWork* InQueueWork) const;



private:
	FQueuedThreadPool* ThreadPool;

	FCNetConnectionManage* Owner;

	

	
};



