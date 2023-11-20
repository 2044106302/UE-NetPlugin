#pragma once
#include "CNetChannelTypes.h"

class CNETCHANNEL_API FCNetGlobalInfo
{
public:

	void Init(const FString &InPath = FPaths::ProjectDir() / TEXT("CNetConfig.ini"));
	
	
	static FCNetGlobalInfo* Get();

	static void Destroy();

	const FCNetConfigInfo& GetConfig() const { return ConfigInfo; };

	

	const TArray<uint8> &GetSecretKey() const;
private:
	FCNetGlobalInfo();
	
	static FCNetGlobalInfo* NetGlobalInfo;
	FCNetConfigInfo ConfigInfo;

	void InitSecretKey();
	TArray<uint8> SecretKey;
};
