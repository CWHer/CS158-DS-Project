#include <functional>
#include <cstddef>
#include "exception.hpp"
#include "utility.hpp"
#include <cstring>
namespace sjtu 
{
    template <class Key, class Value>
    class BTree {
    private:
        static const int blocksize=4096;
        // static const int datasize=40;
        static const int datasize=4000;
        const int maxL,maxM;
        FILE *file;
        char filename[200];
        int cnt,rt;
        // Your private members go here
        struct node
        {
            int pos;
            int pre,nxt;
            bool isleaf;
            char data[4050];
            int sizeM,sizeL;    
            //sizeM=num(key)=num(ch)+1  (key,ch)  
            //sizeL=num(key)    (key,value)
            node()
            {
                pos=1;
                pre=nxt=-1;
                isleaf=1;
                sizeM=sizeL=0;
                memset(data,0,sizeof(data));
            }
            Key &key(int k) {return *(Key*)(data+k*sizeof(Key));}
            Value &value(int k,int maxL) {return *(Value*)(data+(maxL+1)*sizeof(Key)+k*sizeof(Value));}
            int &ch(int k,int maxM) {return *(int*)(data+(maxM+1)*sizeof(Key)+k*sizeof(int));}
            void insert_val(int k,Key _key,Value _value,int maxL)
            {
                ++sizeL;
                for(int i=sizeL-1;i>k;--i)
                {
                    key(i)=key(i-1);
                    value(i,maxL)=value(i-1,maxL);
                }
                key(k)=_key,value(k,maxL)=_value;
            }
            void insert_ch(int k,Key _key,int _pos,int maxM) //key:k,ch:k+1
            {
                ++sizeM;
                for(int i=sizeM-1;i>k;--i)
                {
                    key(i)=key(i-1);
                    ch(i+1,maxM)=ch(i,maxM);
                }
                key(k)=_key,ch(k+1,maxM)=_pos;
            }
            void erase_val(int k,int maxL)
            {
                --sizeL;
                for(int i=k;i<sizeL;++i)
                {
                    key(i)=key(i+1);
                    value(i,maxL)=value(i+1,maxL);
                }
            }
            void erase_ch(int k,int maxM)   //key:k,ch:k+1
            {
                --sizeM;
                for(int i=k;i<sizeM;++i)
                {
                    key(i)=key(i+1);
                    ch(i+1,maxM)=ch(i+2,maxM);
                }
            }
        };
        //for insert
        node prev;  //additional node 
        Key lastkey;    //for insert, last key from ch/for erase, mininum in erase leaf
        Key prekey;     //for erase, when move but not merge
        void split_node(node &x,node &t)
        {
            if (t.isleaf) 
            {
                for(int i=0;i<t.sizeL;++i)
                {
                    t.value(i,maxL)=x.value(i+x.sizeL,maxL);
                    t.key(i)=x.key(i+x.sizeL);  
                }
            }
            else
            {
                for(int i=0;i<t.sizeM;++i)
                {
                    t.key(i)=x.key(i+x.sizeM+1);  
                    t.ch(i,maxM)=x.ch(i+x.sizeM+1,maxM);  
                }
                t.ch(t.sizeM,maxM)=x.ch(t.sizeM+x.sizeM+1,maxM);
            }
        }
        void split(node &x,node &t)     //x>>t  
        {
            t.isleaf=x.isleaf;
            t.sizeM=t.sizeL=0;
            t.pos=++cnt;
            t.pre=x.pos,t.nxt=x.nxt;
            if (x.nxt!=-1)
            {
                node to;
                read_node(to,x.nxt);
                to.pre=cnt,write_node(to);
            }
            x.nxt=cnt;
            if (x.isleaf)
                t.sizeL=x.sizeL=(maxL+1)>>1;
            else 
                t.sizeM=x.sizeM=maxM/2;
            split_node(x,t);
        }
        void merge_node(node &x,node &t)
        {
            if (t.isleaf) 
            {
                for(int i=0;i<t.sizeL;++i)
                {
                    x.value(x.sizeL+i,maxL)=t.value(i,maxL);
                    x.key(x.sizeL+i)=t.key(i);  
                }
            }
            else
            {
                node to;
                read_node(to,t.ch(0,maxM));
                x.key(x.sizeM)=find_min(to);
                for(int i=0;i<t.sizeM;++i)
                {
                    x.key(x.sizeM+i+1)=t.key(i);
                    x.ch(x.sizeM+i+1,maxM)=t.ch(i,maxM);
                }
                x.ch(x.sizeM+t.sizeM+1,maxM)=t.ch(t.sizeM,maxM);
            }
        }
        void merge(node &x,node &t)     //x<<t
        {
            merge_node(x,t);
            x.nxt=t.nxt;
            if (x.nxt!=-1)
            {
                node to;
                read_node(to,x.nxt);
                to.pre=x.pos,write_node(to);
            }
            if (x.isleaf)
                x.sizeL+=t.sizeL;
            else 
                x.sizeM+=t.sizeM+1;
        }
        Key find_min(node x)
        {
            while (!x.isleaf)
                read_node(x,x.ch(0,maxM));
            return x.key(0);
        }
        void write_node(node &x)
        {
            int k=x.pos;
            fseek(file,k*blocksize,SEEK_SET);
            fwrite(&x.pre,sizeof(int),1,file);
            fwrite(&x.nxt,sizeof(int),1,file);
            fwrite(&x.isleaf,sizeof(bool),1,file);
            fwrite(&x.sizeL,sizeof(int),1,file);
            fwrite(&x.sizeM,sizeof(int),1,file);
            if (x.isleaf)
                fwrite(x.data,1,sizeof(Key)*(maxL+1)+sizeof(Value)*(maxL+1),file);
            else
                fwrite(x.data,1,sizeof(Key)*(maxM+1)+sizeof(int)*(maxM+1),file);
            
        }
        void read_node(node &x,int k)
        {
            x.pos=k;
            fseek(file,k*blocksize,SEEK_SET);
            fread(&x.pre,sizeof(int),1,file);
            fread(&x.nxt,sizeof(int),1,file);
            fread(&x.isleaf,sizeof(bool),1,file);
            fread(&x.sizeL,sizeof(int),1,file);
            fread(&x.sizeM,sizeof(int),1,file);
            memset(x.data,0,sizeof(x.data));
            if (x.isleaf)
                fread(x.data,1,sizeof(Key)*(maxL+1)+sizeof(Value)*(maxL+1),file);
            else
                fread(x.data,1,sizeof(Key)*(maxM+1)+sizeof(int)*(maxM+1),file);
        }
        pair<bool,pair<int,int>> find(int rt,const Key &key)
        {
            node x;
            read_node(x,rt);
            while (!x.isleaf)
            {
                int to=0;
                while (to<x.sizeM&&key>=x.key(to)) to++;
                read_node(x,x.ch(to,maxM));
            }
            int k=0;
            while (k<x.sizeL&&key>x.key(k)) k++;
            pair<int,int> ret(x.pos,k);
            if (k==x.sizeL||x.key(k)!=key) 
                return pair<bool,pair<int,int>> (0,ret);
            return pair<bool,pair<int,int>> (1,ret);
        }
        pair<bool,bool> insert(int pos,const Key &key, const Value &value) 
        {
            node x;
            read_node(x,pos);
            if (x.isleaf)
            {
                int k=0;
                while (k<x.sizeL&&key>x.key(k)) k++;
                if (k<x.sizeL&&x.key(k)==key) return pair<bool,bool>(0,0);
                x.insert_val(k,key,value,maxL);
                if (x.sizeL<=maxL)
                {
                    write_node(x);
                    return pair<bool,bool>(1,0);
                }
                split(x,prev);
                lastkey=prev.key(0);
                write_node(x),write_node(prev);
                return pair<bool,bool>(1,1);
            }
            int to=0;
            while (to<x.sizeM&&key>=x.key(to)) to++;
            pair<bool,bool> ret=insert(x.ch(to,maxM),key,value); 
            if (!ret.first||!ret.second) return ret;
            x.insert_ch(to,lastkey,prev.pos,maxM);
            if (x.sizeM<=maxM)
            {
                write_node(x);
                return pair<bool,bool>(1,0);
            }
            lastkey=x.key(maxM>>1);
            split(x,prev);
            write_node(x),write_node(prev);
            return pair<bool,bool>(1,1);
        }
        pair<bool,pair<int,int>> erase(int pos,bool _end,const Key &key)  //-1:none // 0:move l:1,r:0 // 1:merge lmerge:1 rmerge:0 
        {
            node x;
            read_node(x,pos);
            if (x.isleaf)
            {
                int k=0;
                while (k<x.sizeL&&key>x.key(k)) k++;
                if (k>=x.sizeL||x.key(k)!=key) return pair<bool,pair<int,int>> (0,pair<int,int>(-1,0));
                x.erase_val(k,maxL);
                lastkey=x.key(0);
                if (pos==rt||x.sizeL>=(maxL+1)/2)
                {
                    write_node(x);
                    return pair<bool,pair<int,int>> (1,pair<int,int>(-1,0));
                }
                if (!_end)
                {
                    node t;
                    read_node(t,x.nxt);
                    if (t.sizeL>(maxL+1)/2)
                    {
                        x.insert_val(x.sizeL,t.key(0),t.value(0,maxL),maxL);
                        t.erase_val(0,maxL);
                        write_node(x),write_node(t);
                        prekey=t.key(0);
                        return pair<bool,pair<int,int>> (1,pair<int,int>(0,0));
                    }
                    merge(x,t);
                    write_node(x);
                    return pair<bool,pair<int,int>> (1,pair<int,int>(1,0));
                }
                else
                {
                    node t;
                    read_node(t,x.pre);
                    if (t.sizeL>(maxL+1)/2)
                    {
                        x.insert_val(0,t.key(t.sizeL-1),t.value(t.sizeL-1,maxL),maxL);
                        t.erase_val(t.sizeL-1,maxL);
                        write_node(x),write_node(t);
                        prekey=x.key(0);
                        return pair<bool,pair<int,int>> (1,pair<int,int>(0,1));
                    }
                    merge(t,x);
                    write_node(t);
                    return pair<bool,pair<int,int>> (1,pair<int,int>(1,1));
                }
            }
            int to=0;
            while (to<x.sizeM&&key>=x.key(to)) to++;
            pair<bool,pair<int,int>> ret=erase(x.ch(to,maxM),to==x.sizeM,key);
            if (!ret.first) return ret;
            if (to>0&&x.key(to-1)==key) x.key(to-1)=lastkey;
            if (ret.second.first==-1) {write_node(x);return ret;}
            if (ret.second.first==0)
            {
                x.key(to-ret.second.second)=prekey;
                write_node(x);
                return pair<bool,pair<int,int>> (1,pair<int,int>(-1,0));
            }
            x.erase_ch(to-ret.second.second,maxM);
            if (x.sizeM>=maxM/2)
            {
                write_node(x);
                return pair<bool,pair<int,int>> (1,pair<int,int>(-1,0));
            }
            if (!_end)
            {
                node t;
                read_node(t,x.nxt);
                if (t.sizeM>maxM/2)
                {
                    node to;
                    read_node(to,t.ch(0,maxM));
                    x.insert_ch(x.sizeM,find_min(to),to.pos,maxM);
                    prekey=t.key(0);
                    t.ch(0,maxM)=t.ch(1,maxM),t.erase_ch(0,maxM);
                    write_node(x),write_node(t);
                    return pair<bool,pair<int,int>> (1,pair<int,int>(0,0));
                }
                merge(x,t);
                write_node(x);
                return pair<bool,pair<int,int>> (1,pair<int,int>(1,0));
            }
            else
            {
                node t;
                read_node(t,x.pre);
                if (t.sizeM>maxM/2)
                {
                    node to;
                    read_node(to,x.ch(0,maxM));
                    x.insert_ch(0,find_min(to),x.ch(0,maxM),maxM);
                    x.ch(0,maxM)=t.ch(t.sizeM,maxM);

                    prekey=t.key(t.sizeM-1);
                    t.erase_ch(t.sizeM-1,maxM);
                    write_node(x),write_node(t);
                    return pair<bool,pair<int,int>> (1,pair<int,int>(0,1));
                }
                merge(t,x);
                write_node(t);
                return pair<bool,pair<int,int>> (1,pair<int,int>(1,1));
            }
        }
    public:
        //to simplify, let maxM be even and let maxL be odd
        //also let there be extra capacity with maxM/L 
        BTree():maxM(((datasize/(sizeof(int)+sizeof(Key))-2)|1)^1),
            maxL((datasize/(sizeof(Value)+sizeof(Key))-2)|1),
            filename("b+_data")
        {
            file=fopen(filename,"rb+");
            if (!file)
            {
                file=fopen(filename,"wb+");
                node x;
                cnt=rt=1,write_node(x);
            }
            else 
            {
                fseek(file,0,SEEK_SET);
                fread(&rt,sizeof(int),1,file);
                fread(&cnt,sizeof(int),1,file);
            }
        }
        BTree(const char *fname):maxM(((datasize/(sizeof(int)+sizeof(Key))-2)|1)^1),
            maxL((datasize/(sizeof(Value)+sizeof(Key))-2)|1)
        {
            filename=fname;
            file=fopen(filename,"rb+");
            if (!file)
            {
                file=fopen(filename,"wb+");
                node x;
                cnt=rt=1,write_node(x);
            }
            else 
            {
                fseek(file,0,SEEK_SET);
                fread(&rt,sizeof(int),1,file);
                fread(&cnt,sizeof(int),1,file);
            }
        }
        ~BTree() 
        {
            fseek(file,0,SEEK_SET);
            fwrite(&rt,sizeof(int),1,file);
            fwrite(&cnt,sizeof(int),1,file);
            fclose(file);
        }
        // Clear the BTree
        void clear() 
        {
            fclose(file);
            file=fopen(filename,"wb+");
            node x;
            cnt=rt=1;
            write_node(x);
        }
        bool insert(const Key &key, const Value &value) 
        {

            pair<bool,bool> ret=insert(rt,key,value);
            if (ret.second)
            {
                node x;
                x.isleaf=0,x.pos=++cnt;
                x.sizeM=1;
                x.key(0)=lastkey;
                x.ch(0,maxM)=rt,x.ch(1,maxM)=prev.pos;
                rt=x.pos,write_node(x);
            }
            return ret.first;
        }
        bool modify(const Key &key, const Value &value) 
        {
            pair<bool,pair<int,int>> ret=find(rt,key);
            if (!ret.first) return 0;
            node x;
            read_node(x,ret.second.first);
            x.value(ret.second.second,maxL)=value;
            write(x);
            return 1;
        }
        Value at(const Key &key) 
        {
            pair<bool,pair<int,int>> ret=find(rt,key);
            if (!ret.first) return Value();
            node x;
            read_node(x,ret.second.first);
            return x.value(ret.second.second,maxL);
        }

