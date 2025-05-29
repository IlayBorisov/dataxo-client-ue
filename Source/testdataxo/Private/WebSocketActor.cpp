// Fill out your copyright notice in the Description page of Project Settings.

#include "WebSocketActor.h"
#include "GameUIWidget.h"
#include "MyGameInstance.h"
#include "Modules/ModuleManager.h"
#include "WebSockets/Public/WebSocketsModule.h"
#include "WebSockets/Public/IWebSocket.h"
#include "Misc/Guid.h"

AWebSocketActor::AWebSocketActor()
{
	PrimaryActorTick.bCanEverTick = true;
}
void AWebSocketActor::BeginPlay()
{
	Super::BeginPlay();
}
void AWebSocketActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AWebSocketActor::ConnectToServer(const FString& ServerURL)
{
	FString FullUrl;
	FullUrl = FString::Printf(TEXT("%s"), *ServerURL);

	FModuleManager::Get().LoadModule("WebSockets");
	WebSocket = FWebSocketsModule::Get().CreateWebSocket(FullUrl, TEXT("ws"));

	WebSocket->OnConnected().AddLambda([this]() {
		UE_LOG(LogTemp, Log, TEXT("WebSocket connected"));
		OnConnectedEvent.Broadcast();

		RequestPresence();
		RequestGameState();
		RequestSide();
		});

	WebSocket->OnMessage().AddLambda([WeakThis = TWeakObjectPtr<AWebSocketActor>(this)](const FString& Message) {
	if (!WeakThis.IsValid()) return;

	UE_LOG(LogTemp, Log, TEXT("Received message: %s"), *Message);
	WeakThis->OnMessageEvent.Broadcast(Message);

	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Message);
	if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
	{
		WeakThis->ProcessJsonMessage(JsonObject);
	}
	});

	WebSocket->Connect();
}

// Создание игры
void AWebSocketActor::CreateGame()
{
	if (GameCreatorClientId.IsEmpty())
	{
		GameCreatorClientId = FGuid::NewGuid().ToString(EGuidFormats::DigitsWithHyphens);
		UE_LOG(LogTemp, Log, TEXT("Generated GameCreatorClientId: %s"), *GameCreatorClientId);
	}
	const FString FullUrl = FString::Printf(TEXT("ws://51.250.40.190:8080/api/v1/games/create/%s"), *GameCreatorClientId);
	UE_LOG(LogTemp, Log, TEXT("FullUrl for Created games: %s"), *FullUrl);
	ConnectToServer(FullUrl);
}

void AWebSocketActor::JoinGame(const FString& InGameId)
{
	 //Генерируем новый ClientId ТОЛЬКО при присоединении
	if (GameJoinerClientId.IsEmpty())
	{
		GameJoinerClientId = FGuid::NewGuid().ToString(EGuidFormats::DigitsWithHyphens);
	}

	const FString FullUrl = FString::Printf(
		TEXT("ws://51.250.40.190:8080/api/v1/games/%s/%s"),
		*InGameId,
		*GameJoinerClientId
	);

	UE_LOG(LogTemp, Warning, TEXT("Joining game with JoinerID: %s"), *GameJoinerClientId);
	UE_LOG(LogTemp, Warning, TEXT("FullUrl for Join for games: %s"), *FullUrl);
	ConnectToServer(FullUrl);
}

// Метод для отключения от WebSocket сервера
void AWebSocketActor::Disconnect()
{
	if (WebSocket.IsValid() && WebSocket->IsConnected())
	{
		WebSocket->Close();
	}
}

// Проверка состояния подключения к серверу
bool AWebSocketActor::IsConnected() const
{
	return WebSocket.IsValid() && WebSocket->IsConnected();
}

// Метод отправки хода в игре
void AWebSocketActor::MakeMove(int32 X, int32 Y)
{
	if (!bGameStarted)
	{
		UE_LOG(LogTemp, Warning, TEXT("Game has not started yet"));
		return;
	}

	CurrentMoveId++;

	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	JsonObject->SetStringField("type", "game");

	TSharedPtr<FJsonObject> MessageObject = MakeShareable(new FJsonObject);
	MessageObject->SetNumberField("move_id", CurrentMoveId);
	MessageObject->SetNumberField("x", X);
	MessageObject->SetNumberField("y", Y);

	JsonObject->SetObjectField("message", MessageObject);

	SendJsonMessage(JsonObject);
}

void AWebSocketActor::RequestGameState()
{
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	JsonObject->SetStringField("type", "state");
	SendJsonMessage(JsonObject);
}

// Запросить текущее состояние игры
void AWebSocketActor::RequestPresence()
{
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	JsonObject->SetStringField("type", "presence");

	TSharedPtr<FJsonObject> MessageObject = MakeShareable(new FJsonObject);
	MessageObject->SetStringField("action", "join");
	
	JsonObject->SetObjectField("message", MessageObject);

	SendJsonMessage(JsonObject);
}

// Запросить сторону игрока (X или O и т.д.)
void AWebSocketActor::RequestSide()
{
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	JsonObject->SetStringField("type", "side");
	SendJsonMessage(JsonObject);
}

// Общий метод отправки JSON-сообщений через WebSocket
void AWebSocketActor::SendJsonMessage(const TSharedPtr<FJsonObject>& JsonObject)
{
	if (!WebSocket.IsValid() || !WebSocket->IsConnected())
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot send message: WebSocket not connected"));
		return;
	}

	if (!JsonObject.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("Cannot send message: JsonObject is invalid"));
		return;
	}

	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	if (FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer))
	{
		WebSocket->Send(OutputString);
		UE_LOG(LogTemp, Log, TEXT("Sent message: %s"), *OutputString);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to serialize JSON"));
	}
}

// Обработка входящих JSON-сообщений с сервера
void AWebSocketActor::ProcessJsonMessage(const TSharedPtr<FJsonObject>& JsonObject)
{
	if (!JsonObject.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid JSON object received"));
		return;
	}

	// Обработка ошибок
	if (JsonObject->HasField("error"))
	{
		FString ErrorText;
		bool bNeedResync = false;

		if (JsonObject->TryGetStringField("error", ErrorText) &&
			JsonObject->TryGetBoolField("need_re_sync", bNeedResync))
		{
			if (OnErrorMessage.IsBound())
			{
				OnErrorMessage.Broadcast(ErrorText, bNeedResync);
			}
		}
		return;
	}

	FString Type;
	if (!JsonObject->TryGetStringField("type", Type))
	{
		UE_LOG(LogTemp, Warning, TEXT("Message has no type field"));
		return;
	}

	if (Type == "create_message")
	{
		FString ReceivedGameId;
		int32 Side = 0;

		if (JsonObject->TryGetStringField("game_id", ReceivedGameId) &&
			JsonObject->TryGetNumberField("side", Side))
		{
			CurrentGameId = ReceivedGameId;
			PlayerSide = Side;

			if (OnGameCreated.IsBound())
			{
				OnGameCreated.Broadcast(ReceivedGameId, Side);
			}
		}
	}
	else if (Type == "side_message")
	{
		int32 Side = 0;
		if (JsonObject->TryGetNumberField("side", Side))
		{
			PlayerSide = Side;

			if (OnSideAssigned.IsBound())
			{
				OnSideAssigned.Broadcast(Side);
			}
		}
	}
	else if (Type == "start_broadcast")
	{
		RequestGameState();
		UE_LOG(LogTemp, Warning, TEXT("Game started! Broadcast received."));
		bGameStarted = true;

		OnStartBroadcast.Broadcast();
	}
	else if (Type == "new_move_broadcast")
	{
		const TArray<TSharedPtr<FJsonValue>>* MoveEvents;
		if (JsonObject->TryGetArrayField("move_events", MoveEvents))
		{
			for (const TSharedPtr<FJsonValue>& Value : *MoveEvents)
			{
				if (!Value.IsValid()) continue;

				const TSharedPtr<FJsonObject> Move = Value->AsObject();
				if (!Move.IsValid()) continue;

				FString MoveType;
				int32 MoveId = 0, X = 0, Y = 0, Side = 0, TimesUsed = 0;

				if (Move->TryGetStringField("type", MoveType) &&
					Move->TryGetNumberField("move_id", MoveId) &&
					Move->TryGetNumberField("x", X) &&
					Move->TryGetNumberField("y", Y) &&
					Move->TryGetNumberField("side", Side) &&
					Move->TryGetNumberField("times_used", TimesUsed))
				{
					UE_LOG(LogTemp, Warning, TEXT("[WebSocket] Move: Type=%s, MoveId=%d, X=%d, Y=%d, Side=%d, TimesUsed=%d"),
						*MoveType, MoveId, X, Y, Side, TimesUsed);

					if (MoveType == "place_move")
					{
						if (MoveId > CurrentMoveId)
						{
							CurrentMoveId = MoveId;
						}
						else if (MoveId < CurrentMoveId)
						{
							CurrentMoveId = MoveId;
						}
						UE_LOG(LogTemp, Warning, TEXT("[WebSocket] Move for if place_move: Type=%s, MoveId=%d, X=%d, Y=%d, Side=%d, TimesUsed=%d"),
							*MoveType, MoveId, X, Y, Side, TimesUsed);
						OnMoveEvent.Broadcast(MoveType, MoveId, X, Y, Side, TimesUsed);
					}
					else if (MoveType == "remove_move")
					{
						UE_LOG(LogTemp, Warning, TEXT("Remove move at X=%d Y=%d Side=%d"), X, Y, Side);
						OnRemoveMoveEvent.Broadcast(X, Y, 0);
					}
				}
			}
		}
	}
	else if (Type == "game_finish_broadcast")
	{
		if (OnGameFinished.IsBound())
		{
			OnGameFinished.Broadcast();
		}
		RequestGameState();
	}
	else if (Type == "game_state_response")
	{
		HandleGameStateResponse(JsonObject);
	}
	else if (Type == "sync_response")
	{
	FString State = JsonObject->GetStringField("state");
	int32 Winner = JsonObject->GetIntegerField("winner");
	OnGameStateReceived.Broadcast(State, Winner);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Unknown message type: %s"), *Type);
	}
}

