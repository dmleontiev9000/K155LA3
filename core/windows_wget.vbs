Dim objArgs
Set objArgs = WScript.Arguments
If objArgs.Count <> 2 Then
    WScript.Quit 1
End If

strFileURL=objArgs.Item(0)
strLocation=objArgs.Item(1)
Set objXMLHTTP=CreateObject("MSXML2.XMLHTTP")
objXMLHTTP.open "GET", strFileURL, false
objXMLHTTP.send

If objXMLHTTP.Status = 200 Then
    Set objADOStream = CreateObject("ADODB.Stream")
    objADOStream.Open
    objADOStream.Type = 1 'adTypeBinary

    objADOStream.Write objXMLHTTP.ResponseBody
    objADOStream.Position = 0    'Set the stream position to the start

    Set objFSO = CreateObject("Scripting.FileSystemObject")
    If objFSO.Fileexists(strLocation) Then objFSO.DeleteFile strHDLocation

    objADOStream.SaveToFile strLocation
    objADOStream.Close
    Set objADOStream = Nothing
    Set objXMLHTTP = Nothing
    Set objFSO = Nothing

    WScript.Quit 0
End if

Set objXMLHTTP = Nothing
WScript.Quit 1
