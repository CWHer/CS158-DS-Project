#ifndef SJTU_DEQUE_HPP
#define SJTU_DEQUE_HPP

#include "exceptions.hpp"
#include "utility.hpp"
#include <cstddef>
#include <cmath>
#include <cstdio>

namespace sjtu
{
    // NOTE: list + list
    //  deque = list<Block>
    //  Block = list<Node>
    template <class T>
    class deque
    {
    private:
        int total_size, blk_size;
        const int min_blk = 500;

        enum NodeType
        {
            Virtual, // guard node
            Data
        };

        struct Node
        {
            const NodeType node_type;
            Node *prev, *succ;
            T *storage;

            Node() : node_type(NodeType::Virtual),
                     prev(nullptr), succ(nullptr),
                     storage(nullptr) {}

            Node(const T &data)
                : node_type(NodeType::Data),
                  prev(nullptr), succ(nullptr)
            {
                storage = new T(data);
            }

            Node(const T &data, Node *prev, Node *succ)
                : node_type(NodeType::Data),
                  prev(prev), succ(succ)
            {
                storage = new T(data);
            }

            ~Node()
            {
                delete storage;
            }
        };

        struct Block
        {
            const NodeType block_type;

            int n_node;
            Node *head, *tail;
            Block *prev, *succ;

            Block() : block_type(NodeType::Virtual)
            {
                n_node = 0;
                head = new Node();
                tail = new Node();
                head->succ = tail;
                tail->prev = head;
                prev = succ = nullptr;
            }

            Block(Block *prev, Block *succ)
                : block_type(NodeType::Data)
            {
                n_node = 0;
                head = new Node();
                tail = new Node();
                head->succ = tail;
                tail->prev = head;

                this->prev = prev;
                this->succ = succ;
            }

            Block(const Block &other)
                : block_type(NodeType::Data)
            {
                if (other.block_type == NodeType::Virtual)
                    throw index_out_of_bound();

                n_node = other.n_node;
                head = new Node();
                tail = new Node();
                head->succ = tail;
                tail->prev = head;
                prev = succ = nullptr;

                Node *current = head;
                for (Node *t = other.head->succ; t != other.tail; t = t->succ)
                {
                    Node *x = new Node(*(t->storage));
                    current->succ->prev = x;
                    x->succ = current->succ;
                    x->prev = current;
                    current->succ = x;

                    current = x;
                }
            }

            ~Block()
            {
                Node *x = head;
                do
                {
                    Node *succ = x->succ;
                    delete x;
                    x = succ;
                } while (x != nullptr);
            }

            void merge(int blk_size)
            {
                if (n_node == 0)
                {
                    prev->succ = succ;
                    succ->prev = prev;
                    delete this;
                    return;
                }

                if (prev->block_type == NodeType::Data &&
                    prev->n_node + n_node <= blk_size)
                    merge(prev, this);
                else if (succ->block_type == NodeType::Data &&
                         succ->n_node + n_node <= blk_size)
                    merge(this, succ);
            }

            // merge and delete succ
            static void merge(Block *prev, Block *succ)
            {
                // merge node
                prev->tail->prev->succ = succ->head->succ;
                succ->head->succ->prev = prev->tail->prev;
                succ->tail->prev->succ = prev->tail;
                prev->tail->prev = succ->tail->prev;
                prev->n_node += succ->n_node;

                // merge block
                prev->succ = succ->succ;
                succ->succ->prev = prev;

                succ->head->succ = succ->tail;
                delete succ;
            }

            Node *at(int k)
            {
                // [0, n_node]
                if (k > n_node)
                    throw index_out_of_bound();
                Node *x = head->succ;
                while (k--)
                    x = x->succ;
                return x;
            }

            static void split(Block *block, int blk_size)
            {
                if (block->n_node < 2 * blk_size)
                    return;

                Block *prev = new Block(block->prev, nullptr);
                block->prev->succ = prev;
                block->head->succ->prev = prev->head;
                prev->head->succ = block->head->succ;
                prev->n_node = blk_size;

                Block *succ = new Block(prev, block->succ);
                block->succ->prev = succ;
                succ->n_node = block->n_node - blk_size;
                block->tail->prev->succ = succ->tail;
                succ->tail->prev = block->tail->prev;
                prev->succ = succ;

                Node *x = block->at(blk_size - 1);
                succ->head->succ = x->succ;
                x->succ->prev = succ->head;
                succ->tail->prev = block->tail->prev;

                prev->head->succ = block->head->succ;
                prev->tail->prev = x;
                x->succ = prev->tail;

                block->head->succ = block->tail;
                delete block;
            }

