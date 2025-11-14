#pragma once

#include <optional>
#include <vector>
#include <sstream>
#include <filesystem>
#include <fstream> 
#include <unordered_map>

template <typename Key, typename Value>
struct KV {
	Key key;
	Value value;
};

template <typename Base, typename Joined>
struct JoinResult {
	Base base;
	std::optional<Joined> joined;
    bool operator==(const JoinResult& other) const {
        return (base == other.base && joined == other.joined);
    }
};


struct Adaptors {};


template<typename A>
concept Adaptor = std::derived_from<std::decay_t<A>, Adaptors>;

template<typename R>
concept Range = requires(R range) {
	range.begin();
	range.end();
};


template<Range R, Adaptor A>
auto operator|(R range, A adaptor) {
	return adaptor(range);
}


class AsVector : public Adaptors {
public: 
    AsVector() {}

    template<typename Range>
    auto operator()(Range range) const {
        using value_type = std::decay_t<Range>::value_type;
        std::vector<value_type> result;
        for (const auto& value : range) {
            result.push_back(value);
        }
        return result;
    }
};


class Write : public Adaptors {
public: 
    Write(std::ostream& stream, char delimiter)
    : stream_(stream), delimiter_(delimiter) {}

    template<typename Range>
    auto operator()(Range range) const {
        for (const auto& value : range) {
            stream_ << value << delimiter_;
        }
        return range;
    }
private:
    std::ostream& stream_;
    char delimiter_;
};

inline auto Out(std::ostream& stream) {
    return Write(stream, ' ');
}

template<typename S>
concept Stream = requires(S stream) {
	stream.clear();
	stream.seekg(0);
};

template<typename Input>
class AsDataFlowView : public Adaptors {
public: 
    AsDataFlowView(Input& input) : input_(input) {}

    using iterator = std::decay_t<Input>::iterator;
    using value_type = std::decay_t<Input>::value_type;
    
    auto begin() const {
		reset_if_stream();
        return input_.begin();
    }
    auto end() const {
        return input_.end();
    }
private:
	void reset_if_stream() const {
		if constexpr (Stream<value_type>) {
			for (auto it = input_.begin(); it != input_.end(); ++it) {
				it->clear();
				it->seekg(0);
			}
		}
	}
    Input& input_;
};

template<typename Input>
auto AsDataFlow(Input& input) {
    return AsDataFlowView<Input>(input);
}


template<typename It, typename Predicate> 
class FilterIterator {
public: 
    using value_type = It::value_type;
    using iterator_category = std::input_iterator_tag;
    using difference_type = It::difference_type;
    using pointer = It::pointer;
	using reference = value_type;

    FilterIterator(It first, It last, const Predicate& pred)
    : current_it(first), end_it(last), predicate(pred) {
        skip_invalid();
    }
    reference operator*() const {
        return *current_it;
    }
    pointer operator->() const {
        return &*current_it;
    }
    FilterIterator& operator++() {
        ++current_it;
        skip_invalid();
        return *this;
    }
    FilterIterator operator++(int) {
        auto tmp = *this;
        ++(*this);
        return tmp;
    }
    bool operator==(const FilterIterator& other) const {
        return (current_it == other.current_it);
    }
    bool operator!=(const FilterIterator& other) const {
        return (current_it != other.current_it);
    }
private:
    void skip_invalid() {
        while ((current_it != end_it) && !predicate(*current_it)) {
            ++current_it;
        }
    }
    It current_it;
    It end_it;
    Predicate predicate;
};

template<typename Range, typename Predicate>
class FilterView {
public:
    FilterView(Range range, Predicate predicate)
    : range_(range), predicate_(predicate) {}

    using value_type = std::decay_t<Range>::value_type;
    using iterator = FilterIterator<typename Range::iterator, Predicate>;
    auto begin() const {
        return FilterIterator(range_.begin(), range_.end(), predicate_);
    }
    auto end() const {
        return FilterIterator(range_.end(), range_.end(), predicate_);
    }
private:
    Range range_;
    Predicate predicate_;
};

