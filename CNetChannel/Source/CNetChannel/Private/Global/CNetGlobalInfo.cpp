#include "Global/CNetGlobalInfo.h"
#include "../Log/CNetChannelLog.h"
#include "IOStream/CNetIOStream.h"

#if PLATFORM_MAC
#include "Mac/MacPlatformProcess.h"
#define INSERT_TEXT(x) *FString(x)
#else
#define INSERT_TEXT(x) TEXT(x)
#endif

enum class EParamType
{
	Int,
	Float,
	String,
};


const TArray<uint8>& FCNetGlobalInfo::GetSecretKey() const
{
	return SecretKey;
}

FCNetGlobalInfo::FCNetGlobalInfo()
{
}

void FCNetGlobalInfo::InitSecretKey()
{
	if (!ConfigInfo.SecretKey.IsEmpty())
	{
		FCNetIOStream SecretIOStream(SecretKey);
		SecretIOStream << ConfigInfo.SecretKey;
	}
}


void AutoInsertString(TMap<FString,FString> &InConfigInfo,void* InData,const TCHAR* InFieldName, EParamType InType)
{
	if (FString* InValue = InConfigInfo.Find(InFieldName))
	{
		switch (InType)
		{
		case EParamType::Int:
			{
				*reinterpret_cast<int32*>(InData) = FCString::Atoi(**InValue);
				break;
			}
		case EParamType::Float:
			{
				*reinterpret_cast<float*>(InData) = FCString::Atof(**InValue);
				break;
			}
		case EParamType::String:
			{
				*reinterpret_cast<FString*>(InData) = *InValue;
				break;
			}
		}
	}
	else
	{
		UE_LOG(LogCNetChannel, Error, TEXT("The following fields [%s] could not be found."), InFieldName);
	}
}



FCNetGlobalInfo* FCNetGlobalInfo::NetGlobalInfo = nullptr;

