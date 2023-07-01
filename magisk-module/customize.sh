[ ! -d "$MODPATH/libs/$ABI" ] && abort "! $ABI not supported"

if ! $BOOTMODE; then
    abort "! Installing from recovery is not supported"
fi

ui_print "- Extract module..."
cp -af "$MODPATH/libs/$ABI/magiskhide" "$MODPATH/magiskhide"

chmod 755 "$MODPATH/magiskhide"
if ! "$MODPATH/magiskhide" check; then
    abort
fi

cp -af "$MODPATH/libs/$ABI/magic-mount" "$MODPATH/magic-mount"
rm -rf "$MODPATH/libs"