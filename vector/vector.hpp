#ifndef SJTU_VECTOR_HPP
#define SJTU_VECTOR_HPP

#include "exceptions.hpp"

#include <climits>
#include <cstddef>

namespace sjtu
{
    /**
     * a Data container like std::vector
     * store Data in a successive memory and support random access.
     */
    template <typename T>
    class vector
    {
    private:
        T **storage;
        int current_size;
        int max_size;

    public:
        /**
         * TODO
         * a type for actions of the elements of a vector, and you should write
         *   a class named const_iterator with same interfaces.
         */
        /**
         * you can see RandomAccessIterator at CppReference for help.
         */
        class const_iterator;
        class iterator
        {
        private:
            vector<T> *vector_ptr;
            int index;
            /**
             * TODO add Data members
             *   just add whatever you want.
             */
        public:
            iterator() {}
            iterator(vector<T> *ptr, int idx) : vector_ptr(ptr), index(idx) {}
            iterator(const iterator &other)
            {
                vector_ptr = other.vector_ptr;
                index = other.index;
            }
            int iterator_pos()
            {
                return index;
            }
            /**
             * return a new iterator which pointer n-next elements
             * as well as operator-
             */
            iterator operator+(const int &n) const
            {
                return iterator(vector_ptr, index + n);
            }
            iterator operator-(const int &n) const
            {
                return iterator(vector_ptr, index - n);
            }
            // return the distance between two iterators,
            // if these two iterators point to different vectors, throw invalid_iterator.
            int operator-(const iterator &rhs) const
            {
                if (vector_ptr != rhs.vector_ptr)
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
                return vector_ptr->at(index);
            }
            /**
             * a operator to check whether two iterators are same (pointing to the same memory address).
             */
            bool operator==(const iterator &rhs) const
            {
                return vector_ptr == rhs.vector_ptr && index == rhs.index;
            }
            bool operator==(const const_iterator &rhs) const
            {
                return vector_ptr == rhs.vector_ptr && index == rhs.index;
            }
            /**
             * some other operator for iterator.
             */
            bool operator!=(const iterator &rhs) const
            {
                return vector_ptr != rhs.vector_ptr || index != rhs.index;
            }
            bool operator!=(const const_iterator &rhs) const
            {
                return vector_ptr != rhs.vector_ptr || index != rhs.index;
            }
        };
        /**
         * TODO
         * has same function as iterator, just for a const object.
         */
        class const_iterator
        {
        private:
            const vector<T> *vector_ptr;
            int index;
            /**
             * TODO add Data members
             *   just add whatever you want.
             */
        public:
            const_iterator(const vector<T> *ptr, int idx) : vector_ptr(ptr), index(idx) {}
            const_iterator(const const_iterator &other)
            {
                vector_ptr = other.vector_ptr;
                index = other.index;
            }
            /**
             * return a new iterator which pointer n-next elements
             * as well as operator-
             */
            const_iterator operator+(const int &n) const
            {
                return const_iterator(vector_ptr, index + n);
            }
            const_iterator operator-(const int &n) const
            {
                return const_iterator(vector_ptr, index - n);
            }
            // return the distance between two iterators,
            // if these two iterators point to different vectors, throw invalid_iterator.
            int operator-(const const_iterator &rhs) const
            {
                if (vector_ptr != rhs.vector_ptr)
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
                return vector_ptr->at(index);
            }
            /**
             * a operator to check whether two iterators are same (pointing to the same memory address).
             */
            bool operator==(const iterator &rhs) const
            {
                return vector_ptr == rhs.vector_ptr && index == rhs.index;
            }
            bool operator==(const const_iterator &rhs) const
            {
                return vector_ptr == rhs.vector_ptr && index == rhs.index;
            }
            /**
             * some other operator for iterator.
             */
            bool operator!=(const iterator &rhs) const
            {
                return vector_ptr != rhs.vector_ptr || index != rhs.index;
            }
            bool operator!=(const const_iterator &rhs) const
            {
                return vector_ptr != rhs.vector_ptr || index != rhs.index;
            }
        };
        /**
         * TODO Constructs
         * Atleast two: default constructor, copy constructor
         */
        vector()
        {
            current_size = 0;
            max_size = 1;
            storage = new T *[max_size];
        }
        vector(const vector &other)
        {
            max_size = other.max_size;
            current_size = other.current_size;
            storage = new T *[max_size];
            for (int i = 0; i < current_size; ++i)
                storage[i] = new T(*other.storage[i]);
        }
        /**
         * TODO Destructor
         */
        ~vector()
        {
            for (int i = 0; i < current_size; ++i)
                delete storage[i];
            delete[] storage;
        }
        /**
         * TODO Assignment operator
         */
        vector &operator=(const vector &other)
        {
            if (this != &other)
            {
                for (int i = 0; i < current_size; ++i)
                    delete storage[i];
                delete[] storage;
                max_size = other.max_size;
                current_size = other.current_size;
                storage = new T *[max_size];
                for (int i = 0; i < current_size; ++i)
                    storage[i] = new T(*other.storage[i]);
            }
            return *this;
        }
        /**
         * assigns specified element with bounds checking
         * throw index_out_of_bound if pos is not in [0, size)
         */
        T &at(const size_t &pos)
        {
            if (pos < 0 || pos >= current_size)
                throw index_out_of_bound();
            return *storage[pos];
        }
        const T &at(const size_t &pos) const
        {
            if (pos < 0 || pos >= current_size)
                throw index_out_of_bound();
            return *storage[pos];
        }
        /**
         * assigns specified element with bounds checking
         * throw index_out_of_bound if pos is not in [0, size)
         * !!! Pay attentions
         *   In STL this operator does not check the boundary but I want you to do.
         */
        T &operator[](const size_t &pos)
        {
            if (pos < 0 || pos >= current_size)
                throw index_out_of_bound();
            return *storage[pos];
        }
        const T &operator[](const size_t &pos) const
        {
            if (pos < 0 || pos >= current_size)
                throw index_out_of_bound();
            return *storage[pos];
        }
        /**
         * access the first element.
         * throw container_is_empty if size == 0
         */
        const T &front() const
        {
            if (empty())
                throw container_is_empty();
            return *storage[0];
        }
        /**
         * access the last element.
         * throw container_is_empty if size == 0
         */
        const T &back() const
        {
            if (empty())
                throw container_is_empty();
            return *storage[current_size - 1];
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
            return iterator(this, current_size);
        }
        const_iterator cend() const
        {
            return const_iterator(this, current_size);
        }
        /**
         * checks whether the container is empty
         */
        bool empty() const
        {
            return current_size == 0;
        }
        /**
         * returns the number of elements
         */
        size_t size() const
        {
            return current_size;
        }
        /**
         * clears the contents
         */
        void clear()
        {
            for (int i = 0; i < current_size; ++i)
                delete storage[i];
            delete[] storage;
            current_size = 0;
            max_size = 1;
            storage = new T *[max_size];
        }
        /**
         * inserts value before pos
         * returns an iterator pointing to the inserted value.
         */
        iterator insert(iterator pos, const T &value)
        {
            return insert(pos.iterator_pos(), value);
        }
        /**
         * inserts value at index ind.
         * after inserting, this->at(ind) == value
         * returns an iterator pointing to the inserted value.
         * throw index_out_of_bound if ind > size (in this situation ind can be size because after inserting the size will increase 1.)
         */
        iterator insert(const size_t &ind, const T &value)
        {
            if (ind < 0 || ind > current_size)
                throw index_out_of_bound();

            if (current_size == max_size)
            {
                T **old_storage = storage;
                max_size <<= 1;
                storage = new T *[max_size];
                for (int i = 0; i < current_size; ++i)
                    storage[i] = old_storage[i];
                delete[] old_storage;
            }

            for (int i = current_size; i > ind; --i)
                storage[i] = storage[i - 1];
            storage[ind] = new T(value);
            current_size++;
            return iterator(this, ind);
        }
        /**
         * removes the element at pos.
         * return an iterator pointing to the following element.
         * If the iterator pos refers the last element, the end() iterator is returned.
         */
        iterator erase(iterator pos)
        {
            return erase(pos.iterator_pos());
        }
        /**
         * removes the element with index ind.
         * return an iterator pointing to the following element.
         * throw index_out_of_bound if ind >= size
         */
        iterator erase(const size_t &ind)
        {
            if (ind < 0 || ind >= current_size)
                throw index_out_of_bound();
            delete storage[ind];
            for (int i = ind; i < current_size - 1; i++)
                storage[i] = storage[i + 1];
            current_size--;
            return iterator(this, ind);
        }
        /**
         * adds an element to the end.
         */
        void push_back(const T &value)
        {
            if (current_size == max_size)
            {
                T **old_storage = storage;
                max_size <<= 1;
                storage = new T *[max_size];
                for (int i = 0; i < current_size; ++i)
                    storage[i] = old_storage[i];
                delete[] old_storage;
            }
            storage[current_size++] = new T(value);
        }
        /**
         * remove the last element from the end.
         * throw container_is_empty if size() == 0
         */
        void pop_back()
        {
            if (empty())
                throw container_is_empty();
            delete storage[--current_size];
        }
    };

}

#endif
