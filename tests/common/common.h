#include <list>
#include <string>

class GOTestCommon {
public:
  GOTestCommon(const std::string &name);
  virtual ~GOTestCommon() = 0;
  virtual bool setUp();
  virtual void run();
  virtual bool tearDown();
  virtual std::string name();
};

class GOTestResultCollection {
  /* A base class for tests results */
public:
  void error(const std::string &err);
  void fail(const std::string &code, const char *file, size_t line);
  unsigned long failedCount() const;
  void reportError(std::ostream &out) const;
  void reportFailures(std::ostream &out) const;

private:
  class GOTestResult {
  public:
    explicit GOTestResult(
      const std::string &code, const char *file, size_t line);
    void report(std::ostream &out) const;

  private:
    std::string code_;
    std::string file_;
    unsigned long line_;
  };

  typedef std::list<GOTestResult> results_collection;
  typedef results_collection::iterator iterator;
  typedef results_collection::const_iterator const_iterator;
  results_collection results_;
  std::string error_;
};