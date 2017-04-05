# ASIACCS2017

 * Introduction
 * Requirements
 * Installation
 * Configuration
 * Compiling
 * Usage
 * Redo Experiment
 * Maintainers

# INTRODUCTION

EncKV is an encrypted key-value store with secure rich query support. It stores encrypted data records with multiple secondary attributes in the form of encrypted key-value pairs. EncKV leverages the latest practical primitives for search over encrypted data, i.e., searchable symmetric encryption and order-revealing encryption, and provides encrypted indexes with guaranteed security respectively to enable exactmatch and range-match queries via secondary attributes of data records. EncKV carefully integrates the above indexes into a distributed index framework to facilitate secure query processing in parallel. To mitigate recent inference attacks on encrypted database systems, EncKV protects the order information during range queries, and presents
an interactive batch query mechanism to further hide the associations across data values on different attributes.

We implement the EncKV prototype on a Redis cluster, and conduct performance evaluation on Amazon Cloud. The results show that EncKV preserves the efficiency and scalability of plaintext distributed key-value stores.

# PUBLICATION

Xingliang Yuan, Yu Guo, Xinyu Wang, Cong Wang, Baochun Li, and Xiaohua Jia, "EncKV: An Encrypted Key-value Store with Rich Queries", In the 12th ACM Asia Conference on Computer and Communications Security (AISACCS'17).

# REQUIREMENTS

Recommended Environment: Ubuntu 16.04 LTS with gcc version 4.8.4.

This software requires the following libraries:

 * OpenSSL (https://www.openssl.org/source/openssl-1.0.2a.tar.gz)
 * Thrift (http://archive.apache.org/dist/thrift/0.9.2/)
 * boost C++ library (http://sourceforge.net/projects/boost/files/boost/1.58.0/boost_1_58_0.tar.gz)
 * Redis (http://download.redis.io/releases/redis-3.2.0.tar.gz)
 * Redis3m (https://github.com/luca3m/redis3m)

# INSTALLATION

Environment setup:

```shell
 * apt-get update
 * apt-get install gcc g++ libssl-dev libgmp-dev make cmake libboost-dev libboost-test-dev libboost-program-options-dev libboost-system-dev libboost-filesystem-dev libevent-dev automake libtool flex bison pkg-config libglib2.0-dev git
 * apt-get install libmsgpack-dev libboost-thread-dev libboost-date-time-dev libboost-test-dev libboost-filesystem-dev libboost-system-dev libhiredis-dev cmake build-essential libboost-regex-dev
```

Thrift installation:

You can find the HTTP link on 

```shell
 * wget http://archive.apache.org/dist/thrift/0.9.2/thrift-0.9.2.tar.gz
 * tar zxvf 
 * cd 
 * make
 * make install
```

Redis installation:

```shell
 * wget http://download.redis.io/releases/redis-3.2.0.tar.gz
 * tar zxvf redis-3.2.0.tar.gz
 * cd redis-3.2.0
 * make
 * make install
 ```

redis3m (a C++ Redis client) installation:

```shell
 * git clone https://github.com/luca3m/redis3m
 * cd redis3m
 * cmake .
 * make
 * make install
```

# CONFIGURATION


 * Configure the Redis
	Start the redis server listening on port 6379.

	```shell
	* redis-server &
	```

 * Configure the environment
	Add the libraries paths to $LD_LIBRARY_PATH.

	```shell
	* export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib
	```
 * Configure IPs and Ports
	Input IP and PORT of each machine to an configuration file in /BlindDB/src/Client/DemoConfig.h

	```cpp
	#define DEMO_SECURITY_KEY "adfaksdfjklasdjflajsdiofjasodf"
	const std::string kDemoServerIP[] = { "10.4.0.5", "10.4.0.6", "10.4.0.9", "10.4.0.10", "10.4.0.11", "10.4.0.12", "10.4.0.13", "10.4.0.14", "10.4.0.15", "10.4.0.16" };
	const uint16_t kDemoServerPort[] = { 9090, 9090, 9090, 9090, 9090, 9090, 9090, 9090, 9090, 9090 };
	const uint16_t kRedisPort[] = { 6379, 6379, 6379, 6379, 6379, 6379, 6379, 6379, 6379, 6379 };

	```

# COMPILING

 Compile EncKV:

```shell
 * git clone https://github.com/CongGroup/ASIACCS-17.git
 * cd ASIACCS-17
 * make
```

# USAGE

  * Test_insert
	
	Scripts that insert test data into EncKV.

	```
	Usage :	ASIACCS-17/Client/Test_insert [DataNodeNum] [BegNum] [EndNum] [EqualIndexVersion] [OrderIndex] [BlockSizeInBit] [ModNum]

	- [NodeNum]: the number of nodes.
	- [BegNum]: the begin of random insertion number.
	- [EndNum]: the End of random insertion number.
	- [EqualIndexVersion]: 0 means NO index, 1 means strategy I equal index, 2 means strategy II equal index, 3 means plaintext equal index.
	- [OrderIndex]: 0 means NO index, 1 means encrypted order index, 2 means plaintext order index.
	- [BlockSizeInBit]: the blocksize of ORE in bit.
	- [ModNum]: This param is to insert multiple records with a same value. 0 means no mod operation, for others, the insertion value will mod this param.
	```


  * TestCT.sh
	
	Scripts that generate ciphertext workloads


	```
	Usage : /BlindDB/src/Client/TestCT.sh [NodeNum] [LOOP] [OPTION] [TIME] [SEEDS]

	- [NodeNum]: the number of nodes.
	- [LOOP]: the number of clients.
	- [OPTION]: "0" put only workload, "1" get only workload, "2" get heavy workload, "3" put heavy workload.
	- [TIME]: time duration for the workload.
	- [SEEDS]:  random seed.
	```


  * Ciphertext Latency

	Test the latency of secure PUT/GET operations


	```
	Usage : /BlindDB/src/Client/Ciphertext_latency [DataNodeNum] [KeyLen] [ValLen] [Times]

	- [DataNodeNum]: the number of nodes.
	- [KeyLen]: the length of key in bytes.
	- [ValLen]: the length of value in bytes.
	- [Times]: times of operations.
	```


  * Plaintext Cluster Latency

	Test the latency of PUT/GET operations 

	```
	Usage : /BlindDB/src/Client/Plaintext_cluster_latency [DataNodeNum] [KeyLen] [ValLen] [Times]

	- [DataNodeNum]: the number of nodes.
	- [KeyLen]: the length of key in bytes.
	- [ValLen]: the length of value in bytes.
	- [Times]: times of operations.
	```

# REDO EXPERIMENT
	
	
# MAINTAINER

 * Current maintainer

	- Xinyu Wang, City University of Hong Kong, xinyucs@gmail.com
	- Xingliang Yuan, City University of Hong Kong, xyuancs@gmail.com
	- Mengyu Yao, City University of Hong Kong, mengycs@gmail.com
