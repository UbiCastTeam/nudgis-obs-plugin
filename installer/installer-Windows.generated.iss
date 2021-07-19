#define MyAppName "obs-plugintemplate"
#define MyAppVersion "1.0.0"
#define MyAppPublisher "Your Name Here"
#define MyAppURL "http://www.mywebsite.com"

[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{CD703FE5-1F2C-4837-BD3D-DD840D83C3E3}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={code:GetDirName}
DefaultGroupName={#MyAppName}
OutputBaseFilename={#MyAppName}-{#MyAppVersion}-Windows-Installer
Compression=lzma
SolidCompression=yes

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Files]
Source: "..\release\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "..\LICENSE"; Flags: dontcopy
; NOTE: Don't use "Flags: ignoreversion" on any shared system files

[Icons]
Name: "{group}\{cm:ProgramOnTheWeb,{#MyAppName}}"; Filename: "{#MyAppURL}"
Name: "{group}\{cm:UninstallProgram,{#MyAppName}}"; Filename: "{uninstallexe}"

[Code]
procedure InitializeWizard();
var
  GPLText: AnsiString;
  Page: TOutputMsgMemoWizardPage;
begin
  ExtractTemporaryFile('LICENSE');
  LoadStringFromFile(ExpandConstant('{tmp}\LICENSE'), GPLText);
  Page := CreateOutputMsgMemoPage(wpWelcome,
    'License Information', 'Please review the license terms before installing {#MyAppName}',
    'Press Page Down to see the rest of the agreement. Once you are aware of your rights, click Next to continue.',
    String(GPLText)
  );
end;

// credit where it's due :
// following function come from https://github.com/Xaymar/obs-studio_amf-encoder-plugin/blob/master/%23Resources/Installer.in.iss#L45
function GetDirName(Value: string): string;
var
  InstallPath: string;
begin
  // initialize default path, which will be returned when the following registry
  // key queries fail due to missing keys or for some different reason
  Result := '{pf}\obs-studio';
  // query the first registry value; if this succeeds, return the obtained value
  if RegQueryStringValue(HKLM32, 'SOFTWARE\OBS Studio', '', InstallPath) then
    Result := InstallPath
end;