void FCNetGlobalInfo::Init(const FString& InPath)
{
#if PLATFORM_MAC
	
	#if !WITH_EDITOR
	const FString& CNetConfigPath = FPaths::ProjectDir() / TEXT("CNetConfig.ini");
	FString FullPath = FPaths::ConvertRelativePathToFull(CNetConfigPath);
	FString APPName = FPlatformProcess::ExecutableName();
	FString R, L;
	FullPath.Split(*(TEXT("/") + APPName + TEXT(".app/")), &L, &R);
	FullPath = L / TEXT("CNetConfig.ini");
	#endif 
	const FString FullPath = FPaths::ConvertRelativePathToFull(InPath);
#else
	const FString FullPath = FPaths::ConvertRelativePathToFull(InPath);
#endif
	
	TArray<FString> Content;
	FFileHelper::LoadFileToStringArray(Content, *FullPath);
	if (Content.Num())
	{
		auto Lambda = [&](TMap<FString, FString>& OutContent)
		{
			for (auto &Val : Content)
			{
				if (Val.Contains("[") && Val.Contains("]"))//Analytic head
				{
					Val.RemoveFromEnd("]");
					Val.RemoveFromStart("[");
					OutContent.Add("ConfigHead", Val);
				}
				else //Analysis of the body
				{
					FString R, L;
					Val.Split(TEXT("="),&L,&R);
					
					OutContent.Add(L, R);
				}
			}
		};

		TMap<FString, FString> TempConfigInfo;
		Lambda(TempConfigInfo);

		
		AutoInsertString(TempConfigInfo,&ConfigInfo.Version, INSERT_TEXT("Version"), EParamType::String);
		
		AutoInsertString(TempConfigInfo,&ConfigInfo.LocalIP, INSERT_TEXT("LocalIP"), EParamType::String);
		AutoInsertString(TempConfigInfo,&ConfigInfo.LocalPort, INSERT_TEXT("LocalPort"), EParamType::Int);
		AutoInsertString(TempConfigInfo,&ConfigInfo.TargetIP, INSERT_TEXT("TargetIP"), EParamType::String);
		AutoInsertString(TempConfigInfo,&ConfigInfo.TargetPort, INSERT_TEXT("TargetPort"), EParamType::Int);
		
		AutoInsertString(TempConfigInfo,&ConfigInfo.bReliable, INSERT_TEXT("bReliable"), EParamType::Int);
		AutoInsertString(TempConfigInfo,&ConfigInfo.bAsynchronousListening, INSERT_TEXT("bAsynchronousListening"), EParamType::Int);
		AutoInsertString(TempConfigInfo,&ConfigInfo.ReceiveDataSize, INSERT_TEXT("ReceiveDataSize"), EParamType::Int);
		AutoInsertString(TempConfigInfo,&ConfigInfo.SendDataSize, INSERT_TEXT("SendDataSize"), EParamType::Int);
		
		AutoInsertString(TempConfigInfo,&ConfigInfo.MaxConnections, INSERT_TEXT("MaxConnections"), EParamType::Int);
		AutoInsertString(TempConfigInfo,&ConfigInfo.ThreadNums, INSERT_TEXT("ThreadNums"), EParamType::Int);
		
		AutoInsertString(TempConfigInfo,&ConfigInfo.OutLinkTime, INSERT_TEXT("OutLinkTime"), EParamType::Float);
		AutoInsertString(TempConfigInfo,&ConfigInfo.HeartBeatInterval, INSERT_TEXT("HeartBeatInterval"), EParamType::Float);
		AutoInsertString(TempConfigInfo,&ConfigInfo.HeartBeatInterval, INSERT_TEXT("RepackagingTime"), EParamType::Float);
		
		AutoInsertString(TempConfigInfo,&ConfigInfo.SecretKey, INSERT_TEXT("SecretKey"), EParamType::String);

		

		
	}
	else
	{
		Content.Add(TEXT("[CNetConfig]"));
		Content.Add(FString::Printf(TEXT("Version=%s"),*ConfigInfo.Version));
		Content.Add(FString::Printf(TEXT("LocalIP=%s"), *ConfigInfo.LocalIP));
		Content.Add(FString::Printf(TEXT("LocalPort=%i"),ConfigInfo.LocalPort));
		Content.Add(FString::Printf(TEXT("TargetIP=%s"), *ConfigInfo.TargetIP));
		Content.Add(FString::Printf(TEXT("TargetPort=%i"), ConfigInfo.TargetPort));
		Content.Add(FString::Printf(TEXT("bReliable=%i"), ConfigInfo.bReliable));
		Content.Add(FString::Printf(TEXT("bAsynchronousListening=%i"), ConfigInfo.bAsynchronousListening));
		Content.Add(FString::Printf(TEXT("ReceiveDataSize=%i"), ConfigInfo.ReceiveDataSize));
		Content.Add(FString::Printf(TEXT("SendDataSize=%i"), ConfigInfo.SendDataSize));
		Content.Add(FString::Printf(TEXT("MaxConnections=%i"), ConfigInfo.MaxConnections));
		Content.Add(FString::Printf(TEXT("ThreadNums=%i"), ConfigInfo.ThreadNums));
		
		Content.Add(FString::Printf(TEXT("OutLinkTime=%f"), ConfigInfo.OutLinkTime));
		Content.Add(FString::Printf(TEXT("HeartBeatInterval=%f"), ConfigInfo.HeartBeatInterval));
		Content.Add(FString::Printf(TEXT("RepackagingTime=%f"), ConfigInfo.RepackagingTime));
		
		Content.Add(FString::Printf(TEXT("SecretKey=%s"), *ConfigInfo.SecretKey));
		FFileHelper::SaveStringArrayToFile(Content, *FullPath);
	}

	InitSecretKey();
}

FCNetGlobalInfo* FCNetGlobalInfo::Get()
{
	if (!NetGlobalInfo)
	{
		NetGlobalInfo = new FCNetGlobalInfo;
	}

	return NetGlobalInfo;
	
}

void FCNetGlobalInfo::Destroy()
{
	if (NetGlobalInfo)
	{
		delete NetGlobalInfo;
	}
}


