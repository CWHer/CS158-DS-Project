// B+Tree by Haoyi You
#include <functional>
#include <cstddef>
#include "exception.hpp"
#include <cstdio>
namespace sjtu
{
    constexpr int sizeofblock = 4096;
    template <class Key, class Value>
    class BTree
    {
    private:
        // Your private members go here

        struct node
        {
            char data[4096];
            bool *isleaf;
            int *currentsize;
            Key *keys;
            //两种节点共有的数据
            long *child;
            //非叶节点的数据
            Value *values;
            long *next, *ahead;
            //叶节点的数据
            node()
            {
                isleaf = (bool *)(&data[4095]);
                currentsize = (int *)(&data[0]);
                keys = (Key *)(&data[4]);
                const int length1 = (4096 - 5 - 2 * sizeof(long)) / (sizeof(long) + sizeof(Key));  //非叶节点
                const int length2 = (4096 - 5 - 2 * sizeof(long)) / (sizeof(Value) + sizeof(Key)); //叶节点
                child = (long *)(&data[4 + length1 * sizeof(Key)]);
                next = (long *)(&data[4 + length2 * sizeof(Key)]);
                ahead = (long *)(&data[4 + sizeof(long) + length2 * sizeof(Key)]);
                values = (Value *)(&data[4 + 2 * sizeof(long) + length2 * sizeof(Key)]);
                *next = *ahead = -1;
            }
            node(const node &other)
            {
                isleaf = (bool *)(&data[4095]);
                currentsize = (int *)(&data[0]);
                keys = (Key *)(&data[4]);
                const int length1 = (4096 - 5 - 2 * sizeof(long)) / (sizeof(long) + sizeof(Key));  //非叶节点
                const int length2 = (4096 - 5 - 2 * sizeof(long)) / (sizeof(Value) + sizeof(Key)); //叶节点
                child = (long *)(&data[4 + length1 * sizeof(Key)]);
                next = (long *)(&data[4 + length2 * sizeof(Key)]);
                ahead = (long *)(&data[4 + sizeof(long) + length2 * sizeof(Key)]);
                values = (Value *)(&data[4 + 2 * sizeof(long) + length2 * sizeof(Key)]);
                *isleaf = *(other.isleaf);
                for (int i = 0; i < *(other.currentsize); ++i)
                {
                    keys[i] = other.keys[i];
                    values[i] = other.values[i];
                }
                *currentsize = *(other.currentsize);
                *next = *(other.next);
                *ahead = *(other.ahead);
            }
            node operator=(const node &other)
            {
                *isleaf = *(other.isleaf);
                for (int i = 0; i < *(other.currentsize); ++i)
                {
                    keys[i] = other.keys[i];
                    values[i] = other.values[i];
                }
                *currentsize = *(other.currentsize);
                *next = *(other.next);
                *ahead = *(other.ahead);
                return *this;
            }
        };
        FILE *file;
        long int offset;
        char filename[100] = "let_me_ac_b+tree";
        //static int numberofnow;
        const int maxlength1, maxlength2; //1非叶，2叶子
    public:
        void write(node &abc, long size)
        {
            fseek(file, size * sizeofblock, SEEK_SET);
            fwrite(abc.data, 1, 4096, file);
        }
        void read(node &abc, long size)
        {
            fseek(file, size * sizeofblock, SEEK_SET);
            fread(abc.data, 1, sizeofblock, file);
        }