template<typename Predicate>
class Filter : public Adaptors {
public:
    Filter(Predicate predicate) : predicate_(predicate) {}

    template<typename Range>
    auto operator()(Range range) const {
        return FilterView(range, predicate_);
    }
private:
    Predicate predicate_;
};


class DropNullopt : public Adaptors {
public:
    DropNullopt() {}

    template<typename Range>
    auto operator()(Range range) const {
        return range | Filter([](const auto& opt) { return opt.has_value(); });
    }
};


template<typename It, typename Func> 
class TransformIterator {
public: 
    using input_value_type = It::value_type;
    using value_type = std::invoke_result_t<Func, input_value_type>;
    using iterator_category = std::input_iterator_tag;
    using difference_type = It::difference_type;
    using pointer = value_type*;
    using reference = value_type&;

    TransformIterator(It first, It last, const Func& func)
    : current_it(first), end_it(last), func_(func) {}

    value_type operator*() const {
        return func_(*current_it);
    }
    TransformIterator& operator++() {
        ++current_it;
        return *this;
    }
    TransformIterator operator++(int) {
        auto tmp = *this;
        ++(*this);
        return tmp;
    }
    bool operator==(const TransformIterator& other) const {
        return (current_it == other.current_it);
    }
    bool operator!=(const TransformIterator& other) const {
        return (current_it != other.current_it);
    }
private:
    It current_it;
    It end_it;
    Func func_;
    
};

template<typename Range, typename Func>
class TransformView {
public:
    TransformView(Range range, Func func)
    : range_(range), func_(func) {}

    using iterator = TransformIterator<typename Range::iterator, Func>;
    using value_type = TransformIterator<typename Range::iterator, Func>::value_type;
    auto begin() const {
        return TransformIterator(range_.begin(), range_.end(), func_);
    }
    auto end() const {
        return TransformIterator(range_.end(), range_.end(), func_);
    }
private:
    Range range_;
    Func func_;
};

template<typename Func>
class Transform : public Adaptors {
public:
    Transform(Func func) : func_(func) {}

    template<typename Range>
    auto operator()(Range range) const {
        return TransformView(range, func_);
    }
private:
    Func func_;
};


class DirIterator {
public: 
    using value_type = std::filesystem::path;
    using iterator_category = std::input_iterator_tag;
    using difference_type =  std::ptrdiff_t;
    using pointer = std::filesystem::path*;
    using reference = std::filesystem::path;

    DirIterator(std::filesystem::path dir_path, bool recursive, bool is_end)
    : dir_path_(dir_path), recursive_(recursive), is_end_(is_end) {
        if (!is_end_) {
            if (recursive_) {
                current_r_it = std::filesystem::recursive_directory_iterator(dir_path_);
            } else {
                current_it = std::filesystem::directory_iterator(dir_path_);
            }
            ++(*this);
        } else {
            if (recursive_) {
                current_r_it = std::filesystem::recursive_directory_iterator();
            } else {
                current_it = std::filesystem::directory_iterator();
            }
        }
    }
    reference operator*() const {
        return current_path;
    }
    DirIterator& operator++() {
        if (is_end_) {
            return *this;
        }
        if (recursive_) {
            ++current_r_it;
            if (current_r_it == std::filesystem::recursive_directory_iterator()) {
                is_end_ = true;
            } else {
                current_path = current_r_it->path();
            }
        } else {
            ++current_it;
            if (current_it == std::filesystem::directory_iterator()) {
                is_end_ = true;
            } else {
                current_path = current_it->path();
            }
        }
        return *this;
    }
    DirIterator operator++(int) {
        auto tmp = *this;
        ++(*this);
        return tmp;
    }
    bool operator==(const DirIterator& other) const {
        if (is_end_ && other.is_end_) {
            return true;
        }
        if (recursive_ != other.recursive_) {
            return false;
        }
        if (recursive_) {
            return (current_r_it == other.current_r_it);
        }
        return (current_it == other.current_it);
    }
    bool operator!=(const DirIterator& other) const {
        if (is_end_ != other.is_end_) {
            return true;
        }
        if (recursive_ != other.recursive_) {
            return true;
        }
        if (recursive_) {
            return (current_r_it != other.current_r_it);
        }
        return (current_it != other.current_it);
    }
private:
    std::filesystem::path dir_path_;
    bool recursive_;
    bool is_end_;
    std::filesystem::directory_iterator current_it;
    std::filesystem::recursive_directory_iterator current_r_it;
    std::filesystem::path current_path;
};

