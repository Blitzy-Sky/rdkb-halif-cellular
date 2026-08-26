/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2022 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/
/**
* @file cellular_hal.h
* @brief For CCSP Component: Cellular Manager HAL Layer
*
* This header is the whole of the Cellular Manager HAL contract. It declares 36
* entry points over an implicit, singleton modem context, the data types a caller
* constructs and reads, and the seven callback types through which the
* implementation reports asynchronous conditions.
*
* Reading this interface, three properties hold across every declaration and are
* stated once here rather than repeated in full on each block. Each is sourced from
* this repository's specification, docs/pages/halSpec.md, which is the authority for
* repository-wide policy; the declaration is the authority for interface scope.
*
* - Return vocabulary. This header defines exactly two status codes, RETURN_OK (0)
*   and RETURN_ERROR (-1), and no reason code. A failure therefore names neither its
*   cause nor whether it is retryable. Two calls are the exception and return TRUE or
*   FALSE instead of a status: cellular_hal_IsModemDevicePresent() and
*   cellular_hal_IsModemControlInterfaceOpened(). See 'Internal Error Handling'.
* - Thread safety. This interface is not thread safe. The obligation rests on the
*   caller, which must serialise its calls, including calls issued from another
*   process. See 'Threading Model'.
* - Blocking. Every call completes synchronously and returns its result to the
*   caller. No numeric timeout is specified for any call, and none is derivable from
*   these declarations, so a caller that must bound its latency imposes that bound
*   itself. The one stated blocking behaviour is that cellular_hal_init() may block
*   while the modem hardware is not ready. See 'Blocking calls'.
*
* Where this interface establishes no answer to a question a caller has to ask, the
* block on the declaration says so explicitly rather than supplying a plausible
* value.
*/
/**
 * @defgroup CELLULAR_HAL CELLULAR HAL
 *
 * This module provides the function call prototypes used for the Cellular Manager abstraction layer..
 *
 * @defgroup CELLULAR_HAL_TYPES   CELLULAR HAL Data Types
 * @ingroup  CELLULAR_HAL
 *
 * @defgroup CELLULAR_HAL_APIS   CELLULAR HAL APIs
 * @ingroup  CELLULAR_HAL
 *
 **/
#ifndef _CELLULAR_HAL_H_
#define _CELLULAR_HAL_H_
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdint.h>
#include <pthread.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/**********************************************************************
                STRUCTURE AND CONSTANT DEFINITIONS
**********************************************************************/
/**
* @addtogroup CELLULAR_HAL_TYPES
* @{
*/
/**
* @brief Declares that the QMI library is the supported modem control library.
*
* QMI_SUPPORT is defined by this header whenever CELLULAR_MGR_LITE is not defined by
* the build. It is a declaration for vendor code and callers that compile against
* this header; it adds and removes no declaration, data type or return code, so the
* API surface is identical in both configurations. CELLULAR_MGR_LITE is never
* defined here - it is supplied by the build - and defining it suppresses
* QMI_SUPPORT and with it any conditional the vendor guards on that symbol.
*/
#ifndef CELLULAR_MGR_LITE
#define QMI_SUPPORT
#endif
#define RETURN_ERROR        (-1)      /*!< Failure status returned by every call in this interface
                                       *   except the two that return TRUE or FALSE. It is the only
                                       *   failure value defined here: it carries no reason code and
                                       *   does not say whether the failure is retryable. */
#define RETURN_OK           (0)       /*!< Success status returned by every call in this interface
                                       *   except the two that return TRUE or FALSE. */
#ifndef TRUE
#define TRUE                (1)       /*!< Asserted value. Returned by
                                       *   cellular_hal_IsModemDevicePresent() and
                                       *   cellular_hal_IsModemControlInterfaceOpened(), and the set
                                       *   value of every unsigned char flag in this interface's
                                       *   structures. Defined only if not already defined. */
#endif
#ifndef FALSE
#define FALSE               (0)       /*!< Cleared value, the complement of TRUE, on the same terms.
                                       *   Note that it shares the numeric value of RETURN_OK, so a
                                       *   caller must not test one call's result against the other
                                       *   convention. Defined only if not already defined. */
#endif
#ifndef BUFLEN_8
#define BUFLEN_8            (8)       /*!< Buffer length constant, 8 bytes. This interface states the
                                       *   size each argument requires on that argument; it is not
                                       *   always one of these constants. Defined only if not already
                                       *   defined. */
#endif
#ifndef BUFLEN_32
#define BUFLEN_32           (32)      /*!< Buffer length constant, 32 bytes. Larger than the minimum
                                       *   this interface asks for any identifier it returns - IMEI,
                                       *   IMEISV, ICCID and MSISDN - so a caller that allocates it
                                       *   has margin over every one of those minima. Defined only
                                       *   if not already defined. */
#endif
#ifndef BUFLEN_64
#define BUFLEN_64           (64)      /*!< Buffer length constant, 64 bytes. The declared size of the
                                       *   ProfileName and APN members of CellularProfileStruct, and
                                       *   the size this interface asks for the slot name and slot
                                       *   type delivered to cellular_device_slot_status_api_callback.
                                       *   Defined only if not already defined. */
#endif
#ifndef BUFLEN_128
#define BUFLEN_128          (128)     /*!< Buffer length constant, 128 bytes. The size this interface
                                       *   asks for the firmware version and the radio access
                                       *   technology strings. Defined only if not already defined. */
#endif
#ifndef BUFLEN_256
#define BUFLEN_256          (256)     /*!< Buffer length constant, 256 bytes. The declared size of the
                                       *   Username, Password and Domains members of this interface's
                                       *   structures. Defined only if not already defined. */
#endif
#define CELLULAR_PROFILE_ID_UNKNOWN               (-1)          /*!< Sentinel for the ProfileID member of CellularProfileStruct when no profile identifier is present. */
#define CELLULAR_SLOT_ID_UNKNOWN                  (-1)          /*!< Sentinel for a card slot identifier that is not present. No call in this interface takes or returns it directly; it exists for a caller that tracks the selected slot. */
#define CELLULAR_PDP_CONTEXT_UNKNOWN              (-1)          /*!< Sentinel for the PDPContextNumber member of CellularProfileStruct when no packet data protocol context is present. */
#define CELLULAR_PACKET_DATA_INVALID_HANDLE       (0xFFFFFFFF)  /*!< Sentinel for an invalid packet data handle. This interface declares no call that accepts or returns a packet data handle, so the value is published for vendor and caller code that carries one alongside this interface. */

/**
* @brief Operational status of a cellular network interface.
*
* Reported through cellular_hal_get_current_modem_interface_status(). The members
* follow the interface-status vocabulary a TR-181 style management layer expects, so
* a caller can map them onto its own data model without translation. This interface
* specifies no transition set over these values: a caller may read the current
* status but must not infer which status can follow which.
*/
typedef enum _CellularInterfaceStatus_t {
    IF_UP = 1,         /*!< The interface is up and passing traffic. */
    IF_DOWN,           /*!< The interface is administratively or operationally down and passes no traffic. */
    IF_UNKNOWN,        /*!< The implementation cannot determine the interface status. Not an error return: the call still reports RETURN_OK. */
    IF_DORMANT,        /*!< The interface is up but idle, with the radio bearer released and re-established on demand. */
    IF_NOTPRESENT,     /*!< The interface is absent, typically because the modem hardware is missing. */
    IF_LOWERLAYERDOWN, /*!< The interface itself is up but the layer beneath it - the radio or packet bearer - is down. */
    IF_ERROR           /*!< The interface is in an error state. This interface does not enumerate the causes, so a caller cannot learn which from the value. */
} CellularInterfaceStatus_t;

/**
* @brief Preferred IP family for a cellular interface.
*
* Supplied by a caller in the enIPFamilyPreference member of
* CellularContextInitInputStruct. It expresses a preference at initialization; the
* family a packet data network is actually started for is chosen separately, by the
* CellularNetworkIPType_t argument to cellular_hal_start_network().
*/
typedef enum _CellularIpFamilyPref_t {
    IP_FAMILY_UNKNOWN = 1, /*!< No preference is expressed. This interface does not state which family an implementation then selects. */
    IP_FAMILY_IPV4,       /*!< IPv4 is preferred. */
    IP_FAMILY_IPV6,       /*!< IPv6 is preferred. */
    IP_FAMILY_IPV4_IPV6  /*!< Either family is acceptable, including both at once. */
} CellularIpFamilyPref_t;

/**
* @brief Preferred radio access technology for a cellular interface.
*
* Supplied by a caller in the enPreferenceTechnology member of
* CellularContextInitInputStruct. The set spans second-generation packet data
* through to 5G New Radio. It is a preference, not an assertion of availability:
* this interface does not state what an implementation does when the preferred
* technology is not available, and it provides no way to query which members a
* given modem supports. The textual radio access technology calls -
* cellular_hal_get_modem_supported_radio_technology() and its siblings - use an
* independent string vocabulary and do not report these members.
*/
typedef enum _CellularPrefAccessTechnology_t {
    PREF_GPRS = 1,         /*!< GSM with General Packet Radio Service. */
    PREF_EDGE,             /*!< GSM with Enhanced Data rates for GSM Evolution. */
    PREF_UMTS,             /*!< UMTS without High Speed Packet Access. */
    PREF_UMTSHSPA,         /*!< UMTS with 3GPP High Speed Packet Access. */
    PREF_CDMA2000OneX,     /*!< CDMA2000 1x, the circuit and low-rate packet mode. */
    PREF_CDMA2000HRPD,     /*!< CDMA2000 High Rate Packet Data. */
    PREF_LTE,             /*!< E-UTRAN, Long Term Evolution. */
    PREF_NR               /*!< 5G New Radio. This interface does not distinguish standalone from non-standalone operation. */
} CellularPrefAccessTechnology_t;

/**
* @brief Packet Data Protocol (PDP) address type of a PDP context.
*
* Carried in the PDPType member of CellularProfileStruct, so it selects the address
* family a profile requests when the context is activated, and delivered as the
* second argument to cellular_device_profile_status_api_callback. It is distinct
* from CellularNetworkIPType_t, which selects the family of a packet data network
* start or stop rather than of a profile.
*/
typedef enum _CellularPDPType_t {
    CELLULAR_PDP_TYPE_IPV4,       /*!< IPv4 address. */
    CELLULAR_PDP_TYPE_PPP,        /*!< Point-to-Point Protocol address. */
    CELLULAR_PDP_TYPE_IPV6,       /*!< IPv6 address. */
    CELLULAR_PDP_TYPE_IPV4_OR_IPV6 /*!< Either family. This interface does not state whether one address or two are assigned. */
} CellularPDPType_t;

/**
* @brief Authentication method a Packet Data Protocol context uses.
*
* Carried in the PDPAuthentication member of CellularProfileStruct. Where the value
* is not CELLULAR_PDP_AUTHENTICATION_NONE, the Username and Password members of the
* same structure carry the credential the method authenticates with.
*/
typedef enum _CellularPDPAuthentication_t {
    CELLULAR_PDP_AUTHENTICATION_NONE = 0, /*!< No authentication. The Username and Password members are not used. */
    CELLULAR_PDP_AUTHENTICATION_PAP,      /*!< Password Authentication Protocol, which transmits the credential without protecting it. */
    CELLULAR_PDP_AUTHENTICATION_CHAP      /*!< Challenge-Handshake Authentication Protocol. */
} CellularPDPAuthentication_t;

/**
* @brief Standards family a cellular profile belongs to.
*
* Carried in the ProfileType member of CellularProfileStruct. It selects which body
* of standards defines the profile's fields, not which radio technology is used.
*/
typedef enum _CellularProfileType_t {
    CELLULAR_PROFILE_TYPE_3GPP,  /*!< A 3rd Generation Partnership Project profile - GSM, UMTS, LTE and NR. */
    CELLULAR_PROFILE_TYPE_3GPP2  /*!< A 3rd Generation Partnership Project 2 profile - the CDMA2000 family. */
} CellularProfileType_t;

/**
* @brief Source of the network configuration a Packet Data Protocol context uses.
*
* Carried in the PDPNetworkConfig member of CellularProfileStruct. It selects how
* the address, mask, gateway and name servers reported through CellularIPStruct are
* obtained once the context is active.
*/
typedef enum _CellularPDPNetworkConfig_t {
    CELLULAR_PDP_NETWORK_CONFIG_NAS = 1, /*!< The configuration comes from the Network Attachment Subsystem, in the bearer signalling itself. */
    CELLULAR_PDP_NETWORK_CONFIG_DHCP   /*!< The configuration is obtained over the bearer by the Dynamic Host Configuration Protocol. */
} CellularPDPNetworkConfig_t;

/**
* @brief Operating command applied to the modem.
*
* Supplied by a caller to cellular_hal_set_modem_operating_configuration(), and
* reported back as the modem_operating_mode argument of
* cellular_device_open_status_api_callback. The two reset members are destructive.
* All this interface states about them is what it names: CELLULAR_MODEM_SET_RESET
* resets the modem, and CELLULAR_MODEM_SET_FACTORY_RESET resets it to factory
* defaults, which discards its configuration including any profile created through
* this interface. What a plain reset preserves is not established - this interface
* does not state whether configuration, profiles, a card selection or a callback
* installed before the reset are still in place afterwards, so a caller must not
* infer from the factory variant's name that the plain one keeps them. This
* interface also does not state how long either takes, nor which call a caller
* should use to observe that the modem has come back.
*/
typedef enum _CellularModemOperatingConfiguration_t {
    CELLULAR_MODEM_SET_ONLINE = 1,      /*!< Bring the modem online, allowing it to register and carry traffic. */
    CELLULAR_MODEM_SET_OFFLINE,        /*!< Take the modem offline. The radio is deactivated and no traffic is carried. */
    CELLULAR_MODEM_SET_LOW_POWER_MODE, /*!< Place the modem in low power mode. This interface does not state which functions remain available in that mode. */
    CELLULAR_MODEM_SET_RESET,         /*!< Reset the modem. This interface states no more than that: what the reset
                                        *   preserves - configuration, profiles, the card selection, an installed
                                        *   callback - is not specified, so a caller must not read this member as the
                                        *   configuration-preserving counterpart of CELLULAR_MODEM_SET_FACTORY_RESET.
                                        *   Equivalent in effect to cellular_hal_modem_reset(). */
    CELLULAR_MODEM_SET_FACTORY_RESET  /*!< Reset the modem to factory defaults, discarding its configuration. Equivalent in effect to cellular_hal_modem_factory_reset(). */
} CellularModemOperatingConfiguration_t;

/**
* @brief Domains the modem is registered for.
*
* Delivered as the third argument to cellular_device_registration_status_callback
* and carried in the registered_service member of CellularCurrentPlmnInfoStruct. A
* caller that implements the registration callback has to interpret it: packet data
* is only carried where the packet-switched domain is registered.
*/
typedef enum _CellularModemRegisteredServiceType_t
{
    CELLULAR_MODEM_REGISTERED_SERVICE_NONE = 0, /*!< Registered for no service. No traffic of either kind is carried. */
    CELLULAR_MODEM_REGISTERED_SERVICE_PS,      /*!< Registered for packet-switched service only. */
    CELLULAR_MODEM_REGISTERED_SERVICE_CS,      /*!< Registered for circuit-switched service only. Packet data is not available. */
    CELLULAR_MODEM_REGISTERED_SERVICE_CS_PS    /*!< Registered for both the circuit-switched and the packet-switched domain. */
}
CellularModemRegisteredServiceType_t;

/**
* @brief Whether the device is registered on a network.
*
* Carried in the registration_status member of CellularCurrentPlmnInfoStruct. It is
* the two-valued form of the same fact that CellularDeviceNASStatus_t reports with
* three values through the registration callback; the two are independent types and
* neither is convertible to the other.
*/
typedef enum _CellularRegistrationStatus_t
{
    DEVICE_REGISTERED = 1, /*!< The device is registered on the network named by the surrounding structure. */
    DEVICE_NOT_REGISTERED  /*!< The device is not registered. The remaining members of the surrounding structure may not describe a serving network. */
} CellularRegistrationStatus_t;

/**
* @brief A Packet Data Protocol context profile.
*
* The profile is the unit this interface creates, modifies, deletes and lists, and
* the input a packet data network is started on. A caller constructs it for
* cellular_hal_profile_create(), cellular_hal_profile_delete(),
* cellular_hal_profile_modify() and cellular_hal_start_network(), and reads it back
* from cellular_hal_get_profile_list(). It is also embedded by value as the
* stIfInput member of CellularContextInitInputStruct.
*
* Each character array of this structure states its own capacity and its own
* termination rule on the member below, and for every member of this structure that
* rule is the same one: the array is zero-terminated with the terminator included in
* the declared size, so the longest string it carries is one byte shorter than its
* size. That is a per-member statement rather than an interface-wide one, because it
* does not hold everywhere this interface declares a character array: elsewhere a
* member states no termination rule at all, and where an array is exactly as wide as
* the longest value it carries there is no byte left for a terminator -
* CellularUICCSlotInfoStruct.iccid is in that position. A caller reads each array
* under the rule stated on the member it is reading, not under a rule generalised from
* this structure. This interface does not state which members an implementation
* requires, nor what it does with a member left zeroed, so a caller should populate the
* whole structure rather than rely on a default.
*/
typedef struct
{
    int ProfileID;                 /*!< Identifier of this profile. CELLULAR_PROFILE_ID_UNKNOWN (-1) where no
                                    *   identifier is present. This interface does not state whether the caller
                                    *   or the implementation assigns the value on creation. */
    CellularProfileType_t ProfileType;  /*!< Standards family of the profile - 3GPP or 3GPP2. */

    int PDPContextNumber;           /*!< Packet data protocol context number this profile occupies.
                                     *   CELLULAR_PDP_CONTEXT_UNKNOWN (-1) where no context is present. This
                                     *   interface states no valid range for the number. */
    CellularPDPType_t PDPType;       /*!< Address type the context requests - IPv4, IPv6, either, or PPP. */
    CellularPDPAuthentication_t PDPAuthentication; /*!< Authentication method for the context. Where it is not
                                     *   CELLULAR_PDP_AUTHENTICATION_NONE, Username and Password carry the credential. */
    CellularPDPNetworkConfig_t PDPNetworkConfig;  /*!< Whether the address configuration comes from the bearer
                                     *   signalling or from DHCP over the bearer. */

    char ProfileName[64];          /*!< Vendor-specific profile name, at most 63 characters. The string is
                                    *   zero-terminated and the terminator is included in the size. This
                                    *   interface does not require the name to be unique. */
    char APN[64];                   /*!< Access Point Name the context attaches to, at most 63 characters, as a
                                     *   dot-separated label string such as "internet.example.net". The string is
                                     *   zero-terminated and the terminator is included in the size. Vendor and
                                     *   operator specific; this interface states no syntax for it. */
    char Username[256];             /*!< Authentication user name, at most 255 characters, used where
                                     *   PDPAuthentication is PAP or CHAP. The string is zero-terminated and the
                                     *   terminator is included in the size. It is a credential: a caller must not
                                     *   log it. */
    char Password[256];             /*!< Authentication password, at most 255 characters, used where
                                     *   PDPAuthentication is PAP or CHAP. The string is zero-terminated and the
                                     *   terminator is included in the size. It is a credential: a caller must not
                                     *   log it, and this interface does not state whether an implementation
                                     *   returns it through cellular_hal_get_profile_list() or clears it. */
    char Proxy[45];                 /*!< Proxy server address as text, at most 44 characters. The string is
                                     *   zero-terminated and the terminator is included in the size. Any IPv4
                                     *   dotted-decimal address fits; note that 45 bytes is one short of
                                     *   INET6_ADDRSTRLEN (46), so the longest IPv6 text form cannot be stored
                                     *   here with its terminator. Vendor-specific. */
    unsigned int ProxyPort;        /*!< Proxy server TCP port. A port number occupies 16 bits, so only the values
                                    *   0 to 65535 are meaningful in this 32-bit member; this interface does not
                                    *   state what an implementation does with a larger value, and does not
                                    *   define a sentinel for "no proxy". Vendor-specific. */

    unsigned char bIsNoRoaming;     /*!< TRUE (1) to forbid this profile from being used while roaming, FALSE (0)
                                     *   to allow it. */
    unsigned char bIsAPNDisabled;    /*!< TRUE (1) if the Access Point Name is disabled and the profile must not
                                      *   be activated, FALSE (0) otherwise. */
    unsigned char bIsThisDefaultProfile; /*!< TRUE (1) if this is the default profile, which is the profile
                                      *   cellular_hal_start_network() uses when it is passed no profile, FALSE (0)
                                      *   otherwise. This interface does not state what happens if more than one
                                      *   profile carries the flag. */
} CellularProfileStruct;

/**
* @brief Initialization input for cellular_hal_init().
*
* The whole of the configuration this interface takes at initialization: a preferred
* IP family, a preferred radio access technology, and one profile carried by value.
* A caller constructs and owns the structure; cellular_hal_init() reads it during the
* call. Whether the implementation retains the pointer afterwards is not specified by
* this interface, so a caller must keep the structure allocated and unmodified while
* it uses the interface rather than release or reuse the memory once that call has
* returned.
*/
typedef struct
{
    CellularIpFamilyPref_t enIPFamilyPreference; /*!< Preferred IP family. A preference only - the family a
                                                  *   network is started for is chosen by the argument to
                                                  *   cellular_hal_start_network(). */
    CellularProfileStruct stIfInput;             /*!< Interface profile, embedded by value rather than by
                                                  *   pointer, so the caller fills it in place. This interface
                                                  *   does not state whether the profile is created at
                                                  *   initialization or merely recorded. */
    CellularPrefAccessTechnology_t enPreferenceTechnology; /*!< Preferred radio access technology. A preference
                                                  *   only: this interface does not state what an implementation
                                                  *   does when it is unavailable. */
} CellularContextInitInputStruct;

/**
* @brief IP family of a packet data network.
*
* Supplied by a caller to cellular_hal_start_network() and
* cellular_hal_stop_network(), carried in the IPType member of CellularIPStruct, and
* reported as the ip_type argument of
* cellular_network_packet_service_status_api_callback. It is distinct from
* CellularPDPType_t, which selects the address type of a profile.
*/
typedef enum _CellularNetworkIPType_t
{
    CELLULAR_NETWORK_IP_FAMILY_UNKNOWN = 0,   /*!< The family is not known. As an argument this interface does not state which family an implementation then acts on. */
    CELLULAR_NETWORK_IP_FAMILY_IPV4,         /*!< IPv4. */
    CELLULAR_NETWORK_IP_FAMILY_IPV6,         /*!< IPv6. */
    CELLULAR_NETWORK_IP_FAMILY_UNSPECIFIED   /*!< No family is specified. Distinct from UNKNOWN only in name; this interface does not state how the two differ in effect. */
} CellularNetworkIPType_t;

