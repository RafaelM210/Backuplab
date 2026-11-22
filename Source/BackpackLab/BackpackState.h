#pragma once
#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Engine/DataAsset.h"
#include "GameFramework/SaveGame.h"
#include "BackpackState.generated.h"

UCLASS(BlueprintType)
class BACKPACKLAB_API UItemDefinition : public UDataAsset {
 GENERATED_BODY()
public:
 UPROPERTY(EditAnywhere, BlueprintReadOnly) FName ItemId;
 UPROPERTY(EditAnywhere, BlueprintReadOnly) FText DisplayName;
 UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin="1")) int32 MaxStack=1;
};

USTRUCT(BlueprintType)
struct FInventoryEntry {
 GENERATED_BODY()
 UPROPERTY(EditAnywhere, BlueprintReadOnly, SaveGame) FName ItemId;
 UPROPERTY(EditAnywhere, BlueprintReadOnly, SaveGame) int32 Quantity=0;
 FInventoryEntry()=default;
 FInventoryEntry(FName Id,int32 Count):ItemId(Id),Quantity(Count){}
};

UCLASS(BlueprintType, Blueprintable)
class BACKPACKLAB_API UBackpackSave : public USaveGame {
 GENERATED_BODY()
public:
 UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="Week 7|Save Schema") int32 SaveVersion=2;
 UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="Week 7|Save Schema") FName CheckpointId=TEXT("Start");
 UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="Week 7|Save Schema") TArray<FInventoryEntry> Inventory;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FInventoryChanged);
UCLASS(BlueprintType, Blueprintable)
class BACKPACKLAB_API UBackpackState : public UGameInstance {
 GENERATED_BODY()
public:
 virtual void Init() override;
 UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Week 7|Persistence") TSubclassOf<UBackpackSave> SaveClass = UBackpackSave::StaticClass();
 UPROPERTY(BlueprintAssignable, Category="Week 7|Events") FInventoryChanged OnInventoryChanged;
 UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Week 7|Catalog") TArray<TObjectPtr<UItemDefinition>> Catalog;
 UPROPERTY(BlueprintAssignable, Category="Week 7|Events") FInventoryChanged OnStatusChanged;
 // Live state is private. UI reads copies through accessors.
 const TArray<FInventoryEntry>& GetEntries() const {return Entries;}
 UFUNCTION(BlueprintPure, Category="Week 7|State")
 FName GetCheckpoint() const {return Checkpoint;}
 UFUNCTION(BlueprintPure, Category="Week 7|State")
 int32 GetVersion() const {return 2;}
 UFUNCTION(BlueprintPure, Category="Week 7|State")
 bool IsBusy() const {return bSaving;}
 UPROPERTY(BlueprintReadOnly, Category="Week 7|Status") FString Message=TEXT("Ready. Load is explicit: press L after a restart.");
 FString Slot=TEXT("Week07_Backpack");
 static constexpr int32 Capacity=2; // Two distinct IDs, one stack per ID.
 const UItemDefinition* FindDefinition(FName Id) const;
 bool ValidateCatalog(FString& Error) const;
 UFUNCTION(BlueprintCallable, Category="Week 7|Operations")
 bool AddItem(FName Id,int32 Count);
 UFUNCTION(BlueprintCallable, Category="Week 7|Operations")
 bool RemoveItem(FName Id,int32 Count);
 UFUNCTION(BlueprintCallable, Category="Week 7|Operations")
 void SetCheckpoint(FName Id);
 UFUNCTION(BlueprintCallable, Category="Week 7|Operations")
 void Defaults();
 UFUNCTION(BlueprintCallable, Category="Week 7|Operations")
 UBackpackSave* BuildSnapshot();
 UFUNCTION(BlueprintCallable, Category="Week 7|Persistence")
 bool ValidateAndMigrate(const UBackpackSave* Input,TArray<FInventoryEntry>& Out,FName& OutCheckpoint,FString& Error) const;
 UFUNCTION(BlueprintCallable, Category="Week 7|Operations")
 bool ApplySnapshot(const UBackpackSave* Input);
 UFUNCTION(BlueprintCallable, Category="Week 7|Operations")
 bool Save();
 UFUNCTION(BlueprintCallable, Category="Week 7|Operations")
 bool Load();
 UFUNCTION(BlueprintCallable, Category="Week 7|Operations")
 void Fixture(int32 Kind);
 UFUNCTION(BlueprintPure, Category="Week 7|State")
 TArray<FInventoryEntry> GetInventory() const { return Entries; }
 UFUNCTION(BlueprintPure, Category="Week 7|Catalog")
 UItemDefinition* GetDefinition(FName Id) const { return const_cast<UItemDefinition*>(FindDefinition(Id)); }
 UFUNCTION(BlueprintCallable, Category="Week 7|Operations")
 void ToggleCheckpoint() { SetCheckpoint(Checkpoint==TEXT("Start")?FName(TEXT("LabDoor")):FName(TEXT("Start"))); }
 void SetStatus(const FString& Text) { Message=Text; OnStatusChanged.Broadcast(); }
 void Verify(const FString& Mode);
private:
 UPROPERTY() TArray<FInventoryEntry> Entries;
 UPROPERTY() FName Checkpoint=TEXT("Start");
 UPROPERTY() TObjectPtr<UBackpackSave> PendingSave;
 bool bSaving=false;
 bool bVerifyWrite=false;
 void SaveFinished(const FString& Name,int32 User,bool bSuccess);
};