class DirView {
public:
    DirView(std::filesystem::path dir_path, bool recursive)
    : dir_path_(dir_path), recursive_(recursive) {}

    using value_type = std::filesystem::path;
    using iterator = DirIterator;
    auto begin() const {
        return DirIterator(dir_path_, recursive_, false);
    }
    auto end() const {
        return DirIterator(dir_path_, recursive_, true);
    }
private:
    std::filesystem::path dir_path_;
    bool recursive_;
};


inline auto Dir(std::filesystem::path dir_path, bool recursive) {
    return DirView(dir_path, recursive);
}


template<typename It>
class OpenFilesIterator {
public: 
    using value_type = std::istream&;
    using iterator_category = std::input_iterator_tag;
    using difference_type =  std::ptrdiff_t;
    using pointer = std::istream*;
    using reference = std::istream&;

    OpenFilesIterator(It first, It last)
    : current_it(first), end_it(last) {
        open_next_file();
    }
    value_type operator*() {
        if (file_stream) {
            return *file_stream;
        }
        throw std::runtime_error("Open files error");
    }
    OpenFilesIterator& operator++() {
        ++current_it;
        if (current_it != end_it) {
            open_next_file();
        }
        return *this;
    }
    OpenFilesIterator operator++(int) {
        auto tmp = *this;
        ++(*this);
        return tmp;
    }
    bool operator==(const OpenFilesIterator& other) const {
        return (current_it == other.current_it);
    }
    bool operator!=(const OpenFilesIterator& other) const {
        return (current_it != other.current_it);
    }
private:
    void open_next_file() {
        file_stream.reset();
        if (current_it != end_it) {
            file_stream = std::make_shared<std::ifstream>(*current_it);
            if (!file_stream->is_open()) {
                file_stream.reset();
            }
        }
    }
    It current_it;
    It end_it;
    std::shared_ptr<std::ifstream> file_stream;
};

template<typename Range>
class OpenFilesView {
public:
    OpenFilesView(Range range)
    : range_(std::move(range)) {}

    using value_type = std::decay_t<Range>::value_type;
    using iterator = OpenFilesIterator<typename Range::iterator>;
    auto begin() const {
        return OpenFilesIterator(range_.begin(), range_.end());
    }
    auto end() const {
        return OpenFilesIterator(range_.end(), range_.end());
    }
private:
    Range range_;
};

class OpenFiles : public Adaptors {
public:
    OpenFiles() {}
    template<typename Range>
    auto operator()(Range range) {
        return OpenFilesView(range);
    }
};
    


template<typename It> 
class SplitIterator {
public: 
    using input_value_type = It::value_type;
    using value_type = std::string;
    using iterator_category = std::input_iterator_tag;
    using difference_type = It::difference_type;
    using pointer = value_type*;
    using reference = value_type&;

    SplitIterator(It first, It last, const std::string delimiters)
    : current_it(first), end_it(last), delimiters_(delimiters) {
        get_next_element();
    }