/**
* @brief A complete IP configuration for a started packet data network.
*
* Delivered to cellular_device_network_ip_ready_api_callback once the bearer has an
* address. The structure is supplied by the implementation and this interface does
* not state how long it stays valid after the callback returns, so a caller that
* needs any of it later copies it rather than storing the pointer.
*
* Address members are text, and the family they are written in follows the IPType
* member: an IPv4 configuration writes dotted-decimal text and an IPv6
* configuration writes colon-separated hexadecimal text, including the
* IPv4-mapped form. The 128 bytes of each address member therefore accommodate
* either family comfortably - INET6_ADDRSTRLEN is 46 - and every member below states
* its own capacity and its own termination rule, which for each of them leaves the
* terminator room inside the declared size.
*/
typedef  struct
{
    char WANIFName[16];                       /*!< Name of the WAN network interface the bearer is presented on,
                                               *   at most 15 characters, which is exactly the Linux interface
                                               *   name limit (IFNAMSIZ is 16, terminator included). The string is
                                               *   zero-terminated and the terminator is included in the size.
                                               *   Example: "wwan0" */

    char IPAddress[128];                      /*!< IP address assigned to the device, as text in the family given
                                               *   by IPType: dotted-decimal for IPv4, colon-separated
                                               *   hexadecimal for IPv6. The string is zero-terminated and the
                                               *   terminator is included in the size. Examples: "192.168.1.10",
                                               *   "2001:db8::a" */

    CellularNetworkIPType_t IPType;           /*!< Family the address members are written in. It selects how a
                                               *   caller parses them, so a caller reads this member before any of
                                               *   the address members. */

    char SubnetMask[128];                     /*!< Mask of the local subnet, as text. For an IPv4 configuration
                                               *   this is a dotted-decimal mask such as "255.255.255.0". For an
                                               *   IPv6 configuration this interface does not state whether the
                                               *   member carries a prefix length or an address-form mask, so a
                                               *   caller must accept either. The string is zero-terminated and
                                               *   the terminator is included in the size. */

    char DefaultGateWay[128];                 /*!< Address of the default gateway, as text in the family given by
                                               *   IPType. The string is zero-terminated and the terminator is
                                               *   included in the size. Examples: "192.168.1.1", "fe80::1" */

    char DNSServer1[128];                     /*!< Address of the primary name server, as text in the family
                                               *   given by IPType. The string is zero-terminated and the
                                               *   terminator is included in the size. Examples: "8.8.8.8",
                                               *   "2001:4860:4860::8888" */

    char DNSServer2[128];                     /*!< Address of the secondary name server, on the same terms as
                                               *   DNSServer1. This interface does not state how an
                                               *   implementation reports that no second server was supplied, so
                                               *   a caller should treat an empty string as absent. */

    char Domains[256];                        /*!< Domain search list the device is associated with, at most 255
                                               *   characters. The string is zero-terminated and the terminator is
                                               *   included in the size. This interface does not state the
                                               *   separator used between multiple domains. Example:
                                               *   "hsd.pa.crnrstn.comcast.net" */

    unsigned int MTUSize;                     /*!< Maximum transmission unit of the bearer, in bytes. The range
                                               *   this interface states is 1280 to 9000; the lower bound is the
                                               *   IPv6 minimum link MTU. This interface does not state what a
                                               *   value outside that range means, so a caller should treat one as
                                               *   unusable rather than clamp it. */
} CellularIPStruct;

/**
* @brief Traffic counters and maximum bit rates for a cellular connection.
*
* Filled by cellular_hal_get_packet_statistics() into a structure the caller
* allocates. Every counter is a snapshot read at the moment of the call, and this
* interface states neither the epoch a counter is measured from nor its behaviour on
* overflow, so a caller computing a rate must difference two reads itself and must
* tolerate a counter that has wrapped or been reset beneath it.
*/
typedef struct
{
    unsigned long BytesSent;          /*!< Bytes sent over the connection since the counter's unspecified epoch. */
    unsigned long BytesReceived;       /*!< Bytes received over the connection since the counter's unspecified epoch. */
    unsigned long PacketsSent;        /*!< Packets sent over the connection. */
    unsigned long PacketsReceived;     /*!< Packets received over the connection. */
    unsigned long PacketsSentDrop;    /*!< Packets discarded on transmission rather than sent. This interface does not enumerate the reasons. */
    unsigned long PacketsReceivedDrop; /*!< Packets discarded on reception rather than delivered. This interface does not enumerate the reasons. */
    unsigned long UpStreamMaxBitRate;  /*!< Maximum upstream bit rate, in bit/s. This interface does not state
                                        *   whether the value is the rate negotiated for the bearer or the
                                        *   theoretical maximum of the radio technology in use. */
    unsigned long DownStreamMaxBitRate; /*!< Maximum downstream bit rate, in bit/s, on the same terms as
                                         *   UpStreamMaxBitRate. */
} CellularPacketStatsStruct;


/**
* @brief Physical form factor of a Universal Integrated Circuit Card.
*
* Carried in the FormFactor member of CellularUICCSlotInfoStruct. It describes the
* card's physical size only and says nothing about the applications it carries,
* which CellularUICCApplication_t reports.
*/
typedef enum _CellularUICCFormFactor_t {
    CELLULAR_UICC_FORM_FACTOR_1FF = 0, /*!< 1FF, the original full-size card. */
    CELLULAR_UICC_FORM_FACTOR_2FF,    /*!< 2FF, the mini card, commonly called a SIM card. */
    CELLULAR_UICC_FORM_FACTOR_3FF,    /*!< 3FF, the micro card. */
    CELLULAR_UICC_FORM_FACTOR_4FF     /*!< 4FF, the nano card. This interface publishes no member for a soldered MFF2 or embedded card, whose application type is reported as CELLULAR_UICC_APPLICATION_ESIM instead. */
} CellularUICCFormFactor_t;

/**
* @brief Usability of a Universal Integrated Circuit Card.
*
* Reported by cellular_hal_get_active_card_status() and carried in the Status member
* of CellularUICCSlotInfoStruct. Only CELLULAR_UICC_STATUS_VALID indicates a card
* that can be used; the other three are reported through a successful call, so a
* caller must inspect the value rather than rely on the call's status code.
*/
typedef enum _CellularUICCStatus_t
{
    CELLULAR_UICC_STATUS_VALID = 0,   /*!< The card is present, readable and usable. */
    CELLULAR_UICC_STATUS_BLOCKED,     /*!< The card is blocked, for example by exhausted PIN or PUK attempts. This interface declares no call that unblocks it. */
    CELLULAR_UICC_STATUS_ERROR,       /*!< The card is in an error state. This interface does not enumerate the causes. */
    CELLULAR_UICC_STATUS_EMPTY        /*!< The card carries no data, or no card is present in the selected slot. */
} CellularUICCStatus_t;

/**
* @brief Application a Universal Integrated Circuit Card presents.
*
* Carried in the Application member of CellularUICCSlotInfoStruct. Only one member
* is reported per slot, so where a card carries more than one application this
* interface does not state which is named.
*/
typedef enum _CellularUICCApplication_t {
    CELLULAR_UICC_APPLICATION_USIM = 0, /*!< Universal Subscriber Identity Module, the 3G, 4G and 5G access application. */
    CELLULAR_UICC_APPLICATION_ISIM,    /*!< IP Multimedia Services Identity Module, the IMS application. */
    CELLULAR_UICC_APPLICATION_ESIM    /*!< Embedded Subscriber Identity Module. Profile management for an eSIM is not part of this interface; it is the subject of the separate LPA HAL. */
} CellularUICCApplication_t;

/**
* @brief State and identity of one Universal Integrated Circuit Card slot.
*
* Filled by cellular_hal_get_uicc_slot_info() into a structure the caller allocates,
* for the slot index the caller names. A slot may be enabled with no card present, so
* a caller reads IsCardPresent before treating the card members as meaningful.
*/
typedef struct
{
    unsigned char SlotEnable;        /*!< TRUE (1) if the slot is enabled, FALSE (0) if it is not. */
    unsigned char IsCardPresent;     /*!< TRUE (1) if a card is physically present in the slot, FALSE (0) if not.
                                      *   Where it is FALSE the remaining card members do not describe a card. */
    unsigned char CardEnable;        /*!< TRUE (1) if the card in the slot is enabled, FALSE (0) if it is not.
                                      *   Card power is controlled by cellular_hal_sim_power_enable(). */
    CellularUICCFormFactor_t FormFactor; /*!< Physical form factor of the card in the slot. */
    CellularUICCApplication_t Application; /*!< Application the card presents. */
    CellularUICCStatus_t Status;        /*!< Usability of the card. Only CELLULAR_UICC_STATUS_VALID means usable. */
    char MnoName[32];                 /*!< Mobile network operator name held on the card, in a member of 32
                                       *   bytes. This interface does not state whether the value is
                                       *   zero-terminated - a 32-character name would occupy the member with
                                       *   no byte left for a terminator - so a caller must bound every read by
                                       *   the member's declared size and must not assume a terminator. This
                                       *   interface does not state the encoding or the language. */
    char iccid[20];                   /*!< Integrated Circuit Card Identifier of the card. The member holds 20
                                       *   bytes and no length accompanies it, and an ICCID is up to 20 digits
                                       *   (ITU-T E.118), so the two do not both fit: 20 digits occupy the member
                                       *   exactly and leave no room for a terminator. This member is therefore
                                       *   not guaranteed to be a C string. 'Memory Model' in
                                       *   docs/pages/halSpec.md states the position this member is the extreme
                                       *   case of: a termination obligation exists only where a declaration
                                       *   states one, and this declaration states none. A caller must read it as
                                       *   a fixed-width field of the member's own size - copying at most that
                                       *   many bytes and terminating the copy in its own buffer - and must not
                                       *   pass it to a function that expects a terminated string, because a full
                                       *   member has none. Where the identifier is 19 digits or shorter an
                                       *   implementation can terminate it within the member, but this interface
                                       *   states no way to tell the two cases apart, so a caller must not assume
                                       *   the value is terminated and must not assume it is complete.
                                       *   cellular_hal_get_modem_current_iccid() is the call that returns an
                                       *   identifier in full, into a caller buffer of at least 21 bytes. */
    char msisdn[20];                  /*!< Mobile Subscriber ISDN number of the subscription, in a member of 20
                                       *   bytes. An MSISDN is at most 15 digits (3GPP TS 23.003 clause 3.3), so
                                       *   a full-length number occupies at most 15 of them. This interface does
                                       *   not state whether the value is zero-terminated, so a caller must
                                       *   bound every read by the member's declared size and must not assume a
                                       *   terminator. It identifies a subscriber: a caller must not log it. */
} CellularUICCSlotInfoStruct;

/**
* @brief Radio signal measurements for the serving cell.
*
* Filled by cellular_hal_get_signal_info() into a structure the caller allocates.
* Every member is a signed integer, and the unit differs by member: RSSI, RSRP and
* TXPower are absolute powers in dBm, while RSRQ and SNR are ratios in dB. A power
* in dBm is negative for a received signal, and a value closer to zero is the
* stronger one - -60 dBm is a better signal than -95 dBm.
*
* Every member is a whole number because the type is int. Where a quantity is
* reported by the radio in fractional steps this interface does not state whether an
* implementation rounds, truncates or scales it, so a caller must not assume
* sub-unit resolution. The ranges below are the ones this interface states, together
* with the standards definition of each quantity; this interface specifies no
* behaviour for a value outside them and defines no sentinel for "not measured", so
* a caller cannot distinguish an unmeasured member from a measured zero.
*/
typedef  struct
{
    int RSSI;                     /*!< Received signal strength, in dBm, over the receiver's whole measurement
                                   *   bandwidth. The window this interface states runs from -90 dBm, a weak
                                   *   signal, to -30 dBm, a strong one; a value closer to zero is stronger. */

    int RSRQ;                     /*!< Reference Signal Received Quality, in dB. It is a ratio - reference signal
                                   *   power against the total received power on the same resource elements
                                   *   (3GPP TS 36.214 clause 5.1.3) - so its unit is dB and not dBm. The window
                                   *   this interface states runs from -19.5 dB to -3 dB, which is the 3GPP TS
                                   *   36.133 reporting range; that range is defined in 0.5 dB steps, and a half
                                   *   step is not representable in this integer member. */

    int RSRP;                     /*!< Reference Signal Received Power, in dBm: the linear average power of the
                                   *   resource elements carrying the cell-specific reference signal (3GPP TS
                                   *   36.214 clause 5.1.1). The window this interface states runs from -140 dBm
                                   *   to -44 dBm, which is the 3GPP TS 36.133 reporting range. */

    int SNR;                      /*!< Signal-to-noise ratio, in dB. It is a ratio of two powers, so its unit is
                                   *   dB and not dBm, and unlike the members above it may be positive. The
                                   *   window this interface states runs from -20 dB to 30 dB. This interface
                                   *   does not state whether interference is included in the noise term, so a
                                   *   caller must not read the value as a signal-to-interference-plus-noise
                                   *   ratio, nor assume a measurement bandwidth or averaging period. */

    int TXPower;                  /*!< Power the device is transmitting at, in dBm. This is an emitted power, so
                                   *   it is not negative in the window this interface states, which runs from 0
                                   *   dBm to 30 dBm. This interface does not state the measurement point the
                                   *   figure is taken at, so a caller must not treat it as a conducted antenna
                                   *   power. */
} CellularSignalInfoStruct;

/**
* @brief Cell and band identifiers for the current radio location.
*
* Filled by cellular_hal_get_cell_location_info() into a structure the caller
* allocates. Every member is an unsigned int, so each carries a whole number in the
* range 0 to UINT_MAX, which is 0 to 4294967295 (0xFFFFFFFF) on every platform this
* header targets. This interface defines no sentinel for "not known", so a caller
* cannot distinguish an unpopulated member from a genuine zero.
*/
typedef struct
{
    unsigned int globalCellId;  /*!< Identifier of the cell the device is on, in the range 0 to 0xFFFFFFFF. An
                                 *   E-UTRAN Cell Identifier is 28 bits (3GPP TS 36.413) and fits. An NR Cell
                                 *   Identity is 36 bits (3GPP TS 38.413) and does not fit in this member, and
                                 *   this interface does not state how a 5G cell is identified here. Nor does it
                                 *   state whether the value includes the network's own identity: a globally
                                 *   unique cell identity is the cell identifier together with the PLMN, which
                                 *   this member cannot carry on its own. */
    unsigned int bandInfo;      /*!< Frequency band the cell operates on, in the range this interface states as
                                 *   0 to 0x7FFF. This interface does not state which numbering scheme the value
                                 *   uses - an E-UTRA operating band, an NR operating band and a vendor code are
                                 *   all consistent with the declaration - so a caller cannot map it to a
                                 *   frequency without agreeing the scheme with the implementation. */
    unsigned int servingCellId; /*!< Identifier of the serving cell, on the same terms and with the same 32-bit
                                 *   limit as globalCellId. This interface does not state how the two differ, so
                                 *   a caller must not assume that an inequality between them is meaningful. */
} CellLocationInfoStruct;

/**
* @brief Identity and registration state of the serving network.
*
* Filled by cellular_hal_get_current_plmn_information() into a structure the caller
* allocates. A caller reads registration_status first: where the device is not
* registered, the remaining members do not describe a serving network.
*/
typedef struct
{
    char plmn_name[32];                 /*!< Name of the Public Land Mobile Network, in a member of 32 bytes.
                                         *   This interface does not state whether the value is zero-terminated,
                                         *   so a caller must bound every read by the member's declared size and
                                         *   must not assume a terminator. It does not state whether the value
                                         *   is the long name, the short name or the name held on the card. */
    unsigned int MCC;                  /*!< Mobile Country Code, a three-digit number in the range 0 to 999
                                        *   (3GPP TS 23.003 clause 12.1). Because the member is a number rather
                                        *   than text, a code with a leading zero such as 001 is indistinguishable
                                        *   from 1, and a caller that renders it must pad it to three digits. */
    unsigned int MNC;                  /*!< Mobile Network Code, a two-digit or three-digit number in the range 0
                                        *   to 999. This interface does not state which of the two lengths applies
                                        *   to a given value, and a leading zero is not recoverable from the
                                        *   number, so a caller cannot reconstruct the printed PLMN identity from
                                        *   this member alone. */
    CellularRegistrationStatus_t registration_status; /*!< Whether the device is registered on this network. */
    CellularModemRegisteredServiceType_t registered_service; /*!< Which domains the registration covers. Packet
                                        *   data is carried only where the packet-switched domain is included. */
    unsigned char roaming_enabled;          /*!< TRUE (1) if the device is roaming on this network, FALSE (0) if
                                             *   it is on its home network. */
    unsigned int area_code;              /*!< Location or tracking area code of the serving cell. This interface
                                          *   does not state which of the two it is, nor its width: a GSM or UMTS
                                          *   location area code and an E-UTRAN tracking area code are 16 bits,
                                          *   while an NR tracking area code is 24 bits. All three fit the
                                          *   member. */
    unsigned long cell_id;                /*!< Identifier of the serving cell, in the range of an unsigned long,
                                           *   which is at least 32 bits. It is reported separately from the
                                           *   32-bit members of CellLocationInfoStruct and this interface does
                                           *   not state whether the two agree. */
} CellularCurrentPlmnInfoStruct;

/**
* @brief One entry of a network scan result.
*
* An element of the array cellular_hal_get_available_networks_information() reports.
* A scan describes networks the device can see, not networks it is registered on, so
* nothing here implies that an attach would succeed.
*/
typedef struct
{
    char network_name[32];  /*!< Name of the network, in a member of 32 bytes. This interface does not state
                             *   whether the value is zero-terminated, so a caller must bound every read by the
                             *   member's declared size and must not assume a terminator. */
    unsigned int MCC;       /*!< Mobile Country Code of the network, a three-digit number in the range 0 to 999.
                             *   As in CellularCurrentPlmnInfoStruct, a leading zero is not recoverable from the
                             *   number. */
    unsigned int MNC;       /*!< Mobile Network Code of the network, a two-digit or three-digit number in the
                             *   range 0 to 999, on the same terms. */
    unsigned char network_allowed_flag; /*!< TRUE (1) if the device is permitted to use this network, FALSE (0)
                                         *   if it is not. This interface does not state what forbids it - a
                                         *   card policy, an operator policy or a regulatory one. */
} CellularNetworkScanResultInfoStruct;

/**
* @brief Whether the modem device has appeared or been removed.
*
* Delivered as the second argument to cellular_device_removed_status_api_callback.
*/
typedef enum _CellularDeviceDetectionStatus_t {
    DEVICE_DETECTED = 1, /*!< The modem device has been detected. */
    DEVICE_REMOVED     /*!< The modem device has been removed. Calls into this interface that need the device should be expected to fail until it is detected again. */
} CellularDeviceDetectionStatus_t;

/**
* @brief Progress of opening the modem device context.
*
* Delivered as the third argument to cellular_device_open_status_api_callback,
* installed by cellular_hal_open_device(). DEVICE_OPEN_STATUS_INPROGRESS is not
* terminal, so a caller must expect a further notification after it.
*/
typedef enum _CellularDeviceOpenStatus_t {
    DEVICE_OPEN_STATUS_NOT_READY = 1, /*!< The device cannot be opened yet. */
    DEVICE_OPEN_STATUS_INPROGRESS,   /*!< Opening is under way and has not completed. */
    DEVICE_OPEN_STATUS_READY,        /*!< The device context is open and the modem control interface is usable. */
} CellularDeviceOpenStatus_t;

/**
* @brief Progress of selecting a card slot.
*
* Delivered as the fourth argument to cellular_device_slot_status_api_callback,
* installed by cellular_hal_select_device_slot(). DEVICE_SLOT_STATUS_SELECTING is
* not terminal.
*/
typedef enum _CellularDeviceSlotStatus_t
{
    DEVICE_SLOT_STATUS_NOT_READY = 1, /*!< The slot is not ready for use. */
    DEVICE_SLOT_STATUS_SELECTING,   /*!< Selection of the slot is under way and has not completed. */
    DEVICE_SLOT_STATUS_READY,        /*!< The slot is selected and the card in it is usable. */
} CellularDeviceSlotStatus_t;

/**
* @brief Network registration state reported by the Network Attachment Subsystem.
*
* Delivered as the first argument to cellular_device_registration_status_callback,
* installed by cellular_hal_monitor_device_registration(). DEVICE_NAS_STATUS_REGISTERING
* is not terminal, and this interface does not state whether registration is
* retried indefinitely or eventually abandoned.
*/
typedef enum _CellularDeviceNASStatus_t
{
    DEVICE_NAS_STATUS_NOT_REGISTERED = 1, /*!< The device is not registered on any network. */
    DEVICE_NAS_STATUS_REGISTERING,        /*!< Registration is under way and has not completed. */
    DEVICE_NAS_STATUS_REGISTERED        /*!< The device is registered. The domains covered are reported separately, as a CellularModemRegisteredServiceType_t. */
} CellularDeviceNASStatus_t;

/**
* @brief Whether the current registration is a roaming one.
*
* Delivered as the second argument to cellular_device_registration_status_callback.
*/
typedef enum _CellularDeviceNASRoamingStatus_t {
    DEVICE_NAS_STATUS_ROAMING_OFF = 1, /*!< The device is registered on its home network. */
    DEVICE_NAS_STATUS_ROAMING_ON,      /*!< The device is roaming. A profile whose bIsNoRoaming flag is set must not be activated in this state. */
} CellularDeviceNASRoamingStatus_t;

/**
* @brief Whether a Packet Data Protocol context profile is active.
*
* Declared for completeness of the profile vocabulary. No declaration in this
* interface accepts or returns it, and no call reports a profile's active state, so
* a caller cannot obtain this value through this interface.
*/
typedef enum _CellularContextProfileStatus_t {
    PROFILE_STATUS_INACTIVE = 1, /*!< The profile exists and no context is active on it. */
    PROFILE_STATUS_ACTIVE       /*!< A context is active on the profile. */
} CellularContextProfileStatus_t;

/**
* @brief Outcome of a profile create, modify or delete operation.
*
* Delivered as the third argument to cellular_device_profile_status_api_callback,
* which all three profile calls install. The same type reports all three operations,
* so a caller correlates a notification with the operation it issued rather than
* reading the operation from the value.
*/
typedef enum _CellularDeviceProfileSelectionStatus_t {
    DEVICE_PROFILE_STATUS_NOT_READY = 1, /*!< The profile cannot be acted on yet. */
    DEVICE_PROFILE_STATUS_CONFIGURING,  /*!< The profile is being configured; the operation has not completed. */
    DEVICE_PROFILE_STATUS_READY,       /*!< The profile is configured and can carry a packet data network. */
    DEVICE_PROFILE_STATUS_DELETED     /*!< The profile has been deleted and must not be used again. */
} CellularDeviceProfileSelectionStatus_t;

