# 文件读写 for B+树

## by 刘成锴



## 打开文件

`FILE *fopen( const char * filename, const char * mode )`

### 两种二进制访问模式

#### 1. rb+

二进制打开一个文本文件，允许读写文件。

#### 2. wb+

二进制打开一个文本文件，允许读写文件。

如果文件已存在，则文件会被截断为零长度（清空文件内容）。

如果文件不存在，则会创建一个新文件。



## 移动文件指针

`int fseek ( FILE * stream, long int offset, int origin )`

offset：正号指针后移，负号指针前移，offset是从origin位置移动的字节数

* SEEK_SET 文件的开头
* SEEK_CUR 文件指针的当前位置
* SEEK_END 文件的末尾



## 二进制I/O函数

* `size_t fread(void *ptr, size_t size_of_elements, size_t number_of_elements, FILE *a_file)`
* `size_t fwrite(const void *ptr, size_t size_of_elements, size_t number_of_elements, FILE *a_file) `



## 关闭文件

`int fclose(FILE *fp)`



## Reference

* C文件读写和下方的笔记

https://www.runoob.com/cprogramming/c-file-io.html

* fseek的用法

https://www.runoob.com/cprogramming/c-function-fseek.html



## 例子：B+树

```cpp
// 将节点写入文件
void write_block(Node &cur) { //addr offset
            fseek(file, cur.addr, SEEK_SET);

            fwrite(&cur.next, sizeof(offset), 1, file);
            fwrite(&cur.keySize, sizeof(Rank), 1, file);
            fwrite(&cur.pointSize, sizeof(Rank), 1, file);
            fwrite(&cur.isLeaf, sizeof(bool), 1, file);

            if (cur.isLeaf) {
                fwrite(cur.info, 1, sizeof(key_t) * cur.keySize + sizeof(value_t) * cur.pointSize, file);
            }
            else {
                fwrite(cur.info, 1, sizeof(key_t) * cur.keySize + sizeof(offset) * cur.pointSize, file);
            }
        }

// 通过block的offset得到节点信息
void get_block(offset off, Node &ret) {
            ret.addr = off;
            fseek(file, off, SEEK_SET);

            fread(&ret.next, sizeof(offset), 1, file);
            fread(&ret.keySize, sizeof(Rank), 1, file);
            fread(&ret.pointSize, sizeof(Rank), 1, file);
            fread(&ret.isLeaf, sizeof(bool), 1, file);

            if (ret.isLeaf) {
                fread(ret.info, 1, sizeof(key_t) * ret.keySize + sizeof(value_t) * ret.pointSize, file);
            }
            else {
                fread(ret.info, 1, sizeof(key_t) * ret.keySize + sizeof(offset) * ret.pointSize, file);
            }
        }
```



