; InWorldz inno setup installer script by McCabe Maxsted

[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)

; These will change
AppId={{DC6CCE02-BC61-43B1-B4CA-292C6BCCCB34}
AppName=InWorldz
AppVerName=InWorldz 1.2.0
DefaultDirName={pf}\InWorldz
DefaultGroupName=InWorldz
VersionInfoProductName=InWorldz Viewer Release
OutputBaseFilename=InWorldz-1.2.0-Setup
VersionInfoVersion=1.2.0
VersionInfoTextVersion=1.2.0
VersionInfoProductVersion=1.2.0
VersionInfoCopyright=2010
AppCopyright=2010

; These won't change
VersionInfoCompany=InWorldz, LLC
AppPublisher=InWorldz, LLC
AppPublisherURL=http://inworldz.com
AppSupportURL=http://inworldz.com
AppUpdatesURL=http://inworldz.com
AllowNoIcons=true
InfoAfterFile=..\windows\README.txt
OutputDir=C:\inworldz_installers
SetupIconFile=..\windows\install_icon.ico
Compression=lzma2/ultra64
InternalCompressLevel=ultra64
SolidCompression=true
;PrivilegesRequired=poweruser
AllowRootDirectory=true
;WizardImageFile=..\windows\installer_icon_left.bmp
;WizardSmallImageFile=..\windows\installer_icon_right.bmp
SetupLogging=true

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
;Source: ..\..\..\build-vc80\newview\release\packaged\doc\*; DestDir: {app}\doc; Flags: ignoreversion recursesubdirs createallsubdirs
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
Source: ..\..\..\build-vc80\newview\release\packaged\llkdu.dll.2.config; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\packaged\openal32.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\packaged\OpenJPEG.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\packaged\SLPlugin.exe; DestDir: {app}; Flags: ignoreversion

; VC++ 2005 x86 redist
Source: ..\..\..\newview\vcredist_x86.exe; DestDir: {tmp}; DestName: vcredist_x86.exe

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
Filename: {tmp}\vcredist_x86.exe; Parameters: "/q:a /c:""VCREDI~1.EXE /q:a /c:""""msiexec /i vcredist.msi /qn"""" """; Flags: runhidden
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
Name: {app}\lib\gstreamer-plugins\*; Type: filesandordirs; Tasks: ; Languages: 
; Name: {app}\skins\default\xui\*; Type: filesandordirs; Tasks: ; Languages:
; Name: {app}\skins\silver\xui\*; Type: filesandordirs; Tasks: ; Languages:
Name: C:\Documents and Settings\{username}\.gstreamer-0.10\*; Type: filesandordirs
Name: C:\Users\{username}\.gstreamer-0.10\*; Type: filesandordirs
; Breaks the browser if installing on top of 1.1:
Name: {app}\gksvggdiplus.dll; Type: files; Tasks: ; Languages:

; Pre-plugin files:
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

; We don't distribute the CRT like this anymore; kill old files
Name: {app}\SLPlugin.exe.config; Type: files; Tasks: ; Languages:
Name: {app}\Microsoft.VC80.CRT.manifest; Type: files; Tasks: ; Languages:
Name: {app}\msvcp80.dll; Type: files; Tasks: ; Languages:
Name: {app}\msvcr80.dll; Type: files; Tasks: ; Languages:
Name: {app}\imprudence.exe.config; Type: files; Tasks: ; Languages:
