// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomAgoraVideoWidget.h"

void UCustomAgoraVideoWidget::InitAgoraWidget(FString APP_ID, FString TOKEN, FString CHANNEL_NAME)
{
	CheckPermission();

	InitAgoraEngine(APP_ID, TOKEN, CHANNEL_NAME);
}

void UCustomAgoraVideoWidget::CheckPermission() {
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

void UCustomAgoraVideoWidget::InitAgoraEngine(FString APP_ID, FString TOKEN, FString CHANNEL_NAME) {

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
	UBFL_Logger::Print(FString::Printf(TEXT("SDK Info:  %s"), *SDKInfo), nullptr);

	int ret = RtcEngineProxy->initialize(RtcEngineContext);
	UBFL_Logger::Print(FString::Printf(TEXT("%s initialize ret %d"), *FString(FUNCTION_MACRO), ret), nullptr);
}


void UCustomAgoraVideoWidget::UnInitAgoraEngine()
{
	if (RtcEngineProxy)
	{
		RtcEngineProxy->leaveChannel();
		RtcEngineProxy->unregisterEventHandler(UserRtcEventHandler.Get());
		RtcEngineProxy->release();
		RtcEngineProxy = nullptr;
		UBFL_Logger::Print(FString::Printf(TEXT("%s release agora engine"), *FString(FUNCTION_MACRO)), nullptr);
	}
}

void UCustomAgoraVideoWidget::OnBtnJoinChannelClicked()
{
	if (RtcEngineProxy)
	{
		RtcEngineProxy->enableAudio();
		RtcEngineProxy->enableVideo();
		int ret = RtcEngineProxy->joinChannel(TCHAR_TO_UTF8(*Token), TCHAR_TO_UTF8(*ChannelName), "", 0);
		UBFL_Logger::Print(FString::Printf(TEXT("%s ret %d"), *FString(FUNCTION_MACRO), ret), nullptr);
		RtcEngineProxy->setClientRole(agora::rtc::CLIENT_ROLE_TYPE::CLIENT_ROLE_BROADCASTER);
	}
}

void UCustomAgoraVideoWidget::OnBtnLeaveChannelClicked()
{
	if (RtcEngineProxy)
	{
		int ret = RtcEngineProxy->leaveChannel();
		UBFL_Logger::Print(FString::Printf(TEXT("%s ret %d"), *FString(FUNCTION_MACRO), ret), nullptr);
	}
}


void UCustomAgoraVideoWidget::NativeDestruct() {

	Super::NativeDestruct();

	UnInitAgoraEngine();
}


#pragma region UI Utility

int UCustomAgoraVideoWidget::MakeVideoView(uint32 uid, agora::rtc::VIDEO_SOURCE_TYPE sourceType /*= VIDEO_SOURCE_CAMERA_PRIMARY*/, FString channelId /*= ""*/)
{
	int ret = -ERROR_NULLPTR;

	if (RtcEngineProxy == nullptr)
		return ret;

	agora::rtc::VideoCanvas videoCanvas;
	videoCanvas.uid = uid;
	videoCanvas.sourceType = sourceType;

	if (uid == 0 && bMobileUser)
	{
		FVideoViewIdentity VideoViewIdentity(uid, sourceType, "");

		videoCanvas.view = CallView;
		ret = RtcEngineProxy->setupLocalVideo(videoCanvas);
	}

	else if(uid != 0 && !bMobileUser)
	{
		FVideoViewIdentity VideoViewIdentity(uid, sourceType, channelId);
		videoCanvas.view = CallView;
		
		if(channelId == ""){
			ret = RtcEngineProxy->setupRemoteVideo(videoCanvas);
		}
		else{
			agora::rtc::RtcConnection connection;
			std::string StdStrChannelId = TCHAR_TO_UTF8(*channelId);
			connection.channelId = StdStrChannelId.c_str();
			ret = ((agora::rtc::IRtcEngineEx*)RtcEngineProxy)->setupRemoteVideoEx(videoCanvas, connection);
		}
	}
	
	return ret;
}

int UCustomAgoraVideoWidget::ReleaseVideoView(uint32 uid, agora::rtc::VIDEO_SOURCE_TYPE sourceType /*= VIDEO_SOURCE_CAMERA_PRIMARY*/, FString channelId /*= ""*/)
{
	int ret = -ERROR_NULLPTR;

	if(RtcEngineProxy == nullptr)
		return ret;

	agora::rtc::VideoCanvas videoCanvas;
	videoCanvas.view = nullptr;
	videoCanvas.uid = uid;
	videoCanvas.sourceType = sourceType;


	if (uid == 0 && bMobileUser)
	{
		FVideoViewIdentity VideoViewIdentity(uid, sourceType, "");
		ret = RtcEngineProxy->setupLocalVideo(videoCanvas);
	}

	else if (uid != 0 && !bMobileUser)
	{
		FVideoViewIdentity VideoViewIdentity(uid, sourceType, channelId);
		if (channelId == "") {
			ret = RtcEngineProxy->setupRemoteVideo(videoCanvas);
		}
		else {
			agora::rtc::RtcConnection connection;
			std::string StdStrChannelId = TCHAR_TO_UTF8(*channelId);
			connection.channelId = StdStrChannelId.c_str();
			ret = ((agora::rtc::IRtcEngineEx*)RtcEngineProxy)->setupRemoteVideoEx(videoCanvas, connection);
		}
	}

	return ret;
}

#pragma endregion



#pragma region AgoraCallback - IRtcEngineEventHandler

void UCustomAgoraVideoWidget::FUserRtcEventHandler::onJoinChannelSuccess(const char* channel, agora::rtc::uid_t uid, int elapsed) {
	
	if (!IsWidgetValid())
		return;

	AsyncTask(ENamedThreads::GameThread, [=]()
	{
			if (!IsWidgetValid())
			{
				UBFL_Logger::PrintError(FString::Printf(TEXT("%s bIsDestruct "), *FString(FUNCTION_MACRO))); \
				return;
			}
			UBFL_Logger::Print(FString::Printf(TEXT("%s "), *FString(FUNCTION_MACRO)), nullptr);
			WidgetPtr->MakeVideoView(0, agora::rtc::VIDEO_SOURCE_TYPE::VIDEO_SOURCE_CAMERA);
	});
}


void UCustomAgoraVideoWidget::FUserRtcEventHandler::onUserJoined(agora::rtc::uid_t uid, int elapsed) 
{
	if (!IsWidgetValid())
		return;

	AsyncTask(ENamedThreads::GameThread, [=]()
	{
			if (!IsWidgetValid())
			{
				UBFL_Logger::PrintError(FString::Printf(TEXT("%s bIsDestruct "), *FString(FUNCTION_MACRO)));
				return;
			}
			UBFL_Logger::Print(FString::Printf(TEXT("%s "), *FString(FUNCTION_MACRO)), nullptr);
			WidgetPtr->MakeVideoView(uid, agora::rtc::VIDEO_SOURCE_TYPE::VIDEO_SOURCE_REMOTE);

			if (uid != 0)
			{
				WidgetPtr->OnRemoteJoined();
			}
	});
}

void UCustomAgoraVideoWidget::FUserRtcEventHandler::onUserOffline(agora::rtc::uid_t uid, agora::rtc::USER_OFFLINE_REASON_TYPE reason) 
{
	if (!IsWidgetValid())
		return;
	
	AsyncTask(ENamedThreads::GameThread, [=]()
	{
			if (!IsWidgetValid())
			{
				UBFL_Logger::PrintError(FString::Printf(TEXT("%s bIsDestruct "), *FString(FUNCTION_MACRO)));
				return;
			}
			UBFL_Logger::Print(FString::Printf(TEXT("%s "), *FString(FUNCTION_MACRO)), nullptr);
			WidgetPtr->ReleaseVideoView(uid, VIDEO_SOURCE_REMOTE);

			if (uid != 0)
			{
				WidgetPtr->OnRemoteLeave();
			}
	});
}

void UCustomAgoraVideoWidget::FUserRtcEventHandler::onVideoSizeChanged(VIDEO_SOURCE_TYPE sourceType, uid_t uid, int width, int height, int rotation)
{
	if (!IsWidgetValid())
		return;

	AsyncTask(ENamedThreads::GameThread, [=]()
		{
			if (!IsWidgetValid())
			{
				UBFL_Logger::PrintError(FString::Printf(TEXT("%s bIsDestruct "), *FString(FUNCTION_MACRO)));
				return;
			}
			UBFL_Logger::Print(FString::Printf(TEXT("%s uid=%d width=%d height=%d "), *FString(FUNCTION_MACRO), uid, width, height), nullptr);
			
			FVideoViewIdentity VideoViewIdentity(uid, sourceType, "");		
		});
}

void UCustomAgoraVideoWidget::FUserRtcEventHandler::onLocalVideoStateChanged(VIDEO_SOURCE_TYPE source, LOCAL_VIDEO_STREAM_STATE state, LOCAL_VIDEO_STREAM_ERROR error)
{
	if (!IsWidgetValid())
		return;

	AsyncTask(ENamedThreads::GameThread, [=]()
		{
			UE_LOG(LogTemp, Warning, TEXT("Local video changed _ %i _ %i"), state, error);

			if (state == 0)
			{
				WidgetPtr->LocalVideoStoped();
			}

			else
			{
				WidgetPtr->LocalVideoStarted();
			}
		});
}

void UCustomAgoraVideoWidget::FUserRtcEventHandler::onRemoteVideoStateChanged(uid_t uid, REMOTE_VIDEO_STATE state, REMOTE_VIDEO_STATE_REASON reason, int elapsed)
{
	AsyncTask(ENamedThreads::GameThread, [=]()
		{
			UE_LOG(LogTemp, Warning, TEXT("Remote video changed _ %i _ %i"), state, reason);

			if (state == 0)
			{
				WidgetPtr->RemoteVideoStoped();
			}

			else
			{
				WidgetPtr->RemoteVideoStarted();
			}
		});
}

void UCustomAgoraVideoWidget::FUserRtcEventHandler::onLeaveChannel(const agora::rtc::RtcStats& stats) 
{
	if (!IsWidgetValid())
		return;

	AsyncTask(ENamedThreads::GameThread, [=]()
	{
		if (!IsWidgetValid())
		{
			UBFL_Logger::PrintError(FString::Printf(TEXT("%s bIsDestruct "), *FString(FUNCTION_MACRO)));
			return;
		}
		UBFL_Logger::Print(FString::Printf(TEXT("%s "), *FString(FUNCTION_MACRO)), nullptr);
	});
}
#pragma endregion

void UCustomAgoraVideoWidget::EnableAudio(bool Enable)
{
	if (Enable)
	{
		RtcEngineProxy->enableAudio();
	}
	else
	{
		RtcEngineProxy->disableAudio();
	}
}	

void UCustomAgoraVideoWidget::EnableVideo(bool Enable)
{
	if (Enable)
	{
		RtcEngineProxy->enableVideo();
	}
	else
	{
		RtcEngineProxy->disableVideo();
	}
}

void UCustomAgoraVideoWidget::RenewToken(FString NewToken)
{
	Token = NewToken;
}

void UCustomAgoraVideoWidget::ChangeVideoConfig(int32 ResX, int32 ResY, int BitRate)
{
	if(RtcEngineProxy)
	{
		VideoEncoderConfiguration videoEncoderConfiguration;

		VideoDimensions videoDimensions(ResX, ResY);
		videoEncoderConfiguration.dimensions = videoDimensions;
		videoEncoderConfiguration.bitrate = BitRate;

		int ret = RtcEngineProxy->setVideoEncoderConfiguration(videoEncoderConfiguration);

		FVideoViewIdentity LocalVideoFrameIdentity(VIDEO_SOURCE_CAMERA);
	}
}
