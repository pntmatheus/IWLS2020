/**CFile****************************************************************

  FileName    [giaSim.c]

  SystemName  [ABC: Logic synthesis and verification system.]

  PackageName [Scalable AIG package.]

  Synopsis    [Fast sequential simulator.]

  Author      [Alan Mishchenko]
  
  Affiliation [UC Berkeley]

  Date        [Ver. 1.0. Started - June 20, 2005.]

  Revision    [$Id: giaSim.c,v 1.00 2005/06/20 00:00:00 alanmi Exp $]

***********************************************************************/

#include "gia.h"
#include "misc/util/utilTruth.h"
#include "misc/extra/extra.h"

ABC_NAMESPACE_IMPL_START


////////////////////////////////////////////////////////////////////////
///                        DECLARATIONS                              ///
////////////////////////////////////////////////////////////////////////


typedef struct Gia_SimRsbMan_t_ Gia_SimRsbMan_t;
struct Gia_SimRsbMan_t_
{
    Gia_Man_t *    pGia;
    Vec_Int_t *    vTfo;
    Vec_Int_t *    vCands;
    Vec_Int_t *    vFanins;
    Vec_Int_t *    vFanins2;
    Vec_Wrd_t *    vSimsObj;
    Vec_Wrd_t *    vSimsObj2;
    int            nWords;
    word *         pFunc[3];
};


typedef struct Gia_SimAbsMan_t_ Gia_SimAbsMan_t;
struct Gia_SimAbsMan_t_
{
    // problem formulation
    Gia_Man_t *    pGia;       // AIG manager
    word *         pSet[2];    // offset/onset truth tables
    int            nCands;     // candidate count
    int            nWords;     // word count
    Vec_Wrd_t *    vSims;      // candidate simulation info
    Vec_Int_t *    vResub;     // the result
    int            fVerbose;   // verbose
    // intermediate result
    Vec_Int_t *    vValues;       // function values in each pattern
    Vec_Int_t *    vPatPairs;     // used minterms
    int            nWordsTable;   // words of table data
    word *         pTableTemp;    // temporary table pattern
    Vec_Wrd_t *    vCoverTable;   // columns = minterms; rows = classes
    Vec_Int_t *    vTtMints;      // truth table minterms
};

////////////////////////////////////////////////////////////////////////
///                     FUNCTION DEFINITIONS                         ///
////////////////////////////////////////////////////////////////////////

/**Function*************************************************************

  Synopsis    []

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Gia_ManSimPatAssignInputs( Gia_Man_t * p, int nWords, Vec_Wrd_t * vSims, Vec_Wrd_t * vSimsIn )
{
    int i, Id;
    assert( Vec_WrdSize(vSims)   == nWords * Gia_ManObjNum(p) );
    assert( Vec_WrdSize(vSimsIn) == nWords * Gia_ManCiNum(p) );
    Gia_ManForEachCiId( p, Id, i )
        memcpy( Vec_WrdEntryP(vSims, Id*nWords), Vec_WrdEntryP(vSimsIn, i*nWords), sizeof(word)*nWords );
}
static inline void Gia_ManSimPatSimAnd( Gia_Man_t * p, int i, Gia_Obj_t * pObj, int nWords, Vec_Wrd_t * vSims )
{
    word pComps[2] = { 0, ~(word)0 };
    word Diff0 = pComps[Gia_ObjFaninC0(pObj)];
    word Diff1 = pComps[Gia_ObjFaninC1(pObj)];
    word * pSims  = Vec_WrdArray(vSims);
    word * pSims0 = pSims + nWords*Gia_ObjFaninId0(pObj, i);
    word * pSims1 = pSims + nWords*Gia_ObjFaninId1(pObj, i);
    word * pSims2 = pSims + nWords*i; int w;
    if ( Gia_ObjIsXor(pObj) )
        for ( w = 0; w < nWords; w++ )
            pSims2[w] = (pSims0[w] ^ Diff0) ^ (pSims1[w] ^ Diff1);
    else
        for ( w = 0; w < nWords; w++ )
            pSims2[w] = (pSims0[w] ^ Diff0) & (pSims1[w] ^ Diff1);
}
static inline void Gia_ManSimPatSimPo( Gia_Man_t * p, int i, Gia_Obj_t * pObj, int nWords, Vec_Wrd_t * vSims )
{
    word pComps[2] = { 0, ~(word)0 };
    word Diff0     = pComps[Gia_ObjFaninC0(pObj)];
    word * pSims   = Vec_WrdArray(vSims);
    word * pSims0  = pSims + nWords*Gia_ObjFaninId0(pObj, i);
    word * pSims2  = pSims + nWords*i; int w;
    for ( w = 0; w < nWords; w++ )
        pSims2[w] = (pSims0[w] ^ Diff0);
}
Vec_Wrd_t * Gia_ManSimPatSim( Gia_Man_t * pGia )
{
    Gia_Obj_t * pObj;
    int i, nWords = Vec_WrdSize(pGia->vSimsPi) / Gia_ManCiNum(pGia);
    Vec_Wrd_t * vSims = Vec_WrdStart( Gia_ManObjNum(pGia) * nWords );
    assert( Vec_WrdSize(pGia->vSimsPi) % Gia_ManCiNum(pGia) == 0 );
    Gia_ManSimPatAssignInputs( pGia, nWords, vSims, pGia->vSimsPi );
    Gia_ManForEachAnd( pGia, pObj, i ) 
        Gia_ManSimPatSimAnd( pGia, i, pObj, nWords, vSims );
    Gia_ManForEachCo( pGia, pObj, i )
        Gia_ManSimPatSimPo( pGia, Gia_ObjId(pGia, pObj), pObj, nWords, vSims );
    return vSims;
}

/**Function*************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

***********************************************************************/
void Gia_ManSimPatValuesDerive( Gia_Man_t * p, int nWords, Vec_Wrd_t * vSims, Vec_Wrd_t * vValues )
{
    int i, Id;
    assert( Vec_WrdSize(vSims)   == nWords * Gia_ManObjNum(p) );
    assert( Vec_WrdSize(vValues) == nWords * Gia_ManCoNum(p)  );
    Gia_ManForEachCoId( p, Id, i )
        memcpy( Vec_WrdEntryP(vValues, nWords * i), Vec_WrdEntryP(vSims, nWords * Id), sizeof(word)* nWords );
}
Vec_Wrd_t * Gia_ManSimPatValues( Gia_Man_t * p )
{
    int i, Id, nWords   = Vec_WrdSize(p->vSimsPi) / Gia_ManCiNum(p);
    Vec_Wrd_t * vSims   = Gia_ManSimPatSim( p ); 
    Vec_Wrd_t * vValues = Vec_WrdStart( Gia_ManCoNum(p)  * nWords ); 
    assert( Vec_WrdSize(p->vSimsPi) == nWords * Gia_ManCiNum(p)  );
    assert( Vec_WrdSize(vValues)    == nWords * Gia_ManCoNum(p)  );
    assert( Vec_WrdSize(vSims)      == nWords * Gia_ManObjNum(p) );
    Gia_ManForEachCoId( p, Id, i )
        memcpy( Vec_WrdEntryP(vValues, nWords * i), Vec_WrdEntryP(vSims, nWords * Id), sizeof(word)* nWords );
    Vec_WrdFree( vSims );
    return vValues;
}


