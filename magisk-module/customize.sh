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

ui_print "- Enable systemless hosts"
mkdir -p "$MODPATH/system/etc"
cp -af /system/etc/hosts "$MODPATH/system/etc"

cp -af "$MODPATH/libs/$ABI/magic-mount" "$MODPATH/magic-mount"
rm -rf "$MODPATH/libs"