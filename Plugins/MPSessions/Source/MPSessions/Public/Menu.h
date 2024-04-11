// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Menu.generated.h"

/**
 * 
 */
UCLASS()
class MPSESSIONS_API UMenu : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	void MenuSetup(int32 NumberOfPublicConnections = 4, FString TypeOfMatch = FString(TEXT("FreeForAll")), FString LobbyPath = FString(TEXT("/Game/TPSProject/Levels/TestLevels/MPTestRoom")));

protected:

	virtual bool Initialize() override;
	virtual void NativeDestruct() override;

	// Callbacks for the Custom Delegates on the MPSessionsSubsystem
	UFUNCTION()
	void OnCreateSession(bool bwasSuccessful);
	void OnFindSessions(const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful);
	void OnJoinSession(EOnJoinSessionCompleteResult::Type Result);
	UFUNCTION()
	void OnDestroySession(bool bwasSuccessful);
	UFUNCTION()
	void OnStartSession(bool bwasSuccessful);

private:
	UPROPERTY(meta = (BindWidget))
	class UButton* HostBtn;

	UPROPERTY(meta = (BindWidget))
	UButton* JoinBtn;

	UFUNCTION()
	void HostBtnClicked();

	UFUNCTION()
	void JoinBtnClicked();

	void MenuTearDown();

	// The Subsystem Designed to Handle All Online Session Functionality
	class UMPSessionsSubsystem* MPSessionsSubsystem;

	int32 NumPublicConnections{ 4 };
	FString MatchType{ TEXT("FreeForAll") };
	FString PathToLobby{ TEXT("") };
};
