#include "Week07EditorLibrary.h"
#include "BackpackState.h"
#include "BackpackMode.h"
#include "BackpackWidget.h"
#include "WidgetBlueprint.h"
#include "WidgetBlueprintFactory.h"
#include "Blueprint/WidgetTree.h"
#include "Components/ScaleBox.h"
#include "Components/SizeBox.h"
#include "Components/Border.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "K2Node_ComponentBoundEvent.h"
#include "K2Node_CallFunction.h"
#include "K2Node_Event.h"
#include "EdGraphSchema_K2.h"
#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphPin.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "UObject/SavePackage.h"
#include "UObject/UnrealType.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "HAL/FileManager.h"
#include "Styling/CoreStyle.h"
#include "Engine/Font.h"

namespace Week07 {
static bool SaveAsset(UObject* Asset) {
 UPackage* P=Asset->GetOutermost();P->MarkPackageDirty();
 const FString File=FPackageName::LongPackageNameToFilename(P->GetName(),FPackageName::GetAssetPackageExtension());
 IFileManager::Get().MakeDirectory(*FPaths::GetPath(File),true);
 FSavePackageArgs A;A.TopLevelFlags=RF_Public|RF_Standalone;A.SaveFlags=SAVE_NoError;
 return UPackage::SavePackage(P,Asset,*File,A);
}
static UBlueprint* MakeBP(const FString& Name,UClass* Parent) {
 const FString Path=TEXT("/Game/Week07/Blueprints/")+Name;
 if(auto* Existing=LoadObject<UBlueprint>(nullptr,*(Path+TEXT(".")+Name)))return Existing;
 auto* BP=FKismetEditorUtilities::CreateBlueprint(Parent,CreatePackage(*Path),FName(*Name),BPTYPE_Normal,UBlueprint::StaticClass(),UBlueprintGeneratedClass::StaticClass(),TEXT("Week07"));
 FAssetRegistryModule::AssetCreated(BP);return BP;
}
static UK2Node_CallFunction* Call(UEdGraph* Graph,UFunction* Fn,int32 X,int32 Y) {
 auto* N=NewObject<UK2Node_CallFunction>(Graph);Graph->AddNode(N,false,false);
 N->CreateNewGuid();N->SetFromFunction(Fn);N->NodePosX=X;N->NodePosY=Y;N->AllocateDefaultPins();return N;
}
static void Link(UEdGraphPin* A,UEdGraphPin* B) {
 check(A&&B);check(GetDefault<UEdGraphSchema_K2>()->TryCreateConnection(A,B));
}
static UTextBlock* Label(UWidgetTree* Tree,FName Name,const FString& Value,int32 Size,FLinearColor Color) {
 auto* T=Tree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(),Name);
 T->SetText(FText::FromString(Value));T->SetFont(FCoreStyle::GetDefaultFontStyle(TEXT("Regular"),Size));
 T->SetColorAndOpacity(FSlateColor(Color));T->SetAutoWrapText(true);T->bIsVariable=true;return T;
}
static void Add(UVerticalBox* Box,UWidget* Widget,float Bottom=14) {
 auto* Slot=Box->AddChildToVerticalBox(Widget);Slot->SetPadding(FMargin(0,0,0,Bottom));
}
struct FButtonSpec {const TCHAR* Name;const TCHAR* Label;const TCHAR* Function;const TCHAR* Id;int32 Count;};
static const FButtonSpec Buttons[]={
 {TEXT("AddCellButton"),TEXT("+ Cell  [1]"),TEXT("AddItem"),TEXT("Cell"),1},
 {TEXT("AddKeyButton"),TEXT("+ Key  [2]"),TEXT("AddItem"),TEXT("Key"),1},
 {TEXT("RemoveCellButton"),TEXT("- Cell  [3]"),TEXT("RemoveItem"),TEXT("Cell"),1},
 {TEXT("ResetButton"),TEXT("Reset  [R]"),TEXT("Defaults"),TEXT(""),0},
 {TEXT("CheckpointButton"),TEXT("Checkpoint  [C]"),TEXT("ToggleCheckpoint"),TEXT(""),0},
 {TEXT("SaveButton"),TEXT("Save  [S]"),TEXT("Save"),TEXT(""),0},
 {TEXT("LoadButton"),TEXT("Load  [L]"),TEXT("Load"),TEXT(""),0},
 {TEXT("LegacyButton"),TEXT("Version 1  [V]"),TEXT("Fixture"),TEXT(""),1},
 {TEXT("InvalidButton"),TEXT("Unknown  [I]"),TEXT("Fixture"),TEXT(""),2},
 {TEXT("FutureButton"),TEXT("Future v99  [F]"),TEXT("Fixture"),TEXT(""),3}
};
}

