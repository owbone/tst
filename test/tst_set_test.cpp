#include <gtest/gtest.h>
#include <gmock/gmock.h>

// Test dependencies.
#include <deque>
#include <vector>

// TST dependencies.
#include <iterator>
#include <memory>
#include <stack>
#include <string>

template <typename T>
class tst_set {
    struct node;

public:
    template <typename Pointer>
    class iterator_base : public std::iterator<std::bidirectional_iterator_tag,
                                               std::remove_pointer_t<Pointer>,
                                               std::ptrdiff_t,
                                               Pointer,
                                               std::remove_pointer_t<Pointer>&>
    {
    public:
        iterator_base() {
        }

        iterator_base(node* node) {
            if (node) {
                leftmost_path(node, left, std::back_inserter(path));
            }
        }

        bool operator==(const iterator_base& rhs) {
            return path == rhs.path;
        }

        bool operator!=(const iterator_base& rhs) {
            return !(*this == rhs);
        }

        iterator_base& operator++() {
            if (path.empty()) {
                return *this;
            }

            auto current = path.back();

            // First check if there are any children to move down to if we're
            // only at the end of a word and not necessarily at the bottom of
            // the tree.
            if (current.first->end) {
                if (node* next = current.first->left.get()) {
                    leftmost_path(next, left, std::back_inserter(path));
                    return *this;
                }
                if (node* next = current.first->child.get()) {
                    leftmost_path(next, down, std::back_inserter(path));
                    return *this;
                }
                if (node* next = current.first->right.get()) {
                    leftmost_path(next, right, std::back_inserter(path));
                    return *this;
                }
            }

            // We are at the bottom of the tree, so we need to move up a level.
            path.pop_back();

            if (path.empty()) {
                return *this;
            }

            auto previous = path.back().first;

            // Choose the next node depending on the direction of the current
            // node.
            switch (current.second) {
            case left:
                if (node* next = previous->child.get()) {
                    leftmost_path(next, down, std::back_inserter(path));
                    break;
                }
                // [[ fallthrough ]]

            case down:
                if (node* next = previous->right.get()) {
                    leftmost_path(next, right, std::back_inserter(path));
                    break;
                }
                // [[ fallthrough ]]

            case right:
                ++(*this);  // recurse
                break;
            }

            return *this;
        }

        iterator_base operator++(int) {
            iterator next = *this;
            ++(*this);
            return next;
        }

        typename iterator_base::value_type operator*() const {
            typename iterator_base::value_type value;
            if (!path.empty()) {
                element_type* previous_value = nullptr;
                for (const auto& step : path) {
                    // Traversing a node doesn't mean we want to add it to the
                    // output, since we might just end up moving left or right
                    // from it. Therefore we only want to add a node's value if
                    // the *next* node in the path is its equal child.
                    if (step.second == down && previous_value) {
                        value.push_back(*previous_value);
                    }
                    previous_value = &step.first->value;
                }
                // And always add the final node.
                value.push_back(*previous_value);
            }
            return value;
        }

    private:
        enum direction { left, right, down };

        std::vector<std::pair<node*, direction>> path;

        template <class OutputIt>
        void leftmost_path(node* node, direction dir, OutputIt it) {
            *it++ = std::make_pair(node, dir);
            
            if (node->end) {
                return;
            }

            if (node->left) {
                leftmost_path(node->left.get(), left, it);
            } else if (node->child) {
                leftmost_path(node->child.get(), down, it);
            } else if (node->right) {
                leftmost_path(node->right.get(), right, it);
            }
        }
    };

    using key_type = T;
    using element_type = typename T::value_type;
    using iterator = iterator_base<key_type*>;
    using const_iterator = iterator_base<const key_type*>;

    tst_set() = default;
    tst_set(const tst_set&) = default;
    tst_set& operator=(const tst_set&) = default;
    tst_set(tst_set&&) = default;
    tst_set& operator=(tst_set&&) = default;

    iterator begin() const {
        return iterator(root.get());
    }

    iterator end() const {
        return iterator();
    }

    template <class Container>
    void insert(const Container& c) {
        insert(std::begin(c), std::end(c));
    }

    // Special case for null-terminated strings.
    void insert(const element_type* key) {
        auto end = key;
        while (*end != element_type{}) { ++end; }
        insert(key, end);
    }

    template <class InputIt>
    void insert(InputIt begin, InputIt end) {
        root = insert(std::move(root), begin, end);
    }

    bool empty() const {
        return !root;
    }

private:
    struct node {
        element_type value;
        bool end;
        std::unique_ptr<node> left;  // lo kid
        std::unique_ptr<node> right;  // hi kid
        std::unique_ptr<node> child;  // equal kid
    };

    template <typename InputIt>
    static std::unique_ptr<node> insert(std::unique_ptr<node> root,
                                        InputIt begin,
                                        InputIt end) {
        if (begin == end) {
            return root;
        }

        const element_type& value = *begin;

        if (!root) {
            root = std::make_unique<node>();
            root->value = value;
        }

        if (root->value < value) {
            root->right = insert(std::move(root->right), begin, end);
        } else if (root->value > value) {
            root->left = insert(std::move(root->left), begin, end);
        } else {
            std::advance(begin, 1);
            root->child = insert(std::move(root->child), begin, end);
            if (begin == end) {
                root->end = true;
            }
        }

        return root;
    }

    std::unique_ptr<node> root;
};

// Check that a tst_set can be default constructed, and that it is empty.
TEST(tst_set_test, create_default_empty_set) {
    EXPECT_TRUE(tst_set<std::vector<int>>{}.empty());
    EXPECT_TRUE(tst_set<std::deque<void*>>{}.empty());
}

// Check that a single key can be inserted into a default constructed tst_set.
TEST(tst_set_test, insert_single_key) {
    tst_set<std::string> set;
    set.insert("one");
    ASSERT_FALSE(set.empty());
    EXPECT_EQ("one", *set.begin());
}

// Check that multiple keys in the form of any container type can be inserted
// into a tst_set, and iterated in order.
TEST(tst_set_test, iterate_multiple_keys) {
    tst_set<std::string> set;
    set.insert("one");
    set.insert(std::string("two"));
    set.insert(std::vector<char>{'t', 'h', 'r', 'e', 'e'});
    set.insert(std::deque<char>{'f', 'o', 'u', 'r'});
    ASSERT_FALSE(set.empty());

    // They should end up being sorted in lexicographical order.
    const std::vector<std::string> expected_elements = {
        "four", "one", "three", "two"
    };
    std::vector<std::string> elements(set.begin(), set.end());
    EXPECT_EQ(expected_elements, elements);
}

int main(int argc, char* argv[]) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