/**Function*************************************************************

  Synopsis    []

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
Vec_Wrd_t * Gia_ManSimCombine( int nInputs, Vec_Wrd_t * vBase, Vec_Wrd_t * vAddOn, int nWordsUse )
{
    int nWordsBase  = Vec_WrdSize(vBase)  / nInputs;
    int nWordsAddOn = Vec_WrdSize(vAddOn) / nInputs; int i, w;
    Vec_Wrd_t * vSimsIn = Vec_WrdAlloc( nInputs * (nWordsBase + nWordsUse) );
    assert( Vec_WrdSize(vBase)  % nInputs == 0 );
    assert( Vec_WrdSize(vAddOn) % nInputs == 0 );
    assert( nWordsUse <= nWordsAddOn );
    for ( i = 0; i < nInputs; i++ )
    {
        word * pSimsB = nWordsBase  ? Vec_WrdEntryP( vBase,  i * nWordsBase )  : NULL;
        word * pSimsA = nWordsAddOn ? Vec_WrdEntryP( vAddOn, i * nWordsAddOn ) : NULL;
        for ( w = 0; w < nWordsBase; w++ )
            Vec_WrdPush( vSimsIn, pSimsB[w] );
        for ( w = 0; w < nWordsUse; w++ )
            Vec_WrdPush( vSimsIn, pSimsA[w] );
    }
    assert( Vec_WrdSize(vSimsIn) == Vec_WrdCap(vSimsIn) || Vec_WrdSize(vSimsIn) < 16 );
    return vSimsIn;
}
int Gia_ManSimBitPackOne( int nWords, Vec_Wrd_t * vSimsIn, Vec_Wrd_t * vSimsCare, int iPat, int * pLits, int nLits )
{
    word * pSimsI, * pSimsC; int i, k;
    for ( i = 0; i < iPat; i++ )
    {
        for ( k = 0; k < nLits; k++ )
        {
            int iVar = Abc_Lit2Var( pLits[k] );
            pSimsI = Vec_WrdEntryP( vSimsIn,   nWords * iVar );
            pSimsC = Vec_WrdEntryP( vSimsCare, nWords * iVar );
            if ( Abc_TtGetBit(pSimsC, i) && (Abc_TtGetBit(pSimsI, i) == Abc_LitIsCompl(pLits[k])) )
                break;
        }
        if ( k == nLits )
            break;
    }
    for ( k = 0; k < nLits; k++ )
    {
        int iVar = Abc_Lit2Var( pLits[k] );
        pSimsI = Vec_WrdEntryP( vSimsIn,   nWords * iVar );
        pSimsC = Vec_WrdEntryP( vSimsCare, nWords * iVar );
        if ( !Abc_TtGetBit(pSimsC, i) && Abc_TtGetBit(pSimsI, i) == Abc_LitIsCompl(pLits[k]) )
            Abc_TtXorBit( pSimsI, i );
        Abc_TtSetBit( pSimsC, i );
        assert( Abc_TtGetBit(pSimsC, i) && (Abc_TtGetBit(pSimsI, i) != Abc_LitIsCompl(pLits[k])) );
    }
    return (int)(i == iPat);
}
Vec_Wrd_t * Gia_ManSimBitPacking( Gia_Man_t * p, Vec_Int_t * vCexStore, int nCexes )
{
    int c, iCur = 0, iPat = 0;
    int nWordsMax = Abc_Bit6WordNum( nCexes ); 
    Vec_Wrd_t * vSimsIn   = Vec_WrdStartRandom( Gia_ManCiNum(p) * nWordsMax );
    Vec_Wrd_t * vSimsCare = Vec_WrdStart( Gia_ManCiNum(p) * nWordsMax );
    Vec_Wrd_t * vSimsRes  = NULL;
    for ( c = 0; c < nCexes; c++ )
    {
        int Out  = Vec_IntEntry( vCexStore, iCur++ );
        int Size = Vec_IntEntry( vCexStore, iCur++ );
        iPat += Gia_ManSimBitPackOne( nWordsMax, vSimsIn, vSimsCare, iPat, Vec_IntEntryP(vCexStore, iCur), Size );
        iCur += Size;
        assert( iPat <= nCexes );
        Out = 0;
    }
    printf( "Compressed %d CEXes into %d test patterns.\n", nCexes, iPat );
    assert( iCur == Vec_IntSize(vCexStore) );
    vSimsRes = Gia_ManSimCombine( Gia_ManCiNum(p), p->vSimsPi, vSimsIn, Abc_Bit6WordNum(iPat+1) );
    printf( "Combined %d words of the original info with %d words of additional info.\n", 
        Vec_WrdSize(p->vSimsPi) / Gia_ManCiNum(p), Abc_Bit6WordNum(iPat+1) );
    Vec_WrdFree( vSimsIn );
    Vec_WrdFree( vSimsCare );
    return vSimsRes;
}


/**Function*************************************************************

  Synopsis    []

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
int Gia_ManSimPatHashPatterns( Gia_Man_t * p, int nWords, Vec_Wrd_t * vSims, int * pnC0, int * pnC1 )
{
    Gia_Obj_t * pObj; 
    int i, nUnique;
    Vec_Mem_t * vStore;
    vStore = Vec_MemAlloc( nWords, 12 ); // 2^12 N-word entries per page
    Vec_MemHashAlloc( vStore, 1 << 12 );
    Gia_ManForEachCand( p, pObj, i )
    {
        word * pSim = Vec_WrdEntryP(vSims, i*nWords);
        if ( pnC0 && Abc_TtIsConst0(pSim, nWords) )
            (*pnC0)++;
        if ( pnC1 && Abc_TtIsConst1(pSim, nWords) )
            (*pnC1)++;
        Vec_MemHashInsert( vStore, pSim );
    }
    nUnique = Vec_MemEntryNum( vStore );
    Vec_MemHashFree( vStore );
    Vec_MemFree( vStore );
    return nUnique;
}
Gia_Man_t * Gia_ManSimPatGenMiter( Gia_Man_t * p, Vec_Wrd_t * vSims )
{
    Gia_Man_t * pNew;
    Gia_Obj_t * pObj;
    int i, nWords = Vec_WrdSize(vSims) / Gia_ManObjNum(p);
    pNew = Gia_ManStart( Gia_ManObjNum(p) + Gia_ManCoNum(p) );
    Gia_ManHashStart( pNew );
    Gia_ManConst0(p)->Value = 0;
    Gia_ManForEachCi( p, pObj, i )
        pObj->Value = Gia_ManAppendCi( pNew );
    Gia_ManForEachAnd( p, pObj, i )
        pObj->Value = Gia_ManHashAnd( pNew, Gia_ObjFanin0Copy(pObj), Gia_ObjFanin1Copy(pObj) );
    Gia_ManForEachAnd( p, pObj, i )
    {
        word * pSim = Vec_WrdEntryP(vSims, i*nWords);
        if ( Abc_TtIsConst0(pSim, nWords) )
            Gia_ManAppendCo( pNew, Abc_LitNotCond(pObj->Value, 0) );
        if ( Abc_TtIsConst1(pSim, nWords) )
            Gia_ManAppendCo( pNew, Abc_LitNotCond(pObj->Value, 1) );
    }
    Gia_ManHashStop( pNew );
    return pNew;
}


/**Function*************************************************************

  Synopsis    []

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Gia_ManSimPatWriteOne( FILE * pFile, word * pSim, int nWords )
{
    int k, Digit, nDigits = nWords*16;
    for ( k = 0; k < nDigits; k++ )
    {
        Digit = (int)((pSim[k/16] >> ((k%16) * 4)) & 15);
        if ( Digit < 10 )
            fprintf( pFile, "%d", Digit );
        else
            fprintf( pFile, "%c", 'A' + Digit-10 );
    }
    fprintf( pFile, "\n" );
}
void Gia_ManSimPatWrite( char * pFileName, Vec_Wrd_t * vSimsIn, int nWords )
{
    int i, nNodes = Vec_WrdSize(vSimsIn) / nWords;
    FILE * pFile = fopen( pFileName, "wb" );
    if ( pFile == NULL )
    {
        printf( "Cannot open file \"%s\" for writing.\n", pFileName );
        return;
    }
    assert( Vec_WrdSize(vSimsIn) % nWords == 0 );
    for ( i = 0; i < nNodes; i++ )
        Gia_ManSimPatWriteOne( pFile, Vec_WrdEntryP(vSimsIn, i*nWords), nWords );
    fclose( pFile );
    printf( "Written %d words of simulation data.\n", nWords );
}
int Gia_ManSimPatReadOne( char c )
{
    int Digit = 0;
    if ( c >= '0' && c <= '9' )
        Digit = c - '0';
    else if ( c >= 'A' && c <= 'F' )
        Digit = c - 'A' + 10;
    else if ( c >= 'a' && c <= 'f' )
        Digit = c - 'a' + 10;
    else assert( 0 );
    assert( Digit >= 0 && Digit < 16 );
    return Digit;
}
Vec_Wrd_t * Gia_ManSimPatRead( char * pFileName )
{
    Vec_Wrd_t * vSimsIn = NULL; 
    int c, nWords = -1, nChars = 0; word Num = 0;
    FILE * pFile = fopen( pFileName, "rb" );
    if ( pFile == NULL )
    {
        printf( "Cannot open file \"%s\" for reading.\n", pFileName );
        return NULL;
    }
    vSimsIn = Vec_WrdAlloc( 1000 );
    while ( (c = fgetc(pFile)) != EOF )
    {
        if ( c == '\n' && nWords == -1 )
            nWords = Vec_WrdSize(vSimsIn);
        if ( c == '\n' || c == '\r' || c == '\t' || c == ' ' )
            continue;
        Num |= (word)Gia_ManSimPatReadOne((char)c) << (nChars * 4);
        if ( ++nChars < 16 )
            continue;
        Vec_WrdPush( vSimsIn, Num );
        nChars = 0; 
        Num = 0;
    }
    assert( Vec_WrdSize(vSimsIn) % nWords == 0 );
    fclose( pFile );
    printf( "Read %d words of simulation data.\n", nWords );
    return vSimsIn;
}

/**Function*************************************************************

  Synopsis    []

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Gia_ManSimProfile( Gia_Man_t * pGia )
{
    Vec_Wrd_t * vSims = Gia_ManSimPatSim( pGia );
    int nWords = Vec_WrdSize(vSims) / Gia_ManObjNum(pGia);
    int nC0s = 0, nC1s = 0, nUnique = Gia_ManSimPatHashPatterns( pGia, nWords, vSims, &nC0s, &nC1s );
    printf( "Simulating %d words leads to %d unique objects (%.2f %% out of %d), Const0 = %d. Const1 = %d.\n", 
        nWords, nUnique, 100.0*nUnique/Gia_ManCandNum(pGia), Gia_ManCandNum(pGia), nC0s, nC1s );
    Vec_WrdFree( vSims );
}
void Gia_ManSimPat( Gia_Man_t * p, int nWords0, int fVerbose )
{
    extern Vec_Int_t * Cbs2_ManSolveMiterNc( Gia_Man_t * pAig, int nConfs, Vec_Str_t ** pvStatus, int fVerbose );
    int i, Status, Counts[3] = {0};
    Gia_Man_t * pGia;
    Vec_Wrd_t * vSimsIn = NULL;
    Vec_Str_t * vStatus = NULL;
    Vec_Int_t * vCexStore = NULL;
    Vec_Wrd_t * vSims = Gia_ManSimPatSim( p );
    Gia_ManSimProfile( p );
    pGia  = Gia_ManSimPatGenMiter( p, vSims );
    vCexStore = Cbs2_ManSolveMiterNc( pGia, 1000, &vStatus, 0 );
    Gia_ManStop( pGia );
    Vec_StrForEachEntry( vStatus, Status, i )
    {
        assert( Status >= -1 && Status <= 1 );
        Counts[Status+1]++;
    }
    printf( "Total = %d : SAT = %d.  UNSAT = %d.  UNDEC = %d.\n", Counts[1]+Counts[2]+Counts[0], Counts[1], Counts[2], Counts[0] );
    if ( Counts[1] == 0 )
        printf( "There are no counter-examples.  No need for more simulation.\n" );
    else
    {
        vSimsIn = Gia_ManSimBitPacking( p, vCexStore, Counts[1] );
        Vec_WrdFreeP( &p->vSimsPi );
        p->vSimsPi = vSimsIn;
        Gia_ManSimProfile( p );
    }
    Vec_StrFree( vStatus );
    Vec_IntFree( vCexStore );
    Vec_WrdFree( vSims );
}




/**Function*************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

***********************************************************************/
Gia_SimRsbMan_t * Gia_SimRsbAlloc( Gia_Man_t * pGia )
{
    Gia_SimRsbMan_t * p = ABC_CALLOC( Gia_SimRsbMan_t, 1 );
    p->pGia      = pGia;
    p->nWords    = Vec_WrdSize(pGia->vSimsPi) / Gia_ManCiNum(pGia); assert( Vec_WrdSize(pGia->vSimsPi) % Gia_ManCiNum(pGia) == 0 );
    p->pFunc[0]  = ABC_CALLOC( word, p->nWords );
    p->pFunc[1]  = ABC_CALLOC( word, p->nWords );
    p->pFunc[2]  = ABC_CALLOC( word, p->nWords );
    p->vTfo      = Vec_IntAlloc( 1000 );
    p->vCands    = Vec_IntAlloc( 1000 );
    p->vFanins   = Vec_IntAlloc( 10 );
    p->vFanins2  = Vec_IntAlloc( 10 );
    p->vSimsObj  = Gia_ManSimPatSim( pGia );
    p->vSimsObj2 = Vec_WrdStart( Vec_WrdSize(p->vSimsObj) );
    assert( p->nWords == Vec_WrdSize(p->vSimsObj) / Gia_ManObjNum(pGia) );
    Gia_ManStaticFanoutStart( pGia );
    return p;
}
void Gia_SimRsbFree( Gia_SimRsbMan_t * p )
{
    Gia_ManStaticFanoutStop( p->pGia );
    Vec_IntFree( p->vTfo );
    Vec_IntFree( p->vCands );
    Vec_IntFree( p->vFanins );
    Vec_IntFree( p->vFanins2 );
    Vec_WrdFree( p->vSimsObj );
    Vec_WrdFree( p->vSimsObj2 );
    ABC_FREE( p->pFunc[0] );
    ABC_FREE( p->pFunc[1] );
    ABC_FREE( p->pFunc[2] );
    ABC_FREE( p );
}