bool UWeek07EditorLibrary::BuildTeachingAssets() {
 using namespace Week07;
 const FString WidgetPath=TEXT("/Game/Week07/Blueprints/WBP_BackpackDashboard");
 auto* WBP=LoadObject<UWidgetBlueprint>(nullptr,*(WidgetPath+TEXT(".WBP_BackpackDashboard")));
 if(!WBP) {
  auto* Factory=NewObject<UWidgetBlueprintFactory>();Factory->ParentClass=UBackpackWidget::StaticClass();
  WBP=CastChecked<UWidgetBlueprint>(Factory->FactoryCreateNew(UWidgetBlueprint::StaticClass(),CreatePackage(*WidgetPath),TEXT("WBP_BackpackDashboard"),RF_Public|RF_Standalone,nullptr,GWarn));
  FAssetRegistryModule::AssetCreated(WBP);
  auto* Tree=WBP->WidgetTree.Get();
  auto* Scale=Tree->ConstructWidget<UScaleBox>(UScaleBox::StaticClass(),TEXT("ResponsiveScale"));
  Scale->SetStretch(EStretch::ScaleToFit);Tree->RootWidget=Scale;
  auto* Size=Tree->ConstructWidget<USizeBox>(USizeBox::StaticClass(),TEXT("DesignSize"));Size->SetWidthOverride(1100);Size->SetHeightOverride(700);Scale->AddChild(Size);
  auto* BG=Tree->ConstructWidget<UBorder>(UBorder::StaticClass(),TEXT("Background"));
  BG->SetBrushColor(FLinearColor(.025,.055,.075,1));BG->SetPadding(FMargin(30));Size->AddChild(BG);
  auto* Main=Tree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(),TEXT("Layout"));BG->AddChild(Main);
  const FLinearColor White(.94,.97,1),Mint(.3,.95,.72),Muted(.65,.77,.81);
  Add(Main,Label(Tree,TEXT("Title"),TEXT("WEEK 7  /  THE BACKPACK LAB"),34,Mint));
  Add(Main,Label(Tree,TEXT("Subtitle"),TEXT("Editable Widget Blueprint. Button events call the shared inventory API."),20,Muted),25);
  Add(Main,Label(Tree,TEXT("InventoryText"),TEXT("BACKPACK\nEnergy cell   4 / 5\nLab key   1 / 1"),27,White),20);
  Add(Main,Label(Tree,TEXT("CheckpointText"),TEXT("CHECKPOINT   Start     /     SAVE SCHEMA 2"),25,Mint),20);
  auto* Status=Label(Tree,TEXT("StatusText"),TEXT("Ready. Load is explicit after a restart."),22,White);
  auto* StatusBox=Tree->ConstructWidget<USizeBox>(USizeBox::StaticClass(),TEXT("StatusArea"));StatusBox->SetHeightOverride(88);StatusBox->AddChild(Status);Add(Main,StatusBox,15);
  for(int32 Row=0;Row<2;++Row) {
   auto* H=Tree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(),FName(*FString::Printf(TEXT("ButtonRow%d"),Row)));
   for(int32 Col=0;Col<5;++Col) {
    int32 I=Row*5+Col;const auto& Spec=Buttons[I];
    auto* Button=Tree->ConstructWidget<UButton>(UButton::StaticClass(),Spec.Name);Button->bIsVariable=true;if(auto* Focus=FindFProperty<FBoolProperty>(UButton::StaticClass(),TEXT("IsFocusable")))Focus->SetPropertyValue_InContainer(Button,false);
    Button->SetBackgroundColor(FLinearColor(.11,.28,.32));
    Button->AddChild(Label(Tree,FName(*FString::Printf(TEXT("ButtonLabel%d"),I)),Spec.Label,19,White));
    auto* Slot=H->AddChildToHorizontalBox(Button);Slot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));Slot->SetPadding(FMargin(0,0,9,0));
   }
   auto* Height=Tree->ConstructWidget<USizeBox>(USizeBox::StaticClass(),FName(*FString::Printf(TEXT("RowHeight%d"),Row)));Height->SetHeightOverride(55);Height->AddChild(H);Add(Main,Height,12);
  }
  Add(Main,Label(Tree,TEXT("Hint"),TEXT("R resets memory only. V, I and F test objects never edit your manual save.\nFor restart proof: Save, wait for success, close, relaunch, then Load."),18,Muted),0);
  FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(WBP);
  FKismetEditorUtilities::CompileBlueprint(WBP);
  auto* Graph=FBlueprintEditorUtils::FindEventGraph(WBP);
  if(!Graph) {
   Graph=FBlueprintEditorUtils::CreateNewGraph(WBP,TEXT("EventGraph"),UEdGraph::StaticClass(),UEdGraphSchema_K2::StaticClass());
   FBlueprintEditorUtils::AddUbergraphPage(WBP,Graph);
  }
  for(int32 I=0;I<UE_ARRAY_COUNT(Buttons);++I) {
   const auto& Spec=Buttons[I];const int32 Y=I*270;
   auto* Prop=FindFProperty<FObjectProperty>(WBP->SkeletonGeneratedClass,Spec.Name);check(Prop);
   auto* Delegate=FindFProperty<FMulticastDelegateProperty>(UButton::StaticClass(),TEXT("OnClicked"));check(Delegate);
   auto* Event=NewObject<UK2Node_ComponentBoundEvent>(Graph);Graph->AddNode(Event,false,false);Event->CreateNewGuid();
   Event->InitializeComponentBoundEventParams(Prop,Delegate);Event->NodePosX=0;Event->NodePosY=Y;Event->AllocateDefaultPins();
   auto* State=Call(Graph,UBackpackWidget::StaticClass()->FindFunctionByName(TEXT("GetBackpackState")),280,Y+100);
   auto* Op=Call(Graph,UBackpackState::StaticClass()->FindFunctionByName(Spec.Function),640,Y);
   Link(Event->FindPin(UEdGraphSchema_K2::PN_Then),Op->FindPin(UEdGraphSchema_K2::PN_Execute));
   Link(State->GetReturnValuePin(),Op->FindPin(UEdGraphSchema_K2::PN_Self));
   if(auto* Pin=Op->FindPin(TEXT("Id")))Pin->DefaultValue=Spec.Id;
   if(auto* Pin=Op->FindPin(TEXT("Count")))Pin->DefaultValue=FString::FromInt(Spec.Count);
   if(auto* Pin=Op->FindPin(TEXT("Kind")))Pin->DefaultValue=FString::FromInt(Spec.Count);
  }
 }
 auto* FontAsset=LoadObject<UFont>(nullptr,TEXT("/Engine/EngineFonts/Roboto.Roboto"));
 check(FontAsset);
 WBP->WidgetTree->ForEachWidget([FontAsset](UWidget* W){if(auto* T=Cast<UTextBlock>(W)){T->SetFont(FSlateFontInfo(FontAsset,T->GetFont().Size,TEXT("Regular")));if(T->GetFName()==TEXT("ButtonLabel8"))T->SetText(FText::FromString(TEXT("Unknown  [I]")));}});
 FKismetEditorUtilities::CompileBlueprint(WBP);
 if(WBP->Status==BS_Error)return false;
 if(!SaveAsset(WBP))return false;
 auto* Controller=MakeBP(TEXT("BP_BackpackController"),ABackpackController::StaticClass());
 auto* G=FBlueprintEditorUtils::FindEventGraph(Controller);check(G);
 UK2Node_Event* Begin=nullptr;
 for(const auto& N:G->Nodes)if(auto* E=Cast<UK2Node_Event>(N.Get()))if(E->EventReference.GetMemberName()==TEXT("ReceiveBeginPlay"))Begin=E;
 if(!Begin) {
  Begin=NewObject<UK2Node_Event>(G);G->AddNode(Begin,false,false);Begin->CreateNewGuid();
  Begin->EventReference.SetExternalMember(TEXT("ReceiveBeginPlay"),AActor::StaticClass());Begin->bOverrideFunction=true;Begin->NodePosX=0;Begin->NodePosY=0;Begin->AllocateDefaultPins();
 }
 if(Begin->FindPin(UEdGraphSchema_K2::PN_Then)->LinkedTo.IsEmpty()) {
  auto* Show=Call(G,ABackpackController::StaticClass()->FindFunctionByName(TEXT("ShowDashboard")),350,0);
  Link(Begin->FindPin(UEdGraphSchema_K2::PN_Then),Show->FindPin(UEdGraphSchema_K2::PN_Execute));
 }
 FKismetEditorUtilities::CompileBlueprint(Controller);
 CastChecked<ABackpackController>(Controller->GeneratedClass->GetDefaultObject())->DashboardClass=WBP->GeneratedClass;
 if(!SaveAsset(Controller))return false;
 auto* Mode=MakeBP(TEXT("BP_BackpackMode"),ABackpackMode::StaticClass());FKismetEditorUtilities::CompileBlueprint(Mode);
 CastChecked<ABackpackMode>(Mode->GeneratedClass->GetDefaultObject())->PlayerControllerClass=Controller->GeneratedClass;
 if(!SaveAsset(Mode))return false;
 auto* State=MakeBP(TEXT("BP_BackpackState"),UBackpackState::StaticClass());FKismetEditorUtilities::CompileBlueprint(State);if(!SaveAsset(State))return false;
 auto* Save=MakeBP(TEXT("BP_BackpackSave"),UBackpackSave::StaticClass());FKismetEditorUtilities::CompileBlueprint(Save);if(!SaveAsset(Save))return false;
 auto* StateDefaults=CastChecked<UBackpackState>(State->GeneratedClass->GetDefaultObject());
 StateDefaults->SaveClass=Save->GeneratedClass;
 StateDefaults->Catalog={LoadObject<UItemDefinition>(nullptr,TEXT("/Game/Week07/DA_Item_Cell.DA_Item_Cell")),LoadObject<UItemDefinition>(nullptr,TEXT("/Game/Week07/DA_Item_Key.DA_Item_Key"))};
 if(!SaveAsset(State))return false;
 UE_LOG(LogTemp,Display,TEXT("WEEK07_CREATED_WIDGET_AND_4_BLUEPRINTS"));return true;
}

