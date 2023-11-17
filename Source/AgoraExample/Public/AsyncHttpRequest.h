// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Interfaces/IHttpRequest.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "AsyncHttpRequest.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHttpDelegate, FString, Result);

/**
 * 
 */
UCLASS()
class AGORAEXAMPLE_API UAsyncHttpRequest : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()
	

public:
	UAsyncHttpRequest();

	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"))
	static UAsyncHttpRequest* HttpRequest(FString URL, TMap<FString, FString> Headers);

public:

	UPROPERTY(BlueprintAssignable)
	FHttpDelegate OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FHttpDelegate OnFail;

public:

	void Start(FString URL, TMap<FString, FString> Headers);

private:

	/** Handles image requests coming from the web */
	void HandleHttpRequest(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded);
};
