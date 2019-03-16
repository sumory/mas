#include <iostream>
#include "mas/store.h"
#include "test/test_common.h"

using namespace mas;

class State {
public:
    bool enable = false;
    int id = 1;

    State() {}
    explicit State(int i) : id(i) {}

    void print(const std::string& prefix = "") const {
        std::cout << prefix << "state, enable:" << (enable ? "true" : "false")
                  << " id:" << id << std::endl;
    }
};

class Action {
public:
    std::string event;
};

auto reducer = [](const State& state, const Action& action) -> State {
    State new_state = state;

    if (action.event == "add") {
        new_state.id++;
    } else if (action.event == "subtract") {
        new_state.id--;
    } else {
    }

    return new_state;
};

TEST_F(StoreTest, simple_store_basic) {
    using mas::SimpleStore;

    State s(100);
    SimpleStore<State, Action> store(s, reducer);

    {
        Action action;
        action.event = "add";
        store.dispatch(action, "add");
        auto& state = store.state();
        EXPECT_EQ(101, state.id);
    }

    {
        Action action;
        action.event = "subtract";
        store.dispatch(action, [](const Action& a) { return a.event; });
        auto& state = store.state();
        EXPECT_EQ(100, state.id);
    }

    {
        Action action;
        action.event = "not_exist";
        store.dispatch(action, [](const Action& a) { return a.event; });
        auto& state = store.state();
        EXPECT_EQ(100, state.id);
    }
}

TEST_F(StoreTest, simple_store_subscribe) {
    using mas::SimpleStore;

    State s(100);
    SimpleStore<State, Action> store(s, reducer);

    int add_subscriber = 0;
    store.subscribe("add", [&](const State& s) {
        add_subscriber++;
        return 0;
    });
    store.subscribe("add", [&](const State& s) {
        add_subscriber++;
        return 0;
    });

    int subtract_subscriber = 0;
    store.subscribe("subtract", [&](const State& s) {
        subtract_subscriber = 999;
        return 0;
    });

    Action action;
    action.event = "add";
    store.dispatch(action, "add");
    EXPECT_EQ(2, add_subscriber);

    action.event = "subtract";
    store.dispatch(action, [](const Action& a) { return a.event; });
    EXPECT_EQ(999, subtract_subscriber);
}
