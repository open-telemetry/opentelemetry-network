// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include <gtest/gtest.h>

#include <util/cgroup_parser.h>


TEST(cgroup_parser, systemd_happy_path)
{
  CGroupParser c("kubepods-burstable-pod146bb920_a47b_4f6c_a69a_166b63944d15.slice:cri-containerd:c45f3e9c19746eabf0a4af63d780ba5c2a657a7352c7ad7acc5d599da5115eef");
  EXPECT_TRUE(c.get().valid);
  EXPECT_EQ(c.get().qos, "burstable");
  EXPECT_EQ(c.get().pod_id, "146bb920-a47b-4f6c-a69a-166b63944d15");
  EXPECT_EQ(c.get().runtime, "containerd");
  EXPECT_EQ(c.get().container_id, "c45f3e9c19746eabf0a4af63d780ba5c2a657a7352c7ad7acc5d599da5115eef");
  EXPECT_EQ(c.get().service, "");

  CGroupParser c2("kubepods-besteffort-pod745edc0d_4e07_49ef_ba5d_576d66791bfb.slice");
  EXPECT_TRUE(c2.get().valid);
  EXPECT_EQ(c2.get().qos, "besteffort");
  EXPECT_EQ(c2.get().pod_id, "745edc0d-4e07-49ef-ba5d-576d66791bfb");
  EXPECT_EQ(c2.get().runtime, "");
  EXPECT_EQ(c2.get().container_id, "");
  EXPECT_EQ(c2.get().service, "");

  CGroupParser c3("kubepods-besteffort.slice");
  EXPECT_TRUE(c3.get().valid);
  EXPECT_EQ(c3.get().qos, "besteffort");
  EXPECT_EQ(c3.get().pod_id, "");
  EXPECT_EQ(c3.get().runtime, "");
  EXPECT_EQ(c3.get().container_id, "");
  EXPECT_EQ(c3.get().service, "");
}

TEST(cgroup_parser, cri_happy_path)
{
  CGroupParser c("cri-containerd-6f652f89943b50f7b101d13f11371daf34bf836b7e1b725b5e8b6439451018bd.scope");
  EXPECT_TRUE(c.get().valid);
  EXPECT_EQ(c.get().qos, "");
  EXPECT_EQ(c.get().pod_id, "");
  EXPECT_EQ(c.get().runtime, "containerd");
  EXPECT_EQ(c.get().container_id, "6f652f89943b50f7b101d13f11371daf34bf836b7e1b725b5e8b6439451018bd");
  EXPECT_EQ(c.get().service, "");
}

TEST(cgroup_parser, pod_happy_path)
{
  CGroupParser c("pod146bb920_a47b_4f6c_a69a_166b63944d15");
  EXPECT_TRUE(c.get().valid);
  EXPECT_EQ(c.get().qos, "");
  EXPECT_EQ(c.get().pod_id, "146bb920-a47b-4f6c-a69a-166b63944d15");
  EXPECT_EQ(c.get().runtime, "");
  EXPECT_EQ(c.get().container_id, "");
  EXPECT_EQ(c.get().service, "");

  CGroupParser c2("pod146bb920a47b4f6ca69a166b63944d15");
  EXPECT_TRUE(c2.get().valid);
  EXPECT_EQ(c2.get().qos, "");
  EXPECT_EQ(c2.get().pod_id, "146bb920-a47b-4f6c-a69a-166b63944d15");
  EXPECT_EQ(c2.get().runtime, "");
  EXPECT_EQ(c2.get().container_id, "");
  EXPECT_EQ(c2.get().service, "");

  CGroupParser c3("pod146bb920-a47b-4f6c-a69a-166b63944d15");
  EXPECT_TRUE(c3.get().valid);
  EXPECT_EQ(c3.get().qos, "");
  EXPECT_EQ(c3.get().pod_id, "146bb920-a47b-4f6c-a69a-166b63944d15");
  EXPECT_EQ(c3.get().runtime, "");
  EXPECT_EQ(c3.get().container_id, "");
  EXPECT_EQ(c3.get().service, "");
}

TEST(cgroup_parser, container_id_happy_path)
{
  CGroupParser c("6f652f89943b50f7b101d13f11371daf34bf836b7e1b725b5e8b6439451018bd");
  EXPECT_TRUE(c.get().valid);
  EXPECT_EQ(c.get().qos, "");
  EXPECT_EQ(c.get().pod_id, "");
  EXPECT_EQ(c.get().runtime, "");
  EXPECT_EQ(c.get().container_id, "6f652f89943b50f7b101d13f11371daf34bf836b7e1b725b5e8b6439451018bd");
  EXPECT_EQ(c.get().service, "");
}

TEST(cgroup_parser, service_happy_path)
{
  CGroupParser c("systemd-journald.service");
  EXPECT_TRUE(c.get().valid);
  EXPECT_EQ(c.get().qos, "");
  EXPECT_EQ(c.get().pod_id, "");
  EXPECT_EQ(c.get().runtime, "");
  EXPECT_EQ(c.get().container_id, "");
  EXPECT_EQ(c.get().service, "systemd-journald");
}

TEST(cgroup_parser, others)
{
  CGroupParser c1("junk");
  EXPECT_FALSE(c1.get().valid);
  EXPECT_EQ(c1.get().qos, "");
  EXPECT_EQ(c1.get().pod_id, "");
  EXPECT_EQ(c1.get().runtime, "");
  EXPECT_EQ(c1.get().container_id, "");
  EXPECT_EQ(c1.get().service, "");

  CGroupParser c2("kubepods");
  EXPECT_FALSE(c2.get().valid);
  EXPECT_EQ(c2.get().qos, "");
  EXPECT_EQ(c2.get().pod_id, "");
  EXPECT_EQ(c2.get().runtime, "");
  EXPECT_EQ(c2.get().container_id, "");
  EXPECT_EQ(c2.get().service, "");

  CGroupParser c3("kubepods.slice");
  EXPECT_FALSE(c3.get().valid);
  EXPECT_EQ(c3.get().qos, "");
  EXPECT_EQ(c3.get().pod_id, "");
  EXPECT_EQ(c3.get().runtime, "");
  EXPECT_EQ(c3.get().container_id, "");
  EXPECT_EQ(c3.get().service, "");

  // all hex - starts down container_id parsing, but not 64 chars
  CGroupParser c4("1234abcd");
  EXPECT_FALSE(c4.get().valid);
  EXPECT_EQ(c4.get().qos, "");
  EXPECT_EQ(c4.get().pod_id, "");
  EXPECT_EQ(c4.get().runtime, "");
  EXPECT_EQ(c4.get().container_id, "");
  EXPECT_EQ(c4.get().service, "");
}