# Add license headers to Cebu source files
param(
    [string]$ProjectRoot = $PSScriptRoot
)

$LicenseHeader = @"
// Copyright (c) 2026 CeaserZhao (Oasis Company)
// SPDX-License-Identifier: MIT

"@

# Function to check if a file already has the license header
function Has-LicenseHeader {
    param([string]$FilePath)
    $content = Get-Content $FilePath -Raw
    return $content -match "^// Copyright \(c\) 2026 CeaserZhao \(Oasis Company\)"
}

# Function to add license header to a file
function Add-LicenseHeader {
    param([string]$FilePath)
    
    if (Has-LicenseHeader $FilePath) {
        Write-Host "Skipping (already has header): $FilePath"
        return
    }
    
    Write-Host "Adding header to: $FilePath"
    $content = Get-Content $FilePath -Raw
    $newContent = $LicenseHeader + $content
    Set-Content $FilePath -Value $newContent -NoNewline
}

# Process header files in include/cebu/
Write-Host "`nProcessing header files in include/cebu/..."
Get-ChildItem -Path "$ProjectRoot\include\cebu" -Filter "*.h" -File | ForEach-Object {
    Add-LicenseHeader $_.FullName
}

# Process source files in src/
Write-Host "`nProcessing source files in src/..."
Get-ChildItem -Path "$ProjectRoot\src" -Filter "*.cpp" -File | ForEach-Object {
    Add-LicenseHeader $_.FullName
}

# Process test files in tests/
Write-Host "`nProcessing test files in tests/..."
Get-ChildItem -Path "$ProjectRoot\tests" -Filter "*.cpp" -File | ForEach-Object {
    Add-LicenseHeader $_.FullName
}

# Process example files in examples/
Write-Host "`nProcessing example files in examples/..."
Get-ChildItem -Path "$ProjectRoot\examples" -Filter "*.cpp" -File | ForEach-Object {
    Add-LicenseHeader $_.FullName
}

Write-Host "`nDone!"
