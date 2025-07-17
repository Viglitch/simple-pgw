#include "control_plane.h"

std::shared_ptr<pdn_connection> control_plane::find_pdn_by_cp_teid(uint32_t cp_teid) const {
    auto it = _pdns.find(cp_teid);
    return it != _pdns.end() ? it->second : nullptr;
}

std::shared_ptr<pdn_connection> control_plane::find_pdn_by_ip_address(const boost::asio::ip::address_v4& ip) const {
    auto it = _pdns_by_ue_ip_addr.find(ip);
    return it != _pdns_by_ue_ip_addr.end() ? it->second : nullptr;
}

std::shared_ptr<bearer> control_plane::find_bearer_by_dp_teid(uint32_t dp_teid) const {
    auto it = _bearers.find(dp_teid);
    return it != _bearers.end() ? it->second : nullptr;
}

std::shared_ptr<pdn_connection> control_plane::create_pdn_connection(const std::string& apn,
    boost::asio::ip::address_v4 sgw_addr,
    uint32_t sgw_cp_teid) {
    auto apn_it = _apns.find(apn);
    if (apn_it == _apns.end()) {
        return nullptr;
    }

    static uint32_t next_cp_teid = 1;
    uint32_t cp_teid = next_cp_teid++;

    static uint32_t next_ue_ip = 0x0A000001;
    boost::asio::ip::address_v4 ue_ip(next_ue_ip++);

    auto pdn = pdn_connection::create(cp_teid, apn_it->second, ue_ip);
    pdn->set_sgw_cp_teid(sgw_cp_teid);
    pdn->set_sgw_addr(sgw_addr);

    _pdns[cp_teid] = pdn;
    _pdns_by_ue_ip_addr[ue_ip] = pdn;

    return pdn;
}

void control_plane::delete_pdn_connection(uint32_t cp_teid) {
    auto it = _pdns.find(cp_teid);
    if (it != _pdns.end()) {
        auto pdn = it->second;
        _pdns_by_ue_ip_addr.erase(pdn->get_ue_ip_addr());
        _pdns.erase(it);
    }
}

std::shared_ptr<bearer> control_plane::create_bearer(const std::shared_ptr<pdn_connection>& pdn, uint32_t sgw_teid) {
    static uint32_t next_dp_teid = 1;
    uint32_t dp_teid = next_dp_teid++;

    auto new_bearer = std::make_shared<bearer>(dp_teid, *pdn);
    new_bearer->set_sgw_dp_teid(sgw_teid);

    _bearers[dp_teid] = new_bearer;
    pdn->add_bearer(new_bearer);

    if (!pdn->get_default_bearer()) {
        pdn->set_default_bearer(new_bearer);
    }

    return new_bearer;
}

void control_plane::delete_bearer(uint32_t dp_teid) {
    auto it = _bearers.find(dp_teid);
    if (it != _bearers.end()) {
        auto bearer = it->second;
        auto pdn = bearer->get_pdn_connection();
        pdn->remove_bearer(dp_teid);
        _bearers.erase(it);
    }
}

void control_plane::add_apn(std::string apn_name, boost::asio::ip::address_v4 apn_gateway) {
    _apns[std::move(apn_name)] = apn_gateway;
}