    private:
        void insert(const Key &key, const Value &value, long position)
        {
            static bool needchange = false;
            node tmp1;
            read(tmp1, position);
            if (*tmp1.isleaf)
            {
                int position1 = -1;
                int left = 0, right = *tmp1.currentsize, mid;
                while (left < right)
                {
                    mid = (left + right) >> 1;
                    if (tmp1.keys[mid] <= key)
                        left = mid + 1;
                    else
                        right = mid;
                }
                position1 = left - 1;
                if (position1 != -1 && tmp1.child[position1] == key)
                    throw 0;
                for (int i = *tmp1.currentsize; i > position1 + 1; --i)
                {
                    tmp1.keys[i] = tmp1.keys[i - 1];
                    tmp1.values[i] = tmp1.values[i - 1];
                }
                tmp1.values[position1 + 1] = value;
                tmp1.keys[position1 + 1] = key;
                ++(*tmp1.currentsize);
                if ((*tmp1.currentsize) == maxlength2)
                {
                    if (position == 1)
                    { //叶子是根而且满了
                        node newroot;
                        node anotherchild;
                        *newroot.isleaf = false;
                        *anotherchild.isleaf = true;
                        *newroot.currentsize = 2;
                        newroot.child[0] = 2; //接下来
                        newroot.child[1] = 3;
                        offset = 4;
                        *tmp1.next = 3;
                        *anotherchild.ahead = 2;
                        int mid = (*tmp1.currentsize) >> 1;
                        newroot.keys[0] = tmp1.keys[mid];
                        *anotherchild.currentsize = ((*tmp1.currentsize) - mid);
                        for (int i = 0; i < *anotherchild.currentsize; ++i)
                        {
                            anotherchild.keys[i] = tmp1.keys[mid + i];
                            anotherchild.values[i] = tmp1.values[mid + i];
                        }
                        *tmp1.currentsize = mid;
                        write(newroot, 1);
                        write(tmp1, 2);
                        write(anotherchild, 3);
                        return;
                    }
                    else
                    {
                        needchange = true;
                    }
                }
                else
                    needchange = false;
                write(tmp1, position);
            }
            else
            {
                int whichchild;
                if (key < tmp1.keys[0])
                    whichchild = 0;
                else
                {
                    int left = 0, right = *tmp1.currentsize - 1, mid;
                    while (left < right)
                    {
                        mid = (left + right) >> 1;
                        if (tmp1.keys[mid] <= key)
                            left = mid + 1;
                        else
                            right = mid;
                    }
                    whichchild = left; //每个key右边那个儿子是+1
                }
                insert(key, value, tmp1.child[whichchild]);
                if (needchange)
                {
                    node child1; //child1是读的那个儿子
                    read(child1, tmp1.child[whichchild]);
                    for (int i = *tmp1.currentsize; i > whichchild + 1; --i)
                    {
                        tmp1.child[i] = tmp1.child[i - 1];
                        tmp1.keys[i - 1] = tmp1.keys[i - 2];
                    } //child1右边的儿子向右移动
                    ++(*tmp1.currentsize);
                    int mid = *child1.currentsize >> 1;
                    node newchild; //newchild是新增的儿子
                    if (*child1.isleaf)
                    {                                             //儿子叶子节点满了
                        tmp1.keys[whichchild] = child1.keys[mid]; //前面0-mid-1儿子保留，第mid个拿出来，后面cur-mid-1放到下一个儿子
                        *newchild.isleaf = true;
                        *newchild.next = *child1.next;
                        tmp1.child[whichchild + 1] = offset;
                        *child1.next = offset; //offset是要存新节点的文件位置
                        *newchild.ahead = tmp1.child[whichchild];
                        *newchild.currentsize = *child1.currentsize - mid;
                        for (int i = 0; i < *newchild.currentsize; ++i)
                        {
                            newchild.keys[i] = child1.keys[i + mid];
                            newchild.values[i] = child1.values[i + mid];
                        }
                        *child1.currentsize = mid;
                    }
                    else
                    {                                                 //儿子非叶节点满了
                        tmp1.keys[whichchild] = child1.keys[mid - 1]; //前面0-mid-1儿子和0-mid-2的key保留，第mid-1(key)个拿出来，后面mid到cur-1儿子和mid到cur-2放到下一个儿子
                        *newchild.isleaf = false;
                        *newchild.currentsize = *child1.currentsize - mid;
                        tmp1.child[whichchild + 1] = offset;
                        newchild.child[0] = child1.child[mid];
                        for (int i = 1; i < (*newchild.currentsize); ++i)
                        {
                            newchild.child[i] = child1.child[i + mid];
                            newchild.keys[i - 1] = child1.keys[mid + i - 1];
                        }
                        *child1.currentsize = mid;
                    }
                    write(child1, tmp1.child[whichchild]);
                    write(newchild, offset);
                    ++offset;
                    if ((*tmp1.currentsize) >= maxlength1)
                    {
                        if (position == 1)
                        { //如果根满了
                            node newroot;
                            node anotherchild;
                            *newroot.isleaf = false;
                            *anotherchild.isleaf = false;
                            *newroot.currentsize = 2;
                            newroot.child[0] = offset; //接下来
                            newroot.child[1] = offset + 1;
                            int mid1 = (*tmp1.currentsize) >> 1;
                            newroot.keys[0] = tmp1.keys[mid1 - 1];
                            *anotherchild.currentsize = *tmp1.currentsize - mid1;
                            anotherchild.child[0] = tmp1.child[mid1];
                            for (int i = 1; i < (*anotherchild.currentsize); ++i)
                            {
                                anotherchild.child[i] = tmp1.child[i + mid1];
                                anotherchild.keys[i - 1] = tmp1.keys[mid1 + i - 1];
                            }
                            *tmp1.currentsize = mid1;
                            write(newroot, 1); //把新根写到0位置
                            write(tmp1, offset);
                            write(anotherchild, offset + 1); //再写那两个节点
                            offset += 2;
                            return;
                        }
                        else
                        {
                            needchange = true;
                        }
                    }
                    else
                        needchange = false;
                    write(tmp1, position);
                }
            }
        }
        void modify(const Key &key, const Value &value, long position)
        {
            node tmp1;
            read(tmp1, position);
            if (*tmp1.isleaf)
            {
                int left = 0, right = *tmp1.currentsize, mid;
                while (left < right)
                {
                    mid = (left + right) >> 1;
                    if (tmp1.keys[mid] <= key)
                        left = mid + 1;
                    else
                        right = mid;
                }
                int position1 = left - 1; //和key一样的
                if (tmp1.keys[position1] == key)
                {
                    tmp1.values[position1] = value;
                    write(tmp1, position);
                }
                else
                    throw 0;
            }
            else
            {
                int position1;
                if (key < tmp1.keys[0])
                {
                    position1 = 0;
                }
                else
                {
                    int left = 0, right = *tmp1.currentsize - 1, mid;
                    while (left < right)
                    {
                        mid = (left + right) >> 1;
                        if (tmp1.keys[mid] <= key)
                            left = mid + 1;
                        else
                            right = mid;
                    }
                    position1 = left;
                }
                modify(key, value, tmp1.child[position1]);
            }
        }
        void erase(const Key &key, long position)
        {
            static bool needchange = false;
            node tmp1;
            read(tmp1, position);
            if (*tmp1.isleaf)
            {
                int position1;
                int left = 0, right = *tmp1.currentsize, mid;
                while (left < right)
                {
                    mid = (left + right) >> 1;
                    if (tmp1.keys[mid] <= key)
                    {
                        left = mid + 1;
                    }
                    else
                    {
                        right = mid;
                    }
                }
                position1 = left - 1; //和key一样的
                if (position1 < 0 || tmp1.keys[position1] != key)
                    throw 0;
                --(*tmp1.currentsize);
                for (int i = position1; i < (*tmp1.currentsize); ++i)
                {
                    tmp1.keys[i] = tmp1.keys[i + 1];
                    tmp1.values[i] = tmp1.values[i + 1];
                }
                if (*tmp1.currentsize<(maxlength2)> > 1)
                {
                    if (position != 1)
                    { //根节点少了不动，其他的要动
                        needchange = true;
                    }
                }
                else
                    needchange = false;
                write(tmp1, position);
            }
            else
            {
                int whichchild;
                if (key < tmp1.keys[0])
                    whichchild = 0;
                else
                {
                    int left = 0, right = (*tmp1.currentsize) - 1, mid;
                    while (left < right)
                    {
                        mid = (left + right) >> 1;
                        if (tmp1.keys[mid] <= key)
                            left = mid + 1;
                        else
                            right = mid;
                    }
                    whichchild = left; //每个key右边那个儿子是+1
                }
                erase(key, tmp1.child[whichchild]);
                if (needchange)
                {
                    if (whichchild == 0)
                    { //如果是0就是向后借
                        node rent;
                        node nowchild;
                        read(rent, tmp1.child[whichchild + 1]);
                        read(nowchild, tmp1.child[whichchild]);
                        if (*(rent.isleaf))
                        { //合并叶子
                            if ((*rent.currentsize) > ((maxlength2 + 1) >> 1))
                            { //借儿子   5个左1开始处理，右大于3合并  6个左2开始处理，右大于3合并
                                tmp1.keys[whichchild] = rent.keys[1];
                                nowchild.keys[(*nowchild.currentsize)++] = rent.keys[0];
                                nowchild.values[(*nowchild.currentsize) - 1] = rent.values[0];
                                --(*rent.currentsize);
                                for (int i = 0; i < (*rent.currentsize); ++i)
                                {
                                    rent.keys[i] = rent.keys[i + 1];
                                    rent.values[i] = rent.values[i + 1];
                                }
                                write(nowchild, tmp1.child[whichchild]);
                                write(rent, tmp1.child[whichchild + 1]);
                            }
                            else
                            { //合并
                                --(*tmp1.currentsize);
                                for (int i = whichchild; i < *tmp1.currentsize - 1; ++i)
                                {
                                    tmp1.keys[i] = tmp1.keys[i + 1];
                                    tmp1.child[i + 1] = tmp1.child[i + 2];
                                }
                                for (int i = 0; i < (*rent.currentsize); ++i)
                                {
                                    nowchild.keys[i + (*nowchild.currentsize)] = rent.keys[i];
                                    nowchild.values[i + (*nowchild.currentsize)] = rent.values[i];
                                }
                                (*nowchild.currentsize) += (*rent.currentsize);
                                *nowchild.next = *rent.next;
                                write(nowchild, tmp1.child[whichchild]);
                            }
                        }
                        else
                        { //合并非叶子
                            if ((*rent.currentsize) > ((maxlength1 + 1) >> 1))
                            { //借儿子
                                nowchild.keys[*nowchild.currentsize - 1] = tmp1.keys[whichchild];
                                nowchild.child[(*nowchild.currentsize)++] = rent.child[0];
                                tmp1.keys[whichchild] = rent.keys[0];
                                --(*rent.currentsize);
                                for (int i = 0; i < (*rent.currentsize) - 1; ++i)
                                {
                                    rent.keys[i] = rent.keys[i + 1];
                                    rent.child[i] = rent.child[i + 1];
                                }
                                rent.child[(*rent.currentsize) - 1] = rent.child[(*rent.currentsize)];
                                write(nowchild, tmp1.child[whichchild]);
                                write(rent, tmp1.child[whichchild + 1]);
                            }
                            else
                            { //合并儿子
                                nowchild.keys[(*nowchild.currentsize) - 1] = tmp1.keys[whichchild];
                                nowchild.child[(*nowchild.currentsize)] = rent.child[0];
                                for (int i = 0; i < (*rent.currentsize) - 1; ++i)
                                {
                                    nowchild.keys[i + (*nowchild.currentsize)] = rent.keys[i];
                                    nowchild.child[i + (*nowchild.currentsize) + 1] = rent.child[i + 1];
                                }
                                (*nowchild.currentsize) += (*rent.currentsize);
                                --(*tmp1.currentsize);
                                for (int i = whichchild; i < (*tmp1.currentsize) - 1; ++i)
                                {
                                    tmp1.child[i + 1] = tmp1.child[i + 2];
                                    tmp1.keys[i] = tmp1.keys[i + 1];
                                }
                                write(nowchild, tmp1.child[whichchild]);
                            }
                        }
                    }
                    else
                    { //不是0就往前借
                        node rent;
                        node nowchild;
                        read(rent, tmp1.child[whichchild - 1]);
                        read(nowchild, tmp1.child[whichchild]);
                        if (*(rent.isleaf))
                        { //合并叶子
                            if ((*rent.currentsize) > ((maxlength2 + 1) >> 1))
                            { //借儿子   5个左1开始处理，右大于3合并  6个左2开始处理，右大于3合并
                                tmp1.keys[whichchild - 1] = rent.keys[(*rent.currentsize) - 1];
                                for (int i = (*nowchild.currentsize); i >= 1; --i)
                                {
                                    nowchild.keys[i] = nowchild.keys[i - 1];
                                    nowchild.values[i] = nowchild.values[i - 1];
                                }
                                nowchild.keys[0] = rent.keys[(*rent.currentsize) - 1];
                                nowchild.values[0] = rent.values[(*rent.currentsize) - 1];
                                ++(*nowchild.currentsize);
                                --(*rent.currentsize);
                                write(nowchild, tmp1.child[whichchild]);
                                write(rent, tmp1.child[whichchild - 1]);
                            }
                            else
                            { //合并
                                --(*tmp1.currentsize);
                                for (int i = whichchild - 1; i < *tmp1.currentsize - 1; ++i)
                                {
                                    tmp1.keys[i] = tmp1.keys[i + 1];
                                    tmp1.child[i + 1] = tmp1.child[i + 2];
                                }
                                for (int i = 0; i < (*nowchild.currentsize); ++i)
                                {
                                    rent.keys[i + (*rent.currentsize)] = nowchild.keys[i];
                                    rent.values[i + (*rent.currentsize)] = nowchild.values[i];
                                }
                                (*rent.currentsize) += (*nowchild.currentsize);
                                *rent.next = *nowchild.next;
                                write(rent, tmp1.child[whichchild - 1]);
                            }
                        }
                        else
                        { //合并非叶子
                            if ((*rent.currentsize) > ((maxlength1 + 1) >> 1))
                            { //借儿子
                                nowchild.child[*nowchild.currentsize] = nowchild.child[*nowchild.currentsize - 1];
                                for (int i = *nowchild.currentsize - 1; i > 0; --i)
                                {
                                    nowchild.child[i] = nowchild.child[i - 1];
                                    nowchild.keys[i] = nowchild.keys[i - 1];
                                }
                                ++(*nowchild.currentsize);
                                nowchild.keys[0] = tmp1.keys[whichchild - 1];
                                nowchild.child[0] = rent.child[(*rent.currentsize) - 1];
                                tmp1.keys[whichchild - 1] = rent.keys[(*rent.currentsize) - 2];
                                --(*rent.currentsize);
                                write(rent, tmp1.child[whichchild - 1]);
                                write(nowchild, tmp1.child[whichchild]);
                            }
                            else
                            { //合并儿子
                                rent.keys[(*rent.currentsize) - 1] = tmp1.keys[whichchild - 1];
                                rent.child[(*rent.currentsize)] = nowchild.child[0];
                                for (int i = 0; i < (*nowchild.currentsize) - 1; ++i)
                                {
                                    rent.keys[i + (*rent.currentsize)] = nowchild.keys[i];
                                    rent.child[i + (*rent.currentsize) + 1] = nowchild.child[i + 1];
                                }
                                (*rent.currentsize) += (*nowchild.currentsize);
                                --(*tmp1.currentsize);
                                for (int i = whichchild - 1; i < (*tmp1.currentsize) - 1; ++i)
                                {
                                    tmp1.child[i + 1] = tmp1.child[i + 2];
                                    tmp1.keys[i] = tmp1.keys[i + 1];
                                }
                                write(rent, tmp1.child[whichchild - 1]);
                            }
                        }
                    }
                }
                if (*tmp1.currentsize<(maxlength1)> > 1)
                {
                    if (position == 1)
                    {
                        if (*tmp1.currentsize == 1)
                        { //根节点个数小于
                            node tmp2;
                            read(tmp2, tmp1.child[0]);
                            write(tmp2, 1);
                            return;
                        }
                    }
                    else
                        needchange = true;
                }
                else
                    needchange = false;
                write(tmp1, position);
            }
        }

