#include<cstdio>
const int INF=1<<30,N=100050;
int n;
inline int read()
{
    register int x=0,t=1;
    register char ch=getchar();
    while ((ch<'0'||ch>'9')&&ch!='-') ch=getchar();
    if (ch=='-') {t=-1;ch=getchar();}
    while (ch>='0'&&ch<='9') {x=x*10+ch-48;ch=getchar();}
    return x*t;
}
namespace Splay
{
    int rt=0,cnt=0;
    struct node {int ch[2],fa,rec,val,sz;}t[N];
    void calc(int o) {t[o].sz=t[t[o].ch[0]].sz+t[t[o].ch[1]].sz+t[o].rec;}
    void rotate(int x)
    {
        int y=t[x].fa,z=t[y].fa,k=t[y].ch[1]==x;
        t[z].ch[t[z].ch[1]==y]=x,t[x].fa=z;
        t[y].ch[k]=t[x].ch[k^1],t[t[x].ch[k^1]].fa=y;
        t[x].ch[k^1]=y,t[y].fa=x;	
        calc(y),calc(x);
    }
    void splay(int x,int to)
    {
        while (t[x].fa!=to)
        {
            int y=t[x].fa,z=t[y].fa;
            if (z!=to) (t[y].ch[0]==x)^(t[z].ch[0]==y)?rotate(x):rotate(y);
            rotate(x);		
        }
        if (!to) rt=x;
    }
    void ins(int &o,int fa,int x)
    {
        if (!o)
        {
            o=++cnt; 
            t[o].val=x,t[o].fa=fa;
            t[o].sz=t[o].rec=1;	
            splay(o,0);
            return;
        }
        t[o].sz++;
        if (x==t[o].val) {t[o].rec++;return;}
        ins(t[o].ch[x>t[o].val],o,x);
    }
    void find(int x)
    {
        int o=rt;
        while (t[o].ch[x>t[o].val]&&x!=t[o].val) o=t[o].ch[x>t[o].val];
        splay(o,0);
    }
    int next(int x,int opt)
    {
        find(x);
        if ((t[rt].val>x&&opt)||(t[rt].val<x&&!opt)) return rt;
        int o=t[rt].ch[opt];
        while (t[o].ch[opt^1]) o=t[o].ch[opt^1];
        return o;
    }
    void del(int x)
    {
        int pre=next(x,0),nxt=next(x,1);
        splay(pre,0),splay(nxt,pre);
        int o=t[nxt].ch[0];
        if (t[o].rec>1)
        {
            t[o].rec--;
            splay(o,0);
        }
        else t[nxt].ch[0]=0;
        calc(nxt),calc(pre);
    }
    int K_th(int o,int k)
    {
        if (!o) return 0;
        if (k<=t[t[o].ch[0]].sz)
            return K_th(t[o].ch[0],k);
        else if (k>t[t[o].ch[0]].sz+t[o].rec)
            return K_th(t[o].ch[1],k-t[t[o].ch[0]].sz-t[o].rec);
        splay(o,0);
        return t[o].val;
    }
};
using namespace Splay;
int main()
{
    n=read();
    ins(rt,0,INF),ins(rt,0,-INF);
    for(int i=1;i<=n;i++)
    {
        int opt=read(),x=read();
        if (opt==1) ins(rt,0,x);
        if (opt==2) del(x);
        if (opt==3) find(x),printf("%d\n",t[t[rt].ch[0]].sz);
        if (opt==4) printf("%d\n",K_th(rt,x+1));
        if (opt==5) printf("%d\n",t[next(x,0)].val);
        if (opt==6) printf("%d\n",t[next(x,1)].val);	
    }
    return 0;
}
