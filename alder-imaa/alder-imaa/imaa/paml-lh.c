#include <stdio.h>
#include <math.h>
struct TREEB {
    int nbranch;
    int nnode;
    int root;
    int branches[20][2];
} tree;

struct TREEN {
    int father;
    int nson;
    int sons[2];
};

struct TREEN nodes[18];/* = {
    {-1, 0, {-1, -1}},
    {-1, 0, {-1, -1}},
    {-1, 0, {-1, -1}},
    {-1, 0, {-1, -1}},
    {-1, 0, {-1, -1}},
    { 5, 2, { 3,  4}},
    { 6, 2, { 0,  1}},
    { 7, 2, { 6,  2}},
    { 8, 2, { 7,  5}}
};*/

#define NS 10
#define FOR(i,n) for((i) = 0; (i) < (n); (i)++)
#define min2(a,b) ((a)<(b)?(a):(b))
#define max2(a,b) ((a)>(b)?(a):(b))

int GetIofLHistory (void)
{
    /* Get the index of the labelled history (rooted tree with nodes ordered
     according to time).  
     Numbering of nodes: node # increases as the node gets older:
     node d corresponds to time 2*ns-2-d; tree.root=ns*2-2;
     t0=1 > t1 > t2 > ... > t[ns-2]
     */
    int ns;
    int index, i,j,k[NS+1], inode,nnode, nodea[NS], s[2];
    
    ns = 9;
    for (i=0; i<ns; i++) nodea[i]=i;
    for (inode=nnode=ns,index=0; inode<ns*2-1; inode++,nnode--) {
        FOR (i,2) FOR (j,nnode)  
        if (nodes[inode].sons[i]==nodea[j]) { s[i]=j; break; }
        k[nnode]=max2(s[0],s[1]); s[0]=min2(s[0],s[1]); s[1]=k[nnode];
        k[nnode]=s[1]*(s[1]-1)/2+s[0];
        nodea[s[0]]=inode; nodea[s[1]]=nodea[nnode-1];
    }
    for (nnode=2,index=0; nnode<=ns; nnode++)
        index=nnode*(nnode-1)/2*index+k[nnode];
    return (index);
}

int GetLHistoryI (int iLH)
{
    /* Get the ILH_th labelled history.  This function is rather similar to 
     GetTreeI which returns the I_th rooted or unrooted tree topology.
     The labeled history is recorded in the numbering of nodes: 
     node # increases as the node gets older: 
     node d corresponds to time 2*ns-2-d; tree.root=ns*2-2;
     t0=1 > t1 > t2 > ... > t[ns-2]
     k ranges from 0 to i(i-1)/2 and indexes the pair (s1 & s2, with s1<s2),
     out of i lineages, that coalesce.
     */
    int i,k, inode, nodea[NS], s1, s2, it;
    
    int ns;
    
    ns = 9;
    tree.nnode=ns*2-1;
    for (i=0; i<tree.nnode; i++)  {
        nodes[i].father=nodes[i].nson=-1;  FOR (k,ns) nodes[i].sons[k]=-1;
    }
    for (i=0,inode=ns; i<ns; i++) nodea[i]=i;
    for (i=ns,it=iLH; i>=2; i--)  {
        k=it%(i*(i-1)/2);  it/=(i*(i-1)/2); 
        s2=(int)(sqrt(1.+8*k)-1)/2+1;  s1=k-s2*(s2-1)/2; /* s1<s2, k=s2*(s2-1)/2+s1 */
        
        if (s1>=s2 || s1<0)  printf("\nijk%6d%6d%6d", s1, s2, k);
        
        nodes[nodea[s1]].father=nodes[nodea[s2]].father=inode;
        nodes[inode].nson=2;
        nodes[inode].sons[0]=nodea[s1];
        nodes[inode].sons[1]=nodea[s2];
        nodea[s1]=inode;  nodea[s2]=nodea[i-1]; 
        inode++;
    }
    tree.root=inode-1;
    return (0);
}

/* 
 * signed int limit is 2,147,483,647
 * # Labeled History with 10 taxa is 2,571,912,000
 * # for 9 is 57,153,600
 */
int main (int argc, const char * argv[]) {
    // insert code here...
//    printf ("index is %d\n", GetIofLHistory());
    GetLHistoryI (57153599);
    printf ("index is %d\n", GetIofLHistory());    
    return 0;
}
