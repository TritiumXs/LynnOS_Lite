/**
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *   http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <unistd.h>
#include <stdio.h>
#include <securec.h>
#include <pthread.h>
#include <time.h>
#include <sys/resource.h>
#include <limits.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <poll.h>
#include <sys/ioctl.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <netinet/ether.h>
#include <net/if.h>
#include <netdb.h>
#include <arpa/nameser.h>
#include "ohos_types.h"
#include "hctest.h"
#include "utils.h"
#include "log.h"
#include "lwip/sockets.h"

#define PROTOCOL_COUNT 36
#define TEST_FD_COUNT 10
#define STACK_PORT 2288
#define PEER_PORT 2288
#define BUF_SIZE (100)
static const char* g_udpMsg = "Hi, this is UDP";
static const char* g_srvMsg = "Hi, this is TCP server";
static const char* g_cliMsg = "Hello, this is TCP client";
static const char* g_localHost = "127.0.0.1";
static char g_protocolList[PROTOCOL_COUNT][11] = {
    "ip", "icmp", "igmp", "ggp", "ipencap", "st", "tcp",
    "egp", "pup", "udp", "hmp", "xns-idp", "rdp", "iso-tp4", "xtp", "ddp", "idpr-cmtp", "ipv6",
    "ipv6-route", "ipv6-frag", "idrp", "rsvp", "gre", "esp", "ah", "skip", "ipv6-icmp", "ipv6-nonxt",
    "ipv6-opts", "rspf", "vmtp", "ospf", "ipip", "encap", "pim", "raw"
};
static int g_protocolNumber[PROTOCOL_COUNT] = {
    0, 1, 2, 3, 4, 5, 6, 8, 12, 17, 20, 22, 27, 29, 36, 37,
    38, 41, 43, 44, 45, 46, 47, 50, 51, 57, 58, 59, 60, 73, 81, 89, 94, 98, 103, 255
};

static pthread_barrier_t g_barrier;
#define WAIT() pthread_barrier_wait(&g_barrier)

LITE_TEST_SUIT(NET, ActsNet, ActsNetTestSuite);

static BOOL ActsNetTestSuiteSetUp(void)
{
    return TRUE;
}

static BOOL ActsNetTestSuiteTearDown(void)
{
    LOG("+-------------------------------------------+\n");
    return TRUE;
}

/**
 * @tc.number    : SUB_KERNEL_NET_0730
 * @tc.name      : test ioctl get and set IFHWADDR
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(ActsNetTestSuite, testIoctlIfhwAddr, Function | MediumTest | Level2)
{
    int udpFd = socket(AF_INET, SOCK_DGRAM, 0);
    ASSERT_NE(-1, udpFd);

    struct ifreq ifr[5];
    struct ifconf ifc = {0};
    memset_s(&ifc, sizeof(struct ifconf), 0, sizeof(struct ifconf));
    ifc.ifc_len = 5 * sizeof(struct ifreq);
    ifc.ifc_buf = (char *)ifr;
    int ret = ioctl(udpFd, SIOCGIFCONF, (char *)&ifc);
    ASSERT_EQ(0, ret);

    char rst1[18];
    char rst2[18];
    char* macPtr = NULL;
    struct ifreq ifrTmp = {0};
    struct sockaddr_in *addr = NULL;
    int ifrCount = ifc.ifc_len / sizeof(struct ifreq);
    EXPECT_TRUE(ifrCount >= 2);
    for (int i = 0; i < ifrCount; i++) {
        addr = (struct sockaddr_in *)&ifr[i].ifr_addr;
        if (strcmp("lo", ifr[i].ifr_name) != 0) {
            // get ifhwaddr
            memset_s(&ifrTmp, sizeof(struct ifreq), 0, sizeof(struct ifreq));
            ret = strcpy_s(ifrTmp.ifr_name, sizeof(ifrTmp.ifr_name), ifr[i].ifr_name);
            EXPECT_EQ(0, ret);
            ret = ioctl(udpFd, SIOCGIFHWADDR, &ifrTmp);
            ASSERT_EQ(0, ret);
            macPtr = ifrTmp.ifr_hwaddr.sa_data;
            ret = sprintf_s(rst1, sizeof(rst1), "%.2x:%.2x:%.2x:%.2x:%.2x:%.2x", *macPtr, *(macPtr+1),
                *(macPtr+2), *(macPtr+3), *(macPtr+4), *(macPtr+5));
            EXPECT_EQ(strlen(rst1), (unsigned int)ret);
            printf("[###]get %s rst[%s]\n", ifrTmp.ifr_name, rst1);
        }
    }
    ret = close(udpFd);
    EXPECT_EQ(0, ret);
}

/**
 * @tc.number    : SUB_KERNEL_NET_1000
 * @tc.name      : test socket operation
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(ActsNetTestSuite, testSocketOpt, Function | MediumTest | Level2)
{
    socklen_t len;
    struct timeval timeout = {0};
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    EXPECT_NE(-1, fd);

    int error = -1;
    len = sizeof(error);
    int ret = getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, &len);
    EXPECT_EQ(0, ret);
    EXPECT_EQ(0, error);

    len = sizeof(timeout);
    ret = getsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, &len);
    EXPECT_EQ(0, ret);

    timeout.tv_sec = 1000;
    len = sizeof(timeout);
    ret = setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, len);
    EXPECT_EQ(0, ret);

    memset_s(&timeout, len, 0, len);
    ret = getsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, &len);
    EXPECT_EQ(0, ret);
    EXPECT_EQ(1000, timeout.tv_sec);

    int flag = 1;
    ret = setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag));
    EXPECT_EQ(0, ret);

    flag = 0;
    len = sizeof(flag);
    ret = getsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &flag, &len);
    EXPECT_EQ(0, ret);
    EXPECT_EQ(1, flag);

    error = -1;
    len = sizeof(error);
    ret = getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, &len);
    EXPECT_EQ(0, ret);
    EXPECT_EQ(0, error);

    ret = close(fd);
    EXPECT_EQ(0, ret);
}

/**
 * @tc.number    : SUB_KERNEL_NET_1100
 * @tc.name      : test getsockname and getpeername invalid input
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(ActsNetTestSuite, testGetSocketNameInvalidInput, Function | MediumTest | Level3)
{
    struct sockaddr addr = {0};
    socklen_t addrLen = sizeof(addr);
    int ret = getsockname(-1, &addr, &addrLen);
    EXPECT_EQ(-1, ret);
    ret = getpeername(-1, &addr, &addrLen);
    EXPECT_EQ(-1, ret);
    ret = getsockname(0, &addr, &addrLen);
    EXPECT_EQ(-1, ret);
    ret = getpeername(0, &addr, &addrLen);
    EXPECT_EQ(-1, ret);
    ret = getsockname(1, &addr, &addrLen);
    EXPECT_EQ(-1, ret);
    ret = getpeername(1, &addr, &addrLen);
    EXPECT_EQ(-1, ret);
    ret = getsockname(130, &addr, &addrLen);
    EXPECT_EQ(-1, ret);
    ret = getpeername(130, &addr, &addrLen);
    EXPECT_EQ(-1, ret);
    ret = getsockname(10, NULL, &addrLen);
    EXPECT_EQ(-1, ret);
    ret = getpeername(10, NULL, &addrLen);
    EXPECT_EQ(-1, ret);
    ret = getsockname(10, &addr, NULL);
    EXPECT_EQ(-1, ret);
    ret = getpeername(10, &addr, NULL);
    EXPECT_EQ(-1, ret);
}

/**
 * @tc.number    : SUB_KERNEL_NET_2400
 * @tc.name      : test convert value from host to network byte order
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(ActsNetTestSuite, testHostToNetwork, Function | MediumTest | Level2)
{
    uint32_t intInput1 = 0;
    uint32_t intRst1 = htonl(intInput1);
    uint32_t intInput2 = 65536;
    uint32_t intRst2 = htonl(intInput2);

    uint16_t shortInput1 = 0;
    uint16_t shortRst1 = htons(shortInput1);
    uint16_t shortInput2 = 255;
    uint16_t shortRst2 = htons(shortInput2);

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    uint32_t expectZero = 0;
    uint32_t expectForIinput2 = 256;
    uint32_t expectForSinput2 = 65280;
    EXPECT_EQ(expectZero, intRst1);
    EXPECT_EQ(expectForIinput2, intRst2);
    EXPECT_EQ(expectZero, shortRst1);
    EXPECT_EQ(expectForSinput2, shortRst2);
#else
    EXPECT_EQ(intInput1, intRst1);
    EXPECT_EQ(intInput2, intRst2);
    EXPECT_EQ(shortInput1, shortRst1);
    EXPECT_EQ(shortInput2, shortRst2);
#endif
}

/**
 * @tc.number    : SUB_KERNEL_NET_2500
 * @tc.name      : test convert value from network to host byte order
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(ActsNetTestSuite, testNetworkToHost, Function | MediumTest | Level2)
{
    uint32_t intInput1 = 0;
    uint32_t intRst1 = ntohl(intInput1);
    uint32_t intInput2 = 65536;
    uint32_t intRst2 = ntohl(intInput2);
    uint16_t shortInput1 = 0;
    uint16_t shortRst1 = ntohs(shortInput1);
    uint16_t shortInput2 = 255;
    uint16_t shortRst2 = ntohs(shortInput2);

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    uint32_t expectZero = 0;
    uint32_t expectForIinput2 = 256;
    uint32_t expectForSinput2 = 65280;
    EXPECT_EQ(expectZero, intRst1);
    EXPECT_EQ(expectForIinput2, intRst2);
    EXPECT_EQ(expectZero, shortRst1);
    EXPECT_EQ(expectForSinput2, shortRst2);
#else
    EXPECT_EQ(intInput1, intRst1);
    EXPECT_EQ(intInput2, intRst2);
    EXPECT_EQ(shortInput1, shortRst1);
    EXPECT_EQ(shortInput2, shortRst2);
#endif
}

/**
 * @tc.number    : SUB_KERNEL_NET_2600
 * @tc.name      : test inet_pton IPv4 normal
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(ActsNetTestSuite, testInetPtonIpv4Normal, Function | MediumTest | Level2)
{
    int ret;
    struct in_addr rst = {0};
    char cpAddrs[4][16] = {"10.58.212.100", "0.0.0.0", "255.0.0.0", "255.255.255.255"};
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    unsigned int expectLittle[4] = {1691630090, 0, 255, 4294967295};
#else
    unsigned int expectBig[4] = {171627620, 0, 4278190080, 4294967295};
#endif

    for (int i = 0; i < 4; i++) {
        ret = inet_pton(AF_INET, cpAddrs[i], &rst);
        EXPECT_EQ(1, ret);
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
        EXPECT_EQ(expectLittle[i], rst.s_addr);
#else
        EXPECT_EQ(expectBig[i], rst.s_addr);
#endif
        printf("[###]inet_pton %s: un[%u],s[%d],hex[%x]\n", cpAddrs[i], rst.s_addr, rst.s_addr, rst.s_addr);
    }
}

/**
 * @tc.number    : SUB_KERNEL_NET_2800
 * @tc.name      : test inet_pton IPv6 normal
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(ActsNetTestSuite, testInetPtonIpv6Normal, Function | MediumTest | Level2)
{
    int ret;
    struct in6_addr rst = {0};
    char cpAddrs[6][40] = {"0101:0101:0101:0101:1010:1010:1010:1010", "0:0:0:0:0:0:0:0", 
        "FFFF:FFFF:FFFF:FFFF:FFFF:FFFF:FFFF:FFFF", "::", "1::", "0011:0011:0011:0011:11:11:11:11"};
    for (int i = 0; i < 6; i++) {
        ret = inet_pton(AF_INET6, cpAddrs[i], &rst);
printf("i = %d, cpAddrs = %s , ret = %d \n", i, cpAddrs[i], ret);
        EXPECT_EQ(1, ret);
    }
}

/**
 * @tc.number    : SUB_KERNEL_NET_2900
 * @tc.name      : test inet_pton IPv6 abnormal
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(ActsNetTestSuite, testInetPtonIpv6Abnormal, Function | MediumTest | Level2)
{
    int ret;
    struct in6_addr rst = {0};
    char cpAddrs[7][40] = {"127.0.0.1", "f", ":", "0:0", "1:::", ":::::::",
        "1111:1111:1111:1111:1111:1111:1111:111G"};
    for (int i = 0; i < 7; i++) {
        ret = inet_pton(AF_INET6, cpAddrs[i], &rst);
printf("i = %d, cpAddrs = %s , ret = %d \n", i, cpAddrs[i], ret);
        EXPECT_EQ(0, ret);
    }
}

/**
 * @tc.number    : SUB_KERNEL_NET_3000
 * @tc.name      : test inet_pton with invalid family
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(ActsNetTestSuite, testInetPtonInvalidFamily, Function | MediumTest | Level2)
{
    struct in_addr rst = {0};
    int ret = inet_pton(AF_IPX, "127.0.0.1", &rst);
    EXPECT_EQ(-1, ret);
    ret = inet_pton(-1, "127.0.0.1", &rst);
    EXPECT_EQ(-1, ret);
}

/**
 * @tc.number    : SUB_KERNEL_NET_3100
 * @tc.name      : test inet_ntop IPv4 normal
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(ActsNetTestSuite, testInetNtopIpv4Normal, Function | MediumTest | Level2)
{
    const char* ret = NULL;
    struct in_addr inputAddr = {0};
    char rstBuff[INET_ADDRSTRLEN];
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    unsigned int inputLittle[4] = {0x64d43a0a, 0, 255, 4294967295};
#else
    unsigned int inputBig[4] = {171627620, 0, 4278190080, 4294967295};
#endif

    char expectAddrs[4][16] = {"10.58.212.100", "0.0.0.0", "255.0.0.0", "255.255.255.255"};
    for (int i = 0; i < 4; i++) {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
        inputAddr.s_addr = inputLittle[i];
#else
        inputAddr.s_addr = inputBig[i];
#endif
        ret = inet_ntop(AF_INET, &inputAddr, rstBuff, sizeof(rstBuff));
        if (ret == NULL) {
            EXPECT_TRUE(ret != NULL);
        } else {
            printf("[###]inet_ntop expect [%s]: ret[%s], buf[%s]\n", expectAddrs[i], ret, rstBuff);
            EXPECT_STREQ(expectAddrs[i], ret);
            EXPECT_STREQ(expectAddrs[i], rstBuff);
        }
    }
}

/**
 * @tc.number    : SUB_KERNEL_NET_3200
 * @tc.name      : test inet_ntop IPv4 boundary input
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(ActsNetTestSuite, testInetNtopIpv4Abnormal, Function | MediumTest | Level2)
{
    const char* ret = NULL;
    struct in_addr inputAddr = {0};
    char rstBuff[INET_ADDRSTRLEN];
    char expectStr[2][16] = {"255.255.255.255", "0.0.0.0"};
    for (int i = 0; i < 2; i++) {
        inputAddr.s_addr = (i == 0 ? -1 : 4294967296);
        ret = inet_ntop(AF_INET, &inputAddr, rstBuff, sizeof(rstBuff));
        ASSERT_TRUE(ret != NULL);
        EXPECT_STREQ(expectStr[i], ret);
        EXPECT_STREQ(expectStr[i], rstBuff);
    }
}

/**
 * @tc.number    : SUB_KERNEL_NET_3500
 * @tc.name      : test inet_ntop with invalid family
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(ActsNetTestSuite, testInetNtopInvalidFamily, Function | MediumTest | Level2)
{
    int iret;
    const char* ret = NULL;
    struct in6_addr inputAddr = {0};
    char rstBuff[INET6_ADDRSTRLEN];

    iret = inet_pton(AF_INET6, "1::", &inputAddr);
printf("inputAddr = %s , ret = %d \n",  inputAddr, ret);
    ASSERT_EQ(1, iret);
    ret = inet_ntop(AF_IPX, &inputAddr, rstBuff, sizeof(rstBuff));
    EXPECT_TRUE(ret == NULL);
    ret = inet_ntop(-1, &inputAddr, rstBuff, sizeof(rstBuff));
    EXPECT_TRUE(ret == NULL);
}

RUN_TEST_SUITE(ActsNetTestSuite);