/**Function*************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

***********************************************************************/
void Gia_SimRsbTfo_rec( Gia_Man_t * p, int iObj, int iFanout, Vec_Int_t * vTfo )
{
    int i, iFan;
    if ( Gia_ObjIsTravIdCurrentId(p, iObj) )
        return;
    Gia_ObjSetTravIdCurrentId(p, iObj);
    Gia_ObjForEachFanoutStaticId( p, iObj, iFan, i )
        if ( iFanout == -1 || iFan == iFanout )
            Gia_SimRsbTfo_rec( p, iFan, -1, vTfo );
    Vec_IntPush( vTfo, iObj );
}
Vec_Int_t * Gia_SimRsbTfo( Gia_SimRsbMan_t * p, int iObj, int iFanout )
{
    assert( iObj > 0 );
    Vec_IntClear( p->vTfo );
    Gia_ManIncrementTravId( p->pGia );
    Gia_SimRsbTfo_rec( p->pGia, iObj, iFanout, p->vTfo );
    assert( Vec_IntEntryLast(p->vTfo) == iObj );
    Vec_IntPop( p->vTfo );
    Vec_IntReverseOrder( p->vTfo );
    Vec_IntSort( p->vTfo, 0 );
    return p->vTfo;
}

/**Function*************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

***********************************************************************/
word * Gia_SimRsbFunc( Gia_SimRsbMan_t * p, int iObj, Vec_Int_t * vFanins, int fOnSet )
{
    int nTruthWords = Abc_Truth6WordNum( Vec_IntSize(vFanins) );
    word * pTruth   = ABC_CALLOC( word, nTruthWords ); 
    word * pFunc    = Vec_WrdEntryP( p->vSimsObj, p->nWords*iObj );
    word * pFanins[16] = {NULL};  int s, b, iMint, i, iFanin;
    assert( Vec_IntSize(vFanins) <= 16 );
    Vec_IntForEachEntry( vFanins, iFanin, i )
        pFanins[i] = Vec_WrdEntryP( p->vSimsObj, p->nWords*iFanin );
    for ( s = 0; s < 64*p->nWords; s++ )
    {
        if ( !Abc_TtGetBit(p->pFunc[2], s) || Abc_TtGetBit(pFunc, s) != fOnSet )
            continue;
        iMint = 0;
        for ( b = 0; b < Vec_IntSize(vFanins); b++ )
            if ( Abc_TtGetBit(pFanins[b], s) )
                iMint |= 1 << b;
        Abc_TtSetBit( pTruth, iMint );
    }
    return pTruth;
}
int Gia_SimRsbResubVerify( Gia_SimRsbMan_t * p, int iObj, Vec_Int_t * vFanins )
{
    word * pTruth0 = Gia_SimRsbFunc( p, iObj, p->vFanins, 0 );
    word * pTruth1 = Gia_SimRsbFunc( p, iObj, p->vFanins, 1 );
    int Res = !Abc_TtIntersect( pTruth0, pTruth1, p->nWords, 0 );
    ABC_FREE( pTruth0 );
    ABC_FREE( pTruth1 );
    return Res;
}

/**Function*************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

***********************************************************************/
static inline void Gia_SimRsbSimAndCareSet( Gia_Man_t * p, int i, Gia_Obj_t * pObj, int nWords, Vec_Wrd_t * vSims, Vec_Wrd_t * vSims2 )
{
    word pComps[2] = { 0, ~(word)0 };
    word Diff0 = pComps[Gia_ObjFaninC0(pObj)];
    word Diff1 = pComps[Gia_ObjFaninC1(pObj)];
    Vec_Wrd_t * vSims0 = Gia_ObjIsTravIdCurrentId(p, Gia_ObjFaninId0(pObj, i)) ? vSims2 : vSims;
    Vec_Wrd_t * vSims1 = Gia_ObjIsTravIdCurrentId(p, Gia_ObjFaninId1(pObj, i)) ? vSims2 : vSims;
    word * pSims0 = Vec_WrdEntryP( vSims0, nWords*Gia_ObjFaninId0(pObj, i) );
    word * pSims1 = Vec_WrdEntryP( vSims1, nWords*Gia_ObjFaninId1(pObj, i) );
    word * pSims2 = Vec_WrdEntryP( vSims2, nWords*i ); int w;
    if ( Gia_ObjIsXor(pObj) )
        for ( w = 0; w < nWords; w++ )
            pSims2[w] = (pSims0[w] ^ Diff0) ^ (pSims1[w] ^ Diff1);
    else
        for ( w = 0; w < nWords; w++ )
            pSims2[w] = (pSims0[w] ^ Diff0) & (pSims1[w] ^ Diff1);
}
word * Gia_SimRsbCareSet( Gia_SimRsbMan_t * p, int iObj, Vec_Int_t * vTfo )
{
    word * pSims  = Vec_WrdEntryP( p->vSimsObj,  p->nWords*iObj );
    word * pSims2 = Vec_WrdEntryP( p->vSimsObj2, p->nWords*iObj );  int iNode, i;
    Abc_TtCopy( pSims2, pSims, p->nWords, 1 );
    Abc_TtClear( p->pFunc[2], p->nWords );
    Vec_IntForEachEntry( vTfo, iNode, i )
    {
        Gia_Obj_t * pNode = Gia_ManObj(p->pGia, iNode);
        if ( Gia_ObjIsAnd(pNode) )
            Gia_SimRsbSimAndCareSet( p->pGia, iNode, pNode, p->nWords, p->vSimsObj, p->vSimsObj2 );
        else if ( Gia_ObjIsCo(pNode) )
        {
            word * pSimsA = Vec_WrdEntryP( p->vSimsObj,  p->nWords*Gia_ObjFaninId0p(p->pGia, pNode) );
            word * pSimsB = Vec_WrdEntryP( p->vSimsObj2, p->nWords*Gia_ObjFaninId0p(p->pGia, pNode) );
            Abc_TtOrXor( p->pFunc[2], pSimsA, pSimsB, p->nWords );
        }
        else assert( 0 );
    }
    return p->pFunc[2];
}


