/*
 * Generated by asn1c-0.9.27 (http://lionet.info/asn1c)
 * From ASN.1 module "DLT69845"
 * 	found in "test.asn1"
 */

#ifndef	_RCSD_H_
#define	_RCSD_H_


#include <asn_application.h>

/* Including external dependencies */
#include <asn_SEQUENCE_OF.h>
#include <constr_SEQUENCE_OF.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct CSD;

/* RCSD */
typedef struct RCSD {
	A_SEQUENCE_OF(struct CSD) list;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} RCSD_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_RCSD;

#ifdef __cplusplus
}
#endif

/* Referred external types */
#include "CSD.h"

#endif	/* _RCSD_H_ */
#include <asn_internal.h>
