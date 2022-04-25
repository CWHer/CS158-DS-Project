#include <functional>
#include <cstddef>
#include <cstring>
#include "exception.hpp"
#include "utility.hpp"

namespace sjtu
{
    template <class Key, class Value>
    class BTree
    {
    private:
        static const int BLOCK_SIZE = 1 << 12;

        FILE *file;
        char file_path[200];

        enum NodeType
        {
            Internal,
            Leaf,
            None,
        };
        // Your private members go here
        struct Node
        {
            static const int DATA_SIZE = 4000;
            static constexpr int MAX_M =
                DATA_SIZE / (sizeof(Key) + sizeof(int)) - 1;
            static const int MAX_L =
                DATA_SIZE / (sizeof(Key) + sizeof(Value)) - 1;
            // NOTE:
            //  MAX_M: max n_child in internal node
            //      (MAX_M - 1) / 2 <= n_key < MAX_M
            //  MAX_L: max n_key in leaf node
            //      (MAX_L + 1) / 2 <= n_key <= MAX_L

            int byte_offset;
            // >>>>> store in disk
            int prev_offset, succ_offset; // for sequential read
            NodeType node_type;
            int n_key;
            // NOTE: for Internal node
            //  1. n_child = n_key + 1
            //  2. child[k] <= key[k] < child[k + 1]
            //     "==" holds for some keys in child[k]
            char storage[DATA_SIZE];
            // <<<<< store in disk

        public:
            Node() { node_type = NodeType::None; }

            Node(NodeType node_type, int offset)
            {
                byte_offset = offset;
                prev_offset = succ_offset = -1;
                this->node_type = node_type;
                n_key = 0;
            }

            Node(FILE *file, int offset) { load(file, offset); }

            bool isOverflow()
            {
                return (node_type == NodeType::Leaf && n_key > MAX_L) ||
                       (node_type == NodeType::Internal && n_key >= MAX_M);
            }

            bool isUnderflow()
            {
                return (node_type == NodeType::Leaf && n_key < (MAX_L + 1) >> 1) ||
                       (node_type == NodeType::Internal && n_key < (MAX_M - 1) >> 1);
            }

            // load from file
            void load(FILE *file, int offset)
            {
                byte_offset = offset;
                fseek(file, offset, SEEK_SET);

                fread(&prev_offset, sizeof(int), 1, file);
                fread(&succ_offset, sizeof(int), 1, file);
                fread(&node_type, sizeof(NodeType), 1, file);
                fread(&n_key, sizeof(int), 1, file);
                fread(&storage, sizeof(char), DATA_SIZE, file);
            }

            // save to file
            void save(FILE *file)
            {
                fseek(file, byte_offset, SEEK_SET);

                fwrite(&prev_offset, sizeof(int), 1, file);
                fwrite(&succ_offset, sizeof(int), 1, file);
                fwrite(&node_type, sizeof(NodeType), 1, file);
                fwrite(&n_key, sizeof(int), 1, file);
                fwrite(&storage, sizeof(char), DATA_SIZE, file);
            }

            Key &key(int k)
            {
                return *(Key *)(storage + k * sizeof(Key));
            }
            int &child(int k)
            {
                if (node_type != NodeType::Internal)
                    throw runtime_error();
                return *(int *)(storage +
                                (MAX_M + 1) * sizeof(Key) +
                                k * sizeof(int));
            }
            Value &value(int k)
            {
                if (node_type != NodeType::Leaf)
                    throw runtime_error();
                return *(Value *)(storage +
                                  (MAX_L + 1) * sizeof(Key) +
                                  k * sizeof(Value));
            }

            // returns: k
            //  key[k - 1] < target_key <= key[k]
            int find(const Key &target_key)
            {
                int k = 0;
                while (k < n_key && target_key > key(k))
                    k++;
                return k;
            }

            // insert key to key[k] and child to child[k + b]
            // NOTE: "b" is either 0 or 1
            void insertChild(
                int k, int b, Key new_key, int child_offset)
            {
                child(n_key + 1) = child(n_key);
                for (int i = n_key; i > k; --i)
                {
                    key(i) = key(i - 1);
                    child(i + b) = child(i - 1 + b);
                }
                n_key++;
                key(k) = new_key;
                child(k + b) = child_offset;
            }