/**Function*************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

***********************************************************************/
void Gia_ObjSimCollect( Gia_SimRsbMan_t * p )
{
    int i, k, iTemp, iFanout;
    Vec_IntClear( p->vFanins2 );
    assert( Vec_IntSize(p->vFanins) > 0 );
    Vec_IntForEachEntry( p->vFanins, iTemp, i )
    {
        Gia_Obj_t * pObj = Gia_ManObj( p->pGia, iTemp );
        if ( Gia_ObjIsAnd(pObj) && !Gia_ObjIsTravIdCurrentId( p->pGia, Gia_ObjFaninId0(pObj, iTemp) ) )
            Vec_IntPush( p->vFanins2, Gia_ObjFaninId0(pObj, iTemp) );
        if ( Gia_ObjIsAnd(pObj) && !Gia_ObjIsTravIdCurrentId( p->pGia, Gia_ObjFaninId1(pObj, iTemp) ) )
            Vec_IntPush( p->vFanins2, Gia_ObjFaninId1(pObj, iTemp) );
        Gia_ObjForEachFanoutStaticId( p->pGia, iTemp, iFanout, k )
            if ( Gia_ObjIsAnd(Gia_ManObj(p->pGia, iFanout)) && !Gia_ObjIsTravIdCurrentId( p->pGia, iFanout ) )
                Vec_IntPush( p->vFanins2, iFanout );
    }
}
Vec_Int_t * Gia_ObjSimCands( Gia_SimRsbMan_t * p, int iObj, int nCands )
{
    assert( iObj > 0 );
    assert( Gia_ObjIsAnd(Gia_ManObj(p->pGia, iObj)) );
    Vec_IntClear( p->vCands );
    Vec_IntFill( p->vFanins, 1, iObj );
    while ( Vec_IntSize(p->vFanins) > 0 && Vec_IntSize(p->vCands) < nCands )
    {
        int i, iTemp;
        Vec_IntForEachEntry( p->vFanins, iTemp, i )
            Gia_ObjSetTravIdCurrentId( p->pGia, iTemp );
        Gia_ObjSimCollect( p ); // p->vFanins -> p->vFanins2
        Vec_IntAppend( p->vCands, p->vFanins2 );
        ABC_SWAP( Vec_Int_t *, p->vFanins, p->vFanins2 );
    }
    assert( Vec_IntSize(p->vFanins) == 0 || Vec_IntSize(p->vCands) >= nCands );
    if ( Vec_IntSize(p->vCands) > nCands )
        Vec_IntShrink( p->vCands, nCands );
    return p->vCands;
}

/**Function*************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

***********************************************************************/
int Gia_ObjSimRsb( Gia_SimRsbMan_t * p, int iObj, int nCands, int fVerbose, int * pnBufs, int * pnInvs )
{
    int i, iCand, RetValue = 0;
    Vec_Int_t * vTfo   = Gia_SimRsbTfo( p, iObj, -1 );
    word * pCareSet    = Gia_SimRsbCareSet( p, iObj, vTfo );
    word * pFunc       = Vec_WrdEntryP( p->vSimsObj, p->nWords*iObj );
    Vec_Int_t * vCands = Gia_ObjSimCands( p, iObj, nCands );
    Abc_TtAndSharp( p->pFunc[0], pCareSet, pFunc, p->nWords, 1 );
    Abc_TtAndSharp( p->pFunc[1], pCareSet, pFunc, p->nWords, 0 );

/*
printf( "Considering node %d with %d candidates:\n", iObj, Vec_IntSize(vCands) );
Vec_IntPrint( vTfo );
Vec_IntPrint( vCands );
Extra_PrintBinary( stdout, (unsigned *)pCareSet,    64 );  printf( "\n" );
Extra_PrintBinary( stdout, (unsigned *)pFunc,       64 );  printf( "\n" );
Extra_PrintBinary( stdout, (unsigned *)p->pFunc[0], 64 );  printf( "\n" );
Extra_PrintBinary( stdout, (unsigned *)p->pFunc[1], 64 );  printf( "\n" );
*/
    Vec_IntForEachEntry( vCands, iCand, i )
    {
        word * pDiv = Vec_WrdEntryP( p->vSimsObj, p->nWords*iCand );
        if ( !Abc_TtIntersect(pDiv, p->pFunc[0], p->nWords, 0) &&
             !Abc_TtIntersect(pDiv, p->pFunc[1], p->nWords, 1) )
            { (*pnBufs)++; if ( fVerbose ) printf( "Level %3d : %d = buf(%d)\n", Gia_ObjLevelId(p->pGia, iObj), iObj, iCand ); RetValue = 1; }
        if ( !Abc_TtIntersect(pDiv, p->pFunc[0], p->nWords, 1) &&
             !Abc_TtIntersect(pDiv, p->pFunc[1], p->nWords, 0) )
            { (*pnInvs)++; if ( fVerbose ) printf( "Level %3d : %d = inv(%d)\n", Gia_ObjLevelId(p->pGia, iObj), iObj, iCand ); RetValue = 1; }
    }
    return RetValue;
}

int Gia_ManSimRsb( Gia_Man_t * pGia, int nCands, int fVerbose )
{
    abctime clk = Abc_Clock();
    Gia_Obj_t * pObj; int iObj, nCount = 0, nBufs = 0, nInvs = 0;
    Gia_SimRsbMan_t * p = Gia_SimRsbAlloc( pGia );
    assert( pGia->vSimsPi != NULL );
    Gia_ManLevelNum( pGia );
    Gia_ManForEachAnd( pGia, pObj, iObj )
        //if ( iObj == 6 )
        nCount += Gia_ObjSimRsb( p, iObj, nCands, fVerbose, &nBufs, &nInvs );
    printf( "Can resubstitute %d nodes (%.2f %% out of %d) (Bufs = %d Invs = %d)  ", 
        nCount, 100.0*nCount/Gia_ManAndNum(pGia), Gia_ManAndNum(pGia), nBufs, nInvs );
    Abc_PrintTime( 1, "Time", Abc_Clock() - clk );
    Gia_SimRsbFree( p );
    return nCount;
}




/**Function*************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

***********************************************************************/
void Gia_ManSimRelAssignInputs( Gia_Man_t * p, int nWords, Vec_Wrd_t * vSims, int nWordsIn, Vec_Wrd_t * vSimsIn )
{
    int i, m, Id, nMints = nWords / nWordsIn;
    assert( Vec_WrdSize(vSims)   == nWords   * Gia_ManObjNum(p) );
    assert( Vec_WrdSize(vSimsIn) == nWordsIn * Gia_ManCiNum(p) );
    Gia_ManForEachCiId( p, Id, i )
        for ( m = 0; m < nMints; m++ )
            memcpy( Vec_WrdEntryP(vSims, Id * nWords + nWordsIn * m), 
                    Vec_WrdEntryP(vSimsIn, i * nWordsIn), sizeof(word) * nWordsIn );
}
int Gia_ManSimRelCompare( Gia_Man_t * p, int nWords, Vec_Wrd_t * vSims, int nWordsOut, Vec_Wrd_t * vSimsOut, int iPat, int iMint )
{
    int i, Id;
    Gia_ManForEachCoId( p, Id, i )
    {
        word * pSim    = Vec_WrdEntryP( vSims, nWords * Id + iMint * nWordsOut );
        word * pSimOut = Vec_WrdEntryP( vSimsOut, nWordsOut * i );
/*
        int k;
        for ( k = 0; k < 64*nWordsOut; k++ )
            printf( "%d", Abc_TtGetBit( pSim, k ) );
        printf( "\n" );
        for ( k = 0; k < 64*nWordsOut; k++ )
            printf( "%d", Abc_TtGetBit( pSimOut, k ) );
        printf( "\n\n" );
*/
        if ( Abc_TtGetBit(pSim, iPat) != Abc_TtGetBit(pSimOut, iPat) )
            return 0;
    }
    return 1;
}
int Gia_ManSimRelCollectOutputs( Gia_Man_t * p, int nWords, Vec_Wrd_t * vSims, int nWordsOut, Vec_Wrd_t * vSimsOut, Vec_Wrd_t * vRel )
{
    int i, m, nMints = nWords / nWordsOut, Count = 0;
    assert( Vec_WrdSize(vSims)    == nWords   * Gia_ManObjNum(p) );
    assert( Vec_WrdSize(vSimsOut) == nWordsOut * Gia_ManCoNum(p) );
    assert( Vec_WrdSize(vRel)     == nWordsOut * nMints );
    for ( i = 0; i < 64 * nWordsOut; i++ )
    {
        int CountMints = 0;
        for ( m = 0; m < nMints; m++ )
            if ( Gia_ManSimRelCompare(p, nWords, vSims, nWordsOut, vSimsOut, i, m) )
                Abc_TtSetBit( Vec_WrdArray(vRel), i*nMints+m ), CountMints++;
        Count += CountMints == 0;
    }
    if ( Count )
        printf( "The relation is not well-defined for %d (out of %d) patterns.\n", Count, 64 * nWordsOut );
    return Count;
}
Vec_Wrd_t * Gia_ManSimRel( Gia_Man_t * p, Vec_Int_t * vObjs, Vec_Wrd_t * vVals )
{
    int nWords = Vec_WrdSize(p->vSimsPi) / Gia_ManCiNum(p);
    int nMints = 1 << Vec_IntSize(vObjs), i, m, iObj;
    Gia_Obj_t * pObj;
    Vec_Wrd_t * vRel  = Vec_WrdStart( nWords * nMints );
    Vec_Wrd_t * vSims = Vec_WrdStart( Gia_ManObjNum(p) * nWords * nMints );
    Gia_ManSimRelAssignInputs( p, nWords * nMints, vSims, nWords, p->vSimsPi );
    Vec_IntForEachEntry( vObjs, iObj, i )
        for ( m = 0; m < nMints; m++ )
            if ( (m >> i) & 1 )
                memset( Vec_WrdEntryP(vSims, iObj*nMints*nWords + nWords*m), 0xFF, sizeof(word)*nWords );    
            else
                memset( Vec_WrdEntryP(vSims, iObj*nMints*nWords + nWords*m), 0x00, sizeof(word)*nWords );    
    Gia_ManCleanPhase( p );
    Gia_ManForEachObjVec( vObjs, p, pObj, i )
        pObj->fPhase = 1;
    Gia_ManForEachAnd( p, pObj, i ) 
        if ( !pObj->fPhase )
            Gia_ManSimPatSimAnd( p, i, pObj, nWords * nMints, vSims );
    Gia_ManForEachCo( p, pObj, i )
        if ( !pObj->fPhase )
            Gia_ManSimPatSimPo( p, Gia_ObjId(p, pObj), pObj, nWords * nMints, vSims );
    Gia_ManForEachObjVec( vObjs, p, pObj, i )
        pObj->fPhase = 0;
    if ( Gia_ManSimRelCollectOutputs( p, nWords * nMints, vSims, nWords, vVals, vRel ) )
        Vec_WrdFreeP( &vRel );
    Vec_WrdFree( vSims );
    return vRel;
}

