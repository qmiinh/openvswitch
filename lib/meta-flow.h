/*
 * Copyright (c) 2011, 2012 Nicira, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef META_FLOW_H
#define META_FLOW_H 1

#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/ip6.h>
#include "flow.h"
#include "ofp-errors.h"
#include "packets.h"

struct cls_rule;
struct ds;

/* The comment on each of these indicates the member in "union mf_value" used
 * to represent its value. */
enum mf_field_id {
    /* Metadata. */
    MFF_TUN_ID,                 /* be64 */
    MFF_METADATA,               /* be64 */
    MFF_IN_PORT,                /* be16 */

#if FLOW_N_REGS > 0
    MFF_REG0,                   /* be32 */
#endif
#if FLOW_N_REGS > 1
    MFF_REG1,                   /* be32 */
#endif
#if FLOW_N_REGS > 2
    MFF_REG2,                   /* be32 */
#endif
#if FLOW_N_REGS > 3
    MFF_REG3,                   /* be32 */
#endif
#if FLOW_N_REGS > 4
    MFF_REG4,                   /* be32 */
#endif
#if FLOW_N_REGS > 5
    MFF_REG5,                   /* be32 */
#endif
#if FLOW_N_REGS > 6
    MFF_REG6,                   /* be32 */
#endif
#if FLOW_N_REGS > 7
    MFF_REG7,                   /* be32 */
#endif

    /* L2. */
    MFF_ETH_SRC,                /* mac */
    MFF_ETH_DST,                /* mac */
    MFF_ETH_TYPE,               /* be16 */

    MFF_VLAN_TCI,               /* be16 */
    MFF_VLAN_VID,               /* be16 */
    MFF_VLAN_PCP,               /* u8 */

    /* QinQ */
    MFF_VLAN_TPID,              /* be16 */
    MFF_VLAN_QINQ_VID,          /* be16 */
    MFF_VLAN_QINQ_PCP,          /* u8 */

    /* L2.5 */
    MFF_MPLS_LABEL,             /* be32 */
    MFF_MPLS_TC,                /* u8 */
    MFF_MPLS_STACK,             /* u8 */

    /* L3. */
    MFF_IPV4_SRC,               /* be32 */
    MFF_IPV4_DST,               /* be32 */

    MFF_IPV6_SRC,               /* ipv6 */
    MFF_IPV6_DST,               /* ipv6 */
    MFF_IPV6_LABEL,             /* be32 */

    MFF_IP_PROTO,               /* u8 (used for IPv4 or IPv6) */
    MFF_IP_DSCP,                /* u8 (used for IPv4 or IPv6) */
    MFF_IP_ECN,                 /* u8 (used for IPv4 or IPv6) */
    MFF_IP_TTL,                 /* u8 (used for IPv4 or IPv6) */
    MFF_IP_FRAG,                /* u8 (used for IPv4 or IPv6) */

    MFF_ARP_OP,                 /* be16 */
    MFF_ARP_SPA,                /* be32 */
    MFF_ARP_TPA,                /* be32 */
    MFF_ARP_SHA,                /* mac */
    MFF_ARP_THA,                /* mac */

    /* L4. */
    MFF_TCP_SRC,                /* be16 (used for IPv4 or IPv6) */
    MFF_TCP_DST,                /* be16 (used for IPv4 or IPv6) */

    MFF_UDP_SRC,                /* be16 (used for IPv4 or IPv6) */
    MFF_UDP_DST,                /* be16 (used for IPv4 or IPv6) */

    MFF_ICMPV4_TYPE,            /* u8 */
    MFF_ICMPV4_CODE,            /* u8 */

    MFF_ICMPV6_TYPE,            /* u8 */
    MFF_ICMPV6_CODE,            /* u8 */

    /* ICMPv6 Neighbor Discovery. */
    MFF_ND_TARGET,              /* ipv6 */
    MFF_ND_SLL,                 /* mac */
    MFF_ND_TLL,                 /* mac */

    MFF_N_IDS
};

/* Use this macro as CASE_MFF_REGS: in a switch statement to choose all of the
 * MFF_REGx cases. */
#if FLOW_N_REGS == 1
# define CASE_MFF_REGS                                          \
    case MFF_REG0
#elif FLOW_N_REGS == 2
# define CASE_MFF_REGS                                          \
    case MFF_REG0: case MFF_REG1
#elif FLOW_N_REGS == 3
# define CASE_MFF_REGS                                          \
    case MFF_REG0: case MFF_REG1: case MFF_REG2
