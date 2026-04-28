/* Maximum flow - highest lavel push-relabel algorithm */
/* COPYRIGHT C 1995, 2000 by IG Systems, Inc., igsys@eclipse.net */
  
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <values.h>
#include <math.h>

#include "types_treem.h"  /* type definitions */
#include "parser_treem.c" /* parser */
#include "timer.c"        /* timing routine */

#define MAX_LONG LONG_MAX

#define min(s, t) ((s) < (t) ? (s) : (t))
#define max(s, t) ((s) > (t) ? (s) : (t))







///////////////////////////////////////////
///////////////////////////////////////////The definition of functions
///////////////////////////////////////////


//The function for allocation
void *walloc(unsigned int num, unsigned int size)
{
  void *ptr = calloc(num, size);
  assert(ptr != NULL);
  return ptr;
}



RandomData* initrand2(RandomData *rd )
{
  srand((int)(timer() * 1000));

  rd->randNumIdx = 0;

  for (int i = 0; i < rd->maxLen; i++)
  {
    rd->randNums[i] = (cType)rand();
  }
  
  return rd;

}

RandomData* initrand(cType len)
{
  RandomData *rd = walloc(1, sizeof(RandomData));
  rd->maxLen = len;
  rd->randNums = (cType *)walloc(rd->maxLen, sizeof(cType));
  
  initrand2(rd);

  return rd;
}

//The function for randomization
cType mrand(RandomData* rd)
{
  if(rd->randNumIdx >= rd->maxLen){
		initrand2(rd);
  }	  
  return rd->randNums[rd->randNumIdx++];
}

#define NODEDTTMULTI  300
NodeDTTCover* getOneNodeDTTCover(PreprocData * pd){
  if(pd->gIdxFreePndc < pd->gd->N * NODEDTTMULTI){
    return pd->gFreePndc + (pd->gIdxFreePndc++);
  }
  else{
    assert(1==0);
  }
}

AnsAdjustRec* getOneAnsAdjust(PreprocData * pd){
  if(pd->gIdxAnsAdjustRec < pd->gd->N * NODEDTTMULTI){
    return pd->gFreeAnsAdjustRec + (pd->gIdxAnsAdjustRec++);
  }
  else{
    assert(1==0);
  }
}

/////////////////////////The two function for heap sorting edges 
void HeapAdjustDown(sType *idx, edgeP * edges ,int start,int end)  
{  
    sType tempIdx = idx[start];  

    int i = 2*start+1;      
    
    // assert(idx[0] != idx[3]);
    
    while(i<=end)  
    {  
        if(i+1<=end && edges[idx[i+1]].tmp > edges[idx[i]].tmp )    
            i++;  

        if(edges[idx[i]].tmp <= edges[tempIdx].tmp )   
            break;  

        idx[start] = idx[i];

        start = i;  
        i = 2*start+1;  
    }  

    idx[start] = tempIdx;  
}  
  
void HeapSort(sType *idx, edgeP * edges, int len)  
{  

    int i;  
    for(i=(len-1)/2;i>=0;i--){  
        HeapAdjustDown(idx,edges,i,len-1);  
    }

    for(i=len-1;i>0;i--)
    {  
        // printf("swap 0 with %d \n",i);
        sType temp = idx[i];  
        idx[i] = idx[0];  
        idx[0] = temp;  

        HeapAdjustDown(idx,edges,0,i-1);  
    }  

}  

/////////////////////////The two function for heap sorting edges 
void HeapAdjustDown2(sType *idx, edgeP ** edges ,int start,int end)  
{  
    sType tempIdx = idx[start];  

    int i = 2*start+1;      
    
    // assert(idx[0] != idx[3]);
    
    while(i<=end)  
    {  
        if(i+1<=end && edges[idx[i+1]]->tmp > edges[idx[i]]->tmp )    
            i++;  

        if(edges[idx[i]]->tmp <= edges[tempIdx]->tmp )   
            break;  

        idx[start] = idx[i];

        start = i;  
        i = 2*start+1;  
    }  

    idx[start] = tempIdx;  
}  
  
void HeapSort2(sType *idx, edgeP ** edges, int len)  
{  

    int i;  
    for(i=(len-1)/2;i>=0;i--){  
        HeapAdjustDown2(idx,edges,i,len-1);  
    }

    for(i=len-1;i>0;i--)
    {  
        // printf("swap 0 with %d \n",i);
        sType temp = idx[i];  
        idx[i] = idx[0];  
        idx[0] = temp;  

        HeapAdjustDown2(idx,edges,0,i-1);  
    }  

} 
  
/////////////////////The function to sort edges using capacity
void deOrderEdgeByRandomCap(nodeP *np,PreprocData *pd)
{

  cType cnt = np->nIdx;

  sType *idxs = np->orderedEdges;
  edgeP *pedges = np->edges;

  for (int i = 0; i < cnt; i++)
  {
    // pedges[i].tmp = -1*mrand(pd->rd) % (pedges[i].cap+1); //+1防止0的情况
    pedges[i].tmp = 1000-pedges[i].cap+1; //+1防止0的情况
  }

    assert(cnt<4 || idxs[2]!=idxs[3]);
    HeapSort(idxs,pedges,cnt);
    assert(cnt<4 || idxs[2]!=idxs[3]);  
}

///////////////////The function to sort edges using the value of currently minimal cut one edge belongs to 
void aOrderEdgeByAvgCV(nodeP *np,PreprocData *pd)
{
  if (np->nIdx == 0)
  {
    return;
  }
  int cnt = np->nIdx;

  sType *idxs = np->orderedEdges;
  edgeP *pedges = np->edges;

  for (int i = 0; i < cnt; i++)
  {
    edgeP *pp = pedges +i;
    long acv = pp->avgCV;
    if(acv == 0 ){
      pp->tmp = MAX_LONG;
    }
    else{
      pedges[i].tmp = mrand(pd->rd) % acv; 
    }
  }

    HeapSort(idxs,pedges,cnt);
}

///////////////////按度数升序访问 
void aOrderEdgeByDegree(nodeP *np,PreprocData *pd)
{
  if (np->nIdx == 0)
  {
    return;
  }
  int cnt = np->nIdx;
  nodeP* nodes = pd->gd->nodes;
  sType *idxs = np->orderedEdges;
  edgeP *pedges = np->edges;

  for (int i = 0; i < cnt; i++)
  {
    edgeP *pp = pedges +i;
    cType zn = pp->endNode;
    pedges[i].tmp =  -1 * (nodes+zn)->nIdx;//mrand(pd->rd) % acv; 
  }

    HeapSort(idxs,pedges,cnt);
}

///////////////////按邻居赋予序号访问：使得每个节点和祖先连接尽量短一些 
void aOrderEdgeBySeq(nodeP *np,PreprocData *pd)
{
  if (np->nIdx == 0)
  {
    return;
  }
  int cnt = np->nIdx;
  // nodeP* nodes = pd->gd->nodes;
  sType *idxs = np->orderedEdges;
  edgeP *pedges = np->edges;

  for (int i = 0; i < cnt; i++)
  {
    edgeP *pp = pedges +i;
    cType zn = pp->endNode;
    pedges[i].tmp =  pd->gpseq[zn];//mrand(pd->rd) % acv; 
  }
    HeapSort(idxs,pedges,cnt);
}

/////////////////////The function to sort edges using depth
void deOrderEdgeByDepth(nodeP *np,PreprocData *pd)
{

  cType cnt = np->nIdx;

  sType *idxs = np->orderedEdges;
  edgeP *pedges = np->edges;

  for (int i = 0; i < cnt; i++)
  {
    // pedges[i].tmp = -1*mrand(pd->rd) % (pedges[i].cap+1); //+1防止0的情况
    pedges[i].tmp = - pd->gpdep[pedges[i].endNode]; //+1防止0的情况
  }

    assert(cnt<4 || idxs[2]!=idxs[3]);
    HeapSort(idxs,pedges,cnt);
    assert(cnt<4 || idxs[2]!=idxs[3]);  
}

// //根据度数,但是同时
// void aOrderEdgeByDegreeAver(nodeP *np,PreprocData *pd)
// {
//   if (np->nIdx == 0)
//   {
//     return;
//   }
//   int cnt = np->nIdx;
//   nodeP* nodes = pd->gd->nodes;
//   sType *idxs = np->orderedEdges;
//   edgeP *pedges = np->edges;

//   for (int i = 0; i < cnt; i++)
//   {
//     edgeP *pp = pedges +i;
//     cType zn = pp->endNode;
//     pedges[i].tmp =  (nodes+zn)->nIdx;//mrand(pd->rd) % acv; 
//   }

//     HeapSort(idxs,pedges,cnt);
// }



/*
  the function to add more data to a traversal tree to accelerate the searching in the tree
  The idea is to precalcuate minimal cv value of a span of nodes in a traversal tree, e.g., when SPAN_LEN = 100, and a node has depth of 200, then the algorithm will pre-calculate the minimal cv value of the nodes between the node (dep=200) and an acestor(dep=101)
  upid is the id of the last SPAN node, mcv is the min cv among all previous nodes in the recent SPAN
  lastDepMCV is the depth of the node depth that has the minimal cv in the span
  lastJointNodeId is the last ancestor node id that has more than one child nodes
  lastJointMCV is the cv of lastJoineNodeId

  改版后预处理算法要有变化：
      对每个节点的值,都要改进下考虑当前出发节点z的考虑下游分支的更小的cv'=cv2+-cof

  (1)处理时：
      在非段头节点中还要考虑父亲cv'的值,如果更小,则更新指向父亲
      跨段的cv2+-cof直接放到下段mcv中,因为solve中确认跨段了才使用当前段的mcv
  (2)求解时：
      看solve的注释


*/

long gRoot = 0;
void buildAcc(PreprocData *pd, cType curN, cType upid, cType upid_LC, long mcv, long* applyAdj, cType* depMap)
{
  nodeP* nodes = pd->gd->nodes;
  assert(curN <= pd->gd->N && curN >= 1);
  int cnt = (nodes + curN)->nIdx;
  edgeP *pedges = (nodes + curN)->edges;

  long curCV = pd->gpcv[curN];
  cType curDep = pd->gpdep[curN];

  assert(curDep == 0 || curCV >0);

  depMap[curDep] = curN;
  pd->gpaccup[curN] = upid; //the previous segment tail

  int SPAN_LEN = pd->SPAN_LEN;
  int LN = pd->LEVEL_NUM;
  /******************************************/
  //作为祖先节点更新的点
  pd->gpaccup[curN] = upid; //the previous segment tail
  pd->gpaccposmcv[curN] = upid_LC; //the LN Child of previous segment tail
  
  if(curDep % SPAN_LEN == 0){
    upid = curN;
    upid_LC = 0; //节点id最小是1,所以0可以表示无效
  }
  else{
    //不需要更新什么,或者在后面逻辑中包含了

  }


  /****************************************/
  cType ances; 
  for (int i = 1; i <= pd->LEVEL_NUM; i++)
  {
    if (curDep  < (unsigned int)i)
    {
      break;
    }

    ances = depMap[curDep - i];
    assert(ances != 0);
    if (applyAdj[ances] == 0) //ances的applyAdj是0,说明未被祖先设置过
    {
      if (pd->gcutadjsign[curN][i] == 1 ) // 
      {
        // printf("update ApplyAdj level i %ld, curN %ld dep %ld, to set applyAdj[%ld] dep %ld to %ld\n",i,curN,pd->gpdep[curN], ances,pd->gpdep[ances],pd->gcutadj[curN][i]);
        applyAdj[ances] = pd->gcutadj[curN][i];
        pd->gcutadj[curN][i] = -1 * pd->gcutadj[curN][i];
      }
    }

  }

  //更新完,此时curN的LN祖先(最上面祖先)值,在这个支线上肯定不会再变了,可以尝试更新minCV了
  if(curDep >= LN ){
      if(curDep % SPAN_LEN == LN){
        upid_LC = curN; //更新作为祖先的upid_LC值
        if(LN > 0){
          //如果LN是0,这就是上个段尾,不适用此更新规则
          pd->gpaccposmcv[curN] = upid_LC; //把当前节点LC也设置为最新,后面好处理 
        }
      }
      else if(curDep % SPAN_LEN == (LN+1)){
        mcv = MAX_LONG;
      }

      assert(applyAdj[ances] <= 0);
      assert(pd->gcutadj[ances][0] <= 0);
      assert(pd->gpcv[ances]+pd->gcutadj[ances][0] > 0);
      //例行更新祖先值,这个和段无关,而且是循环
      ances = depMap[curDep-LN];
      // printf("applyAdj[ances] %ld\n",applyAdj[ances]);
      //curN的poh装的是LN祖先的在包含curN及其小祖先的基础上的树的最小割,因为applyAdj是负值或0
      //需要反向补充回去的目标值,应该不是祖先的pcv,而是祖先pcv还要小的,树被裁剪后的最小值,应此时cv+[ances][0]
      pd->gpoh[curN] = pd->gpcv[ances] + pd->gcutadj[ances][0] - applyAdj[ances]; 
      mcv = min(mcv, pd->gpoh[curN]);
      pd->gpaccmcv[curN] = mcv; //这个值记录的是LN祖先的该段的从段首到该祖先的所有割值的最小

  }
  else{
    //nothing to update

  }


  while (cnt > 0)
  {
    cType zn = pedges->endNode;
    if (pd->gpfa[zn] == curN)
    {
      buildAcc(pd,zn, upid, upid_LC, mcv, applyAdj, depMap);
    }
    pedges++;
    cnt--;
  }

  //恢复添加到祖先身上的值,恢复成0
  for (int i = 1; i <= pd->LEVEL_NUM; i++)
  {
    if (curDep  < (unsigned int)i)
    {
      break;
    }
    cType ances = depMap[curDep - i];
    // printf("curN %ld, dep %ld, ances %ld dep %ld\n",curN, pd->gpdep[curN], ances, pd->gpdep[ances]);
    // printf("level i %ld sign %ld, adj %ld  --  apladj %ld\n",i,pd->gcutadjsign[curN][i],pd->gcutadj[curN][i],applyAdj[ances] );
    assert(ances != 0);
    //这里是还结果的地方,之前赋值给applyAdj[ances],这里还回去
    if (pd->gcutadjsign[curN][i] == 1 && pd->gcutadj[curN][i] > 0)
    {
      pd->gcutadj[curN][i] = -1 * pd->gcutadj[curN][i];
      assert(applyAdj[ances] == pd->gcutadj[curN][i]);
      applyAdj[ances] = 0;
    }
  }

}

void calcuTotalCap(PreprocData *pd)
{
  nodeP *nodes = pd->gd->nodes;

  for (cType curN = 1; curN <= pd->gd->N; curN++)
  {
    nodeP *np = nodes + curN;
    edgeP *pedges = np->edges;
    int cnt = np->nIdx;
    np->totalCap = 0;
    for (int ni = 0; ni < cnt; ni++)
    {
      edgeP *eh = pedges + ni;
      np->totalCap += eh->cap;
    }
  }
}

/*

算法整体步骤：相对之前treem算法的改进
(1)预处理
	//按之前计算,加上一个值放到n上,代表对于n的father,如果去掉n这一支,f的mc变化
	sum_f = 0
	对f的每个为访问的child n:
		在遍历n前,在f上设置oh_f并置0
		在遍历计算中,每次访问祖先,除了计算mc,还更新oh_f (加上就可以)
		n返回后
			此时知道mc_n, oh_f(oh_f就是n这一支连到f的边的值,要包含f_n父子的边)
			这时计算n这一支去掉对f的mc的影响cof_n = oh_f - (mc_n-oh_f) = 2* oh_f - mc_n
				//oh_f是增加的割,mc_n-oh_f是之间到非f的割
			如果cof_n是负值 //说明割值降低了,值得庆贺
				就加到sum_f上,即sum_f 加上 负值,sum_f指的是f的所有减少割值的子n去掉,总共减少的割值
				如果不减少,这个n就不去掉
	//所有child遍历完之后计算mc2_f,就是f的最优的偏特树,此时这个树包含某些子树,而且mc是最小的
	得到mc2_f = mc_f + sum_f 
				
(2)计算时: solve 和 build的时候
	//因为mc2_f是已经去掉cof_n为负值的n的,正值就不管,还在mc_f中
	每次向上回溯,n回溯到f时,如果cof_n是负值,说明如果要保留n这一支,目前f最优割就包含n,即此时f用于计算的mc应该取(mc2_f + -cof_n),即加上n这一支减掉的值
  现在问题来了：
    f的mc2和访问哪一支有关系,buildAcc预处理咋做？
    这样就意味着,不同的底层上来,每个节点的mc还不一样,导致 节点段 中最小值还不一样
    buildAcc记录的是向上的,所以可以记录的呀

*/
// long level_cumsum[110]; //保存子的每个level的负值的和
//////////////////////////////////The function to traverse the graph for one pass, i.e. the checkNode function of Algorithm 2 in the paper

