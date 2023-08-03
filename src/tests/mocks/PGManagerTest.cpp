#include <chrono>
#include <string>

#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/string_generator.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include <sisl/logging/logging.h>
#include <sisl/options/options.h>

#include "mocks/mock_homeobject.hpp"

using namespace std::chrono_literals;
using homeobject::PGInfo;
using homeobject::PGMember;

SISL_LOGGING_INIT(logging, homeobject)
SISL_OPTIONS_ENABLE(logging)

class PgManagerFixture : public ::testing::Test {
public:
    boost::uuids::uuid _svc_id;

    void SetUp() override {
        auto params = homeobject::init_params{[](homeobject::peer_id const&) { return std::string(); }};
        m_mock_homeobj = homeobject::init_homeobject(params);
    }

protected:
    std::shared_ptr< homeobject::HomeObject > m_mock_homeobj;
};

TEST_F(PgManagerFixture, CreatePgEmpty) {
    m_mock_homeobj->pg_manager()->create_pg(
        PGInfo(0u), [](homeobject::PGError e) { EXPECT_EQ(homeobject::PGError::INVALID_ARG, e); });
}

TEST_F(PgManagerFixture, CreatePgNoLeader) {
    auto info = PGInfo(0u);
    info.members.insert(PGMember{boost::uuids::random_generator()()});
    m_mock_homeobj->pg_manager()->create_pg(
        PGInfo(0u), [](homeobject::PGError e) { EXPECT_EQ(homeobject::PGError::INVALID_ARG, e); });
}

TEST_F(PgManagerFixture, CreatePgOk) {
    auto info = PGInfo(0u);
    info.members.insert(PGMember{boost::uuids::random_generator()(), "noname", 1});
    m_mock_homeobj->pg_manager()->create_pg(
        PGInfo(0u), [](homeobject::PGError e) { EXPECT_EQ(homeobject::PGError::INVALID_ARG, e); });
}

int main(int argc, char* argv[]) {
    int parsed_argc = argc;
    ::testing::InitGoogleTest(&parsed_argc, argv);
    SISL_OPTIONS_LOAD(parsed_argc, argv, logging);
    sisl::logging::SetLogger(std::string(argv[0]));
    return RUN_ALL_TESTS();
}