#elif FLOW_N_REGS == 4
# define CASE_MFF_REGS                                          \
    case MFF_REG0: case MFF_REG1: case MFF_REG2: case MFF_REG3
#elif FLOW_N_REGS == 5
# define CASE_MFF_REGS                                          \
    case MFF_REG0: case MFF_REG1: case MFF_REG2: case MFF_REG3: \
    case MFF_REG4
#elif FLOW_N_REGS == 6
# define CASE_MFF_REGS                                          \
    case MFF_REG0: case MFF_REG1: case MFF_REG2: case MFF_REG3: \
    case MFF_REG4: case MFF_REG5
#elif FLOW_N_REGS == 7
# define CASE_MFF_REGS                                          \
    case MFF_REG0: case MFF_REG1: case MFF_REG2: case MFF_REG3: \
    case MFF_REG4: case MFF_REG5: case MFF_REG6
#elif FLOW_N_REGS == 8
# define CASE_MFF_REGS                                          \
    case MFF_REG0: case MFF_REG1: case MFF_REG2: case MFF_REG3: \
    case MFF_REG4: case MFF_REG5: case MFF_REG6: case MFF_REG7
#else
# error
#endif

/* Prerequisites for matching a field.
 *
 * A field may only be matched if the correct lower-level protocols are also
 * matched.  For example, the TCP port may be matched only if the Ethernet type
 * matches ETH_TYPE_IP and the IP protocol matches IPPROTO_TCP. */
enum mf_prereqs {
    MFP_NONE,

    /* L2 requirements. */
    MFP_ARP,
    MFP_VLAN_VID,
    MFP_VLAN_TPID,
    MFP_MPLS,
    MFP_IPV4,
    MFP_IPV6,
    MFP_IP_ANY,

    /* L2+L3 requirements. */
    MFP_TCP,                    /* On IPv4 or IPv6. */
    MFP_UDP,                    /* On IPv4 or IPv6. */
    MFP_ICMPV4,
    MFP_ICMPV6,

    /* L2+L3+L4 requirements. */
    MFP_ND,
    MFP_ND_SOLICIT,
    MFP_ND_ADVERT
};

/* Forms of partial-field masking allowed for a field.
 *
 * Every field may be masked as a whole. */
enum mf_maskable {
    MFM_NONE,                   /* No sub-field masking. */
    MFM_FULLY,                  /* Every bit is individually maskable. */
    MFM_IPV6_LABEL,             /* Low 20 bits */
    MFM_VLAN_VID,               /* Lower 12 bits are individually maskable,
				 * OFPVID12_PRESENT is also accepted . */
};

/* How to format or parse a field's value. */
enum mf_string {
    /* Integer formats.
     *
     * The particular MFS_* constant sets the output format.  On input, either
     * decimal or hexadecimal (prefixed with 0x) is accepted. */
    MFS_DECIMAL,
    MFS_HEXADECIMAL,

    /* Other formats. */
    MFS_ETHERNET,
    MFS_IPV4,
    MFS_IPV6,
    MFS_OFP_PORT,               /* An OpenFlow port number or name. */
    MFS_FRAG                    /* no, yes, first, later, not_later */
};

struct mf_field {
    /* Identification. */
    enum mf_field_id id;        /* MFF_*. */
    const char *name;           /* Name of this field, e.g. "eth_type". */
    const char *extra_name;     /* Alternate name, e.g. "dl_type", or NULL. */

    /* Size.
     *
     * Most fields have n_bytes * 8 == n_bits.  There are a few exceptions:
     *
     *     - "dl_vlan" is 2 bytes but only 12 bits.
     *     - "dl_vlan_pcp" is 1 byte but only 3 bits.
     *     - "dl_vlan_qinq_vid" is 2 bytes but only 12 bits.
     *     - "dl_vlan_qinq_pcp" is 1 byte but only 3 bits.
     *     - "is_frag" is 1 byte but only 2 bits.
     *     - "ipv6_label" is 4 bytes but only 20 bits.
     *     - "mpls_label" is 4 bytes but only 20 bits.
     *     - "mpls_tc"    is 1 byte but only 3 bits.
     *     - "mpls_stack" is 1 byte but only 1 bit.
     */
    unsigned int n_bytes;       /* Width of the field in bytes. */
    unsigned int n_bits;        /* Number of significant bits in field. */

    /* Properties. */
    enum mf_maskable maskable;
    flow_wildcards_t fww_bit;   /* Either 0 or exactly one FWW_* bit. */
    enum mf_string string;
    enum mf_prereqs prereqs;
    bool writable;              /* May be written by actions? */
    bool oxm_writable;          /* writable by OXM set-field action */

