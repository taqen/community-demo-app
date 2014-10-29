/****************************************************************************/
/// @file    server.cpp
/// @author  Julen Maneros
/// @date    10.10.2010
///
// iTETRIS Cooperative ITS demo server logic
/****************************************************************************/
// iTETRIS; see http://www.ict-itetris.eu/
// Copyright 2008-2010 iTETRIS consortium
/****************************************************************************/

// ===========================================================================
// included modules
// ===========================================================================
#include "server.h"
#include "app-commands-subscriptions-constants.h"
#include "application-logic.h"
#include "utils/log/log.h"
#include <utils/common/MsgHandler.h>
#include <utils/common/ToString.h>
#include <iostream>
#include <string>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifdef _MSC_VER
#include <windows_config.h>
#else
#include <config.h>
#endif

// ===========================================================================
// used namespaces
// ===========================================================================
using namespace std;
using namespace tcpip;

// ===========================================================================
// static member variables
// ===========================================================================
Server* Server::instance_ = 0;
bool Server::closeConnection_;

// ===========================================================================
// method definitions
// ===========================================================================
Server::Server(int port)
{
    try {
        closeConnection_ = false;
        myDoingSimStep = false;
        MsgHandler::getMessageInstance()->inform("Server listening on port: " + toString(port));
        socket_ = new Socket(port);
        socket_->accept();
    } catch (SocketException e) {
        cout << e.what() << endl;
    }
}

Server::~Server()
{
    if (socket_ != NULL) delete socket_;
}

void
Server::processCommands(int port)
{
    try {
        if (instance_ == 0) {
            if (!closeConnection_) {
                instance_ = new Server(port);
            } else {
                return;
            }
        }

        if (instance_->myDoingSimStep) {
            instance_->myDoingSimStep = false;
        }

        while (!closeConnection_) {
            if (!instance_->myInputStorage.valid_pos()) {
                if (instance_->myOutputStorage.size() > 0) {
                    // send out all answers as one storage
                    instance_->socket_->sendExact(instance_->myOutputStorage);
                }
                instance_->myInputStorage.reset();
                instance_->myOutputStorage.reset();
                // Read a message
                instance_->socket_->receiveExact(instance_->myInputStorage);
            }
            while (instance_->myInputStorage.valid_pos() && !closeConnection_) {
                // dispatch each command
                int cmd = instance_->dispatchCommand();
            }
        }
        if (closeConnection_ && instance_->myOutputStorage.size() > 0) {
            // send out all answers as one storage
            instance_->socket_->sendExact(instance_->myOutputStorage);
        }
    } catch (std::invalid_argument e) {
        MsgHandler::getErrorInstance()->inform("Invalid argument: " + toString(e.what()));

    } catch (SocketException e) {
        MsgHandler::getErrorInstance()->inform("Socket exception: " + toString(e.what()));
    }

    if (instance_ != NULL) {
        delete instance_;
        instance_ = 0;
        closeConnection_ = true;
    }
}

int
Server::dispatchCommand()
{
    int commandStart = myInputStorage.position();
    int commandLength = myInputStorage.readInt();
    if (commandLength==0) {
        commandLength = myInputStorage.readInt();
    }

    int commandId = myInputStorage.readUnsignedByte();
    bool success = false;
    // dispatch commands
    switch (commandId) {
    case CMD_ASK_FOR_SUBSCRIPTION: {
        success = LookForNewSubscriptions();
        break;
    }
    case CMD_END_SUBSCRIPTION: {
        success = CheckSubscriptionStatus();
        break;
    }
    case CMD_CARS_IN_ZONE: {
        success = ReceiveSubscriptionCarsInZone();
        break;
    }
    case CMD_NOTIFY_APP_MESSAGE_STATUS: {
        success = ReceiveAppMessageStatusNotification();
        break;
    }
    case CMD_NOTIFY_APP_EXECUTE: {
        success = ExecuteApplication();
        break;
    }
    case CMD_APP_CLOSE: {
        closeConnection_ = true;
        success = true;
        break;
    }
    default:
        writeStatusCmd(commandId, APP_RTYPE_NOTIMPLEMENTED, "Command not implemented in Apps");
    }

    if (!success) {
        while (myInputStorage.valid_pos() && myInputStorage.position() < commandStart + commandLength) {
            myInputStorage.readChar();
        }
    }

    if (myInputStorage.position() != commandStart + commandLength) {
        ostringstream msg;
        msg << "Wrong position in requestMessage after dispatching command.";
        msg << " Expected command length was " << commandLength;
        msg << " but " << myInputStorage.position() - commandStart << " Bytes were read.";
        writeStatusCmd(commandId, APP_RTYPE_ERR, msg.str());
        closeConnection_ = true;
    }

    return commandId;
}

