/*
 * RUN ALL TESTS (from this libft-smoon folder):
 *
 * 1) Rebuild the library with memory/undefined-behavior checks:
 * make re CC=clang CFLAGS="-Wall -Wextra -Werror -g -O1 -fsanitize=address,undefined -fno-sanitize-recover=all"
 *
 * 2) Compile this main (copy the whole command):
 * clang -Wall -Wextra -Werror -g -O1 -fsanitize=address,undefined -fno-sanitize-recover=all main.c -Wl,--whole-archive libft.a -Wl,--no-whole-archive -Wl,--wrap=malloc -Wl,--wrap=free -o test
 *
 * 3) Run EVERY test at once:
 * ASAN_OPTIONS=detect_leaks=0 ./test
 *
 * Optional: ./test ft_atoi        -> only one function
 *           ./test --details     -> full failure messages
 *           ./test --required    -> skip extra NULL-protection tests
 *
 * Green PASS = correct. Red FAIL = wrong result or cleanup failure.
 * UB/crash = invalid memory/arithmetic operation. TIMEOUT = test did not finish.
 * Each test runs separately, so a crash does not stop the remaining tests.
 * EXTRA tests check NULL protection beyond normal libc requirements.
 * The final summary says whether the selected tests passed.
 *
 * IMPORTANT: add ft_putnbr_fd.c to Makefile SRCS once its code compiles.
 * A function absent from libft.a appears as MISSING instead of crashing main.
 * Both ft_strmapi and the old ft_strampi spelling are supported for testing.
 * The --whole-archive and --wrap flags above are required for these tests.
 * Allocation failures/cleanup are tracked; detect_leaks=0 avoids sandbox LSan errors.
 * Results describe tested inputs; they are not proof that every input is safe.
 */
#define _DEFAULT_SOURCE
#include "libft.h"
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

/* These declarations let main test bodies even if libft.h lacks a prototype. */
int		ft_atoi(const char *c) __attribute__((weak));
void	ft_bzero(void *s, size_t n) __attribute__((weak));
void	*ft_calloc(size_t nmemb, size_t size) __attribute__((weak));
int		ft_isalnum(int c) __attribute__((weak));
int		ft_isalpha(int c) __attribute__((weak));
int		ft_isascii(int c) __attribute__((weak));
int		ft_isdigit(int c) __attribute__((weak));
int		ft_isprint(int c) __attribute__((weak));
char	*ft_itoa(int c) __attribute__((weak));
void	ft_lstadd_back(t_list **lst, t_list *new) __attribute__((weak));
void	ft_lstadd_front(t_list **lst, t_list *new) __attribute__((weak));
void	ft_lstclear(t_list **lst, void (*del)(void *)) __attribute__((weak));
void ft_lstdelone(t_list *lst, void (*del)(void *)) __attribute__((weak));
void	ft_lstiter(t_list *lst, void (*f)(void *)) __attribute__((weak));
t_list	*ft_lstlast(t_list *lst) __attribute__((weak));
t_list	*ft_lstmap(t_list *lst, void *(*f)(void *), void (*del)(void *)) __attribute__((weak));
t_list	*ft_lstnew(void *content) __attribute__((weak));
int		ft_lstsize(t_list *lst) __attribute__((weak));
void	*ft_memchr(const void *s, int c, size_t n) __attribute__((weak));
int		ft_memcmp(const void *s1, const void *s2, size_t n) __attribute__((weak));
void	*ft_memcpy(void *dest, const void *src, size_t n) __attribute__((weak));
void	*ft_memmove(void *dest, const void *src, size_t n) __attribute__((weak));
void	*ft_memset(void *s, int c, size_t n) __attribute__((weak));
void	ft_putchar_fd(char c, int fd) __attribute__((weak));
void ft_putendl_fd(char *s, int fd) __attribute__((weak));
void ft_putnbr_fd(int n, int fd) __attribute__((weak));
void	ft_putstr_fd(char *s, int fd) __attribute__((weak));
char	**ft_split(char const *s, char c) __attribute__((weak));
char	*ft_strampi(char const *s, char (*f)(unsigned int, char)) __attribute__((weak));
char	*ft_strchr(const char *s, int c) __attribute__((weak));
char	*ft_strdup(const char *s) __attribute__((weak));
void	ft_striteri(char *s, void (*f)(unsigned int, char *)) __attribute__((weak));
char	*ft_strjoin(char const *s1, char const *s2) __attribute__((weak));
size_t	ft_strlcat(char *dst, const char *src, size_t size) __attribute__((weak));
size_t	ft_strlcpy(char *dst, const char *src, size_t size) __attribute__((weak));
size_t	ft_strlen(const char *c) __attribute__((weak));
char	*ft_strmapi(const char *s, char (*f)(unsigned int, char)) __attribute__((weak));
int		ft_strncmp(const char *s1, const char *s2, size_t n) __attribute__((weak));
char	*ft_strnstr(const char *big, const char *little, size_t len) __attribute__((weak));
char	*ft_strrchr(const char *s, int c) __attribute__((weak));
char	*ft_strtrim(char const *s1, char const *set) __attribute__((weak));
char	*ft_substr(char const *s, unsigned int start, size_t len) __attribute__((weak));
int		ft_tolower(int c) __attribute__((weak));
int		ft_toupper(int c) __attribute__((weak));

static int canonical_mapper_present(void) { return ft_strmapi != NULL; }
static int mapper_available(void) { return ft_strmapi != NULL || ft_strampi != NULL; }
static char *test_strmapi(const char *s, char (*f)(unsigned int, char))
{
    if(ft_strmapi) return ft_strmapi(s,f);
    return ft_strampi(s,f);
}
/* Accept the legacy spelling while still reporting the missing canonical API. */
#define ft_strampi test_strmapi

static unsigned long checks;
#define CHECK(x) do { checks++; if (!(x)) { fprintf(stderr, "ASSERT line %d: %s\n", __LINE__, #x); exit(2); } } while (0)
#define EQ(a,b) do { long long va = (long long)(a), vb = (long long)(b); checks++; if (va != vb) { fprintf(stderr, "ASSERT line %d: %s = %lld; expected %lld\n", __LINE__, #a, va, vb); exit(2); } } while (0)

void *__real_malloc(size_t);
void __real_free(void *);
static int tracking = 1, allocation_index, fail_index;
static void *owned[8192];
static size_t live;
void *__wrap_malloc(size_t n)
{
    void *p;
    if (tracking && ++allocation_index == fail_index) return NULL;
    p = __real_malloc(n);
    if (tracking && p) {
        CHECK(live < sizeof(owned) / sizeof(*owned));
        owned[live++] = p;
    }
    return p;
}
void __wrap_free(void *p)
{
    for (size_t i = 0; i < live; i++) if (owned[i] == p) {
        owned[i] = owned[--live]; break;
    }
    __real_free(p);
}
static void inject(int n) { tracking = 1; allocation_index = 0; fail_index = n; CHECK(live == 0); }
static void stop_inject(void) { tracking = 0; }
static void no_leaks(void) { EQ(live, 0); }
static int signum(int n) { return (n > 0) - (n < 0); }
static uint32_t rng = 0x42c0ffee;
static uint32_t rnd(void) { rng ^= rng << 13; rng ^= rng >> 17; rng ^= rng << 5; return rng; }
static void randstr(char *s, size_t n) { for (size_t i=0;i<n;i++) s[i]=(char)(1+rnd()%255); s[n]=0; }
static void expect_string(char *s, const char *want) { CHECK(s != NULL); CHECK(strcmp(s, want) == 0); free(s); }
static char mapchar(unsigned int i, char c) { return (char)(c + (i % 3)); }
static void iterchar(unsigned int i, char *c) { *c = mapchar(i, *c); }
static int callbacks;
static void count_del(void *p) { (void)p; callbacks++; }
static void free_del(void *p) { callbacks++; free(p); }
static void increment(void *p) { callbacks++; if (p) (*(int *)p)++; }
static void *clone_int(void *p) { int *q=malloc(sizeof(int)); if(q) *q=p ? *(int *)p+1 : 0; return q; }
static void freesplit(char **a) { if(a) { for(size_t i=0;a[i];i++) free(a[i]); free(a); } }
static void checksplit(char **a, const char *s, unsigned char d)
{
    size_t i=0,k=0;
    CHECK(a != NULL);
    while(s[i]) {
        while(s[i] && (unsigned char)s[i]==d) i++;
        size_t start=i;
        while(s[i] && (unsigned char)s[i]!=d) i++;
        if(i>start) { CHECK(a[k]); EQ(strlen(a[k]),i-start); CHECK(!memcmp(a[k],s+start,i-start)); k++; }
    }
    CHECK(a[k]==NULL);
    freesplit(a);
}
static char *ref_nstr(const char *s, const char *t, size_t n)
{
    size_t m=strlen(t);
    if(!m) return (char *)s;
    for(size_t i=0;i<n && s[i];i++) {
        if(m>n-i) break;
        size_t j=0; while(j<m && s[i+j] && s[i+j]==t[j]) j++;
        if(j==m) return (char *)s+i;
    }
    return NULL;
}
static size_t ref_lcpy(char *d,const char *s,size_t n) { size_t m=strlen(s); if(n) { size_t k=m<n-1?m:n-1; memcpy(d,s,k); d[k]=0; } return m; }
static size_t ref_lcat(char *d,const char *s,size_t n) { size_t i=0,m=strlen(s); while(i<n && d[i]) i++; if(i==n) return n+m; size_t k=m<n-i-1?m:n-i-1; memcpy(d+i,s,k); d[i+k]=0; return i+m; }