    value_type operator*() const {
        return current_element;
    }
    SplitIterator& operator++() {
        get_next_element();
        return *this;
    }
    SplitIterator operator++(int) {
        auto tmp = *this;
        ++(*this);
        return tmp;
    }
    bool operator==(const SplitIterator& other) const {
        return (current_it == other.current_it) && (current_element == other.current_element);
    }
    bool operator!=(const SplitIterator& other) const {
        return (current_it != other.current_it) || (current_element != other.current_element);
    }
private:
    void get_next_element() {
        while(true) {
            size_t delim_index = remaining_buffer.find_first_of(delimiters_);
            if (delim_index != std::string::npos) {
                current_element = remaining_buffer.substr(0, delim_index);
                remaining_buffer = remaining_buffer.substr(delim_index + 1);
                break;
            }
            if (current_it != end_it) {
                auto& stream = *current_it;
                char buffer[buffer_size];
                stream.read(buffer, buffer_size);
                size_t bytes_read = stream.gcount();
                if (bytes_read > 0) {
                    remaining_buffer.append(buffer, bytes_read);
                    continue;
                } else {
                    ++current_it;
                }
            }
            if (!remaining_buffer.empty()) {
                current_element = remaining_buffer;
                remaining_buffer.clear();
                break;
            }
            current_element.clear();
            break;
        }

    }
    It current_it;
    It end_it;
    std::string delimiters_;
    std::string remaining_buffer;
    std::string current_element;
    size_t buffer_size = 4096;
};

template<typename Range>
class SplitView {
public:
    SplitView(Range range, std::string delimiters)
    : range_(range), delimiters_(delimiters) {}

    using iterator = SplitIterator<typename Range::iterator>;
    using value_type = std::string;
    auto begin() const {
        return SplitIterator(range_.begin(), range_.end(), delimiters_);
    }
    auto end() const {
        return SplitIterator(range_.end(), range_.end(), delimiters_);
    }
private:
    Range range_;
    std::string delimiters_;
};


class Split : public Adaptors {
public:
    Split(std::string delimiters) : delimiters_(delimiters) {}

    template<typename Range>
    auto operator()(Range range) const {
        return SplitView(range, delimiters_);
    }
private:
    std::string delimiters_;
};



template<typename LeftIt, typename RightIt, typename LeftKey, typename RightKey> 
class JoinIterator {
public: 
    using left_value_type = LeftIt::value_type;
    using right_value_type = RightIt::value_type;
    using value_type = JoinResult<left_value_type, right_value_type>;
    using iterator_category = std::input_iterator_tag;
    using difference_type = LeftIt::difference_type;
    using pointer = value_type*;
    using reference = value_type;

    JoinIterator(LeftIt left_first, LeftIt left_last, RightIt right_first, RightIt right_last, LeftKey left_key, RightKey right_key)
    : left_it(left_first), left_end(left_last), right_it(right_first), right_end(right_last), left_key_func(left_key), right_key_func(right_key) {
        if (left_it != left_end) {
            find_match();
        }
    }
    reference operator*() const {
        return current_value;
    }
    pointer operator->() const {
        return &current_value;
    }
    JoinIterator& operator++() {
        ++left_it;
        if (left_it != left_end) {
            find_match();
        }
        return *this;
    }
    JoinIterator operator++(int) {
        auto tmp = *this;
        ++(*this);
        return tmp;
    }
    bool operator==(const JoinIterator& other) const {
        return (left_it == other.left_it);
    }
    bool operator!=(const JoinIterator& other) const {
        return (left_it != other.left_it);
    }
private:
    void find_match() {
        auto left_key = left_key_func(*left_it);
        auto match_right_it = std::find_if(right_it, right_end, [this, left_key](const auto& right_value) {return left_key == right_key_func(right_value);});
        if (match_right_it != right_end) {
            current_value = {*left_it, *match_right_it};
        } else {
            current_value = {*left_it, std::nullopt};
        }
    }
    LeftIt left_it;
    LeftIt left_end;
    RightIt right_it;
    RightIt right_end;
    LeftKey left_key_func;
    RightKey right_key_func;
    value_type current_value;
    bool is_kv_;
};

