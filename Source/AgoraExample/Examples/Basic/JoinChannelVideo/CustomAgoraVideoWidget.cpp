// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomAgoraVideoWidget.h"

void UCustomAgoraVideoWidget::InitAgoraWidget(FString APP_ID, FString TOKEN, FString CHANNEL_NAME)
{
	LogMsgViewPtr = UBFL_Logger::CreateLogView(CanvasPanel_LogMsgView, DraggableLogMsgViewTemplate);

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
	UBFL_Logger::Print(FString::Printf(TEXT("SDK Info:  %s"), *SDKInfo), LogMsgViewPtr);

	int ret = RtcEngineProxy->initialize(RtcEngineContext);
	UBFL_Logger::Print(FString::Printf(TEXT("%s initialize ret %d"), *FString(FUNCTION_MACRO), ret), LogMsgViewPtr);
}


void UCustomAgoraVideoWidget::UnInitAgoraEngine()
{
	if (RtcEngineProxy)
	{
		RtcEngineProxy->leaveChannel();
		RtcEngineProxy->unregisterEventHandler(UserRtcEventHandler.Get());
		RtcEngineProxy->release();
		RtcEngineProxy = nullptr;
		UBFL_Logger::Print(FString::Printf(TEXT("%s release agora engine"), *FString(FUNCTION_MACRO)), LogMsgViewPtr);
	}
}

void UCustomAgoraVideoWidget::OnBtnJoinChannelClicked()
{
	RtcEngineProxy->enableAudio();
	RtcEngineProxy->enableVideo();
	int ret = RtcEngineProxy->joinChannel(TCHAR_TO_UTF8(*Token), TCHAR_TO_UTF8(*ChannelName), "", 0);
	UBFL_Logger::Print(FString::Printf(TEXT("%s ret %d"), *FString(FUNCTION_MACRO), ret), LogMsgViewPtr);
	RtcEngineProxy->setClientRole(agora::rtc::CLIENT_ROLE_TYPE::CLIENT_ROLE_BROADCASTER);
}

void UCustomAgoraVideoWidget::OnBtnLeaveChannelClicked()
{
	int ret = RtcEngineProxy->leaveChannel();
	UBFL_Logger::Print(FString::Printf(TEXT("%s ret %d"), *FString(FUNCTION_MACRO), ret), LogMsgViewPtr);
}

void UCustomAgoraVideoWidget::OnBtnStartPublishClicked()
{
	ChannelMediaOptions options;
	options.publishMicrophoneTrack = true;
	options.publishCameraTrack = true;
	int ret = RtcEngineProxy->updateChannelMediaOptions(options);
}

void UCustomAgoraVideoWidget::OnBtnStopPublishClicked()
{
	ChannelMediaOptions options;
	options.publishMicrophoneTrack = false;
	options.publishCameraTrack = false;
	int ret = RtcEngineProxy->updateChannelMediaOptions(options);
	UBFL_Logger::Print(FString::Printf(TEXT("%s updateChannelMediaOptions ret %d"), *FString(FUNCTION_MACRO), ret), LogMsgViewPtr);
}

void UCustomAgoraVideoWidget::OnBtnVideoConfigConfirmClicked()
{
	VideoEncoderConfiguration videoEncoderConfiguration;

	FString TxtFPS = ET_FPS->GetText().ToString();
	int ValFPS = FCString::Atoi(*TxtFPS);
	videoEncoderConfiguration.frameRate = ValFPS;
	UBFL_Logger::Print(FString::Printf(TEXT("%s frameRate=%d"), *FString(FUNCTION_MACRO), ValFPS), LogMsgViewPtr);

	FString TxtWidth = ET_Width->GetText().ToString();
	FString TxtHeight = ET_Height->GetText().ToString();
	FString TxtBitRate = ET_BitRate->GetText().ToString();
	int ValWidth = FCString::Atoi(*TxtWidth);
	int ValHeight = FCString::Atoi(*TxtHeight);
	int ValBitRate = FCString::Atoi(*TxtBitRate);
	VideoDimensions videoDimensions(ValWidth, ValHeight);
	videoEncoderConfiguration.dimensions = videoDimensions;
	videoEncoderConfiguration.bitrate = ValBitRate;

	UBFL_Logger::Print(FString::Printf(TEXT("%s Width=%d Height=%d"), *FString(FUNCTION_MACRO), ValWidth, ValHeight), LogMsgViewPtr);
	UBFL_Logger::Print(FString::Printf(TEXT("%s Bitrate=%d"), *FString(FUNCTION_MACRO), ValBitRate), LogMsgViewPtr);

	int ret = RtcEngineProxy->setVideoEncoderConfiguration(videoEncoderConfiguration);

	FVideoViewIdentity LocalVideoFrameIdentity(VIDEO_SOURCE_CAMERA);
	UBFL_VideoViewManager::ChangeSizeForOneVideoView(LocalVideoFrameIdentity,ValWidth,ValHeight, VideoViewMap);
	
	UBFL_Logger::Print(FString::Printf(TEXT("%s setVideoEncoderConfiguration ret %d"), *FString(FUNCTION_MACRO), ret), LogMsgViewPtr);

}

void UCustomAgoraVideoWidget::OnBtnBackToHomeClicked()
{
	UnInitAgoraEngine();
	UGameplayStatics::OpenLevel(UGameplayStatics::GetPlayerController(GWorld, 0)->GetWorld(), FName("MainLevel"));
}


