#ifndef WIFI_H
#define WIFI_H

void setAccessPointMode();
bool setupAccessPoint();
bool manageWiFiConnection();
bool checkWifiConnection();
void queueWiFiConnectionAttempt();

#endif