static void characters(const char *f, int k)
{
    int (*fn)(int)=NULL;
    if(!strcmp(f,"ft_isalpha")) fn=ft_isalpha;
    if(!strcmp(f,"ft_isdigit")) fn=ft_isdigit;
    if(!strcmp(f,"ft_isalnum")) fn=ft_isalnum;
    if(!strcmp(f,"ft_isascii")) fn=ft_isascii;
    if(!strcmp(f,"ft_isprint")) fn=ft_isprint;
    if(!strcmp(f,"ft_tolower")) fn=ft_tolower;
    if(!strcmp(f,"ft_toupper")) fn=ft_toupper;
    CHECK(fn);
    int extremes[]={INT_MIN,-1000,-256,-2,-1,0,127,128,255,256,1000,INT_MAX};
    for(int i=0;i<(k?12:257);i++) {
        int c=k?extremes[i]:i-1, want=0;
        if(!strcmp(f,"ft_isalpha")) want=(c>='a'&&c<='z')||(c>='A'&&c<='Z');
        if(!strcmp(f,"ft_isdigit")) want=c>='0'&&c<='9';
        if(!strcmp(f,"ft_isalnum")) want=(c>='a'&&c<='z')||(c>='A'&&c<='Z')||(c>='0'&&c<='9');
        if(!strcmp(f,"ft_isascii")) want=c>=0&&c<=127;
        if(!strcmp(f,"ft_isprint")) want=c>=32&&c<=126;
        if(!strcmp(f,"ft_tolower")) want=c>='A'&&c<='Z'?c+32:c;
        if(!strcmp(f,"ft_toupper")) want=c>='a'&&c<='z'?c-32:c;
        EQ(fn(c),want);
    }
}

static void memory(const char *f,int k)
{
    unsigned char a[128],b[128],s[128];
    memset(a,0xa5,sizeof(a)); memcpy(b,a,sizeof(a));
    for(size_t i=0;i<sizeof(s);i++) s[i]=(unsigned char)i;
    int set=!strcmp(f,"ft_memset"),zero=!strcmp(f,"ft_bzero"),copy=!strcmp(f,"ft_memcpy"),move=!strcmp(f,"ft_memmove"),chr=!strcmp(f,"ft_memchr");
    if(k==4) {
        size_t n=1024*1024;unsigned char *p=malloc(n+1),*q=malloc(n+1);CHECK(p&&q);memset(p,0x80,n+1);memcpy(q,p,n+1);
        if(set){CHECK(ft_memset(p,0xff,n)==p);memset(q,0xff,n);CHECK(!memcmp(p,q,n+1));}
        else if(zero){ft_bzero(p,n);memset(q,0,n);CHECK(!memcmp(p,q,n+1));}
        else if(copy){memset(q,0,n);CHECK(ft_memcpy(p,q,n)==p);CHECK(!memcmp(p,q,n+1));}
        else if(move){p[0]=q[0]=1;CHECK(ft_memmove(p+1,p,n)==p+1);memmove(q+1,q,n);CHECK(!memcmp(p,q,n+1));}
        else if(chr){p[n-1]=0xff;CHECK(ft_memchr(p,255,n)==p+n-1);}
        else{q[n-1]=0x81;EQ(signum(ft_memcmp(p,q,n)),-1);}
        free(p);free(q);return;
    }
    if(k>=10) {
        size_t n=k==10?0:1;
        if(set) CHECK(ft_memset(NULL,1,n)==NULL);
        else if(zero) ft_bzero(NULL,n);
        else if(copy) { if(k==12) ft_memcpy(a,NULL,1); else if(k==13) ft_memcpy(NULL,s,1); else CHECK(ft_memcpy(NULL,NULL,n)==NULL); }
        else if(move) { if(k==12) ft_memmove(a,NULL,1); else if(k==13) ft_memmove(NULL,s,1); else CHECK(ft_memmove(NULL,NULL,n)==NULL); }
        else if(chr) CHECK(ft_memchr(NULL,1,n)==NULL);
        else { if(k==12) (void)ft_memcmp(a,NULL,1); else if(!n) EQ(ft_memcmp(NULL,s,n),0); else (void)ft_memcmp(NULL,s,n); }
        return;
    }
    if(k==2) {
        unsigned char *p=malloc(1),*q=malloc(1); CHECK(p&&q); p[0]=q[0]='x';
        if(chr) CHECK(ft_memchr(p,'y',1)==NULL);
        else if(!set&&!zero&&!copy&&!move) EQ(ft_memcmp(p,q,1),0);
        else if(set) CHECK(ft_memset(p,255,1)==p);
        else if(zero) {ft_bzero(p,1);EQ(p[0],0);}
        else if(copy) CHECK(ft_memcpy(p,q,1)==p);
        else CHECK(ft_memmove(p,q,1)==p);
        free(p);free(q); return;
    }
    for(int t=0;t<(k?3000:1);t++) {
        size_t n=k?rnd()%65:64;
        int c=k?(int)(rnd()%1024)-512:0x1ff;
        if(set) { CHECK(ft_memset(a+16,c,n)==a+16);memset(b+16,c,n);CHECK(!memcmp(a,b,sizeof(a))); }
        else if(zero) {ft_bzero(a+16,n);memset(b+16,0,n);CHECK(!memcmp(a,b,sizeof(a)));}
        else if(copy) {CHECK(ft_memcpy(a+16,s,n)==a+16);memcpy(b+16,s,n);CHECK(!memcmp(a,b,sizeof(a)));}
        else if(move) {size_t x=k?rnd()%32:17,y=k?rnd()%32:16;CHECK(ft_memmove(a+x,a+y,n)==a+x);memmove(b+x,b+y,n);CHECK(!memcmp(a,b,sizeof(a)));}
        else if(chr) CHECK(ft_memchr(s,c,n)==memchr(s,c,n));
        else {if(k) a[rnd()%128]=(unsigned char)rnd(); EQ(signum(ft_memcmp(a,s,n)),signum(memcmp(a,s,n)));}
    }
}

static void strings(const char *f,int k)
{
    char a[260],b[260],s[130],t[130];
    memset(a,0x5a,sizeof(a));memcpy(b,a,sizeof(a));
    int length=!strcmp(f,"ft_strlen"),chr=!strcmp(f,"ft_strchr"),rchr=!strcmp(f,"ft_strrchr"),cmp=!strcmp(f,"ft_strncmp"),nstr=!strcmp(f,"ft_strnstr"),lcpy=!strcmp(f,"ft_strlcpy");
    if(k==4) {
        size_t n=1024*1024;char *p=malloc(n+1),*q=malloc(n+2);CHECK(p&&q);memset(p,'a',n);p[n-1]='b';p[n]=0;memcpy(q,p,n+1);
        if(length)EQ(ft_strlen(p),n);
        else if(chr)CHECK(ft_strchr(p,'b')==p+n-1);
        else if(rchr)CHECK(ft_strrchr(p,'a')==p+n-2);
        else if(cmp){q[n-1]='c';EQ(ft_strncmp(p,q,n-1),0);}
        else if(nstr)CHECK(ft_strnstr(p,"aab",n)==p+n-3);
        else if(lcpy){memset(q,0,n+2);EQ(ft_strlcpy(q,p,n+1),n);CHECK(!memcmp(p,q,n+1));}
        else{q[0]='x';q[1]=0;EQ(ft_strlcat(q,p,n+2),n+1);EQ(q[0],'x');CHECK(!memcmp(q+1,p,n+1));}
        free(p);free(q);return;
    }
    if(k>=10) {
        if(length) EQ(ft_strlen(NULL),0);
        else if(chr) CHECK(ft_strchr(NULL,'x')==NULL);
        else if(rchr) CHECK(ft_strrchr(NULL,'x')==NULL);
        else if(cmp) {if(k==10) EQ(ft_strncmp(NULL,NULL,0),0);else if(k==11) ft_strncmp(NULL,"x",1);else ft_strncmp("x",NULL,1);}
        else if(nstr) {if(k==10) CHECK(ft_strnstr(NULL,"x",0)==NULL);else if(k==11) CHECK(ft_strnstr(NULL,"",0)==NULL);else if(k==12) ft_strnstr("x",NULL,1);else ft_strnstr(NULL,"x",1);}
        else if(lcpy) {if(k==10) EQ(ft_strlcpy(NULL,"abc",0),3);else if(k==11) ft_strlcpy(NULL,"a",2);else ft_strlcpy(a,NULL,2);}
        else {if(k==10) EQ(ft_strlcat(NULL,"abc",0),3);else if(k==11) ft_strlcat(NULL,"a",2);else ft_strlcat(a,NULL,0);}
        return;
    }
    if(k==2) {
        if(cmp) EQ(ft_strncmp("abX","abY",2),0);
        else if(nstr) {const char *e="";CHECK(ft_strnstr(e,e,0)==e);}
        else if(chr||rchr) {const char *v="xax"; CHECK((chr?ft_strchr(v,256+'x'):ft_strrchr(v,256+'x'))==(chr?v:v+2));}
        else if(length) {char *p=malloc(1);CHECK(p);*p=0;EQ(ft_strlen(p),0);free(p);}
        else if(lcpy) {a[0]='Q';EQ(ft_strlcpy(a,"abc",1),3);EQ(a[0],0);EQ(a[1],0x5a);}
        else {char *p=malloc(3);CHECK(p);memcpy(p,"xyz",3);EQ(ft_strlcat(p,"ab",3),5);CHECK(!memcmp(p,"xyz",3));free(p);}
        return;
    }
    if(k==3) {
        char *p=malloc(2),*q=malloc(2);CHECK(p&&q);memcpy(p,"ab",2);memcpy(q,"ab",2);
        if(cmp) EQ(ft_strncmp(p,q,2),0);
        else if(nstr) CHECK(ft_strnstr(p,"z",0)==NULL);
        free(p);free(q);return;
    }
    for(int i=0;i<(k?3000:1);i++) {
        size_t m=k?rnd()%128:5,n=k?rnd()%128:3;
        randstr(s,m);randstr(t,n);
        if(!k){strcpy(s,"hello");strcpy(t,"ell");}
        size_t limit=k?rnd()%130:5;
        int c=k?(int)(rnd()%256):'l';
        if(length) EQ(ft_strlen(s),strlen(s));
        else if(chr) CHECK(ft_strchr(s,c)==strchr(s,c));
        else if(rchr) CHECK(ft_strrchr(s,c)==strrchr(s,c));
        else if(cmp) {EQ(signum(ft_strncmp(s,t,limit)),signum(strncmp(s,t,limit)));EQ(ft_strncmp(s,s,limit),0);}
        else if(nstr) {CHECK(ft_strnstr(s,t,limit)==ref_nstr(s,t,limit));CHECK(ft_strnstr(s,s+m/2,limit)==ref_nstr(s,s+m/2,limit));}
        else if(lcpy) {memset(a,0x5a,sizeof(a));memcpy(b,a,sizeof(a));EQ(ft_strlcpy(a+8,s,limit),ref_lcpy(b+8,s,limit));CHECK(!memcmp(a,b,sizeof(a)));}
        else {memset(a,0x5a,sizeof(a));strcpy(a+8,t);memcpy(b,a,sizeof(a));EQ(ft_strlcat(a+8,s,limit),ref_lcat(b+8,s,limit));CHECK(!memcmp(a,b,sizeof(a)));}
    }
}

static void numbers(const char *f,int k)
{
    if(!strcmp(f,"ft_atoi")) {
        const char *s[]={"0","42","-42"," 42","\t\n\v\f\r -123tail","2147483647","-2147483648","", "+", "--1", "99999999999999999999999999999999999999"};
        if(k==11) {ft_atoi(NULL);return;}
        if(k<11) {
            if(k==10) {(void)ft_atoi(s[k]);return;} /* overflow probe: no portable return-value oracle */
            int want[]={0,42,-42,42,-123,INT_MAX,INT_MIN,0,0,0};
            EQ(ft_atoi(s[k]),want[k]);return;
        }
        for(int i=0;i<5000;i++){int n=(int)(rnd()%2000000001)-1000000000;char b[64];snprintf(b,sizeof(b),"%d",n);EQ(ft_atoi(b),n);}return;
    }
    if(k>=10) {inject(k-9);char *p=ft_itoa(INT_MIN);stop_inject();if(k==10) CHECK(!p);else expect_string(p,"-2147483648");no_leaks();return;}
    int v[]={0,1,-1,INT_MIN,INT_MAX,10,-10,100,-100};
    for(int i=0;i<(k?5000:9);i++) {int n=k?(int)(rnd()%UINT32_MAX):v[i];char b[64];snprintf(b,sizeof(b),"%d",n);expect_string(ft_itoa(n),b);}
}

static void allocating(const char *f,int k)
{
    int dup=!strcmp(f,"ft_strdup"),sub=!strcmp(f,"ft_substr"),join=!strcmp(f,"ft_strjoin"),trim=!strcmp(f,"ft_strtrim"),split=!strcmp(f,"ft_split"),map=!strcmp(f,"ft_strampi");
    if(k==4) {
        size_t n=1024*1024;char *s=malloc(n+1);CHECK(s);memset(s,'a',n);s[n]=0;char *p=NULL;
        if(dup){p=ft_strdup(s);CHECK(p);CHECK(!memcmp(p,s,n+1));}
        else if(sub){p=ft_substr(s,n/2,SIZE_MAX);CHECK(p);EQ(strlen(p),n/2);CHECK(!memcmp(p,s,n/2+1-1));}
        else if(join){p=ft_strjoin(s,"tail");CHECK(p);EQ(strlen(p),n+4);CHECK(!memcmp(p,s,n));CHECK(!strcmp(p+n,"tail"));}
        else if(trim){s[n/2]='x';p=ft_strtrim(s,"a");CHECK(p);CHECK(!strcmp(p,"x"));}
        else if(split)checksplit(ft_split(s,':'),s,':');
        else if(map){p=ft_strampi(s,mapchar);CHECK(p);for(size_t i=0;i<n;i++)EQ(p[i],mapchar(i,'a'));EQ(p[n],0);}
        free(p);free(s);return;
    }
    if(split && k==5){size_t n=5000;char *s=malloc(2*n+1);CHECK(s);for(size_t i=0;i<2*n;i++)s[i]=i%2?':':'x';s[2*n]=0;checksplit(ft_split(s,':'),s,':');free(s);return;}
    if(k>=20) {
        int nth=(sub && k==21)?1:k-19; inject(nth);char *p=NULL;char **a=NULL;
        if(dup) p=ft_strdup("abc");
        else if(sub) p=ft_substr("abc",k==21?UINT_MAX:1,2);
        else if(join) p=ft_strjoin("a","b");
        else if(trim) p=ft_strtrim(" xx "," ");
        else if(split) a=ft_split("aa:bb:cc",':');
        else if(map) p=ft_strampi("abc",mapchar);
        stop_inject();
        if(split) {if(nth<=4) CHECK(!a);else checksplit(a,"aa:bb:cc",':');}
        else {CHECK(!p);}
        free(p);no_leaks();return;
    }
    if(k>=10) {
        char *p=NULL;char **a=NULL;
        if(dup) p=ft_strdup(NULL);
        else if(sub) p=ft_substr(NULL,0,2);
        else if(join) p=ft_strjoin(k==10||k==12?NULL:"abc",k==11||k==12?NULL:"abc");
        else if(trim) p=ft_strtrim(k==10?NULL:"abc",k==11?NULL:"x");
        else if(split) a=ft_split(NULL,':');
        else if(map) p=ft_strampi(k==10?NULL:"abc",k==11?NULL:mapchar);
        /* NULL probes accept any safe allocated result or NULL; policy is documented separately. */
        free(p);freesplit(a);return;
    }
    if(k==2) {
        if(dup) expect_string(ft_strdup(""),"");
        else if(sub) {expect_string(ft_substr("abc",3,SIZE_MAX),"");expect_string(ft_substr("abc",UINT_MAX,SIZE_MAX),"");expect_string(ft_substr("abc",1,SIZE_MAX),"bc");expect_string(ft_substr("abc",1,0),"");}
        else if(join) {expect_string(ft_strjoin("",""),"");expect_string(ft_strjoin("","abc"),"abc");expect_string(ft_strjoin("abc",""),"abc");}
        else if(trim) {expect_string(ft_strtrim("","x"),"");expect_string(ft_strtrim("xxx","x"),"");expect_string(ft_strtrim("abc",""),"abc");expect_string(ft_strtrim("xxabcxx","x"),"abc");}
        else if(split) {checksplit(ft_split("",':'),"",':');checksplit(ft_split("::: a::b::: ",':'),"::: a::b::: ",':');checksplit(ft_split(":::a:::",':'),":::a:::",':');checksplit(ft_split("abc",0),"abc",0);checksplit(ft_split(":::",':'),":::",':');}
        else if(map) expect_string(ft_strampi("",mapchar),"");
        return;
    }
    for(int i=0;i<(k?2000:1);i++) {
        char s[130],t[130],want[260];size_t n=k?rnd()%128:3,m=k?rnd()%128:3;randstr(s,n);randstr(t,m);
        if(!k){strcpy(s,"abc");strcpy(t,"def");}
        if(dup) {char *p=ft_strdup(s);CHECK(p!=s);expect_string(p,s);}
        else if(sub) {unsigned int start=rnd()%140;size_t len=rnd()%140,sz=strlen(s),z=start<sz?sz-start:0;if(z>len)z=len;memcpy(want,s+(start<sz?start:sz),z);want[z]=0;expect_string(ft_substr(s,start,len),want);}
        else if(join) {strcpy(want,s);strcat(want,t);expect_string(ft_strjoin(s,t),want);}
        else if(trim) {size_t start=0,end=strlen(s);while(start<end&&strchr(t,s[start]))start++;while(end>start&&strchr(t,s[end-1]))end--;memcpy(want,s+start,end-start);want[end-start]=0;expect_string(ft_strtrim(s,t),want);}
        else if(split) {char d=(char)(rnd()%256);checksplit(ft_split(s,d),s,(unsigned char)d);}
        else if(map) {for(size_t j=0;j<=strlen(s);j++)want[j]=s[j]?mapchar(j,s[j]):0;expect_string(ft_strampi(s,mapchar),want);}
    }
}

