#include "BackpackWidget.h"
#include "BackpackState.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
UBackpackState* UBackpackWidget::GetBackpackState() const {return GetGameInstance<UBackpackState>();}
void UBackpackWidget::NativeConstruct() {
 Super::NativeConstruct();
 if(auto* S=GetBackpackState()) {
  S->OnInventoryChanged.AddUniqueDynamic(this,&UBackpackWidget::RefreshDisplay);
  S->OnStatusChanged.AddUniqueDynamic(this,&UBackpackWidget::RefreshDisplay);
 }
 RefreshDisplay();
}
void UBackpackWidget::NativeDestruct() {
 if(auto* S=GetBackpackState()) {
  S->OnInventoryChanged.RemoveDynamic(this,&UBackpackWidget::RefreshDisplay);
  S->OnStatusChanged.RemoveDynamic(this,&UBackpackWidget::RefreshDisplay);
 }
 Super::NativeDestruct();
}
void UBackpackWidget::RefreshDisplay() {
 auto* S=GetBackpackState();if(!S)return;
 FString Items=TEXT("BACKPACK  /  TWO DISTINCT ITEM SLOTS\n");
 for(const auto& Entry:S->GetEntries()) {
  const auto* Def=S->FindDefinition(Entry.ItemId);
  Items+=FString::Printf(TEXT("%s   %d / %d    (ID: %s)\n"),Def?*Def->DisplayName.ToString():TEXT("Unknown"),Entry.Quantity,Def?Def->MaxStack:0,*Entry.ItemId.ToString());
 }
 if(S->GetEntries().IsEmpty())Items+=TEXT("Empty backpack\n");
 if(InventoryText)InventoryText->SetText(FText::FromString(Items));
 if(CheckpointText)CheckpointText->SetText(FText::FromString(TEXT("CHECKPOINT   ")+S->GetCheckpoint().ToString()+TEXT("     /     SAVE SCHEMA 2")));
 if(StatusText)StatusText->SetText(FText::FromString(S->Message));
 if(SaveButton)SaveButton->SetIsEnabled(!S->IsBusy());
 if(LoadButton)LoadButton->SetIsEnabled(!S->IsBusy());
}
