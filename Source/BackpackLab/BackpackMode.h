#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/HUD.h"
#include "Blueprint/UserWidget.h"
#include "BackpackMode.generated.h"
UCLASS(Blueprintable)
class BACKPACKLAB_API ABackpackController : public APlayerController {
 GENERATED_BODY()
public:
 UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Week 7|UI") TSubclassOf<UUserWidget> DashboardClass;
 UPROPERTY(BlueprintReadOnly, Category="Week 7|UI") TObjectPtr<UUserWidget> DashboardWidget;
 UFUNCTION(BlueprintCallable, Category="Week 7|UI") void ShowDashboard();
 void VerifyDashboard();
protected:
 virtual void SetupInputComponent() override;
 void AddCell();void AddKey();void RemoveCell();void Reset();void Checkpoint();void Save();void Load();void Legacy();void Invalid();void Future();
};
UCLASS(Blueprintable)
class BACKPACKLAB_API ABackpackHUD : public AHUD {
 GENERATED_BODY()
 virtual void DrawHUD() override;
};
UCLASS(Blueprintable)
class BACKPACKLAB_API ABackpackMode : public AGameModeBase {
 GENERATED_BODY()
public:
 ABackpackMode();
 virtual void BeginPlay() override;
};
