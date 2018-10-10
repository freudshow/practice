/*
 * Generated by asn1c-0.9.27 (http://lionet.info/asn1c)
 * From ASN.1 module "DLT69845"
 * 	found in "test.asn1"
 */

#ifndef	_Selector4_H_
#define	_Selector4_H_


#include <asn_application.h>

/* Including external dependencies */
#include "DateTimeBCD.h"
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct MS;

/* Selector4 */
typedef struct Selector4 {
	DateTimeBCD_t	 acqStartTime;
	struct MS	*meterSet;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} Selector4_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_Selector4;

#ifdef __cplusplus
}
#endif

/* Referred external types */
#include "MS.h"

#endif	/* _Selector4_H_ */
#include <asn_internal.h>