#include "control_plane.h"
#include "data_plane.h"
#include <iostream>
#include <boost/asio/ip/address_v4.hpp>

class TestDataPlane : public data_plane {
public:
    using data_plane::data_plane;

protected:
    void forward_packet_to_sgw(boost::asio::ip::address_v4 sgw_addr,
        uint32_t sgw_dp_teid,
        Packet&& packet) override {
        std::cout << "�������� ������ � SGW (" << sgw_addr.to_string()
            << "), TEID: " << sgw_dp_teid
            << ", ������: " << packet.size() << " ����\n";
    }

    void forward_packet_to_apn(boost::asio::ip::address_v4 apn_gateway,
        Packet&& packet) override {
        std::cout << "�������� ������ � APN (" << apn_gateway.to_string()
            << "), ������: " << packet.size() << " ����\n";
    }
};

int main() {
    control_plane cp;

    // ��������� APN
    cp.add_apn("internet", boost::asio::ip::address_v4::from_string("192.168.1.1"));

    // ������� PDN ����������
    auto pdn = cp.create_pdn_connection("internet",
        boost::asio::ip::address_v4::from_string("10.0.0.1"),
        12345);

    // ������� bearer
    auto bearer = cp.create_bearer(pdn, 54321);

    // ��������� �������� ������
    TestDataPlane dp(cp);

    // �������� uplink �������
    data_plane::Packet uplink_packet{ 1, 2, 3, 4, 5 };
    dp.handle_uplink(bearer->get_dp_teid(), std::move(uplink_packet));

    // �������� downlink �������
    data_plane::Packet downlink_packet{ 5, 4, 3, 2, 1 };
    dp.handle_downlink(pdn->get_ue_ip_addr(), std::move(downlink_packet));

    return 0;
}
