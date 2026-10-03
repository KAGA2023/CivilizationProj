// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FindSessionsCallbackProxy.h"
#include "FindSessionUI.generated.h"

class USessionItemUI;

UCLASS()
class CIVILIZATION_API UFindSessionUI : public UUserWidget
{
	GENERATED_BODY()

public:
	// ========== 수명 주기 ==========

	// 찾기 버튼을 묶고 세션 검색 완료 델리게이트를 등록합니다.
	virtual void NativeConstruct() override;
	// 세션 검색 완료 델리게이트를 해제합니다.
	virtual void NativeDestruct() override;

	// 로비 목록만 보여 줍니다. Join은 호스트의 로비 맵으로 들어갑니다.

protected:
	// ========== BindWidget ==========

	// 로비 검색을 시작하는 버튼
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* FindSessionBtn = nullptr;

	// 찾은 로비 항목을 쌓는 세로 박스
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UVerticalBox* SessionList = nullptr;

	// 검색 중·실패·결과 개수를 보여 주는 메시지
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* SessionMessageTxt = nullptr;

	// 목록에 넣을 SessionItem 위젯 클래스. 비어 있으면 기본 W_SessionItem을 씁니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")
	TSubclassOf<USessionItemUI> SessionItemWidgetClass;

	// ========== 검색 콜백 ==========

	// 목록을 비우고 로비 검색을 시작합니다.
	UFUNCTION()
	void OnFindSessionButtonClicked();

	// 검색이 끝나면 결과 수만큼 항목 위젯을 만듭니다.
	UFUNCTION()
	void OnFindSessionsComplete(bool bWasSuccessful);

private:
	// ========== 검색 헬퍼 ==========

	// 멀티플레이 세션 서브시스템을 반환합니다.
	class UMultiplayerSessionSubsystem* GetSessionSubsystem() const;
	// 검색 중에는 찾기 버튼을 잠급니다.
	void SetFindSessionButtonEnabled(bool bEnabled);
	// 검색 상태 메시지를 보여 주거나 숨깁니다.
	void SetSessionMessage(const FString& Message);
	// 지정 클래스가 없으면 기본 SessionItem 블루프린트를 불러옵니다.
	TSubclassOf<USessionItemUI> ResolveSessionItemWidgetClass();
	// 검색 결과 한 줄을 SessionItem으로 만들어 목록에 넣습니다.
	void AddItemRenderer(const FBlueprintSessionResult& SearchResult);
	// 찾은 로비 개수에 맞는 결과 문구를 만듭니다.
	FString GetSessionResultMessage(int32 SessionCount) const;
};
