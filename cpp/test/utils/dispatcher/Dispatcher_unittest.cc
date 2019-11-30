
// 为测试方便都include，这样不用改编译过程
#include "IPProcess.cpp"
#include "IPProcess.h"

#include "gtest/gtest.h"
#include "src/utils/utils.h"

namespace {

TEST(DispatcherTest, AddLamdaThenProceed) {
  using namespace std;

  auto process = Dispatcher<string, string>([](string data) -> string { return "unknown"; });

  process.addProcess("aaaa", [](string data) -> bool { return str_starts_with(data, "aaaa"); },
                     [](string data) -> string { return string_to_upper(data); });

  EXPECT_EQ(process.proceed("aaaaa-------bbbbb"), "AAAAA-------BBBBB");
  EXPECT_EQ(process.proceed(".....-------....."), "unknown");
}

TEST(DispatcherTest, AddClassThenProceed) {
  using namespace std;

  auto process = Dispatcher<string, string>([](string data) -> string { return "unknown"; });

  // Dispatcher 存储 Process 使用的 smart pointer 会自动释放 IPProcess
  process.addProcess(new IPProcess());

  EXPECT_EQ(process.proceed("12345-------67890"), "1.2.3.4");
  EXPECT_EQ(process.proceed(".....-------....."), "unknown");
}

TEST(DispatcherTest, EnableDisableThenProceed) {
  using namespace std;

  auto process = Dispatcher<string, string>([](string data) -> string { return "unknown"; });

  process.addProcess("aaaa", [](string data) -> bool { return str_starts_with(data, "aaaa"); },
                     [](string data) -> string { return string_to_upper(data); });

  process.disableProcess("aaaa");
  process.enableProcess("aaaa");

  EXPECT_EQ(process.proceed("aaaaa-------bbbbb"), "AAAAA-------BBBBB");
  EXPECT_EQ(process.proceed(".....-------....."), "unknown");
}

TEST(DispatcherTest, MixAll) {
  using namespace std;

  auto process = Dispatcher<string, string>([](string data) -> string { return "unknown"; });

  // Dispatcher 存储 Process 使用的 smart pointer 会自动释放 IPProcess
  process.addProcess(new IPProcess());

  process.addProcess("aaaa", [](string data) -> bool { return str_starts_with(data, "aaaa"); },
                     [](string data) -> string { return string_to_upper(data); });

  EXPECT_EQ(process.proceed("12345-------67890"), "1.2.3.4");
  EXPECT_EQ(process.proceed("aaaaa-------bbbbb"), "AAAAA-------BBBBB");
  EXPECT_EQ(process.proceed(".....-------....."), "unknown");
}

}  // namespace
