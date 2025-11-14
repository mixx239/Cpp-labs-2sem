#pragma once

#include <cstdint>
#include <memory>
#include <vector>

template<typename Func, typename Arg>
concept method_pointer = requires(Func func, Arg obj) {
    (obj.*func)();
};

template<typename Func, typename Arg1, typename Arg2>
concept method_pointer_arg = requires(Func func, Arg1 obj, Arg2 arg) {
    (obj.*func)(arg);
};


class TTaskScheduler {
private:

    class TaskId {
    public:
        explicit TaskId(uint64_t id) : id_(id) {}

        uint64_t id() const {
            return id_;
        }
    private:
        uint64_t id_;
    };

    template<typename T>
    class FutureResult {
    public:
        FutureResult(uint64_t id) : id_(id) {}

        uint64_t id() const {
            return id_;
        }
    private:
        uint64_t id_;
    };

    class IResultHolder {
    public: 
        virtual ~IResultHolder() = default;
        virtual const std::type_info& type() const = 0;
    };

    template<typename T>
    class ResultHolder : public IResultHolder {
    public:
        ResultHolder(T value) : value_(value) {}

        const std::type_info& type() const {
            return typeid(T);
        }

        T get_value() const {
            return value_;
        }
    private:
        T value_;
    };


    class ITask {
    public:
        virtual ~ITask() = default;
        virtual void execute() = 0;
        virtual bool is_executed() const = 0;
        virtual const std::type_info& type() const = 0;
        virtual const IResultHolder& get_result() const = 0;
    };

    template<typename Func, typename Arg1, typename Arg2>
    class Task : public ITask {
    public:
        Task(TTaskScheduler& scheduler, Func func, Arg1 arg1, Arg2 arg2)
        : scheduler_(scheduler), func_(std::move(func)), arg1_(std::move(arg1)), arg2_(std::move(arg2)) {} 

        void execute() {
            auto&& value1 = scheduler_.get_arg_value(std::forward<Arg1>(arg1_));
            auto&& value2 = scheduler_.get_arg_value(std::forward<Arg2>(arg2_));
            if constexpr(method_pointer_arg<Func, decltype(value1), decltype(value2)>) {
                using result_type = decltype((value1.*func_)(std::forward<decltype(value2)>(value2)));
                result_ = std::make_unique<ResultHolder<result_type>>((value1.*func_)(std::forward<decltype(value2)>(value2)));
            } else {
                using result_type = decltype(func_(std::forward<decltype(value1)>(value1), std::forward<decltype(value2)>(value2)));
                result_ = std::make_unique<ResultHolder<result_type>>(func_(std::forward<decltype(value1)>(value1), std::forward<decltype(value2)>(value2)));
            }
        }

        const std::type_info& type() const {
            if (!result_) {
                throw std::runtime_error("Task not executed");
            }
            return result_->type();
        }

        const IResultHolder& get_result() const {
            if (!result_) {
                throw std::runtime_error("Task not executed");
            }
            return *result_;            
        }

        bool is_executed() const {
            return result_ != nullptr;
        }
    private:
        TTaskScheduler& scheduler_;
        Func func_;
        Arg1 arg1_;
        Arg2 arg2_;
        std::unique_ptr<IResultHolder> result_;
    };

    template<typename Func, typename Arg1>
    class Task<Func, Arg1, void> : public ITask {
    public:
        Task(TTaskScheduler& scheduler, Func func, Arg1 arg)
        : scheduler_(scheduler), func_(std::move(func)), arg1_(std::move(arg)) {} 
       
        void execute() {
            auto&& value1 = scheduler_.get_arg_value(std::forward<Arg1>(arg1_));
            if constexpr(method_pointer<Func, decltype(value1)>) {
                using result_type = decltype((value1.*func_)());
                result_ = std::make_unique<ResultHolder<result_type>>((value1.*func_)());
            } else {
               using result_type = decltype(func_(std::forward<decltype(value1)>(value1)));
               result_ = std::make_unique<ResultHolder<result_type>>(func_(std::forward<decltype(value1)>(value1)));
            }
        }

        const std::type_info& type() const {
            if (!result_) {
                throw std::runtime_error("Task not executed");
            }
            return result_->type();
        }

