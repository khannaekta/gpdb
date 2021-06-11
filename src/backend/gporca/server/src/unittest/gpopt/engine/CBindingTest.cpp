//---------------------------------------------------------------------------
//	Greenplum Database
//	Copyright (C) 2011 EMC Corp.
//
//	@filename:
//		CBindingTest.cpp
//
//	@doc:
//		Test for checking bindings extracted for an expression
//---------------------------------------------------------------------------
#include "unittest/gpopt/engine/CBindingTest.h"

#include "gpopt/engine/CEngine.h"
#include "gpopt/minidump/CMinidumperUtils.h"
#include "gpopt/translate/CTranslatorDXLToExpr.h"

#include "unittest/gpopt/CTestUtils.h"

#define EXPECTED_BINDING 1
static const CHAR *szQueryFile =
	"../data/dxl/minidump/ExtractOneBindingFromScalarGroups.mdp";

GPOS_RESULT
CBindingTest::EresUnittest()
{
	CUnittest rgut[] = {GPOS_UNITTEST_FUNC(CBindingTest::EresUnittest_Basic)};

	return CUnittest::EresExecute(rgut, GPOS_ARRAY_SIZE(rgut));
}
// Consider the below input query tree:
// +--CLogicalInnerJoin
//    |--CLogicalGet "t1"
//    |--CLogicalSelect
//    |  |--CLogicalGet "t3"
//    |  +--CScalarSubqueryAny(=)
//    |     |--CLogicalUnion
//    |     |  |--CLogicalGet "t4"
//    |     |  +--CLogicalGet "t5"
//    |     +--CScalarIdent "b"
//    +--CScalarCmp (=)
//       |--CScalarIdent "b" (1)
//       +--CScalarIdent "b" (10)
//
// For the Operator CLogicalSelect, there are 2 different alternatives for
// scalar condition which are generated by CXformSimplifySelectWithSubquery:
//     1. CScalarSubqueryAny(=)["b" (19)]
//
// For the Xform, CXformInnerJoinWithInnerSelect2IndexGetApply (and other equivalent xforms),
// the pattern is as below:
// +--TJoin
//    |--CPatternLeaf
//    |--CLogicalSelect
//    |  |--TGet
//    |  +--CPatternTree
//    +--CPatternTree
//
// If we look at the pattern for the scalar condition of CLogicalSelect, it's
// defined as a CPatternTree. For the test query, the inner child of the join
// (i.e CLogicalSelect), can have 1 representation for the scalar
// condition as denoted below by Alternative 1.  Anything below the
// Scalar operators (CScalarSubqueryAny, CScalarCmp) is irrelevant in
// determining the indexes which can be used for CLogicalGet "t3". Also, there
// could be several bindings below the scalar operator, for instance, a Union
// Operation can also be performed using Agg (Global) on Union ALL, and the
// Global Agg could further have Global and Local alternatives.  We should
// extract the Scalar Group Expression only once per alternative. In the case
// of the test query used, there is 1 alternative, so the test expects that
// there should be 1 binding for the pattern defined by
// CXformJoin2IndexGetApply.
//
// Alternative: 1
// +--CLogicalSelect
//    |--CLogicalGet "t3"
//    +--CScalarSubqueryAny(=)
//       |--CLogicalUnion
//       |  |--CLogicalGet "t4"
//       |  +--CLogicalGet "t5"
//       +--CScalarIdent
//
GPOS_RESULT
CBindingTest::EresUnittest_Basic()
{
	CAutoMemoryPool amp(CAutoMemoryPool::ElcNone);
	CMemoryPool *mp = amp.Pmp();

	// load dump file
	CDXLMinidump *pdxlmd = CMinidumperUtils::PdxlmdLoad(mp, szQueryFile);
	GPOS_CHECK_ABORT;

	// set up MD providers
	CMDProviderMemory *pmdp = GPOS_NEW(mp) CMDProviderMemory(mp, szQueryFile);
	pmdp->AddRef();

	const CSystemIdArray *pdrgpsysid = pdxlmd->GetSysidPtrArray();
	CMDProviderArray *pdrgpmdp = GPOS_NEW(mp) CMDProviderArray(mp);
	pdrgpmdp->Append(pmdp);

	for (ULONG ul = 1; ul < pdrgpsysid->Size(); ul++)
	{
		pmdp->AddRef();
		pdrgpmdp->Append(pmdp);
	}

	CMDAccessor mda(mp, CMDCache::Pcache(), pdrgpsysid, pdrgpmdp);

	CBitSet *pbsEnabled = NULL;
	CBitSet *pbsDisabled = NULL;
	SetTraceflags(mp, pdxlmd->Pbs(), &pbsEnabled, &pbsDisabled);

	// setup opt ctx
	CAutoOptCtxt aoc(mp, &mda, NULL /* pceeval */,
					 CTestUtils::GetCostModel(mp));

	// translate DXL Tree -> Expr Tree
	CTranslatorDXLToExpr *pdxltr = GPOS_NEW(mp) CTranslatorDXLToExpr(mp, &mda);
	CExpression *pexprTranslated = pdxltr->PexprTranslateQuery(
		pdxlmd->GetQueryDXLRoot(), pdxlmd->PdrgpdxlnQueryOutput(),
		pdxlmd->GetCTEProducerDXLArray());

	gpdxl::ULongPtrArray *pdrgul = pdxltr->PdrgpulOutputColRefs();
	gpmd::CMDNameArray *pdrgpmdname = pdxltr->Pdrgpmdname();

	CQueryContext *pqc = CQueryContext::PqcGenerate(
		mp, pexprTranslated, pdrgul, pdrgpmdname, true /*fDeriveStats*/);

	// initialize engine and optimize query
	CEngine eng(mp);
	eng.Init(pqc, NULL /*search_stage_array*/);
	eng.Optimize();

	// extract plan
	CExpression *pexprPlan = eng.PexprExtractPlan();
	GPOS_ASSERT(NULL != pexprPlan);

	UlongPtrArray *number_of_bindings = eng.GetNumberOfBindings();
	ULONG search_stage = 0;
	ULONG bindings_for_xform =
		(ULONG) (*number_of_bindings)[search_stage]
									 [CXform::ExfJoin2IndexGetApply];

	GPOS_RESULT eres = GPOS_FAILED;

	if (bindings_for_xform == EXPECTED_BINDING)
		eres = GPOS_OK;

	// clean up
	pexprPlan->Release();
	pdrgpmdp->Release();
	GPOS_DELETE(pqc);
	GPOS_DELETE(pdxlmd);
	GPOS_DELETE(pdxltr);
	pexprTranslated->Release();
	pmdp->Release();

	return eres;
}
// EOF
