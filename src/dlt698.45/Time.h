/*
 * Generated by asn1c-0.9.27 (http://lionet.info/asn1c)
 * From ASN.1 module "DLT69845"
 * 	found in "test.asn1"
 */

#ifndef	_Time_H_
#define	_Time_H_


#include <asn_application.h>

/* Including external dependencies */
#include "Unsigned.h"
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Time */
typedef struct Time {
	Unsigned_t	 hour;
	Unsigned_t	 minute;
	Unsigned_t	 second;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} Time_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_Time;

#ifdef __cplusplus
}
#endif

#endif	/* _Time_H_ */
#include <asn_internal.h>