        const IResultHolder& get_result() const {
            if (!result_) {
                throw std::runtime_error("Task not executed");
            }
            return *result_;            
        }

        bool is_executed() const {
            return result_ != nullptr;
        }
    private:
        TTaskScheduler& scheduler_;
        Func func_;
        Arg1 arg1_;
        std::unique_ptr<IResultHolder> result_;
    };

    template<typename Func>
    class Task<Func, void, void> : public ITask {
    public:
        Task(TTaskScheduler& scheduler, Func func)
        : scheduler_(scheduler), func_(std::move(func)) {} 
       
        void execute() {
            using result_type = decltype(func_());
            result_ = std::make_unique<ResultHolder<result_type>>(func_());
        }

        const std::type_info& type() const {
            if (!result_) {
                throw std::runtime_error("Task not executed");
            }
            return result_->type();
        }

        const IResultHolder& get_result() const {
            if (!result_) {
                throw std::runtime_error("Task not executed");
            }
            return *result_;            
        }

        bool is_executed() const {
            return result_ != nullptr;
        }
    private:
        TTaskScheduler& scheduler_;
        Func func_;
        std::unique_ptr<IResultHolder> result_;
    };


    template<typename T>
    auto get_arg_value(T&& arg) {
        return std::forward<T>(arg);
    }

    template<typename T>
    T get_arg_value(T* arg) {
        return *arg;
    }

    template<typename T>
    T get_arg_value(FutureResult<T> arg) {
        return getResult<T>(TaskId(arg.id()));
    }

    TaskId add_task(std::shared_ptr<ITask> task) {
        ++next_id_;
        tasks_.push_back(task);
        dependent_tasks_.resize(tasks_.size());
        return TaskId(next_id_ - 1);        
    }

    void execute_dependent_tasks(uint64_t id) {
        for (auto task_id : dependent_tasks_[id]) {
            if (!tasks_[task_id]->is_executed()) {
                execute_dependent_tasks(task_id);
                tasks_[task_id] -> execute();
            }
        }
    }

    std::vector<std::shared_ptr<ITask>> tasks_;
    std::vector<std::vector<uint64_t>> dependent_tasks_;
    uint64_t next_id_;

public:
    TTaskScheduler() : next_id_(0) {}

    template<typename Func, typename Arg1, typename Arg2>
    TaskId add(Func&& func, Arg1&& arg1, Arg2&& arg2) {
        auto task = std::make_shared<Task<std::decay_t<Func>, std::decay_t<Arg1>, std::decay_t<Arg2>>>(*this, std::forward<Func>(func), std::forward<Arg1>(arg1), std::forward<Arg2>(arg2));
        return add_task(task);
    }
    template<typename Func, typename Arg1>
    TaskId add(Func&& func, Arg1&& arg1) {
        auto task = std::make_shared<Task<std::decay_t<Func>, std::decay_t<Arg1>, void>>(*this, std::forward<Func>(func), std::forward<Arg1>(arg1));
        return add_task(task);
    }
    template<typename Func>
    TaskId add(Func&& func) {
        auto task = std::make_shared<Task<std::decay_t<Func>, void, void>>(*this, std::forward<Func>(func));
        return add_task(task);
    }

    template<typename T>
    FutureResult<T> getFutureResult(TaskId task_id) {
        dependent_tasks_.resize(next_id_ + 1);
        dependent_tasks_[next_id_].push_back(task_id.id());
        return FutureResult<T>(task_id.id());
    }

    void executeAll() {
        for (uint64_t i = 0; i < tasks_.size(); ++i) {
            if (!tasks_[i]->is_executed()) {
                execute_dependent_tasks(i);
                tasks_[i]->execute();
            }
        }
    }

    template<typename T>
    T getResult(TaskId task_id) {
        auto& task = tasks_.at(task_id.id()); 
        if (!task->is_executed()) {
            execute_dependent_tasks(task_id.id());
            task -> execute();
        }
        if (task->type() != typeid(T)) {
            throw std::runtime_error("getResult: invalid type T");
        }
        return static_cast<const ResultHolder<T>&>(task->get_result()).get_value();
    }
};
