#ifndef __BOXSTATE_H__
#define __BOXSTATE_H__

#include "led.h"

class BoxState {

  bool wifi_ = false;
  bool congested_ = false;
  bool privacy_ = false;
  LED &led_;

public:
  BoxState(LED &led): led_(led){ led_.SetBlue(); }
  void WifiOn();
  void WifiOff();
  void PrivacyOn();
  void PrivacyOff();
  void CongestionInPlace();
  void CongestionResolved();

  bool IsWifiOn();
  bool IsPrivacyOn();
  bool IsCongested();
};

#endif
