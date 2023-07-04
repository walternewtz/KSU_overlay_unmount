[ ! -d "$MODPATH/libs/$ABI" ] && abort "! $ABI not supported"

if ! $BOOTMODE; then
    abort "! Installing from recovery is not supported"
fi

ui_print "- Install ksuhide"
cp -af "$MODPATH/libs/$ABI/ksuhide" "$MODPATH/ksuhide"

chmod 755 "$MODPATH/ksuhide"
if ! "$MODPATH/ksuhide" check; then
    abort
fi

ui_print "- Enable systemless hosts"
mkdir -p "$MODPATH/system/etc"
cp -af /system/etc/hosts "$MODPATH/system/etc"