bool
Server::LookForNewSubscriptions()
{
    int timestep = myInputStorage.readInt();
    int nodeId = myInputStorage.readInt();

    // create reply message
    writeStatusCmd(CMD_ASK_FOR_SUBSCRIPTION, APP_RTYPE_OK, "");

    if (!ApplicationLogic::m_camAreaIsSet) {
        Area area = ApplicationLogic::GetCamArea();
        SubsSetCamArea(myOutputStorage, nodeId, timestep, area);
    } else if (!ApplicationLogic::m_returnCarAreaIsSet) {
        Area area = ApplicationLogic::GetReturningCarArea();
        SubsReturnCarsInZone(myOutputStorage, nodeId, timestep, area);
    } else if (ApplicationLogic::m_camAreaIsSet && ApplicationLogic::m_returnCarAreaIsSet) {
        StopAskingForSubscription(myOutputStorage, nodeId, timestep);
    }

    return true;
}

bool
Server::CheckSubscriptionStatus()
{
    int timestep = myInputStorage.readInt();
    int nodeId = myInputStorage.readInt();
    int subscriptionType = myInputStorage.readUnsignedByte();

    // create reply message
    writeStatusCmd(CMD_END_SUBSCRIPTION, APP_RTYPE_OK, "");

    if (ApplicationLogic::DropSubscription()) {
        // command length
        myOutputStorage.writeUnsignedByte(1 + 1 + 1);
        // command type
        myOutputStorage.writeUnsignedByte(CMD_END_SUBSCRIPTION);
        // subscription action
        myOutputStorage.writeUnsignedByte(CMD_DROP_SUBSCRIPTION);
    } else {
        // command length
        myOutputStorage.writeUnsignedByte(1 + 1 + 1);
        // command type
        myOutputStorage.writeUnsignedByte(CMD_END_SUBSCRIPTION);
        // subscription action
        myOutputStorage.writeUnsignedByte(CMD_RENEW_SUBSCRIPTION);
    }

    return true;

}

bool
Server::ReceiveSubscriptionCarsInZone()
{
    int timestep = myInputStorage.readInt();
    int nodeId = myInputStorage.readInt();

    int numberfOfCars = myInputStorage.readInt();
    vector<Vehicle> vehicles;
    for (int i = 0 ; i < numberfOfCars ; i++) {
        Vehicle v;
        v.id = myInputStorage.readInt();
        v.x = myInputStorage.readFloat();
        v.y = myInputStorage.readFloat();
        vehicles.push_back(v);
    }

    ApplicationLogic::ProcessSubscriptionCarsInZone(vehicles);

    // create reply message
    writeStatusCmd(CMD_CARS_IN_ZONE, APP_RTYPE_OK, "");

    return true;
}

bool
Server::ExecuteApplication()
{
    int timestep = myInputStorage.readInt();
    int nodeId = myInputStorage.readInt();

    // create reply message
    writeStatusCmd(CMD_NOTIFY_APP_EXECUTE, APP_RTYPE_OK, "");

    vector<AppMessage> messages = ApplicationLogic::SendBackExecutionResults(nodeId, timestep);

    if (messages.size() != 0) {
        int commandLength = 4 + 1 + 1 + (messages.size() * (4 + 4 + 4 + 4 + 4 + 4 + 4));
        // command length
        myOutputStorage.writeInt(commandLength);
        // command id
        myOutputStorage.writeUnsignedByte(CMD_NOTIFY_APP_EXECUTE);
        // command result status
        myOutputStorage.writeUnsignedByte(APP_RESULT_ON);
        // command number of results
        myOutputStorage.writeInt(messages.size());
        for (vector<AppMessage>::const_iterator it = messages.begin() ; it != messages.end() ; it++) {
            AppMessage message = *it;
            // destination station id of the result
            myOutputStorage.writeInt(message.destinationId);
            // payload value
            myOutputStorage.writeFloat(message.payloadValue);
            // message identifier
            myOutputStorage.writeInt(message.messageId);
            switch (message.status) {
            case(kToBeScheduled) : {
                myOutputStorage.writeInt(APP_RESULT_TO_SCHEDULE); // action of the result
                myOutputStorage.writeInt(1); // fixed frequency
                myOutputStorage.writeInt(10); // fixed payload length
                myOutputStorage.writeFloat(20); // fixed regeneration time
                break;
            }
            case(kToBeApplied) : {
                myOutputStorage.writeInt(APP_RESULT_TO_APPLY); // action of the result
                myOutputStorage.writeInt(1); // fixed frequency
                myOutputStorage.writeInt(10); // fixed payload length
                myOutputStorage.writeFloat(20); // fixed regeneration time
                break;
            }
            default: {
            }
            }
        }
    } else {
        // command length
        myOutputStorage.writeInt(4 + 1 + 1);
        // command id
        myOutputStorage.writeUnsignedByte(CMD_NOTIFY_APP_EXECUTE);
        // command result status
        myOutputStorage.writeUnsignedByte(APP_RESULT_OFF);
    }

    return true;
}

