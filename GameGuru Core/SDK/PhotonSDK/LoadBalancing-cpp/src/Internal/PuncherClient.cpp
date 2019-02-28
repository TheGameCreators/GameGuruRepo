/* Exit Games Photon LoadBalancing - C++ Client Lib
 * Copyright (C) 2004-2018 by Exit Games GmbH. All rights reserved.
 * http://www.photonengine.com
 * mailto:developer@photonengine.com
 */

#include "Common-cpp/inc/MemoryManagement/Allocate.h"
#include "LoadBalancing-cpp/inc/Internal/PuncherClient.h"
#include "LoadBalancing-cpp/inc/Client.h"
#include "LoadBalancing-cpp/inc/Internal/Enums/EventCode.h"

namespace ExitGames
{
	namespace LoadBalancing
	{
		namespace Internal
		{
			using namespace Common;
			using namespace Common::MemoryManagement;
			using namespace Internal::EventCode;

			PuncherClient::PuncherClient(Client& client, Listener& listener, const Common::Logger& logger)
				: mpPuncher(NULL)
				, mLoadBalancingClient(client)
				, mLoadBalancingListener(listener)
				, mLogger(logger)
			{
			}

			PuncherClient::~PuncherClient(void)
			{
				deallocate(mpPuncher);
			}

			bool PuncherClient::initPuncher(void)
			{
				ALLOCATE(Photon::Punchthrough::Puncher, mpPuncher, this, mLogger);
				if(mpPuncher->init(this))
					EGLOG(DebugLevel::INFO, L"Puncher init OK");
				else
					EGLOG(DebugLevel::ERRORS, L"Failed to init Puncher");
				return true;
			}

			bool PuncherClient::startPunch(int playerNr)
			{
				if(mpPuncher)
				{
					bool res = mpPuncher->startPunch(playerNr);
					if(res)
						EGLOG(DebugLevel::ALL, L"punch started with player %d", playerNr);
					else
						EGLOG(DebugLevel::ERRORS, L"punch start failed with player %d", playerNr);
					return res;
				}
				else
				{
					EGLOG(DebugLevel::ERRORS, L"startPunch: puncher is not initialized");
					return false;
				}
			}

			bool PuncherClient::sendDirect(const JVector<nByte>& buffer, int targetID, bool fallbackRelay)
			{
				if(mpPuncher)
					return mpPuncher->sendDirect(buffer, targetID, fallbackRelay);
				else
				{
					EGLOG(DebugLevel::ERRORS, L" puncher is not initialized");
					return false;
				}
			}

			int PuncherClient::sendDirect(const JVector<nByte>& buffer, const JVector<int>& targetIDs, bool fallbackRelay)
			{
				if(mpPuncher)
					return mpPuncher->sendDirect(buffer, targetIDs, fallbackRelay);
				else
				{
					EGLOG(DebugLevel::ERRORS, L"puncher is not initialized");
					return 0;
				}
			}

			void PuncherClient::service(void)
			{
				if(mpPuncher)
					mpPuncher->service();
			}

			bool PuncherClient::processRelayPackage(const JVector<nByte>& packet, int remoteID)
			{
				if(mpPuncher)
					return mpPuncher->processPackage(packet, true, remoteID);
				else
					return false;
			}

			int PuncherClient::getLocalID(void)
			{
				return mLoadBalancingClient.getLocalPlayer().getNumber();
			}

			void PuncherClient::onReceiveDirect(const JVector<nByte>& buffer, int remoteID, bool relay) 
			{
				Common::DeSerializer d(buffer.getCArray(), buffer.getSize());
				ExitGames::Common::Object msg;
				d.pop(msg);
				mLoadBalancingListener.onDirectMessage(msg, remoteID, relay);
			}

			bool PuncherClient::sendRelay(const JVector<nByte>& buffer, const JVector<int>& targetIDs)
			{
				return buffer.getSize()?mLoadBalancingClient.opRaiseEvent(true, buffer.getCArray(), buffer.getSize(), PUNCH_MSG, RaiseEventOptions().setTargetPlayers(targetIDs.getCArray(), targetIDs.getSize())):false;
			}
		}
	}
}