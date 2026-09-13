$PROJECT = "TypingParo"

Write-Host "================================"
Write-Host " Building $PROJECT"
Write-Host "================================"

# Source files
$SRC = @(
    "src/main.c",
    "src/game.c",
    "src/enemy.c",
    "src/player.c",
    "src/word.c",
    "src/gunship.c",
    "src/leaderboard.c",
    "src/shooting.c"
)

# Compile
gcc $SRC `
    -o "$PROJECT.exe" `
    -Iinclude `
    -lraylib `
    -lopengl32 `
    -lgdi32 `
    -lwinmm

# Check whether compilation succeeded
if ($LASTEXITCODE -eq 0) {

    Write-Host ""
    Write-Host "================================"
    Write-Host " Build successful!"
    Write-Host "================================"
    Write-Host ""
    Write-Host "Starting $PROJECT.exe..."

    & ".\$PROJECT.exe"

} else {

    Write-Host ""
    Write-Host "================================"
    Write-Host " Build failed!"
    Write-Host "================================"

    exit 1
}
