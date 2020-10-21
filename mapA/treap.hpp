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

namespace sjtu {

template<class Key,class T,class Compare = std::less<Key>> 
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
        Compare cmp;
        struct node
        {
            node *ch[2],*fa;
            int sz,val;
            const int opt;    //L norm R / 1 -1 0
            value_type *w;
            node (int _opt,int _val,node *_fa=NULL):fa(_fa),val(_val),opt(_opt)
            {
                sz=1;
                ch[0]=ch[1]=NULL;
                w=NULL;
            }
            node (int _opt,int _val,node *_fa,const value_type &value):fa(_fa),val(_val),opt(_opt)
            {
                sz=1;
                ch[0]=ch[1]=NULL;
                w=new value_type(value);
            }
            node (const node &other):sz(other.sz),val(other.val),opt(other.opt)
            {
                ch[0]=ch[1]=NULL;
                fa=NULL;
                if (other.w!=NULL)
                    w=new value_type(*(other.w));
                else w=NULL;
            }
            ~node()
            {
                if (w!=NULL) delete w;
            }
            // node (const int &_opt,const value_type &_w,node *_fa):opt(_opt){}
        };
        node *rt,*head,*tail;
        bool eq(value_type *&w,const Key &key) const
        {
            if (w==NULL) return 0;
            return !(cmp(w->first,key)||cmp(key,w->first));
        }
        void pushup(node *o) 
        {
            o->sz=1;
            for(int i=0;i<2;++i)
                if (o->ch[i]!=NULL) o->sz+=o->ch[i]->sz;
        }
        void dfs_destruct(node *o)
        {
            for(int i=0;i<2;++i)
                if (o->ch[i]!=NULL) dfs_destruct(o->ch[i]);
            delete o;
        }
        void dfs_construct(node *&o,node *fa,const node *other)
        {
            o=new node(*other);
            if (o->opt==1) head=o;
            if (o->opt==0) tail=o;
            o->fa=fa;
            for(int i=0;i<2;++i)
                if (other->ch[i]!=NULL) 
                    dfs_construct(o->ch[i],o,other->ch[i]);
        }
        void rotate(node *&y,int k) 
        {
            node *x=y->ch[k];
            x->fa=y->fa;
            y->ch[k]=x->ch[k^1];
            if (x->ch[k^1]!=NULL) x->ch[k^1]->fa=y;
            x->ch[k^1]=y,y->fa=x;
            pushup(y),pushup(x),y=x;
        }
        node *find(node *rt,const Key &key) const
        {
            node *o=rt;
            while (o!=NULL&&!eq(o->w,key)) 
            {
                int k=o->opt;
                if (k==-1) k=cmp(o->w->first,key);
                if (o->ch[k]==NULL) break;
                o=o->ch[k];
            }
            return o;
        }
        node *pre(node *o) const
        {
            if (o->ch[0]!=NULL)
            {
                o=o->ch[0];
                while (o->ch[1]!=NULL) o=o->ch[1];
                return o;
            }
            while (o->fa->ch[1]!=o) o=o->fa;
            return o->fa;
        }
        node *nxt(node *o) const
        {
            if (o->ch[1]!=NULL)
            {
                o=o->ch[1];
                while (o->ch[0]!=NULL) o=o->ch[0];
                return o;
            }
            while (o->fa->ch[0]!=o) o=o->fa;
            return o->fa;
        }
        pair<node *,bool> insert(node *&o,node *fa,const value_type &value)
        {
            if (o==NULL)
            {
                o=new node(-1,Rand(),fa,value);
                return pair<node *,bool>(o,1);
            }
            if (eq(o->w,value.first)) 
                return pair<node *,bool>(o,0);;
            int k=o->opt;
            if (k==-1) k=cmp(o->w->first,value.first);
            pair<node *,bool> ret=insert(o->ch[k],o,value);
            if (ret.second&&o->ch[k]->val<o->val) rotate(o,k);
            pushup(o);
            return ret;
        }
        bool del(node *&o,node *fa,const value_type &value)
        {
            if (o==NULL) return 0;
            if (eq(o->w,value.first))
            {
                if (o->ch[0]==NULL||o->ch[1]==NULL) 
                {
                    node *tmp=o;
                    o=o->ch[o->ch[0]==NULL];
                    if (o!=NULL) o->fa=fa;
                    delete tmp;
                    return 1;
                }
                int k=o->ch[0]->val<o->ch[1]->val;
                rotate(o,k^1);
                del(o->ch[k],o,value);   
                pushup(o);
                return 1;
            }
            int k=o->opt;
            if (k==-1) k=cmp(o->w->first,value.first);
            bool f=del(o->ch[k],o,value);
            if (f) pushup(o);
            return f;
        }
        int Rand()
        {
            static long long now=1;
            now=(now*13131+20000905)%(int)(1e9+7);    //100019
            return now;
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
            friend class map<Key,T,Compare>;
            private:
                /**
                 * TODO add data members
                 *   just add whatever you want.
                 */
                map<Key,T,Compare> *ptr;
                node *idx;
            public:
                iterator():ptr(NULL),idx(NULL){}
                iterator(map<Key,T,Compare> *_ptr,node *_idx):ptr(_ptr),idx(_idx){}
                iterator(const iterator &other) 
                {
                    ptr=other.ptr;
                    idx=other.idx;
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
                    if (idx->opt==0) throw invalid_iterator();
                    iterator ret(*this);
                    idx=this->ptr->nxt(idx);
                    return ret;
                }
                /**
                 * TODO ++iter
                 */
                iterator & operator++() 
                {
                    if (idx->opt==0) throw invalid_iterator();
                    idx=this->ptr->nxt(idx);
                    return *this;
                }
                /**
                 * TODO iter--
                 */
                iterator operator--(int) 
                {
                    if (idx->opt==1) throw invalid_iterator();
                    iterator ret(*this);
                    node *to=this->ptr->pre(idx);
                    if (to->opt==1) throw invalid_iterator();
                    idx=to;
                    return ret;
                }
                /**
                 * TODO --iter
                 */
                iterator & operator--() 
                {
                    if (idx->opt==1) throw invalid_iterator();
                    node *to=this->ptr->pre(idx);
                    if (to->opt==1) throw invalid_iterator();
                    idx=to;
                    return *this;
                }
                /**
                 * a operator to check whether two iterators are same (pointing to the same memory).
                 */
                value_type & operator*() const 
                {
                    return *(idx->w);
                }
                bool operator==(const iterator &rhs) const 
                {
                    return ptr==rhs.ptr&&idx==rhs.idx;
                }
                bool operator==(const const_iterator &rhs) const 
                {
                    return ptr==rhs.ptr&&idx==rhs.idx;
                }
                /**
                 * some other operator for iterator.
                 */
                bool operator!=(const iterator &rhs) const 
                {
                    return ptr!=rhs.ptr||idx!=rhs.idx;
                }
                bool operator!=(const const_iterator &rhs) const 
                {
                    return ptr!=rhs.ptr||idx!=rhs.idx;
                }

                /**
                 * for the support of it->first.
                 * See <http://kelvinh.github.io/blog/2013/11/20/overloading-of-member-access-operator-dash-greater-than-symbol-in-cpp/> for help.
                 */
                value_type* operator->() const noexcept 
                {
                    return idx->w;
                }
        };
        class const_iterator 
        {
            friend class map<Key,T,Compare>;
            // it should has similar member method as iterator.
            //  and it should be able to construct from an iterator.
            private:
                const map<Key,T,Compare> *ptr;
                node *idx;
                // data members.
            public:
                const_iterator():ptr(NULL),idx(NULL){}
                const_iterator(const map<Key,T,Compare> *_ptr,node *_idx):ptr(_ptr),idx(_idx){}
                const_iterator(const const_iterator &other) 
                {
                    ptr=other.ptr;
                    idx=other.idx;
                }
                const_iterator(const iterator &other) 
                {
                    ptr=other.ptr;
                    idx=other.idx;
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
                    if (idx->opt==0) throw invalid_iterator();
                    const_iterator ret(*this);
                    idx=this->ptr->nxt(idx);
                    return ret;
                }
                /**
                 * TODO ++iter
                 */
                const_iterator & operator++() 
                {
                    if (idx->opt==0) throw invalid_iterator();
                    idx=this->ptr->nxt(idx);
                    return *this;
                }
                /**
                 * TODO iter--
                 */
                const_iterator operator--(int) 
                {
                    if (idx->opt==1) throw invalid_iterator();
                    const_iterator ret(*this);
                    node *to=this->ptr->pre(idx);
                    if (to->opt==1) throw invalid_iterator();
                    idx=to;
                    return ret;
                }
                /**
                 * TODO --iter
                 */
                const_iterator & operator--() 
                {
                    if (idx->opt==1) throw invalid_iterator();
                    node *to=this->ptr->pre(idx);
                    if (to->opt==1) throw invalid_iterator();
                    idx=to;
                    return *this;
                }
                /**
                 * a operator to check whether two iterators are same (pointing to the same memory).
                 */
                const value_type & operator*() const 
                {
                    return *(idx->w);
                }
                bool operator==(const iterator &rhs) const 
                {
                    return ptr==rhs.ptr&&idx==rhs.idx;
                }
                bool operator==(const const_iterator &rhs) const 
                {
                    return ptr==rhs.ptr&&idx==rhs.idx;
                }
                /**
                 * some other operator for iterator.
                 */
                bool operator!=(const iterator &rhs) const 
                {
                    return ptr!=rhs.ptr||idx!=rhs.idx;
                }
                bool operator!=(const const_iterator &rhs) const 
                {
                    return ptr!=rhs.ptr||idx!=rhs.idx;
                }