        bool erase(const Key &key) 
        {
            node x;
            read_node(x,rt);
            if (x.isleaf) return erase(rt,0,key).first; 
            int to=0;
            while (to<x.sizeM&&key>=x.key(to)) to++;
            pair<bool,pair<int,int>> ret=erase(x.ch(to,maxM),to==x.sizeM,key); 
            if (!ret.first) return 0;
            if (to>0&&x.key(to-1)==key) x.key(to-1)=lastkey;
            if (ret.second.first==-1) {write_node(x);return 1;}
            if (ret.second.first==0)
            {
                x.key(to-ret.second.second)=prekey;
                write_node(x);
                return 1;
            }
            x.erase_ch(to-ret.second.second,maxM);
            if (x.sizeM>0) {write_node(x);return 1;}
            rt=x.ch(0,maxM);
            return 1;
        }
        
        
        class iterator 
        {
            friend class BTree;
            private:
                // Your private members go here
                BTree<Key, Value> *ptr;
                int pos,K;  //in file/ in node
            public:
                iterator()
                {
                    ptr=NULL,pos=K=0;
                }
                iterator(BTree<Key, Value> *_ptr):ptr(_ptr)
                {
                    pos=K=0;
                }
                iterator(const iterator& other) 
                {
                    ptr=other.ptr;
                    pos=other.pos;
                    K=other.K;
                }

