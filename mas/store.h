#pragma once
#include <functional>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

namespace mas {

template <typename S, typename R, typename A>
R combine_reducers(const std::vector<R>& reducers) {
    return [=](const S& s, const A& a)->S {
        S new_state = s;  // 复制
        for (auto& sub_reducer : reducers) {
            new_state = sub_reducer(new_state, a);
        }
        return new_state;
    };
}

template <typename S, typename R, typename A>
std::function<std::unordered_map<std::string, S>(
    const std::unordered_map<std::string, S>&, const A&)>
combine_mapped_reducers(
    const std::unordered_map<std::string, R>& reducers,
    std::function<std::string(const A&)> splitter = nullptr) {
    return [=](
               const std::unordered_map<std::string, S>& s, const A& a)
        ->std::unordered_map<std::string, S> {
        std::unordered_map<std::string, S> new_state = s;  // 复制
        for (auto& it : reducers) {
            auto search = s.find(it.first);  // 查找reducer对应的state子域
            if (search == s.end()) {
                continue;
            }

            if (splitter) {  // 定义splitter，则action需要指定的reducer来处理
                std::string key = splitter(a);
                if (key != it.first) {
                    continue;  // 这个reducer不应该处理这个action
                }
            }

            auto& sub_reducer = it.second;
            auto& sub_state = search->second;
            new_state[it.first] = sub_reducer(sub_state, a);
        }
        return new_state;
    };
}

// abstract store, should not focus on details.
template <typename State, typename Action, typename Reducer>
class Store {
public:
    using Func = std::function<int(const State&)>;
    using Funcs = std::vector<Func>;
    using FuncsMap = std::unordered_map<std::string, Funcs>;
    using Reducers = std::vector<Reducer>;
    using Chooser = std::function<std::string()>;
    using Condition = std::function<std::string(const Action& action)>;

    Store(const State& state, const Reducer& reducer)
        : state_(state), reducer_(reducer) {}

    virtual ~Store() = default;

    virtual const State& state() { return state_; }

    virtual int dispatch(const Action& action, const Condition& cond) = 0;

    virtual int dispatch(const Action& action, const std::string& key) = 0;

    virtual int subscribe(const Chooser& c, Func f) {
        std::string key = c();
        funcs_map_[key].push_back(std::move(f));
        return 0;
    }

protected:
    State state_;
    FuncsMap funcs_map_;
    Reducer reducer_;
};

template <typename State, typename Action>
using SimpleReducer = std::function<State(const State&, const Action&)>;

template <typename State, typename Action>
class SimpleStore : public Store<State, Action, SimpleReducer<State, Action>> {
public:
    using ConcreteSimpleReducer = SimpleReducer<State, Action>;
    using Base = Store<State, Action, ConcreteSimpleReducer>;

    SimpleStore(const State& state, const ConcreteSimpleReducer& reducer)
        : Store<State, Action, ConcreteSimpleReducer>(state, reducer) {}

    ~SimpleStore() override {}

    // `cond` to choose subscribers
    int dispatch(const Action& action,
                 const typename Base::Condition& cond) override {
        this->state_ = this->reducer_(this->state_, action);
        std::string key = cond(action);
        return publish(key);
    }

    // `key` to choose subscribers
    int dispatch(const Action& action, const std::string& key) override {
        this->state_ = this->reducer_(this->state_, action);
        return publish(key);
    }

    int subscribe(const std::string& event, typename Base::Func f) {
        return Base::subscribe([event]() { return event; }, std::move(f));
    }

private:
    int publish(const std::string& sub_key) {
        auto search = this->funcs_map_.find(sub_key);
        int result = 0;
        if (search != this->funcs_map_.end()) {
            for (auto& f : search->second) {
                int ret = f(this->state_);
                if (ret != 0) {
                    result--;
                }
            }
        }
        return result;
    }
};

}  // namespace mas