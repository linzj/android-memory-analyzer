#echo $@ >/sdcard/shit
LD_LIBRARY_PATH=/data/local/tmp:$LD_LIBRARY_PATH \
LD_PRELOAD=libmemanaly.so \
exec $@

