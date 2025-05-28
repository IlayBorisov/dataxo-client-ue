// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WebSockets/Public/IWebSocket.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"
#include "WebSocketActor.generated.h"


USTRUCT(BlueprintType)
struct FGameConfig
{
    GENERATED_BODY()

        UPROPERTY(BlueprintReadWrite)
        int32 PlayerFiguresLimit;

    UPROPERTY(BlueprintReadWrite)
        int32 WinLineLength;

    UPROPERTY(BlueprintReadWrite)
        int32 BoardWidth;

    UPROPERTY(BlueprintReadWrite)
        int32 BoardHeight;
};

USTRUCT(BlueprintType)
struct FMoveData
{
    GENERATED_BODY()

        UPROPERTY(BlueprintReadWrite)
        int32 Id;

    UPROPERTY(BlueprintReadWrite)
        int32 InGameId;

    UPROPERTY(BlueprintReadWrite)
        int32 X;

    UPROPERTY(BlueprintReadWrite)
        int32 Y;

    UPROPERTY(BlueprintReadWrite)
        int32 TimesUsed;

    UPROPERTY(BlueprintReadWrite)
        int32 Side;
};

USTRUCT(BlueprintType)
struct FGameStateResponseData
{
    GENERATED_BODY()

        UPROPERTY(BlueprintReadWrite)
        FString Type;

    UPROPERTY(BlueprintReadWrite)
        FString ResponseForId;

    UPROPERTY(BlueprintReadWrite)
        FString GameId;

    UPROPERTY(BlueprintReadWrite)
        FString Mode;

    UPROPERTY(BlueprintReadWrite)
        FGameConfig Config;

    UPROPERTY(BlueprintReadWrite)
        FString State;

    UPROPERTY(BlueprintReadWrite)
        TArray<FMoveData> Moves;

    UPROPERTY(BlueprintReadWrite)
        TArray<FMoveData> WinSequence;

    UPROPERTY(BlueprintReadWrite)
        int32 Winner;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWebSocketMessageDelegate, const FString&, MessageString);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSideAssigned, int32, Side);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGameStateResponseReceived, const FGameStateResponseData&, Data);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGameCreated, const FString&, GameId, int32, Side);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGameStateReceived, const FString&, State, int32, Winner);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnErrorMessage, const FString&, ErrorText, bool, bNeedResync);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FMyParsedMessageDelegate, const FString&, Type, int32, X, int32, Y);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_SixParams(FOnMoveEvent, const FString&, MoveType, int32, MoveId, int32, X, int32, Y, int32, Side, int32, TimesUsed);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGameStarted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStartBroadcast);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGameFinished);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FWebSocketConnectedDelegate);


UCLASS(Blueprintable)
class AWebSocketActor : public AActor
{
    GENERATED_BODY()

public:
    AWebSocketActor();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // WebSocket functions
    UFUNCTION(BlueprintCallable, Category = "WebSocket")
        void ConnectToServer(const FString& ServerURL);

    UFUNCTION(BlueprintCallable, Category = "WebSocket")
        void Disconnect();

    UFUNCTION(BlueprintCallable, Category = "WebSocket")
        bool IsConnected() const;

    // Game functions
    UFUNCTION(BlueprintCallable, Category = "WebSocket|Game")
        void CreateGame();

    UFUNCTION(BlueprintCallable, Category = "WebSocket|Game")
        void JoinGame(const FString& InGameId);

    UFUNCTION(BlueprintCallable, Category = "WebSocket|Game")
        void MakeMove(int32 X, int32 Y, int32 MoveId = 0);

    UFUNCTION(BlueprintCallable, Category = "WebSocket|Game")
        void RequestGameState();

    UFUNCTION(BlueprintCallable, Category = "WebSocket|Game")
        void RequestSide();

    UFUNCTION(BlueprintCallable, Category = "WebSocket|Game")
        void RequestPresence();

    UFUNCTION(BlueprintCallable, Category = "WebSocket|Game")
        void SendJson(const FString& Type, int32 X, int32 Y);

    // Properties
    UPROPERTY(BlueprintReadOnly, Category = "WebSocket")
        FString ClientId;

    UPROPERTY(BlueprintReadOnly, Category = "WebSocket|Game")
        FString CurrentGameId;

    UPROPERTY(BlueprintReadOnly, Category = "WebSocket|Game")
        int32 PlayerSide;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "WebSocket")
        FWebSocketMessageDelegate OnMessageEvent;

    UPROPERTY(BlueprintAssignable, Category = "WebSocket")
        FWebSocketConnectedDelegate OnConnectedEvent;

    UPROPERTY(BlueprintAssignable, Category = "WebSocket|Game")
        FOnGameCreated OnGameCreated;

    UPROPERTY(BlueprintAssignable, Category = "WebSocket|Game")
        FOnSideAssigned OnSideAssigned;

    UPROPERTY(BlueprintAssignable, Category = "WebSocket|Game")
        FOnGameStarted OnGameStarted;

    UPROPERTY(BlueprintAssignable, Category = "WebSocket")
        FOnStartBroadcast OnStartBroadcast;

    UPROPERTY(BlueprintAssignable, Category = "WebSocket|Game")
        FOnGameFinished OnGameFinished;

    UPROPERTY(BlueprintAssignable, Category = "WebSocket|Game")
        FOnGameStateReceived OnGameStateReceived;

    UPROPERTY(BlueprintAssignable, Category = "WebSocket|Game")
        FOnErrorMessage OnErrorMessage;

    UPROPERTY(BlueprintAssignable, Category = "WebSocket|Game")
        FOnMoveEvent OnMoveEvent;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "WebSocket")
        FString GameId;

    UPROPERTY(BlueprintAssignable, Category = "WebSocket|Game")
        FOnGameStateResponseReceived OnGameStateResponseReceived;

private:
    TSharedPtr<IWebSocket> WebSocket;
    FString CurrentUrl;
    void SendJsonMessage(const TSharedPtr<FJsonObject>& JsonObject);
    void ProcessJsonMessage(const TSharedPtr<FJsonObject>& JsonObject);
    bool bShouldSendPresence = false;
    FString PendingGameId;
    bool bGameStarted = false;
    void HandleGameStateResponse(const TSharedPtr<FJsonObject>& JsonObject);

    FString GameCreatorClientId; // ƒл€ создател€ игры
    FString GameJoinerClientId; // ƒл€ присоедин€ющегос€ игрока
};