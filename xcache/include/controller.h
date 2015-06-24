#ifndef __XCACHE_CONTROLLER_H__
#define __XCACHE_CONTROLLER_H__
#include <map>
#include <iostream>
#include "meta.h"

class XcacheController {
private:
  std::map<std::string, XcacheMeta> metaMap;
  std::map<uint32_t, XcacheMeta> sliceMap;

public:
  XcacheController() {
    std::cout << "Reached Constructor\n";
  }

  void addMeta(std::string str, XcacheMeta *meta) {
    std::cout << "Reached AddMeta\n";
    metaMap[str] = *meta;
  }

  void searchMeta(std::string str) {
    std::map<std::string, XcacheMeta>::iterator i = metaMap.find(str);

    std::cout << "Reached SearchMeta\n";
    if(i != metaMap.end()) {
      i->second.print();
    } else {
      std::cout << "Not Found\n";
    }
  }

  void handleCli(void);
  void run(void);

  void search(void);
  void store(void);
  void remove(void);
};
#endif
