/*
 * Generated by asn1c-0.9.27 (http://lionet.info/asn1c)
 * From ASN.1 module "DLT69845"
 * 	found in "test.asn1"
 */

#ifndef	_ComBCD8_H_
#define	_ComBCD8_H_


#include <asn_application.h>

/* Including external dependencies */
#include "OctetString.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ComBCD8 */
typedef OctetString_t	 ComBCD8_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_ComBCD8;
asn_struct_free_f ComBCD8_free;
asn_struct_print_f ComBCD8_print;
asn_constr_check_f ComBCD8_constraint;
ber_type_decoder_f ComBCD8_decode_ber;
der_type_encoder_f ComBCD8_encode_der;
xer_type_decoder_f ComBCD8_decode_xer;
xer_type_encoder_f ComBCD8_encode_xer;

#ifdef __cplusplus
}
#endif

#endif	/* _ComBCD8_H_ */
#include <asn_internal.h>