    public:
        BTree() : maxlength1((4096 - 5 - 2 * sizeof(long)) / (sizeof(long) + sizeof(Key))),
                  maxlength2((4096 - 5 - 2 * sizeof(long)) / (sizeof(Value) + sizeof(Key)))
        {
            offset = 2;
            //++numberofnow;
            //int add=numberofnow, position = 16;
            //while (add) {
            //    filename[position++] = 'a'+add % 10;
            //    add /= 10;
            //}
            //filename[position] = 0;//为了避免重名，将字符串16字节以后弄成字符串
            file = fopen(filename, "rb+");
            if (!file)
            {
                file = fopen(filename, "wb+");
                node tmp;
                *tmp.isleaf = true;
                *tmp.currentsize = 0;
                write(tmp, 1);
            }
            else
            {
                fseek(file, 0, SEEK_SET);
                fread(&offset, sizeof(long), 1, file);
            }
        }
        BTree(const char *fname) : maxlength1((4096 - 5 - 2 * sizeof(long)) / (sizeof(long) + sizeof(Key))),
                                   maxlength2((4096 - 5 - 2 * sizeof(long)) / (sizeof(Value) + sizeof(Key)))
        {
            file = fopen("fname", "rb+");
            if (!file)
            {
                file = fopen("fname", "wb+");
                offset = 2;
                node tmp;
                *tmp.isleaf = true;
                *tmp.currentsize = 0;
                write(tmp, 1);
            }
            else
            {
                fseek(file, 0, SEEK_SET);
                fread(&offset, sizeof(long), 1, file);
            }
        }
        ~BTree()
        {
            fseek(file, 0, SEEK_SET);
            fwrite(&offset, sizeof(long), 1, file);
            fclose(file);
        }
        // Clear the BTree
        void clear()
        {
            fclose(file);
            file = fopen(filename, "wb+");
            offset = 2;
            node tmp;
            *tmp.isleaf = true;
            *tmp.currentsize = 0;
            *tmp.next = -1;
            write(tmp, 1);
        }
        bool insert(const Key &key, const Value &value)
        {
            if (key == 61956)
            {
                puts("");
            }
            try
            {
                insert(key, value, 1);
            }
            catch (int i)
            {
                return false;
            }
            return true;
        }

