all:
	cd Caravel && make clean && make
	cd fastore && make clean && make
	cd Client && make clean && make
	cd Proxy && make clean && make
	cd Client && chmod 751 *.sh
	ntpdate time.nist.gov

clean:
	cd Caravel && make clean
	cd Client && make clean
	cd Proxy && make clean
	cd fastore && make clean

client:
	cd Client && ./Client_cpp

proxy:
	cd Proxy && ./TProxy

shutall:
	ps aux|grep redis|grep -v grep|awk '{print "kill -9 "$$2}'|/bin/sh
	ps aux|grep TProxy|grep -v grep|awk '{print "kill -9 "$$2}'|/bin/sh

start2:
	ps aux|grep redis|grep -v grep|awk '{print "kill -9 "$$2}'|/bin/sh
	ps aux|grep TProxy|grep -v grep|awk '{print "kill -9 "$$2}'|/bin/sh
	nohup redis-server > /data/log/redis-server.log 2>&1 &
	nohup /data/ASIACCS17/Proxy/TProxy 2 > /data/log/TProxy.log 2>&1 &

start4:
	ps aux|grep redis|grep -v grep|awk '{print "kill -9 "$$2}'|/bin/sh
	ps aux|grep TProxy|grep -v grep|awk '{print "kill -9 "$$2}'|/bin/sh
	nohup redis-server > /data/log/redis-server.log 2>&1 &
	nohup /data/ASIACCS17/Proxy/TProxy 4 > /data/log/TProxy.log 2>&1 &

start8:
	ps aux|grep redis|grep -v grep|awk '{print "kill -9 "$$2}'|/bin/sh
	ps aux|grep TProxy|grep -v grep|awk '{print "kill -9 "$$2}'|/bin/sh
	nohup redis-server > /data/log/redis-server.log 2>&1 &
	nohup /data/ASIACCS17/Proxy/TProxy 8 > /data/log/TProxy.log 2>&1 &

delete:
	redis-cli FLUSHALL
