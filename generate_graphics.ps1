param(
  [string]$OutputDir = "output"
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

Set-Location $PSScriptRoot

$binCandidates = @(
  ".",
  "build/vscode-msvc/Release",
  "build/vscode-msvc"
)
$BinDir = $null
foreach ($candidate in $binCandidates) {
  if ((Test-Path (Join-Path $candidate "mate_interact.exe")) -and
      (Test-Path (Join-Path $candidate "drawpullback.exe")) -and
      (Test-Path (Join-Path $candidate "draw_pb_sphere.exe"))) {
    $BinDir = $candidate
    break
  }
}

if (-not (Test-Path $OutputDir)) {
  New-Item -ItemType Directory -Path $OutputDir | Out-Null
}

if ($null -eq $BinDir) {
  throw "Missing required executables. Run VS Code task 'Build All (MSVC)' first."
}

function Run-Case {
  param(
    [int]$p1, [int]$q1, [int]$p2, [int]$q2,
    [int]$iters, [string]$tag
  )

  $paramFile = Join-Path $OutputDir "$tag.txt"
  $planeFile = Join-Path $OutputDir "$($tag)p.pdf"
  $sphereFile = Join-Path $OutputDir "$($tag)s.pdf"

  @"
$p1
$q1
$p2
$q2
$paramFile
$iters
q
"@ | & (Join-Path $BinDir "mate_interact.exe") | Out-File (Join-Path $OutputDir "$tag`_mate.log")

  @"
$planeFile
$paramFile
-3
3
-3
3
120
"@ | & (Join-Path $BinDir "drawpullback.exe") | Out-File (Join-Path $OutputDir "$tag`_plane.log")

  @"
$sphereFile
$paramFile
120
"@ | & (Join-Path $BinDir "draw_pb_sphere.exe") | Out-File (Join-Path $OutputDir "$tag`_sphere.log")

  Write-Host "Created $planeFile and $sphereFile"
}

Run-Case -p1 1 -q1 7 -p2 3 -q2 7 -iters 35 -tag "m17v37"
Run-Case -p1 1 -q1 5 -p2 2 -q2 7 -iters 35 -tag "m15v27"

Write-Host "Done. Graphics are in ./$OutputDir"
