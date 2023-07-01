MODDIR="${0%/*}"
chmod 755 "$MODDIR/magiskhide"
chmod 755 "$MODDIR/magic-mount"
rm -rf /cache/ksu_unmount.log.bak
mv -f /cache/ksu_unmount.log /cache/ksu_unmount.log.bak
exec "$MODDIR/magiskhide"