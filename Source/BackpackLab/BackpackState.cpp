#include "BackpackState.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"

void UBackpackState::Init() {
 Super::Init();
 if(Catalog.IsEmpty()) {
 Catalog.Add(LoadObject<UItemDefinition>(nullptr,TEXT("/Game/Week07/DA_Item_Cell.DA_Item_Cell")));
 Catalog.Add(LoadObject<UItemDefinition>(nullptr,TEXT("/Game/Week07/DA_Item_Key.DA_Item_Key")));
 }
 Defaults();
 FString Error;
 if(!ValidateCatalog(Error)) SetStatus(Error);
}
const UItemDefinition* UBackpackState::FindDefinition(FName Id) const {
 for(const auto& D:Catalog) if(D && D->ItemId==Id) return D;
 return nullptr;
}
bool UBackpackState::ValidateCatalog(FString& Error) const {
 TSet<FName> Seen;
 if(Catalog.Num()!=2) {Error=TEXT("Catalog must contain exactly two definitions.");return false;}
 for(const auto& D:Catalog) {
  if(!D || D->ItemId.IsNone() || D->MaxStack<1 || Seen.Contains(D->ItemId)) {
   Error=TEXT("Invalid catalog: null, empty ID, duplicate ID or invalid stack.");return false;
  }
  Seen.Add(D->ItemId);
 }
 return true;
}
void UBackpackState::Defaults() {
 Entries={FInventoryEntry(TEXT("Cell"),4)};Checkpoint=TEXT("Start");
 SetStatus(TEXT("Default game: Cell x4 at Start. Disk save is untouched."));OnInventoryChanged.Broadcast();
}
bool UBackpackState::AddItem(FName Id,int32 Count) {
 FString Error;
 const auto* D=FindDefinition(Id);
 if(!ValidateCatalog(Error) || !D || Count<=0) {SetStatus(TEXT("Rejected: unknown ID or invalid quantity/catalog."));return false;}
 for(auto& E:Entries) if(E.ItemId==Id) {
  // Subtract before comparing to avoid overflowing quantity + request.
  if(Count>D->MaxStack-E.Quantity) {SetStatus(TEXT("Rejected atomically: the entire request must fit."));return false;}
  E.Quantity+=Count;SetStatus(TEXT("Added entire request."));OnInventoryChanged.Broadcast();return true;
 }
 if(Entries.Num()>=Capacity || Count>D->MaxStack) {SetStatus(TEXT("Rejected: no free slot or request exceeds stack."));return false;}
 Entries.Add(FInventoryEntry(Id,Count));SetStatus(TEXT("New stack added."));OnInventoryChanged.Broadcast();return true;
}
bool UBackpackState::RemoveItem(FName Id,int32 Count) {
 if(Count<=0) return false;
 for(int32 I=0;I<Entries.Num();++I) if(Entries[I].ItemId==Id) {
  if(Count>Entries[I].Quantity) {SetStatus(TEXT("Rejected: insufficient quantity."));return false;}
  Entries[I].Quantity-=Count;if(Entries[I].Quantity==0) Entries.RemoveAt(I);
  SetStatus(TEXT("Removed requested quantity."));OnInventoryChanged.Broadcast();return true;
 }
 SetStatus(TEXT("Rejected: item not owned."));return false;
}
void UBackpackState::SetCheckpoint(FName Id) {
 if(Id==TEXT("Start") || Id==TEXT("LabDoor")) {Checkpoint=Id;OnInventoryChanged.Broadcast();SetStatus(TEXT("Checkpoint changed in memory."));}
}
UBackpackSave* UBackpackState::BuildSnapshot() {
        // LAB 1 BEGIN
        UBackpackSave* Snapshot = NewObject<UBackpackSave>(this);

        Snapshot->SaveVersion = 2;
        Snapshot->CheckpointId = Checkpoint;
        Snapshot->Inventory = Entries;

        SetStatus(TEXT("Snapshot created successfully."));
        return Snapshot;
        // LAB 1 END
    }
