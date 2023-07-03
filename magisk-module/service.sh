MODDIR="${0%/*}"
chmod 755 "$MODDIR/ksuhide"
chmod 755 "$MODDIR/magic-mount"
rm -rf /cache/ksu_umount.log.bak
mv -f /cache/ksu_umount.log /cache/ksu_umount.log.bak
# mount writeable hosts to global mount ns
mount --bind "$MODDIR/system/etc/hosts" /system/etc/hosts
exec "$MODDIR/ksuhide"