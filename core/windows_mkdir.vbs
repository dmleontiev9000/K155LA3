Dim objArgs
Set objArgs = WScript.Arguments
If objArgs.Count <> 1 Then
    WScript.Quit 1
End If

PATH = objArgs.Item(0)

Set fso = CreateObject("Scripting.FileSystemObject")

BuildFullPath PATH

Sub BuildFullPath(ByVal FullPath)
If Not fso.FolderExists(FullPath) Then
BuildFullPath fso.GetParentFolderName(FullPath)
fso.CreateFolder FullPath
End If
End Sub
