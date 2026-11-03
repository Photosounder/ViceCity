@echo off
setlocal
cd /d "%~dp0"
where py >nul 2>nul
if errorlevel 1 goto python
py -3 -c "import sys" >nul 2>nul
if errorlevel 1 goto python
py -3 utils\generate_vs2022.py
exit /b %errorlevel%
:python
python utils\generate_vs2022.py
exit /b %errorlevel%