void debug_checkdv(PreprocData *pd, cType curN)
{
  nodeP *np = pd->gd->nodes + curN;
  edgeP *pedges = np->edges;
  int cnt = np->nIdx;
  long val = 0;
  cType* pdep = pd->gpdep;
  for (int ti = 0; ti < cnt; ti++)
  {
    edgeP *teh = pedges + ti;
    cType aa = teh->endNode;
    if (pdep[aa] == pdep[curN] + 1)
    {
      val += pd->gpdv[aa];

    }

    if(pdep[aa] == (pdep[curN]-1)){
      val += teh->cap;
    }
    else{
      val -= teh->cap;
    }
  }

  assert(pd->gpdv[curN] == val);
}


cType maxDep = 0;
// long descArr[100000]; //保存某个curN需要处理的下方后代的队列
// long descCap[100000]; //对应cap
long tempCap[5100000];
// long tempDVal[2100000][10];
// char tempDSgn[2100000][10];
#define PERNODE_ARRSIZE 3100000
NodeDTTCover* perNodeposPndc[PERNODE_ARRSIZE];
NodeDTTCover* perNodeposPndcSign0[PERNODE_ARRSIZE];
NodeDTTCover* perNodeposPndcLast[PERNODE_ARRSIZE];
long totalCount = 0;
struct timespec time_start={0,0};
void markCut(cType curN, PreprocData *pd)
{
  //debug
    // for(int i=0; i<10;i ++){
    //   tempDSgn[curN][i] = 0;
    //   tempDVal[curN][i] = 0;
    // }
  //end



  nodeP* nodes = pd->gd->nodes;
  assert(curN <= pd->gd->N && curN >= 1);

  // printf("******curN is %ld\n",curN);
  assert((nodes + curN)->nIdx > 0);

  short *curS = pd->gps + curN;

  assert(*curS == 0);

  long *curCV = pd->gpcv + curN;
  cType *curDep = pd->gpdep + curN;



  if(maxDep < *curDep){
	maxDep = *curDep;
  }

  *curS = 1;
  *curCV = 0;
  nodeP *np = nodes + curN;
  edgeP *pedges = np->edges;
  int cnt = np->nIdx;

  if(cnt < 2){
    printf("hhhhhhhhhhhhhhhhhhhhhhhhhhh!\n");
  }

  if (pedges == NULL)
  {
    *curS = 2;
    return;
  }

  if (np->orderedEdges == NULL)
  {
    np->orderedEdges = (sType *)walloc(cnt + 1, sizeof(sType));
    for (int i = 0; i < cnt; i++)
    {
      np->orderedEdges[i] = i;
    }
  }

  long cap;
  sType *idxs = np->orderedEdges;




  if (pd->mode == 1)
  {
    deOrderEdgeByRandomCap(np,pd); //
  }
  else if (pd->mode == 2)
  {
    aOrderEdgeByAvgCV(np,pd);
  }
  else if (pd->mode == 3){
    //度数最小最小先访问
    aOrderEdgeByDegree(np,pd);
  }
  else if(pd->mode == 5){
    //先更新序号
    for (int ni = 0; ni < cnt; ni++)
    {
      edgeP *eh = pedges + idxs[ni];
      cType zn = eh->endNode;
      short zs = pd->gps[zn];

      if(zs == 0){
        pd->gpseq[zn] = *curDep + 1;
      }
      else{
        assert(pd->gpseq[zn] > 0);
      }
    }    
    //按序号访问
    aOrderEdgeBySeq(np,pd);
  }
  else{
    //不做排序,按默认文件导入顺序访问

  }

  // long sum_f = 0;

  //多层遍历算法步骤：每次遍历前,记录上面几位祖先的值到cut_adjust中
  //问题：还没有遍历成功
  cType fa = curN;
  for(int i=1; i<=pd->LEVEL_NUM; i++ ){
    if(pd->gpdep[fa] == 0){
      break;
    }
    // printf("fa is %ld, fasfa is %ld,  dep is %ld\n",fa,pd->gpfa[fa],pd->gpdep[fa]);
    fa = pd->gpfa[fa];
    // printf("   -> fa is %ld, dep is %ld\n",fa,pd->gpdep[fa]);
    
    pd->gcutadj[curN][i] = pd->gpoh[fa]; 
  }

  edgeP *faEdge = NULL;

  for (int ni = 0; ni < cnt; ni++)
  {
    // nodeP* znp = nodes+eh->endNode;

    edgeP *eh = pedges + idxs[ni];
    cType zn = eh->endNode;

    assert(zn != 0);
    assert(zn != curN);

    short zs = pd->gps[zn];

    if((pd->gpfa[zn] == curN && zs == 2)){
      printf("------------ zn %ld curN %ld\n",zn, curN);
    }

    assert(!(pd->gpfa[zn] == curN && zs == 2));

    // printf("zn is %ld (curN %ld) \n",zn,curN);
    if (zs == 1) // zn is an ancestor node or father node
    {
      // printf("\t zs is 1, zn %ld , *cruCV %ld += %ld\n",zn,*curCV,cap);
      cap = eh->cap;
      *curCV += cap;
      pd->gpcv[zn] -= cap;
      pd->gpoh[zn] += cap;
      if(pd->gpdep[zn] == (*curDep-1)){
        faEdge = eh;
      }
    }
    else if (zs == 0) // zn is not accessed, i.e., a child node
    {
      //poh临时使用,后面会作为cv2使用：ph记录curN的当前儿子zn子树遍历中,连到curN的边的容量的和,就是直接和curN连接的割值。所以需要置0
      // pd->gpoh[curN] = 0;

      pd->gpfa[zn] = curN;
      pd->gplfa[zn] = curN;
      pd->gpdep[zn] = *curDep + 1;

      // printf("\t zs is 0, zn %ld , *cruCV %ld before markcut\n",zn, *curCV);
      markCut(zn,pd);

      //集中assert();


      //assert(pd->gpoh[curN] > 0);
      // printf("----marCut return \n");
      assert(pd->gpdep[zn] == pd->gpdep[curN] + 1);
      // printf("\t zs is %ld , zn %ld , *cruCV %ld += %ld after markcut\n",pd->gps[zn],zn, *curCV,pd->gpcv[zn]);
      *curCV += pd->gpcv[zn];

//       //此时：gpoh[curN]是zn子树连到curN的边总容量,即zn树所有节点脱离curN树后,curN树多出来的割
//         //zn树不连其他curN的子的树,只连curN或上面祖先
//       // pd->gpcv[zn] - pd->gpoh[curN]是zn子树在curN之上的割值, 即zn子树去掉后,curN减少的割值
		 // pd->gpoh[curN] 就是zn子树去掉后,增加的割值
         //cof_zn 就是zn树去掉后,整体增加的割值,如果小于0,就可以去掉
      // long cof_zn = pd->gpoh[curN] - ( pd->gpcv[zn] - pd->gpoh[curN]);
      // if(cof_zn < 0){ //如果去掉能进一步优化割值(减少),则记录
      //   sum_f += cof_zn;
      // }
      // assert(pd->gpcv[zn] >= pd->gpoh[curN]);
      // pd->gpcof[zn] = cof_zn;
      // pd->gpcof[zn] = pd->gpoh[curN];

    }
    else
    {
      //这种情况就是,zn是curN的子树的叶子,正好连到curN有条边
      // printf("zs is %ld\n",zs);
      assert(pd->gpdep[curN] < pd->gpdep[zn]);
      
    }

  }

  // if (pd->gpoh[curN] == 0)
  // {
  //   printf("poh error: (cnt is %d ) curN is %ld (gRoot is %ld), cv is %ld, dep is %ld \n", cnt, curN, gRoot, *curCV, pd->gpdep[curN]);
  //   assert(1 == 2);
  // }
  //自己有没算进去？遍历所有邻居就是把自己算进去了
  //------------多层遍历算法步骤：每次遍历后,更新祖先增加的值到cut_adjust中
  fa = curN;
  cType faBelow = 0; //连fa到curN之前的容量值
  int actual_ln = -1;
  for(int i=1; i<=pd->LEVEL_NUM; i++ ){
    // printf("fa is %ld, dep is %ld\n",fa,pd->gpdep[fa]);
  
    if(pd->gpdep[fa] == 0){ //到root了,如果i=1说明自己就是root
	
      actual_ln = i-1;
      break;
    }

    fa = pd->gpfa[fa];
    //此时gcutadj保留的是curN中到fa这个祖先节点的边容量总和, 如果紧挨着,则包含上连边；否则不包含上连边
    pd->gcutadj[curN][i] = pd->gpoh[fa] - pd->gcutadj[curN][i];
    //(1) 下面计算后的faBelow是curN子树所有cut到这层祖先fa内部(包含fa)到curN前的(不包含curN)的边容量总和, faBelow包含curN的上连边
    faBelow += pd->gcutadj[curN][i];
    //(2) 而curN到fa之外(不包含fa)的cut值为  *curV - faBelow;
    //则：(1)-(2)的值是curN的子树全部去除后,对相应祖先fa的cut的增加值,负值更好; 
    pd->gcutadj[curN][i] = faBelow - (*curCV - faBelow);    
  
  }

  //actual_ln说明curN最大可以作为哪一层？ 0就是没有(curN就是root)
  if(actual_ln < 0){
    actual_ln = pd->LEVEL_NUM;
  }

  //如果level_num=5但是actual_ln=2,说明当前节点最多只能作为第2层,而且就是底层了
  //作为底层

  //作为最底层
  // if(pd->gcutadj[curN][actual_ln] < 0){
  //   pd->gcutadjsign[curN][actual_ln] = 1;
  // }

  //----------多层遍历算法步骤：

  /*
	下面记录curN对不同层的祖先,是否移除以及移除对祖先cv的改变
	如果actual_ln=1,说明curN的父亲就是root
  */
  // assert(actual_ln < 20);//给个限制,最多20层
  // memset(level_cumsum,0,(pd->LEVEL_NUM+1)*sizeof(long));
  //现在比较curN的i层和子的i+1层之间的值,两层的这两个值对应的祖先是同一个
  // long tempCumSum = 0;
  // pd->gcutadj[curN][0] = 0; //0层记录的是自己对自己的cv值的改变,或者说整个树中通过裁剪能达到的最大的可减少负值(保留自己为根的前提下),该负值针对curN为根的树

  // //特殊处理：对于L层祖先,要根据负值设置sign
  // if(actual_ln == pd->LEVEL_NUM){
	//   if(pd->gcutadj[curN][actual_ln] < 0){
	// 	  pd->gcutadjsign[curN][actual_ln] = 1; //curN不会删除子孙了,如果<0就是移除他自己
	//   }
  // }
	
  // //for (int i = actual_ln - 1; i >= 0; i--) //只能从curN的level-1开始,因为子是要level开始；i可以为0,因为对应子时i+1层; 
  // for (int i = (actual_ln < pd->LEVEL_NUM ? actual_ln : actual_ln - 1); i >= 0; i--) //只能从curN的level-1开始,因为子是要level开始；i可以为0,因为对应子时i+1层; 如果actual_ln< L, 子可以比actual_ln大,否则得从L-1开始 
  // {
	// //对curN的i层祖先,计算子对该祖先的原始cv的负值的总和,存在这里
	// //i=0时,就是对自己的影响
	// tempCumSum = 0;

  //   //计算整个curN的树,若移除对curN的 i层祖先的CV的影响(就是子对其i+1层祖先),所有负值保存在tempCumSum中；如果i=0,就是对自己的影响
  //   for (int ni = 0; ni < cnt; ni++)
  //   {
  //     edgeP *eh = pedges + idxs[ni];
  //     cType zn = eh->endNode;
  //     assert(zn != 0);
  //     assert(zn != curN);

  //     if (pd->gpfa[zn] != curN)
  //     {
  //       continue;
  //     }

  //     if (pd->gcutadj[zn][i + 1] < 0)
  //     {
	// 	  // if(curN == 820812){ //TODEL including inside
	// 		// printf("curN %ld child zn %ld  val %ld sign %d \n",curN,zn, pd->gcutadj[zn][i+1],pd->gcutadjsign[zn][i+1]);
	// 	  // }
  //       tempCumSum += pd->gcutadj[zn][i + 1];
		
  //     }
  //   }

	// // if(curN == 820812){ //TODEL 包括里面
	// 	//printf("i %d, curN %ld, level_cumsum[ %d ] %ld, cutadj %ld \n",i,curN,i+1,tempCumSum,pd->gcutadj[curN][i]);
	// // }
	
  //   if (pd->gcutadj[curN][i] < tempCumSum) //tempCumSum是全部子的负值的总和(对祖先可减少的总负值)
  //   {
  //     // if(curN == 9904 && i == 4){
  //     //   printf("curN 9904 i=4 set sign self 1");
  //     // }
  //     pd->gcutadjsign[curN][i] = 1; //说明curN子树中,全树全部移除反而减少的更多
  //     // pd->gcutadjsign[curN][i+1] = 0; //设置的不是curN,应该是zn
  //   }
  //   else
  //   {

  //     // if(curN == 9904 && i == 4){
  //     //   printf("curN 9904 i=4 set sign children 1");
  //     // }
  //     // pd->gcutadj[curN][i] = tempCumSum; //TODO: 恢复
  //     pd->gcutadjsign[curN][i] = 0; //虽然用了子孙中所有负值,但是设0表示没有用自己全树参与
  //     // pd->gcutadjsign[curN][i+1] = 1;
  //   }

  // }


  // if(pd->gpoh[curN] == 0 && curN != gRoot){ //当邻居全是祖先是有可能的,已经在上面加了判断
  //   printf("poh error: (cnt is %d ) curN is %ld (gRoot is %ld), cv is %ld, dep is %ld \n",cnt,curN, gRoot,*curCV,pd->gpdep[curN]);
  //   assert(1==2);
  // }
  if(*curCV == 0 && curN != gRoot){
    printf("cv=0 error: (cnt is %d) curN is %ld (gRoot is %ld), cv is %ld, dep is %ld \n",cnt,curN, gRoot,*curCV,pd->gpdep[curN]);
    assert(1==2);
  }

  // assert(pd->gpoh[curN] >0);

  if(pd->mode == 1){
//update ver 2 w,according to
    for (int ni = 0; ni < cnt; ni++)
    {
      // nodeP* znp = nodes+eh->endNode;

      edgeP *eh = pedges + idxs[ni];
      cType zn = eh->endNode;
      // nodeP *znp = nodes+zn;

      assert(zn != 0);
      assert(zn != curN);
      short zs = pd->gps[zn];
    

      //progate weight to curN's edges
      if (zs == 1 && pd->gpdep[zn] != *curDep - 1)
      {
          cType weight = eh-> w;
          if(eh->avgCV == 0){
            eh->avgCV = MAX_LONG;
          }
          eh->avgCV = min(eh->avgCV, *curCV);//((eh->avgCV) * weight + *curCV)/(weight+1);
          eh->w = weight+1;
          
          edgeP *reh = eh->rev;

          if(reh->avgCV == 0){
            reh->avgCV = MAX_LONG;
          }

          weight = reh-> w;
          reh->avgCV = min(reh->avgCV, *curCV);//((reh->avgCV) * weight + *curCV)/(weight+1);
          reh->w = weight+1;

      }
      
    }
  }

    
  assert(pd->gpdep[curN] == 0 || *curCV >0);

// BEGIN of anylevel logic
// #define REP_OPT 1
// #define TEST_MDV 1
// #define TEST_DV 1

  int descIdx = 0; //保存需要调整的后代的数组descArr的下标
  // printf("####c now begin node %ld\n",curN);
  assert(faEdge != NULL || curN == gRoot);
  cap = 0;
  if (faEdge != NULL)
  { cap = faEdge->cap;}

    // dv就计算和父亲的吧,不要算自己的

    //以父亲为目标初始化3个值： odv和dv最开始就是对父亲的(对自己没意义,对自己用mdv和cv相加就够了,mdv就是个差值)
    //最初的dv是啥？是节点DTT对父亲的差分
     pd->gpdv[curN] = 0 - pd->gpcv[curN];
    //odv就是自己的DTT内各种最优组合,对父亲的
     pd->gpmdv[curN] = 0;
    //mdv就很好理解了,就是各个子节点对当前这个节点最优的组合,这个是给自己的

    //用最新数据,最新的odv已经是考虑curN连下面后代更新后的结果
    // int ucnt = 0;
    for (int ni = 0; ni < cnt; ni++)
    {
      // nodeP* znp = nodes+eh->endNode;
      edgeP *eh = pedges + idxs[ni];
      cType zn = eh->endNode;
      if (pd->gpdep[zn] < *curDep)
      {
        continue;
      } // no ancestor
        
      // descArr[descIdx] = zn;
      // descCap[descIdx] = eh->cap;
      pd->gpcv[zn] = -pd->gpcv[zn];
      tempCap[zn] = eh->cap;

      descIdx++;

      if (pd->gpdep[zn] == (*curDep + 1))
      {
        //直接子现在就计算
        // printf("c node %ld before mdv %ld\n",curN, pd->gpmdv[curN]);

        pd->gpmdv[curN] += min(0, pd->gpodv[zn]); //这个也不是对自己的最优,因为zn的函数中,并没有考虑父亲下来的另外的边,只是保证zn自己的子odv更新对了,于是只有zn自己记录所有子的mdv也对了
        //下面就需要：zn的mdv更新为相对curN父的,zn的dv也更新为相对zn父的,所有更新完毕后, curN的mdv至少是对的了
        // printf("c node %ld after mdv %ld\n",curN, pd->gpmdv[curN]);
      }
      else
      {
        //do nothing
      }
    }



    //这个odv不准确,因为mdv现在还没考虑父下连边,mdv只是考虑父-子边的,未来可能差值会变小
    pd->gpodv[curN] = min(pd->gpdv[curN], pd->gpmdv[curN]);



    //下面这个操作没有任何意义！因为mdv对自己,dv对父亲


    // 父亲访问都没有完全结束,你只能用在你的DTT里的节点,这个就有点搞了
    // 下面的逻辑其实也很简单,就是更新curN的mdv为指向父亲的值,同时dv和odv都更新
    // 这个逻辑放到父亲调用里做有没关系
  //use the neighbor desendant of father of curN
  // nodeP *np = nodes + pd->gpfa[curN];
  // edgeP *pedgesf = npf->edges;
  // int cntf = npf->nIdx;
  // sType *idxsf = npf->orderedEdges;

    // for(int ni = 0; ni <cnt; ni++ ){
    //   edgeP *eh = pedges + idxs[ni];
    //   cType zn = eh->endNode;
    //   if (pd->gpdep[zn] < *curDep)
    //   {
    //     continue;
    //   } // no ancestor

    //   if (pd->gpdep[zn] > (*curDep+1)) //不是子的后代
    //   {
    //     pd->gpcv[zn] = -pd->gpcv[zn];
    //     tempCap[zn] = eh->cap;
    //     ucnt++; //非直接子的数量,即后代的数据。如果是0,就没必要继续了
    //   }      
    // }


    


    //下面逻辑主要是更新子的odv,导致更新当前节点的mdv[curN]为子孙向父亲的差分,
    // printf("c init vars node %ld dv(to fa) %ld mdv(to itself) %ld odv(to fa) %ld ucnt %ld \n", curN, pd->gpdv[curN], pd->gpmdv[curN], pd->gpodv[curN],ucnt);

    if (descIdx > 0) 
    {
      //按深度顺序大到小排序
      deOrderEdgeByDepth(np, pd);

      // cType cDep = pd->gpdep[curN];

      for (int ni = 0; ni <cnt; ni++)
      {
        // edgeP *eh = pedges + idxs[ni];
        // nodeP* znp = nodes+eh->endNode;
       
        edgeP *eh = pedges + idxs[ni];
        cType zn = eh->endNode;
        if (pd->gpdep[zn] < *curDep)
        {
          continue;
        } // no ancestor

        if (pd->gpcv[zn] > 0)
        {
          continue;
        }
        //正常应该是负, 但如果变正,说明已经处理过了

        short zs = pd->gps[zn];

        assert(zs == 2);

        // cType leastZn = 0;

        long adv = 0, ladv = 0;//, w = -1;
        // w = -1;
        while (1)
        {
          if (pd->gpcv[zn] < 0) // it is a neighbor of curN, <0 is a cheap way to indicate this relationship
          {
            adv += 2 * tempCap[zn]; //找到zn和当前curN的边的cap
            pd->gpcv[zn] = 0 - pd->gpcv[zn]; //just remove the neighbor sign
            // //连子的时候初始化
            // if(pd->gpdep[zn] == *curDep + 1){
            //   //可以初始化其他变量, 相对于当前父curN的
            //   pd->gpmdv[zn] =
            //   pd->gpodv[zn] =
            // }
          }
          // printf("\n----c curN %ld before update zn %ld, dv %ld mdv %ld odv %ld adv %ld ladv %ld\n",curN, zn, pd->gpdv[zn], pd->gpmdv[zn], pd->gpodv[zn], adv, ladv);
          // 更新代码包含两块,一个是叶子的,一个是初始连子时

          // 这时的pdv只是更新了一部分的值, 有可能没有更新彻底, 这时拿来计算mdv就会出现问题
          // 所有更新完之后的dv才是真正的dv, 此时用来计算odv才对,
          // dv好说,等所有后代都上溯完了,就是dv
          // mdv怎么办？odv不确定,没法计算mdv； 而odv需要依赖dv最终确定才能计算
          // 只能先从多个子更新沿路dv
          // 再重新走这些路径,逐个计算odv的变化, 这个变化就会传到到父的mdv,因此要更新他


          // printf("%ld += %ld\n",pd->gpdv[zn],adv);
          pd->gpdv[zn] += adv;

          //new way 2
          // pd->gpmdv[zn] = 0;
          //       nodeP *np = nodes + zn;
          //       edgeP *pedges = np->edges;
          //       int cnt = np->nIdx;
          //       for (int ti = 0; ti < cnt; ti++)
          //       {
          //         edgeP *teh = pedges + ti;
          //         cType aa = teh->endNode;
          //         if (pd->gpdep[aa] == pd->gpdep[zn] + 1)
          //         {
          //           if(pd->gpodv[aa] < 0){
          //             pd->gpmdv[zn] += pd->gpodv[aa];
          //           }
          //         }
          //       }

          // if (pd->gpdv[zn] < pd->gpmdv[zn])
          // {
          //   pd->gpoSgn[zn] = 1, pd->gpodv[zn] = pd->gpdv[zn];
          // }
          // else
          // {
          //   pd->gpoSgn[zn] = 0, pd->gpodv[zn] = pd->gpmdv[zn];
          // }
          //end new way 2

          pd->gpmdv[zn] += ladv; //做更新,但仍然指向自己, 所有更新完才是指向父curN

          ladv = pd->gpodv[zn];
          // aType oldSign = pd->gpoSgn[zn];

          if (pd->gpdv[zn] < pd->gpmdv[zn])
          {
            pd->gpoSgn[zn] = 1, pd->gpodv[zn] = pd->gpdv[zn];
          }
          else
          {
            pd->gpoSgn[zn] = 0, pd->gpodv[zn] = pd->gpmdv[zn];
          }

          // cType lvl = pd->gpdep[zn] - pd->gpdep[curN];
          // if( lvl <= pd->LEVEL_NUM){
          //   pd->gcutadjsign[zn][lvl] = pd->gpoSgn[zn];
          //   pd->gcutadj[zn][lvl] = pd->gpodv[zn];    
          // }

          NodeDTTCover *pndc;

          ///////////////////////////////////////////////////////
          ///////////////////////////////////////////////////////
          ///////////////////////////////////////////////////////type-1 chain 仅仅sign=1的祖先
          //////////////////////////////////////////////////////用来判断某个根节点某个分支最近的sign=1
          ///////////////////////////////////////////////////////
          ///////////////////////////////////////////////////////
          /*
          每个点zn
            对祖先y开始 sign=1后，就一直是1，直到变成0后再也不出现了
            zn的pndc链记录的是sign=1的这一段祖先里，对每个祖先的值dv发生变化时的情况
                    */
          
          //--------------是每个zn的sign=1的区间内的dv变化的信息，这里sgn0转1转0，并不是针对某个zn的，而只是curN标记某个zn是sign
          cType cDep = pd->gpdep[curN];
          //能不能找每条路上最近的W条，W是分支总数-1
          //就是找最靠近的几个
          if (pd->gpoSgn[zn] == 1)
          {
            // if( (leastZn == 0 && pd->gpdv[zn] < 0) || (leastZn !=0 && pd->gpdv[leastZn] > pd->gpdv[zn])){
            //   leastZn = zn;
            // }

            // least方式会导致漏掉，比如之前找的least,同个curN下由于sign=0取消了，但是如果有初次sign=1的节点就没有加进来，而且他本来是最近的一个，现在没有了
            //如果想简化区间，可以再遍历一次，但是可能会导致链分列，可能又会导致更多，而不是更少；因为我们链本来就是压缩的
            //如果取分支个数量的least，也不一定没问题，因为分支1比如5个最负的，分支2合并进来可能一次把6个都变成sign=0，这样还是会造成遗漏。

            if (pd->gperNodeheadPndc[zn] == NULL)
            {
              pndc = getOneNodeDTTCover(pd);
              pndc->dep = pd->gpdep[curN];
              pndc->dv = pd->gpdv[zn]; //最近的sign=1的祖先，想对他的dv的值，后续可能会变化，我们实时计算吧，也没办法
              pd->gperNodeheadPndc[zn] = pndc;
              perNodeposPndc[zn] = pndc; //当前位置
            }
            else
            {
              assert(perNodeposPndc[zn] != NULL);
              assert(perNodeposPndc[zn]->dv != MAX_LONG); // must have no tail, or something wrong.
              if (perNodeposPndc[zn]->dep > cDep)
              {
                if (pd->gpdv[zn] == perNodeposPndc[zn]->dv)
                {
                  // do nothing,因为我们这个是用紧挨着两个节点标注相同值的范围，最后会加end节点
                }
                else
                {
                  //添加一个新段
                  pndc = getOneNodeDTTCover(pd);
                  pndc->dep = pd->gpdep[curN];
                  pndc->dv = pd->gpdv[zn]; //最近的sign=1的祖先，想对他的dv的值，后续可能会变化，我们实时计算吧，也没办法
                  perNodeposPndc[zn]->next = pndc;
                  perNodeposPndcLast[zn] = perNodeposPndc[zn];
                  perNodeposPndc[zn] = pndc;
                }
              }
              else if (perNodeposPndc[zn]->dep == cDep)
              {
                //又回来了,可能curN连的下层节点，向上回溯有重复的段
                perNodeposPndc[zn]->dv = pd->gpdv[zn]; // 不管是否变化，赋上就行
              }
              else
              {
                assert(2 == 1); // 不可能
              }
            }
          }
          else{
            //针对sign=1时的收尾逻辑
            if(pd->gperNodeheadPndc[zn] == NULL || perNodeposPndc[zn]->dv == MAX_LONG){
              //do nothing
            }
            else{
                assert(perNodeposPndc[zn] != NULL);

                if(perNodeposPndc[zn]->dep  == cDep){
                  //存在交叉点，导致这个zn之前sign=1，但同个curN另个分支更新后不香了。sign=1变成sign=0了
                  if(pd->gperNodeheadPndc[zn] == perNodeposPndc[zn]){
                    pd->gperNodeheadPndc[zn] = perNodeposPndc[zn] = NULL;
                  }
                  else{
                    assert(perNodeposPndcLast[zn] != NULL && perNodeposPndcLast[zn]->next == perNodeposPndc[zn]);
                    perNodeposPndcLast[zn]->next = NULL;
                    perNodeposPndc[zn] = perNodeposPndcLast[zn];
                    perNodeposPndcLast[zn] = NULL;
                  }
                }

                perNodeposPndcLast[zn] = NULL;
                if (pd->gperNodeheadPndc[zn] != NULL) //如果空就不用加结尾节点了
                {
                  pndc = getOneNodeDTTCover(pd);
                  pndc->dep = pd->gpdep[curN];
                  pndc->dv = MAX_LONG; //结束链节
                  perNodeposPndc[zn]->next = pndc;
                  perNodeposPndc[zn] = pndc;
                }
            }
          }
            /////////////////////////////////////////////////////////////
            /////////////////////////////////////////////////////////////
            /////////////////////////////////////////////////////////////
            /////////////////////////////////type-2 chain，即进行。无论sign=0或1，都可以加入
            ////////////////////////////////记录的是zn对祖先们的mdv,直到变成正
            ////////////////////////////////mdv就是zn裁剪后能达到的对祖先的最大负变化，不包含zn这个DTT整个删除
            /////////////////////////////////////////////////////////////
            /////////////////////////////////////////////////////////////
            //↓-------------这里还可以记录mdv的信息，用户改进算法性能
            // if (pd->gpmdv[zn] >= 0) //收尾
            // {
            //   //可以断定，至少变成>=0，后面不可能再变负
            //   if(pd->gperNodeheadPndcSign0[zn] == NULL || perNodeposPndcSign0[zn]->dv == MAX_LONG){
            //     //do nothing
            //   }    
            //   else{
            //       pndc = getOneNodeDTTCover(pd);
            //       pndc->dep = pd->gpdep[curN];
            //       pndc->dv = MAX_LONG; //结束链节
            //       perNodeposPndcSign0[zn]->next = pndc;
            //       perNodeposPndcSign0[zn] = pndc;
            //   }              
            // }
            // else
            // {
            //   if (pd->gperNodeheadPndcSign0[zn] == NULL)
            //   { //初始创建For zn
            //     pndc = getOneNodeDTTCover(pd);
            //     pndc->dep = pd->gpdep[curN];
            //     pndc->dv = pd->gpmdv[zn]; //最近的sign=1的祖先，想对他的dv的值，后续可能会变化，我们实时计算吧，也没办法
            //     pd->gperNodeheadPndcSign0[zn] = pndc;
            //     perNodeposPndcSign0[zn] = pndc; //当前位置
            //   }
            //   else
            //   {
            //     //如果不是空，说明之前有过，这怎么办？没关系，先检查sign=1的，sign=0再说这边
            //     assert(perNodeposPndcSign0[zn] != NULL);
            //     assert(perNodeposPndcSign0[zn]->dv != MAX_LONG); // 不可能是尾
            //     //看有没出阶段
            //     if (perNodeposPndcSign0[zn]->dep > cDep)
            //     {
            //       if (pd->gpmdv[zn] == perNodeposPndcSign0[zn]->dv)
            //       {
            //         // do nothing 因为不用新建段
            //       }
            //       else
            //       {
            //           assert(pd->gpmdv[zn] > perNodeposPndcSign0[zn]->dv);
            //           //添加一个新段
            //           pndc = getOneNodeDTTCover(pd);
            //           pndc->dep = pd->gpdep[curN];
            //           pndc->dv = pd->gpmdv[zn]; //最近的sign=1的祖先，想对他的dv的值，后续可能会变化，我们实时计算吧，也没办法
            //           perNodeposPndcSign0[zn]->next = pndc;
            //           perNodeposPndcSign0[zn] = pndc;

            //       }
            //     }
            //     else if (perNodeposPndcSign0[zn]->dep == cDep)
            //     {
            //       //又回来了,可能curN连的下层节点，向上回溯有重复的段
            //       perNodeposPndcSign0[zn]->dv = pd->gpmdv[zn]; // 不管是否变化，赋上就行
            //     }
            //     else
            //     {
            //       assert(2 == 1); // 不可能
            //     }
            //   }
            // }
          


          if (pd->gpodv[zn] < ladv)
          {
            printf("c error zn %ld has odv %ld < previous odv %ld, with dv %ld mdv %ld\n", zn, pd->gpodv[zn], ladv, pd->gpdv[zn], pd->gpmdv[zn]);
          }

          // printf("    c node %ld  odv %ld  -  ladv %ld\n", zn, pd->gpodv[zn], ladv);
          ladv = min(0, pd->gpodv[zn]) - min(0, ladv);
          // printf(" after ladv %ld\n",ladv);

          // printf("c curN %ld: node %ld updated as dv %d  mdv %ld odv %ld  gen ladv %ld \n\n", curN, zn, pd->gpdv[zn], pd->gpmdv[zn], pd->gpodv[zn], ladv);
          assert(ladv >= 0);



          if (pd->gplfa[zn] == curN)
          {
            break;
          }
          else{

          }

          zn = pd->gplfa[zn];
        }

        //得到本次循环的最小，进行更新
        // zn = leastZn;
        // NodeDTTCover* pndc = NULL;
        // if (leastZn != 0)
        // {

        // }

        //此时zn的mdv指向父,dv和odv也都更新为考虑父的边的信息
        // 指向父了吗？什么叫指向父,什么叫不指向父？
        //只要dv变成向父,mdv考虑了子向父的odv变化,mdv也是向父的
        //因为父亲下来的边,考虑到了所有沿路点的里面。之前只是考虑中父-子边,这时考虑父亲下到后代中的边,此时mdv就是考虑所有和父连的边后形成的odv, 对应父亲的mdv也就更准确了
        
        // pd->gpodv[zn] = min(pd->gpdv[zn], pd->gpmdv[zn]); 多余,里面都已经计算过了
        

        //对父的ladv也更新了,也更新为指向父后的mdv
        pd->gpmdv[curN] += ladv;
        // printf("zn is %ld (curN %ld) \n",zn,curN);
      }

//new way 2
          // pd->gpmdv[curN] = 0;
          //       nodeP *np = nodes + curN;
          //       edgeP *pedges = np->edges;
          //       int cnt = np->nIdx;
          //       for (int ti = 0; ti < cnt; ti++)
          //       {
          //         edgeP *teh = pedges + ti;
          //         cType aa = teh->endNode;
          //         if (pd->gpdep[aa] == pd->gpdep[curN] + 1)
          //         {
          //           if(pd->gpodv[aa] < 0){
          //             pd->gpmdv[curN] += pd->gpodv[aa];
          //           }
          //         }
          //       }


//end new way 2

//debug: 看所有计算的mdv这些是否符合实际定义，等所有都结束来计算
#ifdef TEST_MDV
                for (int ni = 0; ni < cnt; ni++)
                {
                  // edgeP *eh = pedges + idxs[ni];
                  // nodeP* znp = nodes+eh->endNode;

                  edgeP *eh = pedges + idxs[ni];
                  cType zn = eh->endNode;
                  if (pd->gpdep[zn] < *curDep)
                  {
                    continue;
                  } // no ancestor

                  short zs = pd->gps[zn];

                  assert(zs == 2);

                  //每个节点向上重新计算看是否正确

                  while (1)
                  {

                    long mdv = 0;
                    nodeP *np = nodes + zn;
                    edgeP *pedges = np->edges;
                    int cnt = np->nIdx;
                    for (int ti = 0; ti < cnt; ti++)
                    {
                      edgeP *teh = pedges + ti;
                      cType aa = teh->endNode;
                      if (pd->gpdep[aa] == pd->gpdep[zn] + 1)
                      {
                        if (pd->gpodv[aa] < 0)
                        {
                          mdv += pd->gpodv[aa];
                        }
                      }
                    }
                    if(mdv != pd->gpmdv[zn]){
                      printf("================error MDV node %ld checked mdv %ld != pd->gpmdv[] %ld\n", zn, mdv, pd->gpmdv[zn]);
                      assert(mdv == pd->gpmdv[zn]);
                    }
                    // if (pd->gpdv[zn] < pd->gpmdv[zn])
                    // {
                    //   pd->gpoSgn[zn] = 1, pd->gpodv[zn] = pd->gpdv[zn];
                    // }
                    // else
                    // {
                    //   pd->gpoSgn[zn] = 0, pd->gpodv[zn] = pd->gpmdv[zn];
                    // }

                    if (pd->gplfa[zn] == curN)
                    {
                      break;
                    }

                    zn = pd->gplfa[zn];
                  }
                }
#endif
//end

      
              // if(pd->gpdep[curN] % 10000 == 0){
              //   printf("c finished node %ld with dep %ld\n",curN, pd->gpdep[curN]);
              // }



    }

    //debug
    // if(curN == 801138){
      //debug: 测试dv值是否符合规则
      // printf("-----------------------hehehe for curN node %ld\n",curN);
      // printPDTT(pd,curN,curN);
#ifdef TEST_DV
                nodeP *np2 = nodes + curN;
                edgeP *pedges2 = np2->edges;
                int cnt2 = np2->nIdx;
                for (int ti2 = 0; ti2 < cnt2; ti2++)
                {
                  edgeP *teh2 = pedges2 + ti2;
                  cType aa2 = teh2->endNode;
                  if (pd->gpdep[aa2] == pd->gpdep[curN] + 1)
                  {
                    debug_checkdv(pd,aa2); //用程序做检查
                  //printf("%ld的dv: \n %ld的所有子的dv和: \n -%ld到非%ld的边：\n +%ld到%ld的边： \n ........\n",aa2,aa2,aa2,curN,aa2,curN);  
                  }
                }
#endif
      // printf("---------hehe done\n");

      // print_markCutProp(pd,797187); printf("\n");
      // printPDTT(pd,801138,801138);

    // }
    //end of debug

  //now for curN, mdv is right
  if(pd->gpmdv[curN] - pd->gcutadj[curN][0] < 0){
      // printf("%%%%%%%%%%%%%%%%c diffnew node %ld dep %ld (cv %ld) %ld = %ld - %ld\n", curN, pd->gpdep[curN], pd->gpcv[curN], pd->gpmdv[curN] - pd->gcutadj[curN][0], pd->gpmdv[curN], pd->gcutadj[curN][0]);
  }
    if (pd->gpmdv[curN] > pd->gcutadj[curN][0] || pd->gpmdv[curN] + pd->gpcv[curN]  < 0)
    {
      printf("c ERRRRRRRRRRRRRRRRRRR@$@#$@#$#@$@#$@$@#$!!!!!!\n");
    }

    assert(pd->gpmdv[curN] + pd->gpcv[curN] >= 0);
    //到这里后 mdv[curN] 是自己的
    assert(pd->gpmdv[curN] <= pd->gcutadj[curN][0]);

//重要：
  pd->gcutadj[curN][0] = pd->gpmdv[curN];
#ifdef  REP_OPT

  // if(pd->gcutadj[curN][0] == -368563){
  //   printf("------------val-368563: ");
  //   print_markCutProp(pd,curN);
  //   printf("\n");
  // }
  // printf("update it\n");
#endif

if(faEdge == NULL){
  assert(pd->gpcv[curN] == 0);
  pd->gpmdv[curN] = pd->gpodv[curN] = 0;
  for(cType ii=1; ii<=pd->gd->N; ii++){
    // if(pd->gpdv[ii] == pd->gpcv[ii]){
    //   printf("c  node %ld assert problem\n",ii);
    // }

    // printf("c STAT node %ld (fa %ld, dep %ld ): cv %ld, dv %ld,  mdv %ld,   odv %ld,  sign %d\n",ii,pd->gpfa[ii],pd->gpdep[ii],pd->gpcv[ii],pd->gpdv[ii],pd->gpmdv[ii],pd->gpodv[ii], pd->gpoSgn[ii]);
    if(pd->gpdv[ii] != pd->gpcv[ii]){
      printf("c ERR pd->gpdv[%ld] {%ld}  == pd->gpcv[%ld] {%ld}\n",ii,pd->gpdv[ii],ii,pd->gpcv[ii]);
    }
    assert(pd->gpdv[ii] == pd->gpcv[ii]);

  }
}

// printf("-----------------node %ld\n",curN);
// printPDTT(pd,pd->gd->N, curN);

//END of anylevel logic


  // printf("set curN %ld to stat 2 \n",curN);
  *curS = 2;

    if(totalCount++ % 10000 == 0){
      struct timespec time2=time_start;

    clock_gettime(CLOCK_REALTIME,&time_start);

    double tm1 = (10e9*time_start.tv_sec +time_start.tv_nsec)/10e9;
    double tm2 = (10e9*time_start.tv_sec +time_start.tv_nsec-10e9*time2.tv_sec +time2.tv_nsec)/10e9;

    // printf("cnt %ld: done call markcut in %ld dep %ld tm1 %f diff %f\n", totalCount, curN, *curDep,tm1,tm2);
    fflush(stdout);
  }
}


