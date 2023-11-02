// Copyright Epic Games, Inc. All Rights Reserved.


#include "VR_VideoCall_MobileGameModeBase.h"

void AVR_VideoCall_MobileGameModeBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}


void AVR_VideoCall_MobileGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Warning, TEXT("War"));
}


void AVR_VideoCall_MobileGameModeBase::InitAgoraWidget(FString APP_ID, FString TOKEN, FString CHANNEL_NAME)
{
	//CheckPermission();

	//InitAgoraEngine(APP_ID, TOKEN, CHANNEL_NAME);
}
/*
void AVR_VideoCall_MobileGameModeBase::CheckPermission() 
{
#if PLATFORM_ANDROID
	FString TargetPlatformName = UGameplayStatics::GetPlatformName();
	if (TargetPlatformName == "Android")
	{
		TArray<FString> AndroidPermission;
#if !AGORA_UESDK_AUDIO_ONLY || (!(PLATFORM_ANDROID || PLATFORM_IOS))
		AndroidPermission.Add(FString("android.permission.CAMERA"));
#endif
		AndroidPermission.Add(FString("android.permission.RECORD_AUDIO"));
		AndroidPermission.Add(FString("android.permission.READ_PHONE_STATE"));
		AndroidPermission.Add(FString("android.permission.WRITE_EXTERNAL_STORAGE"));
		UAndroidPermissionFunctionLibrary::AcquirePermissions(AndroidPermission);
	}
#endif
}

void AVR_VideoCall_MobileGameModeBase::InitAgoraEngine(FString APP_ID, FString TOKEN, FString CHANNEL_NAME) 
{

	AppId = APP_ID;
	Token = TOKEN;
	ChannelName = CHANNEL_NAME;

	UserRtcEventHandler = MakeShared<FUserRtcEventHandler>(this);
	RtcEngineContext RtcEngineContext;
	std::string StdStrAppId = TCHAR_TO_UTF8(*AppId);
	RtcEngineContext.appId = StdStrAppId.c_str();
	RtcEngineContext.eventHandler = UserRtcEventHandler.Get();
	RtcEngineContext.channelProfile = CHANNEL_PROFILE_TYPE::CHANNEL_PROFILE_LIVE_BROADCASTING;

	RtcEngineProxy = agora::rtc::ue::createAgoraRtcEngine();
	int SDKBuild = 0;
	FString SDKInfo = FString::Printf(TEXT("SDK Version: %s Build: %d"), UTF8_TO_TCHAR(RtcEngineProxy->getVersion(&SDKBuild)), SDKBuild);
	UBFL_Logger::Print(FString::Printf(TEXT("SDK Info:  %s"), *SDKInfo), LogMsgViewPtr);

	int ret = RtcEngineProxy->initialize(RtcEngineContext);
	UBFL_Logger::Print(FString::Printf(TEXT("%s initialize ret %d"), *FString(FUNCTION_MACRO), ret), LogMsgViewPtr);
}
*/