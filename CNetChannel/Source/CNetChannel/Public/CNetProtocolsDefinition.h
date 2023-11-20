#pragma once

#include "CoreMinimal.h"

#include "IOStream/CNetIOStream.h"

#include "../Private/Manage/Net/CUDPNetManage.h"

#include "Connection/CNetConnection.h"
#include "../Private/Manage/Connection/CNetConnectionManage.h"


template<uint32 ProtocolsType>
class FCNetProtocols {};




#define DEFINITION_PROTOCOLS(ProtocolsName,ProtocolsNum,bForceSend) enum {CNetP_##ProtocolsName = ProtocolsNum};\
template<> class CNETCHANNEL_API FCNetProtocols<ProtocolsNum>\
{	\
public:	\
	template<typename ...ParamTypes>	\
	static void Send(FCNetConnection* InConn,ParamTypes &...Params)	\
	{		\
		TArray<uint8> Buffer;	\
		FCNetIOStream Stream = FCNetIOStream(Buffer);	\
		FCNetBaseHead BaseHead;	\
		BaseHead.ProtocolsNumber = ProtocolsNum;	\
		BaseHead.ParamNum = Stream.Info.GetBuildParams(Params...);	\
		BaseHead.ConnID = InConn->GetGuid();	\
		Stream << BaseHead;	\
		Stream.Info.BuildSendParams(Params...);	\
		InConn->Send(Buffer,bForceSend);\
		\
	}	\
	\
	template<typename  ...ParamTypes>	\
	static void Receive(FCNetConnection* InConn,ParamTypes &...Params)	\
	{	\
		TArray<uint8> Buffer;	\
		if (InConn->Receive(Buffer))	\
		{	\
			FCNetIOStream Stream = FCNetIOStream(Buffer);	\
			Stream.Seek(sizeof FCNetBaseHead);	\
			Stream.Info.BuildReceiveParams(Params...);	\
		}	\
	}	\
	\
};



#define DEFINITION_NET_PROTOCOLS(ProtocolsName,ProtocolsNum) DEFINITION_PROTOCOLS(ProtocolsName,ProtocolsNum,false)
#define DEFINITION_FORCE_NET_PROTOCOLS(ProtocolsName,ProtocolsNum) DEFINITION_PROTOCOLS(ProtocolsName,ProtocolsNum,true)





#if PLATFORM_IOS || PLATFORM_ANDROID


#define NET_PROTOCOLS_SEND(InProtocols,args...) FCNetProtocols<InProtocols>::Send(Conn,##args);
#define NET_PROTOCOLS_RECEIVE(InProtocols,args...) FCNetProtocols<InProtocols>::Receive(Conn,##args);


#define NET_PROTOCOLS_MULTICAST_SEND(InProtocols,args...) \
if (Conn &&  && Conn->GetOwner().IsValid())\
{\
	Conn->GetOwner().Pin()->MulticastToClient<InProtocols>(##args);\
}

#else


#define NET_PROTOCOLS_SEND(InProtocols,...) FCNetProtocols<InProtocols>::Send(Conn,__VA_ARGS__);
#define NET_PROTOCOLS_RECEIVE(InProtocols,...) FCNetProtocols<InProtocols>::Receive(Conn,__VA_ARGS__);


#define NET_PROTOCOLS_MULTICAST_SEND(InProtocols,...) \
if (Conn && Conn->GetOwner().IsValid())\
{\
	Conn->GetOwner().Pin()->MulticastToClient<InProtocols>(__VA_ARGS__);\
}

#endif




