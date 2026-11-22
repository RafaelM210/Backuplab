param([Parameter(Mandatory=$true)][string]$EngineRoot,[switch]$Baseline)
$ErrorActionPreference='Stop'
$exe=Join-Path $EngineRoot 'Engine/Binaries/Win64/UnrealEditor-Cmd.exe'
$modes=if($Baseline){@('baseline')}else{@('rules','write','read','widget')}
foreach($mode in $modes) {
 $report=Join-Path $PSScriptRoot "Saved/Week07_$mode.txt"
 if(Test-Path $report){Remove-Item -LiteralPath $report}
 $proc=Start-Process -FilePath $exe -ArgumentList @('"'+$PSScriptRoot+'/BackpackLab.uproject"','/Game/Week07/L_BackpackLab','-game','-nullrhi','-unattended','-nosound',"-Week07Test=$mode",'"-abslog='+$PSScriptRoot+"/Saved/Verify_$mode.log"+'"') -WindowStyle Hidden -PassThru
 if(-not $proc.WaitForExit(300000)) {$proc.Kill();throw "Verification timed out: $mode"}
 $report=Join-Path $PSScriptRoot "Saved/Week07_$mode.txt"
 if($proc.ExitCode -ne 0 -or -not(Test-Path $report) -or (Get-Content $report -Raw) -match '(?m)^FAIL ') {throw "Verification failed: $mode. Read Saved/Verify_$mode.log"}
 Get-Content $report
}