            // insert key to key[k] and value to value[k]
            void insertData(
                int k, Key new_key, Value new_value)
            {
                for (int i = n_key; i > k; --i)
                {
                    key(i) = key(i - 1);
                    value(i) = value(i - 1);
                }
                n_key++;
                key(k) = new_key;
                value(k) = new_value;
            }

            //  remove key[k] and child[k] / value[k]
            void remove(int k)
            {
                for (int i = k; i < n_key; ++i)
                {
                    key(i) = key(i + 1);
                    node_type == NodeType::Leaf
                        ? value(i) = value(i + 1)
                        : child(i) = child(i + 1);
                }
                n_key--;
            }
        };

        // >>>>> store in disk
        int current_offset;
        int root_offset;
        int seq_head, seq_tail;
        // <<<<< store in disk

    private:
        // lower_bound
        // returns: <is_found, <byte_offset, k>>
        pair<bool, pair<int, int>> find(
            int root_offset, const Key &key)
        {
            Node x(file, root_offset);
            while (x.node_type != NodeType::Leaf)
            {
                int k = x.find(key);
                x.load(file, x.child(k));
            }
            int k = x.find(key);
            // if k == x.n_key, this must be end()
            return pair<bool, pair<int, int>>(
                k != x.n_key && x.key(k) == key, pair<int, int>(x.byte_offset, k));
        }

        // >>>>> insert

        // split x into x & x->succ
        //  returns: <new_key, succ_offset>
        //  may change seq_tail
        pair<Key, int> split(Node &x)
        {
            Node succ(x.node_type,
                      current_offset += BLOCK_SIZE);

            if (x.node_type == NodeType::Leaf)
            {
                // link sequential node
                succ.prev_offset = x.byte_offset;
                succ.succ_offset = x.succ_offset;
                if (x.succ_offset != -1)
                {
                    Node t(file, x.succ_offset);
                    t.prev_offset = succ.byte_offset;
                    t.save(file);
                }
                x.succ_offset = succ.byte_offset;
                if (succ.succ_offset == -1)
                    seq_tail = succ.byte_offset;

                x.n_key = (Node::MAX_L + 1) >> 1;
                succ.n_key = Node::MAX_L + 1 - x.n_key;
                for (int i = 0; i < succ.n_key; ++i)
                {
                    succ.key(i) = x.key(x.n_key + i);
                    succ.value(i) = x.value(x.n_key + i);
                }
                x.save(file), succ.save(file);
                return pair<Key, int>(
                    x.key(x.n_key - 1), succ.byte_offset);
            }
            else
            {
                x.n_key = (Node::MAX_M - 1) >> 1;
                succ.n_key = Node::MAX_M - x.n_key - 1;
                for (int i = 0; i < succ.n_key; ++i)
                {
                    succ.key(i) = x.key(x.n_key + 1 + i);
                    succ.child(i) = x.child(x.n_key + 1 + i);
                }
                succ.child(succ.n_key) = x.child(Node::MAX_M);
                x.save(file), succ.save(file);
                return pair<Key, int>(
                    x.key(x.n_key), succ.byte_offset);
            }
        }

        // find() + solveOverflow() requires 2x read + 1x write
        //  However, recursive insert() only need 1x read + 1x write
        // returns: <key_not_found, <new_key, succ_offset>>
        pair<bool, pair<Key, int>> insert(
            int offset, const Key &key, const Value &value)
        {
            // NOTE: root is NOT handled.
            Node x(file, offset);

            if (x.node_type == NodeType::Leaf)
            {
                int k = x.find(key);
                if (k != x.n_key && x.key(k) == key)
                    return pair<bool, pair<Key, int>>(
                        false, pair<Key, int>(key, -1));

                x.insertData(k, key, value);
                if (!x.isOverflow())
                {
                    x.save(file);
                    return pair<bool, pair<Key, int>>(
                        true, pair<Key, int>(key, -1));
                }
                return pair<bool, pair<Key, int>>(true, split(x));
            }

            int k = x.find(key);
            pair<bool, pair<int, Key>>
                result = insert(x.child(k), key, value);
            if (!result.first ||
                result.second.second == -1)
                return result;

            x.insertChild(
                k, 1, result.second.first,
                result.second.second);
            if (!x.isOverflow())
            {
                x.save(file);
                return pair<bool, pair<Key, int>>(
                    true, pair<Key, int>(key, -1));
            }
            return pair<bool, pair<Key, int>>(true, split(x));
        }
        // <<<<< insert

