/*
 * (C) Averisera Ltd 2015
 */
#include <gtest/gtest.h>
#include "microsim-simulator/archived_history.hpp"

using namespace averisera;
using namespace averisera::microsim;

TEST(ArchivedHistory, DefaultConstructor) {
    ArchivedHistory ah;
    ASSERT_EQ(0u, ah.actor_id());
    ASSERT_EQ(0u, ah.variable_id());
    ASSERT_TRUE(ah.empty());
    ASSERT_THROW(ah.history(), std::logic_error);
}

class MockHistory: public History {
public:
    MockHistory(int* flag)
    : _flag(flag) {}    
    ~MockHistory() {
        ++(*_flag);
    }
    bool empty() const override { return true; }
    Date last_date() const override { return Date(); }
    Date first_date() const override { return Date(); }
    Date last_date(Date) const override { return Date(); }
    double_t as_double(Date) const override { return 0; }
    int_t as_int(Date) const override { return 0; }
    double_t last_as_double() const override { return 0; }
    int_t last_as_int() const override { return 0; }
    double_t last_as_double(Date) const override { return 0; }
    int_t last_as_int(Date) const override { return 0; }
    void append(Date, double_t) override {}
    void append(Date, int_t) override {}
    void correct(double_t) override {}
    index_t size() const override { return 0; }
    Date date(index_t) const override { return Date(); }
    double_t as_double(index_t) const override { return 0.; }
    int_t as_int(index_t) const override { return 0; }
    index_t last_index(Date) const override { return 0; }
    index_t first_index(Date) const override { return 0; }
    void print(std::ostream& os) const override { os << "MOCK"; }
    std::unique_ptr<History> clone() const override { return nullptr; }
	HistoryData to_data() const { return HistoryData(); }
	const std::string& name() const override {
		static const std::string name("MOCKITYMOCK");
		return name;
	}
private:
    int* _flag;
};

TEST(ArchivedHistory, FullConstructor) {
    int flag = 0;
    const MockHistory* mh = new MockHistory(&flag);
    {
        ArchivedHistory ah(101, 3, mh);
        ASSERT_EQ(mh, &ah.history());
        ASSERT_EQ(101u, ah.actor_id());
        ASSERT_EQ(3u, ah.variable_id());
        ASSERT_FALSE(ah.empty());
    }
    ASSERT_EQ(1, flag) << "History was destroyed by the destructor of ArchivedHistory";
}
