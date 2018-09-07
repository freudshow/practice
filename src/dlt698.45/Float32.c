/*
 * Generated by asn1c-0.9.27 (http://lionet.info/asn1c)
 * From ASN.1 module "DLT69845"
 * 	found in "test.asn1"
 */

#include "Float32.h"

int
Float32_constraint(asn_TYPE_descriptor_t *td, const void *sptr,
			asn_app_constraint_failed_f *ctfailcb, void *app_key) {
	const OctetString_t *st = (const OctetString_t *)sptr;
	size_t size;
	
	if(!sptr) {
		_ASN_CTFAIL(app_key, td, sptr,
			"%s: value not given (%s:%d)",
			td->name, __FILE__, __LINE__);
		return -1;
	}
	
	size = st->size;
	
	if((size == 4)) {
		/* Constraint check succeeded */
		return 0;
	} else {
		_ASN_CTFAIL(app_key, td, sptr,
			"%s: constraint failed (%s:%d)",
			td->name, __FILE__, __LINE__);
		return -1;
	}
}

/*
 * This type is implemented using OctetString,
 * so here we adjust the DEF accordingly.
 */
static void
Float32_1_inherit_TYPE_descriptor(asn_TYPE_descriptor_t *td) {
	td->free_struct    = asn_DEF_OctetString.free_struct;
	td->print_struct   = asn_DEF_OctetString.print_struct;
	td->check_constraints = asn_DEF_OctetString.check_constraints;
	td->ber_decoder    = asn_DEF_OctetString.ber_decoder;
	td->der_encoder    = asn_DEF_OctetString.der_encoder;
	td->xer_decoder    = asn_DEF_OctetString.xer_decoder;
	td->xer_encoder    = asn_DEF_OctetString.xer_encoder;
	td->uper_decoder   = asn_DEF_OctetString.uper_decoder;
	td->uper_encoder   = asn_DEF_OctetString.uper_encoder;
	if(!td->per_constraints)
		td->per_constraints = asn_DEF_OctetString.per_constraints;
	td->elements       = asn_DEF_OctetString.elements;
	td->elements_count = asn_DEF_OctetString.elements_count;
	td->specifics      = asn_DEF_OctetString.specifics;
}

void
Float32_free(asn_TYPE_descriptor_t *td,
		void *struct_ptr, int contents_only) {
	Float32_1_inherit_TYPE_descriptor(td);
	td->free_struct(td, struct_ptr, contents_only);
}

int
Float32_print(asn_TYPE_descriptor_t *td, const void *struct_ptr,
		int ilevel, asn_app_consume_bytes_f *cb, void *app_key) {
	Float32_1_inherit_TYPE_descriptor(td);
	return td->print_struct(td, struct_ptr, ilevel, cb, app_key);
}

asn_dec_rval_t
Float32_decode_ber(asn_codec_ctx_t *opt_codec_ctx, asn_TYPE_descriptor_t *td,
		void **structure, const void *bufptr, size_t size, int tag_mode) {
	Float32_1_inherit_TYPE_descriptor(td);
	return td->ber_decoder(opt_codec_ctx, td, structure, bufptr, size, tag_mode);
}

asn_enc_rval_t
Float32_encode_der(asn_TYPE_descriptor_t *td,
		void *structure, int tag_mode, ber_tlv_tag_t tag,
		asn_app_consume_bytes_f *cb, void *app_key) {
	Float32_1_inherit_TYPE_descriptor(td);
	return td->der_encoder(td, structure, tag_mode, tag, cb, app_key);
}

asn_dec_rval_t
Float32_decode_xer(asn_codec_ctx_t *opt_codec_ctx, asn_TYPE_descriptor_t *td,
		void **structure, const char *opt_mname, const void *bufptr, size_t size) {
	Float32_1_inherit_TYPE_descriptor(td);
	return td->xer_decoder(opt_codec_ctx, td, structure, opt_mname, bufptr, size);
}

asn_enc_rval_t
Float32_encode_xer(asn_TYPE_descriptor_t *td, void *structure,
		int ilevel, enum xer_encoder_flags_e flags,
		asn_app_consume_bytes_f *cb, void *app_key) {
	Float32_1_inherit_TYPE_descriptor(td);
	return td->xer_encoder(td, structure, ilevel, flags, cb, app_key);
}

static ber_tlv_tag_t asn_DEF_Float32_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (4 << 2))
};
asn_TYPE_descriptor_t asn_DEF_Float32 = {
	"Float32",
	"Float32",
	Float32_free,
	Float32_print,
	Float32_constraint,
	Float32_decode_ber,
	Float32_encode_der,
	Float32_decode_xer,
	Float32_encode_xer,
	0, 0,	/* No PER support, use "-gen-PER" to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_Float32_tags_1,
	sizeof(asn_DEF_Float32_tags_1)
		/sizeof(asn_DEF_Float32_tags_1[0]), /* 1 */
	asn_DEF_Float32_tags_1,	/* Same as above */
	sizeof(asn_DEF_Float32_tags_1)
		/sizeof(asn_DEF_Float32_tags_1[0]), /* 1 */
	0,	/* No PER visible constraints */
	0, 0,	/* No members */
	0	/* No specifics */
};

