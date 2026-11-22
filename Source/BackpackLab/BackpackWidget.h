#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BackpackWidget.generated.h"
class UTextBlock;
class UButton;
class UBackpackState;
UCLASS(BlueprintType, Blueprintable)
class BACKPACKLAB_API UBackpackWidget : public UUserWidget {
 GENERATED_BODY()
public:
 UFUNCTION(BlueprintPure, Category="Week 7|Widget") UBackpackState* GetBackpackState() const;
 UFUNCTION(BlueprintCallable, Category="Week 7|Widget") void RefreshDisplay();
protected:
 virtual void NativeConstruct() override;
 virtual void NativeDestruct() override;
 UPROPERTY(meta=(BindWidgetOptional)) TObjectPtr<UTextBlock> InventoryText;
 UPROPERTY(meta=(BindWidgetOptional)) TObjectPtr<UTextBlock> CheckpointText;
 UPROPERTY(meta=(BindWidgetOptional)) TObjectPtr<UTextBlock> StatusText;
 UPROPERTY(meta=(BindWidgetOptional)) TObjectPtr<UButton> SaveButton;
 UPROPERTY(meta=(BindWidgetOptional)) TObjectPtr<UButton> LoadButton;
};
