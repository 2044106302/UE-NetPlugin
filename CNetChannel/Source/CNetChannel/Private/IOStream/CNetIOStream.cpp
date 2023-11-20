#include "IOStream//CNetIOStream.h"

void FCNetIOStream::Write(const void* InData, const int64 InSize) const
{
	const int32 Flag = Buffer.AddUninitialized(InSize);
	FMemory::Memcpy(&Buffer[Flag], InData, InSize);
}

FCNetIOStream::FCNetIOStream(TArray<uint8>& InBuffer) : Buffer(InBuffer),OffsetPtr(InBuffer.GetData()) , Info(*this)
{
	
}

FCNetIOStream& FCNetIOStream::operator<<(FString& InValue)
{

	*this << InValue.GetCharArray();

	return *this;
	
	
}

FCNetIOStream& FCNetIOStream::operator>>(FString& InValue)
{

	*this >> InValue.GetCharArray();
	return *this;
}



void FCNetIOStream::Seek(int32 InFlag)
{
	if (!OffsetPtr) OffsetPtr = Buffer.GetData();
	
	OffsetPtr+= InFlag;
	
}


FCNetIOStream::FInfo::FInfo(FCNetIOStream& InOwner) : Owner(InOwner)
{

	
}