/**
* @brief Whether the IP configuration of a started network is ready to use.
*
* Delivered as the second argument to
* cellular_device_network_ip_ready_api_callback, installed by
* cellular_hal_start_network().
*/
typedef enum _CellularDeviceIPReadyStatus_t {
    DEVICE_NETWORK_IP_NOT_READY = 1, /*!< No usable IP configuration is available. The accompanying CellularIPStruct must not be relied on. */
    DEVICE_NETWORK_IP_READY        /*!< The accompanying CellularIPStruct carries a usable IP configuration. */
} CellularDeviceIPReadyStatus_t;

/**
* @brief Whether packet service is carrying traffic.
*
* Delivered as the third argument to
* cellular_network_packet_service_status_api_callback, installed by
* cellular_hal_start_network(). It reports the bearer, which is a separate fact from
* whether an IP configuration is ready: a caller waits on both notifications.
*/
typedef enum _CellularNetworkPacketStatus_t
{
    DEVICE_NETWORK_STATUS_DISCONNECTED = 1, /*!< Packet service is not connected and no traffic is carried. */
    DEVICE_NETWORK_STATUS_CONNECTED      /*!< Packet service is connected. */
} CellularNetworkPacketStatus_t;

/** @} */  //END OF GROUP CELLULAR_HAL_TYPES
/**
 * @addtogroup CELLULAR_HAL_APIS
 * @{
 */

/**********************************************************************
                FUNCTION PROTOTYPES
**********************************************************************/

/**
* @brief Reports whether a modem device is present.
*
* Reads presence only - it neither opens the device nor initializes anything - and it
* is the counterpart of cellular_hal_IsModemControlInterfaceOpened(), which reports
* whether the control interface to that device has been opened. It is a call an
* initialized caller uses to check whether the hardware is there, for instance while
* waiting for the modem to come back after cellular_hal_modem_reset(); it is not a
* discovery step that precedes initialization, because this interface requires
* initialization first and excepts no call from that requirement - see @pre.
*
* @return Whether a modem device is present, as one of this header's two boolean
*         values. TRUE (1) means a modem device is present; FALSE (0) means it is
*         not. This call reports a value rather than a status: it uses the
*         TRUE / FALSE convention and not the RETURN_OK / RETURN_ERROR convention
*         the rest of this interface uses, so a caller must not compare its result
*         against RETURN_OK - which shares the numeric value of FALSE - and must
*         not treat any value as an error indication. This interface provides no way
*         to distinguish "no modem is present" from "presence could not be
*         determined"; both are reported as FALSE.
*
* @pre cellular_hal_init() has returned RETURN_OK. 'Initialization and Startup' and
*      'Method Sequencing' in docs/pages/halSpec.md state one ordering rule for this
*      interface - cellular_hal_init() is invoked before any other operation - and
*      neither they nor this header excepts this query from it. Nothing here therefore
*      makes this call available as a pre-initialization probe: issuing it before
*      initialization has succeeded is outside what this interface specifies, so the
*      answer may be reported, or the call may fail, and a caller must not build on
*      either outcome.
* @post None. The call reports state and changes none. A TRUE result does not imply
*       that the device has been opened or that any other call will succeed.
*
* @note Blocking: synchronous. The result is the return value; nothing is reported
*       through a callback. No timeout is specified for this call, per 'Blocking
*       calls' in docs/pages/halSpec.md, so a caller that must bound its own latency
*       imposes that bound itself.
* @note Presence can change: the modem may be removed after a TRUE result, which is
*       the condition cellular_device_removed_status_api_callback reports. A caller
*       must not cache the answer.
* @warning Not thread safe. Per 'Threading Model' in docs/pages/halSpec.md the
*          obligation is the caller's: serialise this call against every other
*          Cellular HAL call, including calls issued from another process.
* @see cellular_hal_IsModemControlInterfaceOpened, cellular_hal_open_device
*/
unsigned int cellular_hal_IsModemDevicePresent(void);

/**
* @brief Initializes the Cellular HAL from the caller's context configuration.
*
* This is the entry point every other call in this interface depends on. It brings
* the layer up against the preferred IP family, the preferred access technology and
* the interface profile the caller supplies, so that the device, slot, profile and
* network calls have a context to act on. This interface declares no counterpart:
* there is no deinitialization call, so a caller cannot undo the effect through this
* interface - cellular_hal_stop_network() stops a packet data network and does not
* tear the layer down. How long the initialized state itself lasts is not stated
* here; see @post.
*
* @param[in] pstCtxInputStruct variable is the Input structure to pass to cellular hal initialization function described by the CellularContextInitInputStruct.
*                              \n The caller allocates the structure and retains
*                              ownership of it. The implementation reads it during
*                              the call. Retention is not established: this interface
*                              does not specify whether the implementation keeps the
*                              pointer after the call has returned, and a caller must
*                              not assume either that it does or that it does not.
*                              The conservative consequence, which is what a caller
*                              has to act on, is that the structure stays allocated and
*                              unmodified for as long as the caller uses this interface,
*                              and this interface declares no call that would end that
*                              period. Equally, a caller must not assume that a change
*                              made to the structure after the call reaches the
*                              initialized context. The profile it carries is
*                              embedded by value, so the whole of it is copied by the
*                              caller's own assignment before the call. This
*                              interface does not state whether NULL is accepted, so
*                              a caller must supply a populated structure.
*
* @returns The status of the operation.
* @retval RETURN_OK    (0) - The layer is initialized and the rest of this interface
*                      may be used.
* @retval RETURN_ERROR (-1) - Initialization failed and no other call should be
*                      relied on. This is the interface's only failure code and it
*                      names no cause, so a caller cannot tell from it why
*                      initialization failed and must not infer a cause from it. The
*                      initialization state after a failure is indeterminate: as the
*                      postcondition below records, this interface does not state
*                      whether a partial initialization remains. It also specifies no
*                      cleanup or recovery call that would return the layer to a known
*                      state - it declares no deinitialization entry point at all, and
*                      the two modem-level calls it does declare,
*                      cellular_hal_modem_reset() and
*                      cellular_hal_modem_factory_reset(), are each specified only
*                      after this call has returned RETURN_OK by the precondition on
*                      their own declarations, so neither is available here. No
*                      in-interface recovery is therefore specified: this interface
*                      does not establish that a further attempt starts from a clean
*                      state or that it is safe, and a caller that repeats the call is
*                      acting outside what is specified here. Diagnosis is equally
*                      unavailable, because cellular_hal_IsModemDevicePresent() is
*                      itself placed after a successful initialization by the
*                      precondition on its own declaration. What is left to a caller
*                      is a decision it takes on its own authority, per 'Internal
*                      Error Handling' in docs/pages/halSpec.md, and not one this
*                      interface sanctions.
*
* @pre None. This call is the precondition of the others, not the other way round.
*      'Initialization and Startup' and 'Method Sequencing' in docs/pages/halSpec.md
*      state the rule as cellular_hal_init() being invoked before any other operation,
*      and state no exception to it - the two boolean queries,
*      cellular_hal_IsModemDevicePresent() and
*      cellular_hal_IsModemControlInterfaceOpened(), are covered by that rule like
*      every other call. Issuing any other call in this interface before this one has
*      returned RETURN_OK is therefore outside what this interface specifies: it may
*      fail, or it may act on an uninitialized context, and a caller must not do it.
* @post On success the layer is initialized and the rest of the interface may be
*       used. This interface declares no call that deinitializes it, so a caller has
*       no way to end that state; it does not state how long the state lasts, nor
*       whether it survives a modem reset or a factory reset, so a caller must not
*       assume that one call is good for the life of the process and must not assume
*       the opposite either. On failure this interface does not state whether a
*       partial initialization remains, so a caller must not assume the layer is
*       untouched, and must not treat a second call as a clean retry of the first.
*
* @note Blocking: synchronous, and this is the one call in the interface with a
*       stated blocking behaviour - per 'Blocking calls' in docs/pages/halSpec.md it
*       is expected to block while the modem hardware is not ready, which at boot is
*       exactly when a caller invokes it. No timeout is specified.
* @warning Not thread safe. Per 'Threading Model' in docs/pages/halSpec.md the
*          obligation is the caller's: complete this call before any other call in
*          this interface is issued from any thread or process, and serialise the
*          calls that follow.
* @see cellular_hal_open_device, cellular_hal_start_network
*/
int cellular_hal_init(CellularContextInitInputStruct *pstCtxInputStruct);

/** @} */  //END OF GROUP CELLULAR_HAL_APIS
/**
* @addtogroup CELLULAR_HAL_TYPES
* @{
*/

/**
* @brief Callback type reporting progress in opening the modem device context.
*
* Installed as the device_open_status_cb member of the
* CellularDeviceContextCBStruct passed to cellular_hal_open_device(), and invoked by
* the implementation as the open proceeds. DEVICE_OPEN_STATUS_INPROGRESS is not
* terminal, so an implementation of this type must tolerate being called more than
* once for one open.
*
* @param[in] device_name variable is a character pointer points the modem device name.
*                        \n It is a vendor specific value.
*                        \n The implementation supplies the buffer. This interface
*                        does not state its size, nor how long it remains valid
*                        after the callback returns, so an implementation of this
*                        callback must treat it as read-only and borrowed: copy what
*                        it needs and never store the pointer or free it.
* @param[in] wan_ifname variable is a character pointer points the WAN interface name.
*                       \n Example: "wwan0"
*                       \n Supplied and owned by the implementation on the same terms
*                       as device_name. The name is the interface the bearer will be
*                       presented on, and matches the WANIFName member of
*                       CellularIPStruct.
* @param[in] device_open_status variable is from the enumerated datatype.
*                              \n It can hold any value from enum CellularDeviceOpenStatus_t.
*                              \n Only DEVICE_OPEN_STATUS_READY reports that the
*                              control interface is usable.
* @param[in] modem_operating_mode variable is from the enumerated datatype.
*                                \n It can hold any value from enum CellularModemOperatingConfiguration_t.
*                                \n The mode the modem is in as the open progresses.
*
* @returns The status the callback reports back to the implementation.
* @retval RETURN_OK    (0) - The notification was accepted.
* @retval RETURN_ERROR (-1) - The notification was not accepted. This interface does
*                      not state what an implementation does with this value, so a
*                      callback must not rely on returning it to suppress, retry or
*                      reorder a notification.
*
* @pre cellular_hal_open_device() has been called with this function installed. A
*      callback is reached no other way: this interface declares no registration
*      call and no means of removing one once installed.
* @post None on the interface's side. Whatever the callback does with the
*       notification is the caller's own business, and this interface states no
*       effect of the return value.
*
* @note Blocking: this callback runs on the implementation's thread, so an
*       implementation of it that blocks delays the HAL. It should return promptly
*       and do its work elsewhere. No deadline is specified.
* @warning Thread safety is the callback author's obligation. This interface does not
*          state the thread a callback arrives on, and 'Threading Model' in
*          docs/pages/halSpec.md permits a vendor to use its own threads, so a
*          callback must assume it is not the thread that installed it and must
*          synchronise access to any state it shares. It must not call back into this
*          interface without serialising against the caller's other Cellular HAL
*          calls.
* @see cellular_hal_open_device, cellular_device_removed_status_api_callback
*/
typedef int (*cellular_device_open_status_api_callback)( char *device_name, char *wan_ifname, CellularDeviceOpenStatus_t device_open_status, CellularModemOperatingConfiguration_t modem_operating_mode );

/**
* @brief Callback type reporting that the modem device has been detected or removed.
*
* Installed as the device_remove_status_cb member of the
* CellularDeviceContextCBStruct passed to cellular_hal_open_device(). It reports
* physical presence, so a caller that has to survive a modem being removed at
* runtime handles it here rather than by polling
* cellular_hal_IsModemDevicePresent().
*
* @param[in] device_name variable is a character pointer points the modem device name.
*                        \n It is a vendor specific value.
*                        \n The implementation supplies the buffer and this interface
*                        states neither its size nor its lifetime after the callback
*                        returns. Treat it as read-only and borrowed: copy what is
*                        needed, never store the pointer and never free it.
* @param[in] device_detection_status variable is from the enumerated datatype.
*                                   \n It can hold any value from enum CellularDeviceDetectionStatus_t.
*                                   \n DEVICE_REMOVED means calls that need the
*                                   device should be expected to fail until a
*                                   DEVICE_DETECTED notification follows.
*
* @returns The status the callback reports back to the implementation.
* @retval RETURN_OK    (0) - The notification was accepted.
* @retval RETURN_ERROR (-1) - The notification was not accepted. This interface does
*                      not state what an implementation does with this value.
*
* @pre cellular_hal_open_device() has been called with this function installed.
* @post None on the interface's side. Removal is reported, not prevented: returning
*       either value does not affect the device's state.
*
* @note Blocking: this callback runs on the implementation's thread and should
*       return promptly rather than performing recovery inline. No deadline is
*       specified.
* @warning Thread safety is the callback author's obligation, on the same terms as
*          cellular_device_open_status_api_callback: the arrival thread is not
*          specified, so shared state must be synchronised.
* @see cellular_hal_open_device, cellular_hal_IsModemDevicePresent
*/
typedef int (*cellular_device_removed_status_api_callback)( char *device_name, CellularDeviceDetectionStatus_t device_detection_status );

/**
* @brief Callback container for the device open and device removal notifications.
*
* The single argument of cellular_hal_open_device(). A caller fills both members
* before that call and owns the structure; the implementation reads it during the
* call. This interface does not state whether the function pointers may be NULL, so
* a caller should install both. Three things are absent and none is supplied here:
* this interface declares no call that removes a callback, does not state when
* delivery on an installed one ends, and does not state whether an installed callback
* survives a modem reset or factory reset. A caller therefore has no point at which
* this interface makes invalidating one of these functions safe, and the conservative
* consequence - stated under 'Asynchronous Notification Model' in
* docs/pages/halSpec.md - is that it keeps both functions callable for as long as it
* uses this interface. That follows from the silence and says nothing about the
* implementation: it is neither a guarantee that delivery continues nor a bound on
* how long it may continue.
*/
typedef  struct
{
    cellular_device_open_status_api_callback device_open_status_cb;        /*!< Reports how opening the modem device context proceeds. Receives the
                                                                           *   vendor-specific modem device name, the WAN interface name the bearer is
                                                                           *   presented on (for example "wwan0"), a CellularDeviceOpenStatus_t value and
                                                                           *   a CellularModemOperatingConfiguration_t value. Both strings are borrowed
                                                                           *   from the implementation for the duration of the call only. */

    cellular_device_removed_status_api_callback device_remove_status_cb;    /*!< Reports that the modem device has been detected or removed.
                                                                            *   Receives the vendor-specific modem device name and a
                                                                            *   CellularDeviceDetectionStatus_t value. The string is borrowed from
                                                                            *   the implementation for the duration of the call only. */
} CellularDeviceContextCBStruct;

/** @} */  //END OF GROUP CELLULAR_HAL_TYPES
/**
 * @addtogroup CELLULAR_HAL_APIS
 * @{
 */

/**
* @brief Instructs the lower layer to create and open the modem device.
*
* The step between initializing the layer and using the modem: it opens the device
* context and installs the two device-context callbacks through which the
* implementation reports how the open proceeds and whether the modem is later
* removed. The open is not necessarily complete when the call returns - progress is
* reported as a CellularDeviceOpenStatus_t through the installed callback, and only
* DEVICE_OPEN_STATUS_READY says the control interface is usable.
*
* @param[in] pstDeviceCtxCB variable is the structure CellularDeviceContextCBStruct receives function pointers for device open/remove status response from driver.
*                           \n It has two members to open and remove the modem from the device
*                           \n The caller allocates the structure and retains
*                           ownership of it. The implementation reads the two
*                           function pointers during the call. This interface declares
*                           no call that removes a callback, does not state when
*                           delivery on an installed one ends, and does not state
*                           whether an installed callback survives a modem reset or
*                           factory reset, so a caller has no point at which this
*                           interface makes invalidating one of the two functions safe.
*                           The conservative consequence - stated under 'Asynchronous
*                           Notification Model' in docs/pages/halSpec.md - is that a
*                           caller keeps both functions callable for as long as it uses
*                           this interface, which follows from the silence and is
*                           neither a guarantee that delivery continues nor a bound on
*                           how long it may continue. This interface does not state
*                           whether NULL, or a structure with a NULL member, is
*                           accepted.
*
* @returns The status of the operation.
* @retval RETURN_OK    (0) - The open has been accepted and the callbacks are
*                      installed. It does not mean the device is open: a caller waits
*                      for DEVICE_OPEN_STATUS_READY, or polls
*                      cellular_hal_IsModemControlInterfaceOpened().
* @retval RETURN_ERROR (-1) - The call failed. This interface does not state whether
*                      the open was begun or the callbacks installed, so the outcome
*                      is unknown rather than undone and a caller must not assume
*                      either. This is the interface's only failure code and it names
*                      no cause, so a caller cannot tell from it why the call failed
*                      and must not infer a cause from it. The action left to a caller
*                      is to read the state back with
*                      cellular_hal_IsModemControlInterfaceOpened() rather than repeat
*                      the call immediately.
*
* @pre cellular_hal_init() has returned RETURN_OK. Calling this before that is
*      outside what this interface specifies and may act on an uninitialized context.
* @post On success the device open is under way and the two callbacks are installed.
*       This interface states no lifetime for an installed callback: it declares no
*       call that removes one, and does not state how long the implementation may keep
*       notifying or whether an installed callback survives cellular_hal_modem_reset()
*       or cellular_hal_modem_factory_reset(), so a caller must not assume that delivery
*       continues and must not assume that it stops. On failure this interface does not
*       state whether the callbacks were
*       installed, so a caller must be prepared to receive a notification even after a
*       failed call.
*
* @note Blocking: synchronous. The call returns a status; the outcome of the open
*       arrives asynchronously through the installed callback. No timeout is
*       specified for either.
* @note This interface declares no close or deinitialize call, so an opened device
*       context cannot be released through this interface.
* @warning Not thread safe. Per 'Threading Model' in docs/pages/halSpec.md the
*          obligation is the caller's: serialise this call against every other
*          Cellular HAL call, including calls issued from another process.
* @see cellular_hal_IsModemControlInterfaceOpened, cellular_hal_select_device_slot
*/
int cellular_hal_open_device(CellularDeviceContextCBStruct *pstDeviceCtxCB);

/**
* @brief Reports whether the modem control interface is open.
*
* The completion test for cellular_hal_open_device() that does not require waiting
* for a callback. A caller that has to know whether the control interface is usable
* before issuing a modem call reads it here.
*
* @return Whether the modem control interface is open, as one of this header's two
*         boolean values. TRUE (1) means it is open; FALSE (0) means it is not. Like
*         cellular_hal_IsModemDevicePresent() this call reports a value rather than a
*         status: it uses the TRUE / FALSE convention and not the
*         RETURN_OK / RETURN_ERROR convention, so a caller must not compare its
*         result against RETURN_OK - which shares the numeric value of FALSE. This interface
*         defines no sentinel for a failed query, so a caller cannot tell a failed call
*         from a genuine value: FALSE covers "not opened", "open still in progress" and
*         "state could not be determined" alike.
*
* @pre cellular_hal_init() has returned RETURN_OK, on exactly the terms stated under
*      cellular_hal_IsModemDevicePresent(): 'Initialization and Startup' and 'Method
*      Sequencing' in docs/pages/halSpec.md state one ordering rule for this interface
*      - cellular_hal_init() is invoked before any other operation - and except this
*      query from it no more than they except any other call, so issuing it before
*      initialization has succeeded is outside what this interface specifies. Beyond
*      that requirement no further ordering is stated: the answer is only informative
*      once cellular_hal_open_device() has been called, but this interface does not
*      require that call to have been issued first.
* @post None. The call reports state and changes none.
*
* @note Blocking: synchronous. No timeout is specified, per 'Blocking calls' in
*       docs/pages/halSpec.md.
* @note The answer can change: a modem removal, reset or factory reset can close the
*       control interface, so a caller must not cache it.
* @warning Not thread safe. Per 'Threading Model' in docs/pages/halSpec.md the
*          obligation is the caller's: serialise this call against every other
*          Cellular HAL call, including calls issued from another process.
* @see cellular_hal_open_device, cellular_hal_IsModemDevicePresent
*/
unsigned char cellular_hal_IsModemControlInterfaceOpened( void );

/** @} */  //END OF GROUP CELLULAR_HAL_APIS
/**
* @addtogroup CELLULAR_HAL_TYPES
* @{
*/

/**
* @brief Callback type reporting progress in selecting a card slot.
*
* Installed directly by cellular_hal_select_device_slot(). DEVICE_SLOT_STATUS_SELECTING
* is not terminal, so an implementation of this type must tolerate being called more
* than once for one selection.
*
* @param[in] slot_name variable is a character pointer points the slot name. The buffer size should be atleast 64 bytes long. It is a vendor specific value.
*                      \n The implementation supplies the buffer and this interface
*                      does not state how long it remains valid after the callback
*                      returns. Treat it as read-only and borrowed: copy what is
*                      needed, never store the pointer and never free it. The 64-byte
*                      figure is the size this interface asks for, matching BUFLEN_64.
*                      This interface does not state whether the string is
*                      zero-terminated within that size, so a caller must bound every
*                      read by it and must not assume a terminator.
* @param[in] slot_type variable is a character pointer points the slot type. The buffer size should be atleast 64 bytes long. It is a vendor specific value.
*                      \n Supplied and owned by the implementation on the same terms
*                      as slot_name. This interface publishes no vocabulary for the
*                      type, so a caller cannot match it against a known set.
* @param[in] slot_num variable is a integer datatype contains the slot num. It is a vendor specific value.
*                     \n The slot the notification concerns. This interface states no
*                     range or numbering for this argument. The only slot numbering it
*                     states anywhere is the 1 or 2 given for
*                     cellular_hal_sim_power_enable()'s slot_id, and it does not state
*                     that this argument uses the same numbering, so a caller must not
*                     pass the value on to that call without confirming the numbering
*                     with the implementation. CELLULAR_SLOT_ID_UNKNOWN (-1) is this
*                     header's sentinel for a slot identifier that is not present.
* @param[in] device_slot_status variable is from the enumerated datatype.
*                              \n It can hold any value from the enum CellularDeviceSlotStatus_t.
*                              \n Only DEVICE_SLOT_STATUS_READY reports that the card
*                              in the slot is usable.
*
* @returns The status the callback reports back to the implementation.
* @retval RETURN_OK    (0) - The notification was accepted.
* @retval RETURN_ERROR (-1) - The notification was not accepted. This interface does
*                      not state what an implementation does with this value.
*
* @pre cellular_hal_select_device_slot() has been called with this function
*      installed.
* @post None on the interface's side; the return value has no stated effect.
*
* @note Blocking: this callback runs on the implementation's thread and should return
*       promptly. No deadline is specified.
* @warning Thread safety is the callback author's obligation: the arrival thread is
*          not specified by this interface, so shared state must be synchronised.
* @see cellular_hal_select_device_slot, cellular_hal_get_uicc_slot_info
*/
typedef int (*cellular_device_slot_status_api_callback)( char *slot_name, char *slot_type, int slot_num, CellularDeviceSlotStatus_t device_slot_status );

