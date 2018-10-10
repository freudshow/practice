/*
 * Generated by asn1c-0.9.27 (http://lionet.info/asn1c)
 * From ASN.1 module "DLT69845"
 * 	found in "test.asn1"
 */

#ifndef	_MAC_H_
#define	_MAC_H_


#include <asn_application.h>

/* Including external dependencies */
#include "OctetString.h"

#ifdef __cplusplus
extern "C" {
#endif

/* MAC */
typedef OctetString_t	 MAC_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_MAC;
asn_struct_free_f MAC_free;
asn_struct_print_f MAC_print;
asn_constr_check_f MAC_constraint;
ber_type_decoder_f MAC_decode_ber;
der_type_encoder_f MAC_encode_der;
xer_type_decoder_f MAC_decode_xer;
xer_type_encoder_f MAC_encode_xer;

#ifdef __cplusplus
}
#endif

#endif	/* _MAC_H_ */
#include <asn_internal.h>