        // >>>>> remove
        // rotate from left/right brother
        bool rotate(Node &x, int k,
                    Node &child, Node &left, Node &right)
        {
            if (left.node_type != NodeType::None)
            {
                left.n_key--;
                if (!left.isUnderflow())
                {
                    if (child.node_type == NodeType::Leaf)
                    {
                        child.insertData(
                            0, left.key(left.n_key), left.value(left.n_key));
                        x.key(k - 1) = left.key(left.n_key - 1);
                    }
                    else
                    {
                        child.insertChild(
                            0, 0, x.key(k - 1), left.child(left.n_key + 1));
                        x.key(k - 1) = left.key(left.n_key);
                    }
                    left.save(file);
                    child.save(file);
                    return true;
                }

                left.n_key++;
            }

            if (right.node_type != NodeType::None)
            {
                right.n_key--;
                if (!right.isUnderflow())
                {
                    if (child.node_type == NodeType::Leaf)
                    {
                        child.insertData(
                            child.n_key, right.key(0), right.value(0));
                        x.key(k) = child.key(child.n_key - 1);
                        right.n_key++, right.remove(0);
                    }
                    else
                    {
                        child.insertChild(
                            child.n_key, 1, x.key(k), right.child(0));
                        x.key(k) = right.key(0);
                        right.n_key++, right.remove(0);
                    }
                    right.save(file);
                    child.save(file);
                    return true;
                }

                right.n_key++;
            }

            return false;
        }

        // merge from left/right brother
        void merge(Node &x, int k,
                   Node &child, Node &left, Node &right)
        {
            if (left.node_type != NodeType::None)
            {
                // merge to left
                if (child.node_type == NodeType::Leaf)
                {
                    left.succ_offset = child.succ_offset;
                    if (left.succ_offset != -1)
                    {
                        // NOTE: succ is not always right
                        Node t(file, left.succ_offset);
                        t.prev_offset = left.byte_offset;
                        t.save(file);
                    }
                    if (left.succ_offset == -1)
                        seq_tail = left.byte_offset;

                    for (int i = 0; i < child.n_key; ++i)
                        left.insertData(left.n_key, child.key(i), child.value(i));
                    x.key(k - 1) = left.key(left.n_key - 1);
                    x.remove(k);
                }
                else
                {
                    for (int i = 0; i <= child.n_key; ++i)
                        left.insertChild(left.n_key, 1,
                                         i == 0
                                             ? x.key(k - 1)
                                             : child.key(i - 1),
                                         child.child(i));
                    x.key(k - 1) = x.key(k);
                    x.remove(k);
                }
                left.save(file);
                return;
            }

            if (right.node_type != NodeType::None)
            {
                // merge to right
                if (child.node_type == NodeType::Leaf)
                {
                    right.prev_offset = child.prev_offset;
                    if (right.prev_offset != -1)
                    {
                        // NOTE: prev is not always left
                        Node t(file, right.prev_offset);
                        t.succ_offset = right.byte_offset;
                        t.save(file);
                    }
                    if (right.prev_offset == -1)
                        seq_head = right.byte_offset;

                    for (int i = child.n_key - 1; i >= 0; --i)
                        right.insertData(0, child.key(i), child.value(i));
                    x.remove(k);
                }
                else
                {
                    for (int i = child.n_key; i >= 0; --i)
                        right.insertChild(0, 0,
                                          i == child.n_key
                                              ? x.key(k)
                                              : child.key(i),
                                          child.child(i));
                    x.remove(k);
                }
                right.save(file);
                return;
            }
        }

