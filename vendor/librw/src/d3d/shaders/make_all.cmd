@echo off

set "fxc_exe=C:\Program Files (x86)\Windows Kits\10\bin\10.0.19041.0\x64\fxc.exe"

"%fxc_exe%" /nologo /T vs_2_0 /Fh default_amb_VS.h default_VS.hlsl
"%fxc_exe%" /nologo /T vs_2_0 /DDIRECTIONALS /Fh default_amb_dir_VS.h default_VS.hlsl
"%fxc_exe%" /nologo /T vs_2_0 /DDIRECTIONALS /DPOINTLIGHTS /DSPOTLIGHTS /Fh default_all_VS.h default_VS.hlsl

"%fxc_exe%" /nologo /T ps_2_0 /Fh default_PS.h default_PS.hlsl
"%fxc_exe%" /nologo /T ps_2_0 /DTEX /Fh default_tex_PS.h default_PS.hlsl

"%fxc_exe%" /nologo /T vs_2_0 /Fh im2d_VS.h im2d_VS.hlsl
"%fxc_exe%" /nologo /T ps_2_0 /Fh im2d_PS.h im2d_PS.hlsl
"%fxc_exe%" /nologo /T ps_2_0 /DTEX /Fh im2d_tex_PS.h im2d_PS.hlsl



"%fxc_exe%" /nologo /T vs_2_0 /Fh matfx_env_amb_VS.h matfx_env_VS.hlsl
"%fxc_exe%" /nologo /T vs_2_0 /DDIRECTIONALS /Fh matfx_env_amb_dir_VS.h matfx_env_VS.hlsl
"%fxc_exe%" /nologo /T vs_2_0 /DDIRECTIONALS /DPOINTLIGHTS /DSPOTLIGHTS /Fh matfx_env_all_VS.h matfx_env_VS.hlsl

"%fxc_exe%" /nologo /T ps_2_0 /Fh matfx_env_PS.h matfx_env_PS.hlsl
"%fxc_exe%" /nologo /T ps_2_0 /DTEX /Fh matfx_env_tex_PS.h matfx_env_PS.hlsl



"%fxc_exe%" /nologo /T vs_2_0 /Fh skin_amb_VS.h skin_VS.hlsl
"%fxc_exe%" /nologo /T vs_2_0 /DDIRECTIONALS /Fh skin_amb_dir_VS.h skin_VS.hlsl
"%fxc_exe%" /nologo /T vs_2_0 /DDIRECTIONALS /DPOINTLIGHTS /DSPOTLIGHTS /Fh skin_all_VS.h skin_VS.hlsl

pause
