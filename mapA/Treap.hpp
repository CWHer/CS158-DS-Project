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
            const NodeType node_type;
            int tree_size, priority;
            Node *child[2], *father;
            value_type *storage;

            Node(NodeType type, int priority, Node *fa = nullptr)
                : node_type(type), priority(priority), father(fa)
            {
                tree_size = 1;
                child[0] = child[1] = nullptr;
                storage = nullptr;
            }
            Node(NodeType type, int priority, Node *fa, const value_type &value)
                : node_type(type), priority(priority), father(fa)
            {
                tree_size = 1;
                child[0] = child[1] = nullptr;
                storage = new value_type(value);
            }
            Node(const Node &other)
                : node_type(other.node_type),
                  tree_size(other.tree_size),
                  priority(other.priority)
            {
                child[0] = child[1] = nullptr;
                father = nullptr;
                storage = other.storage != nullptr
                              ? new value_type(*(other.storage))
                              : nullptr;
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

        void dfsDestruct(Node *o)
        {
            for (int i = 0; i < 2; ++i)
                if (o->child[i] != nullptr)
                    dfsDestruct(o->child[i]);
            delete o;
        }

        void dfsConstruct(Node *&o, Node *fa, const Node *other)
        {
            o = new Node(*other);
            o->father = fa;
            switch (o->node_type)
            {
            case NodeType::Head:
                head = o;
                break;
            case NodeType::Tail:
                tail = o;
                break;
            }

            for (int i = 0; i < 2; ++i)
                if (other->child[i] != nullptr)
                    dfsConstruct(o->child[i], o, other->child[i]);
        }

        void rotate(Node *&y, int k)
        {
            // rotate x ---> y
            Node *x = y->child[k];
            x->father = y->father;
            y->child[k] = x->child[k ^ 1];
            if (x->child[k ^ 1] != nullptr)
                x->child[k ^ 1]->father = y;
            x->child[k ^ 1] = y;
            y->father = x;
            y->pushUp(), x->pushUp();
            // NOTE: change y
            y = x;
        }

        Node *find(Node *root, const Key &key) const
        {
            Node *o = root;
            while (!equal(o->storage, key))
            {
                NodeType type = o->node_type;
                int k = type;
                if (type == NodeType::Data)
                    k = compare_func(o->storage->first, key);
                if (o->child[k] == nullptr)
                    break; // fail
                o = o->child[k];
            }
            return o;
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

        pair<Node *, bool> insert(Node *&o, Node *fa, const value_type &value)
        {
            if (o == nullptr)
            {
                o = new Node(NodeType::Data, randInt(), fa, value);
                return pair<Node *, bool>(o, true);
            }
            if (equal(o->storage, value.first))
                return pair<Node *, bool>(o, false);

            NodeType type = o->node_type;
            int k = type;
            if (type == NodeType::Data)
                k = compare_func(o->storage->first, value.first);

            pair<Node *, bool> ret = insert(o->child[k], o, value);
            if (o->child[k]->priority < o->priority)
                rotate(o, k);
            o->pushUp();
            return ret;
        }

        bool remove(Node *&o, Node *fa, const value_type &value)
        {
            if (o == nullptr)
                return false;

            if (equal(o->storage, value.first))
            {
                if (o->child[0] == nullptr || o->child[1] == nullptr)
                {
                    Node *x = o;
                    o = o->child[o->child[0] == nullptr];
                    if (o != nullptr)
                        o->father = fa;
                    delete x;
                    return true;
                }

                int k = o->child[0]->priority < o->child[1]->priority;
                rotate(o, k ^ 1);
                remove(o->child[k], o, value);
                o->pushUp();
                return true;
            }

            NodeType type = o->node_type;
            int k = type;
            if (type == NodeType::Data)
                k = compare_func(o->storage->first, value.first);
            bool ret = remove(o->child[k], o, value);
            o->pushUp();
            return ret;
        }

        int randInt()
        {
            static long long current = 1;
            current = (current * 13131 + 20000905) % (int)(1e9 + 7);
            return current;
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
            head = root = new Node(NodeType::Head, randInt());
            tail = root->child[1] = new Node(NodeType::Tail, randInt(), root);
            root->pushUp();
            if (root->child[1]->priority < root->priority)
                rotate(root, 1);
        }
        map(const map &other)
        {
            dfsConstruct(root, nullptr, other.root);
        }
        /**
         * TODO assignment operator
         */
        map &operator=(const map &other)
        {
            if (this != &other)
            {
                dfsDestruct(root);
                dfsConstruct(root, nullptr, other.root);
            }
            return *this;
        }
        /**
         * TODO Destructors
         */
        ~map()
        {
            dfsDestruct(root);
        }
        /**
         * TODO
         * access specified element with bounds checking
         * Returns a reference to the mapped value of the element with key equivalent to key.
         * If no such element exists, an exception of type `index_out_of_bound'
         */
        T &at(const Key &key)
        {
            Node *x = find(root, key);
            if (!equal(x->storage, key))
                throw index_out_of_bound();
            return x->storage->second;
        }
        const T &at(const Key &key) const
        {
            Node *x = find(root, key);
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
            Node *x = find(root, key);
            if (!equal(x->storage, key))
            {
                Node *node = insert(root, nullptr, value_type(key, T())).first;
                return node->storage->second;
            }
            return x->storage->second;
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
            // ignore head & tail
            return root->tree_size == 2;
        }
        /**
         * returns the number of elements.
         */
        size_t size() const
        {
            // ignore head & tail
            return root->tree_size - 2;
        }
        /**
         * clears the contents
         */
        void clear()
        {
            dfsDestruct(root);
            head = root = new Node(NodeType::Head, randInt());
            tail = root->child[1] = new Node(NodeType::Tail, randInt(), root);
            root->pushUp();
            if (root->child[1]->priority < root->priority)
                rotate(root, 1);
        }
        /**
         * insert an element.
         * return a pair, the first of the pair is
         *   the iterator to the new element (or the element that prevented the insertion),
         *   the second one is true if insert successfully, or false.
         */
        pair<iterator, bool> insert(const value_type &value)
        {
            pair<Node *, bool> result = insert(root, nullptr, value);
            return pair<iterator, bool>(iterator(this, result.first), result.second);
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
            if (!remove(root, nullptr, *(pos.node_ptr->storage)))
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
            Node *x = find(root, key);
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
            Node *x = find(root, key);
            if (!equal(x->storage, key))
                return end();
            return iterator(this, x);
        }
        const_iterator find(const Key &key) const
        {
            Node *x = find(root, key);
            if (!equal(x->storage, key))
                return cend();
            return const_iterator(this, x);
        }
    };

}

#endif
