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
            int sz,dep;
            const int opt;    //L norm R / 1 -1 0
            value_type *w;
            node (int _opt,int _dep,node *_fa=NULL):fa(_fa),dep(_dep),opt(_opt)
            {
                sz=1;
                ch[0]=ch[1]=NULL;
                w=NULL;
            }
            node (int _opt,int _dep,node *_fa,const value_type &value):fa(_fa),dep(_dep),opt(_opt)
            {
                sz=1;
                ch[0]=ch[1]=NULL;
                w=new value_type(value);
            }
            node (const node &other):sz(other.sz),dep(other.dep),opt(other.opt)
            {
                ch[0]=ch[1]=NULL;
                fa=NULL;
                if (other.w!=NULL)
                    w=new value_type(*(other.w));
                else w=NULL;
            }
            ~node()
            {
                delete w;
            }
            // node (const int &_opt,const value_type &_w,node *_fa):opt(_opt){}
        };
        node *rt,*head,*tail;
        int depth(node *x){return x==NULL?0:x->dep;}
        bool eq(value_type *&w,const Key &key) const
        {
            if (w==NULL) return 0;
            return !(cmp(w->first,key)||cmp(key,w->first));
        }
        void pushup(node *o) 
        {
            o->sz=o->dep=1;
            for(int i=0;i<2;++i)
                if (o->ch[i]!=NULL) 
                {
                    o->sz+=o->ch[i]->sz;
                    if (o->ch[i]->dep+1>o->dep) o->dep=o->ch[i]->dep+1;
                }
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
        node *pre(const Key &key) const
        {
            node *o=rt,*ret=NULL;
            while (o!=NULL)
            {
                int k=o->opt;
                if (k==-1) k=cmp(o->w->first,key)&&!eq(o->w,key); //val>=key -> 0
                if (k==1)
                    if (ret==NULL||ret->w==NULL
                        ||(o->w!=NULL&&cmp(ret->w->first,o->w->first))) ret=o;
                if (o->ch[k]==NULL) break;
                o=o->ch[k];
            }
            return ret;
        }
        node *nxt(const Key &key) const
        {
            node *o=rt,*ret=NULL;
            while (o!=NULL)
            {
                int k=o->opt;
                if (k==-1) k=cmp(o->w->first,key)||eq(o->w,key); //val<=key -> 1
                if (k==0)
                    if (ret==NULL||ret->w==NULL
                        ||(o->w!=NULL&&cmp(o->w->first,ret->w->first))) ret=o;
                if (o->ch[k]==NULL) break;
                o=o->ch[k];
            }
            return ret;
        }
        bool modify(node *&o,int k)
        {
            if (depth(o->ch[k])-depth(o->ch[k^1])==1) return 0;
            if (depth(o->ch[k])==depth(o->ch[k^1])) return 1;
            node *&nxt=o->ch[k];
            int ks=depth(nxt->ch[0])<depth(nxt->ch[1]);
            if (depth(nxt->ch[0])!=depth(nxt->ch[1])&&(k^ks)) 
            {
                rotate(nxt,ks),rotate(o,k);
                return 1;
            }
            rotate(o,k);
            return depth(o->ch[0])==depth(o->ch[1]);
        }
        pair<node *,bool> insert(node *&o,node *fa,const value_type &value)
        {
            if (o==NULL)
            {
                o=new node(-1,1,fa,value);
                return pair<node *,bool>(o,1);
            }
            if (eq(o->w,value.first)) 
                return pair<node *,bool>(o,0);
            int k=o->opt,ks;
            if (k==-1) k=cmp(o->w->first,value.first);
            pair<node *,bool> ret=insert(o->ch[k],o,value);
            if (depth(o->ch[k])-depth(o->ch[k^1])==2) 
            {
                node *&nxt=o->ch[k];
                ks=depth(nxt->ch[0])<depth(nxt->ch[1]);
                if (k^ks) rotate(nxt,ks);
                rotate(o,k);
            }
            pushup(o);
            return ret;
        }
        pair<bool,bool> del(node *&o,node *fa,const value_type &value)
        {
            if (o==NULL) return pair<bool,bool>(0,0);
            if (eq(o->w,value.first))
            {
                if (o->ch[0]==NULL||o->ch[1]==NULL) 
                {
                    node *tmp=o;
                    o=o->ch[o->ch[0]==NULL];
                    if (o!=NULL) o->fa=fa;
                    delete tmp;
                    return pair<bool,bool>(1,1);
                }
                node *x=o->ch[1];
                while (x->ch[0]!=NULL) x=x->ch[0];
                node *tmp=x->ch[1];
                x->fa->ch[x->fa->ch[1]==x]=o;
                x->fa=fa;
                for(int i=0;i<2;++i)
                    if (o->ch[i]!=NULL)
                        x->ch[i]=o->ch[i],o->ch[i]->fa=x;
                o->ch[1]=tmp,o->ch[0]=NULL,o=x;
                pair<bool,bool> ret=del(o->ch[1],o,value);
                pushup(o);
                if (!ret.first) return ret; 
                return pair<bool,bool>(modify(o,0),1);
            }
            int k=o->opt,ks;
            if (k==-1) k=cmp(o->w->first,value.first);
            pair<bool,bool> ret=del(o->ch[k],o,value);
            pushup(o);
            if (!ret.first) return ret; 
            return pair<bool,bool>(modify(o,k^1),1);
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
                    idx=this->ptr->nxt(idx->w->first);
                    return ret;
                }
                /**
                 * TODO ++iter
                 */
                iterator & operator++() 
                {
                    if (idx->opt==0) throw invalid_iterator();
                    idx=this->ptr->nxt(idx->w->first);
                    return *this;
                }
                /**
                 * TODO iter--
                 */
                iterator operator--(int) 
                {
                    if (idx->opt==1) throw invalid_iterator();
                    iterator ret(*this);
                    node *x=idx;
                    if (idx->w==NULL)       //end
                    {
                        if (x->ch[0]!=NULL)
                        {
                            x=x->ch[0];
                            while (x->ch[1]!=NULL) x=x->ch[1];
                        }
                        else x=x->fa;
                    }
                    else x=this->ptr->pre(idx->w->first);
                    if (x->opt==1) throw invalid_iterator();
                    idx=x;
                    return ret;
                }
                /**
                 * TODO --iter
                 */
                iterator & operator--() 
                {
                    if (idx->opt==1) throw invalid_iterator();
                    node *x=idx;
                    if (idx->w==NULL)       //end
                    {
                        if (x->ch[0]!=NULL)
                        {
                            x=x->ch[0];
                            while (x->ch[1]!=NULL) x=x->ch[1];
                        }
                        else x=x->fa;
                    }
                    else x=this->ptr->pre(idx->w->first);
                    if (x->opt==1) throw invalid_iterator();
                    idx=x;
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
                    idx=this->ptr->nxt(idx->w->first);
                    return ret;
                }
                /**
                 * TODO ++iter
                 */
                const_iterator & operator++() 
                {
                    if (idx->opt==0) throw invalid_iterator();
                    idx=this->ptr->nxt(idx->w->first);
                    return *this;
                }
                /**
                 * TODO iter--
                 */
                const_iterator operator--(int) 
                {
                    if (idx->opt==1) throw invalid_iterator();
                    const_iterator ret(*this);
                    node *x=idx;
                    if (idx->w==NULL)       //end
                    {
                        if (x->ch[0]!=NULL)
                        {
                            x=x->ch[0];
                            while (x->ch[1]!=NULL) x=x->ch[1];
                        }
                        else x=x->fa;
                    }
                    else x=this->ptr->pre(idx->w->first);
                    if (x->opt==1) throw invalid_iterator();
                    idx=x;
                    return ret;
                }
                /**
                 * TODO --iter
                 */
                const_iterator & operator--() 
                {
                    if (idx->opt==1) throw invalid_iterator();
                    node *x=idx;
                    if (idx->w==NULL)       //end
                    {
                        if (x->ch[0]!=NULL)
                        {
                            x=x->ch[0];
                            while (x->ch[1]!=NULL) x=x->ch[1];
                        }
                        else x=x->fa;
                    }
                    else x=this->ptr->pre(idx->w->first);
                    if (x->opt==1) throw invalid_iterator();
                    idx=x;
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
            head=rt=new node(1,1);
            tail=rt->ch[1]=new node(0,1,rt);
            pushup(rt);
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
			if (empty()) return end();
            node *o=head;
            if (o->ch[1]!=NULL)
            {
                o=o->ch[1];
                while (o->ch[0]!=NULL) o=o->ch[0];
                return iterator(this,o);
            }
            return iterator(this,o->fa);
        }
        const_iterator cbegin() const 
        {
            if (empty()) return cend();
            node *o=head;
            if (o->ch[1]!=NULL)
            {
                o=o->ch[1];
                while (o->ch[0]!=NULL) o=o->ch[0];
                return const_iterator(this,o);
            }
            return const_iterator(this,o->fa);
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
            head=rt=new node(1,1);
            tail=rt->ch[1]=new node(0,1,rt);
            pushup(rt);
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
            if (!del(rt,NULL,*(pos.idx->w)).second) throw invalid_iterator();
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
        int dfs(node *o)
        {
            if (o==NULL) return 0;
            int ret=0;
            ret+=dfs(o->ch[0]);
            ret+=dfs(o->ch[1]);
            return ret+o->dep;
        }
        int ss()
        {
            return dfs(rt);
        }
};

}

#endif