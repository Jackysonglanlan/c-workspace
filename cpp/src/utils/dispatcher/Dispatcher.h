
// 如何定义 模板类: https://www.jianshu.com/p/70ca94872418

// Best Practice: 模板类的声明和实现都应该在同一个 .h 文件中
// See:https://stackoverflow.com/questions/495021/why-can-templates-only-be-implemented-in-the-header-file

#ifndef DISPATCHER
#define DISPATCHER

#include <algorithm>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>

//////// Lamda type

template <class Data>
// 用 using 而不是 typedef，see:
// https://stackoverflow.com/questions/10747810/what-is-the-difference-between-typedef-and-using-in-c11
using CriteriaLamda = std::function<bool(const Data&)>;

template <class ReturnType, class Data>
using ProceedLamda = std::function<ReturnType(const Data&)>;

//////// 前置声明

template <class ReturnType, class Data>
class Dispatcher;

//////// Process，代表一个完整的业务处理过程：判断能否处理 -> 具体的处理过程 -> 返回结果

template <class ReturnType, class Data>
class Process {
  // Dispatcher 需要访问 private
  friend class Dispatcher<ReturnType, Data>;

 public:
  // 构造子注入，immutable
  explicit Process(const std::string& name) : name(name) {}

  // copy return, 防止调用方改 name
  std::string getName() { return this->name; }

  // 纯虚函数，等价于 Java 的 abstract method，子类必须实现，而正因为有了这个纯虚函数，Process无法被创建
  virtual bool canProceed(const Data& data) = 0;

  virtual ReturnType proceed(const Data& data) = 0;

  // 虚函数，父类里面可以有默认实现，但子类可以重载
  virtual ~Process(){};

 private:
  std::string name;

  bool _isEnable = true;  // 默认 enable
  void enable() { this->_isEnable = true; }
  void disable() { this->_isEnable = false; }
  bool isEnable() { return this->_isEnable; }
};

//////// Dispatcher

template <class ReturnType, class Data>
class Dispatcher {
 private:
  //////// inner class LamdaProcess
  // 负责把 lamda 转换为 Process
  template <class R, class D>
  class LamdaProcess : public Process<R, D> {
   public:
    // 构造子注入，immutable
    explicit LamdaProcess(const std::string& name, const CriteriaLamda<D>& criteria, const ProceedLamda<R, D>& process)
        : Process<R, D>(name)  // 这里，LamdaProcess 继承了模板类，所以调用父类构造函数时需要带类型
    {
      this->criteriaLamda = criteria;  // copy
      this->processLamda  = process;   // copy
    }

    bool canProceed(const D& data) { return this->criteriaLamda(data); }

    R proceed(const D& data) { return this->processLamda(data); }

    ~LamdaProcess() {
      // out("LamdaProcess released");
    }

   private:
    CriteriaLamda<D> criteriaLamda;
    ProceedLamda<R, D> processLamda;
  };

  // map, see https://stackoverflow.com/questions/21149631/c-inserting-object-with-constructor-into-map

  // 如果 map 要装 abstract class，必须装指针，为了自动在 vector 销毁的时候自动销毁里面的 Process，所以装
  // unique_ptr。
  // 为什么必须装指针？See:
  // https://stackoverflow.com/questions/12203878/how-to-store-a-vector-of-objects-of-an-abstract-class-which-are-given-by-stdun

  // name -> Process，不需要有序
  std::unordered_map<std::string, std::unique_ptr<Process<ReturnType, Data>>>* processes;
  ProceedLamda<ReturnType, Data> noneProcess;

 public:
  Dispatcher(const ProceedLamda<ReturnType, Data>& noneProcess) {
    processes         = new std::unordered_map<std::string, std::unique_ptr<Process<ReturnType, Data>>>();
    this->noneProcess = noneProcess;
  }

  void addProcess(Process<ReturnType, Data>* const process) {
    //
    (*processes)[process->getName()] = (std::unique_ptr<Process<ReturnType, Data>>(process));
  }

  // 使用 lamda 快捷添加 Process
  void addProcess(const std::string& name, const CriteriaLamda<Data>& criteria,
                  const ProceedLamda<ReturnType, Data>& process) {
    // see https://stackoverflow.com/questions/14218042/most-efficient-way-to-assign-values-to-maps

    // emplace 会拿着你给它的参数，直接作为 key 和 value 放入 map，不需要创建 std::pair，这样就省了一次操作，所以最快
    (*processes).emplace(name, std::make_unique<LamdaProcess<ReturnType, Data>>(name, criteria, process));

    // 下面的 [] 操作符，就不如上面的快：
    //
    // std::map::operator[] first default-creates the object if it doesn't already exist, then returns a reference that
    // you can use operator= on to set your desired value, i.e. two operations.
    //
    // 即先调用 std::unique_ptr 的默认构造函数，创建一个对象作为 value，再调用 value 的 operator=() 方法给它赋值

    // (*processes)[name] = (std::make_unique<LamdaProcess<ReturnType, Data>>(name, criteria, process));

    // Brief:
    // map 不同插入方法的区别：
    //   [] will replace the old value (if any)
    //   insert/emplace will ignore the new value (if an old value is already present)

    // out((*processes).size());
  }

  void removeProcess(const std::string& name) { (*processes).erase(name); }

  // 运行时控制处理过程
  void enableProcess(const std::string& name) {
    // Dispatcher 是 Process 的 friend class，所以可以访问私有方法
    (*processes)[name]->enable();
  }

  void disableProcess(const std::string& name) { (*processes)[name]->disable(); }

  ReturnType proceed(const Data& data) {
    for (auto& kv : *processes) {
      auto& process = kv.second;
      // processes 里面存的 smart pointer，不是引用，访问方法要用 ->
      if (process->isEnable() && process->canProceed(data)) {
        return process->proceed(data);
      }
    }
    return noneProcess(data);
  }

  ~Dispatcher() {
    delete processes; // 里面是 smart pointer，所以元素会自动回收
    // out("dispatcher released");
  }
};

#endif