/** @} */  //END OF GROUP CELLULAR_HAL_TYPES
/**
 * @addtogroup CELLULAR_HAL_APIS
 * @{
 */

/**
* @brief Instructs the lower layer to select the card slot for the opened device.
*
* Chooses which Universal Integrated Circuit Card slot the modem uses and installs
* the callback that reports how the selection proceeds. Note what the signature does
* not carry: no slot index. The slot to select is not an argument of this call, so
* which slot an implementation chooses is not expressed through this interface - the
* installed callback reports the slot it acted on in its slot_num argument.
*
* @param[in] device_slot_status_cb variable is the function pointer which receives device slot status response from driver.
*                                  \n The members of the function cellular_device_slot_status_api_callback are:
*                                  \n slot_name - variable is a character pointer points the slot name. The buffer size should be atleast 64 bytes long. It is a vendor specific value.
*                                  \n slot_type - variable is a character pointer points the slot type. The buffer size should be atleast 64 bytes long. It is a vendor specific value.
*                                  \n slot_num - variable is a integer datatype contains the slot num. It is a vendor specific value.
*                                  \n device_slot_status - variable is from the enumerated datatype.
*                                  \n It can hold any value from the enum CellularDeviceSlotStatus_t.
*                                  \n The caller owns the function. This interface
*                                  declares no call that removes a callback, does not
*                                  state when delivery on an installed one ends, and
*                                  does not state whether an installed callback survives
*                                  a modem reset or factory reset, so a caller has no
*                                  point at which this interface makes invalidating the
*                                  pointer safe. The conservative consequence - stated
*                                  under 'Asynchronous Notification Model' in
*                                  docs/pages/halSpec.md - is that a caller keeps the
*                                  function callable for as long as it uses this
*                                  interface, which follows from the silence and is
*                                  neither a guarantee that delivery continues nor a
*                                  bound on how long it may continue. This interface
*                                  does not state whether NULL is accepted.
*
* @returns The status of the operation.
* @retval RETURN_OK    (0) - The selection has been accepted and the callback is
*                      installed. It does not mean a slot is ready: a caller waits for
*                      DEVICE_SLOT_STATUS_READY.
* @retval RETURN_ERROR (-1) - The call failed. This interface does not state whether
*                      the selection was begun or the callback installed, so the
*                      outcome is unknown rather than undone and a caller must not
*                      assume either. This is the interface's only failure code and it
*                      names no cause, so a caller cannot tell from it why the call
*                      failed. The action left to a caller is to read the control
*                      interface back with
*                      cellular_hal_IsModemControlInterfaceOpened() and the slot back
*                      with cellular_hal_get_uicc_slot_info() rather than repeat the
*                      request.
*
* @pre cellular_hal_init() has returned RETURN_OK. This interface states no ordering
*      constraint against cellular_hal_open_device() ('Method Sequencing' in
*      docs/pages/halSpec.md), so a caller must not read a failure here as proof that
*      the device is not open.
* @post On success the selection is under way and the callback is installed. This
*       interface states no lifetime for an installed callback - it declares no call
*       that removes one and does not state how long notification may continue or
*       whether it survives a modem reset - so a caller must not assume either that
*       delivery continues or that it stops. On failure
*       this interface does not state whether the callback was installed, so a caller
*       must be prepared to receive a notification even after a failed call.
*
* @note Blocking: synchronous. The call returns a status; the outcome of the
*       selection arrives asynchronously through the installed callback. No timeout is
*       specified for either.
* @warning Not thread safe. Per 'Threading Model' in docs/pages/halSpec.md the
*          obligation is the caller's: serialise this call against every other
*          Cellular HAL call, including calls issued from another process.
* @see cellular_hal_sim_power_enable, cellular_hal_get_uicc_slot_info
*/
int cellular_hal_select_device_slot(cellular_device_slot_status_api_callback device_slot_status_cb);

/**
* @brief Powers the card in one slot on or off.
*
* Controls power to a Universal Integrated Circuit Card, which is how a caller
* activates a card it has selected or quiesces one it is not using. It acts on the
* card, not on the modem: modem power is the business of
* cellular_hal_set_modem_operating_configuration().
*
* @param[in] slot_id variable is a unsigned integer will intimate to lower layer to slot id to be enable/disable
*                    \n The possible values can be 1 or 2 and depends on number of SIM slots provided by vendor.
*                    \n Those two values are the whole of the slot numbering this
*                    interface states, and it states them for this argument alone.
*                    It states no numbering for cellular_hal_get_uicc_slot_info()'s
*                    slot_index, does not state that the two arguments share one
*                    numbering, and does not state how the count reported by
*                    cellular_hal_get_total_no_of_uicc_slots() maps onto either - so
*                    a caller must not derive an identifier from the count and must
*                    not assume that identifiers begin at 0. Nor does this interface
*                    state what an implementation does with an identifier it does not
*                    recognise. A caller that must address a specific slot confirms
*                    the numbering with the implementation.
* @param[in] enable variable is a unsigned character will intimate to lower layer to enable/disable UICC power.
*                   \n TRUE (1) powers the card on, FALSE (0) powers it off. This
*                   interface does not state how any other value is treated, so a
*                   caller must pass one of the two.
*
* @returns The status of the operation.
* @retval RETURN_OK    (0) - The power change has been applied.
* @retval RETURN_ERROR (-1) - The call failed. This interface does not state whether
*                      the power change took effect, so the outcome is unknown rather
*                      than undone: a caller must not read this value as proof that
*                      the slot is unchanged. This is the interface's only failure code
*                      and it names no cause, so a caller cannot tell from it why the
*                      call failed. The action left to a caller is to read the slot
*                      back with cellular_hal_get_uicc_slot_info() rather than retry
*                      the same request.
*
* @pre cellular_hal_init() has returned RETURN_OK, and slot_id names a slot the
*      device has. Which identifiers name a slot is not established beyond the two
*      values stated above, and passing one that names no slot is not specified by
*      this interface.
* @post On success the card in the named slot has been powered as requested; this
*       interface does not state whether the change is immediate or how a caller
*       observes it having taken effect. On failure no change is stated, but a caller
*       must not assume the previous state is intact - it should read the slot back
*       with cellular_hal_get_uicc_slot_info().
*
* @note Blocking: synchronous. No timeout is specified, per 'Blocking calls' in
*       docs/pages/halSpec.md, and powering a card is a hardware operation that may
*       take a perceptible time.
* @note Powering a card off can invalidate the registration and any packet data
*       network running on it. This interface does not state what happens to a
*       started network when its card loses power.
* @warning Not thread safe. Per 'Threading Model' in docs/pages/halSpec.md the
*          obligation is the caller's: serialise this call against every other
*          Cellular HAL call, including calls issued from another process.
* @see cellular_hal_get_total_no_of_uicc_slots, cellular_hal_get_uicc_slot_info
*/
int cellular_hal_sim_power_enable(unsigned int slot_id, unsigned char enable);

/**
* @brief Reports how many Universal Integrated Circuit Card slots the modem has.
*
* How many card slots exist, which is what a caller reads before deciding how many
* slots to address. It is a count and only a count: this interface does not state how
* the count maps onto the identifiers cellular_hal_sim_power_enable() accepts, nor
* onto the index cellular_hal_get_uicc_slot_info() takes, so it says how many slots
* there are and not which numbers name them.
*
* @param[out] total_count variable is a unsigned integer pointer which points the total count of UICC slot.
*                         \n The possible values can be 1 or 2 and depends on number of SIM slots provided by vendor.
*                         \n The caller allocates the unsigned int and retains
*                         ownership of it. The implementation writes through the
*                         pointer during the call. Whether it retains the pointer
*                         afterwards is not specified by this interface, so - per
*                         'Memory Model' in docs/pages/halSpec.md - a caller cannot
*                         rely on either behaviour and keeps the storage valid and
*                         unmoved for as long as it uses this interface. This
*                         interface does not state whether NULL is accepted, so a
*                         caller must supply a valid address, and does not define a
*                         value that means "unknown".
*                         \n The count is not a slot number and no arithmetic on it is
*                         sanctioned here. The only slot numbering this interface
*                         states is the 1 or 2 given for
*                         cellular_hal_sim_power_enable()'s slot_id; it states none
*                         for cellular_hal_get_uicc_slot_info()'s slot_index, and does
*                         not state that either follows from this count. A caller must
*                         therefore not read the count as an upper or lower bound on
*                         either argument, and must confirm the numbering with the
*                         implementation before addressing a specific slot.
*
* @returns The status of the operation.
* @retval RETURN_OK    (0) - The count has been written and may be read.
* @retval RETURN_ERROR (-1) - The call failed and nothing may be read through
*                      total_count; this interface does not state whether it was
*                      written, so a caller must not use the value. This is the
*                      interface's only failure code and it names no cause, so a
*                      caller cannot tell from it why the read failed. The action left
*                      to a caller is to retry the read rather than proceed on a value
*                      it cannot use.
*
* @pre cellular_hal_init() has returned RETURN_OK.
* @post On success the count is written. On failure the pointed-to value is
*       undefined. The call reports state and changes none.
*
* @note Blocking: synchronous. No timeout is specified, per 'Blocking calls' in
*       docs/pages/halSpec.md.
* @warning Not thread safe. Per 'Threading Model' in docs/pages/halSpec.md the
*          obligation is the caller's: serialise this call against every other
*          Cellular HAL call, including calls issued from another process.
* @see cellular_hal_get_uicc_slot_info, cellular_hal_sim_power_enable
*/
int cellular_hal_get_total_no_of_uicc_slots(unsigned int *total_count);

/**
* @brief Reads the state and identity of one Universal Integrated Circuit Card slot.
*
* Fills one caller-supplied CellularUICCSlotInfoStruct with the slot's enable flags,
* card presence, form factor, application type, card status, operator name, card
* identifier and subscription number. It is a snapshot: nothing in this interface
* notifies a caller that a slot's contents have changed, so a caller that tracks a
* slot re-reads it.
*
* @param[in] slot_index variable is a unsigned integer which contains index of UICC slot. The possible range sould not exceed (2^32)-1.
*                       \n This interface states no numbering for this argument: it
*                       gives neither a base nor a range beyond the width of the type
*                       quoted above. It does not state that the index follows the
*                       count reported by cellular_hal_get_total_no_of_uicc_slots(),
*                       and does not state that it shares the numbering of
*                       cellular_hal_sim_power_enable()'s slot_id, whose values it
*                       does state as 1 or 2. A caller must therefore not assume that
*                       indices begin at 0, must not assume that they begin at 1, and
*                       must not derive a range from the count; a caller that must
*                       address a specific slot confirms the numbering with the
*                       implementation. Nor does this interface state what an
*                       implementation does with an index that names no slot.
* @param[out] pstSlotInfo - Pointer to structure CellularUICCSlotInfoStruct that needs to be updated.
*                           \n The caller allocates the structure and retains
*                           ownership of it. The implementation writes through the
*                           pointer during the call. Whether it retains the pointer
*                           afterwards is not specified by this interface, so - per
*                           'Memory Model' in docs/pages/halSpec.md - a caller cannot
*                           rely on either behaviour and keeps the storage valid and
*                           unmoved for as long as it uses this interface. Must point
*                           to storage for a whole CellularUICCSlotInfoStruct. Its
*                           character members are fixed-size arrays inside the
*                           structure, so no separate allocation is involved and
*                           nothing here must be freed. This interface does not state
*                           whether any of those members is zero-terminated, so a
*                           caller bounds every read by the declared size of the
*                           member it is reading. Its iccid member is the extreme
*                           case: it holds 20 bytes and an ICCID is up to 20 digits,
*                           so a full-length identifier fills it with no room for a
*                           terminator. A caller must read that member as a
*                           fixed-width 20-byte field, copying at most 20 bytes and
*                           terminating the copy itself, and must not assume the
*                           member is a C string;
*                           cellular_hal_get_modem_current_iccid() is the call that
*                           returns an identifier in full, into a caller buffer of at
*                           least 21 bytes.
*
* @returns The status of the operation.
* @retval RETURN_OK    (0) - The structure has been populated and may be read. A
*                      caller reads its IsCardPresent member before treating the card
*                      members as meaningful.
* @retval RETURN_ERROR (-1) - The call failed and nothing may be read from the
*                      structure; it may have been partly written. This is the
*                      interface's only failure code and it names no cause, so a
*                      caller cannot tell from it why the read failed - in particular
*                      it cannot tell whether slot_index named a slot. The action left
*                      to a caller is to read the slot count with
*                      cellular_hal_get_total_no_of_uicc_slots() and confirm the
*                      numbering with the implementation rather than retry the same
*                      index.
*
* @pre cellular_hal_init() has returned RETURN_OK and slot_index names a slot the
*      device has. Which indices name a slot is not established by this interface,
*      and passing one that names none is not specified by it.
* @post On success every member of the structure has been written. On failure its
*       contents are undefined. The call reports state and changes none.
*
* @note Blocking: synchronous. No timeout is specified, per 'Blocking calls' in
*       docs/pages/halSpec.md.
* @note The MnoName, iccid and msisdn members identify a subscriber. A caller must
*       not log them.
* @warning Not thread safe. Per 'Threading Model' in docs/pages/halSpec.md the
*          obligation is the caller's: serialise this call against every other
*          Cellular HAL call, including calls issued from another process.
* @see cellular_hal_get_total_no_of_uicc_slots, cellular_hal_get_active_card_status
*/
int cellular_hal_get_uicc_slot_info(unsigned int slot_index, CellularUICCSlotInfoStruct *pstSlotInfo);

/**
* @brief Reads the status of the currently active card.
*
* Reports whether the card the modem is using is usable, without the caller having
* to know which slot it is in. Only CELLULAR_UICC_STATUS_VALID means usable; the
* other three values are delivered through a successful call, so a caller must
* inspect the value rather than rely on the status code.
*
* @param[out] card_status variable is from the enumerated datatype CellularUICCStatus_t.
*                         \n card_status returns the SIM card status. It can be valid / blocked / error / empty.
*                         \n The caller allocates the enumeration variable and
*                         retains ownership of it. The implementation writes through
*                         the pointer during the call. Whether it retains the pointer
*                         afterwards is not specified by this interface, so - per
*                         'Memory Model' in docs/pages/halSpec.md - a caller cannot
*                         rely on either behaviour and keeps the storage valid and
*                         unmoved for as long as it uses this interface. This
*                         interface does not state whether NULL is accepted, so a
*                         caller must supply a valid address.
*
* @returns The status of the operation.
* @retval RETURN_OK    (0) - The card status has been written and may be read.
* @retval RETURN_ERROR (-1) - The call failed and nothing may be read through
*                      card_status. Note the distinction this interface draws through
*                      its own enumeration: a card that is blocked, in error or empty
*                      is reported as a value through RETURN_OK, so RETURN_ERROR means
*                      the status could not be determined at all. Beyond that it names
*                      no cause, so a caller cannot tell from it why. The action left
*                      to a caller is to read the control interface back with
*                      cellular_hal_IsModemControlInterfaceOpened() before retrying
*                      the read.
*
* @pre cellular_hal_init() has returned RETURN_OK.
* @post On success the enumeration is written. On failure the pointed-to value is
*       undefined. The call reports state and changes none.
*
* @note Blocking: synchronous. No timeout is specified, per 'Blocking calls' in
*       docs/pages/halSpec.md.
* @note This interface declares no call that unblocks a blocked card, so
*       CELLULAR_UICC_STATUS_BLOCKED cannot be resolved through it.
* @warning Not thread safe. Per 'Threading Model' in docs/pages/halSpec.md the
*          obligation is the caller's: serialise this call against every other
*          Cellular HAL call, including calls issued from another process.
* @see cellular_hal_get_uicc_slot_info, cellular_hal_sim_power_enable
*/
int cellular_hal_get_active_card_status(CellularUICCStatus_t *card_status);

/** @} */  //END OF GROUP CELLULAR_HAL_APIS
/**
* @addtogroup CELLULAR_HAL_TYPES
* @{
*/

/**
* @brief Callback type reporting a change in network registration.
*
* Installed directly by cellular_hal_monitor_device_registration(). It is the only
* asynchronous report of registration this interface offers, and it carries three
* separate facts: whether the device is registered, whether the registration is a
* roaming one, and which domains it covers. DEVICE_NAS_STATUS_REGISTERING is not
* terminal, so an implementation of this type must tolerate being called more than
* once.
*
* @param[in] device_registration_status variable is from the enumerated datatype CellularDeviceNASStatus_t.
*                                       \n Only DEVICE_NAS_STATUS_REGISTERED reports
*                                       a completed registration. This interface does
*                                       not state whether registration is retried
*                                       indefinitely or eventually abandoned, so a
*                                       callback must not treat
*                                       DEVICE_NAS_STATUS_REGISTERING as a promise of
*                                       a later success.
* @param[in] roaming_status variable is from the enumerated datatype CellularDeviceNASRoamingStatus_t.
*                           \n DEVICE_NAS_STATUS_ROAMING_ON means a profile whose
*                           bIsNoRoaming flag is set must not be activated.
* @param[in] registered_service variable is from the enumerated datatype CellularModemRegisteredServiceType_t.
*                               \n Packet data is carried only where the value
*                               includes the packet-switched domain. This interface
*                               does not state what the argument holds when the
*                               device is not registered.
*
* @returns The status the callback reports back to the implementation.
* @retval RETURN_OK    (0) - The notification was accepted.
* @retval RETURN_ERROR (-1) - The notification was not accepted. This interface does
*                      not state what an implementation does with this value, so a
*                      callback must not rely on returning it to suppress or repeat a
*                      notification.
*
* @pre cellular_hal_monitor_device_registration() has been called with this function
*      installed. A callback is reached no other way, and this interface declares no
*      means of removing one.
* @post None on the interface's side; the return value has no stated effect.
*
* @note Blocking: this callback runs on the implementation's thread, so an
*       implementation of it that blocks delays the HAL. It should return promptly.
*       No deadline is specified.
* @note All three arguments are passed by value, so nothing here is borrowed and
*       nothing must be copied or freed.
* @warning Thread safety is the callback author's obligation. This interface does not
*          state the thread a callback arrives on, so shared state must be
*          synchronised, and a call back into this interface must be serialised
*          against the caller's other Cellular HAL calls.
* @see cellular_hal_monitor_device_registration, cellular_hal_get_current_plmn_information
*/
typedef int (*cellular_device_registration_status_callback)( CellularDeviceNASStatus_t device_registration_status ,
                                                             CellularDeviceNASRoamingStatus_t roaming_status,
                                                             CellularModemRegisteredServiceType_t registered_service );

/** @} */  //END OF GROUP CELLULAR_HAL_TYPES
/**
 * @addtogroup CELLULAR_HAL_APIS
 * @{
 */

/**
* @brief Instructs the lower layer to monitor network registration.
*
* Starts registration monitoring and installs the callback through which changes are
* reported. Monitoring is the only way to observe a registration change as it
* happens; cellular_hal_get_current_plmn_information() reads the current state but
* reports no transition. This interface declares no call that stops monitoring.
*
* @param[in] device_registration_status_cb is a function pointer which receives device registration status response from lower layer.
*                                          \n The caller owns the function. This
*                                          interface declares no call that removes a
*                                          callback, does not state when delivery on an
*                                          installed one ends, and does not state
*                                          whether an installed callback survives a
*                                          modem reset or factory reset, so a caller has
*                                          no point at which this interface makes
*                                          invalidating the pointer safe. The
*                                          conservative consequence - stated under
*                                          'Asynchronous Notification Model' in
*                                          docs/pages/halSpec.md - is that a caller
*                                          keeps the function callable for as long as it
*                                          uses this interface, which follows from the
*                                          silence and is neither a guarantee that
*                                          delivery continues nor a bound on how long it
*                                          may continue. This interface does
*                                          not state whether NULL is accepted, nor what
*                                          happens if the call is issued twice with
*                                          different functions.
*
* @returns The status of the operation.
* @retval RETURN_OK    (0) - Monitoring has been accepted and the callback is
*                      installed. It says nothing about the current registration
*                      state, which arrives through the callback.
* @retval RETURN_ERROR (-1) - The call failed. This interface does not state whether
*                      monitoring was begun or the callback installed, so the outcome
*                      is unknown rather than undone and a caller must not assume
*                      either. This is the interface's only failure code and it names
*                      no cause, so a caller cannot tell from it why the call failed.
*                      The action left to a caller is to read the control interface
*                      back with cellular_hal_IsModemControlInterfaceOpened() before
*                      retrying.
*
* @pre cellular_hal_init() has returned RETURN_OK. Beyond that this interface states
*      no ordering constraint - a caller must not infer that registration monitoring
*      has to begin before a network is started ('Method Sequencing' in
*      docs/pages/halSpec.md).
* @post On success monitoring is under way and the callback is installed. What is
*       established about that installation is only that this interface declares no
*       call that removes it, so a caller cannot stop the notification it started. How
*       long the implementation may keep notifying is not specified, and neither is
*       whether an installed callback survives cellular_hal_modem_reset() or
*       cellular_hal_modem_factory_reset(), so a caller must not assume that monitoring
*       continues and must not assume that it has to be
*       re-established after a reset. A caller that needs either behaviour agrees it
*       with the implementation outside this interface. On failure this interface does
*       not state whether the callback was installed, so a caller must be prepared to
*       receive a notification even after a failed call.
*
* @note Blocking: synchronous. The call returns a status; registration changes arrive
*       asynchronously through the installed callback. No timeout is specified for
*       either.
* @warning Not thread safe. Per 'Threading Model' in docs/pages/halSpec.md the
*          obligation is the caller's: serialise this call against every other
*          Cellular HAL call, including calls issued from another process.
* @see cellular_hal_get_current_plmn_information, cellular_hal_set_modem_network_attach
*/
int cellular_hal_monitor_device_registration(cellular_device_registration_status_callback device_registration_status_cb);

