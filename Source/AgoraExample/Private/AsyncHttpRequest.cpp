// Fill out your copyright notice in the Description page of Project Settings.


#include "AsyncHttpRequest.h"


//#include "Blueprint/AsyncTaskDownloadImage.h"
#include "Modules/ModuleManager.h"
//#include "Engine/Texture2D.h"
//#include "Engine/Texture2DDynamic.h"
//#include "IImageWrapper.h"
//#include "IImageWrapperModule.h"
#include "Interfaces/IHttpResponse.h"
#include "HttpModule.h"
//#include "TextureResource.h"
//#include "RenderingThread.h"

//#include UE_INLINE_GENERATED_CPP_BY_NAME(AsyncTaskDownloadImage)
#include UE_INLINE_GENERATED_CPP_BY_NAME(AsyncHttpRequest)


UAsyncHttpRequest::UAsyncHttpRequest()
{
	if (HasAnyFlags(RF_ClassDefaultObject) == false)
	{
		AddToRoot();
	}
}

UAsyncHttpRequest* UAsyncHttpRequest::HttpRequest(FString URL, TMap<FString, FString> Headers)
{
	UAsyncHttpRequest* HttpTask = NewObject<UAsyncHttpRequest>();
	HttpTask->Start(URL, Headers);

	return HttpTask;
}

void UAsyncHttpRequest::Start(FString URL, TMap<FString, FString> Headers)
{
#if !UE_SERVER
	// Create the Http request and add to pending request list
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();

	HttpRequest->OnProcessRequestComplete().BindUObject(this, &UAsyncHttpRequest::HandleHttpRequest);
	HttpRequest->SetURL(URL);
	HttpRequest->SetVerb(TEXT("GET"));

	for (auto Header : Headers)
	{
		HttpRequest->SetHeader(Header.Key, Header.Value);
	}

	HttpRequest->ProcessRequest();
#else
	// On the server we don't execute fail or success we just don't fire the request.
	RemoveFromRoot();
#endif
}


void UAsyncHttpRequest::HandleHttpRequest(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded)
{
#if !UE_SERVER

	RemoveFromRoot();

	if (bSucceeded && HttpResponse.IsValid() && HttpResponse->GetContentLength() > 0)
	{
// 		IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
// 		TSharedPtr<IImageWrapper> ImageWrappers[3] =
// 		{
// 			ImageWrapperModule.CreateImageWrapper(EImageFormat::PNG),
// 			ImageWrapperModule.CreateImageWrapper(EImageFormat::JPEG),
// 			ImageWrapperModule.CreateImageWrapper(EImageFormat::BMP),
// 		};

// 		for (auto ImageWrapper : ImageWrappers)
// 		{
// 			if (ImageWrapper.IsValid() && ImageWrapper->SetCompressed(HttpResponse->GetContent().GetData(), HttpResponse->GetContentLength()))
// 			{
// 				TArray64<uint8> RawData;
// 				const ERGBFormat InFormat = ERGBFormat::BGRA;
// 				if (ImageWrapper->GetRaw(InFormat, 8, RawData))
// 				{
// 					if (UTexture2DDynamic* Texture = UTexture2DDynamic::Create(ImageWrapper->GetWidth(), ImageWrapper->GetHeight()))
// 					{
// 						Texture->SRGB = true;
// 						Texture->UpdateResource();
// 
// 						FTexture2DDynamicResource* TextureResource = static_cast<FTexture2DDynamicResource*>(Texture->GetResource());
// 						if (TextureResource)
// 						{
// 							ENQUEUE_RENDER_COMMAND(FWriteRawDataToTexture)(
// 								[TextureResource, RawData = MoveTemp(RawData)](FRHICommandListImmediate& RHICmdList)
// 							{
// 								TextureResource->WriteRawToTexture_RenderThread(RawData);
// 							});
// 						}
// 						OnSuccess.Broadcast(Texture);
// 						return;
// 					}
// 				}
// 			}
// 		}
		
		FString Result = HttpResponse->GetContentAsString();

		OnSuccess.Broadcast(Result);

		return;
	}

	

	OnFail.Broadcast("Invalid");
	

#endif
}

