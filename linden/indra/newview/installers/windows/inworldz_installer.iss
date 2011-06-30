; InWorldz inno setup installer script by McCabe Maxsted
; This script only works with VS2005, currently

[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)

; These will change
AppId={{DC6CCE02-BC61-43B1-B4CA-292C6BCCCB34}
AppName=InWorldz Viewer
AppVerName=InWorldz Viewer 1.2.7.3
DefaultDirName={pf}\InWorldz
DefaultGroupName=InWorldz
VersionInfoProductName=InWorldz Viewer
OutputBaseFilename=InWorldz-1.2.7.3
VersionInfoVersion=1.2.7.3
VersionInfoTextVersion=1.2.7.3
VersionInfoProductVersion=1.2.7.3
AppVersion=1.2.7.3
VersionInfoCopyright=2011

; These won't change
VersionInfoCompany=InWorldz, LLC
AppPublisher=InWorldz, LLC
AppPublisherURL=http://inworldz.com
AppSupportURL=http://inworldz.com
AllowNoIcons=true
InfoAfterFile=..\windows\README.txt
OutputDir=C:\inworldz_installers
SetupIconFile=..\windows\install_icon.ico
Compression=lzma2/ultra64
InternalCompressLevel=ultra64
SolidCompression=true
;PrivilegesRequired=poweruser
AllowRootDirectory=true
WizardImageFile=..\windows\installer_icon_left.bmp
WizardSmallImageFile=..\windows\installer_icon_right.bmp
SetupLogging=true
RestartIfNeededByRun=false
AlwaysRestart=false

[Languages]
Name: english; MessagesFile: compiler:Default.isl

[Tasks]
Name: desktopicon; Description: {cm:CreateDesktopIcon}; GroupDescription: {cm:AdditionalIcons}; Flags: checkedonce
Name: quicklaunchicon; Description: {cm:CreateQuickLaunchIcon}; GroupDescription: {cm:AdditionalIcons}; Flags: checkedonce
; TODO: use scripting for something like this on uninstall:
; Name: uninstallsettings; Description: Remove user settings; Flags: checkablealone; Languages: ; GroupDescription: Uninstall:

; NOTE VS2005 is currently the only version supported anywhere in the packaging system, so we can do this here
[Files]
Source: ..\..\..\build-vc80\newview\release\packaged\inworldz.exe; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\packaged\character\*; DestDir: {app}\character; Flags: ignoreversion recursesubdirs createallsubdirs
Source: ..\..\..\build-vc80\newview\release\packaged\fonts\*; DestDir: {app}\fonts; Flags: ignoreversion recursesubdirs createallsubdirs
Source: ..\..\..\build-vc80\newview\release\packaged\app_settings\*; DestDir: {app}\app_settings; Flags: ignoreversion recursesubdirs createallsubdirs
Source: ..\..\..\build-vc80\newview\release\packaged\skins\*; DestDir: {app}\skins; Flags: ignoreversion recursesubdirs createallsubdirs
Source: ..\..\..\build-vc80\newview\release\packaged\llplugin\*; DestDir: {app}\llplugin; Flags: ignoreversion recursesubdirs createallsubdirs
Source: ..\..\..\build-vc80\newview\release\packaged\alut.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\packaged\dbghelp.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\packaged\libapr-1.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\packaged\libapriconv-1.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\packaged\libaprutil-1.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\packaged\licenses.txt; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\packaged\llcommon.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\packaged\featuretable.txt; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\packaged\gpu_table.txt; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\packaged\kdu_v64R.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\packaged\kdu_v64R.dll.config; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\packaged\openal32.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\packaged\OpenJPEG.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\packaged\SLPlugin.exe; DestDir: {app}; Flags: ignoreversion

; Gstreamer-specific files below
Source: ..\..\..\build-vc80\newview\release\packaged\lib\*; DestDir: {app}\lib; Flags: ignoreversion recursesubdirs createallsubdirs
Source: ..\..\..\build-vc80\newview\release\packaged\avcodec-gpl-52.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\packaged\avdevice-gpl-52.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\packaged\avfilter-gpl-1.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\packaged\avformat-gpl-52.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\packaged\avutil-gpl-50.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\packaged\iconv.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\packaged\liba52-0.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\packaged\libbz2.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\packaged\libcelt-0.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\packaged\libdca-0.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\packaged\libexpat-1.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\packaged\libfaad-2.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\packaged\libFLAC-8.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\packaged\libgcrypt-11.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\packaged\libgio-2.0-0.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\packaged\libglib-2.0-0.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\packaged\libgmodule-2.0-0.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\packaged\libgnutls-26.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\packaged\libgobject-2.0-0.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\packaged\libgpg-error-0.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\packaged\libgstapp-0.10.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\packaged\libgstaudio-0.10.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\packaged\libgstbase-0.10.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\packaged\libgstcontroller-0.10.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\packaged\libgstdataprotocol-0.10.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\packaged\libgstfft-0.10.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\packaged\libgstinterfaces-0.10.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\packaged\libgstnet-0.10.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\packaged\libgstnetbuffer-0.10.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\packaged\libgstpbutils-0.10.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\packaged\libgstphotography-0.10.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\packaged\libgstreamer-0.10.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\packaged\libgstriff-0.10.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\packaged\libgstrtp-0.10.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\packaged\libgstrtsp-0.10.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\packaged\libgstsdp-0.10.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\packaged\libgstsignalprocessor-0.10.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\packaged\libgsttag-0.10.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\packaged\libgstvideo-0.10.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\packaged\libgthread-2.0-0.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\packaged\libmms-0.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\packaged\libmpeg2-0.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\packaged\libneon-27.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\packaged\libogg-0.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\packaged\liboil-0.3-0.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\packaged\libsoup-2.4-1.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\packaged\libtasn1-3.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\packaged\libtheora-0.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\packaged\libtheoradec-1.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\packaged\libvorbis-0.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\packaged\libvorbisenc-2.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\packaged\libvorbisfile-3.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\packaged\libwavpack-1.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\packaged\libx264-67.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\packaged\libxml2-2.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\packaged\libxml2.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\packaged\SDL.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\packaged\xvidcore.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\packaged\z.dll; DestDir: {app}; Flags: ignoreversion

; VC++ 2005 SP1 x86 and VC++ 2010 SP1 x86 redist
; TODO: add checking for VS2005. See http://blogs.msdn.com/b/astebner/archive/2007/01/16/mailbag-how-to-detect-the-presence-of-the-vc-8-0-runtime-redistributable-package.aspx
Source: ..\windows\vcredist_x86_VS2005_SP1.exe; DestDir: {tmp}; DestName: vcredist_x86_VS2005_SP1.exe
Source: ..\windows\vcredist_x86_VS2010_SP1.exe; DestDir: {tmp}; DestName: vcredist_x86_VS2010_SP1.exe

; Old files we don't use anymore:
; Source: ..\..\..\build-vc80\newview\release\packaged\dronesettings.xml; DestDir: {app}; Flags: ignoreversion
; Source: ..\..\..\build-vc80\newview\release\packaged\volume_settings.xml; DestDir: {app}; Flags: ignoreversion
; Source: ..\..\..\build-vc80\newview\release\packaged\srtp.dll; DestDir: {app}; Flags: ignoreversion
; Source: ..\..\..\build-vc80\newview\release\packaged\ssleay32.dll; DestDir: {app}; Flags: ignoreversion
; Source: ..\..\..\build-vc80\newview\release\packaged\tntk.dll; DestDir: {app}; Flags: ignoreversion
; Source: ..\..\..\build-vc80\newview\release\packaged\libeay32.dll; DestDir: {app}; Flags: ignoreversion
; Source: ..\..\..\build-vc80\newview\release\packaged\lsl_guide.html; DestDir: {app}; Flags: ignoreversion

; NOTE: Don't use "Flags: ignoreversion" on any shared system files
;Source: ..\..\..\build-vc80\newview\release\packaged\msvcr71.dll; DestDir: {app}; Flags: ignoreversion; MinVersion: 0,6.01; Tasks: ; Languages:

[Registry]
Root: HKCR; Subkey: inworldz; ValueType: string; Flags: uninsdeletekey deletekey; ValueName: (default); ValueData: URL:InWorldz
Root: HKCR; Subkey: inworldz; ValueType: string; Flags: uninsdeletekey deletekey; ValueName: URL Protocol
Root: HKCR; Subkey: inworldz\DefaultIcon; Flags: uninsdeletekey deletekey; ValueType: string; ValueData: {app}\inworldz.exe
Root: HKCR; Subkey: inworldz\shell\open\command; ValueType: expandsz; Flags: uninsdeletekey deletekey; ValueData: "{app}\inworldz.exe -url ""%1"""; Languages:
; Root: HKCU; Subkey: Environment; ValueType: string; ValueName: GST_PLUGIN_PATH; Flags: deletevalue uninsdeletevalue; ValueData: {app}\lib
; Root: HKCU; Subkey: Environment; ValueType: expandsz; ValueName: PATH; ValueData: {app}

[Icons]
Name: {group}\{cm:UninstallProgram,InWorldz}; Filename: {uninstallexe}
Name: {commondesktop}\InWorldz; Filename: {app}\inworldz.exe; Tasks: desktopicon; WorkingDir: {app}; IconIndex: 0
Name: {userappdata}\Microsoft\Internet Explorer\Quick Launch\InWorldz; Filename: {app}\inworldz.exe; Tasks: quicklaunchicon; WorkingDir: {app}
Name: {group}\InWorldz; Filename: {app}\inworldz.exe; WorkingDir: {app}; Comment: inworldz; IconIndex: 0;

[Run]
Filename: {tmp}\vcredist_x86_VS2005_SP1.exe; Parameters: "/q:a /c:""VCREDI~1.EXE /q:a /c:""""msiexec /i vcredist.msi /qn"""" """; Flags: runhidden
Filename: {tmp}\vcredist_x86_VS2010_SP1.exe; Parameters: "/q /norestart"; Check: Needs2010Redist; Flags: runhidden
Filename: {app}\inworldz.exe; WorkingDir: {app}; Flags: nowait postinstall

[UninstallDelete]
Name: {userappdata}\InWorldz\user_settings\password.dat; Type: files; Languages:
Name: {userappdata}\InWorldz\user_settings\settings.xml; Type: files; Languages:
; 1.1 and lower cache location:
Name: {userappdata}\InWorldz\cache; Type: filesandordirs
; 1.2 and higher cache location:
Name: {localappdata}\InWorldz\cache; Type: filesandordirs
Name: {userappdata}\InWorldz\logs; Type: filesandordirs
Name: {userappdata}\InWorldz\browser_profile; Type: filesandordirs
Name: C:\Users\{username}\.gstreamer-0.10; Type: filesandordirs
Name: C:\Documents and Settings\{username}\.gstreamer-0.10; Type: filesandordirs

[InstallDelete]
; Name: {app}\*.dll; Type: files; Tasks: ; Languages:
; ALWAYS delete the plugins! Beware if you don't
Name: {app}\lib\gstreamer-plugins\*; Type: filesandordirs; Tasks: ; Languages: 
; Name: {app}\skins\default\xui\*; Type: filesandordirs; Tasks: ; Languages:
Name: {app}\skins\silver\xui\*; Type: filesandordirs; Tasks: ; Languages:
Name: C:\Documents and Settings\{username}\.gstreamer-0.10\*; Type: filesandordirs
Name: C:\Users\{username}\.gstreamer-0.10\*; Type: filesandordirs
; Breaks the browser if installing on top of 1.1:
Name: {app}\gksvggdiplus.dll; Type: files; Tasks: ; Languages:
; Breaks inworld audio if it's from an old version with a different GUID
Name: {app}\alut.dll; Type: files; Tasks: ; Languages:

; Old plugin files we want to kill:
Name: {app}\charset.dll; Type: files; Tasks: ; Languages:
Name: {app}\freebl3.dll; Type: files; Tasks: ; Languages:
Name: {app}\glew32.dll; Type: files; Tasks: ; Languages:
Name: {app}\iconv.dll; Type: files; Tasks: ; Languages:
Name: {app}\intl.dll; Type: files; Tasks: ; Languages:
Name: {app}\InWorldzViewer.exe; Type: files; Tasks: ; Languages:
Name: {app}\js3250.dll; Type: files; Tasks: ; Languages:
Name: {app}\libcairo-2.dll; Type: files; Tasks: ; Languages:
Name: {app}\libfaad-2.dll; Type: files; Tasks: ; Languages:
Name: {app}\libgcrypt-11.dll; Type: files; Tasks: ; Languages:
Name: {app}\libgio-2.0-0.dll; Type: files; Tasks: ; Languages:
Name: {app}\libglib-2.0-0.dll; Type: files; Tasks: ; Languages:
Name: {app}\libgmodule-2.0-0.dll; Type: files; Tasks: ; Languages:
Name: {app}\libgnutls-26.dll; Type: files; Tasks: ; Languages:
Name: {app}\libgobject-2.0-0.dll; Type: files; Tasks: ; Languages:
Name: {app}\libgpg-error-0.dll; Type: files; Tasks: ; Languages:
Name: {app}\libgstapp.dll; Type: files; Tasks: ; Languages:
Name: {app}\libgstaudio.dll; Type: files; Tasks: ; Languages:
Name: {app}\libgstaudio-0.10.dll; Type: files; Tasks: ; Languages:
Name: {app}\libgstbase-0.10.dll; Type: files; Tasks: ; Languages:
Name: {app}\libgstcdda.dll; Type: files; Tasks: ; Languages:
Name: {app}\libgstcontroller-0.10.dll; Type: files; Tasks: ; Languages:
Name: {app}\libgstdataprotocol-0.10.dll; Type: files; Tasks: ; Languages:
Name: {app}\libgstdshow.dll; Type: files; Tasks: ; Languages:
Name: {app}\libgstfft.dll; Type: files; Tasks: ; Languages:
Name: {app}\libgstinterfaces.dll; Type: files; Tasks: ; Languages:
Name: {app}\libgstnet-0.10.dll; Type: files; Tasks: ; Languages:
Name: {app}\libgstnetbuffer.dll; Type: files; Tasks: ; Languages:
Name: {app}\libgstpbutils.dll; Type: files; Tasks: ; Languages:
Name: {app}\libgstreamer-0.10.dll; Type: files; Tasks: ; Languages:
Name: {app}\libgstriff.dll; Type: files; Tasks: ; Languages:
Name: {app}\libgstrtp.dll; Type: files; Tasks: ; Languages:
Name: {app}\libgstrtsp.dll; Type: files; Tasks: ; Languages:
Name: {app}\libgstsdp.dll; Type: files; Tasks: ; Languages:
Name: {app}\libgsttag.dll; Type: files; Tasks: ; Languages:
Name: {app}\libgstvideo.dll; Type: files; Tasks: ; Languages:
Name: {app}\libgthread-2.0-0.dll; Type: files; Tasks: ; Languages:
Name: {app}\libjpeg.dll; Type: files; Tasks: ; Languages:
Name: {app}\libmp3lame-0.dll; Type: files; Tasks: ; Languages:
Name: {app}\libneon-27.dll; Type: files; Tasks: ; Languages:
Name: {app}\libogg-0.dll; Type: files; Tasks: ; Languages:
Name: {app}\liboil-0.3-0.dll; Type: files; Tasks: ; Languages:
Name: {app}\libopenjpeg-2.dll; Type: files; Tasks: ; Languages:
Name: {app}\libpng12-0.dll; Type: files; Tasks: ; Languages:
Name: {app}\libschroedinger-1.0-0.dll; Type: files; Tasks: ; Languages:
Name: {app}\libspeex-1.dll; Type: files; Tasks: ; Languages:
Name: {app}\libtheora-0.dll; Type: files; Tasks: ; Languages:
Name: {app}\libvorbis-0.dll; Type: files; Tasks: ; Languages:
Name: {app}\libvorbisenc-2.dll; Type: files; Tasks: ; Languages:
Name: {app}\libxml2-2.dll; Type: files; Tasks: ; Languages:
Name: {app}\libxml2.dll; Type: files; Tasks: ; Languages:
Name: {app}\nspr4.dll; Type: files; Tasks: ; Languages:
Name: {app}\nss3.dll; Type: files; Tasks: ; Languages:
Name: {app}\nssckbi.dll; Type: files; Tasks: ; Languages:
Name: {app}\plc4.dll; Type: files; Tasks: ; Languages:
Name: {app}\plds4.dll; Type: files; Tasks: ; Languages:
Name: {app}\smime3.dll; Type: files; Tasks: ; Languages:
Name: {app}\softokn3.dll; Type: files; Tasks: ; Languages:
Name: {app}\ssl3.dll; Type: files; Tasks: ; Languages:
Name: {app}\xpcom.dll; Type: files; Tasks: ; Languages:
Name: {app}\xul.dll; Type: files; Tasks: ; Languages:
Name: {app}\xvidcore.dll; Type: files; Tasks: ; Languages:
Name: {app}\zlib1.dll; Type: files; Tasks: ; Languages:

; We don't distribute the CRT like this anymore; murder death kill
Name: {app}\SLPlugin.exe.config; Type: files; Tasks: ; Languages:
Name: {app}\Microsoft.VC80.CRT.manifest; Type: files; Tasks: ; Languages:
Name: {app}\msvcp80.dll; Type: files; Tasks: ; Languages:
Name: {app}\msvcr80.dll; Type: files; Tasks: ; Languages:
Name: {app}\msvcr71.dll; Type: files; Tasks: ; Languages:
Name: {app}\inworldz.exe.config; Type: files; Tasks: ; Languages:


[Code]
// [HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\VisualStudio\10.0\VC\VCRedist\x86] 
//   Installed = 1 (REG_DWORD)
function IsVS2010RedistInstalled(): Boolean;
var
  V: Cardinal;
  Success: Boolean;
begin
  if IsWin64 then begin
    Success := RegQueryDWordValue(HKLM64, 'SOFTWARE\Microsoft\VisualStudio\10.0\VC\VCRedist\x86', 'Installed', V);
  end else begin
    Success := RegQueryDWordValue(HKLM, 'SOFTWARE\Microsoft\VisualStudio\10.0\VC\VCRedist\x86', 'Installed', V);
  end

  if Success = TRUE then begin
    if V = 1 then begin
      Result := TRUE;
    end else begin
      Result := FALSE;
    end
  end else begin
    Result := FALSE;
  end
end;

function Needs2010Redist(): Boolean;
begin
  Result := (IsVS2010RedistInstalled = FALSE);
end;
