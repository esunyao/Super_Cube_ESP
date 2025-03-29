#pragma once

// 基本协议支持
#define LWIP_IPV4                1
#define LWIP_IPV6                1
#define LWIP_DNS                 1

// TCP配置
#define TCP_WND                  500
#define TCP_SND_BUF              700
#define TCP_RCV_BUF              700
#define MEMP_NUM_TCP_PCB         2

// DNS配置
#define LWIP_DNS_ADDRTYPE_IPV4   0
#define LWIP_DNS_ADDRTYPE_IPV6   1
#define LWIP_DNS_ADDRTYPE_IPV4_IPV6 2

// 包含核心头文件
#include "lwip/opt.h"
#include "lwip/ip_addr.h"
#include "lwip/dns.h"