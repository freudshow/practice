/*
 * Generated by asn1c-0.9.27 (http://lionet.info/asn1c)
 * From ASN.1 module "DLT69845"
 * 	found in "test.asn1"
 */

#include "SID.h"

static asn_TYPE_member_t asn_MBR_SID_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct SID, identity),
		(ASN_TAG_CLASS_UNIVERSAL | (2 << 2)),
		0,
		&asn_DEF_DoubleLongUnsigned,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"identity"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct SID, appendData),
		(ASN_TAG_CLASS_UNIVERSAL | (4 << 2)),
		0,
		&asn_DEF_OctetString,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"appendData"
		},
};
static ber_tlv_tag_t asn_DEF_SID_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_SID_tag2el_1[] = {
    { (ASN_TAG_CLASS_UNIVERSAL | (2 << 2)), 0, 0, 0 }, /* identity */
    { (ASN_TAG_CLASS_UNIVERSAL | (4 << 2)), 1, 0, 0 } /* appendData */
};
static asn_SEQUENCE_specifics_t asn_SPC_SID_specs_1 = {
	sizeof(struct SID),
	offsetof(struct SID, _asn_ctx),
	asn_MAP_SID_tag2el_1,
	2,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	-1,	/* Start extensions */
	-1	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_SID = {
	"SID",
	"SID",
	SEQUENCE_free,
	SEQUENCE_print,
	SEQUENCE_constraint,
	SEQUENCE_decode_ber,
	SEQUENCE_encode_der,
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
	0, 0,	/* No PER support, use "-gen-PER" to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_SID_tags_1,
	sizeof(asn_DEF_SID_tags_1)
		/sizeof(asn_DEF_SID_tags_1[0]), /* 1 */
	asn_DEF_SID_tags_1,	/* Same as above */
	sizeof(asn_DEF_SID_tags_1)
		/sizeof(asn_DEF_SID_tags_1[0]), /* 1 */
	0,	/* No PER visible constraints */
	asn_MBR_SID_1,
	2,	/* Elements count */
	&asn_SPC_SID_specs_1	/* Additional specs */
};