                // modify by iterator
                bool modify(const Value& value) {
                    node x;
                    ptr->read_node(x,pos);
                    x.value(K,ptr->maxL)=value;
                    write(x);
                    return 1;
                }

                Key getKey() const 
                {
                    node x;
                    ptr->read_node(x,pos);
                    return x.key(K);
                }

                Value getValue() const 
                {
                    node x;
                    ptr->read_node(x,pos);
                    return x.value(K,ptr->maxL);
                }

                iterator operator++(int) 
                {
                    iterator ret(*this);
                    node x;
                    ptr->read_node(x,pos);
                    if (++K>=x.sizeL&&x.nxt!=-1) K=0,pos=x.nxt;
                    return ret;
                }

                iterator& operator++() 
                {
                    node x;
                    ptr->read_node(x,pos);
                    if (++K>=x.sizeL&&x.nxt!=-1) K=0,pos=x.nxt;
                    return *this;
                }
                iterator operator--(int) 
                {
                    iterator ret(*this);
                    node x;
                    ptr->read_node(x,pos);
                    if (--K<0) 
                    {
                        pos=x.pre;
                        ptr->read_node(x,pos);
                        K=x.sizeL-1;
                    }
                    return ret;
                }

                iterator& operator--() 
                {
                    node x;
                    ptr->read_node(x,pos);
                    if (--K<0) 
                    {
                        pos=x.pre;
                        ptr->read_node(x,pos);
                        K=x.sizeL-1;
                    }
                    return *this;   
                }

