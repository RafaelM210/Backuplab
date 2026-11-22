#pragma once
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Week07EditorLibrary.generated.h"
UCLASS()
class BACKPACKLABEDITOR_API UWeek07EditorLibrary : public UBlueprintFunctionLibrary {
 GENERATED_BODY()
public:
 UFUNCTION(BlueprintCallable, Category="Week 7|Editor") static bool BuildTeachingAssets();
 UFUNCTION(BlueprintCallable, Category="Week 7|Editor") static bool AuditTeachingAssets();
};