static void calloc_tests(int k)
{
    if(k>=20){inject(1);void *p=ft_calloc(4,4);stop_inject();CHECK(!p);no_leaks();return;}
    if(k==2) {CHECK(ft_calloc(SIZE_MAX,2)==NULL);CHECK(ft_calloc(SIZE_MAX/2+1,2)==NULL);CHECK(ft_calloc(2,SIZE_MAX)==NULL);return;}
    if(k==3) {free(ft_calloc(0,8));free(ft_calloc(8,0));free(ft_calloc(0,0));return;}
    for(int t=0;t<(k?2000:1);t++){size_t n=k?rnd()%64+1:8,m=k?rnd()%64+1:8;unsigned char *p=ft_calloc(n,m);CHECK(p);for(size_t i=0;i<n*m;i++) EQ(p[i],0);free(p);}
}

static void iter_tests(int k)
{
    char s[130],want[130];
    if(k==4){size_t n=1024*1024;char *p=malloc(n+1);CHECK(p);memset(p,'a',n);p[n]=0;ft_striteri(p,iterchar);for(size_t i=0;i<n;i++)EQ(p[i],mapchar(i,'a'));EQ(p[n],0);free(p);return;}
    if(k==10){ft_striteri(NULL,iterchar);return;}
    if(k==11){strcpy(s,"abc");ft_striteri(s,NULL);return;}
    if(k==2){s[0]=0;ft_striteri(s,iterchar);EQ(s[0],0);return;}
    for(int i=0;i<(k?2000:1);i++){randstr(s,k?rnd()%128:3);strcpy(want,s);size_t n=strlen(s);for(size_t j=0;j<n;j++)want[j]=mapchar(j,want[j]);ft_striteri(s,iterchar);CHECK(!memcmp(s,want,n+1));}
}

static void output_tests(const char *f,int k)
{
    if(!strcmp(f,"ft_putnbr_fd")) {
        int v[]={0,1,-1,INT_MIN,INT_MAX,12345,-12345};
        if(k==2){ft_putnbr_fd(INT_MIN,-1);return;}
        for(size_t i=0;i<sizeof(v)/sizeof(*v);i++){
            int pp[2];char b[64]={0},w[64];CHECK(!pipe(pp));snprintf(w,sizeof(w),"%d",v[i]);ft_putnbr_fd(v[i],pp[1]);close(pp[1]);EQ(read(pp[0],b,sizeof(b)),strlen(w));CHECK(!strcmp(b,w));close(pp[0]);
        }return;
    }
    int ch=!strcmp(f,"ft_putchar_fd"),end=!strcmp(f,"ft_putendl_fd");
    if(k==10){if(end)ft_putendl_fd(NULL,-1);else ft_putstr_fd(NULL,-1);return;}
    int p[2];CHECK(pipe(p)==0);
    if(k==11){if(end)ft_putendl_fd(NULL,p[1]);else ft_putstr_fd(NULL,p[1]);close(p[0]);close(p[1]);return;}
    if(k==2){close(p[1]);errno=0;if(ch)ft_putchar_fd('x',p[1]);else if(end)ft_putendl_fd("abc",p[1]);else ft_putstr_fd("abc",p[1]);EQ(errno,EBADF);close(p[0]);return;}
    char data[]={'a',(char)0xff,'b',0},buf[32]={0};
    const char *want=ch?"x":end?"a\xff" "b\n":data;
    size_t sz=ch?1:end?4:3;
    if(k==3){want=end?"\n":"";sz=end?1:0;data[0]=0;}
    if(ch)ft_putchar_fd(k==3?0:'x',p[1]);else if(end)ft_putendl_fd(data,p[1]);else ft_putstr_fd(data,p[1]);
    if(ch&&k==3){want="";sz=1;}
    close(p[1]);ssize_t n=read(p[0],buf,sizeof(buf));EQ(n,sz);CHECK(!memcmp(buf,want,sz));close(p[0]);
}

