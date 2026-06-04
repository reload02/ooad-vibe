$ErrorActionPreference = 'Stop'

$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$coveragePath = Join-Path $PSScriptRoot 'coverage.merged.cobertura.xml'
$testResultsPath = Join-Path $PSScriptRoot 'coverage-test-results.xml'
$outputPath = Join-Path $PSScriptRoot 'coverage_analysis.html'

[xml]$coverage = Get-Content $coveragePath
[xml]$testXml = Get-Content $testResultsPath

function HtmlEscape([string]$value) {
    if ($null -eq $value) { return '' }
    return [System.Net.WebUtility]::HtmlEncode($value)
}

function MissingRanges($numbers) {
    $sorted = @($numbers | Sort-Object {[int]$_})
    if ($sorted.Count -eq 0) { return '-' }

    $ranges = New-Object System.Collections.Generic.List[string]
    $start = [int]$sorted[0]
    $prev = $start

    for ($i = 1; $i -lt $sorted.Count; ++$i) {
        $current = [int]$sorted[$i]
        if ($current -eq $prev + 1) {
            $prev = $current
            continue
        }

        if ($start -eq $prev) { $ranges.Add([string]$start) } else { $ranges.Add("$start-$prev") }
        $start = $current
        $prev = $current
    }

    if ($start -eq $prev) { $ranges.Add([string]$start) } else { $ranges.Add("$start-$prev") }
    return ($ranges -join ', ')
}

