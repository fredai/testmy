DIRS = tsce pshelld/pshell/src alarm/ex_alarm_tstor


info_ = $(shell uname -r)

ifeq ($(findstring el7,$(info_)), el7)
	flag=el7
else
	flag=el6
endif


all:
	for i in $(DIRS); do make -C $$i; done
#	for i in $(DIRS); do cd $$i;make release;cd ..; done

clean:
	rm ./tsced/sbin/* -rf
	rm ./tsced/bin/pshell -rf
	rm ./tsced/bin/tsce_getdata -rf
	rm ./tsced/tools/test_email -rf
	rm ./tsced/tools/getdata -rf
	rm ./tsced/module/*.so -rf
	for i in $(DIRS); do cd $$i;make clean;cd ..; done
	
package:
	cp ./tsce/release/bin/tscecd tsce/release/bin/tscesd ./tsced/sbin/
	cp ./tsce/release/bin/tsce_getdata  ./tsced/bin/
	cp ./tsce/release/bin/getdata  ./tsced/tools/
	cp ./tsce/release/module/*.so  ./tsced/module/
#	cp ./tsce/module/libcudainfo.so ./tsced/lib/
	cp ./pshelld/pshell/release/bin/pshelld ./tsced/sbin/
	cp ./pshelld/pshell/release/bin/pshell ./tsced/bin/ 
	cp ./alarm/ex_alarm_tstor/release/ex-alarm/bin/test_email ./tsced/tools/
	cp ./alarm/ex_alarm_tstor/release/ex-alarm/bin/alarmd ./tsced/sbin/
	chmod 755 ./tsced/sbin/*
	chmod 755 ./tsced/bin/*
	chmod 755 ./tsced/tools/*
	chmod 755 ./tsced/module/*.so
	chmod 755 ./tsced/module/script/*
	chmod 755 ./tsced/lib/*.so*
	cp ./tsced/tools/lt-gnokii.$(flag) ./tsced/tools/lt-gnokii
	tar zcvf tsced-4.0.0.$(flag).x86_64.tar.gz tsced