template<typename LeftRange, typename RightRange, typename LeftKey, typename RightKey> 
class JoinView {
public:
    JoinView(LeftRange left_range, RightRange right_range, LeftKey left_key, RightKey right_key)
    : left_range_(left_range), right_range_(right_range), left_key_func(left_key), right_key_func(right_key) {}

    using iterator = JoinIterator<typename LeftRange::iterator, typename RightRange::iterator, LeftKey, RightKey>;
    using value_type = iterator::value_type;
    auto begin() const {
        return JoinIterator(left_range_.begin(), left_range_.end(), right_range_.begin(), right_range_.end(), left_key_func, right_key_func);
    }
    auto end() const {
        return JoinIterator(left_range_.end(), left_range_.end(), right_range_.begin(), right_range_.end(), left_key_func, right_key_func);
    }
private:
    LeftRange left_range_;
    RightRange right_range_;
    LeftKey left_key_func;
    RightKey right_key_func;
};


template<typename RightRange>
class JoinKv : public Adaptors {
public:
    JoinKv(RightRange right_range) : right_range_(right_range) {}

    template<typename LeftRange>
    auto operator()(LeftRange left_range) const {
        auto left_key_func = [](const auto& kv) { return kv.key; };
        auto right_key_func = [](const auto& kv) { return kv.key; };
        auto join_view = JoinView(left_range, right_range_, left_key_func, right_key_func);

        return join_view | Transform([](const auto& join_value) {
            using LeftValue = decltype(join_value.base.value);
            using RightValue = decltype(join_value.joined->value);
            auto result = JoinResult<LeftValue, RightValue>{join_value.base.value, std::nullopt};
            if (join_value.joined) {
                result.joined = join_value.joined->value;
            }
            return result;
        });
    }

private:
    RightRange right_range_;
};

template<typename RightRange, typename LeftKey, typename RightKey>
class JoinNotKv : public Adaptors {
public:
    JoinNotKv(RightRange right_range, LeftKey left_key, RightKey right_key) : right_range_(right_range), left_key_func(left_key), right_key_func(right_key) {}

    template<typename LeftRange>
    auto operator()(LeftRange left_range) const {
        return JoinView(left_range, right_range_, left_key_func, right_key_func);
    }

private:
    RightRange right_range_;
    LeftKey left_key_func;
    RightKey right_key_func;
};


template<typename RightRange>
auto Join(RightRange right_range) {
    return JoinKv(right_range);
}

template<typename RightRange, typename LeftKey, typename RightKey>
auto Join(RightRange right_range, LeftKey left_key, RightKey right_key) {
    return JoinNotKv(right_range, left_key, right_key);
}



template<typename It, typename InitValue, typename AggregatorFunc, typename KeyFunc>
class AggregateByKeyIterator {
public: 
    using input_value_type = It::value_type;
	using key_type = std::decay_t<decltype(std::declval<KeyFunc>()(std::declval<input_value_type>()))>;
	using value_type = std::pair<key_type, InitValue>;
    using iterator_category = std::input_iterator_tag;
    using difference_type = It::difference_type;
    using pointer = value_type*;
    using reference = value_type&;

