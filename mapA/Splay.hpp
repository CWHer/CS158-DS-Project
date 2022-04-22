/**
 * implement a container like std::map
 */
#ifndef SJTU_MAP_HPP
#define SJTU_MAP_HPP

// only for std::less<T>
#include <functional>
#include <cstddef>
#include "utility.hpp"
#include "exceptions.hpp"

namespace sjtu
{
    template <class Key, class T, class Compare = std::less<Key>>
    class map
    {
    public:
        typedef pair<const Key, T> value_type;

    private:
        /**
         * the internal type of data.
         * it should have a default constructor, a copy constructor.
         * You can use sjtu::map as value_type by typedef.
         */
        Compare compare_func;

        enum NodeType
        {
            Head = 1, // virtual node (only have child[1])
            Tail = 0, // virtual node (only have child[0])
            Data = -1
        };

        struct Node
        {
            NodeType node_type;
            int tree_size;
            Node *child[2], *father;
            value_type *storage;

            Node(NodeType type, Node *fa = nullptr)
                : node_type(type), father(fa)
            {
                tree_size = 1;
                child[0] = child[1] = nullptr;
                storage = nullptr;
            }

            Node(NodeType type, Node *fa, const value_type &value)
                : node_type(type), father(fa)
            {
                tree_size = 1;
                child[0] = child[1] = nullptr;
                storage = new value_type(value);
            }

            ~Node()
            {
                if (storage != nullptr)
                    delete storage;
            }

            void pushUp()
            {
                tree_size = 1;
                for (int i = 0; i < 2; ++i)
                    if (child[i] != nullptr)
                        tree_size += child[i]->tree_size;
            }
        };
        Node *root;
        Node *head, *tail;

        // >>>>>> utilities
        bool equal(value_type *&storage, const Key &key) const
        {
            return storage == nullptr ? false
                                      : !compare_func(storage->first, key) &&
                                            !compare_func(key, storage->first);
        }

        void destruct()
        {
            auto it = begin();
            while (it != end())
            {
                auto x = it++;
                erase(x);
            }
        }

        void construct(Node *&o, Node *fa,
                       value_type *arr[], int l, int r)
        {
            int mid = (l + r) >> 1;
            o = new Node(NodeType::Data, fa, (*arr[mid]));
            if (l < mid)
                construct(o->child[0], o, arr, l, mid - 1);
            if (mid < r)
                construct(o->child[1], o, arr, mid + 1, r);
            o->pushUp();
        }

        void construct(const map &other)
        {
            int index = 0;
            value_type **arr;
            arr = new value_type *[other.size()];
            for (Node *x = other.succ(other.head);
                 x != other.tail; x = other.succ(x))
                arr[index++] = x->storage;

            // the structure of the backbone tree is a little weird...
            Node *start_node = root == tail ? head : tail;
            construct(
                start_node->child[start_node->node_type],
                start_node, arr, 0, index - 1);
            start_node->pushUp();
            root->pushUp();

            delete[] arr;
        }

        void rotate(Node *x)
        {
            // rotate x ---> y
            Node *y = x->father;
            Node *z = y->father;
            int k = y->child[1] == x;
            if (z != nullptr)
                z->child[z->child[1] == y] = x;
            x->father = z;
            y->child[k] = x->child[k ^ 1];
            if (x->child[k ^ 1] != nullptr)
                x->child[k ^ 1]->father = y;
            x->child[k ^ 1] = y;
            y->father = x;
            y->pushUp(), x->pushUp();
        }

        void splay(Node *x, Node *target)
        {
            // splay until x->fa == target
            while (x->father != target)
            {
                Node *y = x->father;
                Node *z = y->father;
                if (z != target)
                {
                    // zig-zig: z->child[0] == y && y->child[0] == x
                    // zig-zag: z->child[1] == y && y->child[0] == x
                    int rotate_type = (y->child[0] == x) ^
                                      (z->child[0] == y);
                    rotate(rotate_type ? x : y);
                }
                rotate(x);
            }
            if (target == nullptr)
                root = x;
        }

        Node *search(const Key &key) const
        {
            Node *x = root;
            while (!equal(x->storage, key))
            {
                NodeType type = x->node_type;
                int k = type;
                if (type == NodeType::Data)
                    k = compare_func(x->storage->first, key);
                if (x->child[k] == nullptr)
                    break; // fail
                x = x->child[k];
            }
            return x;
        }