$files = @{}
foreach ($package in $coverage.coverage.packages.package) {
    foreach ($class in $package.classes.class) {
        $filename = [string]$class.filename
        if ([string]::IsNullOrWhiteSpace($filename)) { continue }
        if (-not $filename.StartsWith($repoRoot, [StringComparison]::OrdinalIgnoreCase)) { continue }

        $relative = $filename.Substring($repoRoot.Length + 1).Replace('\', '/')
        if (!(($relative.StartsWith('src/')) -or ($relative.StartsWith('include/')) -or ($relative.StartsWith('simulator_rvc_interface/')))) { continue }
        if ($relative.StartsWith('simulator_rvc_interface/tests/')) { continue }

        if (-not $files.ContainsKey($relative)) {
            $files[$relative] = [ordered]@{ Path = $filename; Lines = @{} }
        }

        foreach ($line in $class.lines.line) {
            $lineNumber = [int]$line.number
            $hits = [int]$line.hits
            if ((-not $files[$relative].Lines.ContainsKey($lineNumber)) -or $files[$relative].Lines[$lineNumber] -lt $hits) {
                $files[$relative].Lines[$lineNumber] = $hits
            }
        }
    }
}

$rows = foreach ($relative in $files.Keys) {
    $valid = $files[$relative].Lines.Count
    $covered = @($files[$relative].Lines.Values | Where-Object { $_ -gt 0 }).Count
    $missed = $valid - $covered
    $rate = if ($valid -gt 0) { [Math]::Round(($covered / $valid) * 100, 1) } else { 0 }
    $uncovered = @($files[$relative].Lines.Keys | Where-Object { $files[$relative].Lines[$_] -eq 0 } | Sort-Object {[int]$_})

    [PSCustomObject]@{
        File = $relative
        AbsolutePath = $files[$relative].Path
        Covered = $covered
        Lines = $valid
        Missed = $missed
        Rate = $rate
        Uncovered = $uncovered
    }
}

$rows = @($rows | Sort-Object Rate, File)
$totalLines = ($rows | Measure-Object Lines -Sum).Sum
$totalCovered = ($rows | Measure-Object Covered -Sum).Sum
$totalRate = [Math]::Round(($totalCovered / $totalLines) * 100, 1)
$rawRate = [Math]::Round(([double]$coverage.coverage.'line-rate') * 100, 1)
$testSummary = $testXml.testsuites
$generatedAt = Get-Date -Format 'yyyy-MM-dd HH:mm:ss K'

$recommendations = @(
    [PSCustomObject]@{
        Area = 'CLI 옵션/오류 경로'
        Detail = 'src/main.cpp의 --help, unknown/incomplete argument, 예외 처리, quiet-map=false 출력 경로가 미커버입니다.'
        Action = 'CLI smoke test를 3-4개 추가해 정상 출력뿐 아니라 도움말/오류 반환 코드를 검증합니다.'
    },
    [PSCustomObject]@{
        Area = '시나리오 로딩 실패'
        Detail = 'GridSimulator::loadScenario의 파일 열기 실패 분기가 미커버입니다.'
        Action = '존재하지 않는 .rvc 경로를 loadScenario에 전달하는 단위 테스트를 추가합니다.'
    },
    [PSCustomObject]@{
        Area = '방어적 accessor/null 경로'
        Detail = 'Rvc null adapter, GridSimulator accessor, Rvc::lastCommand 같은 낮은 위험 라인이 남아 있습니다.'
        Action = '공개 API 안정성을 문서화하려면 accessor smoke test와 null adapter 예외 테스트를 추가합니다.'
    },
    [PSCustomObject]@{
        Area = 'enum fallback/default arm'
        Detail = 'Types.cpp와 SimulatedHardwareAdapter.cpp의 Unknown/default fallback은 정상 enum 값으로는 거의 도달하지 않습니다.'
        Action = '필요하면 invalid enum cast 기반 방어 테스트를 추가하되, 요구사항 커버리지에는 낮은 우선순위로 둡니다.'
    }
)

$sb = [System.Text.StringBuilder]::new()
[void]$sb.AppendLine('<!doctype html>')
[void]$sb.AppendLine('<html lang="ko">')
[void]$sb.AppendLine('<head>')
[void]$sb.AppendLine('<meta charset="utf-8">')
[void]$sb.AppendLine('<meta name="viewport" content="width=device-width, initial-scale=1">')
[void]$sb.AppendLine('<title>RVC 커버리지 분석지</title>')
[void]$sb.AppendLine('<style>')
[void]$sb.AppendLine(':root{--bg:#f7f8fa;--panel:#ffffff;--ink:#1f2937;--muted:#64748b;--line:#d9e0e8;--green:#15803d;--amber:#b45309;--red:#b91c1c;--blue:#2563eb;--code:#0f172a}*{box-sizing:border-box}body{margin:0;background:var(--bg);color:var(--ink);font-family:Segoe UI,Malgun Gothic,Apple SD Gothic Neo,sans-serif;line-height:1.55}header{background:#fff;border-bottom:1px solid var(--line)}.wrap{max-width:1180px;margin:0 auto;padding:28px 24px}h1{margin:0 0 8px;font-size:30px;letter-spacing:0}h2{margin:34px 0 14px;font-size:21px}h3{margin:24px 0 8px;font-size:17px}.muted{color:var(--muted)}.grid{display:grid;grid-template-columns:repeat(4,minmax(0,1fr));gap:12px;margin-top:20px}.metric{background:var(--panel);border:1px solid var(--line);border-radius:8px;padding:16px}.metric strong{display:block;font-size:28px}.metric span{color:var(--muted);font-size:13px}.panel{background:var(--panel);border:1px solid var(--line);border-radius:8px;padding:18px;margin-top:14px}table{width:100%;border-collapse:collapse;background:var(--panel);border:1px solid var(--line);border-radius:8px;overflow:hidden}th,td{padding:10px 12px;border-bottom:1px solid var(--line);text-align:left;vertical-align:top}th{background:#eef2f7;font-size:13px;color:#334155}tr:last-child td{border-bottom:0}.num{text-align:right;font-variant-numeric:tabular-nums}.bar{height:9px;background:#e2e8f0;border-radius:999px;overflow:hidden;min-width:120px}.bar span{display:block;height:100%;background:var(--blue)}.rate-good{color:var(--green);font-weight:700}.rate-mid{color:var(--amber);font-weight:700}.rate-low{color:var(--red);font-weight:700}.pill{display:inline-block;border:1px solid var(--line);border-radius:999px;padding:3px 9px;font-size:12px;background:#f8fafc;color:#334155}.code{font-family:Cascadia Mono,Consolas,monospace;font-size:12px;background:var(--code);color:#dbeafe;border-radius:8px;overflow:auto;border:1px solid #111827}.code-line{display:grid;grid-template-columns:54px 44px 1fr;gap:10px;padding:2px 10px;white-space:pre}.code-line.hit{background:rgba(22,163,74,.16)}.code-line.miss{background:rgba(239,68,68,.24)}.code-line.neutral{color:#94a3b8}.ln{color:#93c5fd;text-align:right}.hits{text-align:right;color:#cbd5e1}.miss .hits{color:#fecaca}.note{border-left:4px solid var(--blue);padding:10px 12px;background:#eff6ff;border-radius:4px}.rec{display:grid;grid-template-columns:180px 1fr 1.2fr;gap:12px;border-top:1px solid var(--line);padding:12px 0}.rec:first-child{border-top:0}@media(max-width:760px){.grid{grid-template-columns:1fr 1fr}.rec{grid-template-columns:1fr}.wrap{padding:22px 16px}table{font-size:13px}}')
[void]$sb.AppendLine('</style>')
[void]$sb.AppendLine('</head><body>')
[void]$sb.AppendLine('<header><div class="wrap">')
[void]$sb.AppendLine('<h1>RVC 커버리지 분석지</h1>')
[void]$sb.AppendLine('<div class="muted">생성 시각: ' + (HtmlEscape $generatedAt) + ' · 기준 XML: docs/coverage.merged.cobertura.xml · 범위: 프로젝트 소스(src/include/simulator_rvc_interface), 외부 라이브러리 제외</div>')
[void]$sb.AppendLine('<div class="grid">')
[void]$sb.AppendLine('<div class="metric"><strong>' + $totalRate + '%</strong><span>프로젝트 라인 커버리지</span></div>')
[void]$sb.AppendLine('<div class="metric"><strong>' + $totalCovered + ' / ' + $totalLines + '</strong><span>커버된 라인 / 실행 가능 라인</span></div>')
[void]$sb.AppendLine('<div class="metric"><strong>' + $testSummary.tests + '</strong><span>GoogleTest 실행 건수</span></div>')
[void]$sb.AppendLine('<div class="metric"><strong>' + $testSummary.failures + '</strong><span>실패 테스트</span></div>')
[void]$sb.AppendLine('</div></div></header>')
[void]$sb.AppendLine('<main class="wrap">')
[void]$sb.AppendLine('<section class="panel note">원본 Cobertura 전체 수치는 GoogleTest 및 표준 라이브러리 코드까지 포함되어 ' + $rawRate + '%로 표시됩니다. 이 분석지는 제품 코드만 필터링한 ' + $totalRate + '%를 주 지표로 사용합니다.</section>')
[void]$sb.AppendLine('<h2>파일별 요약</h2><table><thead><tr><th>파일</th><th class="num">커버리지</th><th>분포</th><th class="num">커버</th><th class="num">미커버</th><th>미커버 라인</th></tr></thead><tbody>')

foreach ($row in $rows) {
    $rateClass = if ($row.Rate -ge 90) { 'rate-good' } elseif ($row.Rate -ge 75) { 'rate-mid' } else { 'rate-low' }
    $width = [Math]::Max(2, [Math]::Round($row.Rate, 1))
    [void]$sb.AppendLine('<tr><td><code>' + (HtmlEscape $row.File) + '</code></td><td class="num ' + $rateClass + '">' + $row.Rate + '%</td><td><div class="bar"><span style="width:' + $width + '%"></span></div></td><td class="num">' + $row.Covered + ' / ' + $row.Lines + '</td><td class="num">' + $row.Missed + '</td><td>' + (HtmlEscape (MissingRanges $row.Uncovered)) + '</td></tr>')
}

[void]$sb.AppendLine('</tbody></table>')
[void]$sb.AppendLine('<h2>주요 해석</h2><div class="panel">')
foreach ($rec in $recommendations) {
    [void]$sb.AppendLine('<div class="rec"><div><span class="pill">' + (HtmlEscape $rec.Area) + '</span></div><div>' + (HtmlEscape $rec.Detail) + '</div><div><strong>권장:</strong> ' + (HtmlEscape $rec.Action) + '</div></div>')
}
[void]$sb.AppendLine('</div>')
[void]$sb.AppendLine('<h2>미커버 라인 상세</h2>')

foreach ($row in @($rows | Where-Object { $_.Missed -gt 0 })) {
    [void]$sb.AppendLine('<section class="panel"><h3><code>' + (HtmlEscape $row.File) + '</code> <span class="muted">' + $row.Rate + '% · 미커버 ' + (HtmlEscape (MissingRanges $row.Uncovered)) + '</span></h3>')
    $sourceLines = Get-Content $row.AbsolutePath
    $interesting = New-Object System.Collections.Generic.HashSet[int]
    foreach ($n in $row.Uncovered) {
        for ($j = [Math]::Max(1, [int]$n - 2); $j -le [Math]::Min($sourceLines.Count, [int]$n + 2); ++$j) {
            [void]$interesting.Add($j)
        }
    }

    $ordered = @($interesting | Sort-Object)
    [void]$sb.AppendLine('<div class="code" role="table" aria-label="uncovered source lines">')
    $last = 0
    foreach ($lineNo in $ordered) {
        if ($last -ne 0 -and $lineNo -gt $last + 1) {
            [void]$sb.AppendLine('<div class="code-line neutral"><span class="ln">...</span><span class="hits"></span><span>...</span></div>')
        }

        $hitKnown = $files[$row.File].Lines.ContainsKey($lineNo)
        $hits = if ($hitKnown) { [int]$files[$row.File].Lines[$lineNo] } else { -1 }
        $kind = if ($hitKnown -and $hits -eq 0) { 'miss' } elseif ($hitKnown -and $hits -gt 0) { 'hit' } else { 'neutral' }
        $hitLabel = if ($hits -ge 0) { [string]$hits } else { '' }
        $text = HtmlEscape $sourceLines[$lineNo - 1]
        [void]$sb.AppendLine('<div class="code-line ' + $kind + '"><span class="ln">' + $lineNo + '</span><span class="hits">' + $hitLabel + '</span><span>' + $text + '</span></div>')
        $last = $lineNo
    }
    [void]$sb.AppendLine('</div></section>')
}

[void]$sb.AppendLine('<h2>실행 내역</h2><div class="panel"><p><strong>수집 명령:</strong> Microsoft.CodeCoverage.Console collect + merge, output-format cobertura</p><p><strong>수집 대상:</strong> <code>rvc_tests.exe</code>, <code>rvc_simulator.exe --ticks 5 --quiet-map</code>, <code>rvc_simulator.exe --scenario scenarios/continuous_backward.rvc --quiet-map</code></p><p><strong>산출물:</strong> <code>docs/coverage.merged.cobertura.xml</code>, <code>docs/coverage-test-results.xml</code>, <code>docs/coverage_analysis.html</code></p></div>')
[void]$sb.AppendLine('</main></body></html>')

$sb.ToString() | Set-Content -Path $outputPath -Encoding UTF8
Write-Host "Wrote $outputPath"