void
Server::writeStatusCmd(int commandId, int status, const std::string description)
{
    if (status == APP_RTYPE_ERR) {
        MsgHandler::getErrorInstance()->inform("Answered with error to command " + toString(commandId) + " " + toString(description));
    } else if (status == APP_RTYPE_NOTIMPLEMENTED) {
        MsgHandler::getErrorInstance()->inform("Requested command not implemented " + toString(commandId) + " " + toString(description));
    }

    description.length();
    // command length
    myOutputStorage.writeUnsignedByte(1 + 1 + 1 + 4 + static_cast<int>(description.length()));
    // command type
    myOutputStorage.writeUnsignedByte(commandId);
    // status
    myOutputStorage.writeUnsignedByte(status);
    // description
    myOutputStorage.writeString(description);

    return;
}

bool
Server::SubsReturnCarsInZone(Storage& myOutputStorage, int nodeId, int timestep, Area area)
{
    // command length
    myOutputStorage.writeUnsignedByte(1 + 1 + 1 + 4 + 4 + 4);
    // command type
    myOutputStorage.writeUnsignedByte(CMD_ASK_FOR_SUBSCRIPTION);
    // subscription type
    myOutputStorage.writeUnsignedByte(SUB_RETURNS_CARS_IN_ZONE);
    // baseX
    myOutputStorage.writeFloat(area.x);
    // baseY
    myOutputStorage.writeFloat(area.y);
    // radius
    myOutputStorage.writeFloat(area.radius);

    return true;
}

bool
Server::SubsSetCamArea(Storage& myOutputStorage, int nodeId, int timestep, Area area)
{
    // command length
    myOutputStorage.writeUnsignedByte(1 + 1 + 1 + 4 + 4 + 4 + 4 + 1);
    // command type
    myOutputStorage.writeUnsignedByte(CMD_ASK_FOR_SUBSCRIPTION);
    // subscription type
    myOutputStorage.writeUnsignedByte(SUB_SET_CAM_AREA);
    // baseX
    myOutputStorage.writeFloat(area.x);
    // baseY
    myOutputStorage.writeFloat(area.y);
    // radius
    myOutputStorage.writeFloat(area.radius);
    // frequency of the CAM messages within the area
    myOutputStorage.writeFloat(1);
    // info to be transmitted within the area - UNUSED
    myOutputStorage.writeUnsignedByte(0);

    return true;
}

bool
Server::ReceiveAppMessageStatusNotification()
{
    // current time step
    myInputStorage.readInt();
    // node id
    myInputStorage.readInt();
    // number of messages
    vector<AppMessage> messages;
    int numOfMessages = myInputStorage.readInt();
    for (int i = 0; i < numOfMessages ; i++) {
        AppMessage message;
        message.status = kArrived;
        // message id
        message.messageId = myInputStorage.readInt();
        // receiver id
        message.destinationId = myInputStorage.readInt();
        messages.push_back(message);
    }

    ApplicationLogic::ProcessMessageNotifications(messages);

    // create reply message
    writeStatusCmd(CMD_NOTIFY_APP_MESSAGE_STATUS, APP_RTYPE_OK, "");

    return true;
}

bool
Server::StopAskingForSubscription(Storage& myOutputStorage, int nodeId, int timestep)
{
    // command length
    myOutputStorage.writeUnsignedByte(1 + 1 + 1);
    // command type
    myOutputStorage.writeUnsignedByte(CMD_ASK_FOR_SUBSCRIPTION);
    // subscription type
    myOutputStorage.writeUnsignedByte(CMD_END_SUBSCRIPTION_REQUEST);

	return true;
}