long solveMaxFlowAccVER6(PreprocData *pd, cType root, long minCandi, NodePropArr* pnp, cType s, cType t, int SPAN_LEN, aType LN);

#define MARK_CUT  109109
#define TARRLENG 2000000

long total_cut_edge = 0;
long total_ok = 0; //和VER6算法比对正确的

long ch1h, ch1t; //链1的头尾，最近的放尾部(深度最大的)
long ch2h, ch2t; //链2的头尾，最近的放尾部(深度最大的)
long rcht; //范围链表尾部索引，-1是没有
AnsAdjustRec *ypar ;

cType ystack1[TARRLENG];
long ystop1 = -1;
cType ystack2[TARRLENG];
long ystop2 = -1;

AnsAdjustRec* yAAR1[TARRLENG];
AnsAdjustRec* y2AAR[TARRLENG];
cType depM[TARRLENG];

//tail
AnsAdjustRec* pCh1T = NULL;
AnsAdjustRec* p2ChT = NULL;

long idx_tpair = 0;

void collectCutEdges_step2(cType curN, PreprocData *pd )
{

  long ystop1org = ystop1;
  long ystop2org = ystop2;

  nodeP* nodes = pd->gd->nodes;
  long *curCV = pd->gpcv + curN;
  long pdcv = *curCV + pd->gcutadj[curN][0];
  cType *curDep = pd->gpdep + curN;

  depM[*curDep] = curN;

// printf("call markcut in %ld  pdcv %ld\n",curN,pdcv);

  nodeP *np = nodes + curN;
  edgeP *pedges = np->edges;
  int cnt = np->nIdx;
  sType *idxs = np->orderedEdges;

  // printf("step2: curN %ld dep %ld\n",curN,*curDep);

  ////////////////////
  ////////////////////
  ////////////////////
  //更新两个链，都是一样的添加，只是两个链去除方式不一样
      // if(curN == 1391649){
      //   printf("************ before add chain, node %ld dep %ld pdcv %ld \n",curN,*curDep,pdcv);
      // }

  if(pdcv < pd->K){
    // if(curN == 1391649){
    //   printf("************ to add chain, node %ld dep %ld pdcv %ld \n",curN,*curDep,pdcv);
    // }
  //add curN as head to chain1 and chain2    
    //加到尾部
    AnsAdjustRec* aar = getOneAnsAdjust(pd);
    aar->n = curN;
    aar->prev = aar->next = NULL;
    assert(yAAR1[curN] == NULL);
    yAAR1[curN] = aar;

    //链1
    //prev指向尾部
    if(pd->gpCh1 == NULL){
      pd->gpCh1 = aar;
      pCh1T = pd->gpCh1; //指向自己
    }
    else{
      assert(pCh1T != NULL);
      if(pCh1T == pd->gpCh1){
        //之前只有一个节点
        assert(pd->gpCh1->next == NULL);
      }
      else{

        assert(pd->gpCh1->next != NULL);

      }

      pCh1T->next = aar;
      aar->prev = pCh1T;           
      pCh1T = aar;  
    }

    //链2
    aar = getOneAnsAdjust(pd);
    aar->n = curN;    
    aar->prev = aar->next = NULL;
    assert(y2AAR[curN] == NULL);
    y2AAR[curN] = aar;

    if(pd->gp2Ch == NULL){
      pd->gp2Ch = aar;
      p2ChT = pd->gp2Ch; //指向自己
    }
    else{
      assert(p2ChT != NULL);
      if(p2ChT == pd->gp2Ch){
        //之前只有一个节点
        assert(pd->gp2Ch->next == NULL);

      }
      else{

        assert(pd->gp2Ch->next != NULL);

      }

      p2ChT->next = aar;
      aar->prev = p2ChT;   
      p2ChT = aar;        

    }   
  }




  // printf("c after add chain, node %ld dep %ld ch1t %ld [n: %ld] ch1h %ld [n: %ld] , ch2t %ld [n:%ld ] ch2h %ld [n: %ld] \n",curN,*curDep,ch1t, pd->gpCh1[ch1t], ch1h,pd->gpCh1[ch1h], ch2t,pd->gpCh2[ch2t],ch2h,pd->gpCh2[ch2h]);
  ////////////////////
  ////////////////////
  ////////////////////
  //遍历当前节点的祖先链,凡是改变两个链的，分别放到对应堆栈中
  cType ldep = MAX_LONG;
  NodeDTTCover *tch = pd->gperNodeheadPndc[curN];

  if (tch != NULL && tch->dv != MAX_LONG)
  {
      

    for (; tch != NULL && tch->dv != MAX_LONG; tch = tch ->next)
    {

      //循环直到下个tch
      for (cType yDep = tch->dep; yDep > tch->next->dep; yDep --)
      {
        cType y = depM[yDep];  
        assert(yDep == 0 || pd->gpfa[y] == depM[yDep-1]);
        assert(yDep == pd->gpdep[y]);

        long pdyyv = pd->gpcv[y] + pd->gcutadj[y][0];

        if(pdyyv >= pd->K){
          continue;
        }

        if (pd->gpSadj2[y] == 0)
        {
          pd->gpSadj2[y] = 1;

          // printf("c ---before remove y %ld dep %ld from ch2 \n",y, pd->gpdep[y]);
          // remove y from ch2：只要已补回的就移除
          assert(pd->gp2Ch != NULL); // y之前访问到了，链必定不空
          assert(pd->gpdep[pd->gp2Ch->n] <= yDep);

          //链2的尾部节点深度必然在当前y之下(只要补回就去掉的)
          //但是不排除有之前节点未补回
          // assert(pd->gpdep[p2ChT->n] >= yDep);
          /*
            当前这个补回y, 可能在当前未补回的上面，就是在链中间
          */

          //快速定位目前y在链2上的节点->ypar
          ypar = y2AAR[y];
          //把左右两边连通绕过y
          // y在yArr记录指针不变，方便后面找到位置回复
          assert(p2ChT->next == NULL);
          if (ypar == pd->gp2Ch)
          {
            // y是链头
            assert(ypar->prev = NULL);
            pd->gp2Ch = ypar->next;
            if (pd->gp2Ch == NULL)
            {
              p2ChT = NULL;
            }
          }
          else
          {
            assert(ypar->prev != NULL);

            if (ypar->next != NULL)
            {
              assert(p2ChT != ypar);
              ypar->prev->next = ypar->next;
              ypar->next->prev = ypar->prev;
            }
            else
            {
              // y对应节点的的next == NULL,说明什么？
              //不能从y2AAR里删除，因为后面恢复还需要
              //DEBUG BEGIN
              if (p2ChT != ypar)
              {
                AnsAdjustRec *tmp = pd->gp2Ch;
                printf("h ypar %ld dep %ld  curN %ld dep %ld\n", ypar->n, pd->gpdep[ypar->n], curN, pd->gpdep[curN]);
                while (tmp != NULL)
                {
                  printf("here n %ld dep %ld  curN %ld dep %ld\n", tmp->n, pd->gpdep[tmp->n], curN, pd->gpdep[curN]);
                  if (tmp == ypar)
                  {
                    printf("found\n");
                    assert(tmp->n == ypar->n);
                  }
                  else
                  {
                    assert(tmp->n != ypar->n);
                  }
                  tmp = tmp->next;
                }
              }
              //DEBUG END

              assert(p2ChT == ypar);
              p2ChT = ypar->prev;
              p2ChT->next = NULL;
            }
          }

          ystack2[++ystop2] = y;

          // printf("removed y %ld dep %ld from ch2 already\n",y,pd->gpdep[y]);
          // //要求在这个节点前已经恢复到删掉时的样子
          // if(ypar->next != NULL){
          //   printf("next-prev %ld  ypar->prev %ld\n",ypar->next->prev , ypar->prev);
          // }

          // if(ypar->prev != NULL){
          //   printf("prev-next %ld  ypar->next %ld\n",ypar->prev->next , ypar->next);
          // }

          assert(ypar->next == NULL || ypar->next->prev == ypar->prev);
          assert(ypar->prev == NULL || ypar->prev->next == ypar->next);
        }

        if (pd->gpSadj1[y] == 0)
        {
          pd->gpSadj1[y] = 1;// 只是补回了，但是不一定从链1中去掉了
          if (pdyyv - tch->dv >= pd->K)
          { //链1：补回后>=K的就移除
            //从ch1中移除
            // printf("c ---before remove y %ld dep %ld from ch1 \n",y, pd->gpdep[y]);
            pd->gpSadj1[y] = 2; //2说明去掉了
            AnsAdjustRec *ypar = yAAR1[y];
            //把左右两边连通绕过y
            // y在yArr记录指针不变，方便后面找到位置回复
            assert(pCh1T->next == NULL);
            if (ypar == pd->gpCh1)
            {
              // y是链头
              assert(ypar->prev = NULL);
              pd->gpCh1 = ypar->next;
              if (pd->gpCh1 == NULL)
              {
                pCh1T = NULL;
              }
            }
            else
            {
              assert(ypar->prev != NULL);

              if (ypar->next != NULL)
              {
                assert(pCh1T != ypar);
                ypar->prev->next = ypar->next;
                ypar->next->prev = ypar->prev;
              }
              else
              {
                assert(pCh1T == ypar);
                pCh1T = ypar->prev;
                pCh1T->next = NULL;
              }
            }
            ystack1[++ystop1] = y;
            assert(ypar->next == NULL || ypar->next->prev == ypar->prev);
            assert(ypar->prev == NULL || ypar->prev->next == ypar->next);
          }
          else{
            ldep = min(ldep, yDep);
          }
        }

        // assert( (ch1h<0 && ch1t<0) || (ch1h>=0&&ch1t>=0&&pd->gpCh1[ch1h] > 0 && pd->gpCh1[ch1t] >=0));
        // assert( (ch2h<0 && ch2t<0) || (ch2h>=0&&ch2t>=0&&pd->gpCh2[ch2h] > 0 && pd->gpCh2[ch2t] >=0));

        // printf("c $$$$$ after shrink chain, node %ld dep %ld ch1t %ld [n: %ld] ch1h %ld [n: %ld] , ch2t %ld [n:%ld ] ch2h %ld [n: %ld] \n",curN,*curDep,ch1t, pd->gpCh1[ch1t], ch1h,pd->gpCh1[ch1h], ch2t,pd->gpCh2[ch2t],ch2h,pd->gpCh2[ch2h]);

        //找到ch2的head，注意此时要考虑0情况
        if (pd->gp2Ch == NULL)
        {
          //ch2空，没有需要去掉的祖先节点，不用上溯了
          break;
        }
        else
        {
          if (pd->gpdep[pd->gp2Ch->n] > tch->dep)
          { 
            //当前已经上溯到ch2的头部以上了，退出
            //ch2头部以上的<k的都加入了，如果ch2头部以上没有，就是之上所有<k的都被删除掉了，不存在还没删除的
            break;
          }
        }
      }
      
    }

    assert(tch->dv == MAX_LONG);

  }

  /*ldep需要是补回+未补回<K的才行
  
  */
  
  ////////////////////
  ////////////////////
  ////////////////////
  //将刚才补回祖先最大范围更新范围F1

  DepRange tempRange;//保留用于恢复
  tempRange.dep1 = tempRange.dep2 = 0;

  if(ldep != MAX_LONG){
    //对直接连祖先的边，可以更新范围
    //merge [fa_x, ldep ] to range chain
        rcht++;
        assert(rcht>=0);
        pd->gpRch[rcht].dep1 = ldep; //dep1是较小的靠上的
        pd->gpRch[rcht].dep2 = *curDep-1;

  }





  ////////////////////
  ////////////////////
  ////////////////////  
  //得到 ch1 尾部


  cType ty = 0;
  if(pCh1T != NULL){
    ty = pCh1T->n;
  }



  ////////////////////
  ////////////////////
  ////////////////////  
  //对每个邻居y开始判断(x,y)
  if(np->totalCap < pd->K){
      //情况1
      for (int ni = 0; ni < cnt; ni++)
      {
          edgeP *eh = pedges + idxs[ni];
          if(eh->w != MARK_CUT){
            eh->rev->w = eh->w = MARK_CUT;
            total_cut_edge ++;    
          }      
      }      
  }

  for (int ni = 0; ni < cnt; ni++)
  {
    // nodeP* znp = nodes+eh->endNode;

    edgeP *eh = pedges + idxs[ni];
    cType zn = eh->endNode;

    if(pd->gpdep[zn] < *curDep){
      if(eh->w == MARK_CUT){continue;}//祖先及父的边没必要检查了,而且对应祖先边的逻辑只是判断没有修改
    }
    
    aType isFound = 0;

    if(pd->gpdep[zn] > *curDep){
      if(pd->gpfa[zn] == curN){
        collectCutEdges_step2(zn,pd); //到子孙的边他们去判断，这里不能判断
      }
    }
    else if(pd->gpdep[zn] < *curDep && isFound == 0){
      if(pd->gpfa[curN] == zn){
        //情况2和3a
        /*
          情况2就是 curN的DTT
          情况3a就是 curN的PDTT
          这两个的割边都保护curN连父亲zn的，所以直接设置
        
        */
        if(pdcv < pd->K){
          isFound = 1;
        }
      }
      else {
        //情况3b：未补回+补回的树内x，连树外祖先y不包含树根
        if(ty > 0 && pd->gpdep[ty]  > pd->gpdep[zn]){
          //ch1的尾存在，且在当前zn之下(不包含),说明(x,zn)跨<k的树了
          isFound = 1; //这个判断还行
          // if(curN == 39 && zn ==56){
          //   printf("here3b 39 to zn %ld with ty %ld pdcv %ld with dep %ld > zn %ld dep %ld\n",zn,ty,pd->gpdv[ty]+pd->gcutadj[ty][0],pd->gpdep[ty],zn,pd->gpdep[zn]);
          // }
        }
        else{
          //情况3c：x在被裁减掉的部分，连树内
          long tmpt = rcht;
          while(tmpt >= 0){
            //相等可以
            if(pd->gpdep[zn] <= pd->gpRch[tmpt].dep2 && pd->gpdep[zn] >= pd->gpRch[tmpt].dep1){
              //在内部
              isFound = 1;
                // if(curN == 39 && zn ==56){
                //   printf("here3c 39 to zn %ld with dep %ld within [%ld %ld]\n",zn,pd->gpdep[zn],pd->gpRch[tmpt].dep2, pd->gpRch[tmpt].dep2);
                // }
              break;
            }

            tmpt --;  
          }
        
        }
      }

      if (isFound == 1)
      {
        eh->rev->w = eh->w = MARK_CUT;
        total_cut_edge++;
      }



    }



  }

  // for (int ni = 0; ni < cnt; ni++)
  // {
  //   // nodeP* znp = nodes+eh->endNode;

  //   edgeP *eh = pedges + idxs[ni];
  //   cType zn = eh->endNode;

  //   if(pd->gpdep[zn] < *curDep && idx_tpair < 1001){
  //     printf("tpairs[%ld][0]=%ld; tpairs[%ld][1]=%ld; tpairs[%ld][2]=%d; tpairs[%ld][3]=%d;\n", idx_tpair, zn, idx_tpair,curN, idx_tpair,(int)(eh->w==MARK_CUT),idx_tpair,pd->K);
  //     idx_tpair++;
  //   }
  // }


  // for (int ni = 0; ni < cnt; ni++)
  // {
  //   // nodeP* znp = nodes+eh->endNode;

  //   edgeP *eh = pedges + idxs[ni];
  //   cType zn = eh->endNode;
  //   if(pd->gpdep[zn] < *curDep){
  //     //判断边是否正确

  //   long mv = min((pd->gd->nodes+curN)->totalCap, (pd->gd->nodes+zn)->totalCap);
      
  //   // memset(apply_adj, 0, len *  sizeof(long));
  //   mv = solveMaxFlowAccVER6(pd, 0,mv, pd->allResults+0, curN, zn,pd->SPAN_LEN,pd->LEVEL_NUM);

  //   if((mv < pd->K && eh->w == MARK_CUT) || (mv >= pd->K && eh->w != MARK_CUT)){
  //     total_ok ++;
  //   }
  //   printf("c after validate edge (%ld %ld) %ld/%ld\n", curN,zn,total_ok,total_cut_edge);
  //   }
  // }



/////////////////////
/////////////////////
/////////////////////
///恢复
/*
 结束时怎么恢复？
*/
 
  
  /*(1) ch1和ch2删除的，和设置Sadj的y咋弄？
      相当于补回的要重新处理
      这种y要保存到全局数据结构堆栈中，记录堆栈初始点
      把y压入堆栈
      结束时，一直pop到初始点的每个节点(不包含初始点)
        状态归0
        对应ch2移除加回去，如果ch1也移除了也加回去
  */   

  //ch2和sadj的共享,只要<k sadj=1的，肯定从ch2去掉
      // if(curN == 1391639){
      //   printf("ystop %ld stoporg %ld  when curN %ld dep %ld\n", ystop2,ystop2org,curN, pd->gpdep[curN]);
      // }
  while(ystop2 > ystop2org){
    cType y = ystack2[ystop2];
    // if(curN == 1391639){
    //   printf("************** torecover y = %ld to Ch2 when curN %ld dep %ld\n", y,curN, pd->gpdep[curN]);
    // }

    // if(y == 1391650){
    //   printf("*********ss*****y = %ld recoverd to Ch2 when curN %ld dep %ld\n", y, curN, pd->gpdep[curN]);
    // }

    pd->gpSadj2[y] = 0;
    AnsAdjustRec* ypar = y2AAR[y];
    // printf("add y %ld dep %ld back to ch2\n",y,pd->gpdep[y]);
    // //要求在这个节点前已经恢复到删掉时的样子
    // if(ypar->next != NULL){
    //   printf("next-prev %ld  ypar->prev %ld\n",ypar->next->prev , ypar->prev);
    // }

    // if(ypar->prev != NULL){
    //   printf("prev-next %ld  ypar->next %ld\n",ypar->prev->next , ypar->next);
    // }



    assert(ypar->next == NULL || ypar->next->prev == ypar->prev);
    assert(ypar->prev == NULL || ypar->prev->next == ypar->next);

    if(ypar->next != NULL){ ypar->next->prev = ypar; }
    if(ypar->prev != NULL){ ypar->prev->next = ypar; }

   
    if(p2ChT == ypar->prev){
      p2ChT ->next = ypar;
      p2ChT = ypar;
      assert(ypar->next == NULL);
    }

    if(pd->gp2Ch == ypar->next){
      pd->gp2Ch->prev = ypar;
      pd->gp2Ch = ypar;
      assert(ypar->prev == NULL);
    }  

    ystop2--;
  }

  //ch1中去掉的只在这里
  while(ystop1 > ystop1org){
    cType y = ystack1[ystop1];
    assert(pd->gpSadj1[y] == 2);
    pd->gpSadj1[y] = 0;
    AnsAdjustRec* ypar = yAAR1[y];
    //要求在这个节点前已经恢复到删掉时的样子
    assert(ypar->next == NULL || ypar->next->prev == ypar->prev);
    assert(ypar->prev == NULL || ypar->prev->next == ypar->next);
    if(ypar->next != NULL){ ypar->next->prev = ypar; }
    if(ypar->prev != NULL){ ypar->prev->next = ypar; }

    if(pCh1T == ypar->prev){
      pCh1T = ypar;
      assert(ypar->next == NULL);
    } 

    if(pd->gpCh1 == ypar->next){
      pd->gpCh1 = ypar;
      assert(ypar->prev == NULL);
    }  

    ystop1--;
  }

 //(2)x如果在ch1和ch2的尾部，则删除 注意删除尾部必须这里，因为先加尾部再减去祖先y，恢复要反过来
  if(pdcv < pd->K){
    assert(pCh1T !=NULL && pCh1T->n == curN);
    assert(p2ChT !=NULL && p2ChT->n == curN);
  }

  if(pCh1T != NULL && pCh1T->n == curN){
    if(pd->gpCh1 == pCh1T){
      assert(pd->gpCh1->prev == NULL);
      pd->gpCh1 = pCh1T = NULL; //彻底删除不留痕迹
    }  
    else{
      //>=2个节点
      assert(pCh1T->next == NULL);
      pCh1T = pCh1T->prev;
      pCh1T->next = NULL;

    }
    
    yAAR1[curN] = NULL;
  }
  
  if(p2ChT != NULL && p2ChT->n == curN){
    if(pd->gp2Ch == p2ChT){
      assert(pd->gp2Ch->prev == NULL);
      pd->gp2Ch = p2ChT = NULL; //彻底删除不留痕迹
    }  
    else{
      //>=2个节点
      assert(p2ChT->next == NULL);
      p2ChT = p2ChT->prev;
      p2ChT->next = NULL;

    }
    
    y2AAR[curN] = NULL;
  }
  
    // printf("c ***** before shrink chain, node %ld dep %ld ch1t %ld [n: %ld] ch1h %ld [n: %ld] , ch2t %ld [n:%ld ] ch2h %ld [n: %ld] \n",curN,*curDep,ch1t, pd->gpCh1[ch1t], ch1h,pd->gpCh1[ch1h], ch2t,pd->gpCh2[ch2t],ch2h,pd->gpCh2[ch2h]);

  // printf("c ***** after shrink chain, node %ld dep %ld ch1t %ld [n: %ld] ch1h %ld [n: %ld] , ch2t %ld [n:%ld ] ch2h %ld [n: %ld] \n",curN,*curDep,ch1t, pd->gpCh1[ch1t], ch1h,pd->gpCh1[ch1h], ch2t,pd->gpCh2[ch2t],ch2h,pd->gpCh2[ch2h]);


  // assert( (ch1h<0 && ch1t<0) || (ch1h>=0&&ch1t>=0&&pd->gpCh1[ch1h] > 0 && pd->gpCh1[ch1t] >0));
  // assert( (ch2h<0 && ch2t<0) || (ch2h>=0&&ch2t>=0&&pd->gpCh2[ch2h] > 0 && pd->gpCh2[ch2t] >0));

/*   merge咋处理，
      merge时只合并尾部，要么添加一个回退，要么取消合并
      
*/

  if (ldep != MAX_LONG)
  { //这时才需要回退
    assert(rcht >= 0);
    //添加的去掉
    rcht--;
  }

// printf("step2-done : curN %ld dep %ld\n",curN,*curDep);

}