        // returns: <key_found, <child_node, max_key>>
        //  1. update
        //      if max key is removed, key[k] may change
        //      NOTE: max_key is not accurate if (key[k] != key)
        //  2. rotate
        //  3. merge
        pair<bool, pair<Node, Key>> remove(int offset, const Key &key)
        {
            // NOTE: root is NOT handled.
            Node x(file, offset);

            if (x.node_type == NodeType::Leaf)
            {
                int k = x.find(key);
                if (k == x.n_key || x.key(k) != key)
                    return pair<bool, pair<Node, Key>>(
                        false, pair<Node, Key>(x, key));

                x.remove(k);
                return pair<bool, pair<Node, Key>>(
                    true, pair<Node, Key>(x, x.key(x.n_key - 1)));
            }

            int k = x.find(key);
            pair<bool, pair<Node, Key>>
                result = remove(x.child(k), key);
            if (!result.first)
                return result;

            Node child = result.second.first;
            Key max_key = result.second.second;
            // >>>>> update
            //  key[k] may be removed
            if (k < x.n_key && x.key(k) == key)
                x.key(k) = max_key;

            // normal case
            if (!child.isUnderflow())
            {
                child.save(file);
                return pair<bool, pair<Node, Key>>(
                    true, pair<Node, Key>(x, max_key));
            }

            // underflow
            Node left, right;
            if (k - 1 >= 0)
                left.load(file, x.child(k - 1));
            if (k + 1 <= x.n_key)
                right.load(file, x.child(k + 1));
            // >>>>> rotate / merge
            if (!rotate(x, k, child, left, right))
                merge(x, k, child, left, right);

            return pair<bool, pair<Node, Key>>(
                true, pair<Node, Key>(x, max_key));
        }
        // <<<<< remove

    public:
        // DEBUG function
        void displayLeaf()
        {
            printf("[INFO]: ");
            for (auto it = begin(); it != end(); ++it)
                printf("%d ", it.getKey());
            printf("\n");
        }

        void print(int tab, int key)
        {
            while (tab--)
                putchar('\t');
            printf("%d\n", key);
        }

        void displayAll(int offset = -1, int tab = 0)
        {
            if (offset == -1)
            {
                offset = root_offset;
                printf("[INFO]: \n");
            }

            Node x(file, offset);
            if (x.node_type == NodeType::Leaf)
            {
                for (int i = 0; i < x.n_key; ++i)
                    print(tab, x.key(i));
                return;
            }
            for (int i = 0; i < x.n_key; ++i)
            {
                displayAll(x.child(i), tab + 1);
                print(tab, x.key(i));
            }
            displayAll(x.child(x.n_key), tab + 1);
        }

    public:
        BTree() : file_path("tree_data.bin")
        {
            file = fopen(file_path, "rb+");
            if (file != nullptr)
            {
                fseek(file, 0, SEEK_SET);
                fread(&current_offset, sizeof(int), 1, file);
                fread(&root_offset, sizeof(int), 1, file);
                fread(&seq_head, sizeof(int), 1, file);
                fread(&seq_tail, sizeof(int), 1, file);
                return;
            }

            file = fopen(file_path, "wb+");
            current_offset = root_offset = BLOCK_SIZE;
            seq_head = seq_tail = BLOCK_SIZE;
            Node(NodeType::Leaf, root_offset).save(file);
        }

        BTree(const char *fname)
        {
            strcpy(file_path, fname);
            file = fopen(file_path, "rb+");
            if (file != nullptr)
            {
                fseek(file, 0, SEEK_SET);
                fread(&current_offset, sizeof(int), 1, file);
                fread(&root_offset, sizeof(int), 1, file);
                fread(&seq_head, sizeof(int), 1, file);
                fread(&seq_tail, sizeof(int), 1, file);
                return;
            }

            file = fopen(file_path, "wb+");
            current_offset = root_offset = BLOCK_SIZE;
            seq_head = seq_tail = BLOCK_SIZE;
            Node(NodeType::Leaf, root_offset).save(file);
        }

        ~BTree()
        {
            fseek(file, 0, SEEK_SET);
            fwrite(&current_offset, sizeof(int), 1, file);
            fwrite(&root_offset, sizeof(int), 1, file);
            fwrite(&seq_head, sizeof(int), 1, file);
            fwrite(&seq_tail, sizeof(int), 1, file);

            fclose(file);
        }

        // Clear the BTree
        void clear()
        {
            fclose(file);
            file = fopen(file_path, "wb+");
            current_offset = root_offset = BLOCK_SIZE;
            seq_head = seq_tail = BLOCK_SIZE;
            Node(NodeType::Leaf, root_offset).save(file);
        }

        bool insert(const Key &key, const Value &value)
        {
            pair<bool, pair<Key, int>>
                result = insert(root_offset, key, value);
            if (result.second.second != -1)
            {
                // grow taller
                Node x(NodeType::Internal,
                       current_offset += BLOCK_SIZE);
                x.n_key = 1;
                x.key(0) = result.second.first;
                x.child(0) = root_offset;
                x.child(1) = result.second.second;
                root_offset = x.byte_offset;
                x.save(file);
            }
            return result.first;
        }