bool UBackpackState::ValidateAndMigrate(const UBackpackSave* Input,TArray<FInventoryEntry>& Out,FName& OutCheckpoint,FString& Error) const {
 // LAB 2 BEGIN
    if (!Input)
    {
        Error = TEXT("Rejected: save data is null.");
        return false;
    }
    if (Input->SaveVersion != 1 && Input->SaveVersion != 2)
    {
        Error = TEXT("Rejected: unsupported save version.");
        return false;
    }
    if (!ValidateCatalog(Error))
    {
        return false;
    }
    if (Input->Inventory.Num() > 2)
    {
        Error = TEXT("Rejected: too many inventory entries.");
        return false;
    }
    TArray<FInventoryEntry> Candidate;
    TSet<FName> Seen;
    for (const auto& Entry : Input->Inventory)
    {
        const UItemDefinition* Definition = FindDefinition(Entry.ItemId);
        if (!Definition || Seen.Contains(Entry.ItemId))
        {
            Error = TEXT("Rejected: unknown or duplicate item ID.");
            return false;
        }
        if (Entry.Quantity <= 0 || Entry.Quantity > Definition->MaxStack)
        {
            Error = TEXT("Rejected: invalid item quantity.");
            return false;
        }
        Seen.Add(Entry.ItemId);
        Candidate.Add(Entry);
    }
    FName CandidateCheckpoint = TEXT("Start");
    if (Input->SaveVersion == 2)
    {
        if (Input->CheckpointId != TEXT("Start") &&
            Input->CheckpointId != TEXT("LabDoor"))
        {
            Error = TEXT("Rejected: unknown checkpoint.");
            return false;
        }
        CandidateCheckpoint = Input->CheckpointId;
    }
    Out = MoveTemp(Candidate);
    OutCheckpoint = CandidateCheckpoint;
    return true;
 // LAB 2 END
}
bool UBackpackState::ApplySnapshot(const UBackpackSave* Input) {
 TArray<FInventoryEntry> Candidate;FName CandidateCheckpoint;FString Error;
 if(!ValidateAndMigrate(Input,Candidate,CandidateCheckpoint,Error)) {SetStatus(Error);return false;}
 Entries=MoveTemp(Candidate);Checkpoint=CandidateCheckpoint;OnInventoryChanged.Broadcast();
 SetStatus(Input->SaveVersion==1?TEXT("Version 1 migrated to 2; checkpoint defaults to Start."):TEXT("Loaded validated version 2 snapshot."));return true;
}
bool UBackpackState::Save() {
 // LAB 3 BEGIN
    if (bSaving)
    {
        SetStatus(TEXT("Save rejected: another write is pending."));
        return false;
    }
    PendingSave = BuildSnapshot();
    if (!PendingSave)
    {
        SetStatus(TEXT("Save failed: could not build snapshot."));
        return false;
    }
    TArray<FInventoryEntry> CheckedEntries;
    FName CheckedCheckpoint;
    FString Error;
    if (!ValidateAndMigrate(PendingSave, CheckedEntries, CheckedCheckpoint, Error))
    {
        PendingSave = nullptr;
        SetStatus(Error);
        return false;
    }
    if (UGameplayStatics::DoesSaveGameExist(Slot, 0))
    {
        UBackpackSave* Existing = Cast<UBackpackSave>(
            UGameplayStatics::LoadGameFromSlot(Slot, 0));
        if (!Existing ||
            !ValidateAndMigrate(Existing, CheckedEntries, CheckedCheckpoint, Error))
        {
            PendingSave = nullptr;
            SetStatus(TEXT("Save rejected: existing save is invalid or unsupported."));
            return false;
        }
    }
    bSaving = true;
    SetStatus(TEXT("Saving..."));

    UGameplayStatics::AsyncSaveGameToSlot(
        PendingSave,
        Slot,
        0,
        FAsyncSaveGameToSlotDelegate::CreateUObject(
            this, &UBackpackState::SaveFinished));
    return true;
 // LAB 3 END
}
void UBackpackState::SaveFinished(const FString& Name,int32 User,bool bSuccess) {
 bSaving=false;PendingSave=nullptr;
 SetStatus(bSuccess?TEXT("Save complete: bytes written. Now change memory, restart, and load."):TEXT("Save failed: disk write did not complete."));
 UE_LOG(LogTemp,Display,TEXT("WEEK07_SAVE %s %s"),*Name,bSuccess?TEXT("PASS"):TEXT("FAIL"));
 if(bVerifyWrite) {
  FFileHelper::SaveStringToFile(bSuccess?TEXT("PASS async write completed"):TEXT("FAIL async write"),*(FPaths::ProjectSavedDir()/TEXT("Week07_write.txt")));
  FPlatformMisc::RequestExitWithStatus(false,bSuccess?0:1);
 }
}
bool UBackpackState::Load() {
 // LAB 4 BEGIN
    if (bSaving)
    {
        SetStatus(TEXT("Load rejected: save is still in progress."));
        return false;
    }
    if (!UGameplayStatics::DoesSaveGameExist(Slot, 0))
    {
        Defaults();
        SetStatus(TEXT("No save found. Default inventory loaded."));
        return true;
    }
    USaveGame* LoadedObject = UGameplayStatics::LoadGameFromSlot(Slot, 0);
    UBackpackSave* LoadedSave = Cast<UBackpackSave>(LoadedObject);
    if (!LoadedSave)
    {
        SetStatus(TEXT("Load failed: invalid save file."));
        return false;
    }
    return ApplySnapshot(LoadedSave);
 // LAB 4 END
}
void UBackpackState::Fixture(int32 Kind) {
 auto* S=NewObject<UBackpackSave>(this);
 S->Inventory={FInventoryEntry(TEXT("Cell"),2),FInventoryEntry(TEXT("Key"),1)};
 S->CheckpointId=TEXT("LabDoor");
 if(Kind==1) S->SaveVersion=1;
 if(Kind==2) S->Inventory={FInventoryEntry(TEXT("Cell"),2),FInventoryEntry(TEXT("Ghost"),1)};
 if(Kind==3) S->SaveVersion=99;
 // Controlled in-memory objects: these buttons NEVER write fixtures to disk.
 ApplySnapshot(S);
}
void UBackpackState::Verify(const FString& Mode) {
 Slot=TEXT("Week07_AutomatedTest"); // Never touch the student's manual slot.
 if(Mode==TEXT("write")) {
  UGameplayStatics::DeleteGameInSlot(Slot,0);Defaults();AddItem(TEXT("Key"),1);SetCheckpoint(TEXT("LabDoor"));
  bVerifyWrite=true;if(!Save()) FPlatformMisc::RequestExitWithStatus(false,1);return;
 }
 int32 Failed=0;TArray<FString> Lines;
 auto Check=[&](bool Ok,const FString& Name) {Lines.Add((Ok?TEXT("PASS "):TEXT("FAIL "))+Name);if(!Ok) ++Failed;UE_LOG(LogTemp,Display,TEXT("WEEK07_CHECK %s %s"),Ok?TEXT("PASS"):TEXT("FAIL"),*Name);};
 auto Equal=[](const TArray<FInventoryEntry>& A,const TArray<FInventoryEntry>& B) {
  if(A.Num()!=B.Num()) return false;
  for(int32 I=0;I<A.Num();++I) if(A[I].ItemId!=B[I].ItemId || A[I].Quantity!=B[I].Quantity) return false;
  return true;
 };
 FString Error;Check(ValidateCatalog(Error),TEXT("two real data assets resolve with valid distinct IDs"));
 if(Mode==TEXT("baseline")) {
  Defaults();Check(AddItem(TEXT("Key"),1),TEXT("starter inventory API works"));
  Check(!AddItem(TEXT("Key"),1),TEXT("starter stack boundary works"));
  Check(Entries.Num()==2 && Checkpoint==TEXT("Start"),TEXT("starter default inventory and checkpoint usable"));
 } else if(Mode==TEXT("read")) {
  Defaults();Check(Load(),TEXT("load disk snapshot in a new process"));
  Check(Checkpoint==TEXT("LabDoor") && Entries.Num()==2 && Entries[0].ItemId==TEXT("Cell") && Entries[0].Quantity==4 && Entries[1].ItemId==TEXT("Key") && Entries[1].Quantity==1,TEXT("restart restores exact inventory and checkpoint"));
 } else {
  Defaults();auto Before=Entries;
  Check(!AddItem(TEXT("Cell"),2)&&Equal(Before,Entries),TEXT("atomic rejection Cell 4 plus 2 exceeds max 5"));
  Check(AddItem(TEXT("Cell"),1),TEXT("exact stack boundary Cell 4 plus 1 accepted"));
  Check(AddItem(TEXT("Key"),1),TEXT("second slot accepted"));Before=Entries;
  Check(!AddItem(TEXT("Ghost"),1)&&Equal(Before,Entries),TEXT("unknown item rejected unchanged"));
  Check(!AddItem(TEXT("Cell"),MAX_int32)&&Equal(Before,Entries),TEXT("overflow request rejected unchanged"));
  Check(!AddItem(TEXT("Key"),0)&&!AddItem(TEXT("Key"),-1)&&Equal(Before,Entries),TEXT("nonpositive additions rejected"));
  Check(RemoveItem(TEXT("Key"),1)&&Entries.Num()==1,TEXT("remove final unit removes stack"));
  Defaults();AddItem(TEXT("Key"),1);SetCheckpoint(TEXT("LabDoor"));auto* S=BuildSnapshot();
  Check(S && S->SaveVersion==2 && S->CheckpointId==TEXT("LabDoor") && Equal(S->Inventory,Entries),TEXT("snapshot stores complete versioned value state"));
  if(S) {RemoveItem(TEXT("Cell"),1);Check(S->Inventory[0].Quantity==4,TEXT("snapshot independent of later live changes"));Check(ApplySnapshot(S),TEXT("valid v2 commits"));}
  Before=Entries;FName Cp=Checkpoint;
  auto* Bad=NewObject<UBackpackSave>(this);Bad->Inventory={FInventoryEntry(TEXT("Cell"),1),FInventoryEntry(TEXT("Ghost"),1)};
  Check(!ApplySnapshot(Bad)&&Equal(Before,Entries)&&Checkpoint==Cp,TEXT("unknown ID does not partially commit"));
  Bad->Inventory={FInventoryEntry(TEXT("Cell"),1),FInventoryEntry(TEXT("Cell"),1)};
  Check(!ApplySnapshot(Bad)&&Equal(Before,Entries),TEXT("duplicate save IDs rejected"));
  for(int32 Q:{0,-1,6}) {Bad->Inventory={FInventoryEntry(TEXT("Cell"),Q)};Check(!ApplySnapshot(Bad)&&Equal(Before,Entries),FString::Printf(TEXT("invalid saved quantity %d"),Q));}
  Bad->Inventory={FInventoryEntry(TEXT("Cell"),2)};Bad->SaveVersion=1;Bad->CheckpointId=NAME_None;
  Check(ApplySnapshot(Bad)&&Checkpoint==TEXT("Start")&&Entries.Num()==1&&Entries[0].Quantity==2&&Bad->SaveVersion==1,TEXT("v1 migrates to Start without changing source object"));
  Before=Entries;Cp=Checkpoint;Bad->SaveVersion=99;
  Check(!ApplySnapshot(Bad)&&Equal(Before,Entries)&&Checkpoint==Cp,TEXT("future version rejected without mutation"));
  Check(!ApplySnapshot(nullptr)&&Equal(Before,Entries),TEXT("unreadable object rejected without mutation"));
  Bad->SaveVersion=2;Bad->CheckpointId=TEXT("MissingDoor");
  Check(!ApplySnapshot(Bad)&&Equal(Before,Entries),TEXT("unknown checkpoint rejected"));
  auto* SavedDef=Catalog[1].Get();Catalog[1]=Catalog[0];Check(!ValidateCatalog(Error),TEXT("duplicate catalog IDs rejected"));Catalog[1]=SavedDef;
  UGameplayStatics::DeleteGameInSlot(Slot,0);Check(Load()&&Checkpoint==TEXT("Start")&&Entries.Num()==1&&Entries[0].Quantity==4,TEXT("missing save produces valid defaults"));
  Bad->SaveVersion=99;UGameplayStatics::SaveGameToSlot(Bad,Slot,0);Before=Entries;Cp=Checkpoint;
  Check(!Load()&&Equal(Before,Entries)&&Checkpoint==Cp,TEXT("future version from disk leaves live state unchanged"));
  Check(!Save(),TEXT("future-format disk file protected from overwrite"));
  auto* Readback=Cast<UBackpackSave>(UGameplayStatics::LoadGameFromSlot(Slot,0));Check(Readback&&Readback->SaveVersion==99,TEXT("future file remains intact"));
  UGameplayStatics::DeleteGameInSlot(Slot,0);
  // Check the real async path and the same-frame overlap gate.
  Check(Save(),TEXT("async save accepted"));Check(!Save(),TEXT("overlapping write blocked"));Check(!Load(),TEXT("load blocked while write pending"));
 }
 const FString Report=FString::Join(Lines,TEXT("\n"))+FString::Printf(TEXT("\nFailures: %d\n"),Failed);
 FFileHelper::SaveStringToFile(Report,*(FPaths::ProjectSavedDir()/(TEXT("Week07_")+Mode+TEXT(".txt"))));
 FPlatformMisc::RequestExitWithStatus(false,Failed?1:0);
}
