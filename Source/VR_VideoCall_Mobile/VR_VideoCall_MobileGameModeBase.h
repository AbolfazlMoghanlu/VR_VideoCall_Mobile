// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
//#include ""

// #if PLATFORM_ANDROID
// #include "AndroidPermission/Classes/AndroidPermissionFunctionLibrary.h"
// #endif

#include "VR_VideoCall_MobileGameModeBase.generated.h"

// using namespace agora::rtc;
// using namespace agora;

/**
 * 
 */
UCLASS()
class VR_VIDEOCALL_MOBILE_API AVR_VideoCall_MobileGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	virtual void Tick(float DeltaTime) override;

private:
	virtual void BeginPlay() override;


	void InitAgoraWidget(FString APP_ID, FString TOKEN, FString CHANNEL_NAME);
	
	//void CheckPermission();

	//void InitAgoraEngine(FString APP_ID, FString TOKEN, FString CHANNEL_NAME);

protected:

// 	IRtcEngine* RtcEngineProxy;
// 
// 	FString AppId;
// 	FString Token;
// 	FString ChannelName;
// 
// 	TSharedPtr<FUserRtcEventHandler> UserRtcEventHandler;
};
