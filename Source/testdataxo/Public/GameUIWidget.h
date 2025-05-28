// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/EditableTextBox.h"
#include "GameUIWidget.generated.h"


/**
 * 
 */
UCLASS()
class TESTDATAXO_API UGameUIWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	// Получение текста из текстбокса
	UFUNCTION(BlueprintCallable, Category = "WebSocket")
		void SaveGameIdFromTextBox();

	UFUNCTION(BlueprintCallable, Category = "WebSocket")
		FString GetGameIdFromTextBox() const;

	// Поле ввода (должно быть связано через BindWidget)
	UPROPERTY(meta = (BindWidget))
		UEditableTextBox* GameIdTextBox;

	// Хранение полученного GameId
	UPROPERTY(BlueprintReadOnly, Category = "WebSocket")
		FString GameIdFortextBox;
};
