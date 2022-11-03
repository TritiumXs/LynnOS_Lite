/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "IpcMqTest.h"
#include <mqueue.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "log.h"
#include "utils.h"
#include "KernelConstants.h"
#include <securec.h>

#include "ohos_types.h"
#include "hctest.h"

LITE_TEST_SUIT(IPC, IpcMqApi, IpcMqApiTestSuite);

static BOOL IpcMqApiTestSuiteSetUp(void)
{
    return TRUE;
}

static BOOL IpcMqApiTestSuiteTearDown(void)
{
    printf("+-------------------------------------------+\n");
    return TRUE;
}

/* *
 * @tc.number SUB_KERNEL_IPC_MQ_0100
 * @tc.name   mq_send and mq_receive function test
 * @tc.desc   [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(IpcMqApiTestSuite, testMqOneLevelCom, Function | MediumTest | Level0)
{
    mqd_t queue;
    unsigned int prio;
    struct mq_attr getAttr = { 0 };
    struct mq_attr setAttr = { 0 };
    char qName[MQ_NAME_LEN], rMsg[MQ_RX_LEN];

    sprintf_s(qName, sizeof(qName), "testMqOneLevelCom_%d", GetRandom(10000));
    setAttr.mq_msgsize = MQ_MSG_SIZE;
    setAttr.mq_maxmsg = MQ_MAX_MSG;
    queue = mq_open(qName, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, &setAttr);
    TEST_ASSERT_TRUE(queue != (mqd_t)-1);
    TEST_ASSERT_TRUE(mq_send(queue, MQ_MSG, MQ_MSG_LEN, MQ_MSG_PRIO) == 0);

    TEST_ASSERT_TRUE(mq_getattr(queue, &getAttr) == 0);
    TEST_ASSERT_TRUE(getAttr.mq_msgsize == MQ_MSG_SIZE);
    TEST_ASSERT_TRUE(getAttr.mq_maxmsg == MQ_MAX_MSG);

    TEST_ASSERT_TRUE(mq_receive(queue, rMsg, getAttr.mq_msgsize, &prio) != -1);
    TEST_ASSERT_TRUE(prio == 0);
    TEST_ASSERT_TRUE(strncmp(MQ_MSG, rMsg, MQ_MSG_LEN) == 0);
    TEST_ASSERT_TRUE(mq_close(queue) == 0);
    TEST_ASSERT_TRUE(mq_unlink(qName) == 0);
}

/* *
 * @tc.number SUB_KERNEL_IPC_MQ_0200
 * @tc.name   mq_timedsend and mq_timedreceive function test
 * @tc.desc   [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(IpcMqApiTestSuite, testMqTimedOneLevelCom, Function | MediumTest | Level1)
{
    mqd_t queue;
    unsigned int prio;
    struct timespec tts = { 0 }, rts = { 0 };
    struct mq_attr getAttr = { 0 };
    struct mq_attr setAttr = { 0 };
    char qName[MQ_NAME_LEN], rMsg[MQ_RX_LEN];

    sprintf_s(qName, sizeof(qName), "testMqTimedOneLevelCom_%d", GetRandom(10000));
    setAttr.mq_msgsize = MQ_MSG_SIZE;
    setAttr.mq_maxmsg = MQ_MAX_MSG;
    queue = mq_open(qName, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, &setAttr);
    TEST_ASSERT_TRUE(queue != (mqd_t)-1);

    tts.tv_sec = time(NULL) + 1;
    tts.tv_nsec = 0;
    TEST_ASSERT_TRUE(mq_timedsend(queue, MQ_MSG, MQ_MSG_LEN, MQ_MSG_PRIO, &tts) == 0);

    TEST_ASSERT_TRUE(mq_getattr(queue, &getAttr) == 0);
    TEST_ASSERT_TRUE(getAttr.mq_msgsize == MQ_MSG_SIZE);
    TEST_ASSERT_TRUE(getAttr.mq_maxmsg == MQ_MAX_MSG);
    rts.tv_sec = time(NULL) + 1;
    rts.tv_nsec = 0;
    TEST_ASSERT_TRUE(mq_timedreceive(queue, rMsg, getAttr.mq_msgsize, &prio, &rts) != -1);
    TEST_ASSERT_TRUE(prio == 0);
    TEST_ASSERT_TRUE(strncmp(MQ_MSG, rMsg, MQ_MSG_LEN) == 0);
    TEST_ASSERT_TRUE(mq_close(queue) == 0);
    TEST_ASSERT_TRUE(mq_unlink(qName) == 0);
}

/* *
 * @tc.number SUB_KERNEL_IPC_MQ_0300
 * @tc.name   all send and all receive function test
 * @tc.desc   [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(IpcMqApiTestSuite, testMqAllOneLevelCom, Function | MediumTest | Level2)
{
    mqd_t queue;
    unsigned int prio;
    struct timespec tts = { 0 }, rts = { 0 };
    struct mq_attr getAttr = { 0 };
    struct mq_attr setAttr = { 0 };
    char qName[MQ_NAME_LEN], rMsg[MQ_RX_LEN];

    sprintf_s(qName, sizeof(qName), "testMqAllOneLevelCom_%d", GetRandom(10000));

    memset_s(&getAttr, sizeof(getAttr), 0, sizeof(getAttr));
    memset_s(&setAttr, sizeof(setAttr), 0, sizeof(setAttr));
    setAttr.mq_msgsize = MQ_MSG_SIZE;
    setAttr.mq_maxmsg = MQ_MAX_MSG;
    queue = mq_open(qName, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, &setAttr);
    TEST_ASSERT_TRUE(queue != (mqd_t)-1);

    TEST_ASSERT_TRUE(mq_getattr(queue, &setAttr) == 0);
    setAttr.mq_msgsize = MQ_MSG_SIZE;
    setAttr.mq_maxmsg = MQ_MAX_MSG;
    TEST_ASSERT_TRUE(mq_setattr(queue, &setAttr, NULL) == 0);
    TEST_ASSERT_TRUE(mq_send(queue, MQ_MSG, MQ_MSG_LEN, MQ_MSG_PRIO) == 0);

    TEST_ASSERT_TRUE(mq_getattr(queue, &getAttr) == 0);
    TEST_ASSERT_TRUE(getAttr.mq_msgsize == setAttr.mq_msgsize);
    TEST_ASSERT_TRUE(getAttr.mq_maxmsg == setAttr.mq_maxmsg);
    TEST_ASSERT_TRUE(getAttr.mq_curmsgs == 1);

    TEST_ASSERT_TRUE(mq_receive(queue, rMsg, getAttr.mq_msgsize, &prio) != -1);
    TEST_ASSERT_TRUE(strncmp(MQ_MSG, rMsg, MQ_MSG_LEN) == 0);

    tts.tv_sec = time(NULL) + 1;
    tts.tv_nsec = 0;
    TEST_ASSERT_TRUE(mq_timedsend(queue, MQ_MSG, MQ_MSG_LEN, MQ_MSG_PRIO, &tts) == 0);

    TEST_ASSERT_TRUE(mq_getattr(queue, &getAttr) == 0);
    TEST_ASSERT_TRUE(getAttr.mq_msgsize == setAttr.mq_msgsize);
    TEST_ASSERT_TRUE(getAttr.mq_maxmsg == setAttr.mq_maxmsg);
    TEST_ASSERT_TRUE(getAttr.mq_curmsgs == 1);

    rts.tv_sec = time(NULL) + 1;
    rts.tv_nsec = 0;
    TEST_ASSERT_TRUE(mq_timedreceive(queue, rMsg, getAttr.mq_msgsize, &prio, &rts) != -1);
    TEST_ASSERT_TRUE(strncmp(MQ_MSG, rMsg, MQ_MSG_LEN) == 0);

    TEST_ASSERT_TRUE(mq_close(queue) == 0);
    TEST_ASSERT_TRUE(mq_unlink(qName) == 0);
}

static void *PthreadCom(void *arg)
{
    mqd_t queue;
    unsigned int prio;
    struct mq_attr getAttr = { 0 };
    char rMsg[MQ_RX_LEN];

    queue = (mqd_t)arg;
    TEST_ASSERT_TRUE(mq_getattr(queue, &getAttr) == 0);
    TEST_ASSERT_TRUE(mq_receive(queue, rMsg, getAttr.mq_msgsize, &prio) != -1);
    TEST_ASSERT_TRUE(prio == MQ_MSG_PRIO);
    TEST_ASSERT_TRUE(strncmp(MQ_MSG, rMsg, MQ_MSG_LEN) == 0);
    TEST_ASSERT_TRUE(mq_send(queue, MQ_MSG, MQ_MSG_LEN, MQ_MSG_PRIO) == 0);

    return nullptr;
}

/* *
 * @tc.number SUB_KERNEL_IPC_MQ_0700
 * @tc.name   mq_send and mq_receive function test in thread and process
 * @tc.desc   [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(IpcMqApiTestSuite, testMqTwoThreadCom, Function | MediumTest | Level1)
{
    mqd_t queue;
    pthread_t tid;
    unsigned int prio;
    struct mq_attr getAttr = { 0 };
    struct mq_attr setAttr = { 0 };
    char qName[MQ_NAME_LEN], rMsg[MQ_RX_LEN];

    sprintf_s(qName, sizeof(qName), "testMqTwoLevelCom_%d", GetRandom(10000));
    setAttr.mq_msgsize = MQ_MSG_SIZE;
    setAttr.mq_maxmsg = MQ_MAX_MSG;
    queue = mq_open(qName, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, &setAttr);
    TEST_ASSERT_TRUE(queue != (mqd_t)-1);

    TEST_ASSERT_TRUE(pthread_create(&tid, NULL, PthreadCom, (void *)queue) != -1);

    TEST_ASSERT_TRUE(mq_send(queue, MQ_MSG, MQ_MSG_LEN, MQ_MSG_PRIO) == 0);
    TEST_ASSERT_TRUE(pthread_join(tid, NULL) == 0);

    TEST_ASSERT_TRUE(mq_getattr(queue, &getAttr) == 0);
    TEST_ASSERT_TRUE(getAttr.mq_curmsgs == 1);
    TEST_ASSERT_TRUE(mq_receive(queue, rMsg, getAttr.mq_msgsize, &prio) != -1);
    TEST_ASSERT_TRUE(prio == MQ_MSG_PRIO);
    TEST_ASSERT_TRUE(strncmp(MQ_MSG, rMsg, MQ_MSG_LEN) == 0);

    TEST_ASSERT_TRUE(mq_close(queue) == 0);
    TEST_ASSERT_TRUE(mq_unlink(qName) == 0);
}

static void *PthreadTimedCom(void *arg)
{
    mqd_t queue;
    unsigned int prio;
    struct timespec tts = { 0 }, rts = { 0 };
    struct mq_attr getAttr = { 0 };
    char rMsg[MQ_RX_LEN];

    queue = (mqd_t)arg;
    TEST_ASSERT_TRUE(mq_getattr(queue, &getAttr) == 0);
    rts.tv_sec = time(NULL) + 1;
    rts.tv_nsec = 0;
    TEST_ASSERT_TRUE(mq_timedreceive(queue, rMsg, getAttr.mq_msgsize, &prio, &rts) != -1);
    TEST_ASSERT_TRUE(prio == MQ_MSG_PRIO);
    TEST_ASSERT_TRUE(strncmp(MQ_MSG, rMsg, MQ_MSG_LEN) == 0);

    tts.tv_sec = time(NULL) + 1;
    tts.tv_nsec = 0;
    TEST_ASSERT_TRUE(mq_timedsend(queue, MQ_MSG, MQ_MSG_LEN, MQ_MSG_PRIO, &tts) == 0);

    return nullptr;
}

/* *
 * @tc.number SUB_KERNEL_IPC_MQ_0800
 * @tc.name   mq_timedsend and mq_timedreceive function test in thread and process
 * @tc.desc   [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(IpcMqApiTestSuite, testMqTimedTwoThreadCom, Function | MediumTest | Level1)
{
    mqd_t queue;
    pthread_t tid;
    unsigned int prio;
    struct timespec tts = { 0 }, rts = { 0 };
    struct mq_attr getAttr = { 0 };
    struct mq_attr setAttr = { 0 };
    char qName[MQ_NAME_LEN], rMsg[MQ_RX_LEN];

    sprintf_s(qName, sizeof(qName), "testMqTimedTwoThreadCom_%d", GetRandom(10000));
    setAttr.mq_msgsize = MQ_MSG_SIZE;
    setAttr.mq_maxmsg = MQ_MAX_MSG;
    queue = mq_open(qName, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, &setAttr);
    TEST_ASSERT_TRUE(queue != (mqd_t)-1);

    TEST_ASSERT_TRUE(pthread_create(&tid, NULL, PthreadTimedCom, (void *)queue) != -1);

    tts.tv_sec = time(NULL) + 1;
    tts.tv_nsec = 0;
    TEST_ASSERT_TRUE(mq_timedsend(queue, MQ_MSG, MQ_MSG_LEN, MQ_MSG_PRIO, &tts) == 0);
    TEST_ASSERT_TRUE(pthread_join(tid, NULL) == 0);

    TEST_ASSERT_TRUE(mq_getattr(queue, &getAttr) == 0);
    TEST_ASSERT_TRUE(getAttr.mq_curmsgs == 1);
    rts.tv_sec = time(NULL) + 1;
    rts.tv_nsec = 0;
    TEST_ASSERT_TRUE(mq_timedreceive(queue, rMsg, getAttr.mq_msgsize, &prio, &rts) != -1);
    TEST_ASSERT_TRUE(prio == MQ_MSG_PRIO);
    TEST_ASSERT_TRUE(strncmp(MQ_MSG, rMsg, MQ_MSG_LEN) == 0);

    TEST_ASSERT_TRUE(mq_close(queue) == 0);
    TEST_ASSERT_TRUE(mq_unlink(qName) == 0);
}

static void *PthreadAllCom(void *arg)
{
    mqd_t queue;
    unsigned int prio;
    struct timespec tts = { 0 }, rts = { 0 };
    struct mq_attr getAttr = { 0 };
    char rMsg[MQ_RX_LEN];

    queue = (mqd_t)arg;
    TEST_ASSERT_TRUE(mq_getattr(queue, &getAttr) == 0);
    rts.tv_sec = time(NULL) + 1;
    rts.tv_nsec = 0;
    TEST_ASSERT_TRUE(mq_timedreceive(queue, rMsg, getAttr.mq_msgsize, &prio, &rts) != -1);
    TEST_ASSERT_TRUE(prio == MQ_MSG_PRIO);
    TEST_ASSERT_TRUE(strncmp(MQ_MSG, rMsg, MQ_MSG_LEN) == 0);
    tts.tv_sec = time(NULL) + 1;
    tts.tv_nsec = 0;
    TEST_ASSERT_TRUE(mq_timedsend(queue, MQ_MSG, MQ_MSG_LEN, MQ_MSG_PRIO, &tts) == 0);

    return nullptr;
}

/* *
 * @tc.number SUB_KERNEL_IPC_MQ_0900
 * @tc.name   all send and all receive function test in thread and process
 * @tc.desc   [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(IpcMqApiTestSuite, testMqAllTwoThreadCom, Function | MediumTest | Level1)
{
    mqd_t queue;
    pthread_t tid;
    unsigned int prio;
    struct mq_attr getAttr = { 0 };
    struct mq_attr setAttr = { 0 };
    char qName[MQ_NAME_LEN], rMsg[MQ_RX_LEN];

    sprintf_s(qName, sizeof(qName), "testMqAllTwoThreadCom_%d", GetRandom(10000));
    setAttr.mq_msgsize = MQ_MSG_SIZE;
    setAttr.mq_maxmsg = MQ_MAX_MSG;
    queue = mq_open(qName, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, &setAttr);
    TEST_ASSERT_TRUE(queue != (mqd_t)-1);

    TEST_ASSERT_TRUE(pthread_create(&tid, NULL, PthreadAllCom, (void *)queue) != -1);

    TEST_ASSERT_TRUE(mq_send(queue, MQ_MSG, MQ_MSG_LEN, MQ_MSG_PRIO) == 0);
    TEST_ASSERT_TRUE(pthread_join(tid, NULL) == 0);

    TEST_ASSERT_TRUE(mq_getattr(queue, &getAttr) == 0);
    TEST_ASSERT_TRUE(getAttr.mq_curmsgs == 1);
    TEST_ASSERT_TRUE(mq_receive(queue, rMsg, getAttr.mq_msgsize, &prio) != -1);
    TEST_ASSERT_TRUE(prio == MQ_MSG_PRIO);
    TEST_ASSERT_TRUE(strncmp(MQ_MSG, rMsg, MQ_MSG_LEN) == 0);

    TEST_ASSERT_TRUE(mq_close(queue) == 0);
    TEST_ASSERT_TRUE(mq_unlink(qName) == 0);
}

/* *
 * @tc.number SUB_KERNEL_IPC_MQ_1200
 * @tc.name   mq_setattr set and clean mq_flags for O_NONBLOCK function test
 * @tc.desc   [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(IpcMqApiTestSuite, testMqSetGetAttr, Function | MediumTest | Level1)
{
    mqd_t queue;
    struct mq_attr getAttr = { 0 };
    struct mq_attr setAttr = { 0 };
    char qName[MQ_NAME_LEN];

    sprintf_s(qName, sizeof(qName), "testMqFunction_%d", GetRandom(10000));

    memset_s(&setAttr, sizeof(setAttr), 0, sizeof(setAttr));
    setAttr.mq_msgsize = MQ_MSG_SIZE;
    setAttr.mq_maxmsg = MQ_MAX_MSG;
    setAttr.mq_flags = O_NONBLOCK;
    queue = mq_open(qName, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, &setAttr);
    TEST_ASSERT_TRUE(queue != (mqd_t)-1);

    memset_s(&getAttr, sizeof(getAttr), 0, sizeof(getAttr));
    TEST_ASSERT_TRUE(mq_getattr(queue, &getAttr) == 0);
    TEST_ASSERT_TRUE((getAttr.mq_flags & O_NONBLOCK) == O_NONBLOCK);

    setAttr.mq_flags &= ~O_NONBLOCK;
    TEST_ASSERT_TRUE(mq_setattr(queue, &setAttr, NULL) == 0);

    TEST_ASSERT_TRUE(mq_getattr(queue, &getAttr) == 0);
    printf("setAttr.mq_flags = 0x%x, getAttr.mq_flags = 0x%x\n", setAttr.mq_flags, getAttr.mq_flags);
    TEST_ASSERT_TRUE((getAttr.mq_flags & O_NONBLOCK) == 0);
    TEST_ASSERT_TRUE(mq_close(queue) == 0);
    TEST_ASSERT_TRUE(mq_unlink(qName) == 0);
}

RUN_TEST_SUITE(IpcMqApiTestSuite);
