#include "mock_homeobject.hpp"

#include <boost/uuid/uuid_io.hpp>

namespace homeobject {

void MockHomeObject::create_pg(PGInfo const& pg_info, PGManager::ok_cb const& cb) {
    LOGINFO("Creating PG: [{}] of [{}] members", pg_info.id, pg_info.members.size());
    auto err = PGError::OK;
    if (std::none_of(pg_info.members.begin(), pg_info.members.end(),
                     [](PGMember const& m) { return 0 < m.priority; })) {
        LOGERROR("No possible leader for PG: [{}]", pg_info.id);
        err = PGError::INVALID_ARG;
    } else {
        auto lg = std::scoped_lock(_pg_lock);
        [[maybe_unused]] auto [it, _] =
            _pg_map.insert(std::make_pair(pg_info.id, std::make_pair(pg_info, std::unordered_set< shard_id >())));
        DEBUG_ASSERT(_pg_map.end() != it, "Failure to insert Pg into Map!");
    }
    if (cb) cb(err);
}

void MockHomeObject::replace_member(pg_id id, peer_id const& old_member, PGMember const& new_member,
                                    PGManager::ok_cb const& cb) {
    LOGINFO("Replacing PG: [{}] member [{}] with [{}]", id, to_string(old_member), to_string(new_member.id));
    auto err = PGError::OK;
    {
        auto lg = std::scoped_lock(_pg_lock);
        auto pg_it = _pg_map.end();
        if (pg_it = _pg_map.find(id); _pg_map.end() == pg_it) {
            err = PGError::UNKNOWN_PG;
        } else if (auto& members = pg_it->second.first.members; 0 == members.erase(old_member)) {
            err = PGError::UNKNOWN_PEER;
        } else {
            auto [it, happened] = members.insert(new_member);
            if (!happened) {
                LOGERROR("Already have this member [{}] in [pg={}]", to_string(new_member.id), id);
                err = PGError::INVALID_ARG;
            }
        }
    }
    if (cb) cb(err);
}

extern std::shared_ptr< HomeObject > init_homeobject(init_params const& params) {
    return std::make_shared< MockHomeObject >(params.lookup);
}

} // namespace homeobject
