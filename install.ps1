param([switch]$Uninstall)

$ErrorActionPreference = "Stop"
$dll = Join-Path (Split-Path -Parent $MyInvocation.MyCommand.Definition) "PinOverlay.dll"

if (-not (Test-Path $dll)) {
    Write-Error "PinOverlay.dll not found. Run build.bat first."
    exit 1
}

if ($Uninstall) {
    Write-Host "Unregistering overlay..."
    & regsvr32 /s /u $dll
} else {
    Write-Host "Registering overlay..."
    & regsvr32 /s $dll
}

Write-Host "Restarting Explorer..."
Stop-Process -Name explorer -Force -ErrorAction SilentlyContinue
Start-Sleep -Milliseconds 1000
Start-Process explorer

if ($Uninstall) {
    Write-Host "Done. Overlay removed."
} else {
    Write-Host "Done. Pinned files will now show a paperclip overlay icon."
    Write-Host "Note: overlay may take a moment to appear due to icon cache."
}
