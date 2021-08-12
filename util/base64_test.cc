//
// Copyright 2021 Splunk Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#include <gtest/gtest.h>

#include <util/base64.h>

TEST(base64, test_battery)
{
  EXPECT_EQ("", base64_encode(""));

  EXPECT_EQ("MA==", base64_encode("0"));
  EXPECT_EQ("MDE=", base64_encode("01"));
  EXPECT_EQ("MDEy", base64_encode("012"));
  EXPECT_EQ("MDEyMw==", base64_encode("0123"));
  EXPECT_EQ("MDEyMzQ=", base64_encode("01234"));
  EXPECT_EQ("MDEyMzQ1", base64_encode("012345"));
  EXPECT_EQ("MDEyMzQ1Ng==", base64_encode("0123456"));
  EXPECT_EQ("MDEyMzQ1Njc=", base64_encode("01234567"));
  EXPECT_EQ("MDEyMzQ1Njc4", base64_encode("012345678"));
  EXPECT_EQ("MDEyMzQ1Njc4OQ==", base64_encode("0123456789"));

  EXPECT_EQ("OQ==", base64_encode("9"));
  EXPECT_EQ("OTg=", base64_encode("98"));
  EXPECT_EQ("OTg3", base64_encode("987"));
  EXPECT_EQ("OTg3Ng==", base64_encode("9876"));
  EXPECT_EQ("OTg3NjU=", base64_encode("98765"));
  EXPECT_EQ("OTg3NjU0", base64_encode("987654"));
  EXPECT_EQ("OTg3NjU0Mw==", base64_encode("9876543"));
  EXPECT_EQ("OTg3NjU0MzI=", base64_encode("98765432"));
  EXPECT_EQ("OTg3NjU0MzIx", base64_encode("987654321"));
  EXPECT_EQ("OTg3NjU0MzIxMA==", base64_encode("9876543210"));

  EXPECT_EQ("QQ==", base64_encode("A"));
  EXPECT_EQ("QUI=", base64_encode("AB"));
  EXPECT_EQ("QUJD", base64_encode("ABC"));
  EXPECT_EQ("QUJDRA==", base64_encode("ABCD"));
  EXPECT_EQ("QUJDREU=", base64_encode("ABCDE"));
  EXPECT_EQ("QUJDREVG", base64_encode("ABCDEF"));
  EXPECT_EQ("QUJDREVGRw==", base64_encode("ABCDEFG"));
  EXPECT_EQ("QUJDREVGR0g=", base64_encode("ABCDEFGH"));
  EXPECT_EQ("QUJDREVGR0hJ", base64_encode("ABCDEFGHI"));
  EXPECT_EQ("QUJDREVGR0hJSg==", base64_encode("ABCDEFGHIJ"));
  EXPECT_EQ("QUJDREVGR0hJSks=", base64_encode("ABCDEFGHIJK"));
  EXPECT_EQ("QUJDREVGR0hJSktM", base64_encode("ABCDEFGHIJKL"));
  EXPECT_EQ("QUJDREVGR0hJSktMTQ==", base64_encode("ABCDEFGHIJKLM"));
  EXPECT_EQ("QUJDREVGR0hJSktMTU4=", base64_encode("ABCDEFGHIJKLMN"));
  EXPECT_EQ("QUJDREVGR0hJSktMTU5P", base64_encode("ABCDEFGHIJKLMNO"));
  EXPECT_EQ("QUJDREVGR0hJSktMTU5PUA==", base64_encode("ABCDEFGHIJKLMNOP"));
  EXPECT_EQ("QUJDREVGR0hJSktMTU5PUFE=", base64_encode("ABCDEFGHIJKLMNOPQ"));
  EXPECT_EQ("QUJDREVGR0hJSktMTU5PUFFS", base64_encode("ABCDEFGHIJKLMNOPQR"));
  EXPECT_EQ("QUJDREVGR0hJSktMTU5PUFFSUw==", base64_encode("ABCDEFGHIJKLMNOPQRS"));
  EXPECT_EQ("QUJDREVGR0hJSktMTU5PUFFSU1Q=", base64_encode("ABCDEFGHIJKLMNOPQRST"));
  EXPECT_EQ("QUJDREVGR0hJSktMTU5PUFFSU1RV", base64_encode("ABCDEFGHIJKLMNOPQRSTU"));
  EXPECT_EQ("QUJDREVGR0hJSktMTU5PUFFSU1RVVg==", base64_encode("ABCDEFGHIJKLMNOPQRSTUV"));
  EXPECT_EQ("QUJDREVGR0hJSktMTU5PUFFSU1RVVlc=", base64_encode("ABCDEFGHIJKLMNOPQRSTUVW"));
  EXPECT_EQ("QUJDREVGR0hJSktMTU5PUFFSU1RVVldY", base64_encode("ABCDEFGHIJKLMNOPQRSTUVWX"));
  EXPECT_EQ("QUJDREVGR0hJSktMTU5PUFFSU1RVVldYWQ==", base64_encode("ABCDEFGHIJKLMNOPQRSTUVWXY"));
  EXPECT_EQ("QUJDREVGR0hJSktMTU5PUFFSU1RVVldYWVo=", base64_encode("ABCDEFGHIJKLMNOPQRSTUVWXYZ"));

  EXPECT_EQ("Wg==", base64_encode("Z"));
  EXPECT_EQ("Wlk=", base64_encode("ZY"));
  EXPECT_EQ("WllY", base64_encode("ZYX"));
  EXPECT_EQ("WllYVw==", base64_encode("ZYXW"));
  EXPECT_EQ("WllYV1Y=", base64_encode("ZYXWV"));
  EXPECT_EQ("WllYV1ZV", base64_encode("ZYXWVU"));
  EXPECT_EQ("WllYV1ZVVA==", base64_encode("ZYXWVUT"));
  EXPECT_EQ("WllYV1ZVVFM=", base64_encode("ZYXWVUTS"));
  EXPECT_EQ("WllYV1ZVVFNS", base64_encode("ZYXWVUTSR"));
  EXPECT_EQ("WllYV1ZVVFNSUQ==", base64_encode("ZYXWVUTSRQ"));
  EXPECT_EQ("WllYV1ZVVFNSUVA=", base64_encode("ZYXWVUTSRQP"));
  EXPECT_EQ("WllYV1ZVVFNSUVBP", base64_encode("ZYXWVUTSRQPO"));
  EXPECT_EQ("WllYV1ZVVFNSUVBPTg==", base64_encode("ZYXWVUTSRQPON"));
  EXPECT_EQ("WllYV1ZVVFNSUVBPTk0=", base64_encode("ZYXWVUTSRQPONM"));
  EXPECT_EQ("WllYV1ZVVFNSUVBPTk1M", base64_encode("ZYXWVUTSRQPONML"));
  EXPECT_EQ("WllYV1ZVVFNSUVBPTk1MSw==", base64_encode("ZYXWVUTSRQPONMLK"));
  EXPECT_EQ("WllYV1ZVVFNSUVBPTk1MS0o=", base64_encode("ZYXWVUTSRQPONMLKJ"));
  EXPECT_EQ("WllYV1ZVVFNSUVBPTk1MS0pJ", base64_encode("ZYXWVUTSRQPONMLKJI"));
  EXPECT_EQ("WllYV1ZVVFNSUVBPTk1MS0pJSA==", base64_encode("ZYXWVUTSRQPONMLKJIH"));
  EXPECT_EQ("WllYV1ZVVFNSUVBPTk1MS0pJSEc=", base64_encode("ZYXWVUTSRQPONMLKJIHG"));
  EXPECT_EQ("WllYV1ZVVFNSUVBPTk1MS0pJSEdG", base64_encode("ZYXWVUTSRQPONMLKJIHGF"));
  EXPECT_EQ("WllYV1ZVVFNSUVBPTk1MS0pJSEdGRQ==", base64_encode("ZYXWVUTSRQPONMLKJIHGFE"));
  EXPECT_EQ("WllYV1ZVVFNSUVBPTk1MS0pJSEdGRUQ=", base64_encode("ZYXWVUTSRQPONMLKJIHGFED"));
  EXPECT_EQ("WllYV1ZVVFNSUVBPTk1MS0pJSEdGRURD", base64_encode("ZYXWVUTSRQPONMLKJIHGFEDC"));
  EXPECT_EQ("WllYV1ZVVFNSUVBPTk1MS0pJSEdGRURDQg==", base64_encode("ZYXWVUTSRQPONMLKJIHGFEDCB"));
  EXPECT_EQ("WllYV1ZVVFNSUVBPTk1MS0pJSEdGRURDQkE=", base64_encode("ZYXWVUTSRQPONMLKJIHGFEDCBA"));

  EXPECT_EQ("YQ==", base64_encode("a"));
  EXPECT_EQ("YWI=", base64_encode("ab"));
  EXPECT_EQ("YWJj", base64_encode("abc"));
  EXPECT_EQ("YWJjZA==", base64_encode("abcd"));
  EXPECT_EQ("YWJjZGU=", base64_encode("abcde"));
  EXPECT_EQ("YWJjZGVm", base64_encode("abcdef"));
  EXPECT_EQ("YWJjZGVmZw==", base64_encode("abcdefg"));
  EXPECT_EQ("YWJjZGVmZ2g=", base64_encode("abcdefgh"));
  EXPECT_EQ("YWJjZGVmZ2hp", base64_encode("abcdefghi"));
  EXPECT_EQ("YWJjZGVmZ2hpag==", base64_encode("abcdefghij"));
  EXPECT_EQ("YWJjZGVmZ2hpams=", base64_encode("abcdefghijk"));
  EXPECT_EQ("YWJjZGVmZ2hpamts", base64_encode("abcdefghijkl"));
  EXPECT_EQ("YWJjZGVmZ2hpamtsbQ==", base64_encode("abcdefghijklm"));
  EXPECT_EQ("YWJjZGVmZ2hpamtsbW4=", base64_encode("abcdefghijklmn"));
  EXPECT_EQ("YWJjZGVmZ2hpamtsbW5v", base64_encode("abcdefghijklmno"));
  EXPECT_EQ("YWJjZGVmZ2hpamtsbW5vcA==", base64_encode("abcdefghijklmnop"));
  EXPECT_EQ("YWJjZGVmZ2hpamtsbW5vcHE=", base64_encode("abcdefghijklmnopq"));
  EXPECT_EQ("YWJjZGVmZ2hpamtsbW5vcHFy", base64_encode("abcdefghijklmnopqr"));
  EXPECT_EQ("YWJjZGVmZ2hpamtsbW5vcHFycw==", base64_encode("abcdefghijklmnopqrs"));
  EXPECT_EQ("YWJjZGVmZ2hpamtsbW5vcHFyc3Q=", base64_encode("abcdefghijklmnopqrst"));
  EXPECT_EQ("YWJjZGVmZ2hpamtsbW5vcHFyc3R1", base64_encode("abcdefghijklmnopqrstu"));
  EXPECT_EQ("YWJjZGVmZ2hpamtsbW5vcHFyc3R1dg==", base64_encode("abcdefghijklmnopqrstuv"));
  EXPECT_EQ("YWJjZGVmZ2hpamtsbW5vcHFyc3R1dnc=", base64_encode("abcdefghijklmnopqrstuvw"));
  EXPECT_EQ("YWJjZGVmZ2hpamtsbW5vcHFyc3R1dnd4", base64_encode("abcdefghijklmnopqrstuvwx"));
  EXPECT_EQ("YWJjZGVmZ2hpamtsbW5vcHFyc3R1dnd4eQ==", base64_encode("abcdefghijklmnopqrstuvwxy"));
  EXPECT_EQ("YWJjZGVmZ2hpamtsbW5vcHFyc3R1dnd4eXo=", base64_encode("abcdefghijklmnopqrstuvwxyz"));

  EXPECT_EQ("eg==", base64_encode("z"));
  EXPECT_EQ("enk=", base64_encode("zy"));
  EXPECT_EQ("enl4", base64_encode("zyx"));
  EXPECT_EQ("enl4dw==", base64_encode("zyxw"));
  EXPECT_EQ("enl4d3Y=", base64_encode("zyxwv"));
  EXPECT_EQ("enl4d3Z1", base64_encode("zyxwvu"));
  EXPECT_EQ("enl4d3Z1dA==", base64_encode("zyxwvut"));
  EXPECT_EQ("enl4d3Z1dHM=", base64_encode("zyxwvuts"));
  EXPECT_EQ("enl4d3Z1dHNy", base64_encode("zyxwvutsr"));
  EXPECT_EQ("enl4d3Z1dHNycQ==", base64_encode("zyxwvutsrq"));
  EXPECT_EQ("enl4d3Z1dHNycXA=", base64_encode("zyxwvutsrqp"));
  EXPECT_EQ("enl4d3Z1dHNycXBv", base64_encode("zyxwvutsrqpo"));
  EXPECT_EQ("enl4d3Z1dHNycXBvbg==", base64_encode("zyxwvutsrqpon"));
  EXPECT_EQ("enl4d3Z1dHNycXBvbm0=", base64_encode("zyxwvutsrqponm"));
  EXPECT_EQ("enl4d3Z1dHNycXBvbm1s", base64_encode("zyxwvutsrqponml"));
  EXPECT_EQ("enl4d3Z1dHNycXBvbm1saw==", base64_encode("zyxwvutsrqponmlk"));
  EXPECT_EQ("enl4d3Z1dHNycXBvbm1sa2o=", base64_encode("zyxwvutsrqponmlkj"));
  EXPECT_EQ("enl4d3Z1dHNycXBvbm1sa2pp", base64_encode("zyxwvutsrqponmlkji"));
  EXPECT_EQ("enl4d3Z1dHNycXBvbm1sa2ppaA==", base64_encode("zyxwvutsrqponmlkjih"));
  EXPECT_EQ("enl4d3Z1dHNycXBvbm1sa2ppaGc=", base64_encode("zyxwvutsrqponmlkjihg"));
  EXPECT_EQ("enl4d3Z1dHNycXBvbm1sa2ppaGdm", base64_encode("zyxwvutsrqponmlkjihgf"));
  EXPECT_EQ("enl4d3Z1dHNycXBvbm1sa2ppaGdmZQ==", base64_encode("zyxwvutsrqponmlkjihgfe"));
  EXPECT_EQ("enl4d3Z1dHNycXBvbm1sa2ppaGdmZWQ=", base64_encode("zyxwvutsrqponmlkjihgfed"));
  EXPECT_EQ("enl4d3Z1dHNycXBvbm1sa2ppaGdmZWRj", base64_encode("zyxwvutsrqponmlkjihgfedc"));
  EXPECT_EQ("enl4d3Z1dHNycXBvbm1sa2ppaGdmZWRjYg==", base64_encode("zyxwvutsrqponmlkjihgfedcb"));
  EXPECT_EQ("enl4d3Z1dHNycXBvbm1sa2ppaGdmZWRjYmE=", base64_encode("zyxwvutsrqponmlkjihgfedcba"));
}
