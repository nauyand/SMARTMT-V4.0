#ifndef EVENT_IO_H
#define EVENT_IO_H

#include <Arduino.h>

#define MAX_EVENTS 100
#define MAX_AVL 1

struct EventIOMeta
{
  int numberOfTotalID;
  int n1OfOneByteIO;
  int n2OfTwoBytesIO;
  int n4OfFourBytesIO;
  int n8OfEightBytesIO;
  int nXOfXBytesIO;
};

struct EventIO
{
  int ioID;
  int ioValue;
  int ioLength;
};

struct AvlData
{
  unsigned long long timestamp;
  String eventIOID;
  EventIOMeta eventIOMeta;
  EventIO eventIOs[MAX_EVENTS];
  int eventCount;
};

extern EventIOMeta eventIOMeta;
extern EventIO eventIOs[MAX_EVENTS];
int countIOsByLength(int length);
int countIOsByLengthX();
void insertEventIO(int ioID, int ioValue, int ioLength = 1);
void copyEventIOs(EventIO dest[], const EventIO src[], int count);
String toHexStringWithPadding(unsigned long number, int minWidth);
String toHexString(unsigned long long decimalValue);

#endif
