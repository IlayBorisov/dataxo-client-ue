#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
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

	UPROPERTY(BlueprintReadWrite, Category = "WebSocket")
		FString CreatorID;

protected:
	virtual void Init() override;
};