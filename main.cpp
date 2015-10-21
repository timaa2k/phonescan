#include <tins/tins.h>
#include <iostream>
#include <string>
#include <unordered_map>
#include <thread>
#include <mutex>
#include <unistd.h>
#include <chrono>

#define TIMEOUT 300

using namespace Tins;

std::map<std::string, Timestamp> devices;
std::mutex map_mutex;

std::string vendor_string(HWAddress<6> macaddr) {
	return macaddr.to_string();
}

void process(Packet p) {
	if (p.pdu()->find_pdu<Dot11ProbeRequest>(PDU::DOT11_PROBE_REQ)) {
		Timestamp t = p.timestamp();
		Dot11ProbeRequest req = p.pdu()->rfind_pdu<Dot11ProbeRequest>(PDU::DOT11_PROBE_REQ);
		HWAddress<6> macaddr = req.addr2();
		std::string key = vendor_string(macaddr);
		{
			std::lock_guard<std::mutex> lock(map_mutex);
			if (devices.count(key) == 0)
				std::cout << "New device [" << key << "] found at " << t.seconds() << std::endl;
			devices[key] = t;
		}
	}
}

void receive_probe_requests(NetworkInterface iface) {
	SnifferConfiguration config;
	config.set_rfmon(true);
	config.set_filter("type mgt subtype probe-req");
	Sniffer sniffer(iface.name(), config);
	while(true) process(sniffer.next_packet());
}

std::chrono::system_clock::rep current_time_in_seconds(){
    auto now = std::chrono::system_clock::now().time_since_epoch();
    return std::chrono::duration_cast<std::chrono::seconds>(now).count();
}

void delete_devices() {
	while(true) {
		sleep(60);
		{
			std::lock_guard<std::mutex> lock(map_mutex);
			for(auto const &m : devices) {
				unsigned int duration = current_time_in_seconds() - m.second.seconds();
				if (duration > TIMEOUT) {
					std::cout << "Device [" << m.first << "] deleted after " << duration << "s of inactivity" << std::endl;
					devices.erase(m.first);
				}
			}
		}
	}
}

int main(int argc, char *argv[]) {
	NetworkInterface iface;
	if (argc != 2)
		iface = NetworkInterface::default_interface();
	else iface = NetworkInterface(argv[1]);

	std::thread insert(receive_probe_requests, iface);
	std::thread remove(delete_devices);

	while(true) {
		sleep(10);
		{
			std::lock_guard<std::mutex> lock(map_mutex);
			std::cout << "\n" << "Number of devices in your area: " << devices.size() << std::endl;
			for(auto const &m : devices) {
				unsigned int duration = current_time_in_seconds() - m.second.seconds();
				std::cout << "[" << m.first << "] last seen " << duration << "s ago" << std::endl;
			}
			std::cout << "\n";
		}
	}

	return 0;
}
