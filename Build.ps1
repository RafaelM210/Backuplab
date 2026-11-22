param([Parameter(Mandatory=$true)][string]$EngineRoot)
$ErrorActionPreference='Stop'
& (Join-Path $EngineRoot 'Engine/Build/BatchFiles/Build.bat') BackpackLabEditor Win64 Development "-Project=$PSScriptRoot/BackpackLab.uproject" -WaitMutex -NoHotReloadFromIDE
if($LASTEXITCODE -ne 0){throw "Build failed: $LASTEXITCODE"}
