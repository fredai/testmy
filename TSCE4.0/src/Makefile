

DIRS = _con svr clr getdata tools manager  svr/user

all:
	for i in $(DIRS); do make -C $$i; done

clean:
	for i in $(DIRS); do cd $$i;make clean;cd ..; done

release:
	cp ./clr/tscecd ./clr/tscecd_start ./clr/tscecd_stop ../release/bin/
	cp ./svr/tscesd ./svr/tscesd_start ./svr/tscesd_stop ./svr/tscesd_status ../release/bin/
	cp ./tools/tscecd_all_start  ./tools/tscecd_all_stop ./tools/tsce_start ./tools/tsce_stop ./tools/tsce_export ../release/bin/
	cp ./getdata/tsce_getdata ../release/bin/
	cp ./manager/debug/getdata ../release/bin/
	chmod 755 ./manager/status/tsce_node 
	cp ./manager/status/tsce_node  ../release/bin/
#	cp ../module/*.so ../release/module/
#	cp -r ../module/script ../release/module/
#	cp -r ../config ../release/
