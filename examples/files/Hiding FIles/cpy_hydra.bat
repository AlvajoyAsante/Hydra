:: Alvajoy "Alvajoy123" Asante
:: Last updated: 4/23/2023
:: This copies hydra folder to source code folder

@echo off

set API_NAME = Hydra

title Copying over Hydra
:start

:: Deletes the old version so it does not conflit with the new version
echo Deleting the old version of Hydra from testing folder...
rmdir /S/Q ".\src\hydra"

echo Cleaning folder directories for compiling ...
rmdir /S/Q ".\bin"
rmdir /S/Q ".\obj"

:: Clears the screen of any infomations displayed whiles deleting the folder.
cls

:: Makes a folder named notify and copies all the source code into the hyra folder
echo Copying the lastest version of %API_NAME% into testing folder...
mkdir .\src\hydra
xcopy /y/s/i "C:\Users\admin\Documents\TI Projects\2022\HYDRA\HYDRA_SOURCE\hydra" .\src\hydra


echo Finished copying the lastest version.