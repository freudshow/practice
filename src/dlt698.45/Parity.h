/*
 * Generated by asn1c-0.9.27 (http://lionet.info/asn1c)
 * From ASN.1 module "DLT69845"
 * 	found in "test.asn1"
 */

#ifndef	_Parity_H_
#define	_Parity_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeEnumerated.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum Parity {
	Parity_no	= 0,
	Parity_odd	= 1,
	Parity_even	= 2
} e_Parity;

/* Parity */
typedef long	 Parity_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_Parity;
asn_struct_free_f Parity_free;
asn_struct_print_f Parity_print;
asn_constr_check_f Parity_constraint;
ber_type_decoder_f Parity_decode_ber;
der_type_encoder_f Parity_encode_der;
xer_type_decoder_f Parity_decode_xer;
xer_type_encoder_f Parity_encode_xer;

#ifdef __cplusplus
}
#endif

#endif	/* _Parity_H_ */
#include <asn_internal.h>
