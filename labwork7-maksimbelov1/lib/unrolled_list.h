#include <memory>
#include <iostream>

template<typename T, size_t NodeMaxSize = 10, typename Allocator = std::allocator<T>>
class unrolled_list {
private: 
    struct Node {
        alignas(T) char temp_data[NodeMaxSize * sizeof(T)];
        T* data;
        size_t size;
        Node* next;
        Node* prev;
        
        Node(): size(0), next(nullptr), prev(nullptr) {
            data = reinterpret_cast<T*>(temp_data);
        }
    };

public:
    using value_type = T;
    using reference = T&;
    using const_reference = const T&;
    using difference_type = ptrdiff_t;	
    using size_type = size_t;
    using allocator_type = Allocator;

    unrolled_list(const Allocator& alloc = Allocator())    
    : head(nullptr), tail(nullptr), total_size(0), number_of_nodes(0), allocator(alloc) {};
    unrolled_list(unrolled_list&& other, const Allocator& alloc)    
    : head(other.head), tail(other.tail), total_size(other.total_size), number_of_nodes(other.number_of_nodes), allocator(alloc) {
        if (this != &other) {
            other.head = nullptr;
            other.tail = nullptr;
            other.total_size = 0;
            other.number_of_nodes = 0;
        }
    };
    unrolled_list(const size_t n, const T& value, const Allocator& alloc = Allocator())    
    : head(nullptr), tail(nullptr), total_size(0), number_of_nodes(0), allocator(alloc) {
        try {
            for (size_t i = 0; i < n; ++i) {
                push_back(value);
            }
        } catch (...) {
            std::cerr << "Construction error\n";
            clear();
            throw;
        }
    };
    unrolled_list(const std::initializer_list<T> init, const Allocator& alloc = Allocator())    
    : head(nullptr), tail(nullptr), total_size(0), number_of_nodes(0), allocator(alloc) {
        try {
            for (const T& value : init) {
                push_back(value);
            }
        } catch (...) {
            std::cerr << "Construction error\n";
            clear();
            throw;
        }
    };
    template<typename Iter>
    unrolled_list(Iter first, Iter last, const Allocator& alloc = Allocator())    
    : head(nullptr), tail(nullptr), total_size(0), number_of_nodes(0), allocator(alloc) {
        try {
            while (first != last) {
                push_back(*first);
                ++first;
            }
        } catch (...) {
            std::cerr << "Construction error\n";
            clear();
            throw;
        }
    };

    unrolled_list& operator=(const std::initializer_list<T> init) {
        clear();
        try {
            for (const T& value : init) {
                push_back(value);
            }
        } catch (...) {
            std::cerr << "Construction error\n";
            clear();
            throw;
        }
        return *this;
    }
    
    ~unrolled_list() {
        clear();
    };

    size_t size() noexcept {
        return total_size;
    }
    size_t max_size() noexcept {
        return NodeMaxSize * number_of_nodes;
    }
    bool empty() noexcept {
        return (total_size == 0);
    }
    reference front() {
        if (empty()) {
            throw std::out_of_range("Unrolled list is empty");
        }
        return head->data[0];
    }
    const_reference front() const {
        if (empty()) {
            throw std::out_of_range("Unrolled list is empty");
        }
        return head->data[0];
    }
    reference back() {
        if (empty()) {
            throw std::out_of_range("Unrolled list is empty");
        }
        return tail->data[tail->size - 1];
    }
    const_reference back() const {
        if (empty()) {
            throw std::out_of_range("Unrolled list is empty");
        }
        return tail->data[tail->size - 1];
    }

    void push_back(const T& value) {
        if ((tail == nullptr) || (tail->size == NodeMaxSize)) {
            Node* new_node = create_node();
            std::allocator_traits<Allocator>::construct(allocator, new_node->data, value);
            new_node->size = 1;
            total_size++;
            number_of_nodes++;
            if (tail == nullptr) {
                head = new_node;
                tail = new_node;
            } else {
                tail->next = new_node;
                new_node->prev = tail;
                tail = new_node;                
            }
        } else {
            std::allocator_traits<Allocator>::construct(allocator, tail->data + tail->size, value);
            tail->size++;
            total_size++;
        }
    }

    void push_front(const T& value) {
        insert(begin(), value);
    }