///////////////////////////////输出每个节点的一些属性
void print_markCutProp(PreprocData *pd, cType curN){
     printf("%ld(fa:%ld) dp %ld & dv %ld & mdv %ld & cv %ld & ph %ld & 0:%ld", curN, pd->gpfa[curN],pd->gpdep[curN],pd->gpdv[curN],pd->gpmdv[curN],pd->gpcv[curN], pd->gpoh[curN], pd->gcutadj[curN][0]);

    cType fa = curN;

    for (int i = 1; i <= pd->LEVEL_NUM; i++)
    {
      if (pd->gpdep[fa] == 0)
      {
        break;
      }

      fa = pd->gpfa[fa];
      printf(" %d:%ld", i, pd->gcutadj[curN][i]);
    }

    fa = curN;

    printf(" &");
    //printf(" & 0/%d", pd->gcutadjsign[curN][0]);

    for (int i = 1; i <= pd->LEVEL_NUM; i++)
    {
      if (pd->gpdep[fa] == 0)
      {
        break;
      }
      fa = pd->gpfa[fa];
      printf(" %d:%d", i, pd->gcutadjsign[curN][i]);
    } 
}
void print_markCut(PreprocData *pd)
{
  // nodeP *nodes = pd->gd->nodes;
  for (cType curN = 1; curN <= min(10,pd->gd->N); curN++)
  {
    // nodeP *np = nodes + curN;
    // edgeP *pedges = np->edges;
    // int cnt = np->nIdx;
    print_markCutProp(pd,curN);

    printf("\\\\\n");
  }
}

