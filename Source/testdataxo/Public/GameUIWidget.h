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
	// ��������� ������ �� ����������
	UFUNCTION(BlueprintCallable, Category = "WebSocket")
		void SaveGameIdFromTextBox();

	UFUNCTION(BlueprintCallable, Category = "WebSocket")
		FString GetGameIdFromTextBox() const;

	// ���� ����� (������ ���� ������� ����� BindWidget)
	UPROPERTY(meta = (BindWidget))
		UEditableTextBox* GameIdTextBox;

	// �������� ����������� GameId
	UPROPERTY(BlueprintReadOnly, Category = "WebSocket")
		FString GameIdFortextBox;
};
