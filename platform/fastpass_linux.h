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

#ifndef INCLUDE_FASTPASS_FASTPASS_LINUX_H_
#define INCLUDE_FASTPASS_FASTPASS_LINUX_H_

#include <linux/ioctl.h>

#define FASTPASS_IOCTL_MAGIC 0xFA
/* fastpass engine configuration */
#define FASTPASS_IOW_TX_COST _IOW(FASTPASS_IOCTL_MAGIC, 0, u32)
#define FASTPASS_IOW_MAX_CREDIT _IOW(FASTPASS_IOCTL_MAGIC, 1, u32)
#define FASTPASS_IOW_NEW_DATA_TIMEOUT_NS _IOW(FASTPASS_IOCTL_MAGIC, 2, u32)
#define FASTPASS_IOW_RX_TIMEOUT_NS _IOW(FASTPASS_IOCTL_MAGIC, 3, u32)
#define FASTPASS_IOW_TX_TIMEOUT_NS _IOW(FASTPASS_IOCTL_MAGIC, 4, u32)
#define FASTPASS_IOW_RESET_WINDOW_NS _IOW(FASTPASS_IOCTL_MAGIC, 5, u64)
#define FASTPASS_IOW_IS_CLIENT _IOW(FASTPASS_IOCTL_MAGIC, 6, u8)
#define FASTPASS_IOW_MAX_PAYLOAD_LEN _IOW(FASTPASS_IOCTL_MAGIC, 7, u32)

#define TUNSERV_DEFAULT_IPPROTO 223
#define PINGER_DEFAULT_IPPROTO 224
#define FLOWTUNE_DEFAULT_IPPROTO 225
#define SWITCH_PACE_DEFAULT_IPPROTO 226

#endif /* INCLUDE_FASTPASS_FASTPASS_LINUX_H_ */