/**Function*************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

***********************************************************************/
void Gia_ManSimRelCheckFuncs( Gia_Man_t * p, Vec_Wrd_t * vRel, int nOuts, Vec_Wrd_t * vFuncs )
{
    int i, k, m, Values[32], nErrors = 0, nMints = 1 << nOuts, nWords = Vec_WrdSize(vRel) / nMints;
    assert( Vec_WrdSize(vFuncs) == 2 * nOuts * nWords );
    assert( nOuts <= 32 );
    for ( i = 0; i < 64 * nWords; i++ )
    {
        for ( k = 0; k < nOuts; k++ )
        {
            int Value0 = Abc_TtGetBit( Vec_WrdEntryP(vFuncs, (2*k+0)*nWords), i );
            int Value1 = Abc_TtGetBit( Vec_WrdEntryP(vFuncs, (2*k+1)*nWords), i );
            if ( Value0 && !Value1 )
                Values[k] = 1;
            else if ( !Value0 && Value1 )
                Values[k] = 2;
            else if ( !Value0 && !Value1 )
                Values[k] = 3;
            else assert( 0 );
        }
        for ( m = 0; m < nMints; m++ )
        {
            for ( k = 0; k < nOuts; k++ )
                if ( ((Values[k] >> ((m >> k) & 1)) & 1) == 0 )
                    break;
            if ( k < nOuts )
                continue;
            if ( Abc_TtGetBit( Vec_WrdArray(vRel), i*nMints+m ) )
                continue;
            if ( nErrors++ == 0 )
                printf( "For pattern %d, minterm %d produced by function is not in the relation.\n", i, m );
        }
    }
    if ( nErrors )    
        printf( "Total number of similar errors = %d.\n", nErrors );
    else
        printf( "The function agrees with the relation.\n" );
}
Vec_Wrd_t * Gia_ManSimRelDeriveFuncs( Gia_Man_t * p, Vec_Wrd_t * vRel, int nOuts )
{
    int i, k, m, Count = 0, nMints = 1 << nOuts, nWords = Vec_WrdSize(vRel) / nMints;
    Vec_Wrd_t * vFuncs = Vec_WrdStart( 2 * nOuts * nWords );
    assert( Vec_WrdSize(vRel) % nMints == 0 );
    for ( i = 0; i < 64 * nWords; i++ )
    {
        for ( m = 0; m < nMints; m++ )
            if ( Abc_TtGetBit( Vec_WrdArray(vRel), i*nMints+m ) )
                break;
        Count += m == nMints;
        for ( k = 0; k < nOuts; k++ )
            if ( (m >> k) & 1 )
                Abc_TtSetBit( Vec_WrdEntryP(vFuncs, (2*k+1)*nWords), i );
            else
                Abc_TtSetBit( Vec_WrdEntryP(vFuncs, (2*k+0)*nWords), i );
    }
    if ( Count )
        printf( "The relation is not well-defined for %d (out of %d) patterns.\n", Count, 64 * nWords );
    else
        printf( "The relation was successfully determized without don't-cares for %d patterns.\n", 64 * nWords );
    Gia_ManSimRelCheckFuncs( p, vRel, nOuts, vFuncs );
    return vFuncs;
}
Vec_Wrd_t * Gia_ManSimRelDeriveFuncs2( Gia_Man_t * p, Vec_Wrd_t * vRel, int nOuts )
{
    int i, k, m, nDCs[32] = {0}, Count = 0, nMints = 1 << nOuts, nWords = Vec_WrdSize(vRel) / nMints;
    Vec_Wrd_t * vFuncs = Vec_WrdStart( 2 * nOuts * nWords );
    assert( Vec_WrdSize(vRel) % nMints == 0 );
    assert( nOuts <= 32 );
    for ( i = 0; i < 64 * nWords; i++ )
    {
        for ( m = 0; m < nMints; m++ )
            if ( Abc_TtGetBit( Vec_WrdArray(vRel), i*nMints+m ) )
                break;
        Count += m == nMints;
        for ( k = 0; k < nOuts; k++ )
        {
            if ( Abc_TtGetBit( Vec_WrdArray(vRel), i*nMints+(m^(1<<k)) ) )
            {
                nDCs[k]++;
                continue;
            }
            if ( (m >> k) & 1 )
                Abc_TtSetBit( Vec_WrdEntryP(vFuncs, (2*k+1)*nWords), i );
            else
                Abc_TtSetBit( Vec_WrdEntryP(vFuncs, (2*k+0)*nWords), i );
        }
        if ( 0 )
        {
            for ( m = 0; m < nMints; m++ )
                printf( "%d", Abc_TtGetBit( Vec_WrdArray(vRel), i*nMints+m ) );
            printf( " " );
            for ( k = 0; k < nOuts; k++ )
            {
                if ( Abc_TtGetBit( Vec_WrdEntryP(vFuncs, (2*k+0)*nWords), i ) )
                    printf( "0" );
                else if ( Abc_TtGetBit( Vec_WrdEntryP(vFuncs, (2*k+1)*nWords), i ) )
                    printf( "1" );
                else
                    printf( "-" );
            }
            printf( "\n" );
        }
    }
    if ( Count )
        printf( "The relation is not well-defined for %d (out of %d) patterns.\n", Count, 64 * nWords );
    else
    {
        printf( "The relation was successfully determized with don't-cares for %d patterns.\n", 64 * nWords );
        for ( k = 0; k < nOuts; k++ )
        {
            int nOffs = Abc_TtCountOnesVec( Vec_WrdEntryP(vFuncs, (2*k+0)*nWords), nWords );
            int nOns  = Abc_TtCountOnesVec( Vec_WrdEntryP(vFuncs, (2*k+1)*nWords), nWords );
            printf( "%4d : Off = %6d  On = %6d  Dc = %6d (%6.2f %%)\n", k, nOffs, nOns, nDCs[k], 100.0*nDCs[k]/(64*nWords) );
        }
        printf( "\n" );
    }
    Gia_ManSimRelCheckFuncs( p, vRel, nOuts, vFuncs );
    return vFuncs;
}

/**Function*************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

***********************************************************************/
void Gia_ManSimRelPrint( Gia_Man_t * p, Vec_Wrd_t * vRel, Vec_Int_t * vOutMints )
{
    int nWords = Vec_WrdSize(p->vSimsPi) / Gia_ManCiNum(p);
    int nMints = Vec_WrdSize(vRel) / nWords; 
    int i, m, Count;
/*
    for ( i = 0; i < 64 * nWords; i++ )
    {
        int k;
        for ( k = 0; k < Gia_ManCiNum(p); k++ )
            printf( "%d", Abc_TtGetBit( Vec_WrdEntryP(p->vSimsPi, k), i ) );
        printf( " " );
        Count = 0;
        for ( m = 0; m < nMints; m++ )
        {
            printf( "%d", Abc_TtGetBit( Vec_WrdArray(vRel), i*nMints+m ) );
            Count += Abc_TtGetBit( Vec_WrdArray(vRel), i*nMints+m );
        }
        printf( "  Count = %2d ", Count );
        if ( vOutMints )
        {
            printf( "   %3d ", Vec_IntEntry(vOutMints, i) );
            if ( Abc_TtGetBit( Vec_WrdArray(vRel), i*nMints+Vec_IntEntry(vOutMints, i) ) )
                printf( "yes" );
            else
                printf( "no" );
        }
        printf( "\n" );
    }
*/
/*
    for ( i = 0; i < 64 * nWords; i++ )
    {
        Count = 0;
        for ( m = 0; m < nMints; m++ )
            Count += Abc_TtGetBit( Vec_WrdArray(vRel), i*nMints+m );
        printf( "%d ", Count );
    }
    printf( "\n" );
*/
    for ( i = 0; i < 64 * nWords; i++ )
    {
        Count = 0;
        for ( m = 0; m < nMints; m++ )
        {
            printf( "%d", Abc_TtGetBit( Vec_WrdArray(vRel), i*nMints+m ) );
            Count += Abc_TtGetBit( Vec_WrdArray(vRel), i*nMints+m );
        }
        printf( "  Count = %2d \n", Count );
    }
}
Vec_Int_t * Gia_ManSimPatStart( int nItems )
{
    Vec_Int_t * vValues = Vec_IntAlloc( nItems );
    Vec_IntPush( vValues, 17 );
    Vec_IntPush( vValues, 39 );
    Vec_IntPush( vValues, 56 );
    Vec_IntPush( vValues, 221 );
    return vValues;
}
void Gia_ManSimRelTest( Gia_Man_t * p )
{
    //int nWords        = Vec_WrdSize(p->vSimsPi) / Gia_ManCiNum(p);
    Vec_Int_t * vObjs = Gia_ManSimPatStart( 4 ); // can be CI/AND/CO
    Vec_Wrd_t * vVals = Gia_ManSimPatValues( p );
    Vec_Wrd_t * vRel  = Gia_ManSimRel( p, vObjs, vVals );
    assert( p->vSimsPi != NULL );
    Gia_ManSimRelPrint( p, vRel, NULL );
    Vec_IntFree( vObjs );
    Vec_WrdFree( vVals );
    Vec_WrdFree( vRel );
}