static void list_tests(const char *f,int k)
{
    int a=10,b=20,c=30; t_list n3={&c,NULL},n2={&b,&n3},n1={&a,&n2},*head=&n1;
    if(k==3) {
        enum { N=10000 };
        t_list *nodes=malloc(sizeof(*nodes)*N);CHECK(nodes);
        for(int i=0;i<N;i++) nodes[i]=(t_list){&a,i+1<N?&nodes[i+1]:NULL};
        if(!strcmp(f,"ft_lstsize")) EQ(ft_lstsize(nodes),N);
        else if(!strcmp(f,"ft_lstlast")) CHECK(ft_lstlast(nodes)==&nodes[N-1]);
        else if(!strcmp(f,"ft_lstiter")) {
            callbacks=0;ft_lstiter(nodes,increment);EQ(callbacks,N);EQ(a,10+N);
        } else CHECK(0);
        free(nodes);return;
    }
    if(!strcmp(f,"ft_lstnew")) {
        if(k==20){inject(1);t_list *p=ft_lstnew(&a);stop_inject();CHECK(!p);no_leaks();return;}
        t_list *p=ft_lstnew(k?NULL:&a);CHECK(p);CHECK(p->content==(k?NULL:&a));CHECK(!p->next);free(p);return;
    }
    if(!strcmp(f,"ft_lstsize")){EQ(ft_lstsize(k?NULL:&n1),k?0:3);return;}
    if(!strcmp(f,"ft_lstlast")){CHECK(ft_lstlast(k==1?NULL:k==2?&n3:&n1)==(k==1?NULL:&n3));return;}
    if(!strcmp(f,"ft_lstadd_front")) {
        t_list x={NULL,NULL};if(k==10)ft_lstadd_front(NULL,&x);else if(k==11)ft_lstadd_front(&head,NULL);else {if(k==1)head=NULL;ft_lstadd_front(&head,&x);CHECK(head==&x);CHECK(x.next==(k==1?NULL:&n1));}return;
    }
    if(!strcmp(f,"ft_lstadd_back")) {
        t_list x={NULL,NULL};if(k==10)ft_lstadd_back(NULL,&x);else if(k==11){ft_lstadd_back(&head,NULL);CHECK(head==&n1);}else {if(k==1)head=NULL;ft_lstadd_back(&head,&x);CHECK((k==1?head:n3.next)==&x);CHECK(!x.next);}return;
    }
    if(!strcmp(f,"ft_lstdelone")) {
        if(k==10){ft_lstdelone(NULL,count_del);return;}
        t_list *p=malloc(sizeof(*p));CHECK(p);p->content=k==2?NULL:&a;p->next=&n2;callbacks=0;
        if(k==11){ft_lstdelone(p,NULL);free(p);}else{ft_lstdelone(p,count_del);EQ(callbacks,1);CHECK(n2.next==&n3);}return;
    }
    if(!strcmp(f,"ft_lstclear")) {
        if(k==10){ft_lstclear(NULL,count_del);return;}
        if(k==1){head=NULL;ft_lstclear(&head,count_del);CHECK(!head);return;}
        if(k==11){ft_lstclear(&head,NULL);CHECK(head==&n1);return;}
        inject(0);head=malloc(sizeof(*head));t_list *p=malloc(sizeof(*p));CHECK(head&&p);*head=(t_list){&a,p};*p=(t_list){NULL,NULL};callbacks=0;ft_lstclear(&head,count_del);stop_inject();CHECK(!head);EQ(callbacks,2);no_leaks();return;
    }
    if(!strcmp(f,"ft_lstiter")) {
        callbacks=0;if(k==10)ft_lstiter(NULL,increment);else if(k==11)ft_lstiter(head,NULL);else{if(k==2)n2.content=NULL;ft_lstiter(head,increment);EQ(callbacks,3);EQ(a,11);EQ(c,31);}return;
    }
    if(!strcmp(f,"ft_lstmap")) {
        t_list *p;
        if(k==10){CHECK(ft_lstmap(NULL,clone_int,free_del)==NULL);return;}
        if(k==11){CHECK(ft_lstmap(head,NULL,free_del)==NULL);return;}
        if(k==12){CHECK(ft_lstmap(head,clone_int,NULL)==NULL);return;}
        inject(k>=20?k-19:0);p=ft_lstmap(head,clone_int,free_del);stop_inject();
        if(k>=20) {CHECK(!p);no_leaks();return;}
        CHECK(p&&p!=head);CHECK(*(int *)p->content==11);CHECK(p->next&&*(int *)p->next->content==21);CHECK(p->next->next&&*(int *)p->next->next->content==31);CHECK(!p->next->next->next);EQ(a,10);ft_lstclear(&p,free_del);no_leaks();return;
    }
    CHECK(0);
}

/* Valid bounded APIs must not even read beyond the supplied limit. */
static void guard_tests(const char *f,int k)
{
    size_t ps=(size_t)sysconf(_SC_PAGESIZE);
    char *p=mmap(NULL,ps*2,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS,-1,0);CHECK(p!=MAP_FAILED);CHECK(!mprotect(p+ps,ps,PROT_NONE));
    char *last=p+ps-1;*last='x';
    if(!strcmp(f,"ft_strnstr")) CHECK(ft_strnstr(k?last:last+1,"z",k?1:0)==NULL);
    else if(!strcmp(f,"ft_strncmp")) EQ(ft_strncmp(last,last,1),0);
    else CHECK(0);
    CHECK(!munmap(p,ps*2));
}