bool UWeek07EditorLibrary::AuditTeachingAssets() {
 using namespace Week07;
 bool Ok=true;TArray<FString> Report;
 auto Check=[&](bool Pass,const FString& Label){Ok&=Pass;Report.Add((Pass?TEXT("PASS "):TEXT("FAIL "))+Label);};
 const TArray<FString> Names={TEXT("WBP_BackpackDashboard"),TEXT("BP_BackpackController"),TEXT("BP_BackpackMode"),TEXT("BP_BackpackState"),TEXT("BP_BackpackSave")};
 for(const auto& Name:Names) {
  const FString Path=TEXT("/Game/Week07/Blueprints/")+Name+TEXT(".")+Name;
  auto* BP=LoadObject<UBlueprint>(nullptr,*Path);Check(BP!=nullptr,Name+TEXT(" exists"));
  if(BP){FKismetEditorUtilities::CompileBlueprint(BP);Check(BP->Status!=BS_Error,Name+TEXT(" compiles without Blueprint errors"));}
 }
 auto* WBP=LoadObject<UWidgetBlueprint>(nullptr,TEXT("/Game/Week07/Blueprints/WBP_BackpackDashboard.WBP_BackpackDashboard"));
 if(WBP) {
  int32 Events=0;auto* G=FBlueprintEditorUtils::FindEventGraph(WBP);
  if(G)for(const auto& N:G->Nodes)if(Cast<UK2Node_ComponentBoundEvent>(N.Get()))++Events;
  Check(Events==10,TEXT("ten inspectable OnClicked event graphs"));
  for(const auto& B:Buttons)Check(Cast<UButton>(WBP->WidgetTree->FindWidget(B.Name))!=nullptr,FString(B.Name)+TEXT(" exists in Designer"));
 }
 auto* C=LoadClass<ABackpackController>(nullptr,TEXT("/Game/Week07/Blueprints/BP_BackpackController.BP_BackpackController_C"));
 Check(C&&Cast<ABackpackController>(C->GetDefaultObject())->DashboardClass!=nullptr,TEXT("controller assigns Widget Blueprint"));
 auto* M=LoadClass<ABackpackMode>(nullptr,TEXT("/Game/Week07/Blueprints/BP_BackpackMode.BP_BackpackMode_C"));
 Check(M&&Cast<ABackpackMode>(M->GetDefaultObject())->PlayerControllerClass==C,TEXT("GameMode assigns Blueprint controller"));
 const FString Out=FString::Join(Report,TEXT("\n"))+TEXT("\n");
 FFileHelper::SaveStringToFile(Out,*(FPaths::ProjectSavedDir()/TEXT("Week07_blueprint_assets.txt")));
 UE_LOG(LogTemp,Display,TEXT("%s"),*Out);return Ok;
}
