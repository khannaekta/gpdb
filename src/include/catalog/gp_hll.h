/*-------------------------------------------------------------------------
 *
 * gp_hll.h
 *	  definition of the system "hyperloglog" relation (gp_hll).
 *
 * Portions Copyright (c) 2006-2010, Greenplum inc.
 * Portions Copyright (c) 2012-Present Pivotal Software, Inc.
 * Portions Copyright (c) 1996-2009, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * NOTES
 *	  the genbki.sh script reads this file and generates .bki
 *	  information from the DATA() statements.
 *
 *-------------------------------------------------------------------------
 */
#ifndef GP_HLL_H
#define GP_HLL_H

#include "catalog/genbki.h"

/*
 * The CATALOG definition has to refer to the type of stavaluesN as
 * "anyarray" so that bootstrap mode recognizes it.  There is no real
 * typedef for that, however.  Since the fields are potentially-null and
 * therefore can't be accessed directly from C code, there is no particular
 * need for the C struct definition to show a valid field type --- instead
 * we just make it int.
 */
#define anyarray int

/* ----------------
 *		gp_hll definition.  cpp turns this into
 *		typedef struct FormData_gp_hll
 * ----------------
 */
#define HyperLogLogRelationId 5000 

CATALOG(gp_hll,5000) BKI_WITHOUT_OIDS
{
	/* These fields form the unique key for the entry: */
	Oid		hllrelid;		/* relation containing attribute */
	int2		hllattnum;		/* attribute (column) stats are for */

	/* the fraction of the column's entries that are NULL: */
	bytea		hllestimator;
} FormData_gp_hll;

/* GPDB added foreign key definitions for gpcheckcat. */
FOREIGN_KEY(hllrelid REFERENCES pg_attribute(attrelid));

#undef anyarray


/* ----------------
 *		Form_gp_hll corresponds to a pointer to a tuple with
 *		the format of gp_hll relation.
 * ----------------
 */
typedef FormData_gp_hll *Form_gp_hll;

/* ----------------
 *		compiler constants for gp_hll
 * ----------------
 */
#define Natts_gp_hll			3
#define Anum_gp_hll_relid		1
#define Anum_gp_hll_attnum		2
#define Anum_gp_hll_estimator		3

#endif   /* GP_HLL_H */
