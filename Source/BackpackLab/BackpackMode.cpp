#include "BackpackMode.h"
#include "BackpackState.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "InputCoreTypes.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "TimerManager.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
ABackpackMode::ABackpackMode() {PlayerControllerClass=ABackpackController::StaticClass();HUDClass=ABackpackHUD::StaticClass();DefaultPawnClass=nullptr;}
void ABackpackMode::BeginPlay() {
 Super::BeginPlay();FString Mode;
 if(FParse::Value(FCommandLine::Get(),TEXT("Week07Test="),Mode)) {
  FTimerHandle Handle;GetWorldTimerManager().SetTimer(Handle,[this,Mode](){if(Mode==TEXT("widget")){if(auto* PC=Cast<ABackpackController>(GetWorld()->GetFirstPlayerController()))PC->VerifyDashboard();}else GetGameInstance<UBackpackState>()->Verify(Mode);},1.f,false);
 }
}
void ABackpackController::ShowDashboard() {
 if(DashboardWidget || !DashboardClass || !IsLocalController()) return;
 DashboardWidget=CreateWidget<UUserWidget>(this,DashboardClass);
 if(DashboardWidget) {
  DashboardWidget->AddToViewport();bShowMouseCursor=true;
  FInputModeGameAndUI Mode;Mode.SetHideCursorDuringCapture(false);SetInputMode(Mode);
 }
}
void ABackpackController::SetupInputComponent() {
 Super::SetupInputComponent();
 InputComponent->BindKey(EKeys::One,IE_Pressed,this,&ABackpackController::AddCell);
 InputComponent->BindKey(EKeys::Two,IE_Pressed,this,&ABackpackController::AddKey);
 InputComponent->BindKey(EKeys::Three,IE_Pressed,this,&ABackpackController::RemoveCell);
 InputComponent->BindKey(EKeys::R,IE_Pressed,this,&ABackpackController::Reset);
 InputComponent->BindKey(EKeys::C,IE_Pressed,this,&ABackpackController::Checkpoint);
 InputComponent->BindKey(EKeys::S,IE_Pressed,this,&ABackpackController::Save);
 InputComponent->BindKey(EKeys::L,IE_Pressed,this,&ABackpackController::Load);
 InputComponent->BindKey(EKeys::V,IE_Pressed,this,&ABackpackController::Legacy);
 InputComponent->BindKey(EKeys::I,IE_Pressed,this,&ABackpackController::Invalid);
 InputComponent->BindKey(EKeys::F,IE_Pressed,this,&ABackpackController::Future);
}
void ABackpackController::AddCell(){GetGameInstance<UBackpackState>()->AddItem(TEXT("Cell"),1);}
void ABackpackController::AddKey(){GetGameInstance<UBackpackState>()->AddItem(TEXT("Key"),1);}
void ABackpackController::RemoveCell(){GetGameInstance<UBackpackState>()->RemoveItem(TEXT("Cell"),1);}
void ABackpackController::Reset(){GetGameInstance<UBackpackState>()->Defaults();}
void ABackpackController::Checkpoint(){auto* B=GetGameInstance<UBackpackState>();B->SetCheckpoint(B->GetCheckpoint()==TEXT("Start")?FName(TEXT("LabDoor")):FName(TEXT("Start")));}
void ABackpackController::Save(){GetGameInstance<UBackpackState>()->Save();}
void ABackpackController::Load(){GetGameInstance<UBackpackState>()->Load();}
void ABackpackController::Legacy(){GetGameInstance<UBackpackState>()->Fixture(1);}
void ABackpackController::Invalid(){GetGameInstance<UBackpackState>()->Fixture(2);}
void ABackpackController::Future(){GetGameInstance<UBackpackState>()->Fixture(3);}
void ABackpackHUD::DrawHUD() {
 Super::DrawHUD();if(auto* PC=Cast<ABackpackController>(GetOwningPlayerController())) {if(PC->DashboardWidget)return;} auto* B=GetGameInstance<UBackpackState>();if(!Canvas||!B) return;
 const float Scale=FMath::Min(Canvas->SizeX/1100.f,Canvas->SizeY/720.f);
 const float Ox=(Canvas->SizeX-1100*Scale)/2,Oy=(Canvas->SizeY-720*Scale)/2;
 auto Box=[&](float X,float Y,float W,float H,FLinearColor Col){DrawRect(Col,Ox+X*Scale,Oy+Y*Scale,W*Scale,H*Scale);};
 auto Text=[&](const FString& S,float X,float Y,float Size,FLinearColor Col){DrawText(S,Col,Ox+X*Scale,Oy+Y*Scale,GEngine->GetMediumFont(),Size*Scale,false);};
 const FLinearColor Bg(.018f,.028f,.052f),Panel(.035f,.06f,.09f),Mint(.24f,.94f,.70f),White(.92f,.96f,1),Dim(.55f,.67f,.76f);
 DrawRect(Bg,0,0,Canvas->SizeX,Canvas->SizeY);
 Text(TEXT("FIELD STATION 07  /  INVENTORY + PERSISTENCE"),44,30,1.05f,Mint);
 Text(TEXT("Your backpack. Next session."),44,70,2.0f,White);
 Text(TEXT("Collect a little. Save a snapshot. Try to break the rules."),44,116,1.0f,Dim);
 Box(44,165,640,245,Panel);Box(704,165,352,245,Panel);
 Text(TEXT("BACKPACK  /  2 SLOTS"),64,184,1.0f,Dim);
 for(int32 I=0;I<2;++I){float X=64+I*306;
  Box(X,225,286,160,FLinearColor(.055f,.09f,.125f));
  if(B->GetEntries().IsValidIndex(I)){
   const auto& E=B->GetEntries()[I];const auto* D=B->FindDefinition(E.ItemId);
   Text(D?D->DisplayName.ToString():TEXT("UNKNOWN"),X+16,245,1.4f,White);
   Text(FString::Printf(TEXT("%d / %d"),E.Quantity,D?D->MaxStack:0),X+16,289,2.0f,Mint);
   Text(TEXT("ID  ")+E.ItemId.ToString(),X+16,348,.9f,Dim);
  }else Text(TEXT("Empty slot"),X+16,273,1.3f,Dim);
 }
 Text(TEXT("CHECKPOINT"),724,184,1.f,Dim);Text(B->GetCheckpoint().ToString(),724,228,1.65f,Mint);
 Text(TEXT("Schema 2  /  local slot"),724,290,1.f,White);Text(TEXT("C  toggle checkpoint"),724,337,1.f,Dim);
 Box(44,430,1012,70,Panel);Text(B->IsBusy()?TEXT("WRITE PENDING"):TEXT("RESULT"),60,440,.8f,Mint);
 Text(B->Message,60,463,.82f,White);
 Text(TEXT("1  + Cell     2  + Key     3  - Cell     R  reset memory"),44,528,1.1f,White);
 Text(TEXT("S  save       L  load      C  checkpoint"),44,565,1.1f,White);
 Text(TEXT("TEST OBJECTS   V  version 1     I  invalid item     F  future version"),44,612,1.f,Mint);
 Text(TEXT("R and test objects do not edit disk. For restart proof, close the app and relaunch, then L."),44,662,.9f,Dim);
}