        bool search(Node *x, const Key &key, Node *target = nullptr)
        {
            bool result = true;
            while (!equal(x->storage, key))
            {
                NodeType type = x->node_type;
                int k = type;
                if (type == NodeType::Data)
                    k = compare_func(x->storage->first, key);
                if (x->child[k] == nullptr)
                {
                    result = false;
                    break; // fail
                }
                x = x->child[k];
            }
            // NOTE: x is the closest node to the key
            //  (either less or greater)
            // splay to target
            splay(x, target);
            return result;
        }

        Node *prev(Node *o) const
        {
            if (o->child[0] != nullptr)
            {
                o = o->child[0];
                while (o->child[1] != nullptr)
                    o = o->child[1];
                return o;
            }
            while (o->father->child[1] != o)
                o = o->father;
            return o->father;
        }

        Node *succ(Node *o) const
        {
            if (o->child[1] != nullptr)
            {
                o = o->child[1];
                while (o->child[0] != nullptr)
                    o = o->child[0];
                return o;
            }
            while (o->father->child[0] != o)
                o = o->father;
            return o->father;
        }

        bool insert(const value_type &value, int *dummy)
        {
            if (search(root, value.first))
                return false;

            NodeType type = root->node_type;
            int k = type ^ 1;
            if (type == NodeType::Data)
                k = compare_func(value.first, root->storage->first);

            // insert as root
            Node *x = new Node(NodeType::Data, nullptr, value);
            root->father = x;
            x->child[k] = root;
            x->child[k ^ 1] = root->child[k ^ 1];
            root->child[k ^ 1]->father = x;
            root->child[k ^ 1] = nullptr;
            root->pushUp();
            root = x, x->pushUp();
            return true;
        }

        bool remove(const value_type &value)
        {
            if (!search(root, value.first))
                return false;

            // splay succ(root) to root->child[1]
            search(root->child[1], value.first, root);

            // remove root
            Node *x = root;
            root = x->child[1];
            root->father = nullptr;
            root->child[0] = x->child[0];
            x->child[0]->father = root;
            root->pushUp();
            delete x;
            return true;
        }

    public:
        void display() const
        {
            printf("[INFO]: ");
            for (auto it = cbegin(); it != cend(); ++it)
                printf("%d ", it->first);
            printf("\n");
        }

    public:
        /**
         * see BidirectionalIterator at CppReference for help.
         *
         * if there is anything wrong throw invalid_iterator.
         *     like it = map.begin(); --it;
         *       or it = map.end(); ++end();
         */
        class const_iterator;
        class iterator
        {
            friend class map<Key, T, Compare>;

        private:
            /**
             * TODO add data members
             *   just add whatever you want.
             */
            map<Key, T, Compare> *map_ptr;
            Node *node_ptr;

        public:
            iterator() : map_ptr(nullptr), node_ptr(nullptr) {}
            iterator(map<Key, T, Compare> *map_ptr, Node *node_ptr)
                : map_ptr(map_ptr), node_ptr(node_ptr) {}
            iterator(const iterator &other)
            {
                map_ptr = other.map_ptr;
                node_ptr = other.node_ptr;
            }
            /**
             * return a new iterator which pointer n-next elements
             *   even if there are not enough elements, just return the answer.
             * as well as operator-
             */
            /**
             * TODO iter++
             */
            iterator operator++(int)
            {
                if (node_ptr->node_type == NodeType::Tail)
                    throw invalid_iterator();
                iterator ret(*this);
                node_ptr = map_ptr->succ(node_ptr);
                return ret;
            }
            /**
             * TODO ++iter
             */
            iterator &operator++()
            {
                if (node_ptr->node_type == NodeType::Tail)
                    throw invalid_iterator();
                node_ptr = map_ptr->succ(node_ptr);
                return *this;
            }
            /**
             * TODO iter--
             */
            iterator operator--(int)
            {
                if (node_ptr->node_type == NodeType::Head)
                    throw invalid_iterator();
                iterator ret(*this);
                Node *prev_node = map_ptr->prev(node_ptr);
                if (prev_node->node_type == NodeType::Head)
                    throw invalid_iterator();
                node_ptr = prev_node;
                return ret;
            }
            /**
             * TODO --iter
             */
            iterator &operator--()
            {
                if (node_ptr->node_type == NodeType::Head)
                    throw invalid_iterator();
                Node *prev_node = map_ptr->prev(node_ptr);
                if (prev_node->node_type == NodeType::Head)
                    throw invalid_iterator();
                node_ptr = prev_node;
                return *this;
            }
            /**
             * a operator to check whether two iterators are same (pointing to the same memory).
             */
            value_type &operator*() const
            {
                return *(node_ptr->storage);
            }
            bool operator==(const iterator &rhs) const
            {
                return map_ptr == rhs.map_ptr && node_ptr == rhs.node_ptr;
            }
            bool operator==(const const_iterator &rhs) const
            {
                return map_ptr == rhs.map_ptr && node_ptr == rhs.node_ptr;
            }
            /**
             * some other operator for iterator.
             */
            bool operator!=(const iterator &rhs) const
            {
                return map_ptr != rhs.map_ptr || node_ptr != rhs.node_ptr;
            }
            bool operator!=(const const_iterator &rhs) const
            {
                return map_ptr != rhs.map_ptr || node_ptr != rhs.node_ptr;
            }

