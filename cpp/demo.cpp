
#include "src/utils/utils.h"

using namespace std;

#include "src/utils/combinator/Combinator.h"
#include "src/utils/dispatcher/Dispatcher.h"

#include "libs/pprint/pprint.hpp"

pprint::PrettyPrinter printer;

void _urlGenerator() {
  using ProcessParam  = pair<string, string>;  // <ready to process url param name, raw data>
  using ProcessReturn = pair<string, string>;  // <url param name, url param value>

  auto urlGenerator = Dispatcher<ProcessReturn, ProcessParam>([](const ProcessParam &param) -> ProcessReturn {
    return make_pair("", "");  // 无法处理，返回空 pair
  });

  // CriteriaLamda curring
  auto forParam = [](const string &targetName) -> std::function<bool(const ProcessParam &param)> {
    return [&targetName](const ProcessParam &param) -> bool {
      const auto &urlParamName = param.first;
      return targetName.compare(urlParamName) == 0;
    };
  };

  urlGenerator.addProcess(
      "username",
      [](const ProcessParam &param) -> bool {
        const auto &urlParamName = param.first;
        return urlParamName.compare("username") == 0;
      },
      [](const ProcessParam &param) -> ProcessReturn {
        const auto &data = param.second;
        return make_pair("username", data.substr(3));
      });

  urlGenerator.addProcess(
      "pwd",
      [](const ProcessParam &param) -> bool {
        const auto &urlParamName = param.first;
        return urlParamName.compare("pwd") == 0;
      },
      [](const ProcessParam &param) -> ProcessReturn {
        const auto &data = param.second;
        return make_pair("pwd", string_to_upper(data));
      });

  urlGenerator.addProcess(
      "ip",
      [](const ProcessParam &param) -> bool {
        const auto &urlParamName = param.first;
        return urlParamName.compare("ip") == 0;
      },
      [](const ProcessParam &param) -> ProcessReturn { return make_pair("ip", "10.0.0.27"); });

  // use...

  vector<string> paramWeNeed{"username", "pwd", "ip", "jsdfjsdlkfjldsjf"};

  string dataToProcess = "this is test data";

  auto urlParamStr = Combinator<vector<string>>(paramWeNeed)
                         // 处理参数
                         .map([&urlGenerator, &dataToProcess](string urlParamName) -> string {
                           const auto &processed = urlGenerator.proceed(make_pair(urlParamName, dataToProcess));
                           return processed.first + "=" + processed.second;
                         })
                         // 过滤处理不了的数据(空数据)
                         .filter([](const string &data) -> bool { return data != "="; })
                         // 拼接请求参数
                         .reduce(string(""), [](string &accu, string data) -> string { return accu + data + "&"; });

  printer.print("--------");
  printer.print(urlParamStr);
  printer.print("--------");
}

/////// run

#include "runner.cpp"

int main(int argc, char const *argv[]) {
  // _urlGenerator();
  run();
  return 0;
}