////////////////////////////////////////////////print the PDTT with root and child within lvl
void printPDTT(PreprocData *pd, cType root, cType curN)
{
  nodeP *np = pd->gd->nodes + curN;
  int cnt = np->nIdx;
  sType *idxs = np->orderedEdges;
  edgeP *pedges = np->edges;
  cType *pdep = pd->gpdep;
  
  int lvl = pdep[curN] - pdep[root];

  for(int i=0; i<lvl;i++){
    printf("  ");
  }  
  print_markCutProp(pd,curN);
  printf("\n");

  if(lvl == pd->LEVEL_NUM){
    return;
  }
  for (int ni = 0; ni < cnt; ni++)
  {
    edgeP *eh = pedges + idxs[ni];
    cType zn = eh->endNode;
    // short zs = pd->gps[zn];

    if (pdep[zn] == (pdep[curN]+1))
    {
      printPDTT(pd, root,zn);
    }
  }
}

void printPNDC(PreprocData *pd, cType curN){
  printf("------PNDC node %ld dep %ld\n",curN,pd->gpdep[curN]);
  NodeDTTCover *pndc = pd->gperNodeheadPndc[curN];
  while(pndc != NULL){
    printf(" (dep %ld dv %ld) -> ", pndc->dep,pndc->dv);
    pndc = pndc->next;
  }
  printf(" end \n");
}

////////////////////////////////The function to obtain min-cut value of given node pair, i.e., Algorithm 3 in the paper
/*
 (2)求解时：
      深度大的出发节点直接cv2,
      另外一个不是这个的祖先时,另外一个也可以用cv2
      [这个先不优化有点复杂]如果t是s祖先,
        如果cv2正好割掉下面,直接可以用cv2
        如果不是,也可以计算割掉对应树后这个t的割

      PS: t是s祖先,可以用cv2,t不是s的祖先也可以cv2,两个cv2都可以用
      问题是,如果t是s祖先,t的cv2对应的割并不能切断s这条支线,这样就不对了
        如果t是s祖先,其实我们算的是去掉这个支后的t的最小割
      那就简化：
        只要t不是s祖先,就可以用t的cv2

      后面的逐个逼近,需要用cv'
      
*/
//对给定节点,更新祖先值,并记录到按深度差为key的数组中,数组也就LN长度
void traceUp2LN(NodePropArr* pnp, cType startDep, cType curN, cType curDep, long *adj, cType LN)
{

  for (int i = 1; i <= LN; i++)
  {
    if ( curDep < (unsigned int)i || (curDep+LN) <= (startDep+i) )
    {
      break; //祖先已经不存在了,或已经到startDep的LN祖先了,没必要计算了
    }

    if (pnp->pacc_cut_adjust_sign[curN][i] == 1)
    {
      adj[startDep+i-(curDep)] = pnp->pacc_cut_adjust[curN][i]; //这个是curN整个树去掉相对i祖先的cv的影响负值
    }
  }
}

//函数只要检查,LN_v的最优值,是否包含v; 如果不包含,区分是v_top(不含)以下点最优不包含,还是v_top(含)以上
//
//v到LN_v的情况,每个点
long isOptimalExclude(NodePropArr* pnp, cType v, cType v_top, cType LN_v, cType *pDep, cType *pFa, cType LN){
  
  cType minDep = 0;
  // printf("isOptimalExclude: v %ld dep %ld, LN_v %ld dep %ld\n",v,pDep[v],LN_v,pDep[LN_v]);
  while(v != LN_v){
    //如果有节点对应LN_v标志是1,说明肯定排除在最优之外,v的祖先有没有,v都被排除了。
    //如果v没有但祖先有,也可以通过再Fa回溯检测到
    if(pDep[v] - pDep[LN_v] <= LN && pnp->pacc_cut_adjust_sign[v][pDep[v] - pDep[LN_v]]==1){
      //如果不包含,还要体现到哪不包含
      minDep = pDep[v];    
    }
    v = pFa[v];
  }

  // printf("isOptimalExclude DONE \n");

  //除了v到v_top(不含)被包含或不包含的情况,还有一种是根本不知道,因为不在这个深度裁剪,这个时候什么情况？
  //这种咋处理?
  //从共同LN_v开始每个祖先,有几种可能：
  //(1)LN_v的最优包含一方,不包含另外一方
  //具体情况: 
  //(1) LN_s(不含)之下的祖先,单独看就行,其实就只有LN_s,其他也没意义
  /*(2) LN_s(含)及之上的祖先,则需要看不含一方的最高点,如果不含的最高点在LN_s(不含)以下,可以认为不含,因为去掉t不影响s。
        如果在之上就麻烦了,去掉t也去掉s了,这个就不好办了

  */
  //默认LN_v最优不需要去掉v
  //如果包含,那就是都包含,这个不用继续看了
  if(minDep > 0){
    if(minDep <= pDep[v_top]){
      return 2; //在v_top及之上断开了
    }
    else{
      return 1;
    }
  }

  return 0;
}