            /**
             * for the support of it->first.
             * See <http://kelvinh.github.io/blog/2013/11/20/overloading-of-member-access-operator-dash-greater-than-symbol-in-cpp/> for help.
             */
            value_type *operator->() const noexcept
            {
                return node_ptr->storage;
            }
        };
        class const_iterator
        {
            friend class map<Key, T, Compare>;
            // it should has similar member method as iterator.
            //  and it should be able to construct from an iterator.
        private:
            const map<Key, T, Compare> *map_ptr;
            Node *node_ptr;
            // data members.
        public:
            const_iterator() : map_ptr(nullptr), node_ptr(nullptr) {}
            const_iterator(const map<Key, T, Compare> *map_ptr, Node *node_ptr)
                : map_ptr(map_ptr), node_ptr(node_ptr) {}
            const_iterator(const const_iterator &other)
            {
                map_ptr = other.map_ptr;
                node_ptr = other.node_ptr;
            }
            const_iterator(const iterator &other)
            {
                map_ptr = other.map_ptr;
                node_ptr = other.node_ptr;
            }
            // And other methods in iterator.
            // And other methods in iterator.
            // And other methods in iterator.
            /**
             * return a new iterator which pointer n-next elements
             *   even if there are not enough elements, just return the answer.
             * as well as operator-
             */
            /**
             * TODO iter++
             */
            const_iterator operator++(int)
            {
                if (node_ptr->node_type == NodeType::Tail)
                    throw invalid_iterator();
                const_iterator ret(*this);
                node_ptr = map_ptr->succ(node_ptr);
                return ret;
            }
            /**
             * TODO ++iter
             */
            const_iterator &operator++()
            {
                if (node_ptr->node_type == NodeType::Tail)
                    throw invalid_iterator();
                node_ptr = map_ptr->succ(node_ptr);
                return *this;
            }
            /**
             * TODO iter--
             */
            const_iterator operator--(int)
            {
                if (node_ptr->node_type == NodeType::Head)
                    throw invalid_iterator();
                const_iterator ret(*this);
                Node *prev_node = map_ptr->prev(node_ptr);
                if (prev_node->node_type == NodeType::Head)
                    throw invalid_iterator();
                node_ptr = prev_node;
                return ret;
            }
            /**
             * TODO --iter
             */
            const_iterator &operator--()
            {
                if (node_ptr->node_type == NodeType::Head)
                    throw invalid_iterator();
                Node *prev_node = map_ptr->prev(node_ptr);
                if (prev_node->node_type == NodeType::Head)
                    throw invalid_iterator();
                node_ptr = prev_node;
                return *this;
            }
            /**
             * a operator to check whether two iterators are same (pointing to the same memory).
             */
            const value_type &operator*() const
            {
                return *(node_ptr->storage);
            }
            bool operator==(const iterator &rhs) const
            {
                return map_ptr == rhs.map_ptr && node_ptr == rhs.node_ptr;
            }
            bool operator==(const const_iterator &rhs) const
            {
                return map_ptr == rhs.map_ptr && node_ptr == rhs.node_ptr;
            }
            /**
             * some other operator for iterator.
             */
            bool operator!=(const iterator &rhs) const
            {
                return map_ptr != rhs.map_ptr || node_ptr != rhs.node_ptr;
            }
            bool operator!=(const const_iterator &rhs) const
            {
                return map_ptr != rhs.map_ptr || node_ptr != rhs.node_ptr;
            }