        bool modify(const Key &key, const Value &value)
        {
            try
            {
                modify(key, value, 1);
            }
            catch (int i)
            {
                return false;
            }
            return true;
        }
        Value at(const Key &key)
        {
            long position = 1;
            node tmp1;
            int whichchild;
            read(tmp1, position);
            while (!(*tmp1.isleaf))
            {
                if (key < tmp1.keys[0])
                    whichchild = 0;
                else
                {
                    int left = 0, right = (*tmp1.currentsize) - 1, mid;
                    while (left < right)
                    {
                        mid = (left + right) >> 1;
                        if (tmp1.keys[mid] <= key)
                            left = mid + 1;
                        else
                            right = mid;
                    }
                    whichchild = left;
                }
                position = tmp1.child[whichchild];
                read(tmp1, position);
            }
            int left = 0, right = *(tmp1.currentsize), mid;
            while (left < right)
            {
                mid = (left + right) >> 1;
                if (tmp1.keys[mid] <= key)
                    left = mid + 1;
                else
                    right = mid;
            }
            whichchild = left - 1;

            if (whichchild >= 0 && tmp1.keys[whichchild] == key)
            {
                return tmp1.values[whichchild];
            }
            else
            {
                return Value();
            }
        }
        bool erase(const Key &key)
        {
            try
            {
                erase(key, 1);
            }
            catch (int i)
            {
                return false;
            }
            return true;
        }

