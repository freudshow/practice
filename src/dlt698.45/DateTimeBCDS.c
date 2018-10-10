/*
 * Generated by asn1c-0.9.27 (http://lionet.info/asn1c)
 * From ASN.1 module "DLT69845"
 * 	found in "test.asn1"
 */

#include "DateTimeBCDS.h"

static asn_TYPE_member_t asn_MBR_DateTimeBCDS_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct DateTimeBCDS, year),
		(ASN_TAG_CLASS_UNIVERSAL | (4 << 2)),
		0,
		&asn_DEF_ComBCD4,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"year"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct DateTimeBCDS, month),
		(ASN_TAG_CLASS_UNIVERSAL | (4 << 2)),
		0,
		&asn_DEF_ComBCD2,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"month"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct DateTimeBCDS, day),
		(ASN_TAG_CLASS_UNIVERSAL | (4 << 2)),
		0,
		&asn_DEF_ComBCD2,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"day"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct DateTimeBCDS, hour),
		(ASN_TAG_CLASS_UNIVERSAL | (4 << 2)),
		0,
		&asn_DEF_ComBCD2,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"hour"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct DateTimeBCDS, minute),
		(ASN_TAG_CLASS_UNIVERSAL | (4 << 2)),
		0,
		&asn_DEF_ComBCD2,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"minute"
		},
};
static ber_tlv_tag_t asn_DEF_DateTimeBCDS_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_DateTimeBCDS_tag2el_1[] = {
    { (ASN_TAG_CLASS_UNIVERSAL | (4 << 2)), 0, 0, 4 }, /* year */
    { (ASN_TAG_CLASS_UNIVERSAL | (4 << 2)), 1, -1, 3 }, /* month */
    { (ASN_TAG_CLASS_UNIVERSAL | (4 << 2)), 2, -2, 2 }, /* day */
    { (ASN_TAG_CLASS_UNIVERSAL | (4 << 2)), 3, -3, 1 }, /* hour */
    { (ASN_TAG_CLASS_UNIVERSAL | (4 << 2)), 4, -4, 0 } /* minute */
};
static asn_SEQUENCE_specifics_t asn_SPC_DateTimeBCDS_specs_1 = {
	sizeof(struct DateTimeBCDS),
	offsetof(struct DateTimeBCDS, _asn_ctx),
	asn_MAP_DateTimeBCDS_tag2el_1,
	5,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	-1,	/* Start extensions */
	-1	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_DateTimeBCDS = {
	"DateTimeBCDS",
	"DateTimeBCDS",
	SEQUENCE_free,
	SEQUENCE_print,
	SEQUENCE_constraint,
	SEQUENCE_decode_ber,
	SEQUENCE_encode_der,
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
	0, 0,	/* No PER support, use "-gen-PER" to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_DateTimeBCDS_tags_1,
	sizeof(asn_DEF_DateTimeBCDS_tags_1)
		/sizeof(asn_DEF_DateTimeBCDS_tags_1[0]), /* 1 */
	asn_DEF_DateTimeBCDS_tags_1,	/* Same as above */
	sizeof(asn_DEF_DateTimeBCDS_tags_1)
		/sizeof(asn_DEF_DateTimeBCDS_tags_1[0]), /* 1 */
	0,	/* No PER visible constraints */
	asn_MBR_DateTimeBCDS_1,
	5,	/* Elements count */
	&asn_SPC_DateTimeBCDS_specs_1	/* Additional specs */
};
