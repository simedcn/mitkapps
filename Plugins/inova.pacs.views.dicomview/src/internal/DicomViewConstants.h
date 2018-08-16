#ifndef RETRIEVEPROTOCOL_H
#define RETRIEVEPROTOCOL_H

#include <QString>

enum RetrieveProtocol
{
    PROTOCOL_CGET = 0,
    PROTOCOL_CMOVE = 1
};


enum RetrieveDestination
{
    DESTINATION_LOCALFOLDER = 0,
    DESTINATION_PACS = 1
};

static const QString NO_DIRECTORY_SPECIFIED = "<TEMP>";

#endif // RETRIEVEPROTOCOL_H