/**Function*************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

***********************************************************************/
Vec_Int_t * Gia_Sim5CollectValues( word * pOffSet, word * pOnSet, int nWords )
{
    Vec_Int_t * vBits = Vec_IntAlloc( 64*nWords ); int i, Count[2] = {0};
    for ( i = 0; i < 64*nWords; i++ )
        if ( Abc_TtGetBit( pOffSet, i ) )
            Vec_IntPush( vBits, 0 ), Count[0]++;
        else if ( Abc_TtGetBit( pOnSet, i ) )
            Vec_IntPush( vBits, 1 ), Count[1]++;
        else
            Vec_IntPush( vBits, -1 );
    //printf( "Offset = %d.  Onset = %d.  Dcset = %d.\n", Count[0], Count[1], 64*nWords - Count[0] - Count[1] );
    return vBits;
}
Gia_SimAbsMan_t * Gia_SimAbsAlloc( Gia_Man_t * pGia, word * pOffSet, word * pOnSet, Vec_Wrd_t * vSims, int nWords, Vec_Int_t * vResub, int fVerbose )
{
    Gia_SimAbsMan_t * p = ABC_CALLOC( Gia_SimAbsMan_t, 1 );
    p->pGia      = pGia;
    p->pSet[0]   = pOffSet;
    p->pSet[1]   = pOnSet;
    p->nCands    = Vec_WrdSize(vSims)/nWords;
    p->nWords    = nWords;
    p->vSims     = vSims;
    p->vResub    = vResub;
    p->fVerbose  = fVerbose;
    p->vValues      = Gia_Sim5CollectValues( pOffSet, pOnSet, nWords );
    p->vPatPairs    = Vec_IntAlloc( 100 );
    p->vCoverTable  = Vec_WrdAlloc( 10000 );
    p->vTtMints     = Vec_IntAlloc( 100 );
    assert( Vec_WrdSize(vSims) % nWords == 0 );
    return p;
}
void Gia_SimAbsFree( Gia_SimAbsMan_t * p )
{
    Vec_IntFree( p->vValues );
    Vec_IntFree( p->vPatPairs );
    Vec_WrdFree( p->vCoverTable );
    Vec_IntFree( p->vTtMints );
    ABC_FREE( p );
}


/**Function*************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

***********************************************************************/
void Gia_SimAbsCheckSolution( Gia_SimAbsMan_t * p )
{
    int x, y, z, w, fFound = 0;
    assert( Vec_WrdSize(p->vCoverTable) == p->nWordsTable * (p->nCands+1) );

    Abc_TtClear( p->pTableTemp, p->nWordsTable );
    for ( x = 0; x < Vec_IntSize(p->vPatPairs)/2; x++ )
        Abc_TtXorBit( p->pTableTemp, x );

    for ( x =   0; x < p->nCands; x++ )
    {
        word * pSimTableX = Vec_WrdEntryP( p->vCoverTable, p->nWordsTable * x );
        for ( w = 0; w < p->nWordsTable; w++ )
            if ( p->pTableTemp[w] != pSimTableX[w] )
                break;
        if ( w == p->nWordsTable )
        {
            printf( "Found solution { %d }\n", x );
            fFound = 1;
        }
    }
    if ( fFound )
        return;

    for ( x =   0; x < p->nCands; x++ )
    for ( y =   0; y < x;         y++ )
    {
        word * pSimTableX = Vec_WrdEntryP( p->vCoverTable, p->nWordsTable * x );
        word * pSimTableY = Vec_WrdEntryP( p->vCoverTable, p->nWordsTable * y );
        for ( w = 0; w < p->nWordsTable; w++ )
            if ( p->pTableTemp[w] != (pSimTableX[w] | pSimTableY[w]) )
                break;
        if ( w == p->nWordsTable )
        {
            printf( "Found solution { %d %d }\n", y, x );
            fFound = 1;
        }
    }
    if ( fFound )
        return;

    for ( x =   0; x < p->nCands; x++ )
    for ( y =   0; y < x;         y++ )
    for ( z =   0; z < y;         z++ )
    {
        word * pSimTableX = Vec_WrdEntryP( p->vCoverTable, p->nWordsTable * x );
        word * pSimTableY = Vec_WrdEntryP( p->vCoverTable, p->nWordsTable * y );
        word * pSimTableZ = Vec_WrdEntryP( p->vCoverTable, p->nWordsTable * z );
        for ( w = 0; w < p->nWordsTable; w++ )
            if ( p->pTableTemp[w] != (pSimTableX[w] | pSimTableY[w] | pSimTableZ[w]) )
                break;
        if ( w == p->nWordsTable )
            printf( "Found solution { %d %d %d }\n", z, y, x );
    }
}

void Gia_SimAbsSolve( Gia_SimAbsMan_t * p )
{
    abctime clk = Abc_Clock();
    int i, k, iPat, iPat2;
/*
    Vec_Int_t * vSimPats = Vec_IntDup( p->vPatPairs );
    Vec_IntUniqify( vSimPats );
    printf( "Selected %d pattern pairs contain %d unique patterns.\n", Vec_IntSize(p->vPatPairs)/2, Vec_IntSize(vSimPats) );
    Vec_IntFree( vSimPats );
*/
    // set up the covering problem
    p->nWordsTable = Abc_Bit6WordNum( Vec_IntSize(p->vPatPairs)/2 );
    Vec_WrdFill( p->vCoverTable, p->nWordsTable * (p->nCands + 1), 0 );
    p->pTableTemp = Vec_WrdEntryP( p->vCoverTable, p->nWordsTable * p->nCands );
    for ( i = 0; i < p->nCands; i++ )
    {
        word * pSimCand  = Vec_WrdEntryP( p->vSims, p->nWords * i );
        word * pSimTable = Vec_WrdEntryP( p->vCoverTable, p->nWordsTable * i );
        //printf( "%4d : ", i );
        //Extra_PrintBinary( stdout, (word *)pSimCand, p->nCands );  printf( "\n" );
        Vec_IntForEachEntryDouble( p->vPatPairs, iPat, iPat2, k )
        {
            assert( Vec_IntEntry(p->vValues, iPat)  == 0 );
            assert( Vec_IntEntry(p->vValues, iPat2) == 1 );
            if ( Abc_TtGetBit(pSimCand, iPat) != Abc_TtGetBit(pSimCand, iPat2) )
                Abc_TtXorBit(pSimTable, k/2);
        }
        assert( k == Vec_IntSize(p->vPatPairs) );
    }

    if ( 0 )
    {
        printf( "                  " );
        for ( i = 0; i < p->nCands; i++ )
            printf( "%d", i % 10 );
        printf( "\n" );

        Vec_IntForEachEntryDouble( p->vPatPairs, iPat, iPat2, i )
        {
            printf( "%4d  ", i/2 );
            printf( "%4d  ", iPat );
            printf( "%4d  ", iPat2 );
            for ( k = 0; k < p->nCands; k++ )
            {
                word * pSimTable = Vec_WrdEntryP( p->vCoverTable, p->nWordsTable * k );
                printf( "%c", Abc_TtGetBit(pSimTable, i/2) ? '*' : ' ' );
            }
            printf( "\n" );
        }
    }

    //Gia_SimAbsCheckSolution(p);

    Vec_IntClear( p->vResub );
    Abc_TtClear( p->pTableTemp, p->nWordsTable );
    for ( i = 0; i < Vec_IntSize(p->vPatPairs)/2; i++ )
        Abc_TtXorBit( p->pTableTemp, i );

    while ( !Abc_TtIsConst0(p->pTableTemp, p->nWordsTable) )
    {
        word * pSimTable;
        int iArgMax = -1, CostThis, CostMax = -1;
        // compute the cost of each column
        for ( i = 0; i < p->nCands; i++ )
        {
            pSimTable = Vec_WrdEntryP( p->vCoverTable, p->nWordsTable * i );
            CostThis = Abc_TtCountOnesVecMask( pSimTable, p->pTableTemp, p->nWordsTable, 0 );
            if ( CostMax >= CostThis )
                continue;
            CostMax = CostThis;
            iArgMax = i;
        }
        // find the best column
        Vec_IntPush( p->vResub, iArgMax );
        // delete values of this column
        pSimTable = Vec_WrdEntryP( p->vCoverTable, p->nWordsTable * iArgMax );
        Abc_TtSharp( p->pTableTemp, p->pTableTemp, pSimTable, p->nWordsTable );
    }
    if ( p->fVerbose )
    {
        printf( "Solution %2d for covering problem [%5d x %5d]: ", Vec_IntSize(p->vResub), Vec_IntSize(p->vPatPairs)/2, p->nCands );
        Vec_IntForEachEntry( p->vResub, iPat, i )
            printf( "%6d ", iPat );
        for ( ; i < 12; i++ )
            printf( "       " );
        printf( "   " );
        Abc_PrintTime( 1, "Time", Abc_Clock() - clk );
    }
}
int Gia_SimAbsRefine( Gia_SimAbsMan_t * p )
{
    int i, b, Value, iPat, iMint, iObj, Count = 0;
    word ** pFanins = ABC_ALLOC( word *, Vec_IntSize(p->vResub) );
    assert( Vec_IntSize(p->vResub) > 0 );
    Vec_IntForEachEntry( p->vResub, iObj, b )
        pFanins[b] = Vec_WrdEntryP( p->vSims, p->nWords * iObj );
    Vec_IntFill( p->vTtMints, 1 << Vec_IntSize(p->vResub), -1 );
    Vec_IntForEachEntry( p->vValues, Value, i )
    {
        if ( Value == -1 )
            continue;
        iMint = 0;
        for ( b = 0; b < Vec_IntSize(p->vResub); b++ )
            if ( Abc_TtGetBit(pFanins[b], i) )
                iMint |= 1 << b;
        iPat = Vec_IntEntry( p->vTtMints, iMint );
        if ( iPat == -1 )
        {
            Vec_IntWriteEntry( p->vTtMints, iMint, i );
            continue;
        }
        assert( Abc_TtGetBit(p->pSet[Value], i) );
        if ( Abc_TtGetBit(p->pSet[Value], iPat) )
            continue;
        assert( Abc_TtGetBit(p->pSet[!Value], iPat) );
        Vec_IntPushTwo( p->vPatPairs, Value ? iPat : i, Value ? i : iPat );
        //printf( "iPat1 = %d  iPat2 = %d   Mint = %d\n", Value ? iPat : i, Value ? i : iPat, iMint );
        Count++;
        if ( Count == 64 )
        {
            ABC_FREE( pFanins );
            return 1;
        }
    }
    //printf( "Refinement added %d minterm pairs.\n", Count );
    ABC_FREE( pFanins );
    return Count != 0;
}
Vec_Int_t * Gia_SimAbsFind( Vec_Int_t * vValues, int Value )
{
    Vec_Int_t * vSubset = Vec_IntAlloc( 100 ); int i, Entry;
    Vec_IntForEachEntry( vValues, Entry, i )
        if ( Entry == Value )
            Vec_IntPush( vSubset, i );
    return vSubset;
}
void Gia_SimAbsInit( Gia_SimAbsMan_t * p )
{
    int n, nPairsInit = 64;
    Vec_Int_t * vValue0 = Gia_SimAbsFind( p->vValues, 0 );
    Vec_Int_t * vValue1 = Gia_SimAbsFind( p->vValues, 1 );
    Vec_IntClear( p->vPatPairs );
    printf( "There are %d offset and %d onset minterms (%d pairs) and %d divisors.\n", 
        Vec_IntSize(vValue0), Vec_IntSize(vValue1), Vec_IntSize(vValue0)*Vec_IntSize(vValue1), p->nCands );
    Abc_Random( 1 );
    assert( Vec_IntSize(vValue0) > 0 );
    assert( Vec_IntSize(vValue1) > 0 );
    for ( n = 0; n < nPairsInit; n++ )
        Vec_IntPushTwo( p->vPatPairs, 
            Vec_IntEntry(vValue0, Abc_Random(0) % Vec_IntSize(vValue0)), 
            Vec_IntEntry(vValue1, Abc_Random(0) % Vec_IntSize(vValue1)) );
    Vec_IntFree( vValue0 );
    Vec_IntFree( vValue1 );
}

