// Fill out your copyright notice in the Description page of Project Settings.
#include "MyGameInstance.h"
#include "Misc/Guid.h"

void UMyGameInstance::Init()
{
    Super::Init();

    //Генерация CreatorID только если он пустой
    if (CreatorID.IsEmpty())
    {
        CreatorID = FGuid::NewGuid().ToString(EGuidFormats::DigitsWithHyphens);
        UE_LOG(LogTemp, Log, TEXT("Generated new ClientID from game instance: %s"), *CreatorID);
    }
}