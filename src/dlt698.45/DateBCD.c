/*
 * Generated by asn1c-0.9.27 (http://lionet.info/asn1c)
 * From ASN.1 module "DLT69845"
 * 	found in "test.asn1"
 */

#include "DateBCD.h"

static asn_TYPE_member_t asn_MBR_DateBCD_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct DateBCD, year),
		(ASN_TAG_CLASS_UNIVERSAL | (4 << 2)),
		0,
		&asn_DEF_ComBCD4,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"year"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct DateBCD, month),
		(ASN_TAG_CLASS_UNIVERSAL | (4 << 2)),
		0,
		&asn_DEF_ComBCD2,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"month"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct DateBCD, day),
		(ASN_TAG_CLASS_UNIVERSAL | (4 << 2)),
		0,
		&asn_DEF_ComBCD2,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"day"
		},
};
static ber_tlv_tag_t asn_DEF_DateBCD_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_DateBCD_tag2el_1[] = {
    { (ASN_TAG_CLASS_UNIVERSAL | (4 << 2)), 0, 0, 2 }, /* year */
    { (ASN_TAG_CLASS_UNIVERSAL | (4 << 2)), 1, -1, 1 }, /* month */
    { (ASN_TAG_CLASS_UNIVERSAL | (4 << 2)), 2, -2, 0 } /* day */
};
static asn_SEQUENCE_specifics_t asn_SPC_DateBCD_specs_1 = {
	sizeof(struct DateBCD),
	offsetof(struct DateBCD, _asn_ctx),
	asn_MAP_DateBCD_tag2el_1,
	3,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	-1,	/* Start extensions */
	-1	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_DateBCD = {
	"DateBCD",
	"DateBCD",
	SEQUENCE_free,
	SEQUENCE_print,
	SEQUENCE_constraint,
	SEQUENCE_decode_ber,
	SEQUENCE_encode_der,
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
	0, 0,	/* No PER support, use "-gen-PER" to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_DateBCD_tags_1,
	sizeof(asn_DEF_DateBCD_tags_1)
		/sizeof(asn_DEF_DateBCD_tags_1[0]), /* 1 */
	asn_DEF_DateBCD_tags_1,	/* Same as above */
	sizeof(asn_DEF_DateBCD_tags_1)
		/sizeof(asn_DEF_DateBCD_tags_1[0]), /* 1 */
	0,	/* No PER visible constraints */
	asn_MBR_DateBCD_1,
	3,	/* Elements count */
	&asn_SPC_DateBCD_specs_1	/* Additional specs */
};

