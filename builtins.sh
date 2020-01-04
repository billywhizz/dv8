MAIN=${2:-default}
OVERRIDE=${3:-src/override.js}
if [[ "$MAIN" == "override" ]]; then
  echo "building overridden main with $OVERRIDE"
  cp src/main.js src/backup.js
  cp $OVERRIDE src/main.js
fi
xxd -i src/main.js > src/builtins.h
sed -i 's/unsigned char/static const char/g' src/builtins.h
sed -i 's/unsigned int/static unsigned int/g' src/builtins.h
sed -i 's/examples_//g' src/builtins.h
if [[ "$MAIN" == "override" ]]; then
  cp src/backup.js src/main.js
fi