#ifndef SJTU_VECTOR_HPP
#define SJTU_VECTOR_HPP

#include "exceptions.hpp"

#include <climits>
#include <cstddef>

namespace sjtu {
/**
 * a Data container like std::vector
 * store Data in a successive memory and support random access.
 */
template<typename T>
class vector {
	private:
		T **Data;
		int currentsize;
		int maxsize;
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
		class iterator {
			private:
				vector<T> *ptr;
				int idx;
				/**
				 * TODO add Data members
				 *   just add whatever you want.
				 */
			public:
				iterator(){}
				iterator(vector<T> *_ptr,int _idx):ptr(_ptr),idx(_idx){}
				iterator(const iterator &other)
				{
					ptr=other.ptr;
					idx=other.idx;
				}
				int iterator_pos()
				{
					return idx;
				}
				/**
				 * return a new iterator which pointer n-next elements
				 * as well as operator-
				 */
				iterator operator+(const int &n) const 
				{
					return iterator(ptr,idx+n);
				}
				iterator operator-(const int &n) const 
				{
					return iterator(ptr,idx-n);
					//TODO
				}
				// return the distance between two iterators,
				// if these two iterators point to different vectors, throw invaild_iterator.
				int operator-(const iterator &rhs) const 
				{
					if (ptr!=rhs.ptr)
						throw invalid_iterator();
					return idx-rhs.idx;
					//TODO
				}
				iterator& operator+=(const int &n) 
				{
					idx+=n;
					return *this;
					//TODO
				}
				iterator& operator-=(const int &n) 
				{
					idx-=n;
					return *this;
					//TODO
				}
				/**
				 * TODO iter++
				 */
				iterator operator++(int) 
				{
					iterator tmp(*this);
					idx++;
					return tmp;
				}
				/**
				 * TODO ++iter
				 */
				iterator& operator++() 
				{
					idx++;
					return *this;
				}
				/**
				 * TODO iter--
				 */
				iterator operator--(int) 
				{
					iterator tmp(*this);
					idx--;
					return tmp;
				}
				/**
				 * TODO --iter
				 */
				iterator& operator--() 
				{
					idx--;
					return *this;
				}
				/**
				 * TODO *it
				 */
				T& operator*() const
				{
					return ptr->at(idx);
				}
				/**
				 * a operator to check whether two iterators are same (pointing to the same memory address).
				 */
				bool operator==(const iterator &rhs) const 
				{
					if (ptr!=rhs.ptr) return 0;
					return idx==rhs.idx;
				}
				bool operator==(const const_iterator &rhs) const 
				{
					if (ptr!=rhs.ptr) return 0;
					return idx==rhs.idx;
				}
				/**
				 * some other operator for iterator.
				 */
				bool operator!=(const iterator &rhs) const 
				{
					if (ptr!=rhs.ptr) return 1;
					return idx!=rhs.idx;
				}
				bool operator!=(const const_iterator &rhs) const 
				{
					if (ptr!=rhs.ptr) return 1;
					return idx!=rhs.idx;
				}
		};
			/**
			 * TODO
			 * has same function as iterator, just for a const object.
			 */
		class const_iterator {
			private:
				const vector<T> *ptr;
				int idx;
				/**
				 * TODO add Data members
				 *   just add whatever you want.
				 */
			public:
				const_iterator(const vector<T> *_ptr,int _idx):ptr(_ptr),idx(_idx){}
				const_iterator(const const_iterator &other)
				{
					ptr=other.ptr;
					idx=other.idx;
				}
				/**
				 * return a new iterator which pointer n-next elements
				 * as well as operator-
				 */
				const_iterator operator+(const int &n) const 
				{
					return const_iterator(ptr,idx+n);
				}
				const_iterator operator-(const int &n) const 
				{
					return const_iterator(ptr,idx-n);
					//TODO
				}
				// return the distance between two iterators,
				// if these two iterators point to different vectors, throw invaild_iterator.
				int operator-(const const_iterator &rhs) const 
				{
					if (ptr!=rhs.ptr)
						throw invalid_iterator();
					return idx-rhs.idx;
					//TODO
				}
				const_iterator& operator+=(const int &n) 
				{
					idx+=n;
					return *this;
					//TODO
				}
				const_iterator& operator-=(const int &n) 
				{
					idx-=n;
					return *this;
					//TODO
				}
				/**
				 * TODO iter++
				 */
				const_iterator operator++(int) 
				{
					const_iterator tmp(*this);
					idx++;
					return tmp;
				}
				/**
				 * TODO ++iter
				 */
				const_iterator& operator++() 
				{
					idx++;
					return *this;
				}
				/**
				 * TODO iter--
				 */
				const_iterator operator--(int) 
				{
					const_iterator tmp(*this);
					idx--;
					return tmp;
				}
				/**
				 * TODO --iter
				 */
				const_iterator& operator--() 
				{
					idx--;
					return *this;
				}
				/**
				 * TODO *it
				 */
				const T& operator*() const
				{
					return ptr->at(idx);
				}
				/**
				 * a operator to check whether two iterators are same (pointing to the same memory address).
				 */
				bool operator==(const iterator &rhs) const 
				{
					if (ptr!=rhs.ptr) return 0;
					return idx==rhs.idx;
				}
				bool operator==(const const_iterator &rhs) const 
				{
					if (ptr!=rhs.ptr) return 0;
					return idx==rhs.idx;
				}
				/**
				 * some other operator for iterator.
				 */
				bool operator!=(const iterator &rhs) const 
				{
					if (ptr!=rhs.ptr) return 1;
					return idx!=rhs.idx;
				}
				bool operator!=(const const_iterator &rhs) const 
				{
					if (ptr!=rhs.ptr) return 1;
					return idx!=rhs.idx;
				}
		};
		/**
		 * TODO Constructs
		 * Atleast two: default constructor, copy constructor
		 */
		vector() 
		{
			currentsize=0;
			maxsize=1;
			Data=new T*[maxsize];
		}
		vector(const vector &other) 
		{
			if (this!=&other) 
			{
				maxsize=other.maxsize;
				currentsize=other.currentsize;
				Data=new T*[maxsize];
				for(int i=0;i<currentsize;++i) Data[i]=new T(*other.Data[i]);
			}
		}
		/**
		 * TODO Destructor
		 */
		~vector() 
		{
			for(int i=0;i<currentsize;++i) delete Data[i];
			delete [] Data; 
		}
		/**
		 * TODO Assignment operator
		 */
		vector &operator=(const vector &other) 
		{
			if (this!=&other)
			{
				clear();
				maxsize=other.maxsize;
				currentsize=other.currentsize;
				Data=new T*[maxsize];
				for(int i=0;i<currentsize;++i) Data[i]=new T(*other.Data[i]);
			}
			return *this;
		}
		/**
		 * assigns specified element with bounds checking
		 * throw index_out_of_bound if pos is not in [0, size)
		 */
		T & at(const size_t &pos) 
		{
			if (pos<0||pos>currentsize)
				throw index_out_of_bound();
			return *Data[pos];
		}
		const T & at(const size_t &pos) const 
		{
			if (pos<0||pos>currentsize)
				throw index_out_of_bound();
			return *Data[pos];
		}
		/**
		 * assigns specified element with bounds checking
		 * throw index_out_of_bound if pos is not in [0, size)
		 * !!! Pay attentions
		 *   In STL this operator does not check the boundary but I want you to do.
		 */
		T & operator[](const size_t &pos) 
		{
			if (pos<0||pos>currentsize) 
				throw index_out_of_bound();
			return *Data[pos];
		}
		const T & operator[](const size_t &pos) const 
		{
			if (pos<0||pos>currentsize) 
				throw index_out_of_bound();
			return *Data[pos];
		}
		/**
		 * access the first element.
		 * throw container_is_empty if size == 0
		 */
		const T & front() const 
		{
			if (currentsize==0) 
				throw container_is_empty();
			return *Data[0];
		}
		/**
		 * access the last element.
		 * throw container_is_empty if size == 0
		 */
		const T & back() const 
		{
			if (currentsize==0) 
				throw container_is_empty();
			return *Data[currentsize-1];
		}
		/**
		 * returns an iterator to the beginning.
		 */
		iterator begin() 
		{
			return iterator(this,0);
		}
		const_iterator cbegin() const 
		{
			return const_iterator(this,0);
		}
		/**
		 * returns an iterator to the end.
		 */
		iterator end() 
		{
			return iterator(this,currentsize);
		}
		const_iterator cend() const 
		{
			return const_iterator(this,currentsize);
		}
		/**
		 * checks whether the container is empty
		 */
		bool empty() const 
		{
			return currentsize==0;
		}
		/**
		 * returns the number of elements
		 */
		size_t size() const 
		{
			return currentsize;
		}
		/**
		 * clears the contents
		 */
		void clear() 
		{
			for(int i=0;i<currentsize;++i) delete Data[i];
			delete [] Data; 
			currentsize=0;
			maxsize=1;
			Data=new T*[maxsize];		
		}
		/**
		 * inserts value before pos
		 * returns an iterator pointing to the inserted value.
		 */
		iterator insert(iterator pos, const T &value) 
		{
			return insert(pos.iterator_pos(),value);
		}
		/**
		 * inserts value at index ind.
		 * after inserting, this->at(ind) == value
		 * returns an iterator pointing to the inserted value.
		 * throw index_out_of_bound if ind > size (in this situation ind can be size because after inserting the size will increase 1.)
		 */
		iterator insert(const size_t &ind, const T &value) 
		{
			if (ind<0||ind>currentsize)
				throw index_out_of_bound();
			if (currentsize==maxsize)
			{
				T **tmp=Data;
				maxsize*=2;
				Data=new T*[maxsize];
				for(int i=0;i<currentsize;++i) Data[i]=tmp[i];
				delete [] tmp;
			}
			for(int i=currentsize;i>ind;--i) Data[i]=Data[i-1];
			Data[ind]=new T(value);
			currentsize++;
			return iterator(this,ind);
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
			if (ind<0||ind>=currentsize)
				throw index_out_of_bound();
			delete Data[ind];
			for(int i=ind;i<currentsize-1;i++) Data[i]=Data[i+1];
			currentsize--;
			return iterator(this,ind);
		}
		/**
		 * adds an element to the end.
		 */
		void push_back(const T &value) 
		{
			if (currentsize==maxsize)
			{
				T **tmp=Data;
				maxsize*=2;
				Data=new T*[maxsize];
				for(int i=0;i<currentsize;++i) Data[i]=tmp[i];
				delete [] tmp;
			}
			Data[currentsize++]=new T(value);	
		}
		/**
		 * remove the last element from the end.
		 * throw container_is_empty if size() == 0
		 */
		void pop_back() 
		{
			if (currentsize==0) 
				throw container_is_empty();
			--currentsize;
		}
	};


}

#endif
