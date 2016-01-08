#echo $$ > /pipe
LD_PRELOAD=/data/local/tmp/libmemanaly.so \
exec $@

