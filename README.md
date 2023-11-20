# CNetChannel	

## 个人b站主页网址 ： https://b23.tv/wiHB3qk   



#### 在虚幻引擎（Unreal Engine）中,可以快速搭建分布式服务器和客户端的插件 || Plug-ins for distributed servers and clients can be quickly set up

#### 内容：

​	**1.通过应用层手段 实现了 可靠 的 UDP 通信 ： 	性能高，可靠！**

​	**2.使用了 线程池 和 内存预分配方案 ： 		      响应快，防止内存碎片**

​	**3.剥离 定义协议 和 收发消息 模块 ： 			 使用方便 ，很实用**



​	**该插件为本人  大三  第一学期研制(2023/11)，吸取并总结了其他大佬的经验而潜心研制，梦想就是能自己搭建一套完整的网络游戏，不论是服务器还是客户端，如果你有需要，希望这个插件能帮助到你！**



## 使用教程 (一个客户端 和 一个服务端 为例 ) ：







​	**-----将整个 CNetChannel 文件 添加到 虚幻  的 引擎插件 目录里面**





​	**-----在需要使用插件的地方在 build.cs 里面添加 该 模块**





​	**-----创建一个继承自UCNetController的Controller类作为我们的控制器**

​	



**客户端的控制器**

```c++
#pragma once
#include "Controller/CNetObject.h"


class UClientController : public UCNetController
{


protected:

	virtual void Init() override;

	virtual void Tick(float DeltaTime) override;

	virtual void ReceiveProtocol(uint32 InProtocol) override;

	virtual void Close() override;

	virtual void LinkFailure() override;		// 连接失败
	
	virtual void Upgrade() override;		// 需要更新版本

	virtual void LinkSucceed() override;		// 连接成功

	int32 Temp;							// 临时测试
};
```

```c++
#include "ClientController.h"
#include "CNetChannel/Private/Log/CNetChannelLog.h"
#include "Protocols/CNetBaseProtocols.h"




void UClientController::Init()
{
	Super::Init();

	Temp = 0;



	UE_LOG(LogCNetChannel, Warning, TEXT("Client  Init"));

}

void UClientController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);


	if (Temp++ == 250)
	{
		FString TestString = TEXT(" Hello  I am  Client   Test   Msg");

		NET_PROTOCOLS_SEND(CNetP_Test, TestString)
			Temp = 0;

	}


}

void UClientController::Close()
{
	Super::Close();

	UE_LOG(LogCNetChannel, Warning, TEXT("Client Close"));
}

void UClientController::ReceiveProtocol(uint32 InProtocol)
{
	Super::ReceiveProtocol(InProtocol);
	switch (InProtocol)
	{
	default:
		break;
		case CNetP_Test:
		{
			FString RecvMsg;

			NET_PROTOCOLS_RECEIVE(CNetP_Test, RecvMsg);


			UE_LOG(LogCNetChannel, Error, TEXT("Recv Client Msg = %s"), *RecvMsg);


			break;
		}
	}
	
}

void UClientController::LinkFailure()
{
	Super::LinkFailure();


}

void UClientController::Upgrade()
{
	Super::Upgrade();


}

void UClientController::LinkSucceed()
{
	Super::LinkSucceed();


}


```

**服务端控制器**

```c++
#pragma once
#include "CNetChannel/Public/Controller/CNetObject.h"



class UServerController : public UCNetController
{

protected:
	virtual void Init() override;

	virtual void Tick(float DeltaTime) override;

	virtual void Close() override;
	
	virtual void ReceiveProtocol(uint32 InProtocol) override;


};
```

```c++
#include "ServerController.h"

#include "CNetChannel/Private/Log/CNetChannelLog.h"

#include "Protocols/CNetBaseProtocols.h"


void UServerController::Init()
{
	Super::Init();


}

void UServerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void UServerController::Close()
{
	Super::Close();
}

void UServerController::ReceiveProtocol(uint32 InProtocol)
{
	Super::ReceiveProtocol(InProtocol);
	switch (InProtocol)
	{
	default:
		break;
	case CNetP_Test:
	{
		FString RecvMsg;

		NET_PROTOCOLS_RECEIVE(CNetP_Test, RecvMsg);


		UE_LOG(LogCNetChannel, Error, TEXT("Recv Client Msg = %s"), *RecvMsg);


		FString TestString = TEXT(" Hello  I am  Server   Test   Msg ");

		NET_PROTOCOLS_SEND(CNetP_Test, TestString)

		break;
	}
	}
}


```





**在这个示例中 我们让客户的 向 服务端发送一个 消息 FString TestString = TEXT(" Hello  I am  Client   Test   Msg "); ** 

**然后会传到服务端 ReceiveProtocol方法里面  然后我们接受 并且 返回 消息  打印 消息**





### 需要的主要步骤（Server端同理）

```c++
	FCNetGlobalInfo::Get()->Init();

	 FCNetManage* Client/*Server*/ = FCNetManage::CreateNetManage(ECNetType::Client/*Server*/);

    double LastTime = FPlatformTime::Seconds();
    if (!Client) return;
	do
	{	  
        // 初始化端口
		if (!Client->Init(8999))
		{
			UE_LOG(LogTemp, Warning, TEXT("Client Init Error"));
			break;
		}
		
        // 注册 我们的 控制器
		if ( !Client->RegisterController( UCNetController::Type<UClientController>() ) ) break;

		while (true) {

			// FPlatformProcess::Sleep(0.03);
			const double CurrentTime = FPlatformTime::Seconds();
			Client->Tick(CurrentTime - LastTime);
			LastTime = CurrentTime;
			
		}


	} while (false);

	Client->Close();
	delete Client;

```







### 我们如何 定义 我们自己的 协议呢 ？

​	**首先我们需要 再 创建一个新的引擎 插件  然后 在 插件里面 包含我们的  CNetChannel 模块  之后找个地方定义我们的协议 就像这样**

​	

```c++
#pragma once

#include "CNetProtocolsDefinition.h"
#include "Connection/CNetConnection.h"

// 	通过这个宏 自定义 协议 			协议名称 协议号
DEFINITION_NET_PROTOCOLS(Test,900018)
    
// 之后就可以 引入 这个 插件的 头文件 通过
   	// NET_PROTOCOLS_RECEIVE(CNetP_Test, RecvMsg);	接受协议信息
	//  NET_PROTOCOLS_SEND(CNetP_Test, TestString)  发送信息

```







### 最后 看看我们的配表：

​	**在此 我解释一下 一些 特殊的 其他的 按字面意思理解**

​	**bReliable 		为  是否 可靠   如果可靠 就会采用补包策略，相对 不可靠 性能较低**

​	**ThreadNums 		为  线程池中 线程的个数**

​	**OutLinkTime 		为  心跳超时连接的时间**

​	**HeartBeatInterval     为  心跳的频率 多久心跳一次**

​	**SecretKey             为  密钥 用于加密 传输信息 需两端 密钥相同**



```ini
[CNetConfig]
Version=0.0.0.0
LocalIP=127.0.0.1
LocalPort=9999
TargetIP=127.0.0.1
TargetPort=9999
bReliable=1
bAsynchronousListening=1
ReceiveDataSize=10240
SendDataSize=1024
MaxConnections=3
ThreadNums=1
OutLinkTime=20
HeartBeatInterval=10
RepackagingTime=3
SecretKey=www

```

