
#include "src/utils/dispatcher/Dispatcher.h"

#ifndef IPPROCESS
#define IPPROCESS

class IPProcess : public Process<std::string, std::string> {
 public:
  explicit IPProcess(const std::string& name = "IPProcess");

  IPProcess(IPProcess&& same);

  bool canProceed(const std::string& data);

  std::string proceed(const std::string& data);

  ~IPProcess();
};

#endif
