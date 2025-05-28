// Fill out your copyright notice in the Description page of Project Settings.


#include "GameUIWidget.h"

void UGameUIWidget::SaveGameIdFromTextBox()
{
	if (GameIdTextBox)
	{
		GameIdFortextBox = GameIdTextBox->GetText().ToString();
		UE_LOG(LogTemp, Log, TEXT("Saved GameId: %s"), *GameIdFortextBox);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("GameIdTextBox is not assigned!"));
	}
}

FString UGameUIWidget::GetGameIdFromTextBox() const
{
	return GameIdFortextBox;
}