    void pop_back() noexcept {
        if (tail == nullptr) {
            return;
        }
        if (tail->size > 1) {
            std::allocator_traits<Allocator>::destroy(allocator, tail->data + tail->size - 1);
            tail->size--;
        } else {
            Node* prev_node = tail->prev;
            destroy_node(tail);
            number_of_nodes--;
            tail = prev_node;
            if (prev_node != nullptr) {
                prev_node->next = nullptr;
            }
            if (tail == nullptr) {
                head = nullptr;
            }
        }
        total_size--;
    }

    void pop_front() noexcept {
        if (head == nullptr) {
            return;
        }
        if (head->size > 1) {
            std::allocator_traits<Allocator>::destroy(allocator, head->data + head->size - 1);
            head->size--;
        } else {
            Node* next_node = head->next;
            destroy_node(head);
            head = next_node;
            if (next_node != nullptr) {
                next_node->prev = nullptr;
            }
            if (head == nullptr) {
                tail = nullptr;
            }
        }
        total_size--;
    }

    Allocator get_allocator() noexcept {
        return allocator;        
    }

    void clear() noexcept {
        Node* current_node = head;
        Node* next_node = head;
        while (current_node) {
            next_node = current_node->next;
            destroy_node(current_node);
            current_node = next_node;
        }
        head = nullptr;
        tail = nullptr;
        total_size = 0;
        number_of_nodes = 0;
    }

    class iterator {
    public: 
        using value_type = T;
        using pointer = T*;
        using reference = T&;
        using difference_type = ptrdiff_t;	
        using iterator_category = std::bidirectional_iterator_tag;

        iterator(Node* node = nullptr, size_t index = 0): current_node(node), current_index(index) {}

        T& operator*() {
            return current_node->data[current_index];
        }
        T* operator->() {
            return &current_node->data[current_index];
        } 

        iterator& operator++() {
            if (current_node == nullptr) {
                return *this;
            }
            if (current_node->size > current_index + 1) {
                ++current_index;
            } else {
                current_node = current_node->next;
                current_index = 0;
            }
            return *this;
        }
        iterator operator++(int) {
            iterator current_iterator = *this;
            ++(*this);
            return current_iterator;
        }

        iterator& operator--() {
            if (current_node == nullptr) {
                return *this;
            }
            if (current_index > 0) {
                --current_index;
            } else {
                current_node = current_node->prev;
                if (current_node != nullptr) {
                    current_index = current_node->size - 1;
                } else {
                    current_index = 0;
                }
            }
            return *this;
        }
        iterator operator--(int) {
            iterator current_iterator = *this;
            --(*this);
            return current_iterator;
        }

        bool operator==(const iterator& other) {
            return ((current_node == other.current_node) && (current_index == other.current_index));
        }
        bool operator!=(const iterator& other) {
            return ((current_node != other.current_node) || (current_index != other.current_index));
        }

    private:
        Node* current_node;
        size_t current_index;
        friend unrolled_list;
    };

    class const_iterator {
    public: 
        using value_type = T;
        using pointer = T*;
        using reference = T&;
        using difference_type = ptrdiff_t;	
        using iterator_category = std::bidirectional_iterator_tag;

        const_iterator(Node* node = nullptr, size_t index = 0): current_node(node), current_index(index) {}

        const T& operator*() {
            return current_node->data[current_index];
        }
        const T* operator->() {
            return &current_node->data[current_index];
        } 

        const_iterator& operator++() {
            if (current_node == nullptr) {
                return *this;
            }
            if (current_node->size > current_index + 1) {
                ++current_index;
            } else {
                current_node = current_node->next;
                current_index = 0;
            }
            return *this;
        }
        const_iterator operator++(int) {
            const_iterator current_iterator = *this;
            ++(*this);
            return current_iterator;
        }

        const_iterator& operator--() {
            if (current_node == nullptr) {
                return *this;
            }
            if (current_index > 0) {
                --current_index;
            } else {
                current_node = current_node->prev;
                if (current_node != nullptr) {
                    current_index = current_node->size - 1;
                } else {
                    current_index = 0;
                }
            }
            return *this;
        }
        const_iterator operator--(int) {
            const_iterator current_iterator = *this;
            --(*this);
            return current_iterator;
        }

        bool operator==(const const_iterator& other) {
            return ((current_node == other.current_node) && (current_index == other.current_index));
        }
        bool operator!=(const const_iterator& other) {
            return ((current_node != other.current_node) || (current_index != other.current_index));
        }
    private:
        iterator to_non_const() {
            return iterator(current_node, current_index);
        }
        Node* current_node;
        size_t current_index;
        friend unrolled_list;
    };

