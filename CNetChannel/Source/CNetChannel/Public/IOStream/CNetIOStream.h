#pragma once
#include "../Private/Log/CNetChannelLog.h"

class CNETCHANNEL_API FCNetIOStream
{
	
private:
	
	TArray<uint8>& Buffer;

private:
	uint8* OffsetPtr;
	
	void Write(const void* InData, const int64 InSize) const;

	template<class T>
	void Read(T &InValue)
	{
		// Make sure the  OffsetPtr pointer is valid
			Seek(0);
		
			InValue = *reinterpret_cast<T*>(OffsetPtr);
		
			Seek(sizeof(T));
	}


public:
	void Seek(int32 InFlag = 1);

public:

	// I Stream
	
	explicit FCNetIOStream(TArray<uint8>& InBuffer);
	
	template<class T>
	FCNetIOStream& operator<<(T &InValue)
	{
		T* Temp = const_cast<T*>(&InValue);

		Write(reinterpret_cast<void*>(Temp), sizeof(T));
		return *this;
	}

	template<class T>
	FCNetIOStream& operator<<(TArray<T>& InValue)
	{
		int32 Num = InValue.Num();
		*this << Num;

		if (Num > 0)
		{
			Write(InValue.GetData(), sizeof(T) * Num);
		}

		return *this;
	}


	FCNetIOStream& operator<<(TArray<FString>& InValue)
	{

		int32 Num = InValue.Num();
		*this << Num;

		for (int32 i = 0; i < Num; i++)
		{
			*this << InValue[i];
		}

		return *this;
	}

	FCNetIOStream& operator<<(FString& InValue);


public:
	// O Stream
	
	template<class T>
	FCNetIOStream& operator>>(T& InValue)
	{
		Read<T>(InValue);
		return *this;
	}

	template<class T>
	FCNetIOStream& operator>>(TArray<T>& InValue)
	{
		FCNetIOStream& Stream = *this;
		int32 Num = 0;
		Stream >> Num; 

		if (Num >= 1753880896)
		{
			UE_LOG(LogCNetChannel,Warning,TEXT("The Num >= 1753880896  !!!!"));
		}
		else if (Num > 0)
		{
			const int32 Size = sizeof(T) * Num;
			InValue.AddUninitialized(Num);

			FMemory::Memcpy(InValue.GetData(), OffsetPtr, Size);

			Seek(Size);
		}

		return *this;
	}


	FCNetIOStream& operator>>(TArray<FString>& InValue)
	{
		FCNetIOStream& Stream = *this;
		int32 Num = 0;
		Stream >> Num;

		if (Num >= 1753880896)
		{
			UE_LOG(LogCNetChannel,Warning,TEXT("The Num >= 1753880896  !!!!"));
		}
		else if (Num > 0)
		{
			for (int32 i = 0; i < Num; i++)
			{
				InValue.Emplace(FString());
				Stream >> InValue.Last();
			}
		}

		return *this;
	}

	
	FCNetIOStream& operator>>(FString& InValue) ;

public:
	
	class FInfo
	{
	private:
		FCNetIOStream& Owner;
	public:
		explicit  FInfo(FCNetIOStream& InOwner);
		
		template<typename ...ParamTypes>
		static  int32 GetBuildParams(ParamTypes &...Param)
		{
			return sizeof...(Param);
		}

		template<typename ...ParamTypes>
		static void BuildSendParams(ParamTypes &...Param){}
	
		//Input parameters to the stream recursively
		template<class T,typename ...ParamTypes>
		void BuildSendParams(T& Front, ParamTypes &...Param)
		{
			Owner << Front;
			BuildSendParams(Param...);
		}

		template<typename ...ParamTypes>
		static void BuildReceiveParams(ParamTypes &...Param) {}

		template<class T, typename ...ParamTypes>
		void BuildReceiveParams( T& Front, ParamTypes &...Param)
		{
			Owner >> Front;
			BuildReceiveParams(Param...);
		}
	}Info;
	
};
