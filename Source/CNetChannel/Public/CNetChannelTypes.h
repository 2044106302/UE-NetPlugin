#pragma once


enum class ECNetType : uint8
{
	Client,
	Server
};

enum class ECNetConnectionState : uint8
{
	LocalConnection,
	Uninitialized,
	Verify,
	WaitJoin,
	Listening,
	
	
};





struct CNETCHANNEL_API FCNetBaseHead
{
	FCNetBaseHead();

	uint32 ProtocolsNumber;
	FGuid ConnID;
	uint8 ParamNum;
};


struct CNETCHANNEL_API FCNetPackageHead
{
	FCNetPackageHead();

	uint32 Protocol;
	uint32 PackageIndex;
	FGuid PackageID;
	FGuid ConnID;
	uint32 PackageSize;
	bool bAck;
	bool bForceSend;
};




struct FCNetConfigInfo
{

	FCNetConfigInfo();
	
	FString Version;
	
	FString LocalIP;
	
	int32 LocalPort;

	FString TargetIP;
	
	int32 TargetPort;

	bool bReliable;

	bool bAsynchronousListening;
	
	int32 ReceiveDataSize;
	
	int32 SendDataSize;
	
	int32 MaxConnections;

	int32 ThreadNums;
	 
	FString SecretKey;

	float OutLinkTime;

	float HeartBeatInterval;

	float RepackagingTime;
};