            /**
             * insert before k
             */
            void insert(int k, const T &value, int blk_size)
            {
                insert(this->at(k), value, blk_size);
            }

            void insert(Node *x, const T &value, int blk_size)
            {
                Node *new_node = new Node(value, x->prev, x);

                x->prev->succ = new_node;
                x->prev = new_node;

                n_node++;
                split(this, blk_size);
            }

            void remove(int k, int blk_size)
            {
                remove(this->at(k), blk_size);
            }

            void remove(Node *x, int blk_size)
            {
                x->succ->prev = x->prev;
                x->prev->succ = x->succ;
                delete x;

                n_node--;
                merge(blk_size);
            }
        };
        Block *head, *tail;

        pair<Block *, Node *> find(int k) const
        {
            if (k > total_size)
                throw index_out_of_bound();

            Block *block = head->succ;
            while (block->n_node <= k)
            {
                k -= block->n_node;
                block = block->succ;
            }
            return pair<Block *, Node *>(block, block->at(k));
        }

    public:
        // debug function
        void display()
        {
            printf("[INFO]: display\n");
            for (Block *block = head->succ; block != tail; block = block->succ)
                for (Node *x = block->head->succ; x != block->tail; x = x->succ)
                    printf("%d ", *(x->storage));
            printf("\n");
            for (Block *block = tail->prev; block != head; block = block->prev)
                for (Node *x = block->tail->prev; x != block->head; x = x->prev)
                    printf("%d ", *(x->storage));
            printf("\n");
        }

    public:
        class const_iterator;
        class iterator
        {
            friend class deque<T>;

        private:
            deque<T> *deque_ptr;
            int index;

            /**
             * TODO add data members
             *   just add whatever you want.
             */
        public:
            iterator() : deque_ptr(nullptr), index(0) {}
            iterator(deque<T> *deque_ptr, int k) : deque_ptr(deque_ptr), index(k) {}
            iterator(const iterator &other)
            {
                deque_ptr = other.deque_ptr;
                index = other.index;
            }
            /**
             * return a new iterator which pointer n-next elements
             *   even if there are not enough elements, the behaviour is **undefined**.
             * as well as operator-
             */
            iterator operator+(const int &n) const
            {
                return iterator(deque_ptr, index + n);
            }
            iterator operator-(const int &n) const
            {
                return iterator(deque_ptr, index - n);
            }
            // return th distance between two iterator,
            // if these two iterators points to different vectors, throw invaild_iterator.
            int operator-(const iterator &rhs) const
            {
                if (deque_ptr != rhs.deque_ptr)
                    throw invalid_iterator();
                return index - rhs.index;
            }
            iterator &operator+=(const int &n)
            {
                index += n;
                return *this;
            }
            iterator &operator-=(const int &n)
            {
                index -= n;
                return *this;
            }
            /**
             * TODO iter++
             */
            iterator operator++(int)
            {
                iterator ret(*this);
                index++;
                return ret;
            }
            /**
             * TODO ++iter
             */
            iterator &operator++()
            {
                index++;
                return *this;
            }
            /**
             * TODO iter--
             */
            iterator operator--(int)
            {
                iterator ret(*this);
                index--;
                return ret;
            }
            /**
             * TODO --iter
             */
            iterator &operator--()
            {
                index--;
                return *this;
            }
            /**
             * TODO *it
             */
            T &operator*() const
            {
                return deque_ptr->at(index);
            }
            /**
             * TODO it->field
             */
            T *operator->() const noexcept
            {
                return &(deque_ptr->at(index));
            }
            /**
             * a operator to check whether two iterators are same (pointing to the same memory address).
             */
            bool operator==(const iterator &rhs) const
            {
                return deque_ptr == rhs.deque_ptr && index == rhs.index;
            }
            bool operator==(const const_iterator &rhs) const
            {
                return deque_ptr == rhs.deque_ptr && index == rhs.index;
            }
            /**
             * some other operator for iterator.
             */
            bool operator!=(const iterator &rhs) const
            {
                return deque_ptr != rhs.deque_ptr || index != rhs.index;
            }
            bool operator!=(const const_iterator &rhs) const
            {
                return deque_ptr != rhs.deque_ptr || index != rhs.index;
            }
        };
        class const_iterator
        {
            // it should has similar member method as iterator.
            //  and it should be able to construct from an iterator.
            friend class deque<T>;

