/*
 * Generated by asn1c-0.9.27 (http://lionet.info/asn1c)
 * From ASN.1 module "DLT69845"
 * 	found in "test.asn1"
 */

#ifndef	_RSD_H_
#define	_RSD_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NULL.h>
#include "Selector9.h"
#include <constr_CHOICE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum RSD_PR {
	RSD_PR_NOTHING,	/* No components present */
	RSD_PR_notSel,
	RSD_PR_sel1,
	RSD_PR_sel2,
	RSD_PR_sel3,
	RSD_PR_sel4,
	RSD_PR_sel5,
	RSD_PR_sel6,
	RSD_PR_sel7,
	RSD_PR_sel8,
	RSD_PR_sel9,
	RSD_PR_sel10
} RSD_PR;

/* Forward declarations */
struct Selector1;
struct Selector2;
struct Selector3;
struct Selector4;
struct Selector5;
struct Selector6;
struct Selector7;
struct Selector8;
struct Selector10;

/* RSD */
typedef struct RSD {
	RSD_PR present;
	union RSD_u {
		NULL_t	 notSel;
		struct Selector1	*sel1;
		struct Selector2	*sel2;
		struct Selector3	*sel3;
		struct Selector4	*sel4;
		struct Selector5	*sel5;
		struct Selector6	*sel6;
		struct Selector7	*sel7;
		struct Selector8	*sel8;
		Selector9_t	 sel9;
		struct Selector10	*sel10;
	} choice;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} RSD_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_RSD;

#ifdef __cplusplus
}
#endif

/* Referred external types */
#include "Selector1.h"
#include "Selector2.h"
#include "Selector3.h"
#include "Selector4.h"
#include "Selector5.h"
#include "Selector6.h"
#include "Selector7.h"
#include "Selector8.h"
#include "Selector10.h"

#endif	/* _RSD_H_ */
#include <asn_internal.h>
