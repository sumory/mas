### Mas

a C++ port of [redux](https://redux.js.org/) library with some custom features.

```c++
#include <mas/store.h>

struct State {
    int id = 1;
};

struct Action {
    std::string event;
};

auto reducer = [](const State& state, const Action& action) -> State {
    State new_state = state;
    if (action.event == "add") {
        new_state.id++;
    } else if (action.event == "subtract") {
        new_state.id--;
    }
    return new_state;
};

int main(int argc, char** argv) {
    using mas::SimpleStore;

    State s;
    SimpleStore<State, Action> store(s, reducer);
    store.subscribe("add", [](const State& s) {
        std::cout << "receiver 1, receive add event" << std::endl;
        return 0;
    });
    store.subscribe("add", [](const State& s) {
        std::cout << "receiver 2, receive add event" << std::endl;
        return 0;
    });
    store.subscribe("subtract", [](const State& s) {
        std::cout << "receiver 3, receive subtract event" << std::endl;
        return 0;
    });

    Action action;
    {
        action.event = "add";
        store.dispatch(action, "add");
        auto& state = store.state();
        std::cout << "state id:" << state.id << std::endl;
    }
    {
        action.event = "subtract";
        store.dispatch(action, [](const Action& a) { return a.event; });
        auto& state = store.state();
        std::cout << "state id:" << state.id << std::endl;
    }
    
    return 0;
}

```

### License

[MIT](./LICENSE) License