                /**
                 * for the support of it->first.
                 * See <http://kelvinh.github.io/blog/2013/11/20/overloading-of-member-access-operator-dash-greater-than-symbol-in-cpp/> for help.
                 */
                const value_type* operator->() const noexcept 
                {
                    return idx->w;
                }
        };
        /**
         * TODO two constructors
         */
        map() 
        {
            head=rt=new node(1,Rand());
            tail=rt->ch[1]=new node(0,Rand(),rt);
            pushup(rt);
            if (rt->ch[1]->val<rt->val) rotate(rt,1);
        }
        map(const map &other) 
        {
            dfs_construct(rt,NULL,other.rt);
        }
        /**
         * TODO assignment operator
         */
        map & operator=(const map &other) 
        {
            if (this!=&other)
            {
                dfs_destruct(rt);
                dfs_construct(rt,NULL,other.rt);
            }
            return *this;
        }
        /**
         * TODO Destructors
         */
        ~map() 
        {
            dfs_destruct(rt);
        }
        /**
         * TODO
         * access specified element with bounds checking
         * Returns a reference to the mapped value of the element with key equivalent to key.
         * If no such element exists, an exception of type `index_out_of_bound'
         */
        T & at(const Key &key) 
        {
            node *x=find(rt,key);
            if (!eq(x->w,key)) throw index_out_of_bound();
            return x->w->second;
        }
        const T & at(const Key &key) const 
        {
            node *x=find(rt,key);
            if (!eq(x->w,key)) throw index_out_of_bound();
            return x->w->second;
        }
        /**
         * TODO
         * access specified element
         * Returns a reference to the value that is mapped to a key equivalent to key,
         *   performing an insertion if such key does not already exist.
         */
        T & operator[](const Key &key) 
        {
            node *x=find(rt,key);
            if (!eq(x->w,key)) 
                return insert(rt,NULL,value_type(key,T())).first->w->second;
            return x->w->second;
        }
        /**
         * behave like at() throw index_out_of_bound if such key does not exist.
         */
        const T & operator[](const Key &key) const 
        {
            return at(key);
        }
        /**
         * return a iterator to the beginning
         */
        iterator begin() 
        {
            return iterator(this,nxt(head));
        }
        const_iterator cbegin() const 
        {
            return const_iterator(this,nxt(head));
        }
        /**
         * return a iterator to the end
         * in fact, it returns past-the-end.
         */
        iterator end() 
        {
            return iterator(this,tail);
        }
        const_iterator cend() const 
        {
            return const_iterator(this,tail);
        }
        /**
         * checks whether the container is empty
         * return true if empty, otherwise false.
         */
        bool empty() const 
        {
            return rt->sz==2;
        }
        /**
         * returns the number of elements.
         */
        size_t size() const 
        {
            return rt->sz-2;
        }
        /**
         * clears the contents
         */
        void clear() 
        {
            dfs_destruct(rt);
            head=rt=new node(1,Rand());
            tail=rt->ch[1]=new node(0,Rand(),rt);
            pushup(rt);
            if (rt->ch[1]->val<rt->val) rotate(rt,1);
        }
        /**
         * insert an element.
         * return a pair, the first of the pair is
         *   the iterator to the new element (or the element that prevented the insertion),
         *   the second one is true if insert successfully, or false.
         */
        pair<iterator, bool> insert(const value_type &value) 
        {
            pair<node *,bool> pos=insert(rt,NULL,value);
            return pair<iterator,bool> (iterator(this,pos.first),pos.second);
        }
        /**
         * erase the element at pos.
         *
         * throw if pos pointed to a bad element (pos == this->end() || pos points an element out of this)
         */
        void erase(iterator pos) 
        {
            if (pos.ptr!=this) throw invalid_iterator();
            if (pos.idx->opt!=-1) throw invalid_iterator(); 
            if (!del(rt,NULL,*(pos.idx->w))) throw invalid_iterator();
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
            node *x=find(rt,key);          
            return eq(x->w,key);
        }
        /**
         * Finds an element with key equivalent to key.
         * key value of the element to search for.
         * Iterator to an element with key equivalent to key.
         *   If no such element is found, past-the-end (see end()) iterator is returned.
         */
        iterator find(const Key &key) 
        {
            node *x=find(rt,key);
            if (!eq(x->w,key)) return end();
            return iterator(this,x);
        }
        const_iterator find(const Key &key) const 
        {
            node *x=find(rt,key);
            if (!eq(x->w,key)) return cend();
            return const_iterator(this,x);
        }
};

}

#endif