    /* NXM properties.
     *
     * A few "mf_field"s don't correspond to NXM fields.  Those have 0 and
     * NULL for the following members, respectively. */
    uint32_t nxm_header;        /* An NXM_* constant (a few fields have 0). */
    const char *nxm_name;       /* The "NXM_*" constant's name. */

    /* OXM properties */
    uint32_t oxm_header;        /* Field id in the OXM basic class,
				 * an OXM_* constant.
				 * Ignored if oxm_name is NULL */
    const char *oxm_name;	/* The OXM_* constant's name,
				 * NULL if the field is not present
				 * in the OXM basic class */

};

/* The representation of a field's value. */
union mf_value {
    uint8_t u8;
    ovs_be16 be16;
    ovs_be32 be32;
    ovs_be64 be64;
    uint8_t mac[ETH_ADDR_LEN];
    struct in6_addr ipv6;
};
BUILD_ASSERT_DECL(sizeof(union mf_value) == 16);

/* Part of a field. */
struct mf_subfield {
    const struct mf_field *field;
    unsigned int ofs;           /* Bit offset. */
    unsigned int n_bits;        /* Number of bits. */
};

/* Data for some part of an mf_field.
 *
 * The data is stored "right-justified".  For example, if "union mf_subvalue
 * value" contains NXM_OF_VLAN_TCI[0..11], then one could access the
 * corresponding data in value.be16[7] as the bits in the mask htons(0xfff). */
union mf_subvalue {
    uint8_t u8[16];
    ovs_be16 be16[8];
    ovs_be32 be32[4];
    ovs_be64 be64[2];
};
BUILD_ASSERT_DECL(sizeof(union mf_value) == sizeof (union mf_subvalue));

/* Finding mf_fields. */
const struct mf_field *mf_from_id(enum mf_field_id);
const struct mf_field *mf_from_name(const char *name);
const struct mf_field *mf_from_nxm_header(uint32_t nxm_header);
const struct mf_field *mf_from_nxm_name(const char *nxm_name);

/* Inspecting wildcarded bits. */
bool mf_is_all_wild(const struct mf_field *, const struct flow_wildcards *);

bool mf_is_mask_valid(const struct mf_field *, const union mf_value *mask);
void mf_get_mask(const struct mf_field *, const struct flow_wildcards *,
                 union mf_value *mask);

/* Prerequisites. */
bool mf_are_prereqs_ok(const struct mf_field *, const struct flow *);
void mf_force_prereqs(const struct mf_field *, struct cls_rule *);

/* Field values. */
bool mf_is_value_valid(const struct mf_field *, const union mf_value *value);

void mf_get_value(const struct mf_field *, const struct flow *,
                  union mf_value *value);
void mf_set_value(const struct mf_field *, const union mf_value *value,
                  struct cls_rule *);
void mf_set_flow_value(const struct mf_field *, const union mf_value *value,
                       struct flow *);
bool mf_is_zero(const struct mf_field *, const struct flow *);

void mf_get(const struct mf_field *, const struct cls_rule *,
            union mf_value *value, union mf_value *mask);
void mf_set(const struct mf_field *,
            const union mf_value *value, const union mf_value *mask,
            struct cls_rule *);

void mf_set_wild(const struct mf_field *, struct cls_rule *);

void mf_random_value(const struct mf_field *, union mf_value *value);

/* Subfields. */
void mf_write_subfield(const struct mf_subfield *, const union mf_subvalue *,
                       struct cls_rule *);

void mf_read_subfield(const struct mf_subfield *, const struct flow *,
                      union mf_subvalue *);
uint64_t mf_get_subfield(const struct mf_subfield *, const struct flow *);


void mf_format_subfield(const struct mf_subfield *, struct ds *);
char *mf_parse_subfield__(struct mf_subfield *sf, const char **s);
const char *mf_parse_subfield(struct mf_subfield *, const char *);

enum ofperr mf_check_src(const struct mf_subfield *, const struct flow *);
enum ofperr mf_check_dst(const struct mf_subfield *, const struct flow *);

/* Parsing and formatting. */
char *mf_parse(const struct mf_field *, const char *,
               union mf_value *value, union mf_value *mask);
char *mf_parse_value(const struct mf_field *, const char *, union mf_value *);
void mf_format(const struct mf_field *,
               const union mf_value *value, const union mf_value *mask,
               struct ds *);
const struct mf_field *mf_parse_oxm_name(const char *s);

#endif /* meta-flow.h */