                // Overloaded of operator '==' and '!='
                // Check whether the iterators are same
                bool operator==(const iterator& rhs) const 
                {
                    if (ptr!=rhs.ptr) return 0;
                    if (pos!=rhs.pos||K!=rhs.K) return 0;
                    return 1; 
                }

                bool operator!=(const iterator& rhs) const 
                {
                    return !this->operator==(rhs);
                }
            };
            
            iterator begin() 
            {
                node x;
                read_node(x,rt);
                while (!x.isleaf)
                    read_node(x,x.ch(0,maxM));
                iterator ret(this);
                ret.pos=x.pos,ret.K=0;
                return ret;
            }
            
            // return an iterator to the end(the next element after the last)
            iterator end() 
            {
                node x;
                read_node(x,rt);
                while (!x.isleaf)
                    read_node(x,x.ch(x.sizeM,maxM));
                iterator ret(this);
                ret.pos=x.pos,ret.K=x.sizeL;
                return ret;
            }

            iterator find(const Key &key) 
            {
                pair<bool,pair<int,int>> pos=find(rt,key);
                if (!pos.first) throw index_out_of_bound();
                iterator ret(this);
                ret.pos=pos.second.first;
                ret.K=pos.second.second;
                return ret;
            }
            
            // return an iterator whose key is the smallest key greater or equal than 'key'
            iterator lower_bound(const Key &key) 
            {
                pair<bool,pair<int,int>> pos=find(rt,key);
                iterator ret(this);
                ret.pos=pos.second.first;
                ret.K=pos.second.second;
                node x;
                read_node(x,ret.pos);
                if (ret.K==x.sizeL&&ret!=end()) ++ret;
                while (ret!=end()&&ret.getKey()<key) ++ret;
                return ret;    
            }
        };
}  // namespace sjtu
