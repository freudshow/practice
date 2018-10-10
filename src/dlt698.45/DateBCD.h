/*
 * Generated by asn1c-0.9.27 (http://lionet.info/asn1c)
 * From ASN.1 module "DLT69845"
 * 	found in "test.asn1"
 */

#ifndef	_DateBCD_H_
#define	_DateBCD_H_


#include <asn_application.h>

/* Including external dependencies */
#include "ComBCD4.h"
#include "ComBCD2.h"
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* DateBCD */
typedef struct DateBCD {
	ComBCD4_t	 year;
	ComBCD2_t	 month;
	ComBCD2_t	 day;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} DateBCD_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_DateBCD;

#ifdef __cplusplus
}
#endif

#endif	/* _DateBCD_H_ */
#include <asn_internal.h>
