/*
 * Generated by asn1c-0.9.27 (http://lionet.info/asn1c)
 * From ASN.1 module "DLT69845"
 * 	found in "test.asn1"
 */

#ifndef	_DateTimeBCD_H_
#define	_DateTimeBCD_H_


#include <asn_application.h>

/* Including external dependencies */
#include "ComBCD4.h"
#include "ComBCD2.h"
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* DateTimeBCD */
typedef struct DateTimeBCD {
	ComBCD4_t	 year;
	ComBCD2_t	 month;
	ComBCD2_t	 day;
	ComBCD2_t	 hour;
	ComBCD2_t	 minute;
	ComBCD2_t	 sec;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} DateTimeBCD_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_DateTimeBCD;

#ifdef __cplusplus
}
#endif

#endif	/* _DateTimeBCD_H_ */
#include <asn_internal.h>