void ABackpackController::VerifyDashboard() {
 auto* S=GetGameInstance<UBackpackState>();S->Slot=TEXT("Week07_WidgetTest");
 UGameplayStatics::DeleteGameInSlot(S->Slot,0);S->Defaults();
 auto Results=MakeShared<TArray<FString>>();auto Failures=MakeShared<int32>(0);
 auto Check=[Results,Failures](bool Ok,const FString& Label){Results->Add((Ok?TEXT("PASS "):TEXT("FAIL "))+Label);if(!Ok)++(*Failures);};
 auto Click=[this](const TCHAR* Name){if(!DashboardWidget)return false;auto* B=Cast<UButton>(DashboardWidget->GetWidgetFromName(Name));if(!B)return false;B->OnClicked.Broadcast();return true;};
 auto Text=[this](const TCHAR* Name){if(!DashboardWidget)return FString();auto* T=Cast<UTextBlock>(DashboardWidget->GetWidgetFromName(Name));return T?T->GetText().ToString():FString();};
 Check(DashboardWidget&&DashboardWidget->GetClass()->GetName()==TEXT("WBP_BackpackDashboard_C"),TEXT("Blueprint BeginPlay creates the real Widget Blueprint"));
 Check(Click(TEXT("AddKeyButton"))&&S->GetEntries().Num()==2,TEXT("OnClicked graph adds Key through Blueprint-callable API"));
 Check(Click(TEXT("CheckpointButton"))&&S->GetCheckpoint()==TEXT("LabDoor")&&Text(TEXT("CheckpointText")).Contains(TEXT("LabDoor")),TEXT("checkpoint button graph updates visible checkpoint text"));
 Check(Click(TEXT("AddCellButton"))&&S->GetEntries()[0].Quantity==5,TEXT("Cell button reaches stack limit"));
 Click(TEXT("AddCellButton"));Check(S->GetEntries()[0].Quantity==5&&Text(TEXT("StatusText")).Contains(TEXT("Rejected")),TEXT("rejected button operation updates status without mutating inventory"));
 Click(TEXT("ResetButton"));Check(S->GetEntries().Num()==1&&S->GetEntries()[0].Quantity==4&&S->GetCheckpoint()==TEXT("Start"),TEXT("Reset button restores defaults"));
 Click(TEXT("LegacyButton"));Check(S->GetEntries().Num()==2&&S->GetEntries()[0].Quantity==2&&S->GetCheckpoint()==TEXT("Start"),TEXT("version 1 button graph migrates"));
 Click(TEXT("InvalidButton"));Check(S->GetEntries()[0].Quantity==2&&Text(TEXT("StatusText")).Contains(TEXT("Invalid inventory")),TEXT("unknown item graph rejects atomically with visible explanation"));
 Click(TEXT("FutureButton"));Check(S->GetEntries()[0].Quantity==2&&Text(TEXT("StatusText")).Contains(TEXT("Unsupported")),TEXT("future-version graph rejects with visible explanation"));
 Click(TEXT("SaveButton"));
 auto* SaveButton=DashboardWidget?Cast<UButton>(DashboardWidget->GetWidgetFromName(TEXT("SaveButton"))):nullptr;
 auto* LoadButton=DashboardWidget?Cast<UButton>(DashboardWidget->GetWidgetFromName(TEXT("LoadButton"))):nullptr;
 Check(S->IsBusy()&&SaveButton&&!SaveButton->GetIsEnabled()&&LoadButton&&!LoadButton->GetIsEnabled(),TEXT("save graph immediately disables save/load widgets while pending"));
 FTimerHandle Handle;
 GetWorldTimerManager().SetTimer(Handle,[this,S,Check,Results,Failures](){
  auto ClickLater=[this](const TCHAR* Name){if(auto* B=DashboardWidget?Cast<UButton>(DashboardWidget->GetWidgetFromName(Name)):nullptr)B->OnClicked.Broadcast();};
  auto* Status=DashboardWidget?Cast<UTextBlock>(DashboardWidget->GetWidgetFromName(TEXT("StatusText"))):nullptr;
  Check(!S->IsBusy()&&Status&&Status->GetText().ToString().Contains(TEXT("Save complete")),TEXT("async completion updates Widget Blueprint status"));
  ClickLater(TEXT("RemoveCellButton"));ClickLater(TEXT("CheckpointButton"));
  Check(S->GetEntries()[0].Quantity==1&&S->GetCheckpoint()==TEXT("LabDoor"),TEXT("button graphs modify live state after save"));
  ClickLater(TEXT("LoadButton"));
  Check(S->GetEntries().Num()==2&&S->GetEntries()[0].Quantity==2&&S->GetEntries()[1].Quantity==1&&S->GetCheckpoint()==TEXT("Start"),TEXT("Load button graph restores saved inventory and checkpoint"));
  auto* SB=DashboardWidget?Cast<UButton>(DashboardWidget->GetWidgetFromName(TEXT("SaveButton"))):nullptr;
  Check(SB&&SB->GetIsEnabled(),TEXT("async completion re-enables save button"));
  FFileHelper::SaveStringToFile(FString::Join(*Results,TEXT("\n"))+FString::Printf(TEXT("\nFailures: %d\n"),*Failures),*(FPaths::ProjectSavedDir()/TEXT("Week07_widget.txt")));
  UGameplayStatics::DeleteGameInSlot(S->Slot,0);
  FPlatformMisc::RequestExitWithStatus(false,*Failures?1:0);
 },1.0f,false);
}