/** @} */  //END OF GROUP CELLULAR_HAL_APIS
/**
* @addtogroup CELLULAR_HAL_TYPES
* @{
*/

/**
* @brief Callback type reporting the outcome of a profile create, modify or delete.
*
* Installed directly by cellular_hal_profile_create(),
* cellular_hal_profile_delete() and cellular_hal_profile_modify(). One type serves
* all three operations and the notification does not say which operation it belongs
* to, so a caller correlates it with the request it issued.
*
* @param[in] profile_id variable is a character pointer which contains the profile ID. The buffer size should be atleast 64 bytes long. It is a vendor specific value.
*                       \n The implementation supplies the buffer and this interface
*                       does not state how long it remains valid after the callback
*                       returns. Treat it as read-only and borrowed: copy what is
*                       needed, never store the pointer and never free it. Note that
*                       the identifier arrives here as text while the ProfileID
*                       member of CellularProfileStruct is an int, and this interface
*                       does not state how the two forms correspond.
* @param[in] PDPType variable is from the enumerated datatype CellularPDPType_t.
*                    \n The address type of the profile the notification concerns.
* @param[in] device_profile_status variable is from the enumerated datatype CellularDeviceProfileSelectionStatus_t.
*                                  \n DEVICE_PROFILE_STATUS_READY reports a profile
*                                  that can carry a network and
*                                  DEVICE_PROFILE_STATUS_DELETED one that must not be
*                                  used again. DEVICE_PROFILE_STATUS_CONFIGURING is
*                                  not terminal.
*
* @returns The status the callback reports back to the implementation.
* @retval RETURN_OK    (0) - The notification was accepted.
* @retval RETURN_ERROR (-1) - The callback reports that it did not accept the
*                      notification. This interface does not state what an
*                      implementation does with this value, so a callback must not
*                      return it expecting the profile operation to be undone or
*                      repeated.
*
* @pre One of the three profile calls has been made with this function installed.
* @post None on the interface's side; the return value has no stated effect.
*
* @note Blocking: this callback runs on the implementation's thread and should return
*       promptly. No deadline is specified.
* @warning Thread safety is the callback author's obligation: this interface does not
*          state the thread a callback arrives on, so shared state must be
*          synchronised.
* @see cellular_hal_profile_create, cellular_hal_profile_modify, cellular_hal_profile_delete
*/
typedef int (*cellular_device_profile_status_api_callback)( char *profile_id, CellularPDPType_t  PDPType, CellularDeviceProfileSelectionStatus_t device_profile_status );

/** @} */  //END OF GROUP CELLULAR_HAL_TYPES
/**
 * @addtogroup CELLULAR_HAL_APIS
 * @{
 */

/**
* @brief Creates a Packet Data Protocol context profile, or selects the default one.
*
* A profile is what a packet data network is started on, so this is the call that
* makes an Access Point Name, its authentication and its address type available to
* cellular_hal_start_network(). Passing no profile is meaningful here: with a NULL
* pstProfileInput the implementation selects the default profile rather than
* creating one, which is the behaviour a caller wants when the modem is already
* provisioned.
*
* @param[in] pstProfileInput is a Profile structure from CellularProfileStruct which needs to pass when creating a profile.
*                            \n NULL selects the default profile instead of creating
*                            one. Otherwise the caller allocates the structure and
*                            retains ownership of it; the implementation reads it
*                            during the call. This interface does not specify whether
*                            the implementation retains the pointer after the call has
*                            returned, so a caller must not assume either behaviour
*                            and must keep the structure allocated and unmodified
*                            until the outcome has been notified rather than release
*                            or reuse the memory immediately. This interface does not
*                            state which members an implementation requires,
*                            nor whether ProfileID is assigned by the caller or by the
*                            implementation. The Username and Password members are
*                            credentials: a caller must not log the structure.
* @param[in] device_profile_status_cb is a function pointer which receives device profile create status response from driver.
*                                     \n The caller owns the function. This interface
*                                     declares no call that removes a callback, does
*                                     not state when delivery on an installed one ends,
*                                     and does not state whether an installed callback
*                                     survives a modem reset or factory reset, so a
*                                     caller has no point at which this interface makes
*                                     invalidating the pointer safe. The conservative
*                                     consequence - stated under 'Asynchronous
*                                     Notification Model' in docs/pages/halSpec.md - is
*                                     that a caller keeps the function callable for as
*                                     long as it uses this interface, which follows
*                                     from the silence and is neither a guarantee that
*                                     delivery continues nor a bound on how long it may
*                                     continue. This interface does not state whether
*                                     NULL is accepted.
*
* @returns The status of the operation.
* @retval RETURN_OK    (0) - The request has been accepted and the callback is
*                      installed. It does not mean the profile exists: a caller waits
*                      for DEVICE_PROFILE_STATUS_READY, or reads the profiles back
*                      with cellular_hal_get_profile_list().
* @retval RETURN_ERROR (-1) - The call failed. This interface does not state whether a
*                      profile was created, so the outcome is unknown rather than
*                      undone. This is the interface's only failure code and it names
*                      no cause, so a caller cannot tell from it why the call failed.
*                      The action left to a caller is to read the profiles back with
*                      cellular_hal_get_profile_list() before retrying rather than
*                      repeat the request - a retry on an unknown outcome could
*                      otherwise create a second profile.
*
* @pre cellular_hal_init() has returned RETURN_OK. This interface states no further
*      ordering constraint, so a caller must not infer that a card slot has to be
*      selected first ('Method Sequencing' in docs/pages/halSpec.md).
* @post On success profile creation or default selection is under way and the callback
*       is installed. This interface states no lifetime for an installed callback - it
*       declares no call that removes one and does not state how long notification may
*       continue or whether it survives a modem reset - so a caller must not assume
*       either that delivery continues or that it stops. On
*       failure this interface does not state whether a partial profile remains, so a
*       caller should read the profile list to establish what exists.
*
* @note Blocking: synchronous. The call returns a status; the outcome arrives
*       asynchronously through the installed callback. No timeout is specified for
*       either.
* @note Persistence is not this interface's business: per 'Persistence Model' in
*       docs/pages/halSpec.md a caller that needs a profile to survive a restart
*       persists it itself.
* @warning Not thread safe. Per 'Threading Model' in docs/pages/halSpec.md the
*          obligation is the caller's: serialise this call against every other
*          Cellular HAL call, including calls issued from another process.
* @see cellular_hal_profile_modify, cellular_hal_profile_delete, cellular_hal_get_profile_list
*/
int cellular_hal_profile_create(CellularProfileStruct *pstProfileInput, cellular_device_profile_status_api_callback device_profile_status_cb);

/**
* @brief Deletes a Packet Data Protocol context profile.
*
* Removes the profile the caller names, so that it can no longer carry a packet data
* network. Unlike cellular_hal_profile_create() this call has no defined behaviour
* for a NULL profile: it identifies what to delete from the structure it is given.
*
* @param[in] pstProfileInput is a Profile structure from CellularProfileStruct which needs to pass when deleting a profile.
*                            \n The caller allocates the structure and retains
*                            ownership of it; the implementation reads it during the
*                            call. This interface does not specify whether the
*                            implementation retains the pointer afterwards, so a
*                            caller must keep the structure allocated and unmodified
*                            until the deletion has been notified. This interface does
*                            not state which members identify the profile to delete -
*                            ProfileID, ProfileName and APN are all candidates - so a
*                            caller should supply a structure it obtained from
*                            cellular_hal_get_profile_list() rather than one it has
*                            partly filled. This interface does not state whether NULL
*                            is accepted, and does not define a delete-all form.
* @param[in] device_profile_status_cb - The function pointer which receives device profile delete status response from driver.
*                                       \n The caller owns the function. This interface
*                                       declares no call that removes a callback, does
*                                       not state when delivery on an installed one
*                                       ends, and does not state whether an installed
*                                       callback survives a modem reset or factory
*                                       reset, so a caller has no point at which this
*                                       interface makes invalidating the pointer safe.
*                                       The conservative consequence - stated under
*                                       'Asynchronous Notification Model' in
*                                       docs/pages/halSpec.md - is that a caller keeps
*                                       the function callable for as long as it uses
*                                       this interface, which follows from the silence
*                                       and is neither a guarantee that delivery
*                                       continues nor a bound on how long it may
*                                       continue. The deletion is reported as
*                                       DEVICE_PROFILE_STATUS_DELETED.
*
* @returns The status of the operation.
* @retval RETURN_OK    (0) - The request has been accepted and the callback is
*                      installed. It does not mean the profile is gone: a caller waits
*                      for DEVICE_PROFILE_STATUS_DELETED, or reads the profile list
*                      back.
* @retval RETURN_ERROR (-1) - The call failed. This interface does not state whether
*                      the profile was deleted, so the outcome is unknown rather than
*                      undone. This is the interface's only failure code and it names
*                      no cause, so a caller cannot tell from it why the call failed.
*                      The action left to a caller is to read the profiles back with
*                      cellular_hal_get_profile_list() to establish whether the profile
*                      still exists before retrying.
*
* @pre cellular_hal_init() has returned RETURN_OK and the profile named exists. This
*      interface does not specify what deleting an unknown profile does.
* @post On success deletion is under way. This interface does not state what happens
*       to a packet data network running on the profile, so a caller should stop the
*       network with cellular_hal_stop_network() before deleting the profile it runs
*       on. On failure this interface does not state whether the profile was partly
*       removed.
*
* @note Blocking: synchronous. The call returns a status; the outcome arrives
*       asynchronously through the installed callback. No timeout is specified for
*       either.
* @warning Not thread safe. Per 'Threading Model' in docs/pages/halSpec.md the
*          obligation is the caller's: serialise this call against every other
*          Cellular HAL call, including calls issued from another process.
* @see cellular_hal_profile_create, cellular_hal_profile_modify, cellular_hal_stop_network
*/
int cellular_hal_profile_delete(CellularProfileStruct *pstProfileInput, cellular_device_profile_status_api_callback device_profile_status_cb);

/**
* @brief Modifies an existing Packet Data Protocol context profile.
*
* Applies the caller's structure to a profile that already exists - changing an
* Access Point Name, a credential, an address type or a flag without deleting and
* recreating it. A NULL profile is an error here, not a default selection, which is
* the one behavioural difference from cellular_hal_profile_create().
*
* @param[in] pstProfileInput is a Profile structure from CellularProfileStruct which needs to pass when modifying a profile.
*                            \n Must not be NULL: with no profile there is nothing to
*                            modify and the call reports RETURN_ERROR. The caller
*                            allocates the structure and retains ownership of it; the
*                            implementation reads it during the call. This interface
*                            does not specify whether the implementation retains the
*                            pointer afterwards, so a caller must keep the structure
*                            allocated and unmodified until the outcome has been
*                            notified. This interface does not state which members
*                            identify the profile and which carry the new
*                            values, so a caller should read a profile with
*                            cellular_hal_get_profile_list(), change the members it
*                            intends to change and pass the whole structure back. The
*                            Username and Password members are credentials: a caller
*                            must not log the structure.
* @param[in] device_profile_status_cb - The function pointer which receives device profile modify status response from driver.
*                                       \n The caller owns the function. This interface
*                                       declares no call that removes a callback, does
*                                       not state when delivery on an installed one
*                                       ends, and does not state whether an installed
*                                       callback survives a modem reset or factory
*                                       reset, so a caller has no point at which this
*                                       interface makes invalidating the pointer safe.
*                                       The conservative consequence - stated under
*                                       'Asynchronous Notification Model' in
*                                       docs/pages/halSpec.md - is that a caller keeps
*                                       the function callable for as long as it uses
*                                       this interface, which follows from the silence
*                                       and is neither a guarantee that delivery
*                                       continues nor a bound on how long it may
*                                       continue.
*
* @returns The status of the operation.
* @retval RETURN_OK    (0) - The request has been accepted and the callback is
*                      installed. It does not mean the change has been applied: a
*                      caller waits for DEVICE_PROFILE_STATUS_READY.
* @retval RETURN_ERROR (-1) - The call failed. A NULL pstProfileInput is reported
*                      here, and it is the one cause this interface maps to this value.
*                      Beyond it the code names no cause, so a caller cannot tell from
*                      it why the call failed, and this interface does not state
*                      whether the profile was modified, so the outcome is unknown
*                      rather than undone. The action left to a caller is to read the
*                      profile back with cellular_hal_get_profile_list() before
*                      retrying.
*
* @pre cellular_hal_init() has returned RETURN_OK and the profile named exists.
* @post On success the modification is under way. This interface does not state
*       whether a change takes effect on a network already running on the profile, so
*       a caller that needs the new values in force should stop and restart the
*       network. On failure this interface does not state whether the profile was
*       partly modified, so a caller should read it back.
*
* @note Blocking: synchronous. The call returns a status; the outcome arrives
*       asynchronously through the installed callback. No timeout is specified for
*       either.
* @warning Not thread safe. Per 'Threading Model' in docs/pages/halSpec.md the
*          obligation is the caller's: serialise this call against every other
*          Cellular HAL call, including calls issued from another process.
* @see cellular_hal_profile_create, cellular_hal_profile_delete, cellular_hal_get_profile_list
*/
int cellular_hal_profile_modify(CellularProfileStruct *pstProfileInput, cellular_device_profile_status_api_callback device_profile_status_cb);

/**
* @brief Reads the modem's configured Packet Data Protocol context profiles.
*
* Reports every profile the modem holds, as an array reached through the caller's
* pointer together with the number of elements in it. It is how a caller discovers
* what exists before creating, modifying or deleting a profile, and the only call
* that returns a profile rather than accepting one.
*
* @param[out] ppstProfileOutput - List of profiles needs to be return from the structure CellularProfileStruct.
*                                 \n The argument is the address of the caller's own
*                                 CellularProfileStruct pointer, which the
*                                 implementation sets to the start of an array of
*                                 profiles. The caller owns that pointer variable, and
*                                 the implementation writes through its address during
*                                 the call. Whether it retains that address afterwards
*                                 is not specified by this interface, so - per 'Memory
*                                 Model' in docs/pages/halSpec.md - a caller cannot
*                                 rely on either behaviour and keeps the pointer
*                                 variable itself valid and unmoved for as long as it
*                                 uses this interface. The array the variable is
*                                 pointed at is a separate question, addressed next.
*                                 \n Allocation of the array itself is not established
*                                 by this interface. It states neither which side
*                                 allocates it, nor - if the implementation allocates
*                                 it - which function releases it, and 'Memory Model'
*                                 in docs/pages/halSpec.md records the same gap from
*                                 the caller's side. What is established is only this:
*                                 on RETURN_OK the pointer addresses profile_count
*                                 elements that may be read.
*                                 \n No lifetime is specified. This interface does not
*                                 state how long the array stays valid, and it names
*                                 no event - a later call, a reset, a notification -
*                                 that ends its validity, so a caller must not
*                                 substitute a rule of its own for the missing one.
*                                 The conservative consequence is that a caller reads
*                                 what it needs into its own storage as soon as the
*                                 call returns and works from that copy, must not free
*                                 the array, must not assume anything else frees it,
*                                 and must not retain the pointer. A caller that
*                                 requires a defined lifetime has to agree the
*                                 convention with the implementation outside this
*                                 interface.
*                                 \n This interface does not state whether NULL is
*                                 accepted in place of the address.
* @param[out] profile_count variable is a integer pointer, in that total profile count needs to be return.
*                           \n The possible range of acceptable values is 0 to (2^31)-1
*                           \n The number of elements available through the array, and
*                           the only bound on indexing it: this interface publishes no
*                           terminator element and no maximum profile count, so a
*                           caller reads this value before touching the array and
*                           indexes from 0 to one less than it. The caller allocates
*                           the int and retains ownership of it. This interface does
*                           not state what is written on failure, nor whether a count
*                           of 0 leaves the array pointer unchanged or sets it to NULL,
*                           so a caller must initialise both arguments and treat a
*                           zero count as "no profiles readable" rather than
*                           dereference the pointer.
*
* @returns The status of the operation.
* @retval RETURN_OK    (0) - The count has been written and, where it is greater than
*                      zero, the array may be read for that many elements.
* @retval RETURN_ERROR (-1) - The call failed and neither output may be read. This is
*                      the interface's only failure code and it names no cause, so a
*                      caller cannot tell from it why the read failed. This interface
*                      also does not state how a modem holding no profiles is reported,
*                      so a caller must not read a failure as an empty list, and must
*                      not retry indefinitely on the assumption that a profile will
*                      appear.
*
* @pre cellular_hal_init() has returned RETURN_OK.
* @post On success the count is written, and the pointer is set where the count is
*       greater than zero. On failure both outputs are undefined. The call reports
*       state and changes none.
*
* @note Blocking: synchronous. No timeout is specified, per 'Blocking calls' in
*       docs/pages/halSpec.md.
* @note The Username and Password members of a returned profile are credentials. This
*       interface does not state whether an implementation returns them populated or
*       cleared, so a caller must treat the whole structure as sensitive and must not
*       log it.
* @warning Not thread safe. Per 'Threading Model' in docs/pages/halSpec.md the
*          obligation is the caller's: serialise this call against every other
*          Cellular HAL call, including calls issued from another process. The array
*          compounds that obligation rather than softening it: because this interface
*          specifies no lifetime for it, the effect of a concurrent call on a list
*          another thread is still reading is unspecified too, so a caller must
*          finish copying what it needs before it issues its next call.
* @see cellular_hal_profile_create, cellular_hal_profile_modify, cellular_hal_profile_delete
*/
int cellular_hal_get_profile_list(CellularProfileStruct **ppstProfileOutput, int *profile_count);

/** @} */  //END OF GROUP CELLULAR_HAL_APIS
/**
* @addtogroup CELLULAR_HAL_TYPES
* @{
*/

/**
* @brief Callback type reporting whether packet service is carrying traffic.
*
* Installed as the packet_service_status_cb member of the CellularNetworkCBStruct
* passed to cellular_hal_start_network(). It reports the bearer, which is a separate
* fact from whether an IP configuration is ready: a caller that needs a usable
* connection waits for this notification and for
* cellular_device_network_ip_ready_api_callback.
*
* @param[in] device_name variable is a character pointer contains the modem device name.
*                        \n The buffer size should be atleast 16 bytes long.
*                        \n It is a vendor specific value.
*                        \n The implementation supplies the buffer and this interface
*                        does not state how long it remains valid after the callback
*                        returns. Treat it as read-only and borrowed: copy what is
*                        needed, never store the pointer and never free it. The
*                        16-byte figure is the size this interface asks for. It does
*                        not state whether the string is zero-terminated within that
*                        size, so a caller must bound every read by it and must not
*                        assume a terminator.
* @param[in] ip_type - The enum which receives IP configuration.
*                      \n It can hold any value from the enum CellularNetworkIPType_t.
*                      \n The family the notification concerns, matching the argument
*                      the network was started for.
* @param[in] packet_service_status is from the enumerated datatype CellularNetworkPacketStatus_t.
*                                  \n It can hold any value from the enum CellularNetworkPacketStatus_t.
*                                  \n DEVICE_NETWORK_STATUS_DISCONNECTED can arrive
*                                  after a connection has been established, so a
*                                  caller must handle a loss of service and not only
*                                  the initial connect.
*
* @returns The status the callback reports back to the implementation.
* @retval RETURN_OK    (0) - The notification was accepted.
* @retval RETURN_ERROR (-1) - The callback reports that it did not accept the
*                      notification. This interface does not state what an
*                      implementation does with this value, so a callback must not
*                      return it expecting a lost connection to be re-established.
*
* @pre cellular_hal_start_network() has been called with this function installed as a
*      member of its CellularNetworkCBStruct.
* @post None on the interface's side; the return value has no stated effect.
*
* @note Blocking: this callback runs on the implementation's thread and should return
*       promptly. No deadline is specified.
* @warning Thread safety is the callback author's obligation: this interface does not
*          state the thread a callback arrives on, so shared state must be
*          synchronised.
* @see cellular_hal_start_network, cellular_device_network_ip_ready_api_callback
*/
typedef int (*cellular_network_packet_service_status_api_callback)( char *device_name, CellularNetworkIPType_t ip_type, CellularNetworkPacketStatus_t packet_service_status );

/**
* @brief Callback type delivering the IP configuration of a started network.
*
* Installed as the device_network_ip_ready_cb member of the CellularNetworkCBStruct
* passed to cellular_hal_start_network(). It is the only place this interface
* delivers a CellularIPStruct, so a caller that needs the address, mask, gateway,
* name servers or MTU of its bearer takes them here.
*
* @param[in] pstIPStruct variable is a structure pointer from CellularIPStruct.
*                        \n The structure is an input to the callback: the
*                        implementation allocates and fills it and the callback reads
*                        it. This interface does not state how long it remains valid
*                        after the callback returns, so a callback must treat it as
*                        borrowed - copy the whole structure if any of it is needed
*                        later, never store the pointer and never free it. Its
*                        contents may not describe a usable configuration unless
*                        ip_ready_status is DEVICE_NETWORK_IP_READY. Its IPType member
*                        says which family the address members are written in.
* @param[in] ip_ready_status variable is from the enumerated datatype CellularDeviceIPReadyStatus_t.
*                            \n DEVICE_NETWORK_IP_READY reports that the accompanying
*                            structure carries a usable configuration;
*                            DEVICE_NETWORK_IP_NOT_READY reports that it does not.
*
* @returns The status the callback reports back to the implementation.
* @retval RETURN_OK    (0) - The notification was accepted.
* @retval RETURN_ERROR (-1) - The callback reports that it did not accept the
*                      notification. This interface does not state what an
*                      implementation does with this value, so a callback must not
*                      return it expecting the configuration to be delivered again.
*
* @pre cellular_hal_start_network() has been called with this function installed as a
*      member of its CellularNetworkCBStruct.
* @post None on the interface's side; the return value has no stated effect. The
*       implementation retains ownership of the structure.
*
* @note Blocking: this callback runs on the implementation's thread and should return
*       promptly - copying the structure and signalling another thread is the
*       expected shape. No deadline is specified.
* @warning Thread safety is the callback author's obligation: this interface does not
*          state the thread a callback arrives on, so shared state must be
*          synchronised.
* @see cellular_hal_start_network, cellular_network_packet_service_status_api_callback
*/
typedef int (*cellular_device_network_ip_ready_api_callback)( CellularIPStruct *pstIPStruct, CellularDeviceIPReadyStatus_t ip_ready_status );

