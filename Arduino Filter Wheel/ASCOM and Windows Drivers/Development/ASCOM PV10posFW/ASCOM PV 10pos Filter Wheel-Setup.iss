;
; Script generated by the ASCOM Driver Installer Script Generator 6.0.0
; Generated by Quidne IT Ltd on 12/03/2017 (UTC)
; Last Updated on 05/02/2019 (UTC)
;
#define AppVersion = "1.0.0"

;#define ReleaseType = "Release"
#define ReleaseType = "Debug"

#define BaseCodePath = "F:\Projects\Dropbox\Astronomy\Arduino Filter Wheel\ASCOM and Windows Drivers"

#define DefaultCodeFolder="ASCOM PV10posFW"

#define AppName="ASCOM PV 10pos Filter Wheel"

[Setup]
AppId={{C2B5DD9B-D314-40F1-BEB7-D0F619F1802F}
AppName={#AppName}
AppVerName={#AppName} {#AppVersion}
AppVersion={#AppVersion}
AppPublisher=Michael Okincha
AppPublisherURL=http://www.google.com
AppSupportURL=mailto:mokincha@gmail.com
AppUpdatesURL=http://www.google.com
VersionInfoVersion=1.0.0
MinVersion=0,5.0.2195sp4
DefaultDirName="{cf}\ASCOM\{#DefaultCodeFolder}"
DisableDirPage=yes
DisableProgramGroupPage=yes
OutputDir="."
OutputBaseFilename="{#AppName} - {#ReleaseType} ({#AppVersion})"
Compression=lzma
SolidCompression=yes
; Put there by Platform if Driver Installer Support selected
WizardImageFile="{#BaseCodePath}\Development\Installer Generator\Resources\WizardImage.bmp"
LicenseFile="{#BaseCodePath}\Development\Installer Generator\Resources\CreativeCommons.txt"
; {cf}\ASCOM\Uninstall\FilterWheel folder created by Platform, always
UninstallFilesDir="{cf}\ASCOM\Uninstall\{#DefaultCodeFolder}"
UninstallDisplayIcon="{app}\ASCOM.PV_10pos.FilterWheel.dll"

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Dirs]
Name: "{cf}\ASCOM\Uninstall\{#DefaultCodeFolder}"

[Files]
Source: "{#BaseCodePath}\Development\{#DefaultCodeFolder}\bin\{#ReleaseType}\ASCOM.PV_10pos.FilterWheel.dll"; DestDir: "{app}"
Source: "{#BaseCodePath}\Development\{#DefaultCodeFolder}\VersionHistory.txt"; DestDir: "{app}" ;Flags: isreadme

[Icons]
Name: "{commondesktop}\{#DefaultCodeFolder}"; Filename: "{app}\ASCOM.PV_10pos.FilterWheel.dll"

; Only if driver is .NET
[Run]
; Only for .NET assembly/in-proc drivers
Filename: "{dotnet4032}\regasm.exe"; Parameters: "/codebase ""{app}\ASCOM.PV_10pos.FilterWheel.dll"""; Flags: runhidden 32bit
Filename: "{dotnet4064}\regasm.exe"; Parameters: "/codebase ""{app}\ASCOM.PV_10pos.FilterWheel.dll"""; Flags: runhidden 64bit; Check: IsWin64

; Only if driver is .NET
[UninstallRun]
; Only for .NET assembly/in-proc drivers
Filename: "{dotnet4032}\regasm.exe"; Parameters: "-u ""{app}\ASCOM.PV_10pos.FilterWheel.dll"""; Flags: runhidden 32bit
; This helps to give a clean uninstall
Filename: "{dotnet4064}\regasm.exe"; Parameters: "/codebase ""{app}\ASCOM.PV_10pos.FilterWheel.dll"""; Flags: runhidden 64bit; Check: IsWin64
Filename: "{dotnet4064}\regasm.exe"; Parameters: "-u ""{app}\ASCOM.PV_10pos.FilterWheel.dll"""; Flags: runhidden 64bit; Check: IsWin64

[CODE]
//
// Before the installer UI appears, verify that the (prerequisite)
// ASCOM Platform 6.0 or great is installed, including both Helper
// components. Utility is required for all types (COM and .NET)!
//
function InitializeSetup(): Boolean;
var
   U : Variant;
   H : Variant;
begin
   Result := TRUE;  // Assume failure
end;

// Code to enable the installer to uninstall previous versions of itself when a new version is installed
procedure CurStepChanged(CurStep: TSetupStep);
var
  ResultCode: Integer;
  UninstallExe: String;
  UninstallRegistry: String;
begin
  if (CurStep = ssInstall) then // Install step has started
	begin
      // Create the correct registry location name, which is based on the AppId
      UninstallRegistry := ExpandConstant('Software\Microsoft\Windows\CurrentVersion\Uninstall\{#SetupSetting("AppId")}' + '_is1');
      // Check whether an extry exists
      if RegQueryStringValue(HKLM, UninstallRegistry, 'UninstallString', UninstallExe) then
        begin // Entry exists and previous version is installed so run its uninstaller quietly after informing the user
          MsgBox('Setup will now remove the previous version.', mbInformation, MB_OK);
          Exec(RemoveQuotes(UninstallExe), ' /SILENT', '', SW_SHOWNORMAL, ewWaitUntilTerminated, ResultCode);
          sleep(1000);    //Give enough time for the install screen to be repainted before continuing
        end
  end;
end;