void AWebSocketActor::HandleGameStateResponse(const TSharedPtr<FJsonObject>& JsonObject)
{
	FGameStateResponseData GameStateData;

	if (!JsonObject->TryGetStringField("type", GameStateData.Type) ||
		!JsonObject->TryGetStringField("game_id", GameStateData.GameId))
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid game state response: missing required fields"));
		return;
	}

	// Парсим основные поля
	GameStateData.ResponseForId = JsonObject->GetStringField("response_for_id");
	GameStateData.Mode = JsonObject->GetStringField("mode");
	GameStateData.State = JsonObject->GetStringField("state");
	GameStateData.Winner = JsonObject->GetIntegerField("winner");

	// Парсим конфиг
	const TSharedPtr<FJsonObject>* ConfigObject;
	if (JsonObject->TryGetObjectField("config", ConfigObject))
	{
		(*ConfigObject)->TryGetNumberField("player_figures_limit", GameStateData.Config.PlayerFiguresLimit);
		(*ConfigObject)->TryGetNumberField("win_line_length", GameStateData.Config.WinLineLength);
		(*ConfigObject)->TryGetNumberField("board_width", GameStateData.Config.BoardWidth);
		(*ConfigObject)->TryGetNumberField("board_height", GameStateData.Config.BoardHeight);
	}

	// Парсим moves
	const TArray<TSharedPtr<FJsonValue>>* MovesArray;
	if (JsonObject->TryGetArrayField("moves", MovesArray))
	{
		TArray<FMoveData> TempMoves;
		for (const TSharedPtr<FJsonValue>& MoveValue : *MovesArray)
		{
			const TSharedPtr<FJsonObject> MoveObject = MoveValue->AsObject();
			if (MoveObject.IsValid())
			{
				FMoveData Move;
				MoveObject->TryGetNumberField("id", Move.Id);
				MoveObject->TryGetNumberField("in_game_id", Move.InGameId);
				MoveObject->TryGetNumberField("x", Move.X);
				MoveObject->TryGetNumberField("y", Move.Y);
				MoveObject->TryGetNumberField("times_used", Move.TimesUsed);
				MoveObject->TryGetNumberField("side", Move.Side);

				TempMoves.Add(Move);
			}
		}
		GameStateData.Moves = TempMoves;
	}

	// Парсим win_sequence
	const TArray<TSharedPtr<FJsonValue>>* WinSequenceArray;
	if (JsonObject->TryGetArrayField("win_sequence", WinSequenceArray))
	{
		TArray<FMoveData> TempWinMoves;
		for (const TSharedPtr<FJsonValue>& WinMoveValue : *WinSequenceArray)
		{
			const TSharedPtr<FJsonObject> WinMoveObject = WinMoveValue->AsObject();
			if (WinMoveObject.IsValid())
			{
				FMoveData WinMove;
				WinMoveObject->TryGetNumberField("id", WinMove.Id);
				WinMoveObject->TryGetNumberField("in_game_id", WinMove.InGameId);
				WinMoveObject->TryGetNumberField("x", WinMove.X);
				WinMoveObject->TryGetNumberField("y", WinMove.Y);
				WinMoveObject->TryGetNumberField("times_used", WinMove.TimesUsed);
				WinMoveObject->TryGetNumberField("side", WinMove.Side);

				TempWinMoves.Add(WinMove);
			}
		}
		GameStateData.WinSequence = TempWinMoves;
	}

	// Отправляем данные через делегат
	OnGameStateResponseReceived.Broadcast(GameStateData);

	// Также отправляем упрощенные данные через старый делегат для совместимости
	OnGameStateReceived.Broadcast(GameStateData.State, GameStateData.Winner);

	if (GameStateData.State == "started" && !bGameStarted)
	{
		bGameStarted = true;
		OnStartBroadcast.Broadcast();
	}
}

// Альтернативный метод отправки простых JSON-сообщений с типом и координатами
void AWebSocketActor::SendJson(const FString& Type, int32 X, int32 Y)
{
	UE_LOG(LogTemp, Warning, TEXT("SendJson() called with type=%s, x=%d, y=%d"), *Type, X, Y);
	if (!WebSocket.IsValid() || !WebSocket->IsConnected())
	{
		UE_LOG(LogTemp, Warning, TEXT("WebSocket is not connected"));
		return;
	}

	// Создаем JSON объект
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	TSharedPtr<FJsonObject> MessageObject = MakeShareable(new FJsonObject);

	// Заполняем данные
	JsonObject->SetStringField("type", Type);

	MessageObject->SetNumberField("x", X);
	MessageObject->SetNumberField("y", Y);
	JsonObject->SetObjectField("message", MessageObject);

	// Конвертируем JSON в строку
	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

	// Отправляем JSON строку через WebSocket ура
	WebSocket->Send(OutputString);

	UE_LOG(LogTemp, Log, TEXT("Sent JSON: %s"), *OutputString);
}