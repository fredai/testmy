all:
	tar zxvf gnokii-0.6.31.tar.gz
	cd gnokii-0.6.31;./configure;make;
	rm -f ../release/ex-alarm/lib/gnokii
	cp gnokii-0.6.31/gnokii/gnokii ../release/ex-alarm/lib/
	rm -rf gnokii-0.6.31
