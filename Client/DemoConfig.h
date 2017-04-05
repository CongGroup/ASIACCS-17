#ifndef __DEMO_CONFIG_H__
#define __DEMO_CONFIG_H__

#define DEMO_SERVER_NUM 9

//#define DEF_INSERT_INDEX

#define DEMO_SECURITY_KEY "adfaksdfjklasdjflajsdiofjasodf"

const std::string kDemoServerIP[] = {
	"127.0.0.1"
};
const uint16_t kDemoServerPort[] = { 9090, 9090, 9090, 9090, 9090, 9090, 9090, 9090, 9090, 9090 };
const uint16_t kRedisPort[] = { 6379, 6379, 6379, 6379, 6379, 6379, 6379, 6379, 6379, 6379 };


#endif