        class iterator
        {
        private:
            BTree<Key, Value> *tree;
            node datas;
            int position;      //在节点中的position
            long leafposition; //叶子节点在文件中position
            friend class BTree;
            // Your private members go here
        public:
            iterator()
            {
                //leftposition = -1;
                position = 0;
                leafposition = 0;
            }
            iterator(const iterator &other)
            {
                position = other.position;
                leafposition = other.leafposition;
                tree = other.tree;
                datas = other.datas;
            }

            // modify by iterator
            bool modify(const Value &value)
            {
                node tmp;
                tree->read(tmp, leafposition);
                tmp.values[position] = value;
                tree->write(tmp, leafposition);
            }

            Key getKey() const
            {
                return datas.keys[position];
            }

            Value getValue() const
            {
                return datas.values[position];
            }

            iterator operator++(int)
            {
                iterator tmp1 = *this;
                ++position;
                if (*datas.currentsize == position)
                {
                    if (*datas.next != -1)
                    {
                        leafposition = *datas.next;
                        position = 0;
                        tree->read(datas, leafposition);
                    }
                }
                else
                {
                    if (*datas.currentsize < position)
                        throw index_out_of_bound();
                }
                return tmp1;
            }

            iterator &operator++()
            {
                iterator &tmp1 = *this;
                ++position;
                if (*datas.currentsize == position)
                {
                    if (*datas.next != -1)
                    {
                        leafposition = *datas.next;
                        position = 0;
                        tree->read(datas, leafposition);
                    }
                }
                else
                {
                    if (*datas.currentsize < position)
                        throw index_out_of_bound();
                }
                return tmp1;
            }
            iterator operator--(int)
            {
                iterator tmp1 = *this;
                --position;
                if (position < 0)
                {
                    if (*datas.ahead == -1)
                        throw index_out_of_bound();
                    else
                    {
                        leafposition = *datas.ahead;
                        tree->read(datas, leafposition);
                        position = (*datas.currentsize) - 1;
                    }
                }
                return tmp1;
            }

