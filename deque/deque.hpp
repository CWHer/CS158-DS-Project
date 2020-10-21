#ifndef SJTU_DEQUE_HPP
#define SJTU_DEQUE_HPP

#include "exceptions.hpp"

#include <cstddef>
#include <cmath>
namespace sjtu { 

    template<class T>
    class deque {
        private:
            int sumsize,blksize;
            struct node
            {
                T val;
                node *pre,*nxt;
                node(const T &_val):val(_val)
                {
                    pre=nxt=NULL;
                }
                node(const T &_val,node *_pre,node *_nxt):val(_val),pre(_pre),nxt(_nxt){}
            };
            struct root
            {
                int size;
                node *Head,*Tail;
                root *pre,*nxt;
                root()
                {
                    size=0;
                    Head=Tail=NULL;
                    pre=nxt=NULL;
                }
                root(const int &_size):size(_size)
                {
                    Head=Tail=NULL;
                    pre=nxt=NULL;
                }
                root(const int &_size,root *_pre,root *_nxt):size(_size),pre(_pre),nxt(_nxt)
                {
                    Head=Tail=NULL;
                }
                void merge(const int &blksize)
                {
                    root *L=pre,*R=nxt;
                    if (size==0)
                    {
                        L->nxt=R,R->pre=L;
                        delete this;
                        return;
                    }
                    if (L->size>0&&L->size+size<1.2*blksize)
                    {
                        L->Tail->nxt=Head;
                        Head->pre=L->Tail;
                        L->size+=size;
                        L->Tail=Tail;
                        L->nxt=R,R->pre=L;
                        delete this;
                        return;
                    }
                    if (R->size>0&&R->size+size<1.2*blksize)
                    {
                        R->Head->pre=Tail;
                        Tail->nxt=R->Head;
                        R->size+=size;
                        R->Head=Head;
                        L->nxt=R,R->pre=L;
                        delete this;
                        return;
                    }
                }
                void split(const int &blksize)
                {
                    if (size<2*blksize) return;
                    root *ls=new root(blksize,pre,NULL);
                    root *rs=new root(size-blksize,ls,nxt);
                    ls->nxt=rs;
                    pre->nxt=ls,nxt->pre=rs;
                    node *o=Head;
                    for(int i=blksize;i;--i) o=o->nxt;
                    ls->Head=Head;
                    ls->Tail=o->pre,o->pre->nxt=NULL;
                    rs->Tail=Tail;
                    rs->Head=o,o->pre=NULL;
                    delete this;
                }
                void push_back(const T &value,const int &blksize)
                {
                    if (size==0)
                        Head=Tail=new node(value);
                    else
                    {
                        Tail->nxt=new node(value,Tail,NULL);
                        Tail=Tail->nxt;
                    }
                    size++,split(blksize);
                }
                void pop_back(const int &blksize)
                {
                    
                    node *tmp=Tail;
                    if (Head==Tail)
                        Head=Tail=NULL;
                    else
                    {
                        Tail=Tail->pre;
                        Tail->nxt=NULL;
                    }
                    delete tmp;
                    size--,merge(blksize);
                }
                void push_front(const T &value,const int &blksize)
                {
                    if (size==0)
                        Head=Tail=new node(value);
                    else
                    {
                        Head->pre=new node(value,NULL,Head);
                        Head=Head->pre;
                    }
                    size++,split(blksize);
                }
                void pop_front(const int &blksize)
                {   
                    node *tmp=Head;
                    if (Head==Tail)
                        Head=Tail=NULL;
                    else
                    {
                        Head=Head->nxt;
                        Head->pre=NULL;
                    }
                    delete tmp;
                    size--,merge(blksize);    
                }
            };
            root *Head,*Tail;
            template<class Tnode>
            Tnode* del(Tnode *o)
            {
                Tnode *L=o->pre,*R=o->nxt;
                if (L!=NULL) L->nxt=R;
                if (R!=NULL) R->pre=L;
                delete o;
                return R;
            }
        public:
            class const_iterator;
            class iterator {
                friend class deque<T>;
                private:
                    deque<T> *ptr;
                    root *now_rt;
                    node *now;
                    int K_th(const int &sumsize) const
                    {
                        if (now_rt==ptr->Tail) return sumsize+1;
                        int ret=0;
                        root *x=ptr->Head->nxt;
                        while (x!=now_rt)
                        {
                            ret+=x->size;
                            x=x->nxt;
                        }
                        node *o=x->Head;
                        while (o!=now)
                        {
                            ret++;
                            o=o->nxt;
                        }
                        return ++ret;
                    }
                    /**
                     * TODO add data members
                     *   just add whatever you want.
                     */
                public:
                    iterator(){}
                    iterator(deque<T> *_ptr,int K):ptr(_ptr)
                    {
                        now_rt=_ptr->Head->nxt; 
                        while (now_rt!=_ptr->Tail&&K>now_rt->size)
                        {
                            K-=now_rt->size;
                            now_rt=now_rt->nxt;
                        }
                        if (now_rt==_ptr->Tail) return;
                        now=now_rt->Head,K--;
                        while (K--) now=now->nxt;
                    }
                    iterator(deque<T> *_ptr,root *_now_rt=NULL,node *_now=NULL):ptr(_ptr),now_rt(_now_rt),now(_now){}
                    iterator(const iterator &other)
                    {
                        ptr=other.ptr;
                        now_rt=other.now_rt;
                        now=other.now;
                    }
                    /**
                     * return a new iterator which pointer n-next elements
                     *   even if there are not enough elements, the behaviour is **undefined**.
                     * as well as operator-
                     */
                    iterator operator+(const int &n) const 
                    { 
                        if (now_rt==ptr->Tail&&n>0)
                            throw invalid_iterator();
                        if (n<0) return this->operator-(-n);
                        int K=n;
                        iterator ret(ptr,now_rt,now);
                        while (ret.now!=ret.now_rt->Tail&&K--) ret.now=ret.now->nxt;
                        if (K<=0) return ret;
                        ret.now_rt=ret.now_rt->nxt;
                        while (ret.now_rt!=ret.ptr->Tail&&K>ret.now_rt->size) 
                            K-=ret.now_rt->size,ret.now_rt=ret.now_rt->nxt;
                        if (ret.now_rt==ret.ptr->Tail) return ret;
                        ret.now=ret.now_rt->Head,K--;
                        while (K--) ret.now=ret.now->nxt;
                        return ret; 
                    }
                    iterator operator-(const int &n) const 
                    {
                        if (n<0) return this->operator+(-n);
                        int K=n;
                        iterator ret(ptr,now_rt,now);
                        if (ret.now_rt!=ret.ptr->Tail)
                            while (ret.now!=ret.now_rt->Head&&K--) ret.now=ret.now->pre;
                        if (K<=0) return ret;
                        ret.now_rt=ret.now_rt->pre;
                        while (ret.now_rt!=ret.ptr->Head&&K>ret.now_rt->size) 
                            K-=ret.now_rt->size,ret.now_rt=ret.now_rt->pre;
                        if (ret.now_rt==ret.ptr->Head) 
                            throw invalid_iterator();
                        ret.now=ret.now_rt->Tail,K--;
                        while (K--) ret.now=ret.now->pre;
                        return ret; 
                    }
                    // return th distance between two iterator,
                    // if these two iterators points to different vectors, throw invaild_iterator.
                    int operator-(const iterator &rhs) const 
                    {
                        if (ptr!=rhs.ptr)
                            throw invalid_iterator();
                        return K_th(ptr->sumsize)-rhs.K_th(ptr->sumsize);
                        
                    }
                    iterator& operator+=(const int &n) 
                    {
                        if (now_rt==ptr->Tail&&n>0)
                            throw invalid_iterator();
                        if (n<0) return this->operator-=(-n);
                        int K=n;
                        while (now!=now_rt->Tail&&K--) now=now->nxt;
                        if (K<=0) return *this;
                        now_rt=now_rt->nxt;
                        while (now_rt!=ptr->Tail&&K>now_rt->size) 
                            K-=now_rt->size,now_rt=now_rt->nxt;
                        if (now_rt==ptr->Tail) return *this;
                        now=now_rt->Head,K--;
                        while (K--) now=now->nxt;
                        return *this; 
                    }
                    iterator& operator-=(const int &n) 
                    {
                        if (n<0) return this->operator+=(-n);
                        int K=n;
                        if (now_rt!=ptr->Tail)
                            while (now!=now_rt->Head&&K--) now=now->pre;
                        if (K<=0) return *this;
                        now_rt=now_rt->pre;
                        while (now_rt!=ptr->Head&&K>now_rt->size) 
                            K-=now_rt->size,now_rt=now_rt->pre;
                        if (now_rt==ptr->Head) 
                            throw invalid_iterator();
                        now=now_rt->Tail,K--;
                        while (K--) now=now->pre;
                        return *this; 
                    }
                    /**
                     * TODO iter++
                     */
                    iterator operator++(int) 
                    {
                        iterator tmp(*this);
                        this->operator+=(1);
                        return tmp;
                    }
                    /**
                     * TODO ++iter
                     */
                    iterator& operator++() 
                    {
                        this->operator+=(1);
                        return *this;
                    }
                    /**
                     * TODO iter--
                     */
                    iterator operator--(int) 
                    {
                        iterator tmp(*this);
                        this->operator-=(1);
                        return tmp;
                    }
                    /**
                     * TODO --iter
                     */
                    iterator& operator--() 
                    {
                        this->operator-=(1);
                        return *this;
                    }
                    /**
                     * TODO *it
                     */
                    T& operator*() const 
                    {
                        if (now_rt==ptr->Tail) 
                            throw invalid_iterator();
                        return now->val;
                    }
                    /**
                     * TODO it->field
                     */
                    T* operator->() const noexcept 
                    {
                        return &(this->now->val);
                    }
                    /**
                     * a operator to check whether two iterators are same (pointing to the same memory).
                     */
                    bool operator==(const iterator &rhs) const 
                    {
                        if (ptr!=rhs.ptr) return 0;
                        if (now_rt==ptr->Tail&&rhs.now_rt==ptr->Tail) return 1;
                        return now==rhs.now;
                    }
                    bool operator==(const const_iterator &rhs) const 
                    {
                        if (ptr!=rhs.ptr) return 0;
                        if (now_rt==ptr->Tail&&rhs.now_rt==ptr->Tail) return 1;
                        return now==rhs.now;
                    }
                    /**
                     * some other operator for iterator.
                     */
                    bool operator!=(const iterator &rhs) const 
                    {
                        if (ptr!=rhs.ptr) return 1;
                        if (now_rt==ptr->Tail&&rhs.now_rt==ptr->Tail) return 0;
                        return now!=rhs.now;
                    }
                    bool operator!=(const const_iterator &rhs) const 
                    {
                        if (ptr!=rhs.ptr) return 1;
                        if (now_rt==ptr->Tail&&rhs.now_rt==ptr->Tail) return 0;
                        return now!=rhs.now;
                    }
            };
            class const_iterator {
                // it should has similar member method as iterator.
                //  and it should be able to construct from an iterator.
                friend class deque<T>;
                private:
                    const deque<T> *ptr;
                    root *now_rt;
                    node *now;
                    int K_th(const int &sumsize) const
                    {
                        if (now_rt==ptr->Tail) return sumsize+1;
                        int ret=0;
                        root *x=ptr->Head->nxt;
                        while (x!=now_rt)
                        {
                            ret+=x->size;
                            x=x->nxt;
                        }
                        node *o=x->Head;
                        while (o!=now)
                        {
                            ret++;
                            o=o->nxt;
                        }
                        return ++ret;
                    }
                    // data members.
                public:
                    const_iterator() {}
                    const_iterator(const deque<T> *_ptr,int K):ptr(_ptr)
                    {
                        now_rt=_ptr->Head->nxt; 
                        while (now_rt!=_ptr->Tail&&K>now_rt->size)
                        {
                            K-=now_rt->size;
                            now_rt=now_rt->nxt;
                        }
                        if (now_rt==_ptr->Tail) return;
                        now=now_rt->Head,K--;
                        while (K--) now=now->nxt;
                    }
                    const_iterator(const deque<T> *_ptr,root *_now_rt=NULL,node *_now=NULL):ptr(_ptr),now_rt(_now_rt),now(_now){}
                    const_iterator(const const_iterator &other)
                    {
                        ptr=other.ptr;
                        now_rt=other.now_rt;
                        now=other.now;
                    }
                    const_iterator(const iterator &other)
                    {
                        ptr=other.ptr;
                        now_rt=other.now_rt;
                        now=other.now;
                    }
                    // And other methods in iterator.
                    // And other methods in iterator.
                    // And other methods in iterator.
                    const_iterator operator+(const int &n) const 
                    { 
                        if (now_rt==ptr->Tail&&n>0)
                            throw invalid_iterator();
                        if (n<0) return this->operator-(-n);
                        int K=n;
                        const_iterator ret(ptr,now_rt,now);
                        while (ret.now!=ret.now_rt->Tail&&K--) ret.now=ret.now->nxt;
                        if (K<=0) return ret;
                        ret.now_rt=ret.now_rt->nxt;
                        while (ret.now_rt!=ret.ptr->Tail&&K>ret.now_rt->size) 
                            K-=ret.now_rt->size,ret.now_rt=ret.now_rt->nxt;
                        if (ret.now_rt==ret.ptr->Tail) return ret;
                        ret.now=ret.now_rt->Head,K--;
                        while (K--) ret.now=ret.now->nxt;
                        return ret; 
                    }
                    const_iterator operator-(const int &n) const 
                    {
                        if (n<0) return this->operator+(-n);
                        int K=n;
                        const_iterator ret(ptr,now_rt,now);
                        if (ret.now_rt!=ret.ptr->Tail)
                            while (ret.now!=ret.now_rt->Head&&K--) ret.now=ret.now->pre;
                        if (K<=0) return ret;
                        ret.now_rt=ret.now_rt->pre;
                        while (ret.now_rt!=ret.ptr->Head&&K>ret.now_rt->size) 
                            K-=ret.now_rt->size,ret.now_rt=ret.now_rt->pre;
                        if (ret.now_rt==ret.ptr->Head) 
                            throw invalid_iterator();
                        ret.now=ret.now_rt->Tail,K--;
                        while (K--) ret.now=ret.now->pre;
                        return ret; 
                    }
                    // return th distance between two iterator,
                    // if these two iterators points to different vectors, throw invaild_iterator.
                    int operator-(const const_iterator &rhs) const 
                    {
                        if (ptr!=rhs.ptr)
                            throw invalid_iterator();
                        return K_th(ptr->sumsize)-rhs.K_th(ptr->sumsize);
                        
                    }
                    const_iterator& operator+=(const int &n) 
                    {
                        if (now_rt==ptr->Tail&&n>0)
                            throw invalid_iterator();
                        if (n<0) return this->operator-=(-n);
                        int K=n;
                        while (now!=now_rt->Tail&&K--) now=now->nxt;
                        if (K<=0) return *this;
                        now_rt=now_rt->nxt;
                        while (now_rt!=ptr->Tail&&K>now_rt->size) 
                            K-=now_rt->size,now_rt=now_rt->nxt;
                        if (now_rt==ptr->Tail) return *this;
                        now=now_rt->Head,K--;
                        while (K--) now=now->nxt;
                        return *this; 
                    }
                    const_iterator& operator-=(const int &n) 
                    {
                        if (n<0) return this->operator+=(-n);
                        int K=n;
                        if (now_rt!=ptr->Tail)
                            while (now!=now_rt->Head&&K--) now=now->pre;
                        if (K<=0) return *this;
                        now_rt=now_rt->pre;
                        while (now_rt!=ptr->Head&&K>now_rt->size) 
                            K-=now_rt->size,now_rt=now_rt->pre;
                        if (now_rt==ptr->Head) 
                            throw invalid_iterator();
                        now=now_rt->Tail,K--;
                        while (K--) now=now->pre;
                        return *this; 
                    }
                    /**
                     * TODO iter++
                     */
                    const_iterator operator++(int) 
                    {
                        const_iterator tmp(*this);
                        this->operator+(1);
                        return tmp;
                    }
                    /**
                     * TODO ++iter
                     */
                    const_iterator& operator++() 
                    {
                        this->operator+=(1);
                        return *this;
                    }
                    /**
                     * TODO iter--
                     */
                    const_iterator operator--(int) 
                    {
                        const_iterator tmp(*this);
                        this->operator-(1);
                        return tmp;
                    }
                    /**
                     * TODO --iter
                     */
                    const_iterator& operator--() 
                    {
                        this->operator-=(1);
                        return *this;
                    }
                    /**
                     * TODO *it
                     */
                    const T& operator*() const 
                    {
                        if (now_rt==ptr->Tail) 
                            throw invalid_iterator();
                        return now->val;
                    }
                    /**
                     * TODO it->field
                     */
                    const T* operator->() const noexcept 
                    {
                        return &(this->now->val);
                    }
                    /**
                     * a operator to check whether two iterators are same (pointing to the same memory).
                     */
                    bool operator==(const iterator &rhs) const 
                    {
                        if (ptr!=rhs.ptr) return 0;
                        if (now_rt==ptr->Tail&&rhs.now_rt==ptr->Tail) return 1;
                        return now==rhs.now;
                    }
                    bool operator==(const const_iterator &rhs) const 
                    {
                        if (ptr!=rhs.ptr) return 0;
                        if (now_rt==ptr->Tail&&rhs.now_rt==ptr->Tail) return 1;
                        return now==rhs.now;
                    }
                    /**
                     * some other operator for iterator.
                     */
                    bool operator!=(const iterator &rhs) const 
                    {
                        if (ptr!=rhs.ptr) return 1;
                        if (now_rt==ptr->Tail&&rhs.now_rt==ptr->Tail) return 0;
                        return now!=rhs.now;
                    }
                    bool operator!=(const const_iterator &rhs) const 
                    {
                        if (ptr!=rhs.ptr) return 1;
                        if (now_rt==ptr->Tail&&rhs.now_rt==ptr->Tail) return 0;
                        return now!=rhs.now;
                    }
            };
            /**
             * TODO Constructors
             */
            deque() 
            {
                sumsize=blksize=0;
                Head=new root,Tail=new root;
                Head->nxt=Tail,Tail->pre=Head;
            }
            deque(const deque &other)                   //root:x rt node:o r
            {
                blksize=other.blksize;
                sumsize=other.sumsize; 
                Head=new root,Tail=new root;
                Head->nxt=Tail,Tail->pre=Head;
                root *x=Head;
                for(root *rt=other.Head->nxt;rt!=other.Tail;rt=rt->nxt)
                {
                    x->nxt=new root(rt->size,x,Tail);
                    x=x->nxt;
                    node *r=rt->Head;
                    x->Head=new node(r->val);
                    node *o=x->Head;
                    while (r!=rt->Tail)
                    {
                        r=r->nxt;
                        o->nxt=new node(r->val);
                        o->nxt->pre=o;
                        o=o->nxt;
                    }
                    x->Tail=o;
                }
                Tail->pre=x;
            }
            /**
             * TODO Deconstructor
             */
            ~deque() 
            {
                root *x=Head->nxt;
                while (x!=Tail)
                {
                    node *o=x->Head;
                    while (x->size--) o=del(o);
                    x=del(x);
                }
                delete Head;
                delete Tail;
            }
            /**
             * TODO assignment operator
             */
            deque &operator=(const deque &other) 
            {
                if (this!=&other)
                {
                    clear();
                    blksize=other.blksize;
                    sumsize=other.sumsize; 
                    root *x=Head;
                    for(root *rt=other.Head->nxt;rt!=other.Tail;rt=rt->nxt)
                    {
                        x->nxt=new root(rt->size,x,Tail);
                        x=x->nxt;
                        node *r=rt->Head;
                        x->Head=new node(r->val);
                        node *o=x->Head;
                        while (r!=rt->Tail)
                        {
                            r=r->nxt;
                            o->nxt=new node(r->val);
                            o->nxt->pre=o;
                            o=o->nxt;
                        }
                        x->Tail=o;
                    }
                    Tail->pre=x;
                }
                return *this;
            }
            /**
             * access specified element with bounds checking
             * throw index_out_of_bound if out of bound.
             */
            T & at(const size_t &pos) 
            {
                if (pos<0||pos>=sumsize)
                    throw index_out_of_bound();
                int K=pos;
                for(root *x=Head->nxt;x!=Tail;x=x->nxt)
                {
                    if (K<x->size)
                    {
                        node *o=x->Head;
                        while (K--) o=o->nxt;
                        return o->val;
                    } 
                    K-=x->size;
                }
            }
            const T & at(const size_t &pos) const 
            {
                if (pos<0||pos>=sumsize)
                    throw index_out_of_bound();
                int K=pos;
                for(root *x=Head->nxt;x!=Tail;x=x->nxt)
                {
                    if (K<x->size)
                    {
                        node *o=x->Head;
                        while (K--) o=o->nxt;
                        return o->val;
                    } 
                    K-=x->size;
                }
            }
            T & operator[](const size_t &pos) 
            {
                return at(pos);
            }
            const T & operator[](const size_t &pos) const 
            {
                return at(pos);
            }
            /**
             * access the first element
             * throw container_is_empty when the container is empty.
             */
            const T & front() const 
            {
                if (sumsize==0)
                    throw container_is_empty();
                return Head->nxt->Head->val;
            }
            /**
             * access the last element
             * throw container_is_empty when the container is empty.
             */
            const T & back() const 
            {
                if (sumsize==0)
                    throw container_is_empty();
                return Tail->pre->Tail->val;
            }
            /**
             * returns an iterator to the beginning.
             */
            iterator begin() 
            {
                return iterator(this,Head->nxt,Head->nxt->Head);
            }
            const_iterator cbegin() const 
            {
                return const_iterator(this,Head->nxt,Head->nxt->Head);
            }
            /**
             * returns an iterator to the end.
             */
            iterator end() 
            {
                return iterator(this,Tail);
            }
            const_iterator cend() const 
            {
                return const_iterator(this,Tail);
            }
            /**
             * checks whether the container is empty.
             */
            bool empty() const 
            {
                return sumsize==0;
            }
            /**
             * returns the number of elements
             */
            size_t size() const 
            {
                return sumsize;
            }
            /**
             * clears the contents
             */
            void clear() 
            {
                root *x=Head->nxt;
                while (x!=Tail)
                {
                    node *o=x->Head;
                    while (x->size--) o=del(o);
                    x=del(x);
                }
                sumsize=blksize=0;
                Head->nxt=Tail;
                Tail->pre=Head;       
            }
            /**
             * inserts elements at the specified locat on in the container.
             * inserts value before pos
             * returns an iterator pointing to the inserted value
             *     throw if the iterator is invalid or it point to a wrong place.
             */
            iterator insert(iterator pos, const T &value) 
            {
                if (pos.ptr!=this)
                    throw invalid_iterator();
                if (pos.now_rt==Tail)
                {
                    push_back(value);
                    return iterator(this,Tail->pre,Tail->pre->Tail);
                }
                int K=pos.K_th(sumsize);
                blksize=std::sqrt(++sumsize);
                if (pos.now==pos.now_rt->Head) 
                {
                    pos.now_rt->push_front(value,blksize);
                    return iterator(this,K);
                }
                node *tmp=new node(value,pos.now->pre,pos.now);
                pos.now->pre->nxt=tmp;
                pos.now->pre=tmp;
                pos.now_rt->size++;
                pos.now_rt->split(blksize);
                return iterator(this,K);
            }
            /**
             * removes specified element at pos.
             * removes the element at pos.
             * returns an iterator pointing to the following element, if pos pointing to the last element, end() will be returned.
             * throw if the container is empty, the iterator is invalid or it points to a wrong place.
             */
            iterator erase(iterator pos) 
            {
                sumsize--;
                if (pos.ptr!=this)
                    throw invalid_iterator();
                if (pos.now_rt==Head||pos.now_rt==Tail)
                    throw invalid_iterator();
                int K=pos.K_th(sumsize);
                if (pos.now==pos.now_rt->Head) 
                {
                    pos.now_rt->pop_front(blksize);
                    return iterator(this,K);
                }
                if (pos.now==pos.now_rt->Tail)
                {
                    pos.now_rt->pop_back(blksize);
                    return iterator(this,K);
                }
                del(pos.now);
                pos.now_rt->size--;
                pos.now_rt->merge(blksize);
                return iterator(this,K);
            }
            /**
             * adds an element to the end
             */
            void push_back(const T &value) 
            {
                blksize=std::sqrt(++sumsize);
                root *x=Tail->pre;
                if (x==Head)
                {
                    x=new root(0,Head,Tail);
                    Head->nxt=x;
                    Tail->pre=x;
                }
                x->push_back(value,blksize);  
            }
            /**
             * removes the last element
             *     throw when the container is empty.
             */
            void pop_back() 
            {
                if (sumsize==0) 
                    throw container_is_empty();
                Tail->pre->pop_back(blksize);
                sumsize--;
            }
            /**
             * inserts an element to the beginning.
             */
            void push_front(const T &value) 
            {
                blksize=std::sqrt(++sumsize);
                root *x=Head->nxt;
                if (x==Tail)
                {
                    x=new root(0,Head,Tail);
                    Head->nxt=x;
                    Tail->pre=x;
                }
                x->push_front(value,blksize);
            }
            /**
             * removes the first element.
             *     throw when the container is empty.
             */
            void pop_front() 
            {
                if (sumsize==0)
                    throw container_is_empty();
                Head->nxt->pop_front(blksize);
                sumsize--;
            }
    };

}

#endif
