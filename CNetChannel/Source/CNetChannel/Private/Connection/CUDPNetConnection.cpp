#include "CUDPNetConnection.h"
#include "../Manage/Connection/CNetConnectionManage.h"
#include "Global/CNetGlobalInfo.h"
#include "Protocols/CNetBaseProtocols.h"


FCUDPNetConnection::FCUDPNetConnection(const ECNetType InNetType, const TWeakPtr<FCNetConnectionManage>& InOwner) : Super(InNetType,InOwner)
{
	


}

FCUDPNetConnection::~FCUDPNetConnection()
{


}

void FCUDPNetConnection::Init()
{
	Super::Init();

	
}

void FCUDPNetConnection::Send(TArray<uint8>& InData, bool bInForceSend)
{
	if(!Owner.IsValid() || InData.Num() < sizeof FCNetBaseHead) return;
	
	TArray<uint8> WaitSendData = InData;
	do
	{
		if (!FCNetGlobalInfo::Get()->GetConfig().bReliable) break;


		const int32 SendDataSize = FCNetGlobalInfo::Get()->GetConfig().SendDataSize;
		const FCNetBaseHead* BaseHead = reinterpret_cast<FCNetBaseHead*>(WaitSendData.GetData());
		
		FCNetPackageHead PackageHead;

		PackageHead.PackageSize = WaitSendData.Num();
		PackageHead.ConnID = BaseHead->ConnID;

		// Force Send												+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
		if (bInForceSend)
		{
			PackageHead.bForceSend = true;

			int32 PackageHeadSize = sizeof(FCNetPackageHead);
			TArray<uint8> ForceSendData;

			int32 Flag = ForceSendData.AddUninitialized(sizeof(FCNetPackageHead));
			FMemory::Memcpy(ForceSendData.GetData(), &PackageHead, sizeof(FCNetPackageHead));

			Flag = ForceSendData.AddUninitialized(InData.Num());
			FMemory::Memcpy(&ForceSendData[Flag], InData.GetData(), InData.Num());
			
			WaitSendData = ForceSendData;

			break;
			
		}

		// Normal Send												+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
		Batches.Emplace(PackageHead.PackageID, FBatch());
					
		FBatch& Batch = Batches[PackageHead.PackageID];

		PackageHead.Protocol = CNetP_PackageRequestSend;

		TArray<uint8> RequestData;
		RequestData.AddUninitialized(sizeof(FCNetPackageHead));
		FMemory::Memcpy(RequestData.GetData(), &PackageHead, sizeof(FCNetPackageHead));

		
		// Package to large												+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++									
		if (InData.Num() > SendDataSize)
		{
			const int32 BatchesSize = FMath::CeilToInt(static_cast<float>(InData.Num()) / static_cast<float>((SendDataSize)));
			const int32 LastCount = BatchesSize - 1;
			
			for (int32 i = 0; i < BatchesSize; i++)
			{
				PackageHead.PackageIndex = i;

				Batch.Sequence.Emplace(i, FBatch::FElement());
				FBatch::FElement& Tmp = Batch.Sequence[i];

				Tmp.Package.AddUninitialized(sizeof(FCNetPackageHead));
				FMemory::Memcpy(Tmp.Package.GetData(), &PackageHead, sizeof(FCNetPackageHead));
				if (i == LastCount)
				{
					const int32 Surplus = InData.Num() - SendDataSize * LastCount;
					const int32 Flag = Tmp.Package.AddUninitialized(Surplus);

					FMemory::Memcpy(&Tmp.Package[Flag], &InData[LastCount * SendDataSize], Surplus);
				}
				else
				{
					const int32 Flag = Tmp.Package.AddUninitialized(SendDataSize);
					FMemory::Memcpy(&Tmp.Package[Flag], &InData[i * SendDataSize], SendDataSize);
				}
			}
			
		}
		else // 	Normal Size											+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
		{
			
			Batch.Sequence.Emplace(0, FBatch::FElement());
			FBatch::FElement& Tmp = Batch.Sequence[0];

			Tmp.Package.AddUninitialized(sizeof(FCNetPackageHead));
			FMemory::Memcpy(Tmp.Package.GetData(), &PackageHead, sizeof(FCNetPackageHead));

			const int32 Flag = Tmp.Package.AddUninitialized(WaitSendData.Num());
			FMemory::Memcpy(&Tmp.Package[Flag], WaitSendData.GetData(), WaitSendData.Num());
		}


		WaitSendData = RequestData;
		
	}
	while (false);


	// 	Send data  to remote											+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	
	Owner.Pin()->Send(WaitSendData,this->AsShared());
}



void FCUDPNetConnection::Close()
{
	Super::Close();
}



void FCUDPNetConnection::Tick(const float DeltaTime)
{
	Super::Tick(DeltaTime);



}



bool FCUDPNetConnection::Receive(TArray<uint8>& InData)
{
	return Super::Receive(InData);
}



void FCUDPNetConnection::Analysis(TArray<uint8>& InData)
{
#if DEBUG_INFO
	UE_LOG(LogCNetChannel, Error, TEXT("Analysis+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"));
#endif
	const FCNetBaseHead& BaseHead = *reinterpret_cast<FCNetBaseHead*>(InData.GetData());

	
	//Update our Upper-layer interface
	auto UpdateController = [&]()
	{
		if(!Controller) return;
		
		if (BaseHead.ParamNum > 0)
		{
			DataQueue.Enqueue(InData);
		}
		
		Controller->ReceiveProtocol(BaseHead.ProtocolsNumber);
		
	};

	if (NetType == ECNetType::Server)
	{
		switch (BaseHead.ProtocolsNumber)
		{
			case CNetP_HeartBeat:
			{
				ResetHeartBeat();
				ResponseHeartBeat();
				break;
			}
			case CNetP_Close:
			{
				Close();
				break;
			}
			default:
			{
				UpdateController();
				break;
			}

		}
		
	}
	else
	{
		switch (BaseHead.ProtocolsNumber)
		{
			case CNetP_HeartBeatResponse:
			{
				ResetHeartBeat();
				break;
			}
			default:
			{
				UpdateController();
				break;
			}
		}

		
	}

	
}


