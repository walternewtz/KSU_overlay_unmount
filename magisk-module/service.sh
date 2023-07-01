MODDIR="${0%/*}"
chmod 755 "$MODDIR/magiskhide"
chmod 755 "$MODDIR/magic-mount"
rm -rf /cache/ksu_umount.log.bak
mv -f /cache/ksu_umount.log /cache/ksu_umount.log.bak
exec "$MODDIR/magiskhide"