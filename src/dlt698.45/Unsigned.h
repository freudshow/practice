/*
 * Generated by asn1c-0.9.27 (http://lionet.info/asn1c)
 * From ASN.1 module "DLT69845"
 * 	found in "test.asn1"
 */

#ifndef	_Unsigned_H_
#define	_Unsigned_H_


#include <asn_application.h>

/* Including external dependencies */
#include <BIT_STRING.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Unsigned */
typedef BIT_STRING_t	 Unsigned_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_Unsigned;
asn_struct_free_f Unsigned_free;
asn_struct_print_f Unsigned_print;
asn_constr_check_f Unsigned_constraint;
ber_type_decoder_f Unsigned_decode_ber;
der_type_encoder_f Unsigned_encode_der;
xer_type_decoder_f Unsigned_decode_xer;
xer_type_encoder_f Unsigned_encode_xer;

#ifdef __cplusplus
}
#endif

#endif	/* _Unsigned_H_ */
#include <asn_internal.h>