/**
* @brief Callback container for the packet service and IP readiness notifications.
*
* The third argument of cellular_hal_start_network(). A caller fills both members
* before that call and owns the structure; the implementation reads it during the
* call. The two notifications report separate facts - the bearer and the IP
* configuration - so a caller that needs a usable connection installs and waits for
* both. This interface does not state whether either pointer may be NULL. It declares
* no call that removes a callback, does not state when delivery on an installed one
* ends, and does not state whether an installed callback survives a modem reset or
* factory reset, so a caller has no point at which this interface makes invalidating
* one of these functions safe. The conservative consequence - stated under
* 'Asynchronous Notification Model' in docs/pages/halSpec.md - is that a caller keeps
* both functions callable for as long as it uses this interface, which follows from the
* silence and is neither a guarantee that delivery continues nor a bound on how long it
* may continue.
*/
typedef  struct
{
    cellular_device_network_ip_ready_api_callback device_network_ip_ready_cb;        /*!< Delivers the IP configuration of a started network. Receives a
                                                                                     *   CellularIPStruct the implementation owns and fills, and a
                                                                                     *   CellularDeviceIPReadyStatus_t value stating whether that structure
                                                                                     *   carries a usable configuration. The structure is borrowed for the
                                                                                     *   duration of the call only. */
    cellular_network_packet_service_status_api_callback packet_service_status_cb;    /*!< Reports whether packet service is carrying traffic once the
                                                                                     *   network is started. Receives the vendor-specific modem device name in
                                                                                     *   a buffer of at least 16 bytes, a CellularNetworkIPType_t value naming
                                                                                     *   the address family the notification concerns, and a
                                                                                     *   CellularNetworkPacketStatus_t value. The string is borrowed from the
                                                                                     *   implementation for the duration of the call only. */
} CellularNetworkCBStruct;

/** @} */  //END OF GROUP CELLULAR_HAL_TYPES
/**
 * @addtogroup CELLULAR_HAL_APIS
 * @{
 */

/**
* @brief Starts the packet data network for one IP family.
*
* Brings up the bearer: it activates a Packet Data Protocol context on the profile
* supplied, or on the default profile where none is, and installs the two callbacks
* through which the packet service state and the resulting IP configuration are
* reported. The call returns as soon as the request is accepted, so a caller learns
* that the network is usable from the callbacks rather than from the return value.
*
* @param[in] ip_request_type variable is from the enumerated datatype CellularNetworkIPType_t which receives IP configuration for stopped network from driver.
*                            \n The family to start the network for. This interface
*                            does not state what CELLULAR_NETWORK_IP_FAMILY_UNKNOWN or
*                            CELLULAR_NETWORK_IP_FAMILY_UNSPECIFIED request, so a
*                            caller should name IPv4 or IPv6 explicitly. The value is
*                            reported back as the ip_type argument of the packet
*                            service callback.
* @param[in] pstProfileInput - Here needs to pass profile from structure CellularProfileStruct to start network. If NULL then it should take it default profile otherwise start based on input.
*                              \n NULL selects the profile whose
*                              bIsThisDefaultProfile flag is set. Otherwise the caller
*                              allocates the structure and retains ownership of it;
*                              the implementation reads it during the call. This
*                              interface does not specify whether the implementation
*                              retains the pointer after the call has returned, so a
*                              caller must not assume either behaviour and must keep
*                              the structure allocated and unmodified while the network
*                              it started is running. This interface does not state
*                              whether the profile must already exist, so a caller
*                              should create it first. The structure carries credentials: a caller must
*                              not log it.
* @param[in] pstCBStruct - Here needs to fill CB function pointer for packet and ip status from the structure CellularNetworkCBStruct.
*                          \n The caller allocates the structure and retains ownership
*                          of it. The implementation reads the two function pointers
*                          during the call. This interface declares no call that removes
*                          a callback, does not state when delivery on an installed one
*                          ends, and does not state whether an installed callback
*                          survives a modem reset or factory reset, so a caller has no
*                          point at which this interface makes invalidating one of the
*                          two functions safe. The conservative consequence - stated
*                          under 'Asynchronous Notification Model' in
*                          docs/pages/halSpec.md - is that a caller keeps both functions
*                          callable for as long as it uses this interface, which follows
*                          from the silence and is neither a guarantee that delivery
*                          continues nor a bound on how long it may continue. This
*                          interface does not state whether NULL, or a structure with a
*                          NULL member, is accepted.
*
* @returns The status of the operation.
* @retval RETURN_OK    (0) - The start has been accepted and the callbacks are
*                      installed. It does not mean traffic can flow: a caller waits for
*                      DEVICE_NETWORK_STATUS_CONNECTED and DEVICE_NETWORK_IP_READY.
* @retval RETURN_ERROR (-1) - The call failed. This interface does not state whether
*                      the start was begun or the callbacks installed, so the outcome
*                      is unknown rather than undone and a caller must not assume
*                      either. This is the interface's only failure code and it names
*                      no cause, so a caller cannot tell from it why the call failed.
*                      The action left to a caller is to read the modem's state back -
*                      cellular_hal_get_current_plmn_information() for registration and
*                      cellular_hal_get_current_modem_interface_status() for the
*                      interface - rather than repeat the call immediately.
*
* @pre cellular_hal_init() has returned RETURN_OK. Registration is what actually makes
*      a start possible, but this interface states no ordering constraint that requires
*      it ('Method Sequencing' in docs/pages/halSpec.md), so a caller must not read a
*      failure here as proof that the modem is unregistered.
* @post On success the start is under way and both callbacks are installed. This
*       interface states no lifetime for an installed callback - it declares no call
*       that removes one, does not state how long notification may continue, and does
*       not state whether an installed callback survives a modem reset - so a caller
*       must not assume that delivery continues and must
*       not assume that it stops. On failure this interface does not state whether a
*       partial activation remains or whether the callbacks were installed, so a caller
*       must be prepared to receive a notification after a failed call and should stop
*       the network for the family before trying again.
*
* @note Blocking: synchronous. The call returns a status; the outcome arrives
*       asynchronously through the installed callbacks. No timeout is specified for
*       either, and bringing up a bearer can take a perceptible time.
* @note This interface does not state whether two families may be started at once, nor
*       what a second start for a family already running does.
* @warning Not thread safe. Per 'Threading Model' in docs/pages/halSpec.md the
*          obligation is the caller's: serialise this call against every other Cellular
*          HAL call, including calls issued from another process.
* @see cellular_hal_stop_network, cellular_hal_get_packet_statistics, cellular_hal_profile_create
*/
int cellular_hal_start_network( CellularNetworkIPType_t ip_request_type, CellularProfileStruct *pstProfileInput, CellularNetworkCBStruct *pstCBStruct );

/**
* @brief Stops the packet data network for one IP family.
*
* Deactivates the bearer started by cellular_hal_start_network() for the family
* named. It is the counterpart of that call and nothing more: it does not tear the
* Cellular HAL down - this interface declares no deinitialization call - and it does
* not delete the profile the network ran on.
*
* @param[in] ip_request_type variable is from the enumerated datatype CellularNetworkIPType_t which identifies the packet data network to stop.
*                            \n The family whose network is to be stopped, named with
*                            the same enumeration cellular_hal_start_network() takes.
*                            This interface does not state what
*                            CELLULAR_NETWORK_IP_FAMILY_UNKNOWN or
*                            CELLULAR_NETWORK_IP_FAMILY_UNSPECIFIED request, and does
*                            not define a value meaning "every family", so a caller
*                            should name IPv4 or IPv6 explicitly and issue one call per
*                            family it started. Nor does it state what happens when no
*                            network is running for the family named.
*
* @returns The status of the operation.
* @retval RETURN_OK    (0) - The stop has been accepted. This interface does not state
*                      whether the bearer is down when the call returns, so a caller
*                      that needs to know waits for
*                      DEVICE_NETWORK_STATUS_DISCONNECTED on the packet service
*                      callback installed at start.
* @retval RETURN_ERROR (-1) - The call failed. This interface does not state whether
*                      the stop took effect, so the outcome is unknown rather than
*                      undone and a caller must not read this value as proof that the
*                      network is still up. This is the interface's only failure code
*                      and it names no cause, so a caller cannot tell from it why the
*                      call failed. The action left to a caller is to read the bearer's
*                      state back with cellular_hal_get_current_modem_interface_status()
*                      rather than retry blindly.
*
* @pre cellular_hal_init() has returned RETURN_OK, and a network has been started for
*      the family named. Stopping a family that was not started is not specified by
*      this interface.
* @post On success the stop is under way for that family. Any IP configuration
*       previously delivered for it must be treated as invalid. This interface declares
*       no call that removes the callbacks cellular_hal_start_network() installed, so a
*       caller cannot withdraw them here, and it does not state whether delivery through
*       them ends when a family is stopped. Stopping a network is therefore not the point
*       at which invalidating either function becomes safe: a caller keeps both callable
*       for as long as it uses this interface and is prepared to receive a notification
*       afterwards, without relying on one arriving. On failure this interface does not
*       state whether the network was partly torn down.
*
* @note Blocking: synchronous. No timeout is specified, per 'Blocking calls' in
*       docs/pages/halSpec.md, and tearing down a bearer can take a perceptible time.
* @warning Not thread safe. Per 'Threading Model' in docs/pages/halSpec.md the
*          obligation is the caller's: serialise this call against every other Cellular
*          HAL call, including calls issued from another process.
* @see cellular_hal_start_network, cellular_hal_get_packet_statistics
*/
int cellular_hal_stop_network(CellularNetworkIPType_t ip_request_type);

/**
* @brief Reads the current radio signal measurements.
*
* Fills one caller-supplied CellularSignalInfoStruct with the received strength,
* received quality, received power, signal-to-noise ratio and transmit power of the
* serving cell. It is a snapshot with no notification behind it, so a caller that
* tracks signal quality polls it.
*
* @param[out] signal_info variable is a pointer, needs to parse CellularSignalInfoStruct structure to get signal information.
*                         \n The caller allocates the structure and retains ownership
*                         of it. The implementation writes through the pointer during
*                         the call. Whether it retains the pointer afterwards is not
*                         specified by this interface, so - per 'Memory Model' in
*                         docs/pages/halSpec.md - a caller cannot rely on either
*                         behaviour and keeps the storage valid and unmoved for as
*                         long as it uses this interface. Must point to storage for a
*                         whole CellularSignalInfoStruct. Read its members with their
*                         units in mind: RSSI, RSRP and TXPower are absolute powers in
*                         dBm while RSRQ and SNR are ratios in dB, and every member is
*                         a whole number because the type is int - the half-decibel
*                         steps the 3GPP reporting range for RSRQ uses are not
*                         representable here. This interface does not state whether
*                         NULL is accepted.
*
* @returns The status of the operation.
* @retval RETURN_OK    (0) - The structure has been populated and may be read.
* @retval RETURN_ERROR (-1) - The call failed and nothing may be read from the
*                      structure; it may have been partly written. This is the
*                      interface's only failure code and it names no cause, so a caller
*                      cannot tell from it why the read failed. Because this interface
*                      defines no not-measured value for any member, a caller must not
*                      fall back to reading the structure after a failure.
*
* @pre cellular_hal_init() has returned RETURN_OK. The values are only meaningful
*      where the modem has a serving cell; before registration a caller should expect
*      failure or unpopulated members rather than treat the call as broken.
* @post On success every member of the structure has been written. On failure its
*       contents are undefined. The call reports state and changes none.
*
* @note Blocking: synchronous. No timeout is specified, per 'Blocking calls' in
*       docs/pages/halSpec.md.
* @warning Not thread safe. Per 'Threading Model' in docs/pages/halSpec.md the
*          obligation is the caller's: serialise this call against every other Cellular
*          HAL call, including calls issued from another process.
* @see cellular_hal_get_cell_location_info, cellular_hal_get_current_plmn_information
*/
int cellular_hal_get_signal_info(CellularSignalInfoStruct *signal_info);

/**
* @brief Reads the cell and band identifiers of the current radio location.
*
* Fills one caller-supplied CellLocationInfoStruct with the cell identifier, the band
* information and the serving cell identifier. It answers "which cell am I on", which
* is a different question from the network identity that
* cellular_hal_get_current_plmn_information() reports.
*
* @param[out] loc_info variable is a pointer, needs to parse CellLocationInfoStruct structure to get location information.
*                      \n The caller allocates the structure and retains ownership of
*                      it. The implementation writes through the pointer during the
*                      call. Whether it retains the pointer afterwards is not specified
*                      by this interface, so - per 'Memory Model' in
*                      docs/pages/halSpec.md - a caller cannot rely on either behaviour
*                      and keeps the storage valid and unmoved for as long as it uses
*                      this interface. Must point to storage for a whole
*                      CellLocationInfoStruct. Every member is an unsigned int, so each
*                      value lies in 0 to 0xFFFFFFFF: a 28-bit E-UTRAN cell identifier
*                      fits, a 36-bit NR cell identity does not, and this interface
*                      does not state how a 5G cell is identified here. This interface
*                      does not state whether NULL is accepted, and defines no value
*                      meaning "not known".
*
* @returns The status of the operation.
* @retval RETURN_OK    (0) - The structure has been populated and may be read.
* @retval RETURN_ERROR (-1) - The call failed and nothing may be read from the
*                      structure; it may have been partly written. This is the
*                      interface's only failure code and it names no cause, so a caller
*                      cannot tell from it why the read failed. The action left to a
*                      caller is to read registration back with
*                      cellular_hal_get_current_plmn_information() before retrying.
*
* @pre cellular_hal_init() has returned RETURN_OK. The values are only meaningful
*      where the modem has a serving cell.
* @post On success every member of the structure has been written. On failure its
*       contents are undefined. The call reports state and changes none.
*
* @note Blocking: synchronous. No timeout is specified, per 'Blocking calls' in
*       docs/pages/halSpec.md.
* @note A cell identifier locates a device geographically to within the cell's
*       coverage. A caller should treat it as it treats any location data.
* @warning Not thread safe. Per 'Threading Model' in docs/pages/halSpec.md the
*          obligation is the caller's: serialise this call against every other Cellular
*          HAL call, including calls issued from another process.
* @see cellular_hal_get_signal_info, cellular_hal_get_current_plmn_information
*/
int cellular_hal_get_cell_location_info(CellLocationInfoStruct *loc_info);

/**
* @brief Applies an operating command to the modem.
*
* The one call that changes the modem's operating state: it takes the modem online or
* offline, places it in low power mode, resets it, or resets it to factory defaults.
* Three of the five commands interrupt service and two of them are destructive, so
* this is not a call to issue speculatively.
*
* @param[in] modem_operating_config variable is from the enumerated datatype needs to pass CellularModemOperatingConfiguration_t to configure modem state.
*                                   \n One of the five members of
*                                   CellularModemOperatingConfiguration_t. Passing
*                                   CELLULAR_MODEM_SET_OFFLINE or either reset command
*                                   ends any registration and any packet data network
*                                   in progress. CELLULAR_MODEM_SET_FACTORY_RESET is
*                                   named as a reset to factory defaults and discards
*                                   the modem's configuration, including profiles
*                                   created through this interface; what
*                                   CELLULAR_MODEM_SET_RESET preserves is not stated by
*                                   this interface, so a caller must not treat it as the
*                                   configuration-preserving variant. This interface
*                                   does not state what a value outside the enumeration
*                                   does.
*
* @returns The status of the operation.
* @retval RETURN_OK    (0) - The command has been accepted. It does not mean the modem
*                      has reached the requested state: this interface publishes no
*                      call that reports the modem's operating mode, and the mode is
*                      only reported back through
*                      cellular_device_open_status_api_callback when a device is
*                      opened.
* @retval RETURN_ERROR (-1) - The call failed. This interface does not state whether
*                      the command took effect, so the outcome is unknown rather than
*                      undone and a caller must not read this value as proof that the
*                      modem is unchanged. This is the interface's only failure code and
*                      it names no cause, so a caller cannot tell from it why the call
*                      failed. The action left to a caller is to read the modem's state
*                      back - cellular_hal_IsModemControlInterfaceOpened(),
*                      cellular_hal_get_current_plmn_information() and
*                      cellular_hal_get_active_card_status() - before retrying.
*
* @pre cellular_hal_init() has returned RETURN_OK.
* @post On success the command is under way. For the reset commands the modem restarts,
*       so the control interface and any started network must be treated as gone and
*       the caller re-establishes them; CELLULAR_MODEM_SET_FACTORY_RESET leaves no
*       profile in place. For CELLULAR_MODEM_SET_RESET this interface does not state
*       what survives, so a caller must not assume that profiles, the card selection or
*       an installed callback are still in place and must not assume that they are gone
*       either: it re-reads the state it depends on. This interface states no completion
*       notification and no time bound for any of the five commands. On failure this
*       interface does not state whether the command was partly applied.
*
* @note Blocking: synchronous. No timeout is specified, per 'Blocking calls' in
*       docs/pages/halSpec.md, and a reset is a hardware operation that may take a
*       perceptible time.
* @warning CELLULAR_MODEM_SET_FACTORY_RESET destroys configuration that this interface
*          cannot restore, including every profile. A caller must not issue it as part
*          of error recovery without having persisted whatever it needs to recreate -
*          per 'Persistence Model' in docs/pages/halSpec.md the HAL persists nothing on
*          the caller's behalf.
* @warning Not thread safe. Per 'Threading Model' in docs/pages/halSpec.md the
*          obligation is the caller's: serialise this call against every other Cellular
*          HAL call, including calls issued from another process.
* @see cellular_hal_modem_reset, cellular_hal_modem_factory_reset, cellular_hal_sim_power_enable
*/
int cellular_hal_set_modem_operating_configuration(CellularModemOperatingConfiguration_t modem_operating_config);

/**
* @brief Reads the modem's International Mobile Equipment Identity.
*
* Returns the identity of the modem hardware itself, which is the value a management
* layer reports as the device's equipment identifier. It says nothing about the card
* or the subscription: those are cellular_hal_get_modem_current_iccid() and
* cellular_hal_get_modem_current_msisdn().
*
* @param[out] imei variable is a character pointer, needs to return Modem IMEI value on this input.
*                  \n The buffer size should be atleast 20 bytes long. Example: "010928/00/389023/36".
*                  \n The caller allocates the buffer and retains ownership of it. The
*                  implementation writes the identity into it during the call. Whether
*                  it retains the pointer afterwards is not specified by this interface,
*                  so - per 'Memory Model' in docs/pages/halSpec.md - a caller cannot
*                  rely on either behaviour and keeps the buffer valid and unmoved for
*                  as long as it uses this interface. Must not be NULL.
*                  \n Sizing it correctly is the caller's responsibility and this call
*                  gives the implementation no way to help: no length argument
*                  accompanies the pointer, so the implementation cannot bound its write
*                  to what the caller allocated, and a buffer shorter than the value
*                  written is overrun. Two forms are in play. A bare IMEI is 15 decimal
*                  digits (3GPP TS 23.003 clause 6.2.1 - an 8-digit Type Allocation
*                  Code, a 6-digit serial number and a check digit), which occupies 16
*                  bytes if a terminator accompanies it. The separator-delimited printed
*                  form this interface illustrates above is 19 characters, 20 with one.
*                  This interface does not state which form an implementation returns,
*                  so 20 bytes is the minimum a caller may allocate, and allocating
*                  BUFLEN_32 (32) leaves margin for a longer separator convention.
*                  \n This interface does not state whether the value written is
*                  zero-terminated, and no length accompanies it, so a caller cannot
*                  learn from this interface how many bytes were written: it must bound
*                  every read by the size it allocated and must not assume a terminator.
*                  Clearing the buffer before the call is a caller-side measure rather
*                  than anything this interface establishes.
*
* @returns The status of the operation.
* @retval RETURN_OK    (0) - The identity has been written into the buffer and may be
*                      read within the size the caller allocated. This interface does
*                      not state whether it is zero-terminated.
* @retval RETURN_ERROR (-1) - Nothing may be read from the buffer; this interface does
*                      not state whether it was partly written, so a caller must not
*                      use its contents even as a prefix. This is the interface's only
*                      failure code and it names no cause, so a caller cannot tell from
*                      it why the read failed. The action left to a caller is to read
*                      the control interface back with
*                      cellular_hal_IsModemControlInterfaceOpened() before retrying.
*
* @pre cellular_hal_init() has returned RETURN_OK, and the buffer is allocated and at
*      least 20 bytes long. Supplying a shorter buffer is not diagnosable by the
*      implementation and corrupts memory beyond it.
* @post On success the implementation has written the identity into the buffer. This
*       interface states neither how many bytes it wrote nor whether it terminated
*       them, so a caller reads within the size it allocated. On failure the buffer's
*       contents are undefined. The call reports state and changes none - the identity
*       is hardware and this interface publishes no call that changes it.
*
* @note Blocking: synchronous. No timeout is specified, per 'Blocking calls' in
*       docs/pages/halSpec.md.
* @warning An IMEI identifies a specific piece of equipment and is treated as
*          personal data in several jurisdictions. A caller must not write it to a log.
* @warning Not thread safe. Per 'Threading Model' in docs/pages/halSpec.md the
*          obligation is the caller's: serialise this call against every other Cellular
*          HAL call, including calls issued from another process.
* @see cellular_hal_get_device_imei_sv, cellular_hal_get_modem_current_iccid
*/
int cellular_hal_get_device_imei ( char *imei );