typedef struct s_case {
    const char *function;
    int id;
    const char *description;
    int extra;
} t_case;
static const t_case cases[] = {
    {"ft_isalpha", 0, "all bytes 0..255 and EOF", 0},
    {"ft_isalpha", 1, "INT_MIN/INT_MAX and out-of-domain integers", 1},
    {"ft_isdigit", 0, "all bytes 0..255 and EOF", 0},
    {"ft_isdigit", 1, "INT_MIN/INT_MAX and out-of-domain integers", 1},
    {"ft_isalnum", 0, "all bytes 0..255 and EOF", 0},
    {"ft_isalnum", 1, "INT_MIN/INT_MAX and out-of-domain integers", 1},
    {"ft_isascii", 0, "all bytes 0..255 and EOF", 0},
    {"ft_isascii", 1, "INT_MIN/INT_MAX and out-of-domain integers", 1},
    {"ft_isprint", 0, "all bytes 0..255 and EOF", 0},
    {"ft_isprint", 1, "INT_MIN/INT_MAX and out-of-domain integers", 1},
    {"ft_tolower", 0, "all bytes 0..255 and EOF", 0},
    {"ft_tolower", 1, "INT_MIN/INT_MAX and out-of-domain integers", 1},
    {"ft_toupper", 0, "all bytes 0..255 and EOF", 0},
    {"ft_toupper", 1, "INT_MIN/INT_MAX and out-of-domain integers", 1},
    {"ft_memset", 0, "normal binary operation and surrounding bytes", 0},
    {"ft_memset", 1, "3000 deterministic randomized cases", 0},
    {"ft_memset", 2, "exact one-byte heap allocation", 0},
    {"ft_memset", 4, "1 MiB binary buffer stress", 0},
    {"ft_memset", 10, "NULL with zero length", 1},
    {"ft_memset", 11, "NULL with positive length", 1},
    {"ft_bzero", 0, "normal binary operation and surrounding bytes", 0},
    {"ft_bzero", 1, "3000 deterministic randomized cases", 0},
    {"ft_bzero", 2, "exact one-byte heap allocation", 0},
    {"ft_bzero", 4, "1 MiB binary buffer stress", 0},
    {"ft_bzero", 10, "NULL with zero length", 1},
    {"ft_bzero", 11, "NULL with positive length", 1},
    {"ft_memcpy", 0, "normal binary operation and surrounding bytes", 0},
    {"ft_memcpy", 1, "3000 deterministic randomized cases", 0},
    {"ft_memcpy", 2, "exact one-byte heap allocation", 0},
    {"ft_memcpy", 4, "1 MiB binary buffer stress", 0},
    {"ft_memcpy", 10, "NULL with zero length", 1},
    {"ft_memcpy", 11, "NULL with positive length", 1},
    {"ft_memcpy", 12, "NULL source with positive length", 1},
    {"ft_memcpy", 13, "NULL destination with positive length", 1},
    {"ft_memmove", 0, "normal binary operation and surrounding bytes", 0},
    {"ft_memmove", 1, "3000 deterministic randomized cases", 0},
    {"ft_memmove", 2, "exact one-byte heap allocation", 0},
    {"ft_memmove", 4, "1 MiB binary buffer stress", 0},
    {"ft_memmove", 10, "NULL with zero length", 1},
    {"ft_memmove", 11, "NULL with positive length", 1},
    {"ft_memmove", 12, "NULL source with positive length", 1},
    {"ft_memmove", 13, "NULL destination with positive length", 1},
    {"ft_memchr", 0, "normal binary operation and surrounding bytes", 0},
    {"ft_memchr", 1, "3000 deterministic randomized cases", 0},
    {"ft_memchr", 2, "exact one-byte heap allocation", 0},
    {"ft_memchr", 4, "1 MiB binary buffer stress", 0},
    {"ft_memchr", 10, "NULL with zero length", 1},
    {"ft_memchr", 11, "NULL with positive length", 1},
    {"ft_memcmp", 0, "normal binary operation and surrounding bytes", 0},
    {"ft_memcmp", 1, "3000 deterministic randomized cases", 0},
    {"ft_memcmp", 2, "exact one-byte heap allocation", 0},
    {"ft_memcmp", 4, "1 MiB binary buffer stress", 0},
    {"ft_memcmp", 10, "NULL with zero length", 1},
    {"ft_memcmp", 11, "NULL with positive length", 1},
    {"ft_memcmp", 12, "NULL source with positive length", 1},
    {"ft_strlen", 0, "normal string behavior", 0},
    {"ft_strlen", 1, "3000 deterministic randomized cases", 0},
    {"ft_strlen", 4, "1 MiB string stress", 0},
    {"ft_strlen", 2, "empty string at heap boundary", 0},
    {"ft_strlen", 10, "NULL input", 1},
    {"ft_strchr", 0, "normal string behavior", 0},
    {"ft_strchr", 1, "3000 deterministic randomized cases", 0},
    {"ft_strchr", 4, "1 MiB string stress", 0},
    {"ft_strchr", 2, "character conversion: 256 + 'x'", 0},
    {"ft_strchr", 10, "NULL input", 1},
    {"ft_strrchr", 0, "normal string behavior", 0},
    {"ft_strrchr", 1, "3000 deterministic randomized cases", 0},
    {"ft_strrchr", 4, "1 MiB string stress", 0},
    {"ft_strrchr", 2, "character conversion: 256 + 'x'", 0},
    {"ft_strrchr", 10, "NULL input", 1},
    {"ft_strncmp", 0, "normal string behavior", 0},
    {"ft_strncmp", 1, "3000 deterministic randomized cases", 0},
    {"ft_strncmp", 4, "1 MiB string stress", 0},
    {"ft_strncmp", 2, "equal prefix differs just beyond n", 0},
    {"ft_strncmp", 3, "nonterminated bounded heap input", 0},
    {"ft_strncmp", 90, "guard page at exact bound", 0},
    {"ft_strncmp", 10, "both NULL with n=0", 1},
    {"ft_strncmp", 11, "NULL first input", 1},
    {"ft_strncmp", 12, "NULL second input", 1},
    {"ft_strnstr", 0, "normal string behavior", 0},
    {"ft_strnstr", 1, "3000 deterministic randomized cases", 0},
    {"ft_strnstr", 4, "1 MiB string stress", 0},
    {"ft_strnstr", 2, "empty haystack and needle at length zero", 0},
    {"ft_strnstr", 3, "nonterminated bounded heap input", 0},
    {"ft_strnstr", 90, "zero bound at inaccessible page", 0},
    {"ft_strnstr", 91, "one-byte bound before inaccessible page", 0},
    {"ft_strnstr", 10, "NULL haystack, n=0, nonempty needle", 1},
    {"ft_strnstr", 11, "NULL haystack and empty needle", 1},
    {"ft_strnstr", 12, "NULL needle", 1},
    {"ft_strnstr", 13, "NULL haystack, positive length", 1},
    {"ft_strlcpy", 0, "normal string behavior", 0},
    {"ft_strlcpy", 1, "3000 deterministic randomized cases", 0},
    {"ft_strlcpy", 4, "1 MiB string stress", 0},
    {"ft_strlcpy", 2, "size one and untouched surrounding bytes", 0},
    {"ft_strlcpy", 10, "NULL destination with size=0", 1},
    {"ft_strlcpy", 11, "NULL destination, positive size", 1},
    {"ft_strlcpy", 12, "NULL source", 1},
    {"ft_strlcat", 0, "normal string behavior", 0},
    {"ft_strlcat", 1, "3000 deterministic randomized cases", 0},
    {"ft_strlcat", 4, "1 MiB string stress", 0},
    {"ft_strlcat", 2, "nonterminated destination with size equal to allocation", 0},
    {"ft_strlcat", 10, "NULL destination with size=0", 1},
    {"ft_strlcat", 11, "NULL destination, positive size", 1},
    {"ft_strlcat", 12, "NULL source", 1},
    {"ft_atoi", 0, "zero", 0},
    {"ft_atoi", 1, "positive integer", 0},
    {"ft_atoi", 2, "negative integer", 0},
    {"ft_atoi", 3, "leading ASCII space", 0},
    {"ft_atoi", 4, "all whitespace and trailing text", 0},
    {"ft_atoi", 5, "INT_MAX", 0},
    {"ft_atoi", 6, "INT_MIN", 0},
    {"ft_atoi", 7, "empty string", 0},
    {"ft_atoi", 8, "sign without digits", 0},
    {"ft_atoi", 9, "double sign", 0},
    {"ft_atoi", 10, "many-digit overflow: sanitizer probe only", 1},
    {"ft_atoi", 11, "NULL input", 1},
    {"ft_atoi", 12, "5000 random representable integers", 0},
    {"ft_itoa", 0, "zero, signs, decimal boundaries, INT_MIN and INT_MAX", 0},
    {"ft_itoa", 1, "5000 random integer conversions", 0},
    {"ft_itoa", 10, "first malloc fails", 0},
    {"ft_strdup", 0, "normal allocated output", 0},
    {"ft_strdup", 1, "2000 deterministic randomized cases", 0},
    {"ft_strdup", 4, "1 MiB allocated string stress", 0},
    {"ft_strdup", 2, "empty string", 0},
    {"ft_strdup", 10, "NULL input", 1},
    {"ft_strdup", 20, "malloc #1 fails", 0},
    {"ft_substr", 0, "normal allocated output", 0},
    {"ft_substr", 1, "2000 deterministic randomized cases", 0},
    {"ft_substr", 4, "1 MiB allocated string stress", 0},
    {"ft_substr", 2, "start at/past end, UINT_MAX start, SIZE_MAX length, zero length", 0},
    {"ft_substr", 10, "NULL input", 1},
    {"ft_substr", 20, "malloc #1 fails", 0},
    {"ft_substr", 21, "out-of-range start: calloc allocation fails", 0},
    {"ft_strjoin", 0, "normal allocated output", 0},
    {"ft_strjoin", 1, "2000 deterministic randomized cases", 0},
    {"ft_strjoin", 4, "1 MiB allocated string stress", 0},
    {"ft_strjoin", 2, "both/one operand empty", 0},
    {"ft_strjoin", 10, "NULL first input", 1},
    {"ft_strjoin", 11, "NULL second input or callback", 1},
    {"ft_strjoin", 12, "both operands NULL", 1},
    {"ft_strjoin", 20, "malloc #1 fails", 0},
    {"ft_strtrim", 0, "normal allocated output", 0},
    {"ft_strtrim", 1, "2000 deterministic randomized cases", 0},
    {"ft_strtrim", 4, "1 MiB allocated string stress", 0},
    {"ft_strtrim", 2, "empty/all-trimmed/no-set/surrounded strings", 0},
    {"ft_strtrim", 10, "NULL input", 1},
    {"ft_strtrim", 11, "NULL second input or callback", 1},
    {"ft_strtrim", 20, "malloc #1 fails", 0},
    {"ft_split", 0, "normal allocated output", 0},
    {"ft_split", 1, "2000 deterministic randomized cases", 0},
    {"ft_split", 4, "1 MiB allocated string stress", 0},
    {"ft_split", 5, "5000 tokens: contents, NULL sentinel, and cleanup", 0},
    {"ft_split", 2, "empty/all/repeated/end delimiters and NUL delimiter", 0},
    {"ft_split", 10, "NULL input", 1},
    {"ft_split", 20, "malloc #1 fails", 0},
    {"ft_split", 21, "malloc #2 fails", 0},
    {"ft_split", 22, "malloc #3 fails", 0},
    {"ft_split", 23, "malloc #4 fails", 0},
    {"ft_split", 24, "failure index past final allocation: success control", 0},
    {"ft_strampi", 0, "normal allocated output", 0},
    {"ft_strampi", 1, "2000 deterministic randomized cases", 0},
    {"ft_strampi", 4, "1 MiB allocated string stress", 0},
    {"ft_strampi", 2, "empty string callback mapping", 0},
    {"ft_strampi", 10, "NULL input", 1},
    {"ft_strampi", 11, "NULL second input or callback", 1},
    {"ft_strampi", 20, "malloc #1 fails", 0},
    {"ft_calloc", 0, "normal zeroed allocation", 0},
    {"ft_calloc", 1, "2000 randomized allocation sizes", 0},
    {"ft_calloc", 2, "SIZE_MAX multiplication overflow", 0},
    {"ft_calloc", 3, "zero-size allocations", 0},
    {"ft_calloc", 20, "malloc fails", 0},
    {"ft_striteri", 0, "normal mutation with indices", 0},
    {"ft_striteri", 1, "2000 randomized mutations", 0},
    {"ft_striteri", 2, "empty string", 0},
    {"ft_striteri", 4, "1 MiB callback mutation stress", 0},
    {"ft_striteri", 10, "NULL string", 1},
    {"ft_striteri", 11, "NULL callback", 1},
    {"ft_putchar_fd", 0, "pipe captures exact bytes", 0},
    {"ft_putchar_fd", 2, "closed file descriptor returns EBADF", 0},
    {"ft_putchar_fd", 3, "empty string / NUL byte", 0},
    {"ft_putstr_fd", 0, "pipe captures exact bytes", 0},
    {"ft_putstr_fd", 2, "closed file descriptor returns EBADF", 0},
    {"ft_putstr_fd", 3, "empty string / NUL byte", 0},
    {"ft_putstr_fd", 10, "NULL string, invalid descriptor", 1},
    {"ft_putstr_fd", 11, "NULL string, writable pipe", 1},
    {"ft_putendl_fd", 0, "pipe captures exact bytes", 0},
    {"ft_putendl_fd", 2, "closed file descriptor returns EBADF", 0},
    {"ft_putendl_fd", 3, "empty string / NUL byte", 0},
    {"ft_putendl_fd", 10, "NULL string, invalid descriptor", 1},
    {"ft_putendl_fd", 11, "NULL string, writable pipe", 1},
    {"ft_putnbr_fd", 0, "zero/signs/INT_MIN/INT_MAX to pipe", 0},
    {"ft_putnbr_fd", 2, "invalid descriptor", 0},
    {"ft_lstnew", 0, "normal linked-list behavior", 0},
    {"ft_lstnew", 1, "NULL content is valid", 0},
    {"ft_lstnew", 20, "malloc fails", 0},
    {"ft_lstsize", 0, "normal linked-list behavior", 0},
    {"ft_lstsize", 1, "empty list", 0},
    {"ft_lstsize", 3, "10000-node list", 0},
    {"ft_lstlast", 0, "normal linked-list behavior", 0},
    {"ft_lstlast", 1, "empty list", 0},
    {"ft_lstlast", 2, "single-node list", 0},
    {"ft_lstlast", 3, "10000-node list", 0},
    {"ft_lstadd_front", 0, "normal linked-list behavior", 0},
    {"ft_lstadd_front", 1, "insertion into empty list", 0},
    {"ft_lstadd_front", 10, "NULL list double-pointer", 1},
    {"ft_lstadd_front", 11, "NULL new node", 1},
    {"ft_lstadd_back", 0, "normal linked-list behavior", 0},
    {"ft_lstadd_back", 1, "insertion into empty list", 0},
    {"ft_lstadd_back", 10, "NULL list double-pointer", 1},
    {"ft_lstadd_back", 11, "NULL new node", 1},
    {"ft_lstdelone", 0, "normal linked-list behavior", 0},
    {"ft_lstdelone", 10, "NULL list pointer", 1},
    {"ft_lstdelone", 11, "NULL callback", 1},
    {"ft_lstdelone", 2, "NULL node content with compatible callback", 0},
    {"ft_lstclear", 0, "normal linked-list behavior", 0},
    {"ft_lstclear", 1, "empty list", 0},
    {"ft_lstclear", 10, "NULL list pointer", 1},
    {"ft_lstclear", 11, "NULL callback", 1},
    {"ft_lstiter", 0, "normal linked-list behavior", 0},
    {"ft_lstiter", 10, "NULL list pointer", 1},
    {"ft_lstiter", 11, "NULL callback", 1},
    {"ft_lstiter", 2, "NULL node content with compatible callback", 0},
    {"ft_lstiter", 3, "10000-node list", 0},
    {"ft_lstmap", 0, "normal linked-list behavior", 0},
    {"ft_lstmap", 10, "NULL list pointer", 1},
    {"ft_lstmap", 11, "NULL callback", 1},
    {"ft_lstmap", 12, "NULL destructor", 1},
    {"ft_lstmap", 21, "destination node malloc #2 fails; cleanup checked", 0},
    {"ft_lstmap", 23, "destination node malloc #4 fails; cleanup checked", 0},
    {"ft_lstmap", 25, "destination node malloc #6 fails; cleanup checked", 0},
};
static const char *functions[] = {
    "ft_atoi",
    "ft_bzero",
    "ft_calloc",
    "ft_isalnum",
    "ft_isalpha",
    "ft_isascii",
    "ft_isdigit",
    "ft_isprint",
    "ft_itoa",
    "ft_lstadd_back",
    "ft_lstadd_front",
    "ft_lstclear",
    "ft_lstdelone",
    "ft_lstiter",
    "ft_lstlast",
    "ft_lstmap",
    "ft_lstnew",
    "ft_lstsize",
    "ft_memchr",
    "ft_memcmp",
    "ft_memcpy",
    "ft_memmove",
    "ft_memset",
    "ft_putchar_fd",
    "ft_putendl_fd",
    "ft_putnbr_fd",
    "ft_putstr_fd",
    "ft_split",
    "ft_strampi",
    "ft_strchr",
    "ft_strdup",
    "ft_striteri",
    "ft_strjoin",
    "ft_strlcat",
    "ft_strlcpy",
    "ft_strlen",
    "ft_strncmp",
    "ft_strnstr",
    "ft_strrchr",
    "ft_strtrim",
    "ft_substr",
    "ft_tolower",
    "ft_toupper",
};

