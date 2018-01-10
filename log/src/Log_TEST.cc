/*
 * Copyright (C) 2017 Open Source Robotics Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
*/

#include "ignition/transport/log/Log.hh"
#include "gtest/gtest.h"

using namespace ignition;

//////////////////////////////////////////////////
TEST(Log, OpenMemoryDatabase)
{
  transport::log::Log logFile;
  EXPECT_TRUE(logFile.Open(":memory:", std::ios_base::out));
}

//////////////////////////////////////////////////
TEST(Log, OpenImpossibleFileName)
{
  transport::log::Log logFile;
  EXPECT_FALSE(logFile.Open("///////////", std::ios_base::out));
}

//////////////////////////////////////////////////
TEST(Log, InsertMessage)
{
  transport::log::Log logFile;
  ASSERT_TRUE(logFile.Open(":memory:", std::ios_base::out));

  std::string data("Hello World");

  EXPECT_TRUE(logFile.InsertMessage(
      common::Time(),
      "/some/topic/name",
      "some.message.type",
      reinterpret_cast<const void *>(data.c_str()),
      data.size()));
}

//////////////////////////////////////////////////
TEST(Log, AllMessagesNone)
{
  transport::log::Log logFile;
  ASSERT_TRUE(logFile.Open(":memory:", std::ios_base::out));

  auto batch = logFile.AllMessages();
  EXPECT_EQ(transport::log::MsgIter(), batch.begin());
}

//////////////////////////////////////////////////
TEST(Log, InsertMessageGetMessages)
{
  transport::log::Log logFile;
  ASSERT_TRUE(logFile.Open(":memory:", std::ios_base::out));

  std::string data1("first_data");
  std::string data2("second_data");

  EXPECT_TRUE(logFile.InsertMessage(
      common::Time(1, 0),
      "/some/topic/name",
      "some.message.type",
      reinterpret_cast<const void *>(data1.c_str()),
      data1.size()));

  EXPECT_TRUE(logFile.InsertMessage(
      common::Time(2, 0),
      "/some/topic/name",
      "some.message.type",
      reinterpret_cast<const void *>(data2.c_str()),
      data2.size()));

  auto batch = logFile.AllMessages();
  auto iter = batch.begin();
  ASSERT_NE(transport::log::MsgIter(), iter);
  EXPECT_EQ(data1, iter->Data());
  ++iter;
  ASSERT_NE(transport::log::MsgIter(), iter);
  EXPECT_EQ(data2, iter->Data());
  ++iter;
  EXPECT_EQ(transport::log::MsgIter(), iter);
}

//////////////////////////////////////////////////
TEST(Log, QueryMessagesByTopicNone)
{
  transport::log::Log logFile;
  ASSERT_TRUE(logFile.Open(":memory:", std::ios_base::out));

  std::unordered_set<std::string> noTopics;
  auto batch = logFile.QueryMessages(noTopics);
  EXPECT_EQ(transport::log::MsgIter(), batch.begin());
}

//////////////////////////////////////////////////
TEST(Log, Insert2Get1MessageByTopic)
{
  transport::log::Log logFile;
  ASSERT_TRUE(logFile.Open(":memory:", std::ios_base::out));

  std::string data1("first_data");
  std::string data2("second_data");

  EXPECT_TRUE(logFile.InsertMessage(
      common::Time(1, 0),
      "/some/topic/name",
      "some.message.type",
      reinterpret_cast<const void *>(data1.c_str()),
      data1.size()));

  EXPECT_TRUE(logFile.InsertMessage(
      common::Time(2, 0),
      "/second/topic/name",
      "some.message.type",
      reinterpret_cast<const void *>(data2.c_str()),
      data2.size()));

  auto batch = logFile.QueryMessages({"/some/topic/name"});
  auto iter = batch.begin();
  ASSERT_NE(transport::log::MsgIter(), iter);
  EXPECT_EQ(data1, iter->Data());
  ++iter;
  EXPECT_EQ(transport::log::MsgIter(), iter);
}

//////////////////////////////////////////////////
TEST(Log, CheckVersion)
{
  transport::log::Log logFile;
  ASSERT_TRUE(logFile.Open(":memory:", std::ios_base::out));
  EXPECT_EQ("0.1.0", logFile.Version());
}

//////////////////////////////////////////////////
int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