/**
* @brief Reads the modem's International Mobile Equipment Identity and Software Version.
*
* The IMEISV form of the equipment identity: the same equipment, with the software
* version number that the plain IMEI's check digit position is given over to. A
* caller that has to distinguish two modems running different firmware reads this
* rather than cellular_hal_get_device_imei().
*
* @param[out] imei_sv variable is a character pointer, needs to return Modem IMEI Software Version value on this input.
*                     \n The buffer size should be atleast 20 bytes long. Example: 36.
*                     \n The caller allocates the buffer and retains ownership of it.
*                     The implementation writes the value into it during the call.
*                     Whether it retains the pointer afterwards is not specified by
*                     this interface, so - per 'Memory Model' in
*                     docs/pages/halSpec.md - a caller cannot rely on either behaviour
*                     and keeps the buffer valid and unmoved for as long as it uses
*                     this interface. Must not be NULL.
*                     \n As with cellular_hal_get_device_imei(), no length argument
*                     accompanies the pointer, so the implementation cannot bound its
*                     write and the caller must allocate for the longest value the
*                     interface admits. A full IMEISV is 16 decimal digits (3GPP TS
*                     23.003 clause 6.2.2 - the 14 digits of the IMEI without its check
*                     digit, followed by a 2-digit Software Version Number), which
*                     occupies 17 bytes if a terminator accompanies it; the example
*                     above is the 2-digit version number alone, 3 bytes with one. This
*                     interface does not state which of the two an implementation
*                     returns, nor whether it uses the separator-delimited printed form,
*                     so 20 bytes is the minimum a caller may allocate and BUFLEN_32
*                     (32) leaves margin.
*                     \n This interface does not state whether the value written is
*                     zero-terminated, and no length accompanies it, so a caller must
*                     bound every read by the size it allocated and must not assume a
*                     terminator.
*
* @returns The status of the operation.
* @retval RETURN_OK    (0) - The value has been written into the buffer and may be read
*                      within the size the caller allocated. This interface does not
*                      state whether it is zero-terminated.
* @retval RETURN_ERROR (-1) - Nothing may be read from the buffer; this interface does
*                      not state whether it was partly written. This is the interface's
*                      only failure code and it names no cause, so a caller cannot tell
*                      from it why the read failed. The action left to a caller is to
*                      read the control interface back with
*                      cellular_hal_IsModemControlInterfaceOpened() before retrying.
*
* @pre cellular_hal_init() has returned RETURN_OK, and the buffer is allocated and at
*      least 20 bytes long. A shorter buffer is not diagnosable by the implementation
*      and corrupts memory beyond it.
* @post On success the implementation has written the value into the buffer. This
*       interface states neither how many bytes it wrote nor whether it terminated
*       them, so a caller reads within the size it allocated. On failure the buffer's
*       contents are undefined. The call reports state and changes none.
*
* @note Blocking: synchronous. No timeout is specified, per 'Blocking calls' in
*       docs/pages/halSpec.md.
* @note The software version this reports is the modem's, and this interface does not
*       state its relationship to the string
*       cellular_hal_get_modem_firmware_version() returns.
* @warning Where the value carries the full IMEISV it identifies a specific piece of
*          equipment. A caller must not write it to a log.
* @warning Not thread safe. Per 'Threading Model' in docs/pages/halSpec.md the
*          obligation is the caller's: serialise this call against every other Cellular
*          HAL call, including calls issued from another process.
* @see cellular_hal_get_device_imei, cellular_hal_get_modem_firmware_version
*/
int cellular_hal_get_device_imei_sv ( char *imei_sv );

/**
* @brief Reads the Integrated Circuit Card Identifier of the active card.
*
* Identifies the card the modem is currently using, in full. This is the call to use
* rather than the iccid member of CellularUICCSlotInfoStruct, whose 20 bytes cannot
* hold a full-length identifier together with its terminator.
*
* @param[out] iccid variable is a character pointer, needs to return currently choosed ICCID value on this input.
*                   \n The buffer size should be atleast 21 bytes long. Example: "8901260410032962638F".
*                   \n The caller allocates the buffer and retains ownership of it.
*                   The implementation writes the identifier into it during the call.
*                   Whether it retains the pointer afterwards is not specified by this
*                   interface, so - per 'Memory Model' in docs/pages/halSpec.md - a
*                   caller cannot rely on either behaviour and keeps the buffer valid
*                   and unmoved for as long as it uses this interface. Must not be
*                   NULL. No length argument accompanies the pointer, so the
*                   implementation cannot bound its write: the 21-byte minimum is not
*                   advisory. It is the full length an identifier can occupy - an
*                   ICCID is at most 20 characters (ITU-T E.118), and the example
*                   above is exactly 20 - plus one byte of headroom for a terminator.
*                   Allocating BUFLEN_32 (32) leaves margin.
*                   \n This interface does not state whether the value written is
*                   zero-terminated, and no length accompanies it, so a caller must
*                   bound every read by the size it allocated and must not assume a
*                   terminator.
*
* @returns The status of the operation.
* @retval RETURN_OK    (0) - The identifier has been written into the buffer and may be
*                      read within the size the caller allocated. This interface does
*                      not state whether it is zero-terminated.
* @retval RETURN_ERROR (-1) - Nothing may be read from the buffer; this interface does
*                      not state whether it was partly written. This is the interface's
*                      only failure code and it names no cause, so a caller cannot tell
*                      from it why the read failed. The action left to a caller is to
*                      read the card status with cellular_hal_get_active_card_status()
*                      before retrying.
*
* @pre cellular_hal_init() has returned RETURN_OK, a usable card is active, and the
*      buffer is allocated and at least 21 bytes long. A shorter buffer is not
*      diagnosable by the implementation and corrupts memory beyond it.
* @post On success the implementation has written the identifier into the buffer. This
*       interface states neither how many bytes it wrote nor whether it terminated
*       them, so a caller reads within the size it allocated. On failure the buffer's
*       contents are undefined. The call reports state and changes none.
*
* @note Blocking: synchronous. No timeout is specified, per 'Blocking calls' in
*       docs/pages/halSpec.md.
* @note The value can change: powering a slot, selecting another slot or resetting the
*       modem can change which card is active, so a caller must not cache it.
* @warning An ICCID identifies a subscriber's card. A caller must not write it to a
*          log.
* @warning Not thread safe. Per 'Threading Model' in docs/pages/halSpec.md the
*          obligation is the caller's: serialise this call against every other Cellular
*          HAL call, including calls issued from another process.
* @see cellular_hal_get_modem_current_msisdn, cellular_hal_get_uicc_slot_info
*/
int cellular_hal_get_modem_current_iccid ( char *iccid );

/**
* @brief Reads the Mobile Subscriber ISDN number of the active subscription.
*
* Returns the dialable number associated with the active card - the subscription's
* identity rather than the card's or the equipment's.
*
* @param[out] msisdn variable is a character pointer, needs to return currently choosed MSISDN value on this input.
*                    \n The buffer size should be atleast 20 bytes long. Example: "9386720110".
*                    \n The caller allocates the buffer and retains ownership of it.
*                    The implementation writes the number into it during the call.
*                    Whether it retains the pointer afterwards is not specified by
*                    this interface, so per 'Memory Model' in docs/pages/halSpec.md a
*                    caller cannot rely on either behaviour and keeps the buffer valid
*                    and unmoved for as long as it uses this interface. Must not be
*                    NULL. No length argument accompanies the pointer, so the
*                    implementation cannot bound its write and the 20-byte minimum
*                    must be honoured. An MSISDN is at most 15 digits (3GPP TS 23.003
*                    clause 3.3), 16 bytes if a terminator accompanies it, so 20 bytes
*                    accommodates the longest number and matches the declared size of
*                    the msisdn member of CellularUICCSlotInfoStruct. This interface
*                    does not state whether the value is returned in international
*                    format or with a leading plus sign, so a caller must not assume
*                    either.
*                    \n This interface does not state whether the value written is
*                    zero-terminated, and no length accompanies it, so a caller must
*                    bound every read by the size it allocated and must not assume a
*                    terminator.
*
* @returns The status of the operation.
* @retval RETURN_OK    (0) - The number has been written into the buffer and may be read
*                      within the size the caller allocated. This interface does not
*                      state whether it is zero-terminated. A zero-length value is a
*                      legitimate result: not every subscription has an MSISDN stored on
*                      the card.
* @retval RETURN_ERROR (-1) - Nothing may be read from the buffer; this interface does
*                      not state whether it was partly written. This is the interface's
*                      only failure code and it names no cause, so a caller cannot tell
*                      from it why the read failed. The action left to a caller is to
*                      read the card status with cellular_hal_get_active_card_status()
*                      before retrying.
*
* @pre cellular_hal_init() has returned RETURN_OK, a usable card is active, and the
*      buffer is allocated and at least 20 bytes long. A shorter buffer is not
*      diagnosable by the implementation and corrupts memory beyond it.
* @post On success the implementation has written the number into the buffer. This
*       interface states neither how many bytes it wrote nor whether it terminated
*       them, so a caller reads within the size it allocated. On failure the buffer's
*       contents are undefined. The call reports state and changes none.
*
* @note Blocking: synchronous. No timeout is specified, per 'Blocking calls' in
*       docs/pages/halSpec.md.
* @warning An MSISDN identifies a subscriber. A caller must not write it to a log.
* @warning Not thread safe. Per 'Threading Model' in docs/pages/halSpec.md the
*          obligation is the caller's: serialise this call against every other Cellular
*          HAL call, including calls issued from another process.
* @see cellular_hal_get_modem_current_iccid, cellular_hal_get_uicc_slot_info
*/
int cellular_hal_get_modem_current_msisdn ( char *msisdn );

/**
* @brief Reads the traffic counters and maximum bit rates of the packet connection.
*
* Fills one caller-supplied CellularPacketStatsStruct with the byte, packet and
* dropped-packet counters in both directions and the maximum upstream and downstream
* bit rates. It is how a caller measures a bearer it has started; the counters are
* cumulative, so a rate is the difference between two reads.
*
* @param[out] network_packet_stats variable is a pointer, needs to parse CellularPacketStatsStruct structure to get packet statistics information.
*                                  \n The caller allocates the structure and retains
*                                  ownership of it. The implementation writes through
*                                  the pointer during the call. Whether it retains the
*                                  pointer afterwards is not specified by this
*                                  interface, so per 'Memory Model' in
*                                  docs/pages/halSpec.md a caller cannot rely on either
*                                  behaviour and keeps the storage valid and unmoved
*                                  for as long as it uses this interface. Must point to
*                                  storage for a whole CellularPacketStatsStruct. This
*                                  interface does not state whether NULL is accepted.
*                                  Every member is an unsigned long and this interface
*                                  states neither the epoch a counter is measured from
*                                  nor its behaviour on overflow, so a caller
*                                  differencing two reads must tolerate a counter that
*                                  has wrapped or been reset beneath it.
*
* @returns The status of the operation.
* @retval RETURN_OK    (0) - The structure has been populated and may be read.
* @retval RETURN_ERROR (-1) - Nothing may be read from the structure; it may have been
*                      partly written, so a caller must not use a subset of the
*                      counters. This is the interface's only failure code and it names
*                      no cause, so a caller cannot tell from it why the read failed.
*                      The action left to a caller is to read the interface status with
*                      cellular_hal_get_current_modem_interface_status() before
*                      retrying.
*
* @pre cellular_hal_init() has returned RETURN_OK. The counters are only meaningful
*      for a network that has been started with cellular_hal_start_network(); before
*      that a caller should expect failure or zeroes rather than treat the call as
*      broken.
* @post On success every member of the structure has been written. On failure its
*       contents are undefined. The call reports state and changes none - this
*       interface publishes no call that resets a counter.
*
* @note Blocking: synchronous. No timeout is specified, per 'Blocking calls' in
*       docs/pages/halSpec.md.
* @note The counters are not stated to be read atomically with respect to each other,
*       so a caller must not assume that bytes and packets in one snapshot are exactly
*       consistent.
* @warning Not thread safe. Per 'Threading Model' in docs/pages/halSpec.md the
*          obligation is the caller's: serialise this call against every other Cellular
*          HAL call, including calls issued from another process.
* @see cellular_hal_start_network, cellular_hal_stop_network
*/
int cellular_hal_get_packet_statistics( CellularPacketStatsStruct *network_packet_stats );

/**
* @brief Reads the operational status of the modem's network interface.
*
* Reports the interface in the vocabulary a TR-181 style management layer expects -
* up, down, dormant, not present, lower layer down, error or unknown - so a caller can
* surface it without translation. It reports the interface, not the registration:
* cellular_hal_get_current_plmn_information() reports the latter.
*
* @param[out] status variable is a pointer to the structure CellularInterfaceStatus_t, needs to assign modem current registration status.
*                    \n CellularInterfaceStatus_t is an enumeration rather than a
*                    structure. The caller allocates the variable and retains ownership
*                    of it. The implementation writes through the pointer during the
*                    call. Whether it retains the pointer afterwards is not specified by
*                    this interface, so per 'Memory Model' in docs/pages/halSpec.md a
*                    caller cannot rely on either behaviour and keeps the storage valid
*                    and unmoved for as long as it uses this interface. This interface
*                    does not state whether NULL is accepted.
*                    IF_UNKNOWN is a legitimate written value and not an error, so a
*                    caller must inspect the value rather than infer success from the
*                    status code alone.
*
* @returns The status of the operation.
* @retval RETURN_OK    (0) - The interface status has been written and may be read.
* @retval RETURN_ERROR (-1) - The call failed and nothing may be read through status.
*                      Note the distinction this interface draws through its own
*                      enumeration: an interface that is down, absent or in error is
*                      reported as a value through RETURN_OK, so RETURN_ERROR means the
*                      status could not be determined at all. Beyond that it names no
*                      cause, so a caller cannot tell from it why. The action left to a
*                      caller is to read the control interface back with
*                      cellular_hal_IsModemControlInterfaceOpened() before retrying.
*
* @pre cellular_hal_init() has returned RETURN_OK.
* @post On success the enumeration is written. On failure the pointed-to value is
*       undefined. The call reports state and changes none.
*
* @note Blocking: synchronous. No timeout is specified, per 'Blocking calls' in
*       docs/pages/halSpec.md.
* @note This interface specifies no transition set over CellularInterfaceStatus_t, so a
*       caller may read the current value but must not infer which status can follow
*       which.
* @warning Not thread safe. Per 'Threading Model' in docs/pages/halSpec.md the
*          obligation is the caller's: serialise this call against every other Cellular
*          HAL call, including calls issued from another process.
* @see cellular_hal_get_current_plmn_information, cellular_hal_monitor_device_registration
*/
int cellular_hal_get_current_modem_interface_status( CellularInterfaceStatus_t *status );

/**
* @brief Attaches the modem to the network.
*
* Requests network registration explicitly, rather than waiting for the modem to
* attach of its own accord. It takes no argument: the network, the card and the
* profile in use are all implicit in the layer's context, so this call cannot express
* which network to attach to.
*
* @returns The status of the operation.
* @retval RETURN_OK    (0) - The attach has been accepted. It does not mean the modem
*                      is registered: registration completes asynchronously and is
*                      reported as DEVICE_NAS_STATUS_REGISTERED through the callback
*                      installed by cellular_hal_monitor_device_registration(), or read
*                      with cellular_hal_get_current_plmn_information().
* @retval RETURN_ERROR (-1) - The call failed. This interface does not state whether
*                      the attach was begun, so the outcome is unknown rather than
*                      undone. This is the interface's only failure code and it names
*                      no cause, so a caller cannot tell from it why the call failed.
*                      The action left to a caller is to read the card status with
*                      cellular_hal_get_active_card_status() and the interface status
*                      with cellular_hal_get_current_modem_interface_status() before
*                      retrying rather than repeat the call immediately.
*
* @pre cellular_hal_init() has returned RETURN_OK. A usable card is what makes an
*      attach possible, but this interface states no ordering constraint that requires
*      one ('Method Sequencing' in docs/pages/halSpec.md), so a caller must not read a
*      failure as proof that the card is at fault.
* @post On success the attach is under way. This interface states no time bound for it
*       and no completion notification of its own. On failure this interface does not
*       state whether a partial attach remains.
*
* @note Blocking: synchronous. No timeout is specified, per 'Blocking calls' in
*       docs/pages/halSpec.md, and network registration can take a perceptible time.
* @note This interface does not state what an attach does to a packet data network
*       already running, nor whether an attach is required before
*       cellular_hal_start_network() succeeds.
* @warning Not thread safe. Per 'Threading Model' in docs/pages/halSpec.md the
*          obligation is the caller's: serialise this call against every other Cellular
*          HAL call, including calls issued from another process.
* @see cellular_hal_set_modem_network_detach, cellular_hal_monitor_device_registration
*/
int cellular_hal_set_modem_network_attach( void );

/**
* @brief Detaches the modem from the network.
*
* The counterpart of cellular_hal_set_modem_network_attach(): it gives up network
* registration while leaving the modem powered and its control interface open. It
* takes no argument, so it detaches from whatever network the modem is registered on.
*
* @returns The status of the operation.
* @retval RETURN_OK    (0) - The detach has been accepted. This interface does not
*                      state whether registration is gone when the call returns, so a
*                      caller that needs to know reads
*                      cellular_hal_get_current_plmn_information() or waits for
*                      DEVICE_NAS_STATUS_NOT_REGISTERED on the registration callback.
* @retval RETURN_ERROR (-1) - The call failed. This interface does not state whether
*                      the detach took effect, so the outcome is unknown rather than
*                      undone and a caller must not read this value as proof that the
*                      modem is still registered. This is the interface's only failure
*                      code and it names no cause, so a caller cannot tell from it why
*                      the call failed. The action left to a caller is to read the
*                      registration state back with
*                      cellular_hal_get_current_plmn_information() rather than retry
*                      blindly.
*
* @pre cellular_hal_init() has returned RETURN_OK. Detaching a modem that is not
*      registered is not specified by this interface.
* @post On success the detach is under way. Any packet data network depends on the
*       registration, so a caller should stop its networks with
*       cellular_hal_stop_network() before detaching; this interface does not state
*       what happens to a running network otherwise. On failure this interface does not
*       state whether a partial detach remains.
*
* @note Blocking: synchronous. No timeout is specified, per 'Blocking calls' in
*       docs/pages/halSpec.md.
* @warning Not thread safe. Per 'Threading Model' in docs/pages/halSpec.md the
*          obligation is the caller's: serialise this call against every other Cellular
*          HAL call, including calls issued from another process.
* @see cellular_hal_set_modem_network_attach, cellular_hal_stop_network
*/
int cellular_hal_set_modem_network_detach( void );

/**
* @brief Reads the modem's firmware version.
*
* Returns the vendor's version string for the software running on the modem, which is
* what a caller records when it has to correlate behaviour with a firmware level.
*
* @param[out] firmware_version variable is a character pointer contains firmware version of modem.
*                              \n The buffer size should be atleast 128 bytes long. Example: "v2.1.3".
*                              \n The caller allocates the buffer and retains
*                              ownership of it. The implementation writes the version
*                              string into it during the call. Whether it retains the
*                              pointer afterwards is not specified by this interface,
*                              so per 'Memory Model' in docs/pages/halSpec.md a caller
*                              cannot rely on either behaviour and keeps the buffer
*                              valid and unmoved for as long as it uses this
*                              interface. Must not be NULL. No length argument
*                              accompanies the pointer, so the implementation cannot
*                              bound its write and the 128-byte minimum - which is
*                              BUFLEN_128 - must be honoured. This interface states no
*                              maximum length and no format for the string, so a
*                              caller must treat it as opaque vendor text: it must not
*                              parse it for ordering and must not assume the example's
*                              shape.
*                              \n This interface does not state whether the value
*                              written is zero-terminated, and no length accompanies
*                              it, so a caller must bound every read by the size it
*                              allocated and must not assume a terminator.
*
* @returns The status of the operation.
* @retval RETURN_OK    (0) - The version string has been written into the buffer and may
*                      be read within the size the caller allocated. This interface does
*                      not state whether it is zero-terminated.
* @retval RETURN_ERROR (-1) - The call failed and nothing may be read from the buffer;
*                      this interface does not state whether it was partly written.
*                      This is the interface's only failure code and it names no cause,
*                      so a caller cannot tell from it why the read failed. The action
*                      left to a caller is to read the control interface back with
*                      cellular_hal_IsModemControlInterfaceOpened() before retrying.
*
* @pre cellular_hal_init() has returned RETURN_OK, and the buffer is allocated and at
*      least 128 bytes long. A shorter buffer is not diagnosable by the implementation
*      and corrupts memory beyond it.
* @post On success the implementation has written the version string into the buffer.
*       This interface states neither how many bytes it wrote nor whether it terminated
*       them, so a caller reads within the size it allocated. On failure the buffer's
*       contents are undefined. The call reports state and changes none.
*
* @note Blocking: synchronous. No timeout is specified, per 'Blocking calls' in
*       docs/pages/halSpec.md.
* @note This interface publishes no call that updates modem firmware, so nothing here
*       changes the value. It does not state whether cellular_hal_modem_reset() or
*       cellular_hal_modem_factory_reset() can change it, so a caller that cached the
*       string re-reads it after a reset rather than assuming it still holds.
* @warning Not thread safe. Per 'Threading Model' in docs/pages/halSpec.md the
*          obligation is the caller's: serialise this call against every other Cellular
*          HAL call, including calls issued from another process.
* @see cellular_hal_get_device_imei_sv
*/
int cellular_hal_get_modem_firmware_version(char *firmware_version);

/**
* @brief Reads the identity and registration state of the serving network.
*
* Fills one caller-supplied CellularCurrentPlmnInfoStruct with the serving network's
* name, Mobile Country Code and Mobile Network Code, whether the device is registered
* on it, which domains that registration covers, whether it is a roaming
* registration, and the area and cell identifiers. It is the synchronous counterpart
* of the registration callback: the callback reports a change, this call reports the
* current state.
*
* @param[out] plmn_info is a pointer to structure CellularCurrentPlmnInfoStruct that the implementation fills with the current plmn network information from the modem.
*                       \n This is an output: the caller allocates the structure and
*                       retains ownership of it. The implementation writes the serving
*                       network's details through the pointer during the call. Whether
*                       it retains the pointer afterwards is not specified by this
*                       interface, so per 'Memory Model' in docs/pages/halSpec.md a
*                       caller cannot rely on either behaviour and keeps the storage
*                       valid and unmoved for as long as it uses this interface. Must
*                       point to storage for a whole CellularCurrentPlmnInfoStruct;
*                       nothing in it is read on entry and no function pointer is
*                       involved. This interface does not state whether NULL is
*                       accepted.
*                       \n A caller reads the registration_status member first: where
*                       it is DEVICE_NOT_REGISTERED the remaining members do not
*                       describe a serving network. The MCC and MNC members are numbers
*                       rather than text, so a leading zero is not recoverable from
*                       them and the printed PLMN identity cannot be reconstructed from
*                       the structure alone.
*
* @returns The status of the operation.
* @retval RETURN_OK    (0) - The structure has been populated and may be read.
*                      DEVICE_NOT_REGISTERED is a legitimate populated result and not a
*                      failure.
* @retval RETURN_ERROR (-1) - The call failed and nothing may be read from the
*                      structure; it may have been partly written. This is the
*                      interface's only failure code and it names no cause, so a caller
*                      cannot tell from it why the read failed. The action left to a
*                      caller is to read the control interface back with
*                      cellular_hal_IsModemControlInterfaceOpened() before retrying.
*
* @pre cellular_hal_init() has returned RETURN_OK.
* @post On success every member of the structure has been written. On failure its
*       contents are undefined. The call reports state and changes none.
*
* @note Blocking: synchronous. No timeout is specified, per 'Blocking calls' in
*       docs/pages/halSpec.md.
* @note The cell_id member here is an unsigned long while the identifiers in
*       CellLocationInfoStruct are unsigned int, and this interface does not state
*       whether the two agree.
* @warning Not thread safe. Per 'Threading Model' in docs/pages/halSpec.md the
*          obligation is the caller's: serialise this call against every other Cellular
*          HAL call, including calls issued from another process.
* @see cellular_hal_monitor_device_registration, cellular_hal_get_cell_location_info
*/
int cellular_hal_get_current_plmn_information(CellularCurrentPlmnInfoStruct *plmn_info);

