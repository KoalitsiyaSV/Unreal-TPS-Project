// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"

#include "MPSessionsSubsystem.generated.h"

// Declaring our own Custom Delegates for the Menu Class to Bind Callbacks to
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerOnCreateSessionComplete, bool, bWasSuccessful);
DECLARE_MULTICAST_DELEGATE_TwoParams(FMultiplayerOnFindSessionsComplete, const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful);
DECLARE_MULTICAST_DELEGATE_OneParam(FMultiplayerOnJoinSessionComplete, EOnJoinSessionCompleteResult::Type Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerOnDestroySessionComplete, bool, bwasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerOnStartSessionComplete, bool, bwasSuccessful);

/**
 * 
 */
UCLASS()
class MPSESSIONS_API UMPSessionsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UMPSessionsSubsystem();

	// To Handle Session Functionality. The Menu Class will Call These
	void CreateSession(int32 NumPublicConnections, FString MatchType);
	void FindSessions(int32 MaxSearchResults);
	void JoinSession(const FOnlineSessionSearchResult& SessionResult);
	void DestroySession();
	void StartSession();

	bool IsValidSessionInterface();

	// Our own Custom Delegates for the Menu Class to Bind Callbacks to
	FMultiplayerOnCreateSessionComplete MultiPlayerOnCreateSessionComplete;
	FMultiplayerOnFindSessionsComplete MultiPlayerOnFindSessionsComplete;
	FMultiplayerOnJoinSessionComplete MultiPlayerOnJoinSessionComplete;
	FMultiplayerOnDestroySessionComplete MultiPlayerOnDestroySessionComplete;
	FMultiplayerOnStartSessionComplete MultiPlayerOnStartSessionComplete;


protected:

	// Internal Callbacks for the Delegates We'll Add to the Online Session Interface Delegate List.
	// These don't Need to be Called Outside this Class.
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnFindSessionsComplete(bool bWasSuccessful);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);
	void OnStartSessionComplete(FName SessionName, bool bWasSuccessful);

private:
	IOnlineSessionPtr SessionInterface;
	TSharedPtr<FOnlineSessionSettings> LastSessionSettings;
	TSharedPtr<FOnlineSessionSearch> LastSessionSearch;

	// To Add to the Online Session Interface Delegate List
	// We'll Bind Our MPSessionsSubsystem Internal Callbacks to These
	FOnCreateSessionCompleteDelegate CreateSessionCompleteDelegate;
	FDelegateHandle CreateSessionCompleteDelegateHandle;
	FOnFindSessionsCompleteDelegate FindSessionsCompleteDelegate;
	FDelegateHandle FindSessionsCompleteDelegateHandle;
	FOnJoinSessionCompleteDelegate JoinSessionCompleteDelegate;
	FDelegateHandle JoinSessionCompleteDelegateHandle;
	FOnDestroySessionCompleteDelegate DestroySessionCompleteDelegate;
	FDelegateHandle DestroySessionCompleteDelegateHandle;
	FOnStartSessionCompleteDelegate StartSessionCompleteDelegate;
	FDelegateHandle StartSessionCompleteDelegateHandle;

	bool bCreateSessionOnDestroy{ false };
	int32 LastNumPublicConnections;
	FString LastMatchType;
};
