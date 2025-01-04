@echo off
:: Check if the Visual C++ Redistributable is installed
VC_redist.x86.exe /quiet /norestart
echo Installing Visual C++ Redistributable...
timeout /t 10
:: Run the game
MysteryMaze.exe