        private:
            const deque<T> *deque_ptr;
            int index;
            // data members.
        public:
            const_iterator() : deque_ptr(nullptr), index(0) {}
            const_iterator(const deque<T> *deque_ptr, int k) : deque_ptr(deque_ptr), index(k) {}
            const_iterator(const const_iterator &other)
            {
                deque_ptr = other.deque_ptr;
                index = other.index;
            }
            const_iterator(const iterator &other)
            {
                deque_ptr = other.deque_ptr;
                index = other.index;
            }
            // And other methods in iterator.
            // And other methods in iterator.
            // And other methods in iterator.
            const_iterator operator+(const int &n) const
            {
                return const_iterator(deque_ptr, index + n);
            }
            const_iterator operator-(const int &n) const
            {
                return const_iterator(deque_ptr, index - n);
            }
            // return th distance between two iterator,
            // if these two iterators points to different vectors, throw invaild_iterator.
            int operator-(const const_iterator &rhs) const
            {
                if (deque_ptr != rhs.deque_ptr)
                    throw invalid_iterator();
                return index - rhs.index;
            }
            const_iterator &operator+=(const int &n)
            {
                index += n;
                return *this;
            }
            const_iterator &operator-=(const int &n)
            {
                index -= n;
                return *this;
            }
            /**
             * TODO iter++
             */
            const_iterator operator++(int)
            {
                const_iterator ret(*this);
                index++;
                return ret;
            }
            /**
             * TODO ++iter
             */
            const_iterator &operator++()
            {
                index++;
                return *this;
            }
            /**
             * TODO iter--
             */
            const_iterator operator--(int)
            {
                const_iterator ret(*this);
                index--;
                return ret;
            }
            /**
             * TODO --iter
             */
            const_iterator &operator--()
            {
                index--;
                return *this;
            }
            /**
             * TODO *it
             */
            const T &operator*() const
            {
                return deque_ptr->at(index);
            }
            /**
             * TODO it->field
             */
            const T *operator->() const noexcept
            {
                return &(deque_ptr->at(index));
            }
            /**
             * a operator to check whether two iterators are same (pointing to the same memory address).
             */
            bool operator==(const iterator &rhs) const
            {
                return deque_ptr == rhs.deque_ptr && index == rhs.index;
            }
            bool operator==(const const_iterator &rhs) const
            {
                return deque_ptr == rhs.deque_ptr && index == rhs.index;
            }
            /**
             * some other operator for iterator.
             */
            bool operator!=(const iterator &rhs) const
            {
                return deque_ptr != rhs.deque_ptr || index != rhs.index;
            }
            bool operator!=(const const_iterator &rhs) const
            {
                return deque_ptr != rhs.deque_ptr || index != rhs.index;
            }
        };
        /**
         * TODO Constructors
         */
        deque()
        {
            total_size = 0;
            blk_size = min_blk;
            head = new Block();
            tail = new Block();
            head->succ = tail;
            tail->prev = head;
        }
        deque(const deque &other)
        {
            blk_size = other.blk_size;
            total_size = other.total_size;
            head = new Block();
            tail = new Block();
            head->succ = tail;
            tail->prev = head;

            Block *current = head;
            for (Block *t = other.head->succ; t != other.tail; t = t->succ)
            {
                // add Block
                Block *x = new Block(*t);
                current->succ->prev = x;
                x->succ = current->succ;
                x->prev = current;
                current->succ = x;

                current = x;
            }
        }
        /**
         * TODO Deconstructor
         */
        ~deque()
        {
            Block *x = head;
            do
            {
                Block *succ = x->succ;
                delete x;
                x = succ;
            } while (x != nullptr);
        }
        /**
         * TODO assignment operator
         */
        deque &operator=(const deque &other)
        {
            if (this != &other)
            {
                clear();
                blk_size = other.blk_size;
                total_size = other.total_size;

                Block *current = head;
                for (Block *t = other.head->succ; t != other.tail; t = t->succ)
                {
                    // add Block
                    Block *x = new Block(*t);
                    current->succ->prev = x;
                    x->succ = current->succ;
                    x->prev = current;
                    current->succ = x;

                    current = x;
                }
            }
            return *this;
        }
        /**
         * access specified element with bounds checking
         * throw index_out_of_bound if out of bound.
         */
        T &at(const size_t &pos)
        {
            if (pos < 0 || pos >= total_size)
                throw index_out_of_bound();
            pair<Block *, Node *> result = find(pos);
            return *(result.second->storage);
        }
        const T &at(const size_t &pos) const
        {
            if (pos < 0 || pos >= total_size)
                throw index_out_of_bound();
            pair<Block *, Node *> result = find(pos);
            return *(result.second->storage);
        }
        T &operator[](const size_t &pos)
        {
            return at(pos);
        }
        const T &operator[](const size_t &pos) const
        {
            return at(pos);
        }
        /**
         * access the first element
         * throw container_is_empty when the container is empty.
         */
        const T &front() const
        {
            if (total_size == 0)
                throw container_is_empty();
            Block *x = head->succ;
            return *(x->head->succ->storage);
        }
        /**
         * access the last element
         * throw container_is_empty when the container is empty.
         */
        const T &back() const
        {
            if (total_size == 0)
                throw container_is_empty();
            Block *x = tail->prev;
            return *(x->tail->prev->storage);
        }
        /**
         * returns an iterator to the beginning.
         */
        iterator begin()
        {
            return iterator(this, 0);
        }
        const_iterator cbegin() const
        {
            return const_iterator(this, 0);
        }
        /**
         * returns an iterator to the end.
         */
        iterator end()
        {
            return iterator(this, total_size);
        }
        const_iterator cend() const
        {
            return const_iterator(this, total_size);
        }
        /**
         * checks whether the container is empty.
         */
        bool empty() const
        {
            return total_size == 0;
        }
        /**
         * returns the number of elements
         */
        size_t size() const
        {
            return total_size;
        }
        /**
         * clears the contents
         */
        void clear()
        {
            Block *x = head;
            do
            {
                Block *succ = x->succ;
                delete x;
                x = succ;
            } while (x != nullptr);

            total_size = 0;
            blk_size = min_blk;
            head = new Block();
            tail = new Block();
            head->succ = tail;
            tail->prev = head;
        }
        /**
         * inserts elements at the specified locat on in the container.
         * inserts value before pos
         * returns an iterator pointing to the inserted value
         *     throw if the iterator is invalid or it point to a wrong place.
         */
        iterator insert(iterator pos, const T &value)
        {
            if (pos.deque_ptr != this)
                throw invalid_iterator();
            if (pos.index < 0 || pos.index > total_size)
                throw invalid_iterator();

            if (pos.index == total_size)
            {
                push_back(value);
                return iterator(this, pos.index);
            }
            total_size++;
            pair<Block *, Node *> result = find(pos.index);
            result.first->insert(result.second, value, blk_size);
            return iterator(this, pos.index);
        }
        /**
         * removes specified element at pos.
         * removes the element at pos.
         * returns an iterator pointing to the following element, if pos pointing to the last element, end() will be returned.
         * throw if the container is empty, the iterator is invalid or it points to a wrong place.
         */
        iterator erase(iterator pos)
        {
            if (pos.deque_ptr != this)
                throw invalid_iterator();
            if (pos.index < 0 || pos.index >= total_size)
                throw invalid_iterator();

            total_size--;
            pair<Block *, Node *> result = find(pos.index);
            result.first->remove(result.second, blk_size);
            return iterator(this, pos.index);
        }
        /**
         * adds an element to the end
         */
        void push_back(const T &value)
        {
            blk_size = std::max(
                blk_size, (int)std::sqrt(++total_size));
            Block *x = tail->prev;
            if (x == head)
            {
                x = new Block(head, tail);
                head->succ = x;
                tail->prev = x;
            }
            x->insert(x->tail, value, blk_size);
        }
        /**
         * removes the last element
         *     throw when the container is empty.
         */
        void pop_back()
        {
            if (total_size == 0)
                throw container_is_empty();
            Block *x = tail->prev;
            x->remove(x->tail->prev, blk_size);
            total_size--;
        }
        /**
         * inserts an element to the beginning.
         */
        void push_front(const T &value)
        {
            blk_size = std::max(
                blk_size, (int)std::sqrt(++total_size));
            Block *x = head->succ;
            if (x == tail)
            {
                x = new Block(head, tail);
                head->succ = x;
                tail->prev = x;
            }
            x->insert(x->head->succ, value, blk_size);
        }
        /**
         * removes the first element.
         *     throw when the container is empty.
         */
        void pop_front()
        {
            if (total_size == 0)
                throw container_is_empty();
            Block *x = head->succ;
            x->remove(x->head->succ, blk_size);
            total_size--;
        }
    };

}

#endif
