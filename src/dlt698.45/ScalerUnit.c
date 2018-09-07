/*
 * Generated by asn1c-0.9.27 (http://lionet.info/asn1c)
 * From ASN.1 module "DLT69845"
 * 	found in "test.asn1"
 */

#include "ScalerUnit.h"

static asn_TYPE_member_t asn_MBR_ScalerUnit_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct ScalerUnit, conversion),
		(ASN_TAG_CLASS_UNIVERSAL | (2 << 2)),
		0,
		&asn_DEF_Integer8,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"conversion"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct ScalerUnit, scaUnit),
		(ASN_TAG_CLASS_UNIVERSAL | (10 << 2)),
		0,
		&asn_DEF_PhysicalUnit,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"scaUnit"
		},
};
static ber_tlv_tag_t asn_DEF_ScalerUnit_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_ScalerUnit_tag2el_1[] = {
    { (ASN_TAG_CLASS_UNIVERSAL | (2 << 2)), 0, 0, 0 }, /* conversion */
    { (ASN_TAG_CLASS_UNIVERSAL | (10 << 2)), 1, 0, 0 } /* scaUnit */
};
static asn_SEQUENCE_specifics_t asn_SPC_ScalerUnit_specs_1 = {
	sizeof(struct ScalerUnit),
	offsetof(struct ScalerUnit, _asn_ctx),
	asn_MAP_ScalerUnit_tag2el_1,
	2,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	-1,	/* Start extensions */
	-1	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_ScalerUnit = {
	"ScalerUnit",
	"ScalerUnit",
	SEQUENCE_free,
	SEQUENCE_print,
	SEQUENCE_constraint,
	SEQUENCE_decode_ber,
	SEQUENCE_encode_der,
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
	0, 0,	/* No PER support, use "-gen-PER" to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_ScalerUnit_tags_1,
	sizeof(asn_DEF_ScalerUnit_tags_1)
		/sizeof(asn_DEF_ScalerUnit_tags_1[0]), /* 1 */
	asn_DEF_ScalerUnit_tags_1,	/* Same as above */
	sizeof(asn_DEF_ScalerUnit_tags_1)
		/sizeof(asn_DEF_ScalerUnit_tags_1[0]), /* 1 */
	0,	/* No PER visible constraints */
	asn_MBR_ScalerUnit_1,
	2,	/* Elements count */
	&asn_SPC_ScalerUnit_specs_1	/* Additional specs */
};

