

DIRS = src test_email

all:
	for i in $(DIRS); do make -C $$i; done
	for i in $(DIRS); do cd $$i;make release;cd ..; done

clean:
	rm -f ./release/ex-alarm/bin/*
	for i in $(DIRS); do cd $$i;make clean;cd ..; done