            iterator &operator--()
            {
                iterator &tmp1 = *this;
                --position;
                if (position < 0)
                {
                    if (*datas.ahead == -1)
                        throw index_out_of_bound();
                    else
                    {
                        leafposition = *datas.ahead;
                        tree->read(datas, leafposition);
                        position = (*datas.currentsize) - 1;
                    }
                }
                return tmp1;
            }

            // Overloaded of operator '==' and '!='
            // Check whether the iterators are same
            bool operator==(const iterator &rhs) const
            {
                if (tree == rhs.tree && position == rhs.position && leafposition == rhs.leafposition)
                    return true;
                else
                    return false;
            }

            bool operator!=(const iterator &rhs) const
            {
                if (tree == rhs.tree && position == rhs.position && leafposition == rhs.leafposition)
                    return false;
                else
                    return true;
            }
        };

        iterator begin()
        {
            iterator rturn;
            long p;
            read(rturn.datas, 1);
            while (!(*(rturn.datas).isleaf))
            {
                p = (rturn.datas).child[0];
                read((rturn.datas), p);
            }
            rturn.tree = this;
            rturn.leafposition = p;
            rturn.position = 0;
            return rturn;
        }

        // return an iterator to the end(the next element after the last)
        iterator end()
        {
            iterator rturn;
            long p;
            read((rturn.datas), 1);
            while (!(*(rturn.datas).isleaf))
            {
                p = (rturn.datas).child[(*(rturn.datas).currentsize) - 1];
                read((rturn.datas), p);
            }
            rturn.tree = this;
            rturn.leafposition = p;
            rturn.position = (*(rturn.datas).currentsize);
            return rturn;
        }

