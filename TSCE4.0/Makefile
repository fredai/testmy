

DIRS = module src

all:
	for i in $(DIRS); do make -C $$i; done
	for i in $(DIRS); do cd $$i;make release;cd ..; done
	rm -rf ./release/config/ 
	cp -r ./config ./release

clean:
	for i in $(DIRS); do cd $$i;make clean;cd ..; done
	rm ./release/bin/tsce* -rf


