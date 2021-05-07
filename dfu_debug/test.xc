// Copyright (c) 2019, XMOS Ltd, All rights reserved
#include <xs1.h>
#include <stdlib.h>
#include "xua.h"
#include "dfu_interface.h"
#include "dfu_handler.h"
#include "requests.h"

void test(client interface i_dfu i)
{
  initialise();
  download(i, "222.bin");
  //revertfactory(i);
  exit(0);
}

int main(void)
{
  interface i_dfu i;
  par {
    test(i);
    DFUHandler(i, null);
  }
  return 0;
}
