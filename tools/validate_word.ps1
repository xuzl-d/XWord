param(
    [Parameter(Mandatory=$true)][string]$InputPath,
    [Parameter(Mandatory=$true)][string]$OutputPath
)
$ErrorActionPreference = 'Stop'
$source = (Resolve-Path -LiteralPath $InputPath).Path
$destination = [System.IO.Path]::GetFullPath($OutputPath)
if ($source -eq $destination) { throw 'Use a separate output path; the input is opened read-only.' }
if (Get-Process WINWORD -ErrorAction SilentlyContinue) { throw 'Close other Word instances before running this optional isolated client check.' }
$word = $null
$document = $null
try {
    $word = New-Object -ComObject Word.Application
    $word.Visible = $false
    $word.DisplayAlerts = 0
    $word.AutomationSecurity = 3
    $document = $word.Documents.Open($source, $false, $true, $false)
    $updateResult = $document.Fields.Update()
    foreach ($story in $document.StoryRanges) {
        $range = $story
        while ($null -ne $range) {
            $null = $range.Fields.Update()
            $range = $range.NextStoryRange
        }
    }
    foreach ($toc in $document.TablesOfContents) { $toc.Update() }
    foreach ($toc in $document.TablesOfFigures) { $toc.Update() }
    $document.Repaginate()
    $sourceCount = $document.Bibliography.Sources.Count
    $stats = [PSCustomObject]@{
        WordVersion = $word.Version
        WordBuild = $word.Build
        Sections = $document.Sections.Count
        Pages = $document.ComputeStatistics(2)
        Footnotes = $document.Footnotes.Count
        Endnotes = $document.Endnotes.Count
        Comments = $document.Comments.Count
        Sources = $sourceCount
        FieldUpdateResult = $updateResult
        Output = $destination
    }
    $document.SaveAs2($destination, 16)
    $stats | ConvertTo-Json
} finally {
    $discard = 0
    try {
        if ($null -ne $document) { $document.Close([ref]$discard); [void][Runtime.InteropServices.Marshal]::FinalReleaseComObject($document) }
    } finally {
        if ($null -ne $word) { $word.Quit([ref]$discard); [void][Runtime.InteropServices.Marshal]::FinalReleaseComObject($word) }
    }
    [GC]::Collect()
    [GC]::WaitForPendingFinalizers()
}
