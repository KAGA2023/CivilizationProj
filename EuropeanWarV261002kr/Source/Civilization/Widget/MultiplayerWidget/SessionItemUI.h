// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FindSessionsCallbackProxy.h"
#include "SessionItemUI.generated.h"

UCLASS()
class CIVILIZATION_API USessionItemUI : public UUserWidget
{
	GENERATED_BODY()

public:
	// ========== 수명 주기 ==========

	// 참가 버튼을 묶고 세션 참가 완료 델리게이트를 등록합니다.
	virtual void NativeConstruct() override;
	// 세션 참가 완료 델리게이트를 해제합니다.
	virtual void NativeDestruct() override;

	// 검색 결과 한 줄을 넣고 방 이름·인원 텍스트를 갱신합니다.
	UFUNCTION(BlueprintCallable, Category = "Session")
	void SetSearchResult(const FBlueprintSessionResult& InSearchResult);

protected:
	// ========== BindWidget ==========

	// 호스트 표시 이름
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* ServerNameTxt = nullptr;

	// 현재 인원 / 최대 인원 텍스트
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* PlayerCountTxt = nullptr;

	// 이 로비에 참가하는 버튼
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* JoinBtn = nullptr;

	// 이 항목이 가리키는 검색 결과
	UPROPERTY(BlueprintReadWrite, Category = "Session")
	FBlueprintSessionResult SearchResult;

	// 세션 서브시스템으로 이 로비 참가를 요청합니다.
	UFUNCTION()
	void OnJoinButtonClicked();

	// 참가 실패 시 버튼을 다시 켭니다. 성공하면 로비 맵으로 이동합니다.
	UFUNCTION()
	void OnJoinSessionComplete(bool bWasSuccessful);

private:
	// ========== 세션 헬퍼 ==========

	// 멀티플레이 세션 서브시스템을 반환합니다.
	class UMultiplayerSessionSubsystem* GetSessionSubsystem() const;
	// SearchResult에서 방 이름과 인원 수를 텍스트에 씁니다.
	void RefreshSessionInfo();
};
