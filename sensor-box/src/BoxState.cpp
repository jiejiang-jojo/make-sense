#include "BoxState.h"

void BoxState::WifiOn(){
  wifi_ = true;
  if (!privacy_)
    led_.SetGreen();
}

void BoxState::WifiOff(){
  wifi_ = false;
  if (!privacy_)
    led_.SetRed();
}

void BoxState::PrivacyOn(){
  privacy_ = true;
  led_.Off();
}

void BoxState::PrivacyOff(){
  privacy_ = false;
  if (!wifi_)
    led_.SetBlue();
  else
    led_.SetGreen();
}

void BoxState::CongestionInPlace(){

}

void BoxState::CongestionResolved(){

}

bool BoxState::IsWifiOn(){
  return wifi_;
}

bool BoxState::IsPrivacyOn(){
  return privacy_;
}

bool BoxState::IsCongested(){
  return congested_;
}
