$ExecutingDirectory = [System.IO.Path]::GetDirectoryName($PSCommandPath)
Set-Location $ExecutingDirectory

git submodule update --init

Set-Location "$ExecutingDirectory/third-party/openssl"
git config --local core.eol lf
git config --local core.autocrlf false
git rm --cached -r . && git reset --hard