    AggregateByKeyIterator(It first, It last, InitValue init_value, AggregatorFunc aggregator_func, KeyFunc key_func, bool is_end)
    : current_range_it(first), end_range_it(last), init_value_(init_value), aggregator_func_(aggregator_func), key_func_(key_func), is_end_(is_end) {
        if (current_range_it != end_range_it) {
			if (!is_end_) {
				aggregate();
				current_it = aggregated_data.begin();
			}
		}
    }
    reference operator*() const {
        return *current_it;
    }
    pointer operator->() const {
        return &*current_it;
    }
    AggregateByKeyIterator& operator++() {
        ++current_it;
		if (current_it == aggregated_data.end()) {
			is_end_ = true;
		}
        return *this;
    }
    AggregateByKeyIterator operator++(int) {
        auto tmp = *this;
        ++(*this);
        return tmp;
    }
    bool operator==(const AggregateByKeyIterator& other) const {
        return ((is_end_ && other.is_end_) || (current_it == other.current_it));
    }
    bool operator!=(const AggregateByKeyIterator& other) const {
		return !(*this == other);
    }
private:
    void aggregate() {
		std::unordered_map<key_type, InitValue> temp_map;
		for (It it = current_range_it; it != end_range_it; ++it) {
			key_type key = key_func_(*it);
			if (temp_map.find(key) == temp_map.end()) {
				temp_map[key] = init_value_;
			}
			aggregator_func_(*it, temp_map[key]);
		}
		while (current_range_it != end_range_it) {
			key_type key = key_func_(*current_range_it);
			if (temp_map.count(key)) {
				aggregated_data.push_back(std::make_pair(key, temp_map[key]));
				temp_map.erase(key);
			}
			++current_range_it;
		}
	}
    It current_range_it;
    It end_range_it;
	InitValue init_value_;
	AggregatorFunc aggregator_func_;
	KeyFunc key_func_;
	bool is_end_;
	std::vector<value_type> aggregated_data;
	typename std::vector<value_type>::iterator current_it;
};

template<typename Range, typename InitValue, typename AggregatorFunc, typename KeyFunc>
class AggregateByKeyView {
public:
	AggregateByKeyView(Range range, InitValue init_value, AggregatorFunc aggregator_func, KeyFunc key_func) 
    : range_(range), init_value_(init_value), aggregator_func_(aggregator_func), key_func_(key_func) {}

    using iterator = AggregateByKeyIterator<typename Range::iterator, InitValue, AggregatorFunc, KeyFunc>;
	using value_type = iterator::value_type;
    auto begin() const {
        return AggregateByKeyIterator(range_.begin(), range_.end(), init_value_, aggregator_func_, key_func_, false);
    }
    auto end() const {
        return AggregateByKeyIterator(range_.end(), range_.end(), init_value_, aggregator_func_, key_func_, true);
    }
private:
    Range range_;
	InitValue init_value_;
	AggregatorFunc aggregator_func_;
	KeyFunc key_func_;
};

template<typename InitValue, typename AggregatorFunc, typename KeyFunc>
class AggregateByKey : public Adaptors {
public:
	AggregateByKey(InitValue init_value, AggregatorFunc aggregator_func, KeyFunc key_func)
	: init_value_(init_value), aggregator_func_(aggregator_func), key_func_(key_func) {}

    template<typename Range>
    auto operator()(Range range) const {
        return AggregateByKeyView(range, init_value_, aggregator_func_, key_func_);
    }
private:
	InitValue init_value_;
	AggregatorFunc aggregator_func_;
	KeyFunc key_func_;
};



template<typename Range>
class SplitExpectedView {
public: 
	using range_iterator = Range::iterator;
	using range_value_type = range_iterator::value_type;
	using value_type = range_value_type::value_type;
	using error_type = range_value_type::error_type;

	SplitExpectedView(Range range) : range_(range) {}

	auto unexpected_flow() const {
		return range_ | Filter([](const range_value_type& value) { return !value.has_value(); }) | Transform([](const range_value_type& value){ return value.error(); });
	}
	auto good_flow() const {
		return range_ | Filter([](const range_value_type& value) { return value.has_value(); }) | Transform([](const range_value_type& value){ return value.value(); });
	}


private:
	Range range_;
};

class SplitExpected : public Adaptors {
public: 
	SplitExpected() {}

	template<typename Range>
	auto operator()(Range range) const {
		SplitExpectedView<Range> view(range);
		return std::make_pair(view.unexpected_flow(), view.good_flow());
	}
};