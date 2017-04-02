// Miscellaneous commands

#ifndef _EL_CLIENT_CMD_H_
#define _EL_CLIENT_CMD_H_

#include "ELClient.h"
#include "FP.h"

class ELClientCmd {
  public:
    // Constructor
    ELClientCmd(ELClient* elc);
    // Get the current time in seconds since the epoch, 0 if the time is unknown
    uint32_t GetTime();

  private:
    ELClient* _elc;
};
#endif