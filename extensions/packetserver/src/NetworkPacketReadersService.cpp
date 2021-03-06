/**
*** Copyright (c) 2016-present,
*** Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp. All rights reserved.
***
*** This file is part of Catapult.
***
*** Catapult is free software: you can redistribute it and/or modify
*** it under the terms of the GNU Lesser General Public License as published by
*** the Free Software Foundation, either version 3 of the License, or
*** (at your option) any later version.
***
*** Catapult is distributed in the hope that it will be useful,
*** but WITHOUT ANY WARRANTY; without even the implied warranty of
*** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*** GNU Lesser General Public License for more details.
***
*** You should have received a copy of the GNU Lesser General Public License
*** along with Catapult. If not, see <http://www.gnu.org/licenses/>.
**/

#include "NetworkPacketReadersService.h"
#include "catapult/config/LocalNodeConfiguration.h"
#include "catapult/extensions/NetworkUtils.h"
#include "catapult/extensions/PeersConnectionTasks.h"
#include "catapult/extensions/ServiceLocator.h"
#include "catapult/extensions/ServiceState.h"
#include "catapult/thread/MultiServicePool.h"

namespace catapult { namespace packetserver {

	namespace {
		constexpr auto Service_Name = "readers";

		thread::Task CreateAgePeersTask(extensions::ServiceState& state, net::ConnectionContainer& connectionContainer) {
			const auto& connectionsConfig = state.config().Node.IncomingConnections;
			auto& nodes = state.nodes();

			auto serviceId = ionet::ServiceIdentifier(0x52454144);
			auto selector = extensions::CreateRemoveOnlyNodeSelector(serviceId, connectionsConfig, nodes);
			auto task = extensions::CreateAgePeersTask(nodes, connectionContainer, serviceId, selector);
			task.Name += " for service Readers";
			return task;
		}

		class NetworkPacketReadersServiceRegistrar : public extensions::ServiceRegistrar {
		public:
			extensions::ServiceRegistrarInfo info() const override {
				return { "NetworkPacketReaders", extensions::ServiceRegistrarPhase::Post_Packet_Handlers };
			}

			void registerServiceCounters(extensions::ServiceLocator& locator) override {
				locator.registerServiceCounter<net::PacketReaders>(Service_Name, "READERS", [](const auto& writers) {
					return writers.numActiveReaders();
				});
			}

			void registerServices(extensions::ServiceLocator& locator, extensions::ServiceState& state) override {
				const auto& config = state.config();
				auto pServiceGroup = state.pool().pushServiceGroup(Service_Name);
				auto pReaders = pServiceGroup->pushService(
						net::CreatePacketReaders,
						state.packetHandlers(),
						locator.keyPair(),
						extensions::GetConnectionSettings(config),
						extensions::GetMaxIncomingConnectionsPerIdentity(config.Node.Local.Roles));
				extensions::BootServer(*pServiceGroup, config.Node.Port, config, [&acceptor = *pReaders](
						const auto& socketInfo,
						const auto& callback) {
					acceptor.accept(socketInfo, callback);
				});

				locator.registerService(Service_Name, pReaders);

				// add tasks
				state.tasks().push_back(CreateAgePeersTask(state, *pReaders));
			}
		};
	}

	DECLARE_SERVICE_REGISTRAR(NetworkPacketReaders)() {
		return std::make_unique<NetworkPacketReadersServiceRegistrar>();
	}
}}