        iterator find(const Key &key)
        {
            iterator rturn;
            long position = 1;
            int whichchild;
            read((rturn.datas), position);
            while (!(*(rturn.datas).isleaf))
            {
                if (key < (rturn.datas).keys[0])
                    whichchild = 0;
                else
                {
                    int left = 0, right = (*(rturn.datas).currentsize) - 1, mid;
                    while (left < right)
                    {
                        mid = (left + right) >> 1;
                        if ((rturn.datas).keys[mid] <= key)
                            left = mid + 1;
                        else
                            right = mid;
                    }
                    whichchild = left;
                }
                position = (rturn.datas).child[whichchild];
                read((rturn.datas), position);
            }
            int left = 0, right = *((rturn.datas).currentsize), mid;
            while (left < right)
            {
                mid = (left + right) >> 1;
                if ((rturn.datas).keys[mid] <= key)
                    left = mid + 1;
                else
                    right = mid;
            }
            whichchild = left - 1;
            if (whichchild >= 0 && (rturn.datas).keys[whichchild] == key)
            {
                rturn.tree = this;
                rturn.leafposition = position;
                rturn.position = whichchild;
            }
            return rturn;
        }

        // return an iterator whose key is the smallest key greater or equal than 'key'
        iterator lower_bound(const Key &key)
        {
            long position = 1;
            iterator rturn;
            int whichchild;
            read((rturn.datas), position);
            while (!(*(rturn.datas).isleaf))
            {
                if (key < (rturn.datas).keys[0])
                    whichchild = 0;
                else
                {
                    int left = 0, right = (*(rturn.datas).currentsize) - 1, mid;
                    while (left < right)
                    {
                        mid = (left + right) >> 1;
                        if ((rturn.datas).keys[mid] <= key)
                            left = mid + 1;
                        else
                            right = mid;
                    }
                    whichchild = left;
                }
                position = (rturn.datas).child[whichchild];
                read((rturn.datas), position);
            }
            int left = 0, right = *((rturn.datas).currentsize), mid;
            while (left < right)
            {
                mid = (left + right) >> 1;
                if ((rturn.datas).keys[mid] <= key)
                    left = mid + 1;
                else
                    right = mid;
            }
            whichchild = left - 1;
            rturn.tree = this;
            rturn.leafposition = position;
            if (whichchild >= 0 && (rturn.datas).keys[whichchild] == key)
            {
                rturn.position = whichchild;
            }
            else
            {
                if (whichchild < 0)
                {
                    rturn.position = 0;
                }
                else
                {
                    ++whichchild;
                    if (whichchild == *(rturn.datas).currentsize)
                    {
                        if (*(rturn.datas).next == -1)
                            throw index_out_of_bound();
                        else
                        {
                            rturn.leafposition = *(rturn.datas).next;
                            rturn.position = 0;
                            rturn.tree->read(rturn.datas, rturn.leafposition);
                        }
                    }
                    else
                    {
                        rturn.position = whichchild;
                    }
                }
            }
            return rturn;
        }
    };
    //template <class Key, class Value>
    //int BTree<Key, Value>::numberofnow = 0;
} // namespace sjtu