            /**
             * for the support of it->first.
             * See <http://kelvinh.github.io/blog/2013/11/20/overloading-of-member-access-operator-dash-greater-than-symbol-in-cpp/> for help.
             */
            const value_type *operator->() const noexcept
            {
                return node_ptr->storage;
            }
        };
        /**
         * TODO two constructors
         */
        map()
        {
            head = root = new Node(NodeType::Head);
            tail = root->child[1] = new Node(NodeType::Tail, root);
            root->pushUp();
        }
        map(const map &other)
        {
            head = root = new Node(NodeType::Head);
            tail = root->child[1] = new Node(NodeType::Tail, root);
            root->pushUp();
            if (!other.empty())
                construct(other);
        }
        /**
         * TODO assignment operator
         */
        map &operator=(const map &other)
        {
            if (this != &other)
            {
                destruct();
                if (!other.empty())
                    construct(other);
            }
            return *this;
        }
        /**
         * TODO Destructors
         */
        ~map()
        {
            destruct();
            delete head;
            delete tail;
        }
        /**
         * TODO
         * access specified element with bounds checking
         * Returns a reference to the mapped value of the element with key equivalent to key.
         * If no such element exists, an exception of type `index_out_of_bound'
         */
        T &at(const Key &key)
        {
            if (!search(root, key))
                throw index_out_of_bound();
            return root->storage->second;
        }
        const T &at(const Key &key) const
        {

            Node *x = search(key);
            if (!equal(x->storage, key))
                throw index_out_of_bound();
            return x->storage->second;
        }
        /**
         * TODO
         * access specified element
         * Returns a reference to the value that is mapped to a key equivalent to key,
         *   performing an insertion if such key does not already exist.
         */
        T &operator[](const Key &key)
        {
            if (!search(root, key))
                insert(value_type(key, T()));
            return root->storage->second;
        }
        /**
         * behave like at() throw index_out_of_bound if such key does not exist.
         */
        const T &operator[](const Key &key) const
        {
            return at(key);
        }
        /**
         * return a iterator to the beginning
         */
        iterator begin()
        {
            return iterator(this, succ(head));
        }
        const_iterator cbegin() const
        {
            return const_iterator(this, succ(head));
        }
        /**
         * return a iterator to the end
         * in fact, it returns past-the-end.
         */
        iterator end()
        {
            return iterator(this, tail);
        }
        const_iterator cend() const
        {
            return const_iterator(this, tail);
        }
        /**
         * checks whether the container is empty
         * return true if empty, otherwise false.
         */
        bool empty() const
        {
            return root->tree_size == 2;
        }
        /**
         * returns the number of elements.
         */
        size_t size() const
        {
            return root->tree_size - 2;
        }
        /**
         * clears the contents
         */
        void clear()
        {
            destruct();
        }
        /**
         * insert an element.
         * return a pair, the first of the pair is
         *   the iterator to the new element (or the element that prevented the insertion),
         *   the second one is true if insert successfully, or false.
         */
        pair<iterator, bool> insert(const value_type &value)
        {
            bool result = insert(value, nullptr);
            return pair<iterator, bool>(iterator(this, root), result);
        }
        /**
         * erase the element at pos.
         *
         * throw if pos pointed to a bad element (pos == this->end() || pos points an element out of this)
         */
        void erase(iterator pos)
        {
            if (pos.map_ptr != this)
                throw invalid_iterator();
            if (pos.node_ptr->node_type != NodeType::Data)
                throw invalid_iterator();
            if (!remove(*pos))
                throw invalid_iterator();
        }
        /**
         * Returns the number of elements with key
         *   that compares equivalent to the specified argument,
         *   which is either 1 or 0
         *     since this container does not allow duplicates.
         * The default method of check the equivalence is !(a < b || b > a)
         */
        size_t count(const Key &key) const
        {
            Node *x = search(key);
            return equal(x->storage, key);
        }
        /**
         * Finds an element with key equivalent to key.
         * key value of the element to search for.
         * Iterator to an element with key equivalent to key.
         *   If no such element is found, past-the-end (see end()) iterator is returned.
         */
        iterator find(const Key &key)
        {
            if (!search(root, key))
                return end();
            return iterator(this, root);
        }
        const_iterator find(const Key &key) const
        {
            Node *x = search(key);
            if (!equal(x->storage, key))
                return cend();
            return const_iterator(this, x);
        }
    };

}

#endif
