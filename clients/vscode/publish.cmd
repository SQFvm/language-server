yarn run compile
vsce package --yarn
vsce publish --pre-release --yarn
pause