void UCustomAgoraVideoWidget::NativeDestruct() {

	Super::NativeDestruct();

	UnInitAgoraEngine();

	
}


#pragma region UI Utility

int UCustomAgoraVideoWidget::MakeVideoView(uint32 uid, agora::rtc::VIDEO_SOURCE_TYPE sourceType /*= VIDEO_SOURCE_CAMERA_PRIMARY*/, FString channelId /*= ""*/)
{
	/*
		For local view:
			please reference the callback function Ex.[onCaptureVideoFrame]

		For remote view:
			please reference the callback function [onRenderVideoFrame]

		channelId will be set in [setupLocalVideo] / [setupRemoteVideo]
	*/

	int ret = -ERROR_NULLPTR;

	if (RtcEngineProxy == nullptr)
		return ret;

	agora::rtc::VideoCanvas videoCanvas;
	videoCanvas.uid = uid;
	videoCanvas.sourceType = sourceType;

	if (uid == 0) {
		FVideoViewIdentity VideoViewIdentity(uid, sourceType, "");
		videoCanvas.view = UBFL_VideoViewManager::CreateOneVideoViewToCanvasPanel(VideoViewIdentity, CanvasPanel_VideoView, VideoViewMap, DraggableVideoViewTemplate);
		ret = RtcEngineProxy->setupLocalVideo(videoCanvas);
	}
	else
	{

		FVideoViewIdentity VideoViewIdentity(uid, sourceType, channelId);
		videoCanvas.view = UBFL_VideoViewManager::CreateOneVideoViewToCanvasPanel(VideoViewIdentity, CanvasPanel_VideoView, VideoViewMap, DraggableVideoViewTemplate);
		
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

	if (uid == 0) {
		FVideoViewIdentity VideoViewIdentity(uid, sourceType, "");
		UBFL_VideoViewManager::ReleaseOneVideoView(VideoViewIdentity, VideoViewMap);
		ret = RtcEngineProxy->setupLocalVideo(videoCanvas);
	}
	else
	{
		FVideoViewIdentity VideoViewIdentity(uid, sourceType, channelId);
		UBFL_VideoViewManager::ReleaseOneVideoView(VideoViewIdentity, VideoViewMap);
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
			UBFL_Logger::Print(FString::Printf(TEXT("%s "), *FString(FUNCTION_MACRO)), WidgetPtr->GetLogMsgViewPtr());
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
			UBFL_Logger::Print(FString::Printf(TEXT("%s "), *FString(FUNCTION_MACRO)), WidgetPtr->GetLogMsgViewPtr());
			WidgetPtr->MakeVideoView(uid, agora::rtc::VIDEO_SOURCE_TYPE::VIDEO_SOURCE_REMOTE);
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
			UBFL_Logger::Print(FString::Printf(TEXT("%s "), *FString(FUNCTION_MACRO)), WidgetPtr->GetLogMsgViewPtr());
			WidgetPtr->ReleaseVideoView(uid, VIDEO_SOURCE_REMOTE);
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
			UBFL_Logger::Print(FString::Printf(TEXT("%s uid=%d width=%d height=%d "), *FString(FUNCTION_MACRO), uid, width, height), WidgetPtr->GetLogMsgViewPtr());
			
			FVideoViewIdentity VideoViewIdentity(uid, sourceType, "");
			UBFL_VideoViewManager::ChangeSizeForOneVideoView(VideoViewIdentity, width, height, WidgetPtr->VideoViewMap);
			
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
		UBFL_Logger::Print(FString::Printf(TEXT("%s "), *FString(FUNCTION_MACRO)), WidgetPtr->GetLogMsgViewPtr());

		//WidgetPtr->ReleaseVideoView(0, agora::rtc::VIDEO_SOURCE_TYPE::VIDEO_SOURCE_CAMERA);

// 		TArray<uint32> uids;
// 
// 
// 		for (auto& map : WidgetPtr->VideoViewMap)
// 		{
// 			uids.AddUnique(map.Key.uid);
// 		}
// 
// 		for (uint32 i : uids)
// 		{
// 			WidgetPtr->ReleaseVideoView(i, agora::rtc::VIDEO_SOURCE_TYPE::VIDEO_SOURCE_CAMERA);
// 		}

		UBFL_VideoViewManager::ReleaseAllVideoView(WidgetPtr->VideoViewMap);

	});
}
#pragma endregion



UImage* UCustomAgoraVideoWidget::GetRemoteImage() const
{
	for(auto& View : VideoViewMap)
	{
		if(View.Key.uid != 0)
		{
			return View.Value->View;
		}
	}

	return nullptr;
}

UImage* UCustomAgoraVideoWidget::GetLocalImage() const
{
	for(auto& View : VideoViewMap)
	{
		if(View.Key.uid == 0)
		{
			return View.Value->View;
		}
	}

	return nullptr;
}

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
	VideoEncoderConfiguration videoEncoderConfiguration;

	VideoDimensions videoDimensions(ResX, ResY);
	videoEncoderConfiguration.dimensions = videoDimensions;
	videoEncoderConfiguration.bitrate = BitRate;

	int ret = RtcEngineProxy->setVideoEncoderConfiguration(videoEncoderConfiguration);

	FVideoViewIdentity LocalVideoFrameIdentity(VIDEO_SOURCE_CAMERA);
	UBFL_VideoViewManager::ChangeSizeForOneVideoView(LocalVideoFrameIdentity, ResX, ResY, VideoViewMap);
}