//其实还可以考察共同点v上面的祖先点W,只要v到w都没被丢弃,则s和t单独被丢弃的情况,都可以作为最优解来处理
//因为w不确定,其实是找最低的w, w的最小值
long solveMaxFlowAccVER4(PreprocData *pd, cType root, long minCandi, NodePropArr* pnp, cType s, cType t, int SPAN_LEN, aType LN,long* adjs, long*adjt)
{
  cType *pDep = pnp->pdep;
  long *pCV = pnp->pcv;
  cType *pFa = pnp->pfa;
  // long *poh = pnp->poh;
  // cType *paccup = pnp->pacc_upid;
  // cType *paccup_LC = pnp->pacc_pos_upmincv;
  // long *paccmcv = pnp->pacc_upmincv;

  // cType os = s, ot = t;
  assert(s != t);
  if (pDep[s] < pDep[t])
  {
    cType tmp = s;
    s = t;
    t = tmp;
  }

  // cType curNDCDep2 = MAX_LONG;

  printf("\nstart: s %ld dep %ld, t %ld dep %ld\n",s,pDep[s],t,pDep[t]);

  assert(pDep[s] >= pDep[t]);


  // long *adj = (long *)walloc(pd->gd->N+1, sizeof(long));

  // memset(adj, 0, (pd->gd->N+1) *  sizeof(long));
  memset(adjs, 0, (pd->gd->N+1) *  sizeof(long));
  memset(adjt, 0, (pd->gd->N+1) *  sizeof(long));

  long mcv = MAX_LONG;

  long startDeps = pDep[s];
  long startDept = pDep[t];
	
	//long temp;
  while(pDep[s] > pDep[t]){

    //先更新对祖先的补回值
    for (int i = 1; i <= min(LN,startDeps); i++)
    {
      //不能校验&& adjs[ startDeps - (pDep[s]-i) ] == 0，因为对每个祖先，s都是由远及近，必须得到了最近的设置才行，所以需要覆盖
      if (pnp->pacc_cut_adjust_sign[s][i] == 1)
      {
        // printf("c s %ld, depth -%ld, adjust: %ld\n",s, i, pnp->pacc_cut_adjust[s][i]);
        adjs[ pDep[s]-i ] = pnp->pacc_cut_adjust[s][i]; //这个是curN整个树去掉相对i祖先的cv的影响负值
      }
    }

    //调用祖先优化

	  long temp = pCV[s] + pnp->pacc_cut_adjust[s][0] - adjs[pDep[s]]; //TODEL
    // if(temp < 100 && mcv > temp){
    //   printf("mcv is %ld and temp %ld for node %ld,pcv %ld, cadj %ld, adj %ld\n",mcv,temp,s,pCV[s] , pnp->pacc_cut_adjust[s][0] , adjs[pDep[s]]);
    //   print_markCutProp(pd,s);
    //   printf("\n");
    // }
	
    mcv = min(mcv,temp ); 
/*	
	if(mcv < temp){//TODEL 包括内部	
		printf("update using node s %ld , before mcv %ld\n", s,temp); 	
		printf("after mcv %ld\n", mcv); 
	}
*/
//debug
  // printf("--c node %ld dep1 %ld dep2 %ld\n",s,pd->gpndc[s].dep1,pd->gpndc[s].dep2);
  // if(pd->gpndc[s].dep2 > 0){
  //   if(pd->gpndc[s].dep2 < curNDCDep2){

  //     printf("----------c in node %ld, curNDCDep %ld ->  %ld\n",s,curNDCDep2,pd->gpndc[s].dep2);

  //     curNDCDep2 = pd->gpndc[s].dep2;
  //   }
  //   else if(pd->gpndc[s].dep2 > curNDCDep2){
  //     printf("----------c error in node %ld, curNDCDep %ld <  %ld\n",s,curNDCDep2,pd->gpndc[s].dep2);
  //     assert(1==0);
  //   }
  // }

//end

    s = pFa[s];

  }

  assert(pDep[s] == pDep[t]);

  if(s == t){
    goto end;
  }

  //此时s和t不相等,一起上溯
  while(s != t){
    for (int i = 1; i <= LN; i++)
    {
      if (pnp->pacc_cut_adjust_sign[s][i] == 1)
      {
        adjs[ startDeps - (pDep[s]-i) ] = pnp->pacc_cut_adjust[s][i]; //这个是curN整个树去掉相对i祖先的cv的影响负值
      }

      if (pnp->pacc_cut_adjust_sign[t][i] == 1)
      {
        adjs[ startDept - (pDep[t]-i) ] = pnp->pacc_cut_adjust[t][i]; //这个是curN整个树去掉相对i祖先的cv的影响负值
      }


    }

    //调用祖先优化
    mcv = min(mcv, pCV[s] + pnp->pacc_cut_adjust[s][0] - adjs[startDeps - pDep[s]]); 
    mcv = min(mcv, pCV[t] + pnp->pacc_cut_adjust[t][0] - adjs[startDept - pDep[t]]); 

    s = pFa[s];
    t = pFa[t];
  }

end:
  // free(adjs);
  // free(adjt);
  return mcv;


}


// long solveMaxFlowAccVER5(PreprocData *pd, cType root, long minCandi, NodePropArr* pnp, cType s, cType t, int SPAN_LEN, aType LN,long *adjs, long* adjt)
// {
//   cType *pDep = pnp->pdep;
//   long *pCV = pnp->pcv;
//   cType *pFa = pnp->pfa;
//   // long *poh = pnp->poh;
//   // cType *paccup = pnp->pacc_upid;
//   // cType *paccup_LC = pnp->pacc_pos_upmincv;
//   // long *paccmcv = pnp->pacc_upmincv;

//   // cType os = s, ot = t;
//   assert(s != t);
//   if (pDep[s] < pDep[t])
//   {
//     cType tmp = s;
//     s = t;
//     t = tmp;
//   }

//   long tcount = 0;
//   long count = 0;

//   cType curNDCDep2 = MAX_LONG;

//   printf("\nstart: s %ld dep %ld, t %ld dep %ld\n",s,pDep[s],t,pDep[t]);

//   assert(pDep[s] >= pDep[t]);

//   //简化版，仅仅记录当前节点，在s之上有几个sign=1，至少不是0，就都加上

//   // NodeDTTCover **stack = (NodeDTTCover **)walloc(pd->gd->N+1, sizeof(NodeDTTCover*));
//   // memset(stack, 0, (pd->gd->N+1) *  sizeof(NodeDTTCover*));
//   // cType stop = 0;

  
//   // NodeDTTCover **adjs = (NodeDTTCover **)walloc(pd->gd->N+1, sizeof(NodeDTTCover*));
//   // memset(adjs, 0, (pd->gd->N+1) *  sizeof(NodeDTTCover*));  

//   // NodeDTTCover **adjt = (NodeDTTCover **)walloc(pd->gd->N+1, sizeof(NodeDTTCover*));
//   // memset(adjt, NULL, (pd->gd->N+1) *  sizeof(NodeDTTCover*));  

//   memset(adjs, 0, (pd->gd->N+1) *  sizeof(long));  
//   memset(adjt, 0, (pd->gd->N+1) *  sizeof(long));  

//   long branch_s = 0;
//   long branch_t = 0;

//   long mcv = MAX_LONG;

//   long startDeps = pDep[s];
//   long startDept = pDep[t];
	
//   NodeDTTCover* curNDC = NULL;
//   NodeDTTCover * pndc = pd->gpndc;
// 	//long temp;
//   while(pDep[s] > pDep[t]){

//     //先把放到对应祖先处，不到最下面祖先这个sign=1不成立  
//     //作为下面节点，先更新对最下面祖先的这个记录
//     if(pndc[s].dep2 > 0){
//       // assert(adjs[pndc[s].dep1] == 0); //不一定，有可能两个子孙对同一个开始节点是sign=1，等于这个祖先有两个子孙是sign=1
//       adjs[pndc[s].dep1]++; //这个值只是说明到这里了，同时并行的sign=1子孙变化量是多少
//       adjs[pndc[s].dep2]--;
//     }

//     assert(branch_s >= 0);
//     //作为祖先，使得对应sign=1生效
//     branch_s += adjs[pDep[s]];
//     if(branch_s > 0){
//       //说明在s上有sign=1的整颗树去掉的情况
//       //下面子孙的整个mdt都要去掉，关键现在mdt也不是了呀，只有整体mdt再cadj[0]中
//       //这个s不计算在内
     
//     }
//     else{
//       //当前s在这个分支上没有需要去掉的DTT
//       count++;
//       mcv = min(mcv, pCV[s] + pnp->pacc_cut_adjust[s][0]);
//     }

//     //调用祖先优化

// 	  // long temp = pCV[s] + pnp->pacc_cut_adjust[s][0] - adjs[pDep[s]]; //TODEL
//     // if(temp < 100 && mcv > temp){
//     //   printf("mcv is %ld and temp %ld for node %ld,pcv %ld, cadj %ld, adj %ld\n",mcv,temp,s,pCV[s] , pnp->pacc_cut_adjust[s][0] , adjs[pDep[s]]);
//     //   print_markCutProp(pd,s);
//     //   printf("\n");
//     // }
	
//     // mcv = min(mcv,temp ); 
// /*	
// 	if(mcv < temp){//TODEL 包括内部	
// 		printf("update using node s %ld , before mcv %ld\n", s,temp); 	
// 		printf("after mcv %ld\n", mcv); 
// 	}
// */
// //debug
//   // printf("--c node %ld dep1 %ld dep2 %ld\n",s,pd->gpndc[s].dep1,pd->gpndc[s].dep2);
//   // if(pd->gpndc[s].dep2 > 0){
//   //   if(pd->gpndc[s].dep2 < curNDCDep2){

//   //     printf("----------c in node %ld, curNDCDep %ld ->  %ld\n",s,curNDCDep2,pd->gpndc[s].dep2);

//   //     curNDCDep2 = pd->gpndc[s].dep2;
//   //   }
//   //   else if(pd->gpndc[s].dep2 > curNDCDep2){
//   //     printf("----------c error in node %ld, curNDCDep %ld <  %ld\n",s,curNDCDep2,pd->gpndc[s].dep2);
//   //     assert(1==0);
//   //   }
//   // }

// //end

//     s = pFa[s];
//     tcount ++;

//   }

//   assert(pDep[s] == pDep[t]);

//   if(s == t){
//     goto end;
//   }

//   //此时s和t不相等,一起上溯
//   while(s != t){
//     // for (int i = 1; i <= LN; i++)
//     // {
//     //   if (pnp->pacc_cut_adjust_sign[s][i] == 1)
//     //   {
//     //     adjs[ startDeps - (pDep[s]-i) ] = pnp->pacc_cut_adjust[s][i]; //这个是curN整个树去掉相对i祖先的cv的影响负值
//     //   }

//     //   if (pnp->pacc_cut_adjust_sign[t][i] == 1)
//     //   {
//     //     adjs[ startDept - (pDep[t]-i) ] = pnp->pacc_cut_adjust[t][i]; //这个是curN整个树去掉相对i祖先的cv的影响负值
//     //   }


//     // }
//   assert(branch_s >= 0);
//   assert(branch_t >= 0);
    


//     if(pndc[s].dep2 > 0){
//       // assert(adjs[pndc[s].dep1] == NULL);
//       adjs[pndc[s].dep1]++;
//       adjs[pndc[s].dep2]--;
//     }

//     if(pndc[t].dep2 > 0){
//       // assert(adjt[pndc[t].dep1] == NULL);
//       adjt[pndc[t].dep1]++;
//       adjt[pndc[t].dep2]--;
//     }

//     branch_s += adjs[pDep[s]];
//     branch_t += adjt[pDep[t]];
//     //调用祖先优化
//     if(branch_s == 0){
//       mcv = min(mcv, pCV[s] + pnp->pacc_cut_adjust[s][0] ); 
//       count++;
//     }

//     if(branch_t == 0){
//       mcv = min(mcv, pCV[t] + pnp->pacc_cut_adjust[t][0] ); 
//       count++;
//     }

//     if(branch_s != 0 && branch_t != 0){
//       break;
//     }

//     s = pFa[s];
//     t = pFa[t];
//     tcount+=2;
//   }

// end:

//   printf("c checked %ld nodes, with %ld used\n",tcount, count);
//   // free(adjs);
//   // free(adjt);
//   return mcv;


// }

