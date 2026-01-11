# Build script for 2048 SDL project
$ErrorActionPreference = "Stop"

Write-Host "Building 2048 project..." -ForegroundColor Green
Write-Host "Current directory: $PWD" -ForegroundColor Cyan

# Create build directory if it doesn't exist
if (-not (Test-Path "build")) {
    Write-Host "Creating build directory..." -ForegroundColor Yellow
    New-Item -ItemType Directory -Path "build" | Out-Null
}

# Change to build directory
Set-Location build

try {
    # Run cmake configuration
    Write-Host "Running CMake configuration..." -ForegroundColor Yellow
    cmake ..
    if ($LASTEXITCODE -ne 0) {
        throw "CMake configuration failed"
    }

    # Build the project
    Write-Host "Building project..." -ForegroundColor Yellow
    cmake --build .
    if ($LASTEXITCODE -ne 0) {
        throw "Build failed"
    }

    Write-Host "Build completed successfully!" -ForegroundColor Green
}
catch {
    Write-Host "ERROR: $_" -ForegroundColor Red
    Set-Location ../build
    exit 1
}
finally {
    Set-Location ..
}

