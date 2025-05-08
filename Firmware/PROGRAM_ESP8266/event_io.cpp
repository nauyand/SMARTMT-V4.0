#include "event_io.h"

String prefix = "00000000";
String postfix = "00008612";
String codecId = "7e";
//unsigned long currentTimestamp;

//unsigned long long timestampB = 1700169727733ULL;

int numberOfRecords = 1;
String eventIOID = "";
String datetime;
EventIOMeta eventIOMeta;
EventIO eventIOs[MAX_EVENTS];

int countIOsByLength(int length)
{
  int count = 0;
  for (int i = 0; i < eventIOMeta.numberOfTotalID; i++)
  {
    if (eventIOs[i].ioLength == length)
    {
      count++;
    }
  }
  return count;
}

int countIOsByLengthX()
{
  int count = 0;
  for (int i = 0; i < eventIOMeta.numberOfTotalID; i++)
  {
    if (eventIOs[i].ioLength != 1 && eventIOs[i].ioLength != 2 && eventIOs[i].ioLength != 4 && eventIOs[i].ioLength != 8)
    {
      count++;
    }
  }
  return count;
}

void insertEventIO(int ioID, int ioValue, int ioLength)
{
  bool found = false;
  for (int i = 0; i < eventIOMeta.numberOfTotalID; i++)
  {
    if (eventIOs[i].ioID == ioID)
    {
      eventIOs[i].ioValue = ioValue;
      eventIOs[i].ioLength = ioLength;
      found = true;
      break;
    }
  }
  if (!found && eventIOMeta.numberOfTotalID < MAX_EVENTS)
  {
    eventIOs[eventIOMeta.numberOfTotalID++] = {ioID, ioValue, ioLength};
  }
  eventIOMeta.n1OfOneByteIO = countIOsByLength(1);
  eventIOMeta.n2OfTwoBytesIO = countIOsByLength(2);
  eventIOMeta.n4OfFourBytesIO = countIOsByLength(4);
  eventIOMeta.n8OfEightBytesIO = countIOsByLength(8);
  eventIOMeta.nXOfXBytesIO = countIOsByLengthX();
}

void copyEventIOs(EventIO dest[], const EventIO src[], int count)
{
  for (int i = 0; i < count; i++)
  {
    dest[i] = src[i];
  }
}

String toHexStringWithPaddingB(unsigned long long number, int minWidth) {
  String hexString = String(number, HEX);
  hexString.toUpperCase();  // Mengubah ke huruf besar sesuai dengan format umum

  int leadingZeros = minWidth - hexString.length();
  String result = "";

  for (int i = 0; i < leadingZeros; i++) {
    result += '0';
  }

  result += hexString;
  return result;
}

String toHexStringWithPadding(unsigned long number, int minWidth)
{
  String hexString = String(number, HEX);
  hexString.toLowerCase();

  int leadingZeros = minWidth - hexString.length();
  String result = "";

  for (int i = 0; i < leadingZeros; i++)
  {
    result += '0';
  }

  result += hexString;
  return result;
}

String toHexString(unsigned long long decimalValue)
{
  String hexData = "";
  String avlDataHex = "";

  AvlData avlData[MAX_AVL];


  avlData[0].timestamp = decimalValue;
  avlData[0].eventIOID = eventIOID;
  avlData[0].eventIOMeta = eventIOMeta;
  avlData[0].eventCount = eventIOMeta.numberOfTotalID;
  copyEventIOs(avlData[0].eventIOs, eventIOs, eventIOMeta.numberOfTotalID);

  for (int i = 0; i < MAX_AVL; i++)
  {
    avlDataHex += toHexStringWithPaddingB(avlData[i].timestamp,16);
    avlDataHex += toHexStringWithPadding(avlData[i].eventIOID.toInt(), 4);
    avlDataHex += toHexStringWithPadding(avlData[i].eventIOMeta.numberOfTotalID, 4);

    avlDataHex += toHexStringWithPadding(avlData[i].eventIOMeta.n1OfOneByteIO, 4);
    for (int j = 0; j < avlData[i].eventCount; j++)
    {
      if (avlData[i].eventIOs[j].ioLength == 1)
      {
        avlDataHex += toHexStringWithPadding(avlData[i].eventIOs[j].ioID, 4);
        avlDataHex += toHexStringWithPadding(avlData[i].eventIOs[j].ioValue, 2);
      }
    }
    avlDataHex += toHexStringWithPadding(avlData[i].eventIOMeta.n2OfTwoBytesIO, 4);
    for (int j = 0; j < avlData[i].eventCount; j++)
    {
      if (avlData[i].eventIOs[j].ioLength == 2)
      {
        avlDataHex += toHexStringWithPadding(avlData[i].eventIOs[j].ioID, 4);
        avlDataHex += toHexStringWithPadding(avlData[i].eventIOs[j].ioValue, 4);
      }
    }
    avlDataHex += toHexStringWithPadding(avlData[i].eventIOMeta.n4OfFourBytesIO, 4);
    for (int j = 0; j < avlData[i].eventCount; j++)
    {
      if (avlData[i].eventIOs[j].ioLength == 4)
      {
        avlDataHex += toHexStringWithPadding(avlData[i].eventIOs[j].ioID, 4);
        avlDataHex += toHexStringWithPadding(avlData[i].eventIOs[j].ioValue, 8);
      }
    }
    avlDataHex += toHexStringWithPadding(avlData[i].eventIOMeta.n8OfEightBytesIO, 4);
    for (int j = 0; j < avlData[i].eventCount; j++)
    {
      if (avlData[i].eventIOs[j].ioLength == 8)
      {
        avlDataHex += toHexStringWithPadding(avlData[i].eventIOs[j].ioID, 4);
        avlDataHex += toHexStringWithPadding(avlData[i].eventIOs[j].ioValue, 16);
      }
    }
    if (codecId == "7e")
    {
      avlDataHex += toHexStringWithPadding(avlData[i].eventIOMeta.nXOfXBytesIO, 4);
      for (int j = 0; j < avlData[i].eventCount; j++)
      {
        if (!(avlData[i].eventIOs[j].ioLength == 1 || avlData[i].eventIOs[j].ioLength == 2 || avlData[i].eventIOs[j].ioLength == 4 || avlData[i].eventIOs[j].ioLength == 8))
        {
          avlDataHex += toHexStringWithPadding(avlData[i].eventIOs[j].ioID, 4);
          avlDataHex += toHexStringWithPadding(avlData[i].eventIOs[j].ioLength, 4);
          avlDataHex += toHexStringWithPadding(avlData[i].eventIOs[j].ioValue, avlData[i].eventIOs[j].ioLength * 2);
        }
      }
    }
  }

  hexData += prefix;
  hexData += toHexStringWithPadding(avlDataHex.length() / 2, 8);
  hexData += codecId;
  hexData += toHexStringWithPadding(numberOfRecords, 2);
  hexData += avlDataHex;
  hexData += toHexStringWithPadding(numberOfRecords, 2);
  hexData += postfix;

  return hexData;
}
