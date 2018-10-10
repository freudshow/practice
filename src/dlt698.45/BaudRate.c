/*
 * Generated by asn1c-0.9.27 (http://lionet.info/asn1c)
 * From ASN.1 module "DLT69845"
 * 	found in "test.asn1"
 */

#include "BaudRate.h"

int
BaudRate_constraint(asn_TYPE_descriptor_t *td, const void *sptr,
			asn_app_constraint_failed_f *ctfailcb, void *app_key) {
	/* Replace with underlying type checker */
	td->check_constraints = asn_DEF_NativeEnumerated.check_constraints;
	return td->check_constraints(td, sptr, ctfailcb, app_key);
}

/*
 * This type is implemented using NativeEnumerated,
 * so here we adjust the DEF accordingly.
 */
static void
BaudRate_1_inherit_TYPE_descriptor(asn_TYPE_descriptor_t *td) {
	td->free_struct    = asn_DEF_NativeEnumerated.free_struct;
	td->print_struct   = asn_DEF_NativeEnumerated.print_struct;
	td->check_constraints = asn_DEF_NativeEnumerated.check_constraints;
	td->ber_decoder    = asn_DEF_NativeEnumerated.ber_decoder;
	td->der_encoder    = asn_DEF_NativeEnumerated.der_encoder;
	td->xer_decoder    = asn_DEF_NativeEnumerated.xer_decoder;
	td->xer_encoder    = asn_DEF_NativeEnumerated.xer_encoder;
	td->uper_decoder   = asn_DEF_NativeEnumerated.uper_decoder;
	td->uper_encoder   = asn_DEF_NativeEnumerated.uper_encoder;
	if(!td->per_constraints)
		td->per_constraints = asn_DEF_NativeEnumerated.per_constraints;
	td->elements       = asn_DEF_NativeEnumerated.elements;
	td->elements_count = asn_DEF_NativeEnumerated.elements_count;
     /* td->specifics      = asn_DEF_NativeEnumerated.specifics;	// Defined explicitly */
}

void
BaudRate_free(asn_TYPE_descriptor_t *td,
		void *struct_ptr, int contents_only) {
	BaudRate_1_inherit_TYPE_descriptor(td);
	td->free_struct(td, struct_ptr, contents_only);
}

int
BaudRate_print(asn_TYPE_descriptor_t *td, const void *struct_ptr,
		int ilevel, asn_app_consume_bytes_f *cb, void *app_key) {
	BaudRate_1_inherit_TYPE_descriptor(td);
	return td->print_struct(td, struct_ptr, ilevel, cb, app_key);
}

asn_dec_rval_t
BaudRate_decode_ber(asn_codec_ctx_t *opt_codec_ctx, asn_TYPE_descriptor_t *td,
		void **structure, const void *bufptr, size_t size, int tag_mode) {
	BaudRate_1_inherit_TYPE_descriptor(td);
	return td->ber_decoder(opt_codec_ctx, td, structure, bufptr, size, tag_mode);
}

asn_enc_rval_t
BaudRate_encode_der(asn_TYPE_descriptor_t *td,
		void *structure, int tag_mode, ber_tlv_tag_t tag,
		asn_app_consume_bytes_f *cb, void *app_key) {
	BaudRate_1_inherit_TYPE_descriptor(td);
	return td->der_encoder(td, structure, tag_mode, tag, cb, app_key);
}

asn_dec_rval_t
BaudRate_decode_xer(asn_codec_ctx_t *opt_codec_ctx, asn_TYPE_descriptor_t *td,
		void **structure, const char *opt_mname, const void *bufptr, size_t size) {
	BaudRate_1_inherit_TYPE_descriptor(td);
	return td->xer_decoder(opt_codec_ctx, td, structure, opt_mname, bufptr, size);
}

asn_enc_rval_t
BaudRate_encode_xer(asn_TYPE_descriptor_t *td, void *structure,
		int ilevel, enum xer_encoder_flags_e flags,
		asn_app_consume_bytes_f *cb, void *app_key) {
	BaudRate_1_inherit_TYPE_descriptor(td);
	return td->xer_encoder(td, structure, ilevel, flags, cb, app_key);
}

static asn_INTEGER_enum_map_t asn_MAP_BaudRate_value2enum_1[] = {
	{ 0,	10,	"baud300bps" },
	{ 1,	10,	"baud600bps" },
	{ 2,	11,	"baud1200bps" },
	{ 3,	11,	"baud2400bps" },
	{ 4,	11,	"baud4800bps" },
	{ 5,	11,	"baud7200bps" },
	{ 6,	11,	"baud9600bps" },
	{ 7,	12,	"baud19200bps" },
	{ 8,	12,	"baud38400bps" },
	{ 9,	12,	"baud57600bps" },
	{ 10,	13,	"baud115200bps" },
	{ 255,	9,	"autoAjust" }
};
static unsigned int asn_MAP_BaudRate_enum2value_1[] = {
	11,	/* autoAjust(255) */
	10,	/* baud115200bps(10) */
	2,	/* baud1200bps(2) */
	7,	/* baud19200bps(7) */
	3,	/* baud2400bps(3) */
	0,	/* baud300bps(0) */
	8,	/* baud38400bps(8) */
	4,	/* baud4800bps(4) */
	9,	/* baud57600bps(9) */
	1,	/* baud600bps(1) */
	5,	/* baud7200bps(5) */
	6	/* baud9600bps(6) */
};
static asn_INTEGER_specifics_t asn_SPC_BaudRate_specs_1 = {
	asn_MAP_BaudRate_value2enum_1,	/* "tag" => N; sorted by tag */
	asn_MAP_BaudRate_enum2value_1,	/* N => "tag"; sorted by N */
	12,	/* Number of elements in the maps */
	0,	/* Enumeration is not extensible */
	1,	/* Strict enumeration */
	0,	/* Native long size */
	0
};
static ber_tlv_tag_t asn_DEF_BaudRate_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (10 << 2))
};
asn_TYPE_descriptor_t asn_DEF_BaudRate = {
	"BaudRate",
	"BaudRate",
	BaudRate_free,
	BaudRate_print,
	BaudRate_constraint,
	BaudRate_decode_ber,
	BaudRate_encode_der,
	BaudRate_decode_xer,
	BaudRate_encode_xer,
	0, 0,	/* No PER support, use "-gen-PER" to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_BaudRate_tags_1,
	sizeof(asn_DEF_BaudRate_tags_1)
		/sizeof(asn_DEF_BaudRate_tags_1[0]), /* 1 */
	asn_DEF_BaudRate_tags_1,	/* Same as above */
	sizeof(asn_DEF_BaudRate_tags_1)
		/sizeof(asn_DEF_BaudRate_tags_1[0]), /* 1 */
	0,	/* No PER visible constraints */
	0, 0,	/* Defined elsewhere */
	&asn_SPC_BaudRate_specs_1	/* Additional specs */
};