/////////////////////////////////////////////
/*
  不能只取栈顶，栈顶没有得往下找
  相当于你还要把开始点那里放上信息，知道这个时候来了什么点
  栈只是目前叠加的所有，不能只看栈顶，得往下看，直到包含所在的点
  对于一个系列：目前assert说明成立：
  如果a1 a2 分别是祖先 b1 b2的纯DTT，他们都在 a1 -> root路径上
  则如果a1.dep > a2.dep,  则b1.dep > b2.dep
  
  用sign=0链的原因是
  s一旦在y的PDTT中补回就是对应的中间节点n的DTT全补回，实际上如果知道s对这个祖先y的mdv，可以补回后再去掉mdv，进一步降低损耗

*/
long curDep = -1;
//nlink1 是初始的s或t他的sign=1的链， nlink0 是sign=0的链, 两个星是这边还要向外更新
long getAdj(NodePropArr* pnp, NodeDTTCover **stack, long* stop, NodeDTTCover **mark, cType curN, NodeDTTCover** pnlink0, NodeDTTCover** pnlink0a, NodeDTTCover** pnlink0b, int logicKind, int noChild){
  
  NodeDTTCover* nlink0 = *pnlink0;

  cType dep = pnp->pdep[curN];

  if(*stop >= 0){
    assert(stack[*stop] != NULL);
  }

  if(!noChild){
    //作为孩子，添加到对应祖先处
    if(pnp->perNodeheadPndc[curN] != NULL){
      cType aDep = pnp->perNodeheadPndc[curN]->dep;
      // assert(mark[aDep] == NULL);
      mark[aDep] = pnp->perNodeheadPndc[curN];
    }
  }

  //作为祖先  
  //如有mark,把mark放到栈上
  if(mark[dep] != NULL){
    stack[++(*stop)] = mark[dep];
    assert(*stop == 0 || stack[*stop]->dep <= stack[*stop-1]->dep); // 不仅*stop的子dep更小，其对应的PDTT祖先的深度也更小
  }
  
  //获取调整值
  long adj = 0;
  NodeDTTCover *head = NULL;
  while(*stop >= 0){
    head = stack[*stop];
    assert(head != NULL && dep <= head->dep); //必然有
    // if(head == NULL || dep > head->dep){
    //   break;
    // }

    assert(dep <= head->dep);
    while(head->next != NULL){
      if(dep > head->next->dep){
        //正好是head所属的链节
        adj = head->dv;
        break;  //找到了
      }

      head = head->next;
    }

    if(head->dv == MAX_LONG){
      //如果找到的是尾节，需要弹出栈，继续找
      *stop = *stop - 1;
      continue;
    }

    break;
  }

  if(*stop >=0){
    stack[*stop] = head;
  }
  else{
    //do nothing
  }

  assert(adj<=0);

  //下面的逻辑，是遍历链
  //如果s补回了，用mdv来再割一次
  //如果不是s而是上面补回了，也可以用s的mdv再割一次
  /*
  如果adj是负值，意味着要填回，此时可以用下面的nlink0,重新删除一次
  如果adj不是负值，s都没有删掉，只删除s之下的即可

  */
  if(nlink0 != NULL){

      assert(dep <= nlink0->dep);
      while (1 == 1)
      {
        if (nlink0->dv == MAX_LONG)
        {
          nlink0 = NULL;
          break;
        }
        else
        {
          if (nlink0->next->dep < dep)
          {
            //就是当前nlink0了
            if(logicKind == 1){
              if(adj < 0){
                assert(adj < nlink0->dv);
                adj = min (adj, nlink0->dv);
              }
              else{
                //s分支没有填回，祖先PDTT里已经考虑了s下面怎么prune的情况
              }
            }
            else{
              //所以干脆全部注释掉
              // if(adj < 0){
              //   //adj变幻为需要补回的最多值的负值，看外面调用这的说明
              //   //啥也不做，就是返回纯的，不考虑周围mdv
              //   //不考虑mdv因为是要补回，所以尽量补回少一点
              //   //adj = min(adj, nlink0->dv);
              // }    
              // else{
              //   //do nothing
              //   //从而提示外部，没有sign=1
              // }
            }
            
            break;
          }
          else
          {
            nlink0 = nlink0->next;
          }
        }
      }

    *pnlink0 = nlink0;
  }

  if(pnlink0a != NULL && *pnlink0a != NULL){
    //不更新adj,只是向前推进，这个是合并点之前一个点a或b的链
    if(dep > (*pnlink0a)->dep){
      printf("------ dep %ld > %ld\n",dep , (*pnlink0a)->dep);
    }
    assert(dep <= (*pnlink0a)->dep);
      while (1 == 1)
      {
        if ((*pnlink0a)->dv == MAX_LONG)
        {
          *pnlink0a = NULL;
          break;
        }
        else
        {
          if ((*pnlink0a)->next->dep < dep)
          {
            //就是当前nlink0了
            assert(adj < (*pnlink0a)->dv);
            break;
          }
          else
          {
            *pnlink0a = (*pnlink0a)->next;
          }
        }
      }

  }

  if(pnlink0b != NULL  && *pnlink0b != NULL){
    //不更新adj,只是向前推进，这个是合并点之前一个点a或b的链
    assert(dep <= (*pnlink0b)->dep);
      while (1 == 1)
      {
        if ((*pnlink0b)->dv == MAX_LONG)
        {
          *pnlink0b = NULL;
          break;
        }
        else
        {
          if ((*pnlink0b)->next->dep < dep)
          {
            //就是当前nlink0了
            assert(adj < (*pnlink0b)->dv);
            break;
          }
          else
          {
            *pnlink0b = (*pnlink0b)->next;
          }
        }
      }

  }

  return adj;

}
// #define DEBUG_VER6 1
/*
st怎么求解，每个节点更新祖先的adj值，到了之后取最小就行
过了交叉点o, 找一边被裁掉的情况，取最小，直到两边都找不到



*/
long solveMaxFlowAccVER6(PreprocData *pd, cType root, long minCandi, NodePropArr* pnp, cType s, cType t, int SPAN_LEN, aType LN)
{
  curDep = -1;
  cType *pDep = pnp->pdep;
  long *pCV = pnp->pcv;
  cType *pFa = pnp->pfa;
  NodeDTTCover ** perNodeheadPndcSign0 = pnp->perNodeheadPndcSign0;
  // long *poh = pnp->poh;
  // cType *paccup = pnp->pacc_upid;
  // cType *paccup_LC = pnp->pacc_pos_upmincv;
  // long *paccmcv = pnp->pacc_upmincv;

  // cType os = s, ot = t;

  assert(s != t);
  if (pDep[s] < pDep[t])
  {
    cType tmp = s;
    s = t;
    t = tmp;
  }

  long tcount = 0;
  long count = 0;

  
  // cType curNDCDep2 = MAX_LONG;

  printf("\nstart: s %ld dep %ld, t %ld dep %ld\n",s,pDep[s],t,pDep[t]);

  assert(pDep[s] >= pDep[t]);


  //用来将纯DTT范围标记到对应祖先处
  NodeDTTCover **tempMark = (NodeDTTCover **)walloc(pd->gd->N+1, sizeof(NodeDTTCover*));
  memset(tempMark, 0, (pd->gd->N+1) *  sizeof(NodeDTTCover*));

  NodeDTTCover **tempMark2 = (NodeDTTCover **)walloc(pd->gd->N+1, sizeof(NodeDTTCover*));
  memset(tempMark2, 0, (pd->gd->N+1) *  sizeof(NodeDTTCover*));

  NodeDTTCover **tempMark3 = (NodeDTTCover **)walloc(pd->gd->N+1, sizeof(NodeDTTCover*));
  memset(tempMark3, 0, (pd->gd->N+1) *  sizeof(NodeDTTCover*));

  //是当前节点堆积的范围的堆栈，最上面需要考虑
  NodeDTTCover **stack = (NodeDTTCover **)walloc(pd->gd->N+1, sizeof(NodeDTTCover*));
  memset(stack, 0, (pd->gd->N+1) *  sizeof(NodeDTTCover*));
  long stop = -1;

  NodeDTTCover **stack2 = (NodeDTTCover **)walloc(pd->gd->N+1, sizeof(NodeDTTCover*));
  memset(stack2, 0, (pd->gd->N+1) *  sizeof(NodeDTTCover*));
  long stop2 = -1;

  NodeDTTCover **stack3 = (NodeDTTCover **)walloc(pd->gd->N+1, sizeof(NodeDTTCover*));
  memset(stack3, 0, (pd->gd->N+1) *  sizeof(NodeDTTCover*));
  long stop3 = -1;

  
  // long branch_s = 0;
  // long branch_t = 0;

  long mcv = MAX_LONG;

  // long startDeps = pDep[s];
  // long startDept = pDep[t];
	
  long adj;

  cType ps = 0;
  cType pt = 0;

  //加上两个头，因为默认头dep是指向fa的
  NodeDTTCover shead;
  NodeDTTCover thead;
  shead.dep = pDep[s];
  thead.dep = pDep[t];
  shead.dv = 0;
  thead.dv = 0;
  shead.next = perNodeheadPndcSign0[s];
  thead.next = perNodeheadPndcSign0[t];

  NodeDTTCover* s0pndc = &shead;
  NodeDTTCover* t0pndc = &thead;



  
  if(shead.next == NULL){
    s0pndc = NULL;
  }

  if(thead.next == NULL){
    t0pndc = NULL;
  }

	//long temp;
  while(pDep[s] > pDep[t]){

    adj = getAdj(pnp, stack, &stop, tempMark, s, &s0pndc, NULL, NULL,1,0); //连放带取

    mcv = min(mcv, pCV[s] + pnp->pacc_cut_adjust[s][0] - adj);
    
#ifdef DEBUG_VER6
    printf("s node %ld dep %ld cv %ld adj %ld  -  dv %ld\n",s,pDep[s],pCV[s],pnp->pacc_cut_adjust[s][0],adj);
#endif

    ps = s;
    s = pFa[s];
    tcount ++;

  }

    assert(pDep[s] == pDep[t]);

  if(s == t){
    goto end;
  }

  //此时s和t不相等,一起上溯
  while(s != t){

    adj = getAdj(pnp, stack, &stop, tempMark, s, &s0pndc, NULL, NULL,1,0);


#ifdef DEBUG_VER6    
    printf("s node %ld dep %ld cv %ld adj %ld  -  dv %ld\n",s,pDep[s],pCV[s],pnp->pacc_cut_adjust[s][0],adj);
#endif
    mcv = min(mcv, pCV[s] + pnp->pacc_cut_adjust[s][0] - adj);

    adj = getAdj(pnp, stack2, &stop2, tempMark2, t, &t0pndc, NULL, NULL,1,0);


#ifdef DEBUG_VER6    
    printf("t node %ld dep %ld cv %ld adj %ld  -  dv %ld\n",t,pDep[t],pCV[t],pnp->pacc_cut_adjust[t][0],adj);
#endif
    mcv = min(mcv, pCV[t] + pnp->pacc_cut_adjust[t][0] - adj);

    ps = s;
    pt = t;

    s = pFa[s];
    t = pFa[t];
    tcount+=2;
  }


  /*加上[o,r]里y的逻辑，
  每个分支找
  */
  //两个交叉点下面的头
  NodeDTTCover ashead;
  NodeDTTCover athead;
  NodeDTTCover* as0pndc = &ashead;
  NodeDTTCover* at0pndc = &athead;      

end:
  ashead.dep = pDep[ps];
  athead.dep = pDep[pt];
  ashead.dv = 0;
  athead.dv = 0;
  ashead.next = perNodeheadPndcSign0[ps];
  athead.next = perNodeheadPndcSign0[pt];

  if(ps == 0 || ashead.next == NULL){ as0pndc = NULL;}
  if(pt == 0 || athead.next == NULL){ at0pndc = NULL;}

  cType o = s;
  long adj1;
  long adj2; 

  printf("c o is %ld with depth %ld\n",o, pDep[o]);
  NodeDTTCover oshead;
  NodeDTTCover* o0pndc = &oshead;

  oshead.dep = pDep[o];
  oshead.dv = 0;
  oshead.next = perNodeheadPndcSign0[o];
  if(oshead.next == NULL){
    o0pndc = NULL;
  }

  //Debug
  // NodeDTTCover * tmppndc = o0pndc; //加这一句就不出错
  // if(tmppndc != NULL){
  //   // printf("c ----------------------test o0pdnc\n");
  //   while(tmppndc->dv != MAX_LONG){
  //     // printf("c pndc dep %ld dv %ld\n",tmppndc->dep,tmppndc->dv);
  //     tmppndc = tmppndc->next;
  //   }
  // }




  while(1==1){
    //随着交叉点[o 开始节点向上，还要作为子和祖先分别更新stack3
    //此时要保证o 一定在，返回的adj是考虑sign=1补回，同时考虑o的mdv再次去掉的情况
    adj = getAdj(pnp, stack3, &stop3, tempMark3, o, &o0pndc, &as0pndc, &at0pndc,1,0); //连放带取
    //这个adj是从o开始的调整，之前的不算
      /*
        也就是说，如果o以下的sign=1，这个不需要修复，而是让他们自己选
      */

     //注意：此时adj是把sign=1补回了的情况(不包括o),且把a的mdv又去掉了
     //我们希望的是：不要任何补回，就是看sign=1效果怎样， 然后和a的odv做比较，如果a是sign=1的，不用比就是a的 dv， 如果a 是sign=0的，就是和a的mdv比
     //所以总结，无论如何，就是sign=1和a的mdv比
     //但是下面这里是，得到sign=1，又用mdv补上了：我们希望是取负值较小的那个，同时知道是否有sign=1
     //逻辑怎么处理？
     //如果adj 负值， 取adj 和 mdv较小的那个？
     //如果adj >=0, 就返回adj >= 0;


    adj1 = getAdj(pnp, stack, &stop, tempMark, o, &as0pndc, NULL, NULL, 2, 1); //不作为child向上添加了
    adj2 = getAdj(pnp, stack2, &stop2, tempMark2, o, &at0pndc, NULL, NULL, 2, 1); //不做为chld向上添加了    
    
    // printf("c curN %ld dep %ld adj %ld adj1 %ld adj2 %ld",)
    if(adj < 0){

      //[o y)有sign = 1，而且o保证还在，但是o的mdv被去掉了
      //这时候，就要弄清楚,a,b两个分支是否有sign=1
      //如果某一支sign != 1，说明s还在adj结果里,因为o的mdv也不能把去掉；如果等于1就是已经去掉了
      //所以要补回
      //补回就是看谁绝对值更小，负值就看谁最大

      //此时o一下两个分支需要做减法，反过来想就是：如果[o 之上补回了，那就是整个DTT就补回了，因此现在要在a,b带个分支找最大化的负值(这个取a mdv和这个分支adj最小)，再去掉
      //两个stack做更新，看哪个当前adj比较大

      if(adj1 >=0 && adj2 >= 0)//两个都没有sign==1
      {
        //do nothing
        //两个都在o去掉mdv的结果里，所以这个结果没法用
        adj = 0;
        //以后也不会有了
        break;
      }
      else
      {
        if(adj1 < 0 && adj2 < 0){
          //两个都在里面，则补回负值较小那个
          adj -= (min(adj1, adj2));
        }
        else{
          //只有<0那个被o的mdv去掉了
          //donothing
        }
      }
    } 
    else{
      //[o, y) 没有sign=1
      //此时o没被补回，也没有去mdv，就是正常状态。如果分支有sign=1，就已经考虑在PDTT割值里了
      //因此看两个分支，如果分支有sign=1
      if(adj1 >=0 && adj2 >=0){
        //如果两个分支也没有，那就不能用
        adj = 0;
        break;
      }
      else{
        if(adj1 < 0 && adj2 < 0){
          //两个都在里面，则两个都已经在里面，补回负值较小那个
          adj -= (min(adj1, adj2));
        }
        else{
          //只有<0那个不在里面，因此不用管
          //donothing
        }        
      }
    }

    mcv = min(mcv, pCV[s] + pnp->pacc_cut_adjust[s][0] - adj);
    o = pFa[o];
  }

  free(stack);
  free(stack2);
  free(stack3);
  free(tempMark);
  free(tempMark2);
  free(tempMark3);
  printf("c checked %ld nodes, with %ld used\n",tcount, count);
  // free(adjs);
  // free(adjt);
  return mcv;
}


//////////////////////function to load graph data
void loadGraphData(PreprocData *pd){
  pd->gd = walloc(1,sizeof(GraphData));  
  printf("c\nc hi_treem version 1.1\n");
  printf("c Copyright C by nsyncw, nsyncw@gmail.com\nc\n");

  parse(&(pd->gd->N), &(pd->gd->M), &(pd->gd->nodes));

  printf("c nodes:       %10ld\nc arcs:        %10ld\nc\n", pd->gd->N, pd->gd->M);
}

///////////////////function to initialize data structure
void initPreprocData(PreprocData *pd){
  pd->rd = NULL;
  pd->SPAN_LEN = (int)(sqrt(pd->gd->N));

  pd->roots = (cType *)walloc(pd->total + 2, sizeof(cType));
  pd->allResults = walloc(pd->total+2, sizeof(NodePropArr));

  NodePropArr * allResults = pd->allResults;
  cType len = pd->gd->N + 2;

  calcuTotalCap(pd);

  int LN = pd->LEVEL_NUM+1;
  for (int i = 0; i < pd->total; i++)
  {
    allResults[i].pfa = (cType *)walloc(len, sizeof(cType));
    allResults[i].pdep = (cType *)walloc(len, sizeof(cType));
    allResults[i].pcv = (long *)walloc(len, sizeof(long));
    allResults[i].poh = (long *)walloc(len, sizeof(long));
    allResults[i].pcof = (long *)walloc(len, sizeof(long));
    allResults[i].ps = (short *)walloc(len, sizeof(short));
    allResults[i].pacc_upid = (cType *)walloc(len, sizeof(cType));
    allResults[i].pacc_upmincv = (long *)walloc(len, sizeof(long));
    allResults[i].pacc_pos_upmincv = (cType *)walloc(len, sizeof(cType));

    allResults[i].pdv = (long *)walloc(len, sizeof(long));
    allResults[i].poSgn = (aType *)walloc(len, sizeof(aType));
    allResults[i].pmdv = (long *)walloc(len, sizeof(long));
    allResults[i].podv = (long *)walloc(len, sizeof(long));
    allResults[i].plfa = (cType *)walloc(len, sizeof(cType));
    allResults[i].pseq = (cType *)walloc(len, sizeof(cType));
    allResults[i].perNodeheadPndc = (NodeDTTCover **)walloc(len, sizeof(NodeDTTCover*));
    allResults[i].perNodeheadPndcSign0 = (NodeDTTCover **)walloc(len, sizeof(NodeDTTCover*));
    allResults[i].freePndc = (NodeDTTCover *)walloc(len*NODEDTTMULTI, sizeof(NodeDTTCover));
    allResults[i].idxFreePndc = 0;

    allResults[i].freeAnsAdjustRec = (AnsAdjustRec *)walloc(len*NODEDTTMULTI, sizeof(AnsAdjustRec));
    allResults[i].idxAnsAdjustRec = 0;
    allResults[i].pCh1 = NULL; //(AnsAdjustRec *)walloc(len, sizeof(AnsAdjustRec));
    allResults[i].p2Ch = NULL; //(AnsAdjustRec *)walloc(len, sizeof(AnsAdjustRec));
    allResults[i].pSadj1 = (aType *)walloc(len, sizeof(aType));
    allResults[i].pSadj2 = (aType *)walloc(len, sizeof(aType));
    allResults[i].pRch = (DepRange *)walloc(2*len, sizeof(DepRange));

    long* ptr = (long *)walloc(len*LN, sizeof(long));
    memset(ptr, 0, len * LN * sizeof(long));
    allResults[i].pacc_cut_adjust = (long **)walloc(len, sizeof(long*));
    for(int j = 0; j<len; j++){
      allResults[i].pacc_cut_adjust[j] = ptr+j*LN;
    }


    aType* ptr2 = (aType *)walloc(len*LN, sizeof(aType));
    memset(ptr2, 0, len * LN * sizeof(aType));
    allResults[i].pacc_cut_adjust_sign = (aType **)walloc(len, sizeof(aType*));
    for(int j = 0; j<len; j++){
      allResults[i].pacc_cut_adjust_sign[j] = ptr2+j*LN;
    }    

    memset(allResults[i].pfa, 0, len * sizeof(cType));
    memset(allResults[i].pdep, 0, len * sizeof(cType));
    memset(allResults[i].pcv, 0, len * sizeof(long));
    memset(allResults[i].poh, 0, len * sizeof(long));
    memset(allResults[i].pcof, 0, len * sizeof(long));
    memset(allResults[i].ps, 0, len * sizeof(short));
    memset(allResults[i].pacc_upid, 0, len * sizeof(cType));
    memset(allResults[i].pacc_upmincv, 0, len * sizeof(long));

    memset(allResults[i].pdv, 0, len * sizeof(long));
    memset(allResults[i].poSgn, 0, len * sizeof(aType));
    memset(allResults[i].pmdv, 0, len * sizeof(long));
    memset(allResults[i].podv, 0, len * sizeof(long));
    memset(allResults[i].plfa, 0, len * sizeof(cType));
    memset(allResults[i].pseq, 0, len * sizeof(cType));
    memset(allResults[i].freePndc, 0, len * NODEDTTMULTI * sizeof(NodeDTTCover));
    memset(allResults[i].perNodeheadPndc, 0, len * sizeof(NodeDTTCover*));
    memset(allResults[i].perNodeheadPndcSign0, 0, len * sizeof(NodeDTTCover*));
    // memset(allResults[i].pacc_cut_adjust, 0, len * sizeof(cType*)); //存的指针不能初始化,指针再指向区域已经初始化了
    // memset(allResults[i].pacc_cut_adjust_sign, 0, len * sizeof(aType*));

    memset(allResults[i].pSadj1, 0, len * sizeof(aType));
    memset(allResults[i].pSadj2, 0, len * sizeof(aType));
    memset(allResults[i].pRch, 0, 2*len * sizeof(DepRange));
  }  
}


