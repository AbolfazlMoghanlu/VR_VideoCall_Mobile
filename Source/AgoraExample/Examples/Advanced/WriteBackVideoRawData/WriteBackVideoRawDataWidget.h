// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../../BaseAgoraUserWidget.h"
#include "AgoraPluginInterface.h"


// UI
#include "Components/Button.h"
#include "Components/CanvasPanel.h"

// UI Utility
#include "../../../Utility/BFL_VideoViewManager.h"
#include "../../../Utility/BFL_Logger.h"

#if PLATFORM_ANDROID
#include "AndroidPermission/Classes/AndroidPermissionFunctionLibrary.h"
#endif

#include "WriteBackVideoRawDataWidget.generated.h"

using namespace agora::rtc;
using namespace agora;


class FUserRtcEventHandler;
class FUserVideoFrameObserver;

/**
 * 
 */
UCLASS()
class AGORAEXAMPLE_API UWriteBackVideoRawDataWidget : public UBaseAgoraUserWidget
{
	GENERATED_BODY()
public:

#pragma region Event Handler

	class FUserRtcEventHandlerEx : public agora::rtc::IRtcEngineEventHandlerEx
	{

	public:

		FUserRtcEventHandlerEx(UWriteBackVideoRawDataWidget* InVideoWidget) : WidgetPtr(InVideoWidget) {};

#pragma region AgoraCallback - IRtcEngineEventHandlerEx

		void onJoinChannelSuccess(const agora::rtc::RtcConnection& connection, int elapsed) override;

		void onLeaveChannel(const agora::rtc::RtcConnection& connection, const agora::rtc::RtcStats& stats) override;

		void onUserJoined(const agora::rtc::RtcConnection& connection, agora::rtc::uid_t remoteUid, int elapsed) override;

		void onUserOffline(const agora::rtc::RtcConnection& connection, agora::rtc::uid_t remoteUid, agora::rtc::USER_OFFLINE_REASON_TYPE reason) override;

#pragma endregion

		inline bool IsWidgetValid() { return WidgetPtr.IsValid(); }

	private:

		TWeakObjectPtr<UWriteBackVideoRawDataWidget> WidgetPtr;
	};
	
	class FUserVideoFrameObserver : public agora::media::IVideoFrameObserver
	{

	public:

		FUserVideoFrameObserver(UWriteBackVideoRawDataWidget* InVideoWidget) : WidgetPtr(InVideoWidget) {
			LogSet.Empty();
		};

#pragma region AgoraCallback - FUserVideoFrameObserver

		bool onCaptureVideoFrame(agora::rtc::VIDEO_SOURCE_TYPE sourceType, agora::media::base::VideoFrame& videoFrame) override;

		bool onPreEncodeVideoFrame(agora::rtc::VIDEO_SOURCE_TYPE sourceType, agora::media::base::VideoFrame& videoFrame) override;

		bool onMediaPlayerVideoFrame(agora::media::base::VideoFrame& videoFrame, int mediaPlayerId) override;

		bool onRenderVideoFrame(const char* channelId, agora::rtc::uid_t remoteUid, agora::media::base::VideoFrame& videoFrame) override;

		bool onTranscodedVideoFrame(agora::media::base::VideoFrame& videoFrame) override;

		agora::media::IVideoFrameObserver::VIDEO_FRAME_PROCESS_MODE getVideoFrameProcessMode() override;

		agora::media::base::VIDEO_PIXEL_FORMAT getVideoFormatPreference() override;

#pragma endregion
		
		inline bool IsWidgetValid() { return WidgetPtr.IsValid(); }

	private:
		TSet<FName> LogSet;
		TWeakObjectPtr<UWriteBackVideoRawDataWidget> WidgetPtr;
	};

#pragma endregion


public:

#pragma region UI
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (BindWidget))
		UButton* Btn_BackToHome = nullptr;

	UFUNCTION(BlueprintCallable)
	void OnBtnBackToHomeClicked();

#pragma endregion


	void InitAgoraWidget(FString APP_ID, FString TOKEN, FString CHANNEL_NAME) override;


#pragma region UI Utility - Video View

public:

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (BindWidget))
	UCanvasPanel* CanvasPanel_VideoView = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UDraggableVideoViewWidget> DraggableVideoViewTemplate;
	
protected:

	int MakeVideoView(uint32 uid, agora::rtc::VIDEO_SOURCE_TYPE sourceType = VIDEO_SOURCE_CAMERA_PRIMARY,FString channelId = "");
	int ReleaseVideoView(uint32 uid, agora::rtc::VIDEO_SOURCE_TYPE sourceType = VIDEO_SOURCE_CAMERA_PRIMARY, FString channelId = "");

	TMap<FVideoViewIdentity, UDraggableVideoViewWidget*> VideoViewMap;

#pragma endregion

#pragma region UI Utility - Log Msg View

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (BindWidget))
	UCanvasPanel* CanvasPanel_LogMsgView = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TSubclassOf<UDraggableLogMsgViewWidget> DraggableLogMsgViewTemplate;

public:
	inline UDraggableLogMsgViewWidget* GetLogMsgViewPtr() {return LogMsgViewPtr;} 

private:
	UDraggableLogMsgViewWidget* LogMsgViewPtr = nullptr;

#pragma endregion


public:
	inline FString GetAppId() { return AppId; };
	inline FString GetToken() { return Token; };
	inline FString GetChannelName() { return ChannelName; };


protected:
	void CheckPermission();
	void JoinChannel();
	void NativeDestruct() override;

	void InitAgoraEngine(FString APP_ID, FString TOKEN, FString CHANNEL_NAME);
	void UnInitAgoraEngine();

	FString AppId = "";
	FString Token = "";
	FString ChannelName = "";

	IRtcEngine* RtcEngineProxy;
	agora::media::IMediaEngine* MediaEngine;


		
	TSharedPtr<FUserRtcEventHandlerEx> UserRtcEventHandlerEx;
	TSharedPtr<FUserVideoFrameObserver> UserVideoFrameObserver;

};




