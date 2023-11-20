#pragma once

#include "Global/CNetGlobalInfo.h"
#include "../Log/CNetChannelLog.h"
#include "CoreMinimal.h"



namespace CNetEncryptionAndDecryption{

	 void Encryption(TArray<uint8>& InData)
	{
		const FString& SecretKey = FCNetGlobalInfo::Get()->GetConfig().SecretKey;
		if (!SecretKey.IsEmpty())
		{
			const TArray<uint8>& SecretKeyArray = FCNetGlobalInfo::Get()->GetSecretKey();
			for (auto& Val : InData)
			{
				for (auto& TmpSecretKey : SecretKeyArray)
				{
					Val ^= TmpSecretKey;
				}
			}
		}
	}

	 void Decryption(uint8* InData, int32 InSize)
	{

		const FString& SecretKey = FCNetGlobalInfo::Get()->GetConfig().SecretKey;
		if (!SecretKey.IsEmpty())
		{
			const TArray<uint8>& SecretKeyArray = FCNetGlobalInfo::Get()->GetSecretKey();
			for (int32 i = 0; i < InSize; i++)
			{
				for (int32 j = (SecretKeyArray.Num() - 1); j > -1; j--)
				{
					InData[i] ^= SecretKeyArray[j];
				}
			}
		}
	}

	FORCEINLINE void Decryption(TArray<uint8>& InData)
	{
		Decryption(InData.GetData(), InData.Num());
	}

	


}