    iterator begin() noexcept {
        return iterator(head, 0);
    }
    iterator end() noexcept {
        return iterator(nullptr, 0); 
    }
    const_iterator begin() const noexcept {
        return const_iterator(head, 0);
    }
    const_iterator end() const noexcept {
        return const_iterator(nullptr, 0);
    }
    const_iterator cbegin() const noexcept {
        return const_iterator(head, 0);
    }
    const_iterator cend() const noexcept {
        return const_iterator(nullptr, 0);
    }
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    reverse_iterator rbegin() noexcept {
        if (tail == nullptr) {
            return reverse_iterator(end());
        }
        return reverse_iterator(iterator(tail, tail->size));
    }
    reverse_iterator rend() noexcept {
        return reverse_iterator(begin());
    }
    const_reverse_iterator rbegin() const noexcept {
        if (tail == nullptr) {
            return const_reverse_iterator(end());
        }
        return const_reverse_iterator(iterator(tail, tail->size));
    }
    const_reverse_iterator rend() const noexcept {
        return const_reverse_iterator(begin());
    }
    const_reverse_iterator crbegin() const noexcept {
        if (tail == nullptr) {
            return const_reverse_iterator(end());
        }
        return const_reverse_iterator(iterator(tail, tail->size));
    }
    const_reverse_iterator crend() const noexcept {
        return const_reverse_iterator(begin());
    }

    bool operator==(const unrolled_list& other) noexcept {
        if (total_size != other.size()) {
            return false;
        }
        iterator it_1 = begin();
        iterator it_2 = other.begin();
        while (it_1 != end()) {
            if (*it_1 != *it_2) {
                return false;
            }
            ++it_1;
            ++it_2;
        }
        return true;
    }
    bool operator!=(const unrolled_list& other) noexcept {
        return !(*this == other);
    }

    iterator insert(iterator pos, const T& value) {
        if(pos == end()) {
            push_back(value);
            return iterator(tail, tail->size - 1);
        }
        Node* node = pos.current_node;
        size_t index = pos.current_index;
        Node* new_node = create_node();
        if (node->size < NodeMaxSize) {
            for (size_t i = 0; i < index; ++i) {
                std::allocator_traits<Allocator>::construct(allocator, new_node->data + i, node->data[i]);
            }
            std::allocator_traits<Allocator>::construct(allocator, new_node->data + index, value);
            for (size_t i = index; i < node->size; ++i) {
                std::allocator_traits<Allocator>::construct(allocator, new_node->data + i + 1, node->data[i]);
            }
            new_node->size = node->size + 1;
            new_node->prev = node->prev;
            if (node->prev != nullptr) {
                node->prev->next = new_node;
            } else {
                head = new_node;
            }
            new_node->next = node->next;
            if (node->next != nullptr) {
                node->next->prev = new_node;
            } else {
                tail = new_node;
            }
            destroy_node(node);
            total_size++;
            return iterator(new_node, index);
        }
        Node* new_node_2 = create_node();
        for (size_t i = 0; i < (NodeMaxSize / 2); ++i) {
            std::allocator_traits<Allocator>::construct(allocator, new_node->data + i, node->data[i]);
            new_node->size++;
        }
        for (size_t i = (NodeMaxSize / 2); i < NodeMaxSize; ++i) {
            std::allocator_traits<Allocator>::construct(allocator, new_node_2->data + i - (NodeMaxSize / 2), node->data[i]);
            new_node_2->size++;
        }
      
        if (index < (NodeMaxSize / 2)) {
            for (size_t i = new_node->size; i > index; --i) {
                std::allocator_traits<Allocator>::construct(allocator, new_node->data + i, new_node->data[i - 1]);
                std::allocator_traits<Allocator>::destroy(allocator, new_node->data + (i - 1));
            }
            std::allocator_traits<Allocator>::construct(allocator, new_node->data + index, value);
            new_node->size++;
        } else {
            for (size_t i = new_node_2->size; i > (index - (NodeMaxSize / 2)); --i) {
                std::allocator_traits<Allocator>::construct(allocator, new_node_2->data + i, new_node_2->data[i - 1]);
                std::allocator_traits<Allocator>::destroy(allocator, new_node_2->data + (i - 1));
            }
            std::allocator_traits<Allocator>::construct(allocator, new_node_2->data + (index - (NodeMaxSize / 2)), value);
            new_node_2->size++;
        }
        new_node->prev = node->prev;
        if (node->prev != nullptr) {
            node->prev->next = new_node;
        } else {
            head = new_node;
        }
        new_node->next = new_node_2;
        new_node_2->prev = new_node;
        new_node_2->next = node->next;
        if (node->next != nullptr) {
            node->next->prev = new_node_2;
        } else {
            tail = new_node_2;
        }
        destroy_node(node);
        total_size++;
        number_of_nodes++;
        if (index < (NodeMaxSize / 2)) {
            return iterator(new_node, index);
        }
        return iterator(new_node_2, index - (NodeMaxSize / 2));
    }
    iterator insert(const_iterator pos, const T& value) {
        return insert(pos.to_non_const(), value);
    }

