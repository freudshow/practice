/*
 * Generated by asn1c-0.9.27 (http://lionet.info/asn1c)
 * From ASN.1 module "DLT69845"
 * 	found in "test.asn1"
 */

#include "Selector8.h"

static asn_TYPE_member_t asn_MBR_Selector8_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct Selector8, acqSucTimeStartValue),
		(ASN_TAG_CLASS_UNIVERSAL | (16 << 2)),
		0,
		&asn_DEF_DateTimeBCD,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"acqSucTimeStartValue"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct Selector8, acqSucTimeEndValue),
		(ASN_TAG_CLASS_UNIVERSAL | (16 << 2)),
		0,
		&asn_DEF_DateTimeBCD,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"acqSucTimeEndValue"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct Selector8, timeInterv),
		(ASN_TAG_CLASS_UNIVERSAL | (16 << 2)),
		0,
		&asn_DEF_TI,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"timeInterv"
		},
	{ ATF_POINTER, 0, offsetof(struct Selector8, meterSet),
		-1 /* Ambiguous tag (CHOICE?) */,
		0,
		&asn_DEF_MS,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"meterSet"
		},
};
static ber_tlv_tag_t asn_DEF_Selector8_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_Selector8_tag2el_1[] = {
    { (ASN_TAG_CLASS_UNIVERSAL | (16 << 2)), 0, 0, 2 }, /* acqSucTimeStartValue */
    { (ASN_TAG_CLASS_UNIVERSAL | (16 << 2)), 1, -1, 1 }, /* acqSucTimeEndValue */
    { (ASN_TAG_CLASS_UNIVERSAL | (16 << 2)), 2, -2, 0 }, /* timeInterv */
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 3, 0, 0 }, /* noMeter */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 3, 0, 0 }, /* allAddr */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 3, 0, 0 }, /* aGrpUserType */
    { (ASN_TAG_CLASS_CONTEXT | (3 << 2)), 3, 0, 0 }, /* aGrpUserAddr */
    { (ASN_TAG_CLASS_CONTEXT | (4 << 2)), 3, 0, 0 }, /* aGrpCfgSeq */
    { (ASN_TAG_CLASS_CONTEXT | (5 << 2)), 3, 0, 0 }, /* aGrpUserTypeInterv */
    { (ASN_TAG_CLASS_CONTEXT | (6 << 2)), 3, 0, 0 }, /* aGrpUserAddrInterv */
    { (ASN_TAG_CLASS_CONTEXT | (7 << 2)), 3, 0, 0 } /* aGrpUserCfgSeqInterv */
};
static asn_SEQUENCE_specifics_t asn_SPC_Selector8_specs_1 = {
	sizeof(struct Selector8),
	offsetof(struct Selector8, _asn_ctx),
	asn_MAP_Selector8_tag2el_1,
	11,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	-1,	/* Start extensions */
	-1	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_Selector8 = {
	"Selector8",
	"Selector8",
	SEQUENCE_free,
	SEQUENCE_print,
	SEQUENCE_constraint,
	SEQUENCE_decode_ber,
	SEQUENCE_encode_der,
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
	0, 0,	/* No PER support, use "-gen-PER" to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_Selector8_tags_1,
	sizeof(asn_DEF_Selector8_tags_1)
		/sizeof(asn_DEF_Selector8_tags_1[0]), /* 1 */
	asn_DEF_Selector8_tags_1,	/* Same as above */
	sizeof(asn_DEF_Selector8_tags_1)
		/sizeof(asn_DEF_Selector8_tags_1[0]), /* 1 */
	0,	/* No PER visible constraints */
	asn_MBR_Selector8_1,
	4,	/* Elements count */
	&asn_SPC_Selector8_specs_1	/* Additional specs */
};
