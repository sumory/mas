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

TEST_F(StoreTest, combine_reducers) {
    using mas::SimpleReducer;
    using mas::SimpleStore;
    using MappedState = std::unordered_map<std::string, State>;
    using ConcreteReducer = SimpleReducer<State, Action>;

    ConcreteReducer r1 = [](const State& state, const Action& action) -> State {
        State new_state = state;
        new_state.id--;
        return new_state;
    };

    ConcreteReducer r2 = [](const State& state, const Action& action) -> State {
        State new_state = state;
        if (action.event == "subtract") {
            new_state.id = new_state.id - 2;
        }
        return new_state;
    };

    auto reducer = mas::combine_reducers<State, ConcreteReducer, Action>(
        {r1, r2});  // 组装subtract reducer

    State s(100);
    SimpleStore<State, Action> store(s, reducer);

    Action action;
    store.dispatch(action, "any");  // 没有指定reducer, 使用r1
    EXPECT_EQ(99, store.state().id);

    action.event = "not_subtract";
    store.dispatch(action, "random");  // 没有指定reducer, 使用r1
    EXPECT_EQ(98, store.state().id);

    action.event = "subtract";
    store.dispatch(action, "random");  // 指定reducer, 使用r1和r2
    EXPECT_EQ(95, store.state().id);
}

TEST_F(StoreTest, combine_mapped_reducers_without_splitter) {
    using mas::SimpleReducer;
    using mas::SimpleStore;
    using MappedState = std::unordered_map<std::string, State>;

    using ConcreteReducer = SimpleReducer<State, Action>;

    ConcreteReducer r1 = [](const State& state, const Action& action) -> State {
        State new_state = state;
        new_state.id++;
        return new_state;
    };

    ConcreteReducer r2 = [](const State& state, const Action& action) -> State {
        State new_state = state;
        new_state.id--;
        return new_state;
    };

    ConcreteReducer r3 = [](const State& state, const Action& action) -> State {
        State new_state = state;
        new_state.id = new_state.id - 2;
        return new_state;
    };

    auto sub_subtract_reducers =
        mas::combine_reducers<State, ConcreteReducer, Action>(
            {r2, r3});  // 组装subtract reducer

    MappedState mapped_state{{"add", State(111)}, {"subtract", State(222)}};
    std::unordered_map<std::string, ConcreteReducer> reducers;
    reducers["add"] = r1;
    reducers["subtract"] = sub_subtract_reducers;

    // 组装map类型的reducers，注意这里没有加splitter，则这些reducers处理所有actions
    auto combined_reducer =
        mas::combine_mapped_reducers<State, ConcreteReducer, Action>(reducers);

    SimpleStore<MappedState, Action> store(mapped_state, combined_reducer);

    {
        Action action;
        action.event = "add";
        store.dispatch(action, "add");
        auto& state = store.state();
        auto search = state.find("add");
        EXPECT_EQ(112, search->second.id);
    }

    {
        Action action;
        action.event = "subtract";
        store.dispatch(action, [](const Action& a) {
            return a.event;
        });  // 支持通过解析action获取dispath目标
        auto& state = store.state();
        auto search = state.find("subtract");

        // 注意，因为combine的时候没有指明splitter，则上一个add类型的action也调用了sub_subtract_reducer
        // 所以最终结果为：原始值-1 -2 -1 -2，即222-6=216
        EXPECT_EQ(216, search->second.id);
    }
}

TEST_F(StoreTest, combine_mapped_reducers_with_splitter) {
    using mas::SimpleReducer;
    using mas::SimpleStore;
    using MappedState = std::unordered_map<std::string, State>;

    using ConcreteReducer = SimpleReducer<State, Action>;

    ConcreteReducer r1 = [](const State& state, const Action& action) -> State {
        State new_state = state;
        new_state.id++;
        return new_state;
    };

    ConcreteReducer r2 = [](const State& state, const Action& action) -> State {
        State new_state = state;
        new_state.id--;
        return new_state;
    };

    ConcreteReducer r3 = [](const State& state, const Action& action) -> State {
        State new_state = state;
        new_state.id = new_state.id - 2;
        return new_state;
    };

    auto sub_subtract_reducers =
        mas::combine_reducers<State, ConcreteReducer, Action>(
            {r2, r3});  // 组装subtract reducer

    MappedState mapped_state{{"add", State(111)}, {"subtract", State(222)}};
    std::unordered_map<std::string, ConcreteReducer> reducers;
    reducers["add"] = r1;
    reducers["subtract"] = sub_subtract_reducers;

    auto combined_reducer = mas::combine_mapped_reducers<
        State, ConcreteReducer, Action>(reducers, [](const Action& a) {
        return a.event;
    });  // 组装map类型的reducers，注意这里指明了splitter，则这些reducers只会处理对应的actions

    SimpleStore<MappedState, Action> store(mapped_state, combined_reducer);

    {
        Action action;
        action.event = "add";
        store.dispatch(action, "add");
        auto& state = store.state();
        auto search = state.find("add");
        EXPECT_EQ(112, search->second.id);
    }

    {
        Action action;
        action.event = "subtract";
        store.dispatch(action, [](const Action& a) {
            return a.event;
        });  // 支持通过解析action获取dispath目标
        auto& state = store.state();
        auto search = state.find("subtract");

        // 注意，因为combine的时候指明splitter，则上一个add类型的action不会影响到subtract类型的action
        // 所以最终结果为：原始值-1 -2，即222-3=219
        EXPECT_EQ(219, search->second.id);
    }
}