/**Function*************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

***********************************************************************/
Vec_Int_t * Gia_SimAbsPerformOne( Gia_Man_t * pGia, word * pOffSet, word * pOnSet, Vec_Wrd_t * vSimsCands, int nWords, int fVerbose )
{
    abctime clk = Abc_Clock();
    Vec_Int_t * vResub  = Vec_IntAlloc( 10 );
    Gia_SimAbsMan_t * p = Gia_SimAbsAlloc( pGia, pOffSet, pOnSet, vSimsCands, nWords, vResub, fVerbose );
    Gia_SimAbsInit( p );
    while ( 1 )
    {
        Gia_SimAbsSolve( p );
        if ( !Gia_SimAbsRefine( p ) )
            break;
    }
    Gia_SimAbsFree( p );
    Abc_PrintTime( 1, "Resubstitution time", Abc_Clock() - clk );
    return vResub;
}



/**Function*************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

***********************************************************************/

typedef struct Gia_RsbMan_t_ Gia_RsbMan_t;
struct Gia_RsbMan_t_
{
    Gia_Man_t *    pGia;
    word *         pOffSet;
    word *         pOnSet;
    int            nWords;
    int            nWordsT;
    Vec_Wrd_t *    vSims;
    Vec_Wrd_t *    vSimsT;
    Vec_Int_t *    vCands;
    Vec_Int_t *    vObjs;
    Vec_Int_t *    vObjs2;
    Vec_Wec_t *    vSets[2];
    word *         pSet[3];
    Vec_Int_t *    vActive;
};
Gia_RsbMan_t * Gia_RsbAlloc( Gia_Man_t * pGia, word * pOffSet, word * pOnSet, Vec_Wrd_t * vSims, int nWords, Vec_Wrd_t * vSimsT, int nWordsT, Vec_Int_t * vCands )
{
    int i, iObj;
    Gia_RsbMan_t * p = ABC_CALLOC( Gia_RsbMan_t, 1 );
    assert( nWords <= 1024 );
    assert( Vec_WrdSize(vSims) == 64 * nWords * nWordsT );
    assert( Vec_WrdSize(vSims) == Vec_WrdSize(vSimsT) );
    p->pGia      = pGia;
    p->pOffSet   = pOffSet;
    p->pOnSet    = pOnSet;
    p->nWords    = nWords;
    p->nWordsT   = nWordsT;
    p->vSims     = vSims;
    p->vSimsT    = vSimsT;
    p->vCands    = vCands;
    p->vObjs     = Vec_IntAlloc( 100 );
    p->vObjs2    = Vec_IntAlloc( 100 );
    p->vSets[0]  = Vec_WecAlloc( 1024 );
    p->vSets[1]  = Vec_WecAlloc( 1024 );
    p->pSet[0]   = ABC_CALLOC( word, nWordsT );
    p->pSet[1]   = ABC_CALLOC( word, nWordsT );
    p->pSet[2]   = ABC_CALLOC( word, nWordsT );
    p->vActive   = Vec_IntAlloc( 100 );
    Vec_IntForEachEntry( vCands, iObj, i )
    {
        assert( iObj < nWordsT * 64 );
        Abc_TtSetBit( p->pSet[0], iObj );
    }
    Vec_WecPushLevel( p->vSets[0] );
    Vec_WecPushLevel( p->vSets[1] );
    for ( i = 0; i < 64*nWords; i++ )
    {
        int Value0 = Abc_TtGetBit( pOffSet, i );
        int Value1 = Abc_TtGetBit( pOnSet,  i );
        if ( Value0 && !Value1 )
            Vec_WecPush( p->vSets[0], 0, i );
        else if ( !Value0 && Value1 )
            Vec_WecPush( p->vSets[1], 0, i );
        else assert( !Value0 || !Value1 );
    }
    assert( Vec_WecSize(p->vSets[0]) == 1 );
    assert( Vec_WecSize(p->vSets[1]) == 1 );
    Abc_Random( 1 );
    //Extra_PrintBinary2( stdout, (unsigned*)pOffSet, 64*nWords ); printf( "\n" );
    //Extra_PrintBinary2( stdout, (unsigned*)pOnSet,  64*nWords ); printf( "\n" );
    return p;
}
void Gia_RsbFree( Gia_RsbMan_t * p )
{
    Vec_IntFree( p->vActive );
    Vec_IntFree( p->vObjs );
    Vec_IntFree( p->vObjs2 );
    Vec_WecFree( p->vSets[0] );
    Vec_WecFree( p->vSets[1] );
    ABC_FREE( p->pSet[0] );
    ABC_FREE( p->pSet[1] );
    ABC_FREE( p->pSet[2] );
    ABC_FREE( p );
}


