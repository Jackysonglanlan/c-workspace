
#include "IPProcess.h"

#include "src/utils/utils.h"

using namespace std;

IPProcess::IPProcess(const string& name) : Process<string, string>(std::move(name)) {
  // out(this->getName() + " created");
}

IPProcess::IPProcess(IPProcess&& same) : Process<string, string>(std::move(same.getName())) {
  // out(this->getName() + " moved");
}

bool IPProcess::canProceed(const string& data) { return str_starts_with(data, "1234"); }

string IPProcess::proceed(const string& data) {
  //
  return "1.2.3.4";
}

IPProcess::~IPProcess() {
  // out("IPProcess released");
}
