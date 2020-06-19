/**CFile****************************************************************

  FileName    [wlnNtk.c]

  SystemName  [ABC: Logic synthesis and verification system.]

  PackageName [Word-level network.]

  Synopsis    [Network construction procedures.]

  Author      [Alan Mishchenko]
  
  Affiliation [UC Berkeley]

  Date        [Ver. 1.0. Started - September 23, 2018.]

  Revision    [$Id: wlnNtk.c,v 1.00 2018/09/23 00:00:00 alanmi Exp $]

***********************************************************************/

#include "wln.h"

ABC_NAMESPACE_IMPL_START


////////////////////////////////////////////////////////////////////////
///                        DECLARATIONS                              ///
////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
///                     FUNCTION DEFINITIONS                         ///
////////////////////////////////////////////////////////////////////////

/**Function*************************************************************

  Synopsis    [Creating networks.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
Wln_Ntk_t * Wln_NtkAlloc( char * pName, int nObjsMax )
{
    Wln_Ntk_t * p; int i;
    p = ABC_CALLOC( Wln_Ntk_t, 1 );
    p->pName = pName ? Extra_FileNameGeneric( pName ) : NULL;
    Vec_IntGrow( &p->vCis, 111 );
    Vec_IntGrow( &p->vCos, 111 );
    Vec_IntGrow( &p->vFfs, 111 );
    Vec_IntGrow( &p->vTypes,  nObjsMax+1 );
    Vec_StrGrow( &p->vSigns,  nObjsMax+1 );
    Vec_IntGrow( &p->vRanges, nObjsMax+1 );
    Vec_IntPush( &p->vTypes,  -1 );
    Vec_StrPush( &p->vSigns,  -1 );
    Vec_IntPush( &p->vRanges, -1 );
    p->vFanins = ABC_CALLOC( Wln_Vec_t, nObjsMax+1 );
    p->pRanges = Hash_IntManStart( 1000 );
    for ( i = 0; i < 65; i++ )
        Hash_Int2ManInsert( p->pRanges, i, i, 0 );
    for ( i = 1; i < 64; i++ )
        Hash_Int2ManInsert( p->pRanges, i, 0, 0 );
    assert( Hash_IntManEntryNum(p->pRanges) == 128 );
    return p;
}
void Wln_NtkFree( Wln_Ntk_t * p )
{
    int i;
    for ( i = 0; i < Wln_NtkObjNum(p); i++ )
        if ( Wln_ObjFaninNum(p, i) > 2 ) 
            ABC_FREE( p->vFanins[i].pArray[0] );
    ABC_FREE( p->vFanins );

    if ( p->pRanges )  Hash_IntManStop( p->pRanges );
    if ( p->pManName ) Abc_NamStop( p->pManName );

    ABC_FREE( p->vCis.pArray );
    ABC_FREE( p->vCos.pArray );
    ABC_FREE( p->vFfs.pArray );

    ABC_FREE( p->vTypes.pArray );
    ABC_FREE( p->vSigns.pArray );
    ABC_FREE( p->vRanges.pArray );
    ABC_FREE( p->vNameIds.pArray );
    ABC_FREE( p->vInstIds.pArray );
    ABC_FREE( p->vTravIds.pArray );
    ABC_FREE( p->vCopies.pArray );
    ABC_FREE( p->vBits.pArray );
    ABC_FREE( p->vLevels.pArray );
    ABC_FREE( p->vRefs.pArray );
    ABC_FREE( p->vFanout.pArray );
    ABC_FREE( p->vFaninAttrs.pArray );
    ABC_FREE( p->vFaninLists.pArray );

    ABC_FREE( p->pName );
    ABC_FREE( p->pSpec );
    ABC_FREE( p );
}
int Wln_NtkMemUsage( Wln_Ntk_t * p )
{
    int Mem = sizeof(Wln_Ntk_t);
    Mem += 4 * p->vCis.nCap;
    Mem += 4 * p->vCos.nCap;
    Mem += 4 * p->vFfs.nCap;
    Mem += 1 * p->vTypes.nCap;
    Mem += 4 * p->vRanges.nCap;
    Mem += 4 * p->vNameIds.nCap;
    Mem += 4 * p->vInstIds.nCap;
    Mem += 4 * p->vTravIds.nCap;
    Mem += 4 * p->vCopies.nCap;
    Mem += 4 * p->vBits.nCap;
    Mem += 4 * p->vLevels.nCap;
    Mem += 4 * p->vRefs.nCap;
    Mem += 4 * p->vFanout.nCap;
    Mem += 4 * p->vFaninAttrs.nCap;
    Mem += 4 * p->vFaninLists.nCap;
    Mem += 20 * Hash_IntManEntryNum(p->pRanges);
    Mem += Abc_NamMemUsed(p->pManName);
    return Mem;
}
void Wln_NtkPrint( Wln_Ntk_t * p )
{
    int iObj;
    printf( "Printing %d objects of network \"%s\":\n", Wln_NtkObjNum(p), p->pName );
    Wln_NtkForEachObj( p, iObj )
        Wln_ObjPrint( p, iObj );
    printf( "\n" );
}

/**Function*************************************************************

  Synopsis    [Duplicating network.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Wln_NtkTransferNames( Wln_Ntk_t * pNew, Wln_Ntk_t * p )
{
    assert( pNew->pManName  == NULL && p->pManName != NULL );
    pNew->pManName = p->pManName; 
    p->pManName = NULL;
    assert( !Wln_NtkHasCopy(pNew)   && Wln_NtkHasCopy(p)   );
    if ( Wln_NtkHasNameId(p) )
    {
        int i;
        assert( !Wln_NtkHasNameId(pNew) && Wln_NtkHasNameId(p) );
        Wln_NtkCleanNameId( pNew );
        Wln_NtkForEachObj( p, i ) 
            if ( Wln_ObjCopy(p, i) && i < Vec_IntSize(&p->vNameIds) && Wln_ObjNameId(p, i) )
                Wln_ObjSetNameId( pNew, Wln_ObjCopy(p, i), Wln_ObjNameId(p, i) );
        Vec_IntErase( &p->vNameIds );
    }
    if ( Wln_NtkHasInstId(p) )
    {
        int i;
        assert( !Wln_NtkHasInstId(pNew) && Wln_NtkHasInstId(p) );
        Wln_NtkCleanInstId( pNew );
        Wln_NtkForEachObj( p, i ) 
            if ( Wln_ObjCopy(p, i) && i < Vec_IntSize(&p->vInstIds) && Wln_ObjInstId(p, i) )
                Wln_ObjSetInstId( pNew, Wln_ObjCopy(p, i), Wln_ObjInstId(p, i) );
        Vec_IntErase( &p->vInstIds );
    }
}
int Wln_ObjDup( Wln_Ntk_t * pNew, Wln_Ntk_t * p, int iObj )
{
    int i, iFanin, iObjNew = Wln_ObjClone( pNew, p, iObj );
    Wln_ObjForEachFanin( p, iObj, iFanin, i )
        Wln_ObjAddFanin( pNew, iObjNew, Wln_ObjCopy(p, iFanin) );
    if ( Wln_ObjIsConst(p, iObj) )
        Wln_ObjSetConst( pNew, iObjNew, Wln_ObjFanin0(p, iObj) );
    else if ( Wln_ObjIsSlice(p, iObj) || Wln_ObjIsRotate(p, iObj) || Wln_ObjIsTable(p, iObj) )
        Wln_ObjSetFanin( pNew, iObjNew, 1, Wln_ObjFanin1(p, iObj) );
    Wln_ObjSetCopy( p, iObj, iObjNew );
    return iObjNew;
}
int Wln_NtkDupDfs_rec( Wln_Ntk_t * pNew, Wln_Ntk_t * p, int iObj )
{
    int i, iFanin;
    if ( iObj == 0 )
        return 0;
    if ( Wln_ObjCopy(p, iObj) )
        return Wln_ObjCopy(p, iObj);
    //printf( "Visiting node %d\n", iObj );
    assert( !Wln_ObjIsFf(p, iObj) );
    Wln_ObjForEachFanin( p, iObj, iFanin, i )
        Wln_NtkDupDfs_rec( pNew, p, iFanin );
    return Wln_ObjDup( pNew, p, iObj );
}
Wln_Ntk_t * Wln_NtkDupDfs( Wln_Ntk_t * p )
{
    int i, k, iObj, iFanin;
    Wln_Ntk_t * pNew = Wln_NtkAlloc( p->pName, Wln_NtkObjNum(p) );
    pNew->fSmtLib = p->fSmtLib;
    if ( p->pSpec ) pNew->pSpec = Abc_UtilStrsav( p->pSpec );
    Wln_NtkCleanCopy( p );
    Wln_NtkForEachCi( p, iObj, i )
        Wln_ObjDup( pNew, p, iObj );
    Wln_NtkForEachFf( p, iObj, i )
        Wln_ObjSetCopy( p, iObj, Wln_ObjClone(pNew, p, iObj) );
    Wln_NtkForEachCo( p, iObj, i )
        Wln_NtkDupDfs_rec( pNew, p, iObj );
    Wln_NtkForEachFf( p, iObj, i )
        Wln_ObjForEachFanin( p, iObj, iFanin, k )
            Wln_ObjAddFanin( pNew, Wln_ObjCopy(p, iObj), Wln_NtkDupDfs_rec(pNew, p, iFanin) );
    if ( Wln_NtkHasNameId(p) )
        Wln_NtkTransferNames( pNew, p );
    return pNew;
}

/**Function*************************************************************

  Synopsis    [Create fanin/fanout map.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Wln_NtkCreateRefs( Wln_Ntk_t * p )  
{
    int k, iObj, iFanin;
    Wln_NtkCleanRefs( p );
    Wln_NtkForEachObj( p, iObj )
        Wln_ObjForEachFanin( p, iObj, iFanin, k )
            Wln_ObjRefsInc( p, iFanin );
}
int Wln_NtkFaninNum( Wln_Ntk_t * p )
{
    int iObj, nEdges = 0;
    Wln_NtkForEachObj( p, iObj )
        nEdges += Wln_ObjFaninNum(p, iObj);
    return nEdges;
}
void Wln_NtkStartFaninMap( Wln_Ntk_t * p, Vec_Int_t * vFaninMap, int nMulti )
{
    int iObj, iOffset = Wln_NtkObjNum(p);
    Vec_IntFill( vFaninMap, iOffset + nMulti * Wln_NtkFaninNum(p), 0 );
    Wln_NtkForEachObj( p, iObj )
    {
        Vec_IntWriteEntry( vFaninMap, iObj, iOffset );
        iOffset += nMulti * Wln_ObjFaninNum(p, iObj);
    }
    assert( iOffset == Vec_IntSize(vFaninMap) );
}
void Wln_NtkStartFanoutMap( Wln_Ntk_t * p, Vec_Int_t * vFanoutMap, Vec_Int_t * vFanoutNums, int nMulti )
{
    int iObj, iOffset = Wln_NtkObjNum(p);
    Vec_IntFill( vFanoutMap, iOffset + nMulti * Vec_IntSum(vFanoutNums), 0 );
    Wln_NtkForEachObj( p, iObj )
    {
        Vec_IntWriteEntry( vFanoutMap, iObj, iOffset );
        iOffset += nMulti * Wln_ObjRefs(p, iObj);
    }
    assert( iOffset == Vec_IntSize(vFanoutMap) );
}

/**Function*************************************************************

  Synopsis    [Static fanout.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Wln_NtkStaticFanoutStart( Wln_Ntk_t * p )
{
    int k, iObj, iFanin;
    Vec_Int_t * vRefsCopy = Vec_IntAlloc(0);
    Wln_NtkCreateRefs( p );
    Wln_NtkStartFanoutMap( p, &p->vFanout, &p->vRefs, 1 );
    ABC_SWAP( Vec_Int_t, *vRefsCopy, p->vRefs );
    // add fanouts
    Wln_NtkCleanRefs( p );
    Wln_NtkForEachObj( p, iObj )
        Wln_ObjForEachFanin( p, iObj, iFanin, k )
            Wln_ObjSetFanout( p, iFanin, Wln_ObjRefsInc(p, iFanin), iObj );
    // double-check the current number of fanouts added
    Wln_NtkForEachObj( p, iObj )
        assert( Wln_ObjRefs(p, iObj) == Vec_IntEntry(vRefsCopy, iObj) );
    Vec_IntFree( vRefsCopy );
}
void Wln_NtkStaticFanoutStop( Wln_Ntk_t * p )
{
    Vec_IntErase( &p->vRefs );
    Vec_IntErase( &p->vFanout );
}
void Wln_NtkStaticFanoutTest( Wln_Ntk_t * p )
{
    int k, iObj, iFanout;
    printf( "Printing fanouts of %d objects of network \"%s\":\n", Wln_NtkObjNum(p), p->pName );
    Wln_NtkStaticFanoutStart( p );
    Wln_NtkForEachObj( p, iObj )
    {
        Wln_ObjPrint( p, iObj );
        printf( "   Fanouts : " );
        Wln_ObjForEachFanoutStatic( p, iObj, iFanout, k )
            printf( "%5d ", iFanout );
        printf( "\n" );
    }
    Wln_NtkStaticFanoutStop( p );
    printf( "\n" );
}

////////////////////////////////////////////////////////////////////////
///                       END OF FILE                                ///
////////////////////////////////////////////////////////////////////////


ABC_NAMESPACE_IMPL_END

