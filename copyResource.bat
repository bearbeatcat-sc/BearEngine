setlocal
    xcopy /e .\TestApp\BasicResources .\x64\Release\BasicResources
    xcopy /e .\TestApp\Resources .\x64\Release\Resources
    xcopy /e .\TestApp\BasicResources .\x64\Debug\BasicResources
    xcopy /e .\TestApp\Resources .\x64\Debug\Resources
    pause
endlocal