    iterator insert(iterator pos, size_t n, const T& value) {
        if (n == 0) {
            return pos;
        }
        Node* node = pos.current_node;
        size_t index = pos.current_index;
        if (pos == nullptr) {
            try {
                for (size_t i = 0; i < n; ++i) {
                    push_back(value);
                }
            } catch (...) {
                clear();
                throw;
            }
            return iterator(tail, tail->size - 1);
        }
        unrolled_list<T, NodeMaxSize, Allocator> new_list(allocator);
        for (size_t i = 0; i < node->size; ++i) {
            new_list.push_back(node->data[i]);
        }
        iterator new_pos = new_list.begin();
        for (size_t i = 0; i < index; ++i) {
            ++new_pos;
        }
        for (size_t i = 0; i < n; ++i) {
            new_pos = new_list.insert(new_pos, value);
        }
        
        if (node->prev != nullptr) {
            new_list.head->prev = node->prev;
            node->prev->next = new_list.head;
        } else {
            head = new_list.head;
        }
        if (node->next != nullptr) {
            new_list.tail->next = node->next;
            node->next->prev = new_list.tail;
        } else {
            tail = new_list.tail;
        }
        total_size += n;
        number_of_nodes += new_list.number_of_nodes - 1;
        new_list.total_size = 0;
        new_list.number_of_nodes = 0;
        new_list.head = nullptr;
        new_list.tail = nullptr;
        return new_pos;
    }
    iterator insert(const_iterator pos, size_t n, const T& value) {
        return insert(pos.to_non_const(), n, value);
    }

    iterator erase(iterator pos) noexcept {
        if (pos == end()) {
            return pos;
        }
        Node* node = pos.current_node;
        size_t index = pos.current_index;
        try {
            std::allocator_traits<Allocator>::destroy(allocator, node->data + index);
            for (size_t i = index + 1; i < node->size; ++i) {
                std::allocator_traits<Allocator>::construct(allocator, node->data + i - 1, node->data[i]);
                std::allocator_traits<Allocator>::destroy(allocator, node->data + i);
            }
            node->size--;
            total_size--;
        } catch(...) {
            std::cerr << "Construction error\n";
        }
        Node* next_node = node->next;
        if (node->size == 0) {
            if (node->prev != nullptr) {
                node->prev->next = next_node;
            } else {
                head = next_node;
            }
            if (next_node != nullptr) {
                next_node->prev = node->prev;
            } else {
                tail = node->prev;
            }
            destroy_node(node);
            number_of_nodes--;
            if (next_node != nullptr) {
                return iterator(next_node, 0);
            } else {
                return end();
            }
        }
        return iterator(node, index);
    }
    iterator erase(const_iterator pos) noexcept {
        return erase(pos.to_non_const());
    }

    iterator erase(iterator pos, iterator pos2) noexcept {
        while (pos != pos2) {
            pos = erase(pos);
        }
        return pos2;
    }
    iterator erase(const_iterator pos, const_iterator pos2) noexcept {
        return erase(pos.to_non_const(), pos2.to_non_const());
    }

private: 
    Node* create_node() {
        Node* new_node = nullptr;
        new_node = node_allocator.allocate(1);
        std::allocator_traits<NodeAllocator>::construct(node_allocator, new_node);
        return new_node;
    }

    void destroy_node(Node* node) noexcept {
        if (node != nullptr) {
            try {
                for (int i = 0; i < node->size; ++i) {
                    std::allocator_traits<Allocator>::destroy(allocator, node->data + i);
                }
                std::allocator_traits<NodeAllocator>::destroy(node_allocator, node);
                std::allocator_traits<NodeAllocator>::deallocate(node_allocator, node, 1);
            } catch (...) {
                std::cerr << "Error deleting object\n";
            }
        }
    }

    Allocator allocator;
    using NodeAllocator = typename std::allocator_traits<Allocator>::template rebind_alloc<Node>;
    NodeAllocator node_allocator;

    Node* head;
    Node* tail;
    size_t total_size;
    size_t number_of_nodes;
};