static int available(const char *name)
{
    if(!strcmp(name,"ft_atoi")) return ft_atoi != NULL;
    if(!strcmp(name,"ft_bzero")) return ft_bzero != NULL;
    if(!strcmp(name,"ft_calloc")) return ft_calloc != NULL;
    if(!strcmp(name,"ft_isalnum")) return ft_isalnum != NULL;
    if(!strcmp(name,"ft_isalpha")) return ft_isalpha != NULL;
    if(!strcmp(name,"ft_isascii")) return ft_isascii != NULL;
    if(!strcmp(name,"ft_isdigit")) return ft_isdigit != NULL;
    if(!strcmp(name,"ft_isprint")) return ft_isprint != NULL;
    if(!strcmp(name,"ft_itoa")) return ft_itoa != NULL;
    if(!strcmp(name,"ft_lstadd_back")) return ft_lstadd_back != NULL;
    if(!strcmp(name,"ft_lstadd_front")) return ft_lstadd_front != NULL;
    if(!strcmp(name,"ft_lstclear")) return ft_lstclear != NULL;
    if(!strcmp(name,"ft_lstdelone")) return ft_lstdelone != NULL;
    if(!strcmp(name,"ft_lstiter")) return ft_lstiter != NULL;
    if(!strcmp(name,"ft_lstlast")) return ft_lstlast != NULL;
    if(!strcmp(name,"ft_lstmap")) return ft_lstmap != NULL;
    if(!strcmp(name,"ft_lstnew")) return ft_lstnew != NULL;
    if(!strcmp(name,"ft_lstsize")) return ft_lstsize != NULL;
    if(!strcmp(name,"ft_memchr")) return ft_memchr != NULL;
    if(!strcmp(name,"ft_memcmp")) return ft_memcmp != NULL;
    if(!strcmp(name,"ft_memcpy")) return ft_memcpy != NULL;
    if(!strcmp(name,"ft_memmove")) return ft_memmove != NULL;
    if(!strcmp(name,"ft_memset")) return ft_memset != NULL;
    if(!strcmp(name,"ft_putchar_fd")) return ft_putchar_fd != NULL;
    if(!strcmp(name,"ft_putendl_fd")) return ft_putendl_fd != NULL;
    if(!strcmp(name,"ft_putnbr_fd")) return ft_putnbr_fd != NULL;
    if(!strcmp(name,"ft_putstr_fd")) return ft_putstr_fd != NULL;
    if(!strcmp(name,"ft_split")) return ft_split != NULL;
    if(!strcmp(name,"ft_strampi")) return mapper_available();
    if(!strcmp(name,"ft_strchr")) return ft_strchr != NULL;
    if(!strcmp(name,"ft_strdup")) return ft_strdup != NULL;
    if(!strcmp(name,"ft_striteri")) return ft_striteri != NULL;
    if(!strcmp(name,"ft_strjoin")) return ft_strjoin != NULL;
    if(!strcmp(name,"ft_strlcat")) return ft_strlcat != NULL;
    if(!strcmp(name,"ft_strlcpy")) return ft_strlcpy != NULL;
    if(!strcmp(name,"ft_strlen")) return ft_strlen != NULL;
    if(!strcmp(name,"ft_strncmp")) return ft_strncmp != NULL;
    if(!strcmp(name,"ft_strnstr")) return ft_strnstr != NULL;
    if(!strcmp(name,"ft_strrchr")) return ft_strrchr != NULL;
    if(!strcmp(name,"ft_strtrim")) return ft_strtrim != NULL;
    if(!strcmp(name,"ft_substr")) return ft_substr != NULL;
    if(!strcmp(name,"ft_tolower")) return ft_tolower != NULL;
    if(!strcmp(name,"ft_toupper")) return ft_toupper != NULL;
    return 0;
}

/* Each dangerous call runs in a child: a crash cannot stop the next test. */
static void execute_case(const char *f, int k)
{
    checks=0; rng=0x42c0ffee; live=0; tracking=1;
    allocation_index=0; fail_index=0;
    if(k>=90)guard_tests(f,k-90);
    else if(!strncmp(f,"ft_is",5)||!strcmp(f,"ft_tolower")||!strcmp(f,"ft_toupper"))characters(f,k);
    else if(!strncmp(f,"ft_mem",6)||!strcmp(f,"ft_bzero"))memory(f,k);
    else if(!strcmp(f,"ft_atoi")||!strcmp(f,"ft_itoa"))numbers(f,k);
    else if(!strcmp(f,"ft_calloc"))calloc_tests(k);
    else if(!strcmp(f,"ft_striteri"))iter_tests(k);
    else if(!strncmp(f,"ft_put",6))output_tests(f,k);
    else if(!strncmp(f,"ft_lst",6))list_tests(f,k);
    else if(!strcmp(f,"ft_strdup")||!strcmp(f,"ft_substr")||!strcmp(f,"ft_strjoin")||!strcmp(f,"ft_strtrim")||!strcmp(f,"ft_split")||!strcmp(f,"ft_strampi"))allocating(f,k);
    else strings(f,k);
    no_leaks();
}

static const char *green="", *red="", *yellow="", *cyan="", *reset="";
typedef struct s_score { int passed, failed, missing; } t_score;

static const char *public_name(const char *s)
{
    return !strcmp(s,"ft_strampi") ? "ft_strmapi" : s;
}

static void help(const char *program)
{
    printf("Usage: %s [ft_function] [--required] [--details] [--no-color] [--list]\n",program);
    puts("  default       All functions: correctness + extra NULL protections");
    puts("  ft_function   Only this function (ft_strampi also accepted)");
    puts("  --required    Correctness/limit tests only; skip extra NULL probes");
    puts("  --details     Print full diagnostics for failing tests");
    puts("  --list        List tests without executing them");
    puts("  Exit: 0 = selected tests pass, 1 = failed/missing, 2 = usage/setup error.");
}

/* Read only a short useful diagnostic by default; nothing is saved as a log. */
static int show_diagnostic(FILE *capture, int details)
{
    char line[2048];
    int shown=0, memory_error=0;
    rewind(capture);
    while(fgets(line,sizeof(line),capture)) {
        int useful=strstr(line,"ASSERT line")!=NULL || strstr(line,"runtime error:")!=NULL
            || strstr(line,"SUMMARY:")!=NULL;
        if(strstr(line,"AddressSanitizer") || strstr(line,"runtime error:")) memory_error=1;
        if(details || (useful && !shown)) {
            printf("      %s",line);
            shown=1;
        }
    }
    return memory_error;
}

static int run_isolated(const t_case *test, int details)
{
    FILE *capture=tmpfile();
    pid_t child;
    int status;
    if(!capture){perror("tmpfile");return 2;}
    fflush(NULL); /* Do not duplicate the parent's buffered output after fork. */
    child=fork();
    if(child<0){perror("fork");fclose(capture);return 2;}
    if(child==0) {
        if(dup2(fileno(capture),STDOUT_FILENO)<0 || dup2(fileno(capture),STDERR_FILENO)<0)
            _exit(125);
        alarm(8); /* A broken infinite loop becomes TIMEOUT after 8 seconds. */
        execute_case(test->function,test->id);
        fflush(NULL);
        _exit(0);
    }
    while(waitpid(child,&status,0)<0) {
        if(errno==EINTR)continue;
        perror("waitpid");fclose(capture);return 2;
    }
    if(WIFEXITED(status) && WEXITSTATUS(status)==0) {
        printf("    %s[PASS]%s %s\n",green,reset,test->description);
        fclose(capture);return 0;
    }
    if(WIFSIGNALED(status) && WTERMSIG(status)==SIGALRM)
        printf("    %s[TIMEOUT]%s %s\n",red,reset,test->description);
    else if(WIFSIGNALED(status))
        printf("    %s[CRASH: signal %d]%s %s\n",red,WTERMSIG(status),reset,test->description);
    else if(WEXITSTATUS(status)==2)
        printf("    %s[FAIL]%s %s\n",red,reset,test->description);
    else
        printf("    %s[ERROR / UB]%s %s\n",red,reset,test->description);
    show_diagnostic(capture,details);
    fclose(capture);return 1;
}

int main(int argc,char **argv)
{
    const char *filter=NULL;
    int required_only=0, details=0, list=0, no_color=0, selected=0, setup_errors=0;
    t_score required={0,0,0}, extra={0,0,0};
    struct rlimit core_limit={0,0};
    setrlimit(RLIMIT_CORE,&core_limit);
    for(int i=1;i<argc;i++) {
        if(!strcmp(argv[i],"--help") || !strcmp(argv[i],"-h")){help(argv[0]);return 0;}
        else if(!strcmp(argv[i],"--required"))required_only=1;
        else if(!strcmp(argv[i],"--details"))details=1;
        else if(!strcmp(argv[i],"--no-color"))no_color=1;
        else if(!strcmp(argv[i],"--list"))list=1;
        else if(!strncmp(argv[i],"ft_",3) && !filter)filter=argv[i];
        else {fprintf(stderr,"Unknown argument: %s\n",argv[i]);help(argv[0]);return 2;}
    }
    if(!no_color && !getenv("NO_COLOR") && (isatty(STDOUT_FILENO) || getenv("FORCE_COLOR"))) {
        green="\033[32m";red="\033[31m";yellow="\033[33m";cyan="\033[36m";reset="\033[0m";
    }
    printf("\n%s========== SMOON LIBFT TESTS ==========%s\n",cyan,reset);
    puts("PASS = expected result | FAIL = wrong result/leak | UB = invalid C operation");
    puts("EXTRA = NULL/out-of-domain protection; these are not normal libc requirements.");
    puts("Allocation cleanup is tracked directly; use sanitizers to detect overreads/UB.\n");
    for(size_t i=0;i<sizeof(functions)/sizeof(*functions);i++) {
        const char *fn=functions[i];
        int function_passed=0,function_failed=0;
        if(filter && strcmp(filter,fn) && strcmp(filter,public_name(fn)))continue;
        printf("%s%s%s",cyan,public_name(fn),reset);
        if(!strcmp(fn,"ft_strampi") && !canonical_mapper_present())
            printf(" %s(legacy ft_strampi body; rename still needed)%s",yellow,reset);
        putchar('\n');
        for(size_t j=0;j<sizeof(cases)/sizeof(*cases);j++) {
            const t_case *test=&cases[j];
            t_score *score=test->extra ? &extra : &required;
            if(strcmp(test->function,fn) || (required_only && test->extra))continue;
            selected++;
            if(list) {printf("    [%s] #%d %s\n",test->extra?"EXTRA":"REQUIRED",test->id,test->description);continue;}
            if(test->extra)printf("    %sEXTRA NULL/robustness:%s\n",yellow,reset);
            if(!available(fn)) {
                printf("    %s[MISSING / BUILD FAILED]%s %s\n",red,reset,test->description);
                score->missing++;function_failed++;continue;
            }
            int result=run_isolated(test,details);
            if(result==0){score->passed++;function_passed++;}
            else {score->failed++;function_failed++;if(result==2)setup_errors++;}
        }
        if(!list)printf("    %s=> %d passed, %d failed/missing%s\n\n",function_failed?red:green,function_passed,function_failed,reset);
    }
    if(!selected){fprintf(stderr,"No tests matched. Use --list or --help.\n");return 2;}
    if(list){printf("\n%d selected tests.\n",selected);return 0;}
    printf("%s================ RESULTS ================%s\n",cyan,reset);
    printf("Correctness : %d passed | %d failed | %d missing/build failed\n",required.passed,required.failed,required.missing);
    printf("Extra guards: %d passed | %d failed | %d missing/build failed\n",extra.passed,extra.failed,extra.missing);
    if(required.failed+required.missing+extra.failed+extra.missing) {
        printf("%sSTILL NEEDS FIXES%s\n",red,reset);
        return setup_errors ? 2 : 1;
    }
    printf("%sALL SELECTED TESTS PASSED%s\n",green,reset);
    return 0;
}
