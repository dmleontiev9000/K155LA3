Dim objArgs
Set objArgs = WScript.Arguments
If objArgs.Count <> 2 Then
    WScript.Quit 1
End If

ZipFile   = objArgs.Item(0)
ExtractTo = objArgs.Item(1)

'If the extraction location does not exist create it.
Set fso = CreateObject("Scripting.FileSystemObject")
If NOT fso.FolderExists(ExtractTo) Then
   fso.CreateFolder(ExtractTo)
End If

'Extract the contants of the zip file.
set objShell = CreateObject("Shell.Application")
set FilesInZip=objShell.NameSpace(ZipFile).items
objShell.NameSpace(ExtractTo).CopyHere(FilesInZip)
Set fso = Nothing
Set objShell = Nothing