/////////////////////function to traverse the graph data for multiple times, i.e., Algorithm 2 in the paper
void preProc(PreprocData *pd){
  // double tm;
  // double totalProcTime = 0;
  NodePropArr *allResults = pd->allResults;
  //calculate total cap of one node
  cType root;

  cType len = pd->gd->N + 2;
  long *apply_adj = (long *)walloc(len, sizeof(long));
  cType *depth_map = (cType *)walloc(len, sizeof(cType));

  for (int ipass = 0; ipass < pd->total; ipass++)
  {
    if(pd->rd != NULL){
      free(pd->rd);
      pd->rd = NULL;
    }
    pd->rd = initrand(pd->gd->M*2);    
    // printf("the %d times\n",i);
    pd->gpfa = allResults[ipass].pfa;
    pd->gpdep = allResults[ipass].pdep;
    pd->gpcv = allResults[ipass].pcv;
    pd->gpoh = allResults[ipass].poh;
    pd->gpcof = allResults[ipass].pcof;
    pd->gps = allResults[ipass].ps;
    pd->gpaccup = allResults[ipass].pacc_upid;
    pd->gpaccmcv = allResults[ipass].pacc_upmincv;
    pd->gpaccposmcv = allResults[ipass].pacc_pos_upmincv;
    pd->gcutadj = allResults[ipass].pacc_cut_adjust;
    pd->gcutadjsign = allResults[ipass].pacc_cut_adjust_sign;

    pd->gpdv = allResults[ipass].pdv;
    pd->gpmdv = allResults[ipass].pmdv;
    pd->gpodv = allResults[ipass].podv;
    pd->gpoSgn = allResults[ipass].poSgn;
    pd->gplfa = allResults[ipass].plfa;
    pd->gpseq = allResults[ipass].pseq;
    pd->gFreePndc = allResults[ipass].freePndc;
    pd->gIdxFreePndc = allResults[ipass].idxFreePndc;
    pd->gperNodeheadPndc = allResults[ipass].perNodeheadPndc;
    pd->gperNodeheadPndcSign0 = allResults[ipass].perNodeheadPndcSign0;

    pd->gFreeAnsAdjustRec = allResults[ipass].freeAnsAdjustRec;
    pd->gIdxAnsAdjustRec = allResults[ipass].idxAnsAdjustRec;

    pd->gpCh1 = allResults[ipass].pCh1;
    pd->gp2Ch = allResults[ipass].p2Ch;
    pd->gpSadj1 = allResults[ipass].pSadj1;
    pd->gpSadj2 = allResults[ipass].pSadj2;
    pd->gpRch = allResults[ipass].pRch;

    if (pd->P == 300)
    {
      pd->mode = 3;
    }
    else if(pd->P == 400){
      pd->mode = 4;
    }
    else if(pd->P == 500){
      pd->mode = 5;
    }
    else
    {
      pd->mode = ipass < pd->P * pd->total / 100 ? 1 : 2;
    }
	

    //----------------准备深度遍历   
    // if(pd->mode == 3){
    //   //此时预期是sf图,而根据我们生成sf图的方式,1是度数最大的节点
    //   root = 90000;
    // }
    // else{
    root = pd->gd->N - ipass-1 ;//+ ((mrand(pd->rd) * mrand(pd->rd)) % pd->gd->N);
    // root = ((mrand(pd->rd) * mrand(pd->rd)) % pd->gd->N);
    // }
    pd->roots[ipass] = root;
    pd->gpdep[root] = 0;
    pd->gpseq[root] = 1;
    printf("pass %d before markCut: root is %ld,mode is %d, K is %d\n",ipass,root,pd->mode, pd->K);
    fflush(stdout);
    //printf("pass %d, randidx %ld, root is %ld\n",i, randNumIdx,root);
    // printf("root fa %ld\n",allResults[i].pfa[root]);
    // tm = timer();
    gRoot = root;
    maxDep = 0;
    memset(perNodeposPndc, 0, PERNODE_ARRSIZE *  sizeof(NodeDTTCover*));
    memset(perNodeposPndcLast, 0, PERNODE_ARRSIZE *  sizeof(NodeDTTCover*));
    memset(perNodeposPndcSign0, 0, PERNODE_ARRSIZE *  sizeof(NodeDTTCover*));    

    markCut(root,pd);

    //debug: 看sign=1链的情况
    for(cType tc = 1; tc <=pd->gd->N; tc++){
      NodeDTTCover *pndc = pd->gperNodeheadPndc[tc];
      
      // printf("---curN %ld deppppp %ld\n",tc,pd->gpdep[tc]);
      if(pndc != NULL){
        assert(pndc->dep == pd->gpdep[tc] - 1);
      }
      // while(pndc != NULL){
      //   printf("\t\tpndc->dv %ld dep %ld \n",pndc->dv,pndc->dep);
      //   if(pndc->dv == MAX_LONG){
      //     break;
      //   }
      //   pndc = pndc->next;
      // }
    }
	
    //end

    // printf(" void initTpair(unsigned long tpairs[][4]){ //tpairs\n");
    memset(depM, 0, TARRLENG*sizeof(cType));
    memset(yAAR1, 0, TARRLENG*sizeof(AnsAdjustRec*));
    memset(y2AAR, 0, TARRLENG*sizeof(AnsAdjustRec*));
    ystop1=ystop2=rcht=ch1h=ch1t=ch2h=ch2t=-1;

    printf("now goto step2\n");
    total_cut_edge = total_ok = 0;
    collectCutEdges_step2(root,pd);
    printf("total_cut_edge is %ld/%ld\n",total_cut_edge, pd->gd->M);
    // printf("} //tpairs\n");

    for (cType k = 1; k <= pd->gd->N; k++)
    {
      if (!(perNodeposPndc[k] == NULL || perNodeposPndc[k]->dv == MAX_LONG))
      {
        printf("error: node %ld perNodeposPndc problem \n", k);
        assert(1==0);
      }
      if (!(perNodeposPndcSign0[k] == NULL || perNodeposPndcSign0[k]->dv == MAX_LONG))
      {
        printf("error: node %ld perNodeposPndc problem \n", k);
        assert(1==0);
      }
    }
//debug
    // print_markCut(pd);
    // printf("----------------------------------------------------------------------------------\n");
    // // printPDTT(pd,root,root);
    // printf("----------------------------------------------------------------------------------\n");
    // for(cType ai = 1; ai<=pd->gd->N; ai++){
    //     printf("node %ld(dp %ld), cv %ld, cutadj[0] %ld\n", ai, pd->gpdep[ai],pd->gpcv[ai],pd->gcutadj[ai][0]);   
    //     printPNDC(pd,ai);
    // }
    // for(cType ai = 1; ai<=pd->gd->N; ai++){
    //   for(int lvl = 1; lvl<pd->LEVEL_NUM; lvl++){
    //     if(tempDSgn[ai][lvl] > 0){
    //       printf("*****error: zn %ld(dp %ld), cutadj[%d] %ld != dv %ld\n", ai, pd->gpdep[ai],lvl,pd->gcutadj[ai][lvl] , tempDVal[ai][lvl]);   
    //     }
    //   }
    // }

//end

    // printPDTT(pd,976898,976898);

	// printf("c maxDep is %ld\n",maxDep);
	// for(int i=0; i<(pd->gd->nodes+root)->nIdx; i++){
	// 	cType n = (pd->gd->nodes+root)->edges[i].endNode;
	// 	printf("c root neibor %ld fa %ld\n",n, pd->gpfa[n]);
	// }

    if(pd->mode == 4){
      //print_markCut(pd);
    }

    pd->gpcv[root] = MAX_LONG;
    pd->gpoh[root] = MAX_LONG;


    //----------------准备构建加速结构
    /*
    printf("c before buildAcc\n");
	  fflush(stdout);

    memset(apply_adj, 0, len *  sizeof(long));
    memset(depth_map, 0, len *  sizeof(cType));
    buildAcc(pd, root, root, 0,MAX_LONG, apply_adj,depth_map);
    printf("c after buildAcc\n");
    fflush(stdout);
    */

    // totalProcTime += timer() - tm;
    // printf("c proctime for onepass: %10.06f\n", timer() - tm);
    if (ipass % 10 == 0)
    {
      printf("c the %d passes\n", ipass);
    }
  }

  free(apply_adj);
  free(depth_map);

  // printf("c preprocess times %10.6f\n", totalProcTime);

}


/////////////////////////////////////用来求解最小生成树的函数
////////只找非edge cut的边访问，即w == 9的边不能前进
edgeP** estack = NULL;
int stackTop = 0;
void buildMCT_cost0Tree(cType curN, PreprocData *pd, cType treeId)
{

// printf("call markcut in %ld\n",curN);
  nodeP* nodes = pd->gd->nodes;
  nodeP *np = nodes + curN;
  edgeP *pedges = np->edges;
  int cnt = np->nIdx;
  pd->gpaccmcv[curN] = treeId;

  for (int ni = 0; ni < cnt; ni++)
  {
    // nodeP* znp = nodes+eh->endNode;

    edgeP *eh = pedges + ni;
    cType zn = eh->endNode;
    if(eh->w == MARK_CUT){ //割边
      //不访问另外一端，且只有两边都标记tree_id且不等时，再加入estack
      if(pd->gpaccmcv[zn] > 0 && pd->gpaccmcv[zn] != treeId){
        // printf("stack %ld is point %x \n",stackTop, eh);
        estack[stackTop++] = eh;
        // eh->tmp = eh->cap;
        eh->tmp = 1+(zn+curN)%10;
        // eh->tmp = (rand()%10)+1;
      }
    }
    else{
      if(pd->gpaccmcv[zn] == 0){
        //未访问过，访问之
        buildMCT_cost0Tree(zn,pd,treeId);
      }
      else{
        //已访问过，就不访问了
      }
    }

 
    

  }

}

sType* idxs = NULL;

//对所有找到的割边，结合其他边，通过最小生成树的方法找需要保护的边
void buildMCT(PreprocData *pd){
    //pd->gpaccmcv 记录所属的树
    //边的栈直接放边，可以找到双向边，就能得到所有顶点信息
    long M = pd->gd->M;
    long N = pd->gd->N;
    long len = M + 2;
  

    estack = (edgeP**)walloc(len, sizeof(edgeP*)); 
    memset(estack, 0, len * sizeof(edgeP*));
    stackTop = 0;

    idxs = (sType*)walloc(len,sizeof(sType));
    memset(idxs,0,len*sizeof(sType));  
    
    //buildMCT_cost0Tree 负责实现
    //深度遍历，躲开w=MarkSet的边，直到遍历返回
    //找未染色的点，继续上述过程，直到没有未染色的点
        //不一样的边，放入堆栈
    // cType treeId = 1;
    for(int i=1; i<=N; i++){
      if(pd->gpaccmcv[i] > 0){
        //已访问了，略过    
      }
      else{
        buildMCT_cost0Tree(i,pd, i);
      }
    }
    
    printf("stackTop is %d \n",stackTop);
    // printf("ggggg stack %ld is point %x \n",stackTop, estack[stackTop-1]);
    for(int i=0; i<stackTop; i++){
      idxs[i] = i;
    }

    // printf("ggggg2 stack %ld is point %x \n",stackTop, estack[stackTop-1]);
    //边从小到大排序
    HeapSort2(idxs, estack, stackTop);  
    //For each 边 in stack 处理
    /*
      先根据映射树，确认两边节点所属于的最终的ID
        此时同时更新为id直接映射过去
      如果属于同个ID,则continue下一个
        否则可以合并树，得到仍然是树

        //合并意味着两个树id属于新id(也可以认为新set的集合),这个可以用映射表示(逻辑上近似于一棵树)

    */    
   long usedEdgeCount = 0;
   long totalCost = 0;
    for(int k=0; k<stackTop; k++){
      int i = idxs[k]; // 第k个位置就是排第k的estack中元素下标
      cType n1 = estack[i]->endNode;
      cType n2 = estack[i]->rev->endNode;
      cType t1 = n1;
      while(t1 != pd->gpaccmcv[t1]){
        assert(t1 > 0);
        t1 = pd->gpaccmcv[t1];
      }

      cType t2 = n2;
      while(t2 != pd->gpaccmcv[t2]){
        assert(t2 > 0);
        t2 = pd->gpaccmcv[t2];
      }

      pd->gpaccmcv[n1] = t1;
      pd->gpaccmcv[n2] = t2;

      if(t1 != t2){
        pd->gpaccmcv[t2] = t1;    
        usedEdgeCount ++;
        // printf("c use cut edge %ld %ld with cost %ld\n",n1, n2, estack[i]->tmp);
        totalCost += estack[i] ->tmp;      
      }
      else{
        //do nothing;
      }

    }

    printf("c total, chosen, cost is %ld %ld %ld\n",total_cut_edge,usedEdgeCount,totalCost);

}


// #include "nTXroadc1.c"

  unsigned long tpairs[2000000][4];
//////////////////////////function to calculate multiple random node pairs, i.e., the calling of Algoirthm 3 for multiple times
void calcuRandomPairs(int numOfPairs, PreprocData *pd){

  // initTpair(tpairs);

  double totalTime = 0;
  long mv = MAX_LONG;

  double curTime = 0;
  cType ns, nt;

  if(pd->rd != NULL){
    free(pd->rd);
    pd->rd = NULL;
  }
  pd->rd = initrand(pd->gd->M*2);  
  ns =1, nt = (pd->gd->nodes+ns)->edges[0].endNode; //pd->gd->N;  //第一遍不会执行

  // long *adjs = (long *)walloc(pd->gd->N+1, sizeof(long));
  // long *adjt = (long *)walloc(pd->gd->N+1, sizeof(long));

  for (int ipair = 0; ipair < min(numOfPairs,10000);)
  {
     ns = tpairs[ipair+1][0];
     nt = tpairs[ipair+1][1];

    if (ns != nt)
    {
      // mv = MAX_LONG;
      mv = min((pd->gd->nodes+ns)->totalCap, (pd->gd->nodes+nt)->totalCap);
      
      curTime = timer();
      for (int j = 0; j < pd->total; j++)
      {
        cType root = pd->roots[j];
        // memset(apply_adj, 0, len *  sizeof(long));
        long tmp = solveMaxFlowAccVER6(pd, root,mv, pd->allResults+j, ns, nt,pd->SPAN_LEN,pd->LEVEL_NUM);
        // printf("--solve return %ld\n",tmp);
        if (mv > tmp)
        {
          mv = tmp;
        }
        // printf("--mv is %ld\n",mv);
      }

      if(!((tpairs[ipair+1][2] == 1 && mv <tpairs[ipair+1][3]) || (tpairs[ipair+1][2] == 0 && mv >= tpairs[ipair+1][3]))){
        printf("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ERRR below isCandi=%ld K=%ld\n",tpairs[ipair+1][2],tpairs[ipair+1][3]);
      }

      curTime = timer() - curTime;
      totalTime += curTime;
      ipair++;
      printf("c hi_treem_res(n,s,mflow,tm) %lu %lu %12.01f %12.06f1\n", ns, nt, 1.0 * mv, curTime);
    }
	
	    // printf("%d\n",i);
    // ns = ipair+1; //1 + ((mrand(pd->rd) * mrand(pd->rd)) % (pd->gd->N));
    // nt = ns+1; //1 + ((mrand(pd->rd) * mrand(pd->rd)) % (pd->gd->N));
    ns = 1 + ((mrand(pd->rd) * mrand(pd->rd)) % (pd->gd->N));
    nt = (pd->gd->nodes+ns)->edges[0].endNode;
  }

  printf("c run ok! average time %10.6f\n", totalTime / numOfPairs);


}