int Gia_RsbCost( Gia_RsbMan_t * p )
{
    Vec_Int_t * vLevel[2]; int i, Cost = 0;
    Vec_WecForEachLevelTwo( p->vSets[0], p->vSets[1], vLevel[0], vLevel[1], i )
        Cost += Vec_IntSize(vLevel[0]) * Vec_IntSize(vLevel[1]);
    return Cost;
}
void Gia_RsbPrint( Gia_RsbMan_t * p )
{
    Vec_Int_t * vLevel[2]; 
    int n, i, nLeaves = 1 << Vec_IntSize(p->vObjs);
    assert( Vec_WecSize(p->vSets[0]) == nLeaves );
    assert( Vec_WecSize(p->vSets[1]) == nLeaves );
    printf( "Database for %d objects and cost %d:\n", Vec_IntSize(p->vObjs), Gia_RsbCost(p) );
    Vec_WecForEachLevelTwo( p->vSets[0], p->vSets[1], vLevel[0], vLevel[1], i )
    {
        for ( n = 0; n < 2; n++ )
        {
            printf( "%5d : ", i );
            Extra_PrintBinary2( stdout, (unsigned*)&i, Vec_IntSize(p->vObjs) ); printf( " %d ", n );
            Vec_IntPrint( vLevel[n] );
        }
    }    
}
void Gia_RsbUpdateAdd( Gia_RsbMan_t * p, int iObj )
{
    int n, i, nLeaves = 1 << Vec_IntSize(p->vObjs);
    assert( Vec_WecSize(p->vSets[0]) == nLeaves );
    assert( Vec_WecSize(p->vSets[1]) == nLeaves );
    for ( i = 0; i < nLeaves; i++ )
    {
        for ( n = 0; n < 2; n++ )
        {
            Vec_Int_t * vLevelN = Vec_WecPushLevel(p->vSets[n]);  
            Vec_Int_t * vLevel  = Vec_WecEntry(p->vSets[n], i);
            int iMint, j, k = 0;
            Vec_IntForEachEntry( vLevel, iMint, j )
            {
                if ( Abc_TtGetBit(Vec_WrdEntryP(p->vSims, p->nWords*iObj), iMint) )
                    Vec_IntPush( vLevelN, iMint );
                else
                    Vec_IntWriteEntry( vLevel, k++, iMint );
            }
            Vec_IntShrink( vLevel, k );
        }
    }
    Vec_IntPush( p->vObjs, iObj );
    assert( Vec_WecSize(p->vSets[0]) == 2*nLeaves );
    assert( Vec_WecSize(p->vSets[1]) == 2*nLeaves );
}
void Gia_RsbUpdateRemove( Gia_RsbMan_t * p, int Index )
{
    Vec_Int_t * vLevel[2], * vTemp[2][2];
    int k = 0, m, m2, nLeaves = 1 << Vec_IntSize(p->vObjs);
    assert( Index < Vec_IntSize(p->vObjs) );
    assert( Vec_WecSize(p->vSets[0]) == nLeaves );
    assert( Vec_WecSize(p->vSets[1]) == nLeaves );
    for ( m = 0; m < nLeaves; m++ )
    {
        if ( m & (1 << Index) )
            continue;
        m2 = m ^ (1 << Index);
        vTemp[0][0] = Vec_WecEntry(p->vSets[0], m);
        vTemp[0][1] = Vec_WecEntry(p->vSets[1], m);
        vTemp[1][0] = Vec_WecEntry(p->vSets[0], m2);
        vTemp[1][1] = Vec_WecEntry(p->vSets[1], m2);
        Vec_IntAppend( vTemp[0][0], vTemp[1][0] );
        Vec_IntAppend( vTemp[0][1], vTemp[1][1] );
        Vec_IntClear( vTemp[1][0] );
        Vec_IntClear( vTemp[1][1] );
    }
    Vec_IntDrop( p->vObjs, Index );
    Vec_WecForEachLevelTwo( p->vSets[0], p->vSets[1], vLevel[0], vLevel[1], m )
    {
        if ( m & (1 << Index) )
            continue;
        ABC_SWAP( Vec_Int_t, Vec_WecArray(p->vSets[0])[k], Vec_WecArray(p->vSets[0])[m] );
        ABC_SWAP( Vec_Int_t, Vec_WecArray(p->vSets[1])[k], Vec_WecArray(p->vSets[1])[m] );
        k++;
    }
    assert( k == nLeaves/2 );
    Vec_WecShrink( p->vSets[0], k );
    Vec_WecShrink( p->vSets[1], k );
}
int Gia_RsbRemovalCost( Gia_RsbMan_t * p, int Index )
{
    Vec_Int_t * vTemp[2][2];
    //unsigned Mask = Abc_InfoMask( Index );
    int m, m2, Cost = 0, nLeaves = 1 << Vec_IntSize(p->vObjs);
    assert( Vec_WecSize(p->vSets[0]) == (1 << Vec_IntSize(p->vObjs)) );
    assert( Vec_WecSize(p->vSets[1]) == (1 << Vec_IntSize(p->vObjs)) );
    for ( m = 0; m < nLeaves; m++ )
    {
        if ( m & (1 << Index) )
            continue;
        m2 = m ^ (1 << Index);
        vTemp[0][0] = Vec_WecEntry(p->vSets[0], m);
        vTemp[0][1] = Vec_WecEntry(p->vSets[1], m);
        vTemp[1][0] = Vec_WecEntry(p->vSets[0], m2);
        vTemp[1][1] = Vec_WecEntry(p->vSets[1], m2);
        Cost += (Vec_IntSize(vTemp[0][0]) + Vec_IntSize(vTemp[1][0])) * (Vec_IntSize(vTemp[0][1]) + Vec_IntSize(vTemp[1][1]));
    }
    return Cost;
}
int Gia_RsbFindNodeToRemove( Gia_RsbMan_t * p, int * pMinCost )
{
    int i, iObj, iMin = -1, CostMin = ABC_INFINITY;
    Vec_IntForEachEntry( p->vObjs, iObj, i )
    {
        int Cost = Gia_RsbRemovalCost( p, i );
        if ( CostMin > Cost )
        {
            CostMin = Cost;
            iMin = i;
        }
    }
    if ( pMinCost )
        *pMinCost = CostMin;
    return iMin;
}

void Gia_RsbFindMints( Gia_RsbMan_t * p, int * pMint0, int * pMint1 )
{
    int iSetI = Abc_Random(0) % Vec_IntSize(p->vActive);
    int iSet  = Vec_IntEntry( p->vActive, iSetI );
    Vec_Int_t * vArray0 = Vec_WecEntry(p->vSets[0], iSet);
    Vec_Int_t * vArray1 = Vec_WecEntry(p->vSets[1], iSet);
    int iMint0i = Abc_Random(0) % Vec_IntSize(vArray0);
    int iMint1i = Abc_Random(0) % Vec_IntSize(vArray1);
    int iMint0 = Vec_IntEntry( vArray0, iMint0i );
    int iMint1 = Vec_IntEntry( vArray1, iMint1i );
    *pMint0 = iMint0;
    *pMint1 = iMint1;
}
int Gia_RsbFindNode( Gia_RsbMan_t * p )
{
    int i, iObj, nNodes, nNodesNew = -1, nNodesOld = -1, Mint0, Mint1, Shift;
    Abc_TtCopy( p->pSet[1], p->pSet[0], p->nWordsT, 0 );
    Vec_IntForEachEntry( p->vObjs, iObj, i )
    {
        assert( Abc_TtGetBit(p->pSet[1], iObj) );
        Abc_TtXorBit(p->pSet[1], iObj);
    }
    Abc_TtCopy( p->pSet[2], p->pSet[1], p->nWordsT, 0 );
    Gia_RsbFindMints( p, &Mint0, &Mint1 );
    nNodes = Abc_TtAndXorSum( p->pSet[1], Vec_WrdEntryP(p->vSimsT, p->nWordsT*Mint0), Vec_WrdEntryP(p->vSimsT, p->nWordsT*Mint1), p->nWordsT );
    for ( i = 0; i < 5 && nNodes > 1; i++ )
    {
        nNodesOld = nNodes;
        Abc_TtCopy( p->pSet[2], p->pSet[1], p->nWordsT, 0 );
        Gia_RsbFindMints( p, &Mint0, &Mint1 );
        nNodesNew = Abc_TtAndXorSum( p->pSet[1], Vec_WrdEntryP(p->vSimsT, p->nWordsT*Mint0), Vec_WrdEntryP(p->vSimsT, p->nWordsT*Mint1), p->nWordsT );
        assert( nNodesNew <= nNodes );
        if ( nNodesNew < nNodes )
            i = 0;
        nNodes = nNodesNew;
    }
    Shift = Abc_Random(0) % (64*p->nWordsT);
    for ( i = 0; i < 64*p->nWordsT; i++ )
    {
        int Index = (i+Shift) % (64*p->nWordsT);
        if ( Abc_TtGetBit( p->pSet[2], Index ) )
            return Index;
    }
    assert( 0 );      
    return -1;
}
int Gia_RsbCollectValid( Gia_RsbMan_t * p )
{
    Vec_Int_t * vLevel[2]; int i;
    Vec_IntClear( p->vActive );
    assert( Vec_WecSize(p->vSets[0]) == Vec_WecSize(p->vSets[1]) );
    Vec_WecForEachLevelTwo( p->vSets[0], p->vSets[1], vLevel[0], vLevel[1], i )
        if ( Vec_IntSize(vLevel[0]) && Vec_IntSize(vLevel[1]) )
            Vec_IntPush( p->vActive, i );
    if ( Vec_IntSize(p->vActive) == 0 )
        return 0;
    return 1;
}
Vec_Int_t * Gia_RsbSolve( Gia_RsbMan_t * p )
{
    int i, iMin;
    Vec_IntClear( p->vObjs );
    while ( Gia_RsbCollectValid(p) )
        Gia_RsbUpdateAdd( p, Gia_RsbFindNode(p) );
    for ( i = 0; i < 100; i++ )
    {
        int k, nUndo = 1 + Abc_Random(0) % Vec_IntSize(p->vObjs);
        for ( k = 0; k < nUndo; k++ )
        {
            iMin = Gia_RsbFindNodeToRemove( p, NULL );// &MinCost );
            Gia_RsbUpdateRemove( p, iMin );
        }
        while ( Gia_RsbCollectValid(p) )
            Gia_RsbUpdateAdd( p, Gia_RsbFindNode(p) );
        if ( Vec_IntSize(p->vObjs2) == 0 || Vec_IntSize(p->vObjs2) > Vec_IntSize(p->vObjs) )
        {
            Vec_IntClear( p->vObjs2 );
            Vec_IntAppend( p->vObjs2, p->vObjs );
        }
    }
    //Gia_RsbPrint( p );
    return Vec_IntDup( p->vObjs2 );
}
Vec_Int_t * Gia_RsbSetFind( word * pOffSet, word * pOnSet, Vec_Wrd_t * vSims, int nWords, Vec_Wrd_t * vSimsT, int nWordsT, Vec_Int_t * vCands )
{
    Gia_RsbMan_t * p  = Gia_RsbAlloc( NULL, pOffSet, pOnSet, vSims, nWords, vSimsT, nWordsT, vCands );
    Vec_Int_t * vObjs = Gia_RsbSolve( p );
    Gia_RsbFree( p );
    Vec_IntSort( vObjs, 0 );
    return vObjs;
}

////////////////////////////////////////////////////////////////////////
///                       END OF FILE                                ///
////////////////////////////////////////////////////////////////////////


ABC_NAMESPACE_IMPL_END

