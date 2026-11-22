param([Parameter(Mandatory=$true)][string]$EngineRoot)
& (Join-Path $EngineRoot 'Engine/Binaries/Win64/UnrealEditor.exe') "$PSScriptRoot/BackpackLab.uproject" /Game/Week07/L_BackpackLab