/**
* @brief Reads the networks a scan has found.
*
* Reports the visible networks as an array reached through the caller's pointer
* together with the number of elements in it - name, Mobile Country Code, Mobile
* Network Code and whether the device is permitted to use each. A scan describes what
* the radio can see, not what it is registered on, so nothing here implies that an
* attach to a listed network would succeed.
*
* @param[out] network_info is a structure CellularNetworkScanResultInfoStruct filled with available networks information from Modem.
*                          \n The argument is the address of the caller's own
*                          CellularNetworkScanResultInfoStruct pointer, which the
*                          implementation sets to the start of an array of results. The
*                          caller owns that pointer variable, and the implementation
*                          writes through its address during the call. Whether it
*                          retains that address afterwards is not specified by this
*                          interface, so per 'Memory Model' in docs/pages/halSpec.md a
*                          caller cannot rely on either behaviour and keeps the pointer
*                          variable itself valid and unmoved for as long as it uses
*                          this interface. The array the variable is pointed at is a
*                          separate question, addressed next.
*                          \n Allocation of the array itself is not established by this
*                          interface. It states neither which side allocates it, nor -
*                          if the implementation allocates it - which function releases
*                          it, and 'Memory Model' in docs/pages/halSpec.md records the
*                          same gap from the caller's side. What is established is only
*                          this: on RETURN_OK the pointer addresses
*                          total_network_count elements that may be read.
*                          \n No lifetime is specified. This interface does not state
*                          how long the array stays valid, and it names no event that
*                          ends its validity, so a caller must not substitute a rule of
*                          its own for the missing one. The conservative consequence is
*                          that a caller reads what it needs into its own storage as
*                          soon as the call returns and works from that copy, must not
*                          free the array, must not assume anything else frees it, and
*                          must not retain the pointer. A caller that requires a
*                          defined lifetime has to agree the convention with the
*                          implementation outside this interface.
*                          \n This interface does not state whether NULL is accepted in
*                          place of the address.
* @param[out] total_network_count variable is a unsigned integer pointer filled with total no of available networks.
*                                 \n The possible range of acceptable values is 0 to 1100000. Example: 5
*                                 \n The number of elements available through the array,
*                                 and the only bound on indexing it: this interface
*                                 publishes no terminator element, so a caller reads
*                                 this value before touching the array and indexes from
*                                 0 to one less than it. The upper bound above is the
*                                 one this interface states; it is far above any
*                                 plausible scan result and must not be read as a
*                                 promise that a smaller allocation suffices for the
*                                 array, whose size is the implementation's business.
*                                 The caller allocates the unsigned int and retains
*                                 ownership of it. This interface does not state what is
*                                 written on failure, nor whether a count of 0 leaves
*                                 the array pointer unchanged or sets it to NULL, so a
*                                 caller must initialise both arguments and treat a zero
*                                 count as "no networks readable" rather than
*                                 dereference the pointer.
*
* @returns The status of the operation.
* @retval RETURN_OK    (0) - The count has been written and, where it is greater than
*                      zero, the array may be read for that many elements.
* @retval RETURN_ERROR (-1) - The call failed and neither output may be read. This is
*                      the interface's only failure code and it names no cause, so a
*                      caller cannot tell from it why the call failed. This interface
*                      also does not state how a scan that found nothing is reported,
*                      so a caller must not read a failure as an empty result.
*
* @pre cellular_hal_init() has returned RETURN_OK. This interface declares no call that
*      starts a scan, so it does not state whether this call performs one or reads the
*      result of one the modem performed already.
* @post On success the count is written, and the pointer is set where the count is
*       greater than zero. On failure both outputs are undefined. The call reports state
*       and changes none.
*
* @note Blocking: synchronous, and this call is the one most likely to be slow: where an
*       implementation performs the scan inline it must search the bands, which on
*       cellular hardware takes seconds rather than milliseconds. No timeout is
*       specified, per 'Blocking calls' in docs/pages/halSpec.md, so a caller that
*       cannot afford the wait must not issue it on a latency-sensitive thread.
* @note A scan can interrupt service on some modems. This interface does not state
*       whether a scan affects an existing registration or a running packet data
*       network.
* @warning Not thread safe. Per 'Threading Model' in docs/pages/halSpec.md the
*          obligation is the caller's: serialise this call against every other Cellular
*          HAL call, including calls issued from another process. The array compounds
*          that obligation: because this interface specifies no lifetime for it, the
*          effect of a concurrent call on a list another thread is still reading is
*          unspecified too, so a caller must finish copying what it needs before it
*          issues its next call.
* @see cellular_hal_get_current_plmn_information, cellular_hal_set_modem_network_attach
*/
int cellular_hal_get_available_networks_information(CellularNetworkScanResultInfoStruct **network_info, unsigned int *total_network_count);

/**
* @brief Reads the modem's preferred radio access technologies.
*
* Returns the preference that cellular_hal_set_modem_preferred_radio_technology()
* writes: which technologies the modem should choose between, as text. It is a
* preference and not an observation - the technology actually in use is reported by
* cellular_hal_get_modem_current_radio_technology().
*
* @param[out] preferred_rat variable is a character array contains preferred technology.
*                           \n The buffer size sholud be atleast 128 bytes long.
*                           \n Possible combination of strings: AUTO, CDMA20001X, EVDO, GSM, UMTS, LTE
*                           \n Example: "{UMTS,LTE / WCDMA,LTE}".
*                           \n The caller allocates the buffer and retains ownership
*                           of it. The implementation writes the preference into it
*                           during the call. Whether it retains the pointer afterwards
*                           is not specified by this interface, so per 'Memory Model'
*                           in docs/pages/halSpec.md a caller cannot rely on either
*                           behaviour and keeps the buffer valid and unmoved for as
*                           long as it uses this interface. Must not be NULL. No
*                           length argument accompanies the pointer, so the
*                           implementation cannot bound its write and the 128-byte
*                           minimum - which is BUFLEN_128 - must be honoured. This
*                           interface does not state whether the value written is
*                           zero-terminated, so a caller must bound every read by the
*                           size it allocated and must not assume a terminator.
*                           \n The vocabulary above is the whole of what this
*                           interface publishes, and it has no 5G token even though
*                           CellularPrefAccessTechnology_t has PREF_NR, so a caller
*                           must not expect the two vocabularies to correspond. This
*                           interface does not state the separator or grouping syntax
*                           beyond the example, so a caller should treat the string as
*                           opaque unless it has agreed the syntax with the
*                           implementation.
*
* @returns The status of the operation.
* @retval RETURN_OK    (0) - The preference string has been written into the buffer and
*                      may be read within the size the caller allocated. This interface
*                      does not state whether it is zero-terminated.
* @retval RETURN_ERROR (-1) - The call failed and nothing may be read from the buffer;
*                      this interface does not state whether it was partly written.
*                      This is the interface's only failure code and it names no cause,
*                      so a caller cannot tell from it why the read failed. The action
*                      left to a caller is to read the control interface back with
*                      cellular_hal_IsModemControlInterfaceOpened() before retrying.
*
* @pre cellular_hal_init() has returned RETURN_OK, and the buffer is allocated and at
*      least 128 bytes long. A shorter buffer is not diagnosable by the implementation
*      and corrupts memory beyond it.
* @post On success the implementation has written the preference into the buffer. This
*       interface states neither how many bytes it wrote nor whether it terminated them,
*       so a caller reads within the size it allocated. On failure the buffer's contents
*       are undefined. The call reports state and changes none.
*
* @note Blocking: synchronous. No timeout is specified, per 'Blocking calls' in
*       docs/pages/halSpec.md.
* @warning Not thread safe. Per 'Threading Model' in docs/pages/halSpec.md the
*          obligation is the caller's: serialise this call against every other Cellular
*          HAL call, including calls issued from another process.
* @see cellular_hal_set_modem_preferred_radio_technology, cellular_hal_get_modem_supported_radio_technology
*/
int cellular_hal_get_modem_preferred_radio_technology( char *preferred_rat );

/**
* @brief Sets the modem's preferred radio access technologies.
*
* Writes the preference the modem uses when choosing a technology, as text in the
* vocabulary this interface publishes. The preference is advisory in a specific,
* stated way: a technology the modem does not support is not rejected but replaced -
* the implementation sets AUTO instead - so a caller that needs to know what took
* effect reads the value back.
*
* @param[in] preferred_rat variable is a character array contains preferred technology. Should be part of supported RAT otherwise AUTO will be set.
*                          \n The buffer size sholud be atleast 128 bytes long.
*                          \n Possible combination of strings: AUTO, CDMA20001X, EVDO, GSM, UMTS, LTE
*                          \n Example: "{LTE / AUTO}".
*                          \n This is an input: the caller supplies a zero-terminated
*                          string and retains ownership of the buffer. The declaration
*                          is not const-qualified, but this interface states no output
*                          through the pointer, so a caller must not expect a value to
*                          be written back. The implementation reads the string during
*                          the call. This interface does not specify whether it retains
*                          the pointer afterwards, so a caller must not assume either
*                          behaviour: conservatively the buffer stays allocated and
*                          unmodified while the caller continues to use this
*                          interface.
*                          \n The string must be zero-terminated within the size stated
*                          above, so it carries at most 127 characters; this interface
*                          gives the implementation no length argument, so an
*                          unterminated buffer is read past its end. The vocabulary
*                          above is the whole of what this interface publishes and has
*                          no 5G token. This interface does not state whether an
*                          unrecognised token is rejected or silently replaced along
*                          with an unsupported one, so a caller should verify the result
*                          rather than assume it.
*
* @returns The status of the operation.
* @retval RETURN_OK    (0) - The preference has been accepted. It does not mean the
*                      requested technologies were set: an unsupported technology is
*                      replaced by AUTO, which is also reported as success, so a caller
*                      reads the value back with
*                      cellular_hal_get_modem_preferred_radio_technology() to learn what
*                      took effect.
* @retval RETURN_ERROR (-1) - The call failed. This interface does not state whether
*                      the preference took effect, so the outcome is unknown rather
*                      than undone and a caller must not read this value as proof that
*                      the previous preference is intact. This is the interface's only
*                      failure code and it names no cause, so a caller cannot tell from
*                      it why the call failed. The action left to a caller is to read
*                      the preference back with
*                      cellular_hal_get_modem_preferred_radio_technology() rather than
*                      retry blindly.
*
* @pre cellular_hal_init() has returned RETURN_OK, and preferred_rat points to a
*      zero-terminated string of at most 127 characters. Supplying an unterminated
*      buffer is not diagnosable by the implementation and reads memory beyond it.
* @post On success the preference is applied, possibly as AUTO rather than as
*       requested. Changing the preferred technology can force the modem to reselect,
*       which can interrupt registration and any packet data network; this interface
*       states no notification for that. On failure this interface does not state
*       whether the preference was partly applied.
*
* @note Blocking: synchronous. No timeout is specified, per 'Blocking calls' in
*       docs/pages/halSpec.md, and a reselection can take a perceptible time.
* @warning Not thread safe. Per 'Threading Model' in docs/pages/halSpec.md the
*          obligation is the caller's: serialise this call against every other Cellular
*          HAL call, including calls issued from another process.
* @see cellular_hal_get_modem_preferred_radio_technology, cellular_hal_get_modem_supported_radio_technology
*/
int cellular_hal_set_modem_preferred_radio_technology( char *preferred_rat );

/**
* @brief Reads the radio access technology the modem is currently using for data.
*
* Reports what the modem actually settled on, as opposed to the preference
* cellular_hal_get_modem_preferred_radio_technology() returns and the capability
* cellular_hal_get_modem_supported_radio_technology() returns.
*
* @param[out] current_rat variable is a character pointer contains current technology used for data.
*                         \n The buffer size sholud be atleast 128 bytes long.
*                         \n Possible strings: AUTO, CDMA20001X, EVDO, GSM, UMTS, LTE
*                         \n Example: "LTE".
*                         \n The caller allocates the buffer and retains ownership of
*                         it. The implementation writes the technology name into it
*                         during the call. Whether it retains the pointer afterwards
*                         is not specified by this interface, so per 'Memory Model' in
*                         docs/pages/halSpec.md a caller cannot rely on either
*                         behaviour and keeps the buffer valid and unmoved for as long
*                         as it uses this interface. Must not be NULL. No length
*                         argument accompanies the pointer, so the implementation
*                         cannot bound its write and the 128-byte minimum - which is
*                         BUFLEN_128 - must be honoured. This interface does not state
*                         whether the value written is zero-terminated, so a caller
*                         must bound every read by the size it allocated and must not
*                         assume a terminator.
*                         \n Unlike the preference calls this one names a single
*                         technology, and the vocabulary above has no 5G token. This
*                         interface does not state what is written when no technology is
*                         in use, so a caller should treat a zero-length value as
*                         "none".
*
* @returns The status of the operation.
* @retval RETURN_OK    (0) - The technology name has been written into the buffer and may
*                      be read within the size the caller allocated. This interface does
*                      not state whether it is zero-terminated.
* @retval RETURN_ERROR (-1) - The call failed and nothing may be read from the buffer;
*                      this interface does not state whether it was partly written.
*                      This is the interface's only failure code and it names no cause,
*                      so a caller cannot tell from it why the read failed. The action
*                      left to a caller is to read registration back with
*                      cellular_hal_get_current_plmn_information() before retrying.
*
* @pre cellular_hal_init() has returned RETURN_OK, and the buffer is allocated and at
*      least 128 bytes long. The value is only meaningful where the modem is
*      registered.
* @post On success the implementation has written the technology name into the buffer.
*       This interface states neither how many bytes it wrote nor whether it terminated
*       them, so a caller reads within the size it allocated. On failure the buffer's
*       contents are undefined. The call reports state and changes none.
*
* @note Blocking: synchronous. No timeout is specified, per 'Blocking calls' in
*       docs/pages/halSpec.md.
* @note The value changes as the modem reselects, so a caller must not cache it.
* @warning Not thread safe. Per 'Threading Model' in docs/pages/halSpec.md the
*          obligation is the caller's: serialise this call against every other Cellular
*          HAL call, including calls issued from another process.
* @see cellular_hal_get_modem_preferred_radio_technology, cellular_hal_get_signal_info
*/
int cellular_hal_get_modem_current_radio_technology( char *current_rat );

/**
* @brief Reads the radio access technologies the modem supports.
*
* Reports the modem's capability, which is what a caller needs before setting a
* preference: cellular_hal_set_modem_preferred_radio_technology() silently substitutes
* AUTO for a technology that is not in this set.
*
* @param[out] supported_rat variable is a character pointer contains information about supported RAT.
*                           \n The buffer size sholud be atleast 128 bytes long.
*                           \n Possible combination of strings: AUTO, CDMA20001X, EVDO, GSM, UMTS, LTE
*                           \n Example: "UMTS, LTE".
*                           \n The caller allocates the buffer and retains ownership
*                           of it. The implementation writes the capability string
*                           into it during the call. Whether it retains the pointer
*                           afterwards is not specified by this interface, so per
*                           'Memory Model' in docs/pages/halSpec.md a caller cannot
*                           rely on either behaviour and keeps the buffer valid and
*                           unmoved for as long as it uses this interface. Must not be
*                           NULL. No length argument accompanies the pointer, so the
*                           implementation cannot bound its write and the 128-byte
*                           minimum - which is BUFLEN_128 - must be honoured. This
*                           interface does not state whether the value written is
*                           zero-terminated, so a caller must bound every read by the
*                           size it allocated and must not assume a terminator.
*                           \n The vocabulary above is the whole of what this interface
*                           publishes and has no 5G token, so a modem supporting 5G New
*                           Radio cannot report it here. This interface does not state
*                           the separator syntax beyond the example.
*
* @returns The status of the operation.
* @retval RETURN_OK    (0) - The capability string has been written into the buffer and
*                      may be read within the size the caller allocated. This interface
*                      does not state whether it is zero-terminated.
* @retval RETURN_ERROR (-1) - The call failed and nothing may be read from the buffer;
*                      this interface does not state whether it was partly written.
*                      This is the interface's only failure code and it names no cause,
*                      so a caller cannot tell from it why the read failed. The action
*                      left to a caller is to read the control interface back with
*                      cellular_hal_IsModemControlInterfaceOpened() before retrying.
*
* @pre cellular_hal_init() has returned RETURN_OK, and the buffer is allocated and at
*      least 128 bytes long. A shorter buffer is not diagnosable by the implementation
*      and corrupts memory beyond it.
* @post On success the implementation has written the capability string into the buffer.
*       This interface states neither how many bytes it wrote nor whether it terminated
*       them, so a caller reads within the size it allocated. On failure the buffer's
*       contents are undefined. The call reports state and changes none.
*
* @note Blocking: synchronous. No timeout is specified, per 'Blocking calls' in
*       docs/pages/halSpec.md.
* @note The set is a property of the modem, so it does not change between calls unless
*       the modem's firmware does.
* @warning Not thread safe. Per 'Threading Model' in docs/pages/halSpec.md the
*          obligation is the caller's: serialise this call against every other Cellular
*          HAL call, including calls issued from another process.
* @see cellular_hal_set_modem_preferred_radio_technology, cellular_hal_get_modem_current_radio_technology
*/
int cellular_hal_get_modem_supported_radio_technology ( char *supported_rat );

/**
* @brief Resets the modem to factory defaults.
*
* The most destructive call in this interface: it restarts the modem and discards its
* configuration, which includes every Packet Data Protocol context profile created
* through cellular_hal_profile_create(). It is equivalent to passing
* CELLULAR_MODEM_SET_FACTORY_RESET to
* cellular_hal_set_modem_operating_configuration(), and takes no argument, so there
* is nothing to scope it to.
*
* @returns The status of the operation.
* @retval RETURN_OK    (0) - The factory reset has been accepted. It does not mean the
*                      modem has come back: this interface states no completion
*                      notification and no time bound, so a caller polls
*                      cellular_hal_IsModemDevicePresent() and
*                      cellular_hal_IsModemControlInterfaceOpened() to observe the
*                      modem returning.
* @retval RETURN_ERROR (-1) - The call failed. This interface does not state whether
*                      the factory reset was begun, so the outcome is unknown rather
*                      than undone and a caller must not read this value as proof that
*                      nothing happened. This is the interface's only failure code and
*                      it names no cause, so a caller cannot tell from it why the call
*                      failed. A caller must not assume the previous state is intact,
*                      and this interface specifies no recovery from the unknown state
*                      it leaves: it declares no de-initialization or cleanup call, and
*                      cellular_hal_init()'s own postcondition states that a second
*                      call is not a clean retry of the first, so re-initializing is
*                      not a defined route out either.
*
* @pre cellular_hal_init() has returned RETURN_OK. Anything the caller needs to
*      recreate afterwards must already be persisted: per 'Persistence Model' in
*      docs/pages/halSpec.md the HAL persists nothing on the caller's behalf.
* @post On success the modem restarts with no configuration. The control interface, the
*       card selection, the registration, every profile and any started network must be
*       treated as gone, and the caller re-establishes them from
*       cellular_hal_open_device() onwards. On failure this interface does not state
*       whether the reset was partly performed.
*
* @note Blocking: synchronous. No timeout is specified, per 'Blocking calls' in
*       docs/pages/halSpec.md, and a factory reset is a hardware operation that takes a
*       perceptible time.
* @warning This call destroys configuration that this interface cannot restore. A
*          caller must not issue it as a step in routine error recovery.
* @warning Not thread safe. Per 'Threading Model' in docs/pages/halSpec.md the
*          obligation is the caller's: serialise this call against every other Cellular
*          HAL call, including calls issued from another process. A concurrent call is
*          especially unsafe here, because the reset invalidates the context every other
*          call depends on.
* @see cellular_hal_modem_reset, cellular_hal_set_modem_operating_configuration
*/
int cellular_hal_modem_factory_reset( void );

/**
* @brief Resets the modem.
*
* Restarts the modem, which is the recovery step to reach for when it stops
* responding. It is equivalent to passing CELLULAR_MODEM_SET_RESET to
* cellular_hal_set_modem_operating_configuration(). The only distinction this
* interface draws between it and cellular_hal_modem_factory_reset() is in the names:
* that call is a reset to factory defaults and discards the modem's configuration,
* this one is a reset. What this call preserves is not established - see @post - so a
* caller must not read the naming as a guarantee that configuration, profiles or an
* installed callback survive it.
*
* @returns The status of the operation.
* @retval RETURN_OK    (0) - The reset has been accepted. It does not mean the modem
*                      has come back: this interface states no completion notification
*                      and no time bound, so a caller polls
*                      cellular_hal_IsModemDevicePresent() and
*                      cellular_hal_IsModemControlInterfaceOpened() to observe the
*                      modem returning.
* @retval RETURN_ERROR (-1) - The call failed. This interface does not state whether
*                      the reset was begun, so the outcome is unknown rather than undone
*                      and a caller must not read this value as proof that the modem is
*                      untouched. This is the interface's only failure code and it names
*                      no cause, so a caller cannot tell from it why the call failed.
*                      The action left to a caller is to read the modem's state back
*                      with cellular_hal_IsModemDevicePresent() and
*                      cellular_hal_IsModemControlInterfaceOpened() rather than assume
*                      the previous state is intact.
*
* @pre cellular_hal_init() has returned RETURN_OK.
* @post On success the modem restarts. What survives the restart is not specified by
*       this interface: it does not state whether the modem's configuration, the
*       profiles created through cellular_hal_profile_create(), the card selection, or
*       a callback installed by cellular_hal_open_device(),
*       cellular_hal_select_device_slot(), cellular_hal_monitor_device_registration() or
*       cellular_hal_start_network() are still in place afterwards, and it does not
*       state whether a device opened before the reset remains open. A caller must
*       therefore assume nothing in either direction: it re-reads the state it depends
*       on - the control interface, the registration, the card and the profile list -
*       and re-establishes what is missing, while remaining prepared for a notification
*       through a callback it installed before the reset. The registration and any
*       started network must be treated as gone in any case, because the modem
*       restarts. On failure this interface does not state whether the reset was partly
*       performed.
*
* @note Blocking: synchronous. No timeout is specified, per 'Blocking calls' in
*       docs/pages/halSpec.md, and a reset is a hardware operation that takes a
*       perceptible time.
* @warning Not thread safe. Per 'Threading Model' in docs/pages/halSpec.md the
*          obligation is the caller's: serialise this call against every other Cellular
*          HAL call, including calls issued from another process. A concurrent call is
*          especially unsafe here, because the reset invalidates the context every other
*          call depends on.
* @see cellular_hal_modem_factory_reset, cellular_hal_set_modem_operating_configuration
*/
int cellular_hal_modem_reset( void );
/** @} */  //END OF GROUP CELLULAR_HAL_APIS
#endif //_CELLULAR_HAL_H_