        bool modify(const Key &key, const Value &value)
        {
            pair<bool, pair<int, int>>
                result = find(root_offset, key);
            if (!result.first)
                return false;
            iterator it(this, result.second);
            it.modify(value);
            return true;
        }

        Value at(const Key &key)
        {
            pair<bool, pair<int, int>>
                result = find(root_offset, key);
            if (!result.first)
                return Value();
            iterator it(this, result.second);
            return it.getValue();
        }

        bool erase(const Key &key)
        {
            pair<bool, pair<Node, Key>>
                result = remove(root_offset, key);
            if (result.first)
            {
                Node root = result.second.first;
                if (root.n_key > 0)
                    root.save(file);
                if (root.n_key == 0 &&
                    root.node_type == NodeType::Internal)
                    root_offset = root.child(0);
            }
            return result.first;
        }

        class iterator
        {
            friend class BTree;

        private:
            // Your private members go here
            BTree<Key, Value> *tree_ptr;
            int offset, k;

        public:
            iterator() : tree_ptr(nullptr), offset(-1), k(-1) {}
            iterator(BTree<Key, Value> *tree_ptr, int offset, int k)
                : tree_ptr(tree_ptr), offset(offset), k(k) {}
            iterator(BTree<Key, Value> *tree_ptr, pair<int, int> loc)
                : tree_ptr(tree_ptr), offset(loc.first), k(loc.second) {}
            iterator(const iterator &other)
            {
                tree_ptr = other.tree_ptr;
                offset = other.offset;
                k = other.k;
            }

            // modify by iterator
            // HACK: cause UB if iterator is invalid
            bool modify(const Value &value)
            {
                Node x(tree_ptr->file, offset);
                x.value(k) = value;
                x.save(file);
                return true;
            }

            Key getKey() const
            {
                Node x(tree_ptr->file, offset);
                return x.key(k);
            }

            Value getValue() const
            {
                Node x(tree_ptr->file, offset);
                return x.value(k);
            }

            iterator operator++(int)
            {
                if (*this == tree_ptr->end())
                    throw invalid_iterator();

                iterator ret(*this);
                Node x(tree_ptr->file, offset);
                if (++k >= x.n_key &&
                    x.succ_offset != -1)
                    k = 0, offset = x.succ_offset;
                return ret;
            }

            iterator &operator++()
            {
                if (*this == tree_ptr->end())
                    throw invalid_iterator();

                Node x(tree_ptr->file, offset);
                if (++k >= x.n_key &&
                    x.succ_offset != -1)
                    k = 0, offset = x.succ_offset;
                return *this;
            }

            iterator operator--(int)
            {
                if (*this == tree_ptr->begin())
                    throw invalid_iterator();

                iterator ret(*this);
                Node x(tree_ptr->file, offset);
                if (--k < 0)
                {
                    offset = x.prev_offset;
                    Node prev(tree_ptr->file, offset);
                    k = x.n_key - 1;
                }
                return ret;
            }

            iterator &operator--()
            {
                if (*this == tree_ptr->begin())
                    throw invalid_iterator();

                Node x(tree_ptr->file, offset);
                if (--k < 0)
                {
                    offset = x.prev_offset;
                    Node prev(tree_ptr->file, offset);
                    k = x.n_key - 1;
                }
                return *this;
            }

            // Overloaded of operator '==' and '!='
            // Check whether the iterators are same
            bool operator==(const iterator &rhs) const
            {
                return tree_ptr == rhs.tree_ptr &&
                       offset == rhs.offset && k == rhs.k;
            }

            bool operator!=(const iterator &rhs) const
            {
                return !this->operator==(rhs);
            }
        };

        iterator begin()
        {
            return iterator(this, seq_head, 0);
        }

        // return an iterator to the end(the next element after the last)
        iterator end()
        {
            Node x(file, seq_tail);
            return iterator(this, seq_tail, x.n_key);
        }

        iterator find(const Key &key)
        {
            pair<bool, pair<int, int>>
                result = find(root_offset, key);
            if (!result.first)
                return end();
            return iterator(this, result.second);
        }

        // return an iterator whose key is the smallest key greater or equal than 'key'
        iterator lower_bound(const Key &key)
        {
            return iterator(this, find(root_offset, key).second);
        }
    };

} // namespace sjtu
