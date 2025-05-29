// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
//#include "WebSocketActor.h"
#include "MyGameInstance.generated.h"

UCLASS()
class TESTDATAXO_API UMyGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:

	UPROPERTY(BlueprintReadWrite)
		FString GameId;

	UPROPERTY(BlueprintReadWrite)
		int32 sidePlayerId;

	UPROPERTY(BlueprintReadWrite)
		int32 CreateOrJoin;

	UPROPERTY(BlueprintReadWrite)
		FString GameIdForTextBox;

	UPROPERTY(BlueprintReadWrite)
		int32 board_wight;

	UPROPERTY(BlueprintReadWrite)
		int32 board_height;
};