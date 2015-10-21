#include <tins/tins.h>
#include <iostream>
#include <string>
#include <unordered_map>

using namespace Tins;

std::map<HWAddress<6>, Timestamp> devices;

void process(Packet p) {
	if (p.pdu()->find_pdu<Dot11ProbeRequest>(PDU::DOT11_PROBE_REQ)) {
		Timestamp timestamp = p.timestamp();
		Dot11ProbeRequest req = p.pdu()->rfind_pdu<Dot11ProbeRequest>(PDU::DOT11_PROBE_REQ);
		HWAddress<6> mac = req.addr2();
		if (devices.count(mac) == 0) {
			std::cout << "First probe request received from " << mac.to_string() << " at " << timestamp.seconds() << std::endl;
			std::cout << "Number of devices in your area " << devices.size() << std::endl;
		}
		devices[mac] = timestamp;
	}
}

int main(int argc, char *argv[]) {
	NetworkInterface iface;
	if (argc != 2)
		iface = NetworkInterface::default_interface();
	else iface = NetworkInterface(argv[1]);

	//NetworkInterface::Info info = iface.addresses();

	SnifferConfiguration config;
	config.set_rfmon(true);

	config.set_filter("type mgt subtype probe-req");

	Sniffer sniffer(iface.name(), config);

	while(true) {
		process(sniffer.next_packet());
	}

	return 0;
}
