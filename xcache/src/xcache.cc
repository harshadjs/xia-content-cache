#include <iostream>
#include "controller.h"
#include "../clicknet/xid.h"

int main(void)
{
  XcacheController ctrl;
  XcacheMeta meta;

  std::cout << "Reached main" << std::endl;
  ctrl.addMeta("Index", &meta);
  ctrl.searchMeta("Index");
  ctrl.searchMeta("Index10");
  ctrl.run();
  return 0;
}

bool operator<(const struct click_xia_xid& x, const struct click_xia_xid& y)
{
  if(x.type < y.type) {
    return true;
  } else if(x.type > y.type) {
    return false;
  }

  for(int i = 0; i < CLICK_XIA_XID_ID_LEN; i++) {
    if(x.id[i] < y.id[i])
      return true;
    else if(x.id[i] > y.id[i])
      return false;
  }

  return false;
}

