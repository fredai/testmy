

DIRS = util collector collectord collectord/tools

all:
	for i in $(DIRS); do make -C $$i; done

clean:
	for i in $(DIRS); do cd $$i;make clean;cd ..; done

##VERDATE := $(shell date +%Y-%m-%d)

release:
	cp ./collectord/collectord ../release/collectord/lib/
	cp ./collectord/tools/checkstatus ../release/collectord/bin/
	cp ./collectord/tools/getdata ../release/collectord/bin/
	cp ./collectord/tools/sendcmd ../release/collectord/bin/
	cp ./collector